"""B47～B52 分时服务费计费模型解析。"""

from .common import register_parser
from .models import ParseField, ParseIssue
from .reader import ByteReader


RATE_TYPES = {1: "尖", 2: "峰", 3: "平", 4: "谷", 5: "深谷", 6: "尖扩展", 7: "峰扩展", 8: "平扩展"}


def _base(reader: ByteReader) -> list[ParseField]:
    """@brief 解析计费帧共有的桩号和接口号。"""
    fields = [reader.read_bcd("充电桩编号", 8), reader.read_uint("充电接口标识", 1)]
    return fields


def _periods(reader: ByteReader, count: int) -> ParseField:
    """@brief 解析计费时段数组。"""
    start = reader.base_offset + reader.position
    children: list[ParseField] = []
    for index in range(min(count, 12)):
        item_start = reader.base_offset + reader.position
        item_fields = [
            reader.read_uint("时段序号", 1),
            reader.read_enum("时段类型", 1, RATE_TYPES),
            reader.read_bcd("开始时间(HHmm)", 2, reverse=False),
            reader.read_bcd("结束时间(HHmm)", 2, reverse=False),
            reader.read_uint("电费", 4, 0.00001, "元/kWh"),
            reader.read_uint("服务费", 4, 0.00001, "元/kWh"),
        ]
        raw_length = reader.base_offset + reader.position - item_start
        children.append(ParseField(f"时段 {index + 1}", item_start, raw_length, "", "", children=item_fields))
    result = ParseField("时段列表", start, reader.base_offset + reader.position - start, "", "", children=children)
    return result


def _simple_model_result(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B48/B50 的模型编号和结果。"""
    reader = ByteReader(data, offset)
    fields = _base(reader)
    fields.extend([reader.read_bcd("计费模型ID", 8), reader.read_enum("成功标识", 1, {0: "成功", 1: "失败"})])
    return fields, reader.issues


@register_parser(0x85, 0x06, 0x40, "B47 分时服务费计费模型下发", "平台→桩")
def parse_b47(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B47 计费模型。"""
    reader = ByteReader(data, offset)
    fields = _base(reader)
    fields.extend([
        reader.read_bcd("计费模型ID", 8),
        reader.read_cp56time2a("切换时间"),
        reader.read_cp56time2a("失效时间"),
        reader.read_bcd("执行状态", 2, reverse=False),
    ])
    count_field = reader.read_uint("时段数量", 1)
    fields.append(count_field)
    fields.append(_periods(reader, int(float(count_field.value))))
    return fields, reader.issues


@register_parser(0x82, 0x06, 0x24, "B48 计费模型下发结果", "桩→平台")
def parse_b48(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B48。"""
    return _simple_model_result(data, offset)


@register_parser(0x82, 0x07, 0x25, "B49 计费模型切换生效上报", "桩→平台")
def parse_b49(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B49。"""
    reader = ByteReader(data, offset)
    fields = _base(reader)
    fields.extend([reader.read_bcd("计费模型ID", 8), reader.read_cp56time2a("切换时间"),
                   reader.read_enum("切换结果", 1, {0: "成功", 1: "失败"})])
    return fields, reader.issues


@register_parser(0x85, 0x06, 0x41, "B50 计费模型切换确认", "平台→桩")
def parse_b50(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B50。"""
    return _simple_model_result(data, offset)


@register_parser(0x85, 0x06, 0x42, "B51 计费模型召测", "平台→桩")
def parse_b51(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B51。"""
    reader = ByteReader(data, offset)
    fields = _base(reader)
    fields.append(reader.read_cp56time2a("时间戳序号"))
    return fields, reader.issues


@register_parser(0x82, 0x07, 0x26, "B52 计费模型召测结果", "桩→平台")
def parse_b52(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B52。"""
    reader = ByteReader(data, offset)
    fields = _base(reader)
    fields.extend([reader.read_cp56time2a("时间戳序号"), reader.read_bcd("计费模型ID", 8),
                   reader.read_cp56time2a("切换时间"), reader.read_cp56time2a("失效时间")])
    count = reader.read_uint("时段数量", 1)
    fields.extend([count, _periods(reader, int(float(count.value)))])
    fields.append(reader.read_enum("成功标识", 1, {0: "成功", 1: "失败"}))
    return fields, reader.issues

