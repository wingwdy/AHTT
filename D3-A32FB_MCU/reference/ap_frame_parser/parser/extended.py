"""B31、B33/B34、B39/B40、B45/B46、B57 扩展报文解析。"""

from .common import register_parser
from .models import ParseField, ParseIssue
from .reader import ByteReader


def _read_identity(reader: ByteReader, with_interface: bool) -> list[ParseField]:
    """@brief 读取扩展报文的桩编号和可选充电接口标识。
    @param[in,out] reader 顺序字段读取器。
    @param[in] with_interface 是否包含充电接口标识。
    @retval list 身份字段列表。
    """
    fields = [reader.read_bcd("充电桩编号", 8)]
    if with_interface:
        fields.append(reader.read_uint("充电接口标识", 1))
    return fields


@register_parser(0x85, 0x06, 0x0F, "B23 远程升级启动", "平台→桩")
def parse_b23(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B23 远程升级 FTP 和目标版本参数。
    @param[in] data B23 业务数据。
    @param[in] offset 业务数据在完整报文中的偏移。
    @retval tuple 字段列表和解析问题列表。
    """
    reader = ByteReader(data, offset)
    fields = _read_identity(reader, False)
    ip_field = reader.read_bytes("FTP服务器", 4)
    ip_raw = bytes.fromhex(ip_field.raw_hex) if ip_field.raw_hex else b""
    ip_field.value = ".".join(str(byte) for byte in ip_raw)
    fields.extend([
        ip_field,
        reader.read_uint("端口号", 2, 1, "TCP"),
        reader.read_ascii("FTP用户名", 10),
        reader.read_ascii("FTP密码", 10),
        reader.read_ascii("FTP路径", 50),
        reader.read_ascii("FTP文件名", 50),
        reader.read_uint("升级型号", 1),
        reader.read_ascii("升级硬件版本号", 30),
        reader.read_ascii("升级软件版本号", 20),
    ])
    return fields, reader.issues


@register_parser(0x82, 0x07, 0x0E, "B24 远程升级启动结果", "桩→平台")
@register_parser(0x82, 0x07, 0x10, "B24 远程升级启动结果", "桩→平台")
def parse_b24(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B24 结果位和低七位失败原因。
    @note 同时兼容当前固件 recordKind=0x0E 和 PDF recordKind=0x10。
    @param[in] data B24 业务数据。
    @param[in] offset 业务数据在完整报文中的偏移。
    @retval tuple 字段列表和解析问题列表。
    """
    reader = ByteReader(data, offset)
    fields = _read_identity(reader, False)
    result_field = reader.read_bytes("升级结果", 1)
    raw = bytes.fromhex(result_field.raw_hex) if result_field.raw_hex else b""
    result_value = raw[0] if raw else 0
    success_text = "失败" if result_value & 0x80 else "成功"
    reason = result_value & 0x7F
    reason_text = {0: "无", 1: "SM4密钥错误", 127: "其他原因"}.get(reason, "未定义原因")
    result_field.value = f"{success_text}，失败原因={reason}（{reason_text}）"
    fields.append(result_field)
    return fields, reader.issues


@register_parser(0x82, 0x07, 0x1B, "B31 SIM卡信息上报", "桩→平台")
def parse_b31(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B31 SIM 卡 ICCID 和手机号码。
    @param[in] data B31 业务数据。
    @param[in] offset 业务数据在完整报文中的偏移。
    @retval tuple 字段列表和解析问题列表。
    """
    reader = ByteReader(data, offset)
    fields = _read_identity(reader, False)
    fields.extend([
        reader.read_ascii("SIM卡卡号(ICCID)", 20),
        reader.read_ascii("手机号码", 11),
    ])
    return fields, reader.issues


@register_parser(0x85, 0x06, 0x3A, "B33 充电功率控制下发", "平台→桩")
def parse_b33(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B33 功率控制类别、目标功率和上报策略。
    @param[in] data B33 业务数据。
    @param[in] offset 业务数据在完整报文中的偏移。
    @retval tuple 字段列表和解析问题列表。
    """
    reader = ByteReader(data, offset)
    fields = _read_identity(reader, True)
    fields.extend([
        reader.read_cp56time2a("时间戳序号"),
        reader.read_enum("功率控制类别", 1, {1: "默认功率", 2: "动态功率", 3: "控制功率"}),
        reader.read_uint("功率值", 4, 0.01, "kW"),
        reader.read_enum("暂停充电", 1, {0: "否", 1: "是"}),
        reader.read_uint("实时数据上报周期", 2, 1, "s"),
    ])
    return fields, reader.issues


@register_parser(0x82, 0x06, 0x1C, "B34 充电功率控制结果", "桩→平台")
def parse_b34(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B34 功率控制执行结果。
    @param[in] data B34 业务数据。
    @param[in] offset 业务数据在完整报文中的偏移。
    @retval tuple 字段列表和解析问题列表。
    """
    reader = ByteReader(data, offset)
    fields = _read_identity(reader, True)
    fields.extend([
        reader.read_cp56time2a("时间戳序号"),
        reader.read_enum("成功标志", 1, {0: "成功", 1: "失败"}),
    ])
    return fields, reader.issues


@register_parser(0x82, 0x06, 0x29, "B57 功率控制实时状态", "桩→平台")
def parse_b57(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B57 功率控制过程中的扩展实时状态。
    @note 字段含充电桩编号、枪号、时间、电压/电流/功率和预留。
    @param[in] data B57 业务数据。
    @param[in] offset 业务数据在完整报文中的偏移。
    @retval tuple 字段列表和解析问题列表。
    """
    reader = ByteReader(data, offset)
    fields = _read_identity(reader, True)
    fields.extend([
        reader.read_cp56time2a("时间戳序号"),
        reader.read_uint("输出电压", 2, 0.1, "V"),
        reader.read_uint("输出电流", 2, 0.1, "A"),
        reader.read_bytes("预留", 1),
        reader.read_uint("当前功率", 2, 0.1, "kW"),
        reader.read_bytes("预留", 6),
    ])
    return fields, reader.issues


@register_parser(0x85, 0x06, 0x3C, "B39 FTP服务器地址下发", "平台→桩")
def parse_b39(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B39 FTP 连接和日志上传参数。
    @note 密码按用户确认完整显示，便于协议联调。
    @param[in] data B39 业务数据。
    @param[in] offset 业务数据在完整报文中的偏移。
    @retval tuple 字段列表和解析问题列表。
    """
    reader = ByteReader(data, offset)
    fields = _read_identity(reader, False)
    ip_field = reader.read_bytes("FTP服务器", 4)
    ip_raw = bytes.fromhex(ip_field.raw_hex) if ip_field.raw_hex else b""
    ip_field.value = ".".join(str(byte) for byte in ip_raw)
    fields.extend([
        ip_field,
        reader.read_uint("端口号", 2, 1, "TCP"),
        reader.read_ascii("FTP用户名", 10),
        reader.read_ascii("FTP密码", 10),
        reader.read_ascii("FTP路径", 50),
        reader.read_enum("数据交互方式", 1, {
            1: "主动上报模式",
            2: "召测应答模式",
            3: "上报+召测",
        }),
        reader.read_uint("主动上报周期", 2, 1, "h"),
        reader.read_enum("日志类型", 1, {
            1: "账单日志",
            2: "运行日志（故障）",
            3: "BMS日志",
            4: "配置文件",
            0xFF: "全部",
        }),
    ])
    return fields, reader.issues


@register_parser(0x82, 0x07, 0x20, "B40 FTP地址下发结果", "桩→平台")
def parse_b40(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B40 FTP 参数接收结果。
    @param[in] data B40 业务数据。
    @param[in] offset 业务数据在完整报文中的偏移。
    @retval tuple 字段列表和解析问题列表。
    """
    reader = ByteReader(data, offset)
    fields = _read_identity(reader, False)
    fields.append(reader.read_enum("结果", 1, {0: "成功", 1: "失败"}))
    return fields, reader.issues


@register_parser(0x85, 0x06, 0x3F, "B45 充电功率召测下发", "平台→桩")
def parse_b45(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B45 充电功率召测下发。
    @param[in] data B45 业务数据。
    @param[in] offset 业务数据在完整报文中的偏移。
    @retval tuple 字段列表和解析问题列表。
    """
    reader = ByteReader(data, offset)
    fields = _read_identity(reader, True)
    return fields, reader.issues


@register_parser(0x82, 0x07, 0x23, "B46 充电功率召测上行", "桩→平台")
def parse_b46(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B46 当前功率策略和召测结果。
    @param[in] data B46 业务数据。
    @param[in] offset 业务数据在完整报文中的偏移。
    @retval tuple 字段列表和解析问题列表。
    """
    reader = ByteReader(data, offset)
    fields = _read_identity(reader, True)
    fields.extend([
        reader.read_uint("控制功率", 4, 0.01, "kW"),
        reader.read_uint("默认功率", 4, 0.01, "kW"),
        reader.read_uint("动态功率", 4, 0.01, "kW"),
        reader.read_enum("成功标志", 1, {0: "成功", 1: "失败"}),
    ])
    return fields, reader.issues


@register_parser(0x85, 0x06, 0x15, "B4 充电启停控制下发", "平台→桩")
def parse_b4(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B4 充电启停控制下发参数。
    @note 业务数据 40 字节：桩号BCD逆序→枪号→控制命令→启动条件→启动方式→控制数据→用户编号→订单号。
    @param[in] data B4 业务数据。
    @param[in] offset 业务数据在完整报文中的偏移。
    @retval tuple 字段列表和解析问题列表。
    """
    reader = ByteReader(data, offset)
    fields = _read_identity(reader, True)
    fields.append(reader.read_enum("控制命令", 1, {0: "停止充电", 1: "启动充电"}))
    fields.append(reader.read_enum("启动条件", 1, {0: "即时充电", 1: "定时充电"}))
    way_field = reader.read_uint("启动方式", 1)
    way_num = int(float(way_field.value))
    way_text = {1: "电量控制", 2: "时间控制", 3: "金额控制", 4: "充满为止"}.get(way_num, "未知")
    way_field.value = f"{way_num}（{way_text}）"
    fields.append(way_field)
    if way_num == 1:
        ctrl_data = reader.read_uint("控制数据", 4, 1, "kWh")
    elif way_num == 2:
        ctrl_data = reader.read_uint("控制数据", 4, 1, "min")
    elif way_num == 3:
        ctrl_data = reader.read_uint("控制数据", 4, 1, "元")
    else:
        ctrl_data = reader.read_uint("控制数据", 4)
    fields.append(ctrl_data)
    fields.append(reader.read_bytes("用户编号", 8))
    fields.append(reader.read_bytes("订单号", 16))
    return fields, reader.issues


@register_parser(0x85, 0x07, 0x15, "B5 充电启停控制结果", "桩→平台")
def parse_b5(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B5 充电启停控制结果确认。
    @note 业务数据 36 字节：桩号BCD逆序→枪号→成功标志→启动充电失败原因(2字节小端BCD)→控制命令→控制时间(CP56)→订单号(16字节BCD逆序)。
    @param[in] data B5 业务数据。
    @param[in] offset 业务数据在完整报文中的偏移。
    @retval tuple 字段列表和解析问题列表。
    """
    reader = ByteReader(data, offset)
    fields = _read_identity(reader, True)
    fields.append(reader.read_enum("成功标志", 1, {0: "成功", 1: "失败"}))
    reason_field = reader.read_bcd("启动充电失败原因", 2)
    raw = bytes.fromhex(reason_field.raw_hex) if reason_field.raw_hex else b""
    if len(raw) == 2 and all((byte >> 4) <= 9 and (byte & 0x0F) <= 9 for byte in raw):
        reason_digits = reason_field.value
        reason_val = int(reason_digits)
        reason_text = {0: "成功", 1: "正在充电中", 2: "系统故障", 3: "其他原因"}.get(reason_val, "未知")
        reason_field.value = f"{reason_digits}（{reason_text}）"
    fields.append(reason_field)
    fields.append(reader.read_enum("控制命令", 1, {0: "停止充电", 1: "启动充电", 2: "定时充电启动", 3: "预约充电启动"}))
    fields.append(reader.read_cp56time2a("控制时间"))
    fields.append(reader.read_bcd("订单号", 16))
    return fields, reader.issues
