"""带边界检查的协议字段读取器。"""

from decimal import Decimal

from .models import IssueLevel, ParseField, ParseIssue


def _format_scaled_value(raw_value: int, scale: float) -> str:
    """@brief 使用十进制定点运算格式化带倍率整数，避免大数浮点精度丢失。
    @param[in] raw_value 原始整数值。
    @param[in] scale 字段倍率。
    @retval str 去除无意义尾零后的十进制文本。
    """
    result = format(Decimal(raw_value) * Decimal(str(scale)), "f")
    if "." in result:
        result = result.rstrip("0").rstrip(".")
    if result in ("", "-0"):
        result = "0"
    return result


class ByteReader:
    """顺序读取业务字节并生成可定位字段。"""

    def __init__(self, data: bytes, base_offset: int = 0):
        """@brief 初始化读取器。
        @param[in] data 待读取字节。
        @param[in] base_offset 数据在完整报文中的起始偏移。
        """
        self.data = data
        self.base_offset = base_offset
        self.position = 0
        self.issues: list[ParseIssue] = []

    @property
    def remaining(self) -> int:
        """@brief 返回尚未读取的字节数。"""
        return len(self.data) - self.position

    def read_bytes(self, name: str, length: int, note: str = "") -> ParseField:
        """@brief 安全读取指定长度字节，越界时读取剩余数据并记录错误。"""
        start = self.position
        actual = min(length, self.remaining)
        raw = self.data[start:start + actual]
        self.position += actual
        field_issues: list[ParseIssue] = []
        if actual != length:
            issue = ParseIssue(IssueLevel.ERROR, f"{name}字段越界：需要 {length} 字节，实际 {actual} 字节",
                               self.base_offset + start, actual)
            self.issues.append(issue)
            field_issues.append(issue)
        return ParseField(name, self.base_offset + start, actual, raw.hex(" ").upper(),
                          raw.hex(" ").upper(), note, issues=field_issues)

    def read_uint(self, name: str, length: int, scale: float = 1.0, unit: str = "") -> ParseField:
        """@brief 读取小端无符号整数并应用倍率。"""
        field = self.read_bytes(name, length, unit)
        raw = bytes.fromhex(field.raw_hex) if field.raw_hex else b""
        field.value = _format_scaled_value(int.from_bytes(raw, "little"), scale)
        return field

    def read_int(self, name: str, length: int, scale: float = 1.0, unit: str = "") -> ParseField:
        """@brief 读取小端有符号整数并应用倍率。"""
        field = self.read_bytes(name, length, unit)
        raw = bytes.fromhex(field.raw_hex) if field.raw_hex else b""
        field.value = _format_scaled_value(int.from_bytes(raw, "little", signed=True), scale)
        return field

    def read_bcd(self, name: str, length: int, reverse: bool = True) -> ParseField:
        """@brief 读取 BCD 字段并检查每个半字节合法性。"""
        field = self.read_bytes(name, length)
        raw = bytes.fromhex(field.raw_hex) if field.raw_hex else b""
        valid = all((byte >> 4) <= 9 and (byte & 0x0F) <= 9 for byte in raw)
        ordered = raw[::-1] if reverse else raw
        field.value = "".join(f"{byte:02X}" for byte in ordered)
        if not valid:
            issue = ParseIssue(IssueLevel.ERROR, f"{name}包含非法 BCD", field.offset, field.length)
            field.issues.append(issue)
            self.issues.append(issue)
        return field

    def read_ascii(self, name: str, length: int) -> ParseField:
        """@brief 读取定长 ASCII，去除尾部 NUL。"""
        field = self.read_bytes(name, length)
        raw = bytes.fromhex(field.raw_hex) if field.raw_hex else b""
        field.value = raw.rstrip(b"\x00").decode("ascii", errors="replace")
        return field

    def read_enum(self, name: str, length: int, values: dict[int, str]) -> ParseField:
        """@brief 读取枚举并显示数值和中文含义。"""
        field = self.read_uint(name, length)
        number = int(float(field.value))
        field.value = f"{number}（{values.get(number, '未知')}）"
        if number not in values:
            issue = ParseIssue(IssueLevel.WARNING, f"{name}枚举值未知", field.offset, field.length)
            field.issues.append(issue)
            self.issues.append(issue)
        return field

    def read_cp56time2a(self, name: str) -> ParseField:
        """@brief 读取 7 字节 CP56Time2a 时间并校验日期范围。"""
        field = self.read_bytes(name, 7)
        raw = bytes.fromhex(field.raw_hex) if field.raw_hex else b""
        if len(raw) == 7:
            millisecond = int.from_bytes(raw[0:2], "little")
            minute = raw[2] & 0x3F
            hour = raw[3] & 0x1F
            day = raw[4] & 0x1F
            month = raw[5] & 0x0F
            year = 2000 + (raw[6] & 0x7F)
            valid = minute < 60 and hour < 24 and 1 <= day <= 31 and 1 <= month <= 12
            field.value = f"{year:04d}-{month:02d}-{day:02d} {hour:02d}:{minute:02d}:{millisecond / 1000:06.3f}"
            if not valid:
                issue = ParseIssue(IssueLevel.ERROR, f"{name}日期时间非法", field.offset, field.length)
                field.issues.append(issue)
                self.issues.append(issue)
        return field
