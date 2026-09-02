# AHTT Bore 模拟平台联调手册

> 状态：导航页与历史验证摘要。当前可执行运行手册以 `ahtt-bore-sim-validation` Skill 为准。

## 当前操作入口

执行 Bore、串口或测试端点操作前，阅读 [AHTT-Bore模拟联调运行手册](../../.agents/skills/ahtt-bore-sim-validation/references/AHTT-Bore模拟联调运行手册.md)，并遵守 `AGENTS.md`、`docs/ai/AHTT_AI_Development_Prompts.md` 的当前角色和设备授权 Gate。

COM 口、串口参数、Bore 端口、测试域名、恢复地址和 timeout 必须由当前代码、`--help`、设备配置与本次用户确认动态取得；历史值不是当前默认值。

## 历史验证摘要

- `[OBSERVED]` 已观察到独立流水号签到和 `0x81` 心跳静默后的 Offline。
- `[OBSERVED]` 已观察到 TCP backoff、Bore 恢复后的重新签到/心跳。
- `[OBSERVED]` 2026-09-02 已观察到 `0x04` 候选端点切换、候选签到、重启保持候选端点及测试结束恢复原确认端点。

这些历史证据不代表 DNS、端口拒绝、NVM 写失败、掉电、20ms 实时性、看门狗或真实生产平台已完成验证。
