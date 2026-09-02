import pathlib
import sys
import unittest


TOOL_ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOL_ROOT))

from parser.common import parse_frame
from parser.frame_splitter import split_frames
from parser.models import IssueLevel, ParseField, ParseIssue, ParsedFrame
from parser.reader import ByteReader


class ModelTest(unittest.TestCase):
    def test_frame_status_uses_highest_issue_level(self):
        frame = ParsedFrame(
            raw=b"\x68\x00",
            name="未知帧",
            direction="未知",
            fields=[ParseField("帧头", 0, 1, "68", "0x68")],
            issues=[
                ParseIssue(IssueLevel.WARNING, "存在残留"),
                ParseIssue(IssueLevel.ERROR, "长度错误", 1, 1),
            ],
        )
        self.assertEqual("错误", frame.status_text)


class SplitterTest(unittest.TestCase):
    def test_split_space_separated_frames(self):
        result = split_frames("RX: 68 04 83 00 00 00\nTX: 68 04 0B 00 00 00")
        self.assertEqual(
            [bytes.fromhex("68 04 83 00 00 00"), bytes.fromhex("68 04 0B 00 00 00")],
            result.frames,
        )

    def test_split_continuous_hex(self):
        result = split_frames("68048300000068040B000000")
        self.assertEqual(2, len(result.frames))

    def test_incomplete_frame_is_warning(self):
        result = split_frames("68 04 83 00")
        self.assertEqual([], result.frames)
        self.assertTrue(any("不完整" in issue.message for issue in result.issues))


class ReaderTest(unittest.TestCase):
    def test_read_scaled_uint(self):
        reader = ByteReader(bytes.fromhex("10 27 00 00"))
        field = reader.read_uint("电量", 4, scale=0.001, unit="kWh")
        self.assertEqual("10", field.value)
        self.assertEqual("kWh", field.note)

    def test_large_scaled_uint_keeps_fraction(self):
        reader = ByteReader(bytes.fromhex("EC BF 30 04"))
        field = reader.read_uint("余额", 4, scale=0.01, unit="元")
        self.assertEqual("703037.24", field.value)

    def test_invalid_bcd_adds_issue(self):
        reader = ByteReader(bytes.fromhex("FA"))
        field = reader.read_bcd("编号", 1)
        self.assertEqual(IssueLevel.ERROR, field.issues[0].level)

    def test_out_of_bounds_does_not_raise(self):
        reader = ByteReader(b"\x01")
        field = reader.read_uint("四字节字段", 4)
        self.assertEqual(1, field.length)
        self.assertTrue(reader.issues)


class CommonFrameTest(unittest.TestCase):
    def test_length_mismatch_is_error(self):
        frame = parse_frame(bytes.fromhex("68 05 83 00 00 00"))
        self.assertEqual("错误", frame.status_text)
        self.assertTrue(any("长度" in issue.message for issue in frame.issues))

    def test_unknown_frame_keeps_payload(self):
        frame = parse_frame(bytes.fromhex("68 04 AA BB CC DD"))
        self.assertTrue(any(field.name == "未解析业务数据" for field in frame.fields))


if __name__ == "__main__":
    unittest.main()
