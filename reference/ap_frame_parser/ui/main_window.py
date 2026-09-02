"""安培 AP 报文解析工具主窗口。"""

import tkinter as tk
from tkinter import messagebox, ttk

from parser.common import parse_frame
from parser.frame_splitter import split_frames
from parser.models import ParseField, ParsedFrame


def byte_range_to_text_indices(offset: int, length: int, bytes_per_line: int = 16) -> tuple[str, str]:
    """@brief 将字节范围转换为 Tk Text 行列索引。"""
    start_line = offset // bytes_per_line + 1
    start_column = (offset % bytes_per_line) * 3
    end_offset = offset + max(length - 1, 0)
    end_line = end_offset // bytes_per_line + 1
    end_column = (end_offset % bytes_per_line) * 3 + (2 if length else 0)
    return f"{start_line}.{start_column}", f"{end_line}.{end_column}"


class MainWindow(ttk.Frame):
    """桌面解析器主界面。"""

    def __init__(self, master: tk.Tk):
        """@brief 创建控件并绑定交互。"""
        super().__init__(master, padding=8)
        self.pack(fill=tk.BOTH, expand=True)
        self.frames: list[ParsedFrame] = []
        self.tree_objects: dict[str, ParseField] = {}
        self._build_widgets()

    def _build_widgets(self) -> None:
        """@brief 构建输入区、报文列表、字段树和问题区。"""
        toolbar = ttk.Frame(self)
        toolbar.pack(fill=tk.X)
        ttk.Button(toolbar, text="粘贴并解析", command=self.paste_and_parse).pack(side=tk.LEFT)
        ttk.Button(toolbar, text="解析", command=self.parse_input).pack(side=tk.LEFT, padx=6)
        ttk.Button(toolbar, text="清空", command=self.clear_all).pack(side=tk.LEFT)

        self.input_text = tk.Text(self, height=7, wrap=tk.WORD)
        self.input_text.pack(fill=tk.X, pady=(6, 8))
        body = ttk.PanedWindow(self, orient=tk.HORIZONTAL)
        body.pack(fill=tk.BOTH, expand=True)
        self.frame_tree = ttk.Treeview(body, columns=("direction", "type", "length", "status"), show="headings")
        for key, title, width in (("direction", "方向", 80), ("type", "报文", 210),
                                  ("length", "长度", 60), ("status", "状态", 60)):
            self.frame_tree.heading(key, text=title)
            self.frame_tree.column(key, width=width, anchor=tk.CENTER)
        body.add(self.frame_tree, weight=1)

        right = ttk.PanedWindow(body, orient=tk.VERTICAL)
        body.add(right, weight=3)
        columns = ("offset", "length", "raw", "value", "note")
        self.field_tree = ttk.Treeview(right, columns=columns, show="tree headings")
        self.field_tree.heading("#0", text="字段名称")
        for key, title, width in (("offset", "偏移", 55), ("length", "长度", 55), ("raw", "原始 HEX", 220),
                                  ("value", "解析值", 190), ("note", "单位/说明", 130)):
            self.field_tree.heading(key, text=title)
            self.field_tree.column(key, width=width)
        right.add(self.field_tree, weight=3)

        lower = ttk.PanedWindow(right, orient=tk.VERTICAL)
        right.add(lower, weight=2)
        self.raw_text = tk.Text(lower, height=8, wrap=tk.NONE, font=("Consolas", 10))
        self.raw_text.tag_configure("selected_byte", background="#FFD966")
        lower.add(self.raw_text, weight=2)
        self.issue_tree = ttk.Treeview(lower, columns=("level", "message"), show="headings", height=5)
        self.issue_tree.heading("level", text="级别")
        self.issue_tree.heading("message", text="问题")
        self.issue_tree.column("level", width=60)
        self.issue_tree.column("message", width=600)
        lower.add(self.issue_tree, weight=1)
        self.status = ttk.Label(self, text="请粘贴 AP 报文")
        self.status.pack(fill=tk.X, pady=(5, 0))
        self.frame_tree.bind("<<TreeviewSelect>>", self._on_frame_selected)
        self.field_tree.bind("<<TreeviewSelect>>", self._on_field_selected)

    def paste_and_parse(self) -> None:
        """@brief 用剪贴板内容替换输入区并立即解析。"""
        try:
            text = self.clipboard_get()
            self.input_text.delete("1.0", tk.END)
            self.input_text.insert("1.0", text)
            self.parse_input()
        except tk.TclError:
            messagebox.showwarning("剪贴板", "剪贴板中没有可读取的文本")

    def clear_all(self) -> None:
        """@brief 清空输入和所有解析结果。"""
        self.input_text.delete("1.0", tk.END)
        self.frames.clear()
        for tree in (self.frame_tree, self.field_tree, self.issue_tree):
            tree.delete(*tree.get_children())
        self.raw_text.delete("1.0", tk.END)
        self.status.config(text="已清空")

    def parse_input(self) -> None:
        """@brief 拆分并解析输入区中的全部报文。"""
        try:
            split_result = split_frames(self.input_text.get("1.0", tk.END))
            self.frames = [parse_frame(raw) for raw in split_result.frames]
            self.frame_tree.delete(*self.frame_tree.get_children())
            for index, frame in enumerate(self.frames):
                self.frame_tree.insert("", tk.END, iid=str(index),
                                       values=(frame.direction, frame.name, len(frame.raw), frame.status_text))
            self.status.config(text=f"解析完成：{len(self.frames)} 条报文，输入警告 {len(split_result.issues)} 条")
            if self.frames:
                self.frame_tree.selection_set("0")
                self._show_frame(self.frames[0])
        except Exception as error:
            messagebox.showerror("解析失败", str(error))

    def _insert_field(self, parent: str, field: ParseField) -> None:
        """@brief 递归插入字段树并保存对象映射。"""
        item = self.field_tree.insert(parent, tk.END, text=field.name,
                                      values=(field.offset, field.length, field.raw_hex, field.value, field.note))
        self.tree_objects[item] = field
        for child in field.children:
            self._insert_field(item, child)

    def _show_frame(self, frame: ParsedFrame) -> None:
        """@brief 展示当前报文的字段、原文和问题。"""
        self.field_tree.delete(*self.field_tree.get_children())
        self.issue_tree.delete(*self.issue_tree.get_children())
        self.tree_objects.clear()
        for field in frame.fields:
            self._insert_field("", field)
        lines = [" ".join(f"{byte:02X}" for byte in frame.raw[index:index + 16])
                 for index in range(0, len(frame.raw), 16)]
        self.raw_text.delete("1.0", tk.END)
        self.raw_text.insert("1.0", "\n".join(lines))
        for issue in frame.issues:
            self.issue_tree.insert("", tk.END, values=(issue.level.value, issue.message))

    def _on_frame_selected(self, _event=None) -> None:
        """@brief 响应报文列表选择。"""
        selection = self.frame_tree.selection()
        if selection:
            self._show_frame(self.frames[int(selection[0])])

    def _on_field_selected(self, _event=None) -> None:
        """@brief 高亮所选字段对应的原始字节。"""
        selection = self.field_tree.selection()
        self.raw_text.tag_remove("selected_byte", "1.0", tk.END)
        if selection and selection[0] in self.tree_objects:
            field = self.tree_objects[selection[0]]
            start, end = byte_range_to_text_indices(field.offset, field.length)
            self.raw_text.tag_add("selected_byte", start, end)
            self.raw_text.see(start)

