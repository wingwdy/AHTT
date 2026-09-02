"""B1 实时监测数据解析。"""

from .common import register_parser
from .models import ParseField, ParseIssue
from .reader import ByteReader


@register_parser(0x86, 0x06, None, "B1 充电过程实时监测数据", "桩→平台")
def parse_b1(data: bytes, offset: int) -> tuple[list[ParseField], list[ParseIssue]]:
    """@brief 按当前固件 B1 组包顺序解析实时数据。"""
    reader = ByteReader(data, offset)
    fields = [reader.read_bcd("充电桩编号", 8), reader.read_uint("充电接口标识", 1),
              reader.read_enum("连接确认开关状态", 1, {0: "未插枪", 1: "已插枪"}),
              reader.read_bcd("工作状态", 2, reverse=False),
              reader.read_bytes("基础告警及三相输入数据", 21),
              reader.read_uint("输出电压", 2, 0.1, "V"), reader.read_uint("输出电流", 2, 0.01, "A"),
              reader.read_enum("接触器状态", 1, {0: "断开", 1: "闭合"}),
              reader.read_uint("BMS通信状态", 1), reader.read_uint("电池连接状态", 1),
              reader.read_uint("单体最高电压", 2, 0.001, "V"), reader.read_uint("单体最低电压", 2, 0.001, "V"),
              reader.read_uint("总电量", 4, 0.001, "kWh")]
    for name in ("尖电量", "峰电量", "平电量", "谷电量"):
        fields.append(reader.read_uint(name, 4, 0.001, "kWh"))
    fields.extend([reader.read_uint("SOC", 2, 1, "%"), reader.read_uint("累计充电时间", 2, 1, "min"),
                   reader.read_ascii("电动汽车唯一标识", 32), reader.read_bytes("BMS状态", 3),
                   reader.read_bytes("设备告警", 18), reader.read_uint("充电费", 4, 0.01, "元"),
                   reader.read_uint("服务费", 4, 0.01, "元"), reader.read_uint("剩余时长", 4, 1, "min"),
                   reader.read_bcd("订单号", 16), reader.read_bytes("故障位", 8)])
    return fields, reader.issues
