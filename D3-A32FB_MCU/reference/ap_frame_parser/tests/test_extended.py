import pathlib
import sys
import unittest


TOOL_ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOL_ROOT))

from parser.common import parse_frame


B39 = bytes.fromhex(
    "68 65 00 00 00 00 85 00 06 00 00 00 00 00 3C "
    "01 00 84 00 00 00 00 31 78 37 B7 A4 15 00 "
    "7A 6A 65 63 00 00 00 00 00 00 "
    "31 32 33 34 35 36 00 00 00 00"
) + b"/hongyuan".ljust(50, b"\x00") + bytes.fromhex("01 04 00 01")
B40 = bytes.fromhex(
    "68 16 00 00 00 00 82 00 07 00 00 00 00 00 20 "
    "01 00 84 00 00 00 00 31 00"
)
B31 = bytes.fromhex(
    "68 34 00 00 00 00 82 00 07 00 00 00 00 00 1B "
    "01 00 84 00 00 00 00 31 "
    "38 39 38 36 30 38 38 30 31 31 32 35 34 30 30 33 39 30 34 32 "
    "00 00 00 00 00 00 00 00 00 00 00"
)
B33 = bytes.fromhex(
    "68 25 00 00 00 00 85 00 06 00 00 00 00 00 3A "
    "01 00 84 00 00 00 00 31 00 "
    "90 E2 2F 08 1E 06 1A 01 58 02 00 00 00 EE EE"
)
B34 = bytes.fromhex(
    "68 1E 00 00 00 00 82 00 06 00 00 00 00 00 1C "
    "01 00 84 00 00 00 00 31 00 "
    "90 E2 2F 08 1E 06 1A 00"
)
B45 = bytes.fromhex(
    "68 16 00 00 00 00 85 00 06 00 00 00 00 00 3F "
    "01 00 84 00 00 00 00 31 00"
)
B46 = bytes.fromhex(
    "68 23 00 00 00 00 82 00 07 00 00 00 00 00 23 "
    "01 00 84 00 00 00 00 31 00 "
    "00 00 00 00 BC 02 00 00 00 00 00 00 00"
)
B23 = (
    bytes.fromhex(
        "68 C6 00 00 00 00 85 00 06 00 00 00 00 00 0F "
        "01 00 84 00 00 00 00 31 8B E0 43 60 15 00"
    )
    + b"admin".ljust(10, b"\x00")
    + b"bUpdate".ljust(10, b"\x00")
    + b"/AC_pile/ap".ljust(50, b"\x00")
    + b"D3-A32FB.bin".ljust(50, b"\x00")
    + bytes([3])
    + bytes(30)
    + bytes(20)
)
B57 = bytes.fromhex(
    "68 2A 00 00 00 00 82 00 06 00 00 00 00 00 29 "
    "01 00 84 00 00 00 00 31 00 "
    "F0 D2 15 09 61 07 1A "
    "CC 08 6D 02 00 90 01 00 00 00 00 00 00"
)
B24 = bytes.fromhex(
    "68 16 00 00 00 00 82 00 07 00 00 00 00 00 0E "
    "01 00 84 00 00 00 00 31 00"
)
B4 = bytes.fromhex(
    "68 35 00 00 00 00 85 00 06 00 00 00 00 00 15 "
    "01 00 84 00 00 00 00 31 00 "
    "01 00 03 02 00 00 00 "
    "72 50 00 07 52 20 15 07 "
    "72 50 00 07 52 20 15 07 26 20 00 00 00 00 00 00"
)
B5 = bytes.fromhex(
    "68 31 00 00 00 00 85 00 07 00 00 00 00 00 15 "
    "01 00 84 00 00 00 00 31 00 "
    "00 00 00 01 "
    "10 27 34 14 6F 07 1A "
    "72 50 00 07 52 20 15 07 26 20 00 00 00 00 00 00"
)


def field_value(frame, name):
    field = next(item for item in frame.fields if item.name == name)
    return field.value


class ExtendedFrameTest(unittest.TestCase):
    def test_b23_remote_upgrade(self):
        frame = parse_frame(B23)
        self.assertEqual(200, len(B23))
        self.assertEqual("B23 远程升级启动", frame.name)
        self.assertEqual("139.224.67.96", field_value(frame, "FTP服务器"))
        self.assertEqual("21", field_value(frame, "端口号"))
        self.assertEqual("admin", field_value(frame, "FTP用户名"))
        self.assertEqual("bUpdate", field_value(frame, "FTP密码"))
        self.assertEqual("/AC_pile/ap", field_value(frame, "FTP路径"))
        self.assertEqual("D3-A32FB.bin", field_value(frame, "FTP文件名"))
        self.assertEqual("3", field_value(frame, "升级型号"))
        self.assertEqual("正常", frame.status_text)

    def test_b24_remote_upgrade_result(self):
        frame = parse_frame(B24)
        self.assertEqual("B24 远程升级启动结果", frame.name)
        self.assertEqual("成功，失败原因=0（无）", field_value(frame, "升级结果"))
        self.assertEqual("正常", frame.status_text)

    def test_b31_sim_information(self):
        frame = parse_frame(B31)
        self.assertEqual("B31 SIM卡信息上报", frame.name)
        self.assertEqual("89860880112540039042", field_value(frame, "SIM卡卡号(ICCID)"))
        self.assertEqual("", field_value(frame, "手机号码"))
        self.assertEqual("正常", frame.status_text)

    def test_b33_power_control(self):
        frame = parse_frame(B33)
        self.assertEqual("B33 充电功率控制下发", frame.name)
        self.assertEqual("2026-06-30 08:47:58.000", field_value(frame, "时间戳序号"))
        self.assertEqual("1（默认功率）", field_value(frame, "功率控制类别"))
        self.assertEqual("6", field_value(frame, "功率值"))
        self.assertEqual("正常", frame.status_text)

    def test_b34_power_control_result(self):
        frame = parse_frame(B34)
        self.assertEqual("B34 充电功率控制结果", frame.name)
        self.assertEqual("2026-06-30 08:47:58.000", field_value(frame, "时间戳序号"))
        self.assertEqual("0（成功）", field_value(frame, "成功标志"))
        self.assertEqual("正常", frame.status_text)

    def test_b45_power_query(self):
        frame = parse_frame(B45)
        self.assertEqual("B45 充电功率召测下发", frame.name)
        self.assertEqual("3100000000840001", field_value(frame, "充电桩编号"))
        self.assertEqual("0", field_value(frame, "充电接口标识"))
        self.assertEqual("正常", frame.status_text)

    def test_b46_power_query_result(self):
        frame = parse_frame(B46)
        self.assertEqual("B46 充电功率召测上行", frame.name)
        self.assertEqual("3100000000840001", field_value(frame, "充电桩编号"))
        self.assertEqual("0", field_value(frame, "充电接口标识"))
        self.assertEqual("0", field_value(frame, "控制功率"))
        self.assertEqual("7", field_value(frame, "默认功率"))
        self.assertEqual("0", field_value(frame, "动态功率"))
        self.assertEqual("0（成功）", field_value(frame, "成功标志"))
        self.assertEqual("正常", frame.status_text)

    def test_b57_power_realtime_status(self):
        frame = parse_frame(B57)
        self.assertEqual("B57 功率控制实时状态", frame.name)
        self.assertEqual("3100000000840001", field_value(frame, "充电桩编号"))
        self.assertEqual("0", field_value(frame, "充电接口标识"))
        self.assertEqual("2026-07-01 09:21:54.000", field_value(frame, "时间戳序号"))
        self.assertEqual("225.2", field_value(frame, "输出电压"))
        self.assertEqual("62.1", field_value(frame, "输出电流"))
        self.assertEqual("40", field_value(frame, "当前功率"))
        self.assertEqual("正常", frame.status_text)

    def test_b39_ftp_parameters(self):
        frame = parse_frame(B39)
        self.assertEqual("B39 FTP服务器地址下发", frame.name)
        self.assertEqual("120.55.183.164", field_value(frame, "FTP服务器"))
        self.assertEqual("21", field_value(frame, "端口号"))
        self.assertEqual("zjec", field_value(frame, "FTP用户名"))
        self.assertEqual("123456", field_value(frame, "FTP密码"))
        self.assertEqual("/hongyuan", field_value(frame, "FTP路径"))
        self.assertEqual("1（主动上报模式）", field_value(frame, "数据交互方式"))
        self.assertEqual("4", field_value(frame, "主动上报周期"))
        self.assertEqual("1（账单日志）", field_value(frame, "日志类型"))
        self.assertEqual("正常", frame.status_text)

    def test_b40_ftp_result(self):
        frame = parse_frame(B40)
        self.assertEqual("B40 FTP地址下发结果", frame.name)
        self.assertEqual("0（成功）", field_value(frame, "结果"))
        self.assertEqual("正常", frame.status_text)

    def test_b4_charge_ctrl(self):
        frame = parse_frame(B4)
        self.assertEqual("B4 充电启停控制下发", frame.name)
        self.assertEqual("3100000000840001", field_value(frame, "充电桩编号"))
        self.assertEqual("0", field_value(frame, "充电接口标识"))
        self.assertEqual("1（启动充电）", field_value(frame, "控制命令"))
        self.assertEqual("0（即时充电）", field_value(frame, "启动条件"))
        self.assertEqual("3（金额控制）", field_value(frame, "启动方式"))
        self.assertEqual("2", field_value(frame, "控制数据"))
        self.assertEqual("72 50 00 07 52 20 15 07", field_value(frame, "用户编号"))
        self.assertEqual("72 50 00 07 52 20 15 07 26 20 00 00 00 00 00 00", field_value(frame, "订单号"))
        self.assertEqual("正常", frame.status_text)

    def test_b5_charge_ctrl_result(self):
        frame = parse_frame(B5)
        field_names = [field.name for field in frame.fields]
        reason_field = next(field for field in frame.fields if field.name == "启动充电失败原因")
        ctrl_field = next(field for field in frame.fields if field.name == "控制命令")
        time_field = next(field for field in frame.fields if field.name == "控制时间")
        order_field = next(field for field in frame.fields if field.name == "订单号")
        self.assertEqual("B5 充电启停控制结果", frame.name)
        self.assertEqual(51, len(B5))
        self.assertEqual("3100000000840001", field_value(frame, "充电桩编号"))
        self.assertEqual("0", field_value(frame, "充电接口标识"))
        self.assertEqual("0（成功）", field_value(frame, "成功标志"))
        self.assertNotIn("预留", field_names)
        self.assertEqual(2, reason_field.length)
        self.assertEqual("00 00", reason_field.raw_hex)
        self.assertEqual("0000（成功）", reason_field.value)
        self.assertEqual(27, ctrl_field.offset)
        self.assertEqual(28, time_field.offset)
        self.assertEqual("1（启动充电）", field_value(frame, "控制命令"))
        self.assertEqual("2026-07-15 20:52:10.000", field_value(frame, "控制时间"))
        self.assertEqual(35, order_field.offset)
        self.assertEqual(16, order_field.length)
        self.assertEqual("72 50 00 07 52 20 15 07 26 20 00 00 00 00 00 00", order_field.raw_hex)
        self.assertEqual("00000000000020260715205207005072", order_field.value)
        self.assertEqual("正常", frame.status_text)

        failure_data = bytearray(B5)
        failure_data[24] = 1
        failure_data[25:27] = bytes.fromhex("01 00")
        failure_frame = parse_frame(bytes(failure_data))
        self.assertEqual("0001（正在充电中）", field_value(failure_frame, "启动充电失败原因"))


if __name__ == "__main__":
    unittest.main()
