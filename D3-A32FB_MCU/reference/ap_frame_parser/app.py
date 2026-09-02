"""安培 AP V1.4.1 报文解析工具入口。"""

import pathlib
import sys
import tkinter as tk


TOOL_ROOT = pathlib.Path(__file__).resolve().parent
sys.path.insert(0, str(TOOL_ROOT))

from ui.main_window import MainWindow


def main() -> None:
    """@brief 创建并运行桌面主窗口。"""
    root = tk.Tk()
    root.title("安培 AP V1.4.1 报文解析工具")
    root.geometry("1280x820")
    root.minsize(980, 650)
    MainWindow(root)
    root.mainloop()


if __name__ == "__main__":
    main()
