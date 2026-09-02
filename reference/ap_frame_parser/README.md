# 安培 AP V1.4.1 报文解析工具

Windows 桌面工具，用于解析实时数据、计费模型、订单交易和基础链路长报文。

## 支持范围

- B1 实时监测数据
- B6/B7 刷卡鉴权
- B10/B11 远程启动通知
- B13/B14 在线交易与扣款结果
- B23/B24 远程升级参数与启动结果
- B31 SIM 卡信息上报
- B33/B34 充电功率控制与结果
- B57 功率控制实时状态
- B39/B40 FTP 服务器参数与接收结果
- B45/B46 充电功率召测与结果
- B4 充电启停控制下发
- B5 充电启停控制结果
- B47～B52 分时服务费计费模型
- B53/B54 在线分时交易明细
- F1～F8 登录、认证、心跳和校时
- 未知帧公共字段及原始业务数据

首版不解析 B15/B16，不提供报文组包、网络收发或日志文件导入。

## 使用

```powershell
C:\Users\Administrator\AppData\Local\Programs\Python\Python312\python.exe .\tools\ap_frame_parser\app.py
```

粘贴带空格、换行或日志前缀的十六进制报文，点击“解析”。左侧选择报文，右侧查看字段；选择字段会高亮对应字节。

## 测试

```powershell
C:\Users\Administrator\AppData\Local\Programs\Python\Python312\python.exe -m unittest discover -s tools\ap_frame_parser\tests -p "test_*.py" -v
```

## 打包 EXE

```powershell
C:\Users\Administrator\AppData\Local\Programs\Python\Python312\python.exe -m pip install -r .\tools\ap_frame_parser\requirements-build.txt
powershell -ExecutionPolicy Bypass -File .\tools\ap_frame_parser\build_exe.ps1
```

输出文件位于 `tools\ap_frame_parser\dist\AP_Frame_Parser.exe`。

## 协议核对说明

字段顺序和倍率对照 AP V1.4.1 PDF、仓库示例报文、当前 `Protocol_AP` 组包代码和旧工程实现。当前测试覆盖 B1、B6、B7、B10、B11、B14、B48、B49、B54 及异常输入；B47、B52、B53 的可变数组解析已实现，仍建议继续用平台真实报文补充回归样例。
