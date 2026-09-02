import pathlib
import sys
import unittest


TOOL_ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOL_ROOT))

from parser.common import parse_frame


B48 = bytes.fromhex(
    "68 1F 00 00 00 00 82 00 06 00 00 00 00 00 24 "
    "01 00 84 00 00 00 00 31 00 00 00 00 10 00 41 38 94 00"
)
B49 = bytes.fromhex(
    "68 26 00 00 00 00 82 00 07 00 00 00 00 00 25 "
    "01 00 84 00 00 00 00 31 00 79 89 09 00 10 00 00 00 00 39 10 0F 05 14 00"
)
B54 = bytes.fromhex(
    "68 27 00 00 00 00 85 00 06 00 00 00 00 00 43 "
    "01 00 84 00 00 00 00 31 00 00 "
    "27 68 28 38 21 09 30 06 26 20 00 00 00 00 00 00"
)
B14 = bytes.fromhex(
    "68 40 00 00 00 00 85 00 06 00 00 00 00 00 03 "
    "01 00 84 00 00 00 00 31 85 16 00 00 09 00 21 20 "
    "60 00 00 00 EC BF 30 04 01 00 00 "
    "00 00 00 00 00 00 00 00 00 00 00 00 "
    "00 00 00 00 00 00 00 00 00 00 00 00"
)


class BillingTest(unittest.TestCase):
    def test_b48_fields(self):
        frame = parse_frame(B48)
        names = [field.name for field in frame.fields]
        self.assertEqual("B48 计费模型下发结果", frame.name)
        self.assertIn("充电桩编号", names)
        self.assertIn("计费模型ID", names)
        self.assertEqual("正常", frame.status_text)

    def test_b49_switch_time(self):
        frame = parse_frame(B49)
        self.assertEqual("B49 计费模型切换生效上报", frame.name)
        self.assertTrue(any(field.name == "切换时间" for field in frame.fields))


class TransactionTest(unittest.TestCase):
    def test_b54_order_ack(self):
        frame = parse_frame(B54)
        self.assertEqual("B54 在线分时交易确认", frame.name)
        order = next(field for field in frame.fields if field.name == "订单号")
        self.assertEqual(16, order.length)
        self.assertEqual("00000000000020260630092138286827", order.value)
        self.assertEqual("正常", frame.status_text)

    def test_b14_large_balance_keeps_fraction(self):
        frame = parse_frame(B14)
        values = {field.name: field.value for field in frame.fields}
        self.assertEqual("B14 扣款结果", frame.name)
        self.assertEqual("0.96", values["扣款金额"])
        self.assertEqual("703037.24", values["账户余额"])
        self.assertEqual("1（成功）", values["扣款成功标志"])
        self.assertEqual("正常", frame.status_text)


class RealtimeTest(unittest.TestCase):
    def test_b1_short_header_starts_payload_at_11(self):
        payload = bytes.fromhex("01 00 84 00 00 00 00 31 00 01 03 00") + bytes(145)
        frame = bytes([0x68, len(payload) + 9, 0, 0, 0, 0, 0x86, 0, 6, 0, 0]) + payload
        result = parse_frame(frame)
        pile = next(field for field in result.fields if field.name == "充电桩编号")
        self.assertEqual(11, pile.offset)
        self.assertEqual("3100000000840001", pile.value)


class TimeSyncTest(unittest.TestCase):
    def test_f8_time_starts_after_information_address(self):
        frame = bytes.fromhex(
            "68 13 00 00 00 00 67 00 07 00 00 00 00 00 "
            "3B C8 2F 08 1E 06 1A"
        )
        result = parse_frame(frame)
        sync_time = next(field for field in result.fields if field.name == "同步时间")
        self.assertEqual(14, sync_time.offset)
        self.assertEqual("3B C8 2F 08 1E 06 1A", sync_time.raw_hex)
        self.assertEqual("2026-06-30 08:47:51.259", sync_time.value)
        self.assertEqual("正常", result.status_text)


if __name__ == "__main__":
    unittest.main()
