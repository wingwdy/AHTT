"""解析结果数据模型。"""

from dataclasses import dataclass, field
from enum import Enum
from typing import Optional


class IssueLevel(Enum):
    """解析问题级别。"""

    WARNING = "警告"
    ERROR = "错误"


@dataclass
class ParseIssue:
    """解析过程中发现的问题。"""

    level: IssueLevel
    message: str
    offset: Optional[int] = None
    length: Optional[int] = None


@dataclass
class ParseField:
    """一个可显示、可定位的协议字段。"""

    name: str
    offset: int
    length: int
    raw_hex: str
    value: str
    note: str = ""
    children: list["ParseField"] = field(default_factory=list)
    issues: list[ParseIssue] = field(default_factory=list)


@dataclass
class ParsedFrame:
    """一条 AP 报文的完整解析结果。"""

    raw: bytes
    name: str
    direction: str
    fields: list[ParseField]
    issues: list[ParseIssue]
    type_id: Optional[int] = None
    cot: Optional[int] = None
    record_kind: Optional[int] = None

    @property
    def status_text(self) -> str:
        """@brief 返回报文的最高问题级别。"""
        result = "正常"
        if any(issue.level is IssueLevel.ERROR for issue in self.issues):
            result = "错误"
        elif self.issues:
            result = "警告"
        return result


@dataclass
class SplitResult:
    """输入文本的拆帧结果。"""

    frames: list[bytes]
    issues: list[ParseIssue]

