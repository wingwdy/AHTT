# SDD ledger — plan: docs/superpowers/plans/2026-09-01-ahtt-independent-sequence-fix.md

## 预检

| 任务组合 | 共享文件或接口 | 产出与消费关系 | 结论 |
| --- | --- | --- | --- |
| Task 1 与 Task 2 | `IotAHTT_DecodeData()` 的 `responseMatched` | Task 1 的失败样例证明 Task 2 必须移除跨方向流水号比较 | 一致 |
| Task 2 与 Task 3 | `Common_SetRecvSeq()` / `Common_GetRecvSeq()` | Task 2 仅保存平台主动请求流水号；Task 3 不再保存设备主动请求流水号 | 一致 |
| Task 4 与 Task 5 | 两个 AHTT 收发文件 | Task 4 静态检查覆盖 Task 5 的构建前影响面 | 一致 |

| 单项任务 | 测试、修改与验证是否一致 | 结论 |
| --- | --- | --- |
| Task 1 | 用真实板端报文验证现有失败条件 | 一致 |
| Task 2 | 删除错误匹配，保留等待定时器与平台请求流水号保存 | 一致 |
| Task 3 | 删除设备请求流水号保存，保留平台请求应答读取 | 一致 |
| Task 4 | 搜索 AHTT 读写点和对照参考项目 | 一致 |
| Task 5 | 源级向量、Keil 编译和板端清单 | 一致 |

Ruling: 在当前 `develop` 主检出执行，不创建工作树 — 用户已确认当前方案，且 AHTT 协议目录为未提交文件，创建工作树会遗漏该目录 — 若判断错误，风险是污染当前工作区；本次仅修改计划指定的两处源文件。

Ruling: 不创建提交 — 用户未授权提交，且相关 AHTT 目录及工程已有用户未提交修改 — 若判断错误，风险是需要用户自行暂存或提交变更。

Task 1: complete (RED 检查已失败，原因符合预期)

Task 2 与 Task 3: complete（未提交；范围受限审查通过）

Task 4: complete（AHTT 应答确认路径不再读取接收控制块流水号；平台请求路径仅在接收侧写入 `recvSeq`；设备应答发送路径仍读取该值）

Task 5: complete（源级向量 `01 00 -> 87 04` 检查通过；Keil 目标 `D3_A32FB_GD32E503RE` 构建日志：0 Error(s), 0 Warning(s)）

Ruling: AHTT 目录在任务开始前即为未跟踪目录，无法制作只含本次两处源修改的 Git 审查包 — 采用任务简报、实施报告、目标函数片段和独立只读审查替代 — 若判断错误，风险是遗漏该目录中既有未跟踪内容带来的上下文差异。
