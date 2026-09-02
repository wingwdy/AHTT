---
name: ahtt-development
description: Use when 在 D3-A32FB_MCU 中分析、设计、实现、调试或评审安徽铁塔 AHTT V3.12 平台协议、报文、状态机、充电业务、运维、告警、持久化及板端联调。
---

# AHTT 开发

## 1. Skill 定位

本 Skill 只维护 **AHTT 领域知识、事实来源、架构导航和任务分析方法**。

以下内容不在本 Skill 重复定义：

- C 编码规范。
- Git 安全规则。
- 代码修改前置确认。
- Planner / Executor / Reviewer 的角色门禁。
- 通用调试、测试驱动、代码评审方法论。

上述规则分别由：

```text
AGENTS.md
docs/ai/AHTT_AI_Development_Prompts.md
全局 Skills
```

负责。

执行 AHTT 任务时必须先遵守仓库根目录 `AGENTS.md`。

---

## 2. 何时使用

以下任务优先加载本 Skill：

- AHTT V3.12 协议分析。
- AHTT 命令帧新增、修改或排障。
- `Protocol_AHTT` Send / Recv / M 层开发。
- 登录、心跳、离线、重连和业务状态机分析。
- 平台命令与充电业务接口适配。
- AHTT 私有参数和 NVM 设计。
- AHTT 与 `Asw_PlatM`、`FrameQueue`、`Cdd_NetM` 等模块的集成。
- AHTT 协议代码 Review。
- 报文、流水号、CRC、端序、幂等、超时和重试问题。
- 板端联调前的 AHTT 代码与协议核对。

如果任务属于 Bore / TCP 模拟器 / 4G 桩联调，再加载：

```text
ahtt-bore-sim-validation
```

---

## 3. 开始任务时的读取顺序

### 3.1 先读取规则和角色定义

首先读取：

```text
AGENTS.md
```

如果当前任务明确处于 Planner / Executor / Reviewer 工作流，再读取：

```text
docs/ai/AHTT_AI_Development_Prompts.md
```

并只执行当前角色允许的动作。

### 3.2 再读取当前 AHTT 权威文档

从 `docs/ahtt/` 发现当前真实存在的相关文档。

当前核心文档如存在，应优先查看：

```text
AHTT项目开发总纲.md
AHTT-软件架构设计.md
AHTT-V3.12-事实台账.md
AHTT-V3.12-命令追踪矩阵.md
```

按任务需要继续读取：

```text
AHTT-M*.md
AHTT-报文测试向量.md
AHTT-板端验证清单.md
AHTT-联调问题记录.md
```

不得因为本 Skill 列出了历史文件名就假设其永久存在；新增的当前权威文档同样必须纳入。

### 3.3 协议任务必须定位原始协议

涉及协议字段、命令、方向、长度、字节序、单位、取值范围、应答语义或超时时：

优先定位：

```text
reference/安徽省铁塔充电平台设备交互协议V3.12（新协议）.docx
```

读取对应：

- 章节。
- 表格。
- 字段定义。
- 示例。
- 修订说明。
- 专节约束。

页码只有在实际渲染核对后才能引用。

### 3.4 最后读取真实目标代码

必须读取当前真实目标代码及最小调用闭环。

不得仅依据：

- 历史聊天。
- 旧 Plan。
- 旧文档摘要。
- Skill 中的历史基线。

推断当前实现。

---

## 4. 事实来源优先级

AHTT 任务按以下顺序判断事实：

```text
1. 用户确认的当前产品需求
2. AHTT V3.12 原始协议
3. 当前硬件 / 配置 / 代码 / build / test / 抓包 / 板端证据
4. 当前 docs/ahtt 权威文档
5. 当前项目 GN 对照实现
6. reference/参考项目中的旧实现
7. 工程经验
```

冲突时：

```text
[CONFLICT]
```

缺少产品决策时：

```text
[TBD]
```

仍需真实验证时：

```text
[TBC]
```

不得静默使用低优先级来源覆盖高优先级来源。

---

## 5. AHTT 架构导航

当前 AHTT 运行时代码的主要职责通常分布在：

```text
Asw_IotProtoAHTTTypes.h
Asw_IotProtoAHTTSend.c/.h
Asw_IotProtoAHTTRecv.c/.h
Asw_IotProtoAHTTM.c/.h
```

职责核对基线：

| 区域                        | 主要职责                                 |
| --------------------------- | ---------------------------------------- |
| `Asw_IotProtoAHTTTypes.h`   | 命令、字段、Context、控制表、协议类型    |
| `Asw_IotProtoAHTTSend.c/.h` | payload 组包、发送调度、发送控制         |
| `Asw_IotProtoAHTTRecv.c/.h` | 帧定位、合法性检查、解析、应答匹配、超时 |
| `Asw_IotProtoAHTTM.c/.h`    | 平台状态机、周期业务、业务接口适配       |

常见集成点还包括：

```text
Asw_PlatM
FrameQueue
Cdd_NetM
MS_Nvm
充电业务模块
SysCfg / 平台配置
02_App/Prj/Project.uvprojx
```

以上只是导航基线。

真正 owner、接口和调用路径必须以当前代码为准。

---

## 6. 最小调用闭环

分析一个 AHTT 功能时，不要只看单个函数。

### 6.1 平台下行

至少追踪：

```text
TCP / NetM
→ FrameQueue
→ frame validation
→ command dispatch
→ parser
→ M layer / business adapter
→ business / NVM
→ response scheduling
```

### 6.2 主动上报

至少追踪：

```text
M / state / business trigger
→ SendCtrl
→ payload pack
→ frame pack
→ FrameQueue
→ NetM
```

### 6.3 请求 / 应答事务

如命令存在等待态，还要追踪：

```text
request start
→ sequence / transaction context
→ waiting
→ matching response
→ success / failure / timeout
→ cleanup
```

---

## 7. GN → AHTT 对照

GN 的使用规则以根目录 `AGENTS.md` 为准。

本 Skill 只补充 AHTT 领域上的执行方法。

当当前任务存在合理 GN 同职责实现时：

1. 找到 GN 对应 symbol / path。
2. 找到 AHTT 目标 symbol / path。
3. 比较职责、调用层级、控制流和状态。
4. 明确哪些应保持同构。
5. 明确哪些因 AHTT 协议语义必须不同。
6. 将差异写入 Plan 或 Review 证据。

详细对照方法见：

```text
references/GN同构与校验边界.md
```

如果任务本身没有合理 GN 对应实现：

```text
GN Mapping: N/A
```

不得为了模板完整而制造虚假对应关系。

---

## 8. 状态与数据 owner 分析

任何涉及新状态、缓存、flag、Context 字段、transaction 状态或持久化数据的任务，都必须先回答：

```text
谁产生这个状态？
谁拥有这个状态？
谁更新？
谁消费？
生命周期何时开始？
何时清理？
是否需要持久化？
是否已经存在同一事实的 owner？
```

重点核对：

- AHTT runtime Context。
- 网络状态 owner。
- 充电业务 owner。
- NVM owner。
- FrameQueue。
- command / transaction runtime。

不得仅因为历史 Skill 中曾记录某个 owner，就跳过当前代码确认。

---

## 9. AHTT 命令分析模板

涉及具体协议命令时，至少建立：

```text
Command:
Direction:
Request / Response:
Parameter layout:
Parameter length:
Endian:
Unit:
Range:
Sequence semantics:
Transaction / order id:
Retry:
Timeout:
Idempotency:
Business precondition:
NVM impact:
Failure response:
```

如协议存在歧义：

```text
[CONFLICT]
```

如果产品仍需决策：

```text
[TBD]
```

不要用旧项目行为自动补齐协议缺口。

---

## 10. 设备级与端口级语义

分析命令时必须确认：

- 该命令是设备级还是端口级。
- 协议中是否真实存在端口字段。
- 内部控制表槽位是否只是实现细节。
- 旧项目中的固定枪口假设是否仍成立。

如果设备级命令没有端口字段：

- 不得因为旧项目曾固定使用某个枪口而把它当成协议字段。
- 内部索引和协议语义必须分开描述。

---

## 11. 流水号与事务关系

AHTT 下行、上行和应答逻辑必须区分：

```text
设备流水号
平台流水号
请求流水号
应答流水号
transaction / order id
```

分析一个命令时必须回答：

- 流水号由谁生成。
- 应答应复制哪一侧的流水号。
- 等待项按什么条件匹配。
- 重复报文是否幂等。
- 超时后旧应答如何处理。
- 断链 / 重连后等待态如何清理。

不得只按“数值相同”判断语义相同。

---

## 12. 旧实现的使用方式

`reference/参考项目` 仅提供历史经验。

读取旧实现时按最小调用闭环定位：

- 命令常量。
- 组包。
- 解析。
- 收发控制表。
- 成功回调。
- 超时。
- 状态入口。
- 业务接口。
- NVM 依赖。

旧实现可以帮助回答：

```text
过去怎么做？
```

不能自动回答：

```text
当前 AHTT 必须怎么做？
```

旧实现和当前协议 / 当前架构冲突时，不得直接移植。

---

## 13. 与全局 Skills 的协作

AHTT 领域上下文由本 Skill 提供。

按任务可组合全局 Skills：

| 场景                  | 建议的全局 Skill                             |
| --------------------- | -------------------------------------------- |
| Bug / 异常 / 测试失败 | `systematic-debugging`                       |
| RTOS 调用链与任务关系 | `rtos-code-navigation`                       |
| 板级配置              | `rtos-board-config-analysis`                 |
| Bootloader / OTA      | `mcu-bootloader-code-navigation`             |
| 新功能实施            | `test-driven-development`                    |
| 完成前证据核对        | `verification-before-completion`             |
| 高风险需求尚未收敛    | `grill-me-codex`                             |
| 通用变更评审          | 通用 review Skill，作为 AHTT Reviewer 的辅助 |

如果当前任务使用 AHTT Planner / Executor / Reviewer：

```text
docs/ai/AHTT_AI_Development_Prompts.md
```

的角色边界和输出契约优先。

通用 `writing-plans`、`requesting-code-review` 等 Skill 不得覆盖 AHTT 专用角色 SOP。

---

## 14. Bore 联调路由

出现以下任务时加载：

```text
ahtt-bore-sim-validation
```

包括：

- AHTT 命令帧主机验证完成后进入真实 4G 桩联调。
- TCP 模拟平台验证。
- Bore 内网穿透。
- 心跳静默。
- Offline / reconnect。
- TCP backoff。
- 临时测试端点配置。
- 串口配置和测试后恢复。

本 Skill 不重复 Bore 运行命令和串口操作细节。

这些细节由 Bore Skill 及其运行手册维护。

---

## 15. 文档回写判断

AHTT 功能实现或验证完成后，需要判断当前任务是否影响：

- 命令追踪矩阵。
- 事实台账。
- 测试向量。
- 板端验证清单。
- 联调问题记录。
- 当前阶段 M* 文档。
- 软件架构设计。

只更新与当前任务真实变化有关的文档。

如果代码、测试和文档状态不一致，应明确记录 documentation drift，不得假装一致。

---

## 16. 常见错误

重点防止：

- 把旧项目行为当成 AHTT V3.12 当前需求。
- 只读一个函数，不追调用闭环。
- 只实现 happy path。
- 把设备级命令误当端口级命令。
- 混用设备流水号和平台流水号。
- 新增第二份业务、网络或持久化状态。
- 为了“更安全”无证据改变 GN 同构控制流。
- 只创建协议文件，遗漏 PlatM / NVM / 工程集成点。
- 修改持久化结构但不检查兼容和恢复路径。
- Host test 通过后直接宣称板端或真实平台完成。
- 使用历史 Skill / Plan 中的架构基线覆盖当前代码。
