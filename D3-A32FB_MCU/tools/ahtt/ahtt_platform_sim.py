#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""AHTT TCP 平台模拟器。

默认回复 0x01 签到，收到 0x81 心跳只记录而不回复，用于验证设备端
10 秒重试和 Offline 流程。使用 --reply-heartbeat 才会回复心跳。
"""

from __future__ import annotations

import argparse
import socket
import threading
import time
from dataclasses import dataclass
from typing import Optional


AHTT_FRAME_HEAD = 0xEA
AHTT_VERSION = 0x01
AHTT_DEVICE_NUM_LEN = 5
AHTT_FRAME_MIN_DECLARED_LEN = 11
AHTT_FRAME_MAX_LEN = 1024
AHTT_CMD_LOGIN = 0x01
AHTT_CMD_SET_DOMAIN_PORT = 0x04
AHTT_CMD_HEARTBEAT = 0x81
AHTT_SEQ_MIN = 1
AHTT_SEQ_MAX = 60000
AHTT_DOMAIN_FIELD_LEN = 24
AHTT_PORT_FIELD_LEN = 6


@dataclass(frozen=True)
class AhttFrame:
    raw: bytes
    version: int
    device_num: bytes
    sequence: int
    command: int
    parameter: bytes


def format_hex(data: bytes) -> str:
    return " ".join(f"{value:02X}" for value in data)


def crc16_modbus(data: bytes) -> int:
    crc = 0xFFFF
    for value in data:
        crc ^= value
        for _ in range(8):
            if crc & 0x0001:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc & 0xFFFF


def build_frame(device_num: bytes, sequence: int, command: int, parameter: bytes = b"") -> bytes:
    if len(device_num) != AHTT_DEVICE_NUM_LEN:
        raise ValueError("AHTT device number must contain 5 bytes")
    if not AHTT_SEQ_MIN <= sequence <= AHTT_SEQ_MAX:
        raise ValueError("AHTT sequence must be in range 1..60000")

    declared_len = AHTT_FRAME_MIN_DECLARED_LEN + len(parameter)
    frame = bytes([
        AHTT_FRAME_HEAD,
        declared_len & 0xFF,
        (declared_len >> 8) & 0xFF,
        AHTT_VERSION,
    ]) + device_num + bytes([
        sequence & 0xFF,
        (sequence >> 8) & 0xFF,
        command,
    ]) + parameter
    crc = crc16_modbus(frame)
    return frame + bytes([crc & 0xFF, (crc >> 8) & 0xFF])


def build_domain_switch_parameter(domain: str, port: int) -> bytes:
    if not domain:
        raise ValueError("AHTT domain must not be empty")
    if not 1 <= port <= 65535:
        raise ValueError("AHTT domain switch port must be in range 1..65535")

    try:
        domain_bytes = domain.encode("ascii")
    except UnicodeEncodeError as exc:
        raise ValueError("AHTT domain must contain ASCII characters only") from exc
    port_bytes = str(port).encode("ascii")
    if len(domain_bytes) > AHTT_DOMAIN_FIELD_LEN:
        raise ValueError("AHTT domain must not exceed 24 ASCII bytes")

    return (
        domain_bytes.ljust(AHTT_DOMAIN_FIELD_LEN, b"\x00")
        + port_bytes.ljust(AHTT_PORT_FIELD_LEN, b"\x00")
    )


def build_domain_switch_frame(
    device_num: bytes, sequence: int, domain: str, port: int
) -> bytes:
    return build_frame(
        device_num,
        sequence,
        AHTT_CMD_SET_DOMAIN_PORT,
        build_domain_switch_parameter(domain, port),
    )


def parse_frame(frame: bytes) -> AhttFrame:
    if len(frame) < AHTT_FRAME_MIN_DECLARED_LEN + 3:
        raise ValueError("AHTT frame is shorter than the minimum length")
    if frame[0] != AHTT_FRAME_HEAD:
        raise ValueError("AHTT frame head is invalid")

    declared_len = frame[1] | (frame[2] << 8)
    if declared_len < AHTT_FRAME_MIN_DECLARED_LEN or declared_len + 3 != len(frame):
        raise ValueError("AHTT declared length is invalid")
    if frame[3] != AHTT_VERSION:
        raise ValueError("AHTT version is invalid")

    received_crc = frame[-2] | (frame[-1] << 8)
    if crc16_modbus(frame[:-2]) != received_crc:
        raise ValueError("AHTT CRC16-MODBUS is invalid")

    return AhttFrame(
        raw=frame,
        version=frame[3],
        device_num=frame[4:4 + AHTT_DEVICE_NUM_LEN],
        sequence=frame[9] | (frame[10] << 8),
        command=frame[11],
        parameter=frame[12:-2],
    )


class AhttFrameDecoder:
    """面向 TCP 字节流的 AHTT 完整帧提取器。"""

    def __init__(self) -> None:
        self._buffer = b""

    def feed(self, data: bytes) -> list[bytes]:
        self._buffer += data
        frames: list[bytes] = []

        while self._buffer:
            head_index = self._buffer.find(bytes([AHTT_FRAME_HEAD]))
            if head_index < 0:
                self._buffer = b""
                break
            if head_index > 0:
                self._buffer = self._buffer[head_index:]
            if len(self._buffer) < 3:
                break

            declared_len = self._buffer[1] | (self._buffer[2] << 8)
            total_len = declared_len + 3
            if declared_len < AHTT_FRAME_MIN_DECLARED_LEN or total_len > AHTT_FRAME_MAX_LEN:
                self._buffer = self._buffer[1:]
                continue
            if len(self._buffer) < total_len:
                break

            candidate = self._buffer[:total_len]
            try:
                parse_frame(candidate)
            except ValueError:
                self._buffer = self._buffer[1:]
                continue
            frames.append(candidate)
            self._buffer = self._buffer[total_len:]

        return frames


class AhttMessageHandler:
    """为设备主动签到和心跳提供最小平台侧行为。"""

    def __init__(self, reply_heartbeat: bool, next_sequence: int = AHTT_SEQ_MIN) -> None:
        self.reply_heartbeat = reply_heartbeat
        self.next_sequence = next_sequence
        self.login_count = 0
        self.heartbeat_count = 0

    def _allocate_sequence(self) -> int:
        sequence = self.next_sequence
        self.next_sequence += 1
        if self.next_sequence > AHTT_SEQ_MAX:
            self.next_sequence = AHTT_SEQ_MIN
        return sequence

    def handle(self, raw_frame: bytes) -> Optional[bytes]:
        frame = parse_frame(raw_frame)
        if frame.command == AHTT_CMD_LOGIN:
            self.login_count += 1
            return build_frame(frame.device_num, self._allocate_sequence(), AHTT_CMD_LOGIN)
        if frame.command == AHTT_CMD_HEARTBEAT:
            self.heartbeat_count += 1
            if self.reply_heartbeat:
                return build_frame(frame.device_num, self._allocate_sequence(), AHTT_CMD_HEARTBEAT)
        return None

    def build_domain_switch(self, raw_login_frame: bytes, domain: str, port: int) -> bytes:
        frame = parse_frame(raw_login_frame)
        if frame.command != AHTT_CMD_LOGIN:
            raise ValueError("AHTT domain switch requires a login frame")
        return build_domain_switch_frame(
            frame.device_num,
            self._allocate_sequence(),
            domain,
            port,
        )


class AhttPlatformSimulator:
    def __init__(
        self,
        host: str,
        port: int,
        reply_heartbeat: bool,
        domain_switch: Optional[tuple[str, int]] = None,
        domain_switch_delay: float = 1.0,
    ) -> None:
        self.host = host
        self.port = port
        self.handler = AhttMessageHandler(reply_heartbeat=reply_heartbeat)
        self.domain_switch = domain_switch
        self.domain_switch_delay = domain_switch_delay
        self.domain_switch_sent = False
        self.server: Optional[socket.socket] = None
        self.client: Optional[socket.socket] = None
        self.lock = threading.Lock()
        self.stop_event = threading.Event()

    def start(self) -> None:
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server.bind((self.host, self.port))
        server.listen(1)
        self.server = server
        mode = "回复心跳" if self.handler.reply_heartbeat else "心跳静默"
        print(f"[SIM] listening on {self.host}:{self.port}, mode={mode}")
        threading.Thread(target=self._accept_loop, daemon=True).start()

    def close(self) -> None:
        self.stop_event.set()
        with self.lock:
            for sock in (self.client, self.server):
                if sock is not None:
                    try:
                        sock.close()
                    except OSError:
                        pass
            self.client = None
            self.server = None

    def _accept_loop(self) -> None:
        while not self.stop_event.is_set():
            try:
                if self.server is None:
                    return
                client, address = self.server.accept()
            except OSError:
                return

            with self.lock:
                if self.client is not None:
                    self.client.close()
                self.client = client
            print(f"[SIM] client connected: {address[0]}:{address[1]}")
            threading.Thread(target=self._recv_loop, args=(client,), daemon=True).start()

    def _recv_loop(self, client: socket.socket) -> None:
        decoder = AhttFrameDecoder()
        while not self.stop_event.is_set():
            try:
                data = client.recv(4096)
            except OSError:
                break
            if not data:
                break

            for raw_frame in decoder.feed(data):
                self._handle_frame(client, raw_frame)

        with self.lock:
            if self.client is client:
                self.client = None
        print("[SIM] client disconnected")

    def _handle_frame(self, client: socket.socket, raw_frame: bytes) -> None:
        frame = parse_frame(raw_frame)
        print(
            f"[RX] cmd=0x{frame.command:02X}, seq=0x{frame.sequence:04X}, "
            f"paramLen={len(frame.parameter)}, time={time.strftime('%H:%M:%S')}"
        )
        print(format_hex(raw_frame))

        response = self.handler.handle(raw_frame)
        if response is None:
            if frame.command == AHTT_CMD_HEARTBEAT:
                print("[SIM] heartbeat reply intentionally suppressed; TCP remains connected")
            return

        try:
            client.sendall(response)
        except OSError as exc:
            print(f"[ERR] send response failed: {exc}")
            return
        response_frame = parse_frame(response)
        print(
            f"[TX] cmd=0x{response_frame.command:02X}, seq=0x{response_frame.sequence:04X}, "
            f"paramLen=0"
        )
        print(format_hex(response))

        if response_frame.command == AHTT_CMD_LOGIN:
            self._schedule_domain_switch(client, raw_frame)

    def _schedule_domain_switch(self, client: socket.socket, raw_login_frame: bytes) -> None:
        with self.lock:
            if self.domain_switch is None or self.domain_switch_sent:
                return
            self.domain_switch_sent = True
        timer = threading.Timer(
            self.domain_switch_delay,
            self.send_domain_switch,
            args=(client, raw_login_frame),
        )
        timer.daemon = True
        timer.start()

    def send_domain_switch(self, client: socket.socket, raw_login_frame: bytes) -> None:
        if self.domain_switch is None:
            raise RuntimeError("AHTT domain switch endpoint is not configured")

        try:
            domain_switch = self.handler.build_domain_switch(
                raw_login_frame,
                self.domain_switch[0],
                self.domain_switch[1],
            )
            client.sendall(domain_switch)
        except (OSError, ValueError) as exc:
            print(f"[ERR] send domain switch failed: {exc}")
            return

        frame = parse_frame(domain_switch)
        print(
            f"[TX] cmd=0x{frame.command:02X}, seq=0x{frame.sequence:04X}, "
            f"paramLen={len(frame.parameter)}"
        )
        print(format_hex(domain_switch))


def selftest() -> int:
    device_num = bytes.fromhex("0100900534")
    heartbeat = build_frame(device_num, 2, AHTT_CMD_HEARTBEAT, bytes.fromhex("021BFCFFFF"))
    decoder = AhttFrameDecoder()
    if decoder.feed(heartbeat[:7]):
        return 1
    if decoder.feed(heartbeat[7:] + heartbeat) != [heartbeat, heartbeat]:
        return 1

    silent_handler = AhttMessageHandler(reply_heartbeat=False, next_sequence=0x0626)
    login = build_frame(device_num, 1, AHTT_CMD_LOGIN, bytes(28))
    if silent_handler.handle(login) != bytes.fromhex("EA0B00010100900534260601EB57"):
        return 1
    if silent_handler.handle(heartbeat) is not None:
        return 1

    reply_handler = AhttMessageHandler(reply_heartbeat=True, next_sequence=0x06A6)
    if reply_handler.handle(heartbeat) != bytes.fromhex("EA0B00010100900534A60681EB1F"):
        return 1
    print("AHTT platform simulator selftest: PASS")
    return 0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="AHTT TCP platform simulator")
    parser.add_argument("--host", default="0.0.0.0", help="listen host, default: 0.0.0.0")
    parser.add_argument("--port", type=int, default=18888, help="listen port, default: 18888")
    parser.add_argument(
        "--reply-heartbeat",
        action="store_true",
        help="reply to 0x81 heartbeat; omitted means intentionally suppress replies",
    )
    parser.add_argument(
        "--send-domain-switch-domain",
        help="send one 0x04 after the first login response",
    )
    parser.add_argument(
        "--send-domain-switch-port",
        type=int,
        help="candidate port for --send-domain-switch-domain",
    )
    parser.add_argument(
        "--send-domain-switch-delay",
        type=float,
        default=1.0,
        help="seconds to wait after the login response before sending 0x04",
    )
    parser.add_argument("--selftest", action="store_true", help="run protocol selftest and exit")
    args = parser.parse_args()
    if (args.send_domain_switch_domain is None) != (args.send_domain_switch_port is None):
        parser.error("domain switch requires both --send-domain-switch-domain and port")
    if args.send_domain_switch_domain is not None:
        try:
            build_domain_switch_parameter(
                args.send_domain_switch_domain,
                args.send_domain_switch_port,
            )
        except ValueError as exc:
            parser.error(str(exc))
    if args.send_domain_switch_delay < 0:
        parser.error("domain switch delay must not be negative")
    return args


def main() -> int:
    args = parse_args()
    if args.selftest:
        return selftest()

    domain_switch = None
    if args.send_domain_switch_domain is not None:
        domain_switch = (args.send_domain_switch_domain, args.send_domain_switch_port)
    simulator = AhttPlatformSimulator(
        args.host,
        args.port,
        args.reply_heartbeat,
        domain_switch=domain_switch,
        domain_switch_delay=args.send_domain_switch_delay,
    )
    try:
        simulator.start()
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\n[SIM] stopping")
    finally:
        simulator.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
