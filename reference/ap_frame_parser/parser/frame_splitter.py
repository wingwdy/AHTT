"""文本清洗与 AP 报文拆分。"""

import re

from .models import IssueLevel, ParseIssue, SplitResult


BYTE_SEQUENCE_RE = re.compile(r"(?i)(?:\b[0-9a-f]{2}\b[\s,:;\-\]>]*){2,}")
CONTINUOUS_RE = re.compile(r"(?i)(?<![0-9a-f])[0-9a-f]{4,}(?![0-9a-f])")


def _line_to_bytes(line: str) -> tuple[bytes, bool]:
    """@brief 从单行日志中提取最长的十六进制字节序列。
    @param[in] line 日志行。
    @retval tuple 提取的字节及奇数长度标志。
    """
    candidates: list[str] = []
    odd = False
    for match in BYTE_SEQUENCE_RE.finditer(line):
        candidates.append("".join(re.findall(r"(?i)\b[0-9a-f]{2}\b", match.group())))
    if not candidates:
        candidates.extend(match.group() for match in CONTINUOUS_RE.finditer(line))
    clean = max(candidates, key=len) if candidates else ""
    if len(clean) % 2:
        odd = True
        clean = clean[:-1]
    data = bytes.fromhex(clean) if clean else b""
    return data, odd


def split_frames(text: str) -> SplitResult:
    """@brief 清洗粘贴文本并按 AP 长度字段拆分多条报文。
    @note Step1: 各行独立提取，避免把时间戳拼入报文。
          Step2: 搜索 0x68 并按 2+长度字段截帧。
          Step3: 保留半包和残留问题，不影响其他完整帧。
    @param[in] text 用户粘贴的日志或十六进制文本。
    @retval SplitResult 完整帧和清洗问题。
    """
    stream = bytearray()
    issues: list[ParseIssue] = []
    for line_number, line in enumerate(text.splitlines() or [text], start=1):
        data, odd = _line_to_bytes(line)
        stream.extend(data)
        if odd:
            issues.append(ParseIssue(IssueLevel.WARNING, f"第 {line_number} 行存在奇数个十六进制字符"))

    frames: list[bytes] = []
    position = 0
    while position < len(stream):
        marker = stream.find(0x68, position)
        if marker < 0:
            if position < len(stream):
                issues.append(ParseIssue(IssueLevel.WARNING, "存在无法识别的非 AP 数据"))
            position = len(stream)
        elif marker + 2 > len(stream):
            issues.append(ParseIssue(IssueLevel.WARNING, "发现不完整 AP 帧头"))
            position = len(stream)
        else:
            if marker > position:
                issues.append(ParseIssue(IssueLevel.WARNING, "帧头前存在无法识别的数据"))
            frame_length = 12 if stream[marker + 1] in (0x00, 0x01) else 2 + stream[marker + 1]
            if marker + frame_length <= len(stream):
                frames.append(bytes(stream[marker:marker + frame_length]))
                position = marker + frame_length
            else:
                issues.append(ParseIssue(IssueLevel.WARNING, f"发现不完整 AP 报文，需要 {frame_length} 字节"))
                position = len(stream)
    return SplitResult(frames, issues)
