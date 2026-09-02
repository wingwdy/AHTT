"""B13/B14、B53/B54 交易报文解析。"""

from .common import register_parser
from .models import ParseField, ParseIssue
from .reader import ByteReader


RATE_TYPES = {1: "尖", 2: "峰", 3: "平", 4: "谷", 5: "深谷", 6: "尖扩展", 7: "峰扩展", 8: "平扩展"}


def _identity(reader: ByteReader) -> list[ParseField]:
    """@brief 解析交易帧桩号和接口号。"""
    return [reader.read_bcd("充电桩编号", 8), reader.read_uint("充电接口标识", 1)]


@register_parser(0x82, 0x07, 0x02, "B13 在线交易确认", "平台→桩")
def parse_b13(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B13。"""
    reader = ByteReader(data, offset)
    fields = _identity(reader)
    fields.extend([reader.read_enum("成功标识", 1, {0: "成功", 1: "失败"}), reader.read_bcd("订单号", 16)])
    return fields, reader.issues


@register_parser(0x85, 0x06, 0x03, "B14 扣款结果", "平台→桩")
def parse_b14(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B14。"""
    reader = ByteReader(data, offset)
    fields = [reader.read_bcd("充电桩编号", 8), reader.read_bcd("物理卡号", 8),
              reader.read_uint("扣款金额", 4, 0.01, "元"), reader.read_uint("账户余额", 4, 0.01, "元"),
              reader.read_enum("扣款成功标志", 1, {0: "失败", 1: "成功"}), reader.read_bcd("失败原因", 2)]
    for name in ("扣除里程", "剩余里程", "扣除电量", "剩余电量", "扣除次数", "剩余次数"):
        fields.append(reader.read_uint(name, 4, 0.01))
    return fields, reader.issues


@register_parser(0x82, 0x06, 0x27, "B53 在线分时交易明细", "桩→平台")
def parse_b53(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B53 分时交易明细。"""
    reader = ByteReader(data, offset)
    fields = _identity(reader)
    fields.append(reader.read_bcd("订单号", 16))
    count_field = reader.read_uint("时段个数", 1)
    fields.append(count_field)
    item_children: list[ParseField] = []
    item_start_all = reader.base_offset + reader.position
    for index in range(min(int(float(count_field.value)), 12)):
        item_start = reader.base_offset + reader.position
        children = [reader.read_uint("时段序号", 1), reader.read_enum("时段类型", 1, RATE_TYPES),
                    reader.read_uint("时段电量", 3, 0.001, "kWh"),
                    reader.read_uint("时段电费", 3, 0.01, "元"),
                    reader.read_uint("时段服务费", 3, 0.01, "元")]
        item_children.append(ParseField(f"时段 {index + 1}", item_start,
                                        reader.base_offset + reader.position - item_start, "", "", children=children))
    fields.append(ParseField("时段明细", item_start_all, reader.base_offset + reader.position - item_start_all,
                             "", "", children=item_children))
    fields.extend([reader.read_cp56time2a("充电开始时间"), reader.read_cp56time2a("充电结束时间"),
                   reader.read_uint("累计充电时间", 2, 1, "min"), reader.read_uint("充电费", 3, 0.01, "元"),
                   reader.read_uint("服务费", 3, 0.01, "元"), reader.read_uint("总电量", 3, 0.001, "kWh"),
                   reader.read_uint("总起示值", 4, 0.001, "kWh"), reader.read_uint("总止示值", 4, 0.001, "kWh"),
                   reader.read_uint("充电前SOC", 2, 1, "%"), reader.read_uint("结束后SOC", 2, 1, "%"),
                   reader.read_bcd("物理卡号", 8), reader.read_ascii("电动汽车唯一标识", 32),
                   reader.read_bcd("停止充电原因", 2, reverse=False)])
    return fields, reader.issues


@register_parser(0x85, 0x06, 0x43, "B54 在线分时交易确认", "平台→桩")
def parse_b54(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 解析 B54。"""
    reader = ByteReader(data, offset)
    fields = _identity(reader)
    fields.extend([reader.read_enum("成功标识", 1, {0: "成功", 1: "失败"}), reader.read_bcd("订单号", 16)])
    return fields, reader.issues

