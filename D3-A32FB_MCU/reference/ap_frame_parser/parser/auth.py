"""B6/B7 刷卡鉴权与 B10/B11 启动通知报文解析。"""

from .common import register_parser
from .models import ParseField, ParseIssue
from .reader import ByteReader


def _read_identity(reader: ByteReader) -> list[ParseField]:
    """@brief 读取充电桩编号和充电接口标识。
    @param[in,out] reader 顺序字段读取器。
    @retval list 身份字段列表。
    """
    fields = [
        reader.read_bcd("充电桩编号", 8),
        reader.read_uint("充电接口标识", 1),
    ]
    return fields


def _read_vehicle_identity(reader: ByteReader, length: int) -> ParseField:
    """@brief 读取车辆标识，并识别全 0xEE 的未提供占位值。
    @param[in,out] reader 顺序字段读取器。
    @param[in] length 字段长度。
    @retval ParseField 车辆标识字段。
    """
    field = reader.read_bytes("电动汽车唯一标识", length)
    raw = bytes.fromhex(field.raw_hex) if field.raw_hex else b""
    if raw and all(byte == 0xEE for byte in raw):
        field.value = "未提供（0xEE 填充）"
    else:
        field.value = raw.rstrip(b"\x00").decode("ascii", errors="replace")
    return field


@register_parser(0x82, 0x06, 0x01, "B6 刷卡鉴权上行", "桩→平台")
def parse_b6(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B6 刷卡鉴权上行报文。
    @param[in] data B6 业务数据。
    @param[in] offset 业务数据在完整报文中的偏移。
    @retval tuple 字段列表和解析问题列表。
    """
    reader = ByteReader(data, offset)
    fields = _read_identity(reader)
    fields.extend([
        reader.read_bcd("物理卡号", 8),
        reader.read_ascii("密码", 16),
        reader.read_uint("账户余额", 4, 0.01, "元"),
        _read_vehicle_identity(reader, 32),
    ])
    return fields, reader.issues


@register_parser(0x85, 0x07, 0x02, "B7 刷卡鉴权下行", "平台→桩")
def parse_b7(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B7 刷卡鉴权下行报文。
    @param[in] data B7 业务数据。
    @param[in] offset 业务数据在完整报文中的偏移。
    @retval tuple 字段列表和解析问题列表。
    """
    reader = ByteReader(data, offset)
    fields = _read_identity(reader)
    fields.extend([
        reader.read_bcd("物理卡号", 8),
        _read_vehicle_identity(reader, 32),
        reader.read_bcd("计费模型编号", 8),
        reader.read_uint("账户余额", 4, 0.01, "元"),
        reader.read_enum("鉴权结果", 1, {0: "失败", 1: "成功"}),
        reader.read_bcd("失败原因", 2),
        reader.read_uint("剩余里程", 4, 1, "km"),
        reader.read_uint("可用电量", 4, 0.01, "kWh"),
        reader.read_uint("剩余次数", 4),
        reader.read_enum("充电模式", 1, {0: "有卡充电", 1: "无卡充电"}),
    ])
    if reader.remaining >= 8:
        fields.append(reader.read_ascii("车牌号", 8))
    return fields, reader.issues


@register_parser(0x82, 0x06, 0x0E, "B10 远程启动通知上报", "桩→平台")
def parse_b10(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B10 远程启动通知上报。
    @param[in] data B10 业务数据。
    @param[in] offset 业务数据在完整报文中的偏移。
    @retval tuple 字段列表和解析问题列表。
    """
    reader = ByteReader(data, offset)
    fields = _read_identity(reader)
    fields.extend([
        reader.read_bcd("物理卡号", 8),
        reader.read_ascii("密码", 16),
        reader.read_uint("账户余额", 4, 0.01, "元"),
        _read_vehicle_identity(reader, 32),
        reader.read_enum("启动充电控制模式", 1, {
            1: "电量控制",
            2: "时间控制",
            3: "金额控制",
            4: "充满为止",
        }),
        reader.read_uint("启动充电控制数据", 4),
    ])
    return fields, reader.issues


@register_parser(0x85, 0x07, 0x0C, "B11 启动通知下行", "平台→桩")
def parse_b11(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B11 启动通知下行报文及可选车辆信息。
    @param[in] data B11 业务数据。
    @param[in] offset 业务数据在完整报文中的偏移。
    @retval tuple 字段列表和解析问题列表。
    """
    reader = ByteReader(data, offset)
    fields = _read_identity(reader)
    fields.extend([
        reader.read_enum("启动通知结果", 1, {0: "失败", 1: "成功"}),
        reader.read_bcd("失败原因", 2),
        reader.read_bcd("订单号", 16),
    ])
    if reader.remaining >= 17:
        fields.append(_read_vehicle_identity(reader, 17))
    if reader.remaining >= 8:
        fields.append(reader.read_ascii("车牌号", 8))
    return fields, reader.issues
