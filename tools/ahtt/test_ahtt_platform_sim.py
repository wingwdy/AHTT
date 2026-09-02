#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""AHTT平台模拟器的协议行为测试。"""

import socket
import unittest

from ahtt_platform_sim import (
    AhttFrameDecoder,
    AhttMessageHandler,
    AhttPlatformSimulator,
    build_domain_switch_frame,
    build_domain_switch_parameter,
    parse_frame,
)


LOGIN_REQUEST = bytes.fromhex(
    "EA 27 00 01 01 00 90 05 34 01 00 01 "
    "38 39 38 36 30 38 35 36 31 31 32 34 39 30 32 39 38 36 36 30 "
    "02 04 00 00 00 00 00 00 10 69"
)
LOGIN_RESPONSE = bytes.fromhex("EA0B00010100900534260601EB57")
HEARTBEAT_REQUEST = bytes.fromhex("EA1000010100900534020081021BFCFFFFBA17")
HEARTBEAT_RESPONSE = bytes.fromhex("EA0B00010100900534A60681EB1F")


class AhttPlatformSimulatorTest(unittest.TestCase):
    def test_domain_switch_parameter_uses_trailing_zero_padding(self):
        parameter = build_domain_switch_parameter("bore.pub", 58405)

        self.assertEqual(
            parameter,
            bytes.fromhex(
                "62 6F 72 65 2E 70 75 62 00 00 00 00 00 00 00 00 "
                "00 00 00 00 00 00 00 00 35 38 34 30 35 00"
            ),
        )

    def test_domain_switch_frame_preserves_platform_sequence_and_payload(self):
        frame = build_domain_switch_frame(
            bytes.fromhex("0100900534"),
            0x1234,
            "bore.pub",
            58405,
        )
        parsed = parse_frame(frame)

        self.assertEqual(parsed.command, 0x04)
        self.assertEqual(parsed.sequence, 0x1234)
        self.assertEqual(parsed.parameter, bytes.fromhex(
            "62 6F 72 65 2E 70 75 62 00 00 00 00 00 00 00 00 "
            "00 00 00 00 00 00 00 00 35 38 34 30 35 00"
        ))

    def test_platform_allocates_independent_sequence_for_domain_switch(self):
        handler = AhttMessageHandler(reply_heartbeat=False, next_sequence=0x0626)

        handler.handle(LOGIN_REQUEST)
        domain_switch = handler.build_domain_switch(
            LOGIN_REQUEST,
            "bore.pub",
            58405,
        )
        parsed = parse_frame(domain_switch)

        self.assertEqual(parsed.command, 0x04)
        self.assertEqual(parsed.sequence, 0x0627)
        self.assertEqual(parsed.device_num, bytes.fromhex("0100900534"))

    def test_simulator_sends_domain_switch_to_the_login_connection(self):
        simulator = AhttPlatformSimulator(
            "127.0.0.1",
            18888,
            reply_heartbeat=False,
            domain_switch=("bore.pub", 58405),
        )
        server, client = socket.socketpair()
        try:
            simulator.send_domain_switch(server, LOGIN_REQUEST)
            received = client.recv(128)
        finally:
            server.close()
            client.close()

        parsed = parse_frame(received)
        self.assertEqual(parsed.command, 0x04)
        self.assertEqual(parsed.device_num, bytes.fromhex("0100900534"))
        self.assertEqual(parsed.sequence, 1)

    def test_login_request_returns_independent_sequence_zero_parameter_response(self):
        handler = AhttMessageHandler(reply_heartbeat=False, next_sequence=0x0626)

        response = handler.handle(LOGIN_REQUEST)

        self.assertEqual(response, LOGIN_RESPONSE)

    def test_heartbeat_is_silent_by_default(self):
        handler = AhttMessageHandler(reply_heartbeat=False, next_sequence=0x06A6)

        response = handler.handle(HEARTBEAT_REQUEST)

        self.assertIsNone(response)
        self.assertEqual(handler.heartbeat_count, 1)

    def test_heartbeat_reply_mode_returns_zero_parameter_response(self):
        handler = AhttMessageHandler(reply_heartbeat=True, next_sequence=0x06A6)

        response = handler.handle(HEARTBEAT_REQUEST)

        self.assertEqual(response, HEARTBEAT_RESPONSE)

    def test_decoder_handles_split_and_joined_tcp_frames(self):
        decoder = AhttFrameDecoder()

        self.assertEqual(decoder.feed(HEARTBEAT_REQUEST[:7]), [])
        self.assertEqual(
            decoder.feed(HEARTBEAT_REQUEST[7:] + HEARTBEAT_REQUEST),
            [HEARTBEAT_REQUEST, HEARTBEAT_REQUEST],
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
