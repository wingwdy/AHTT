"""AP 公共帧解析与业务分派。"""

from typing import Callable, Optional

from .models import IssueLevel, ParseField, ParseIssue, ParsedFrame


BusinessParser = Callable[[bytes, int], tuple[list[ParseField], list[ParseIssue]]]
PARSER_REGISTRY: dict[tuple[int, int, Optional[int]], tuple[str, str, BusinessParser]] = {}
_PARSERS_LOADED = False


def register_parser(type_id: int, cot: int, record_kind: Optional[int], name: str, direction: str):
    """@brief 注册业务解析函数。"""
    def decorator(func: BusinessParser) -> BusinessParser:
        PARSER_REGISTRY[(type_id, cot, record_kind)] = (name, direction, func)
        return func
    return decorator


def _raw_field(name: str, raw: bytes, offset: int) -> ParseField:
    """@brief 创建原始字节字段。"""
    return ParseField(name, offset, len(raw), raw.hex(" ").upper(), raw.hex(" ").upper())


def _ensure_business_parsers() -> None:
    """@brief 延迟加载业务模块，避免公共注册器循环导入。"""
    global _PARSERS_LOADED
    if not _PARSERS_LOADED:
        from . import auth, billing, extended, realtime, transaction  # noqa: F401
        _PARSERS_LOADED = True


def parse_frame(raw: bytes) -> ParsedFrame:
    """@brief 解析公共帧头并分派业务数据。"""
    _ensure_business_parsers()
    fields: list[ParseField] = []
    issues: list[ParseIssue] = []
    name = "未知帧"
    direction = "未知"
    type_id = None
    cot = None
    record_kind = None
    if raw:
        fields.append(_raw_field("帧头", raw[0:1], 0))
    if len(raw) >= 2:
        fields.append(ParseField("长度", 1, 1, f"{raw[1]:02X}", str(raw[1]), "字节"))
        expected = raw[1] + 2
        if expected != len(raw):
            issues.append(ParseIssue(IssueLevel.ERROR,
                                     f"声明长度 {expected} 与实际长度 {len(raw)} 不一致", 1, 1))
    if len(raw) >= 6 and not (len(raw) == 12 and raw[1] in (0x00, 0x01)):
        fields.append(_raw_field("控制域", raw[2:6], 2))
    if len(raw) == 12 and raw[1] in (0x00, 0x01):
        name = "F1/F2 登录验证帧"
        fields.extend([_raw_field("登录标识", raw[1:2], 1),
                       ParseField("充电桩编号", 2, 8, raw[2:10].hex(" ").upper(),
                                  raw[2:10][::-1].hex().upper()),
                       _raw_field("登录状态/保留", raw[10:12], 10)])
    elif len(raw) == 6:
        control_names = {
            0x07: ("F3 U 帧认证请求", "桩→平台"),
            0x0B: ("F4 U 帧认证应答", "平台→桩"),
            0x43: ("F5 心跳请求", "桩→平台"),
            0x83: ("F6 心跳应答", "平台→桩"),
        }
        name, direction = control_names.get(raw[2], (name, direction))
        if name == "未知帧":
            fields.append(_raw_field("未解析业务数据", raw[2:], 2))
            issues.append(ParseIssue(IssueLevel.WARNING, "无法识别短帧控制域", 2, 4))
    elif len(raw) >= 11:
        type_id = raw[6]
        cot = raw[8]
        is_realtime_frame = type_id == 0x86
        has_record_kind = type_id not in (0x86, 0x67)
        record_kind = raw[14] if has_record_kind and len(raw) >= 15 else None
        fields.extend([
            ParseField("TypeID", 6, 1, f"{type_id:02X}", f"0x{type_id:02X}"),
            ParseField("VSQ", 7, 1, f"{raw[7]:02X}", str(raw[7])),
            ParseField("COT", 8, 1, f"{cot:02X}", f"0x{cot:02X}"),
            _raw_field("应用服务地址", raw[9:11], 9),
        ])
        payload_offset = 11 if is_realtime_frame else 14
        if not is_realtime_frame:
            fields.append(_raw_field("信息对象地址", raw[11:14], 11))
        if record_kind is not None and len(raw) >= 15:
            fields.append(ParseField("记录类型", 14, 1, f"{record_kind:02X}", f"0x{record_kind:02X}"))
            payload_offset = 15
        parser_def = PARSER_REGISTRY.get((type_id, cot, record_kind))
        if type_id == 0x67 and cot in (0x06, 0x07):
            name = "F7 时钟同步请求" if cot == 0x06 else "F8 时钟同步应答"
            direction = "平台→桩" if cot == 0x06 else "桩→平台"
            parser_def = None
            if len(raw) >= payload_offset + 7:
                from .reader import ByteReader
                time_reader = ByteReader(raw[payload_offset:], payload_offset)
                fields.append(time_reader.read_cp56time2a("同步时间"))
                issues.extend(time_reader.issues)
                payload_offset = len(raw)
        if parser_def:
            name, direction, parser = parser_def
            business_fields, business_issues = parser(raw[payload_offset:], payload_offset)
            fields.extend(business_fields)
            issues.extend(business_issues)
        elif payload_offset < len(raw):
            fields.append(_raw_field("未解析业务数据", raw[payload_offset:], payload_offset))
            issues.append(ParseIssue(IssueLevel.WARNING, "业务数据暂未支持", payload_offset,
                                     len(raw) - payload_offset))
    elif raw:
        fields.append(_raw_field("未解析业务数据", raw[2:], 2))
        issues.append(ParseIssue(IssueLevel.WARNING, "无法识别帧结构", 0, len(raw)))
    return ParsedFrame(raw, name, direction, fields, issues, type_id, cot, record_kind)
