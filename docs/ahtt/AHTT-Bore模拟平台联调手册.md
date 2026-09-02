# AHTT Bore 模拟平台联调手册

> 状态：可复用操作手册。对应项目内 Skill：`ahtt-bore-sim-validation`。

## 目标

提供命令帧代码完成后的可重复闭环：主机脚本验证 → 本机AHTT模拟器 → Bore公网TCP入口 → 串口配置测试域名与重启 → 板端协议、超时、重连验证 → 恢复生产地址与证据回写。

## 已验证基线

- 模拟器：`tools/ahtt/ahtt_platform_sim.py`，监听`127.0.0.1:18888`，回复`0x01`、默认静默`0x81`。
- Bore：`bore local 18888 --local-host 127.0.0.1 --to bore.pub`；以`RUST_LOG=info`读取动态端口。
- 串口：测试桩当前使用`COM3`、`115200 8N1`、无流控；命令为`set para domain:bore.pub,<port>`后接`reboot`。
- 板端：已验证独立流水号签到、`0x81`静默下10秒×3与约30秒Offline、Bore停止后的5/10/30/30秒退避、模组恢复与Bore恢复后的重新签到/心跳。

## 强制边界

- Bore的随机端口只适合短时测试，不得作为生产平台入口。
- 不带`--reply-heartbeat`执行心跳超时测试。
- 必须由当前 Bore 实例日志获得端口；端口变更后必须重新配置桩并重启。
- 串口被占用时，不得抢占、终止用户工具或扫描其他端口。
- 测试结束必须恢复用户确认的生产端点。默认历史端点为`www.ahttcd.cn:8888`。

完整命令见 [Skill运行手册](../../.agents/skills/ahtt-bore-sim-validation/references/AHTT-Bore模拟联调运行手册.md)。
