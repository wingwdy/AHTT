import pathlib
import sys
import unittest


TOOL_ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOL_ROOT))

from parser.common import parse_frame


B6 = bytes.fromhex(
    "68 52 00 00 00 00 82 00 06 00 00 00 00 00 01 "
    "01 00 84 00 00 00 00 31 00 85 16 00 00 09 00 21 20 "
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
    "00 00 00 00 "
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"
)
B7 = bytes.fromhex(
    "68 5A 00 00 00 00 85 00 07 00 00 00 00 00 02 "
    "01 00 84 00 00 00 00 31 00 85 16 00 00 09 00 21 20 "
    "EE EE EE EE EE EE EE EE EE EE EE EE EE EE EE EE "
    "EE EE EE EE EE EE EE EE EE EE EE EE EE EE EE EE "
    "00 00 00 00 00 00 00 00 4C C0 30 04 01 00 00 "
    "00 00 00 00 00 00 00 00 00 00 00 00 00"
)
B10 = bytes.fromhex(
    "68 57 00 00 00 00 82 00 06 00 00 00 00 00 0E "
    "01 00 84 00 00 00 00 31 00 85 16 00 00 09 00 21 20 "
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
    "4C C0 30 04 "
    "EE EE EE EE EE EE EE EE EE EE EE EE EE EE EE EE "
    "EE EE EE EE EE EE EE EE EE EE EE EE EE EE EE EE "
    "04 00 00 00 00"
)
B11 = bytes.fromhex(
    "68 29 00 00 00 00 85 00 07 00 00 00 00 00 0C "
    "01 00 84 00 00 00 00 31 00 01 00 00 "
    "27 68 28 38 21 09 30 06 26 20 00 00 00 00 00 00"
)


class AuthenticationTest(unittest.TestCase):
    def test_b6_card_authentication_uplink(self):
        frame = parse_frame(B6)
        values = {field.name: field.value for field in frame.fields}
        self.assertEqual("B6 刷卡鉴权上行", frame.name)
        self.assertEqual("2021000900001685", values["物理卡号"])
        self.assertEqual("", values["密码"])
        self.assertEqual("0", values["账户余额"])
        self.assertEqual("", values["电动汽车唯一标识"])
        self.assertEqual("正常", frame.status_text)

    def test_b7_card_authentication_downlink(self):
        frame = parse_frame(B7)
        values = {field.name: field.value for field in frame.fields}
        self.assertEqual("B7 刷卡鉴权下行", frame.name)
        self.assertEqual("未提供（0xEE 填充）", values["电动汽车唯一标识"])
        self.assertEqual("703038.2", values["账户余额"])
        self.assertEqual("1（成功）", values["鉴权结果"])
        self.assertEqual("正常", frame.status_text)

    def test_b10_remote_start_notification(self):
        frame = parse_frame(B10)
        values = {field.name: field.value for field in frame.fields}
        self.assertEqual("B10 远程启动通知上报", frame.name)
        self.assertEqual("703038.2", values["账户余额"])
        self.assertEqual("未提供（0xEE 填充）", values["电动汽车唯一标识"])
        self.assertEqual("4（充满为止）", values["启动充电控制模式"])
        self.assertEqual("正常", frame.status_text)

    def test_b11_start_notification_downlink(self):
        frame = parse_frame(B11)
        values = {field.name: field.value for field in frame.fields}
        self.assertEqual("B11 启动通知下行", frame.name)
        self.assertEqual("1（成功）", values["启动通知结果"])
        self.assertEqual("00000000000020260630092138286827", values["订单号"])
        self.assertNotIn("电动汽车唯一标识", values)
        self.assertEqual("正常", frame.status_text)


if __name__ == "__main__":
    unittest.main()
