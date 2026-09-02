# AHTT 公司仓库 AI 工作流更新修复实施文档

> 适用场景：  
> 公司仓库是实际开发主仓库；GitHub `wingwdy/AHTT` 是从公司仓库同步出的公开/镜像仓库。  
> 本文用于先在公司仓库完成 AI 工作流治理修复，再把相同版本化文件同步到 GitHub 仓库。

---

# 1. 修复目标

本轮修复只解决以下问题：

```text
Web ChatGPT
  ↓ Planner
批准的 Implementation Plan
  ↓ Handoff
Desktop Codex
  ↓ Executor
git diff + Validation
  ↓
Independent Reviewer
  ↓
Human Git Approval
```

确保公司仓库和 GitHub 镜像对以下内容保持一致：

```text
AGENTS.md
项目级 Skills
AI Development Prompts
AI Handoff 文档
AHTT 权威文档
Bore 联调规则
Simulator 文档
AI Workflow 自检脚本
.gitignore
```

本轮不修改：

```text
AHTT 固件业务行为
Protocol_AHTT C 代码
GN / OM / XDT 等其他协议代码
Bootloader
Flash / OTA / 安全逻辑
reference 原始协议资料
```

---

# 2. 仓库角色定义

以后建议固定：

```text
公司仓库
=
开发主仓库 / 实际编译 / 板端验证 / 正式提交来源

GitHub wingwdy/AHTT
=
Web ChatGPT 可读取的镜像 / Planner 信息源 / AI 工作流公开镜像
```

原则：

```text
公司仓库先改
→ 公司仓库验证
→ 公司仓库确认版本
→ 再同步 GitHub
```

不要反过来长期维护两套文件。

---

# 3. 本轮版本化文件清单

公司仓库中需要检查或更新以下文件。

## 3.1 项目规则

```text
AGENTS.md
```

## 3.2 项目 Skills

```text
.agents/skills/ahtt-development/SKILL.md
.agents/skills/ahtt-development/agents/openai.yaml
.agents/skills/ahtt-development/references/GN同构与校验边界.md
.agents/skills/ahtt-development/references/任务交付模板.md

.agents/skills/ahtt-bore-sim-validation/SKILL.md
.agents/skills/ahtt-bore-sim-validation/agents/openai.yaml
.agents/skills/ahtt-bore-sim-validation/references/AHTT-Bore模拟联调运行手册.md
```

## 3.3 AI Workflow

```text
docs/ai/AHTT_AI_Development_Prompts.md
docs/ai/README.md
docs/ai/plans/README.md
```

## 3.4 AHTT 当前文档

```text
docs/ahtt/AHTT项目开发总纲.md
docs/ahtt/AHTT-Bore模拟平台联调手册.md
docs/ahtt/AHTT-V3.12-事实台账.md
docs/ahtt/AHTT-V3.12-命令追踪矩阵.md
docs/ahtt/AHTT-板端验证清单.md
docs/ahtt/AHTT-M1基础框架实施计划.md
docs/ahtt/AHTT-M2签到与在线保持实施计划.md
docs/ahtt/AHTT-M3参数与NVM实施计划.md
```

## 3.5 工具与仓库治理

```text
README.md
.gitignore
tools/ahtt/README-ahtt-platform-sim.md
tools/ahtt/Validate-AHTTAIWorkflow.ps1
docs/superpowers/README.md
```

---

# 4. `AGENTS.md` 修复

## 4.1 修复 Markdown 结构

确认文件开头必须是：

```markdown
# AHTT 项目 AGENTS.md
```

不能是：

```markdown
- # AHTT 项目 AGENTS.md
```

所有一级、二级标题必须是正常 Markdown heading，不要整篇缩进在一个列表项中。

---

## 4.2 增加 AI Workflow 入口

在 AHTT AI 工作流章节明确：

```markdown
完整角色定义位于：

`docs/ai/AHTT_AI_Development_Prompts.md`

当用户明确要求进入 Planner、Executor 或 Reviewer 阶段时，
必须读取上述文件中的对应角色规则，并且一次只执行一个角色。
```

---

## 4.3 增加 Web / Cloud 可移植性说明

增加：

```markdown
Web / Cloud Planner 可能无法访问用户本机 Global `AGENTS.md`
和本机 Global Skills。

这种情况下不视为阻断。

Web 端至少必须读取：

- 当前仓库 `AGENTS.md`
- 当前任务需要的项目级 Skills
- 当前代码
- 当前 `docs/ahtt` 权威文档

Desktop 环境如果能读取用户级 Global AGENTS / Skills，
则在项目规则基础上继续叠加遵守。

本机规则不得覆盖 AHTT Planner / Executor / Reviewer 的项目 Gate。
```

目的：

```text
网页版没有 C:\Users\Administrator\.codex
也能完整做 Planner。
```

---

# 5. `ahtt-development` 修复

文件：

```text
.agents/skills/ahtt-development/SKILL.md
```

## 5.1 修复 Markdown

确认：

```markdown
# AHTT 开发

## 1. Skill 定位
```

不要出现：

```markdown
- ## 1. Skill 定位
```

---

## 5.2 Prompt 路径统一

将所有：

```text
AHTT AI Development Prompts.md
```

统一替换为：

```text
docs/ai/AHTT_AI_Development_Prompts.md
```

---

## 5.3 Skill 职责保持 Domain-only

保留：

```text
AHTT V3.12
架构导航
Send / Recv / M
GN → AHTT
状态 / 数据 owner
命令分析
sequence / transaction
旧项目参考边界
Global Skill 路由
Bore Skill 路由
```

不要重新加入：

```text
Git commit 规则
C 局部变量规则
U/L 后缀规则
完整 Planner / Executor / Reviewer SOP
```

这些已经分别由：

```text
AGENTS.md
docs/ai/AHTT_AI_Development_Prompts.md
```

维护。

---

# 6. `ahtt-development/agents/openai.yaml`

建议完整内容：

```yaml
interface:
  display_name: "AHTT 开发"
  short_description: "安徽铁塔 AHTT V3.12 固件开发导航与领域约束"
  default_prompt: "使用 $ahtt-development 处理当前 AHTT 任务。如任务属于 Planner、Executor 或 Reviewer，先读取 docs/ai/AHTT_AI_Development_Prompts.md，并仅执行用户明确指定的一个角色。"
```

避免使用过于宽泛的：

```text
分析并推进当前 AHTT 开发任务
```

因为“推进”容易造成角色越界。

---

# 7. `GN同构与校验边界.md`

当前规则应从：

```text
所有 AHTT 代码任务都必须找 GN
```

调整成：

```markdown
涉及 `Protocol_AHTT` 运行时代码，
或者当前任务存在合理 GN 同职责实现时，
必须执行 GN → AHTT 对照。

如果当前任务没有合理 GN 对应实现：

`GN Mapping: N/A`

不得为了模板完整制造虚假 GN 映射。
```

仍然保持：

```text
Protocol_AHTT runtime
状态机
Send / Recv
PlatM
网络业务控制流
```

是 GN 强约束区域。

---

# 8. `任务交付模板.md`

文件顶部增加：

```markdown
> Workflow 说明：
> 如果当前任务处于 AHTT Planner / Executor / Reviewer 工作流，
> 输出契约统一以
> `docs/ai/AHTT_AI_Development_Prompts.md`
> 为准。
>
> 本文仅用于 workflow 外的普通分析，
> 或作为协议/代码检查项参考。
```

避免它和新的 Planner 输出模板竞争。

---

# 9. `ahtt-bore-sim-validation/SKILL.md`

## 9.1 Markdown 修复

确认：

```markdown
# AHTT Bore 模拟联调
```

不要：

```markdown
- # AHTT Bore 模拟联调
```

## 9.2 Prompt 路径

统一为：

```text
docs/ai/AHTT_AI_Development_Prompts.md
```

## 9.3 保持当前授权模型

必须继续保持：

```text
知道 COM 口
≠
获得设备写配置 / reboot 授权
```

允许写设备必须同时满足：

```text
用户要求执行 Bore 联调
+
确认目标设备 / COM
+
已批准 Plan 包含临时端点和 reboot
```

---

# 10. Bore `openai.yaml`

新增：

```text
.agents/skills/ahtt-bore-sim-validation/agents/openai.yaml
```

建议：

```yaml
interface:
  display_name: "AHTT Bore 联调"
  short_description: "AHTT 模拟平台、Bore 与测试桩联调"
  default_prompt: "使用 $ahtt-bore-sim-validation 处理当前 AHTT 板端联调任务。先读取 AGENTS.md、ahtt-development 和 docs/ai/AHTT_AI_Development_Prompts.md，并严格遵守当前角色、Host Gate 与设备授权边界。"
```

---

# 11. Bore 动态运行手册

文件：

```text
.agents/skills/ahtt-bore-sim-validation/references/AHTT-Bore模拟联调运行手册.md
```

必须继续保持动态原则：

```text
Current Evidence > Historical Value
Runtime-discovered Path > Hardcoded Absolute Path
Confirmed Restore Target > Historical Production Endpoint
```

不得重新硬编码：

```text
C:\Users\...
固定 Python.exe
COM3
固定 115200
固定生产域名
固定生产端口
固定 Bore 端口
固定 heartbeat timeout
固定 TCP backoff
```

具体值必须从：

```text
当前代码
当前 --help
用户本次确认
当前设备配置
当前测试会话
```

获取。

同时将所有旧 Prompt 路径改为：

```text
docs/ai/AHTT_AI_Development_Prompts.md
```

---

# 12. AI Development Prompts

文件：

```text
docs/ai/AHTT_AI_Development_Prompts.md
```

保留 v2 三角色结构。

必须具备：

```text
Planner
Executor
Reviewer
```

以及：

```text
Repository Anchor
Approved Plan Gate
PLAN STALE
SCOPE EXPANSION
GN Mapping: N/A
动态 Validation discovery
Correctness > Style
Git Final Gate
```

---

# 13. 新增 Web → Desktop Handoff

在 Prompt 中增加：

```markdown
## P15. Web → Desktop Plan Handoff
```

规则：

Planner 仍然只读，不直接修改仓库。

用户在 Web 端批准 Plan 后：

```text
批准版 Plan
→ 交给 Desktop
→ 保存到 docs/ai/plans/
→ Executor 执行
```

推荐路径：

```text
docs/ai/plans/YYYY-MM-DD-<task-slug>.md
```

Plan 文件顶部增加：

```markdown
## Approval Handoff

Approval Status: APPROVED
Planner Environment: Web ChatGPT
Repository:
Branch:
HEAD:
User Approval Statement:
Handoff Time:
```

以下内容批准后不得由 Desktop 静默改变：

```text
Goal
明确不做范围
Files to Change
Detailed Changes
GN Mapping
State / Data Ownership
State Machine Impact
Validation Plan
```

如果需要改变：

```text
STOP
PLAN STALE
```

或：

```text
STOP
SCOPE EXPANSION
```

然后返回 Planner / 用户重新批准。

---

# 14. 新增 `docs/ai/README.md`

该文件只解释“人怎么用 AI Workflow”。

建议章节：

```text
1. 文件职责
2. Web Planner
3. 用户批准
4. Web → Desktop Handoff
5. Desktop Executor
6. Reviewer
7. Git Gate
8. Plan 生命周期
9. 历史计划
```

推荐生命周期：

```text
DRAFT
→ APPROVED
→ EXECUTING
→ EXECUTED
→ REVIEWED
→ CLOSED
```

---

# 15. 新增 `docs/ai/plans/README.md`

明确：

```text
这里只保存新的、已批准的 AI Implementation Plan。
```

新 Plan 命名：

```text
YYYY-MM-DD-<task-slug>.md
```

新任务不要再默认写入：

```text
docs/superpowers/plans/
```

---

# 16. 根 `README.md`

当前根 README 应成为人工和 AI 的导航入口。

至少包含：

```text
项目规则：
AGENTS.md

AI Workflow：
docs/ai/AHTT_AI_Development_Prompts.md

Web/Desktop 使用说明：
docs/ai/README.md

项目 Skill：
.agents/skills/ahtt-development/

Bore Skill：
.agents/skills/ahtt-bore-sim-validation/

权威文档：
docs/ahtt/

验证：
tools/ahtt/
```

并写清：

```text
Web Planner
→ Desktop Executor
→ Independent Reviewer
→ Human Commit Approval
```

---

# 17. `.gitignore`

公司仓库根目录增加：

```gitignore
# Python runtime
__pycache__/
*.py[cod]

# AHTT runtime / simulator artifacts
tmp/ahtt_sim/
tmp/pdfs/

# Local build logs
02_App/Prj/ahtt_*_build.log

# Editor / OS
.vscode/
.idea/
.DS_Store
Thumbs.db
```

如果公司仓库当前还有其他正式 `.gitignore` 规则：

```text
不要覆盖原文件。
```

只追加本轮缺失项。

---

# 18. 已被 Git 跟踪的运行时文件

注意：

`.gitignore` 不能自动移除已经 tracked 的文件。

先检查：

```bash
git ls-files | grep -E "__pycache__|^tmp/ahtt_sim/|^tmp/pdfs/"
```

如果确认这些全是临时产物，再执行：

```bash
git rm -r --cached --ignore-unmatch tools/ahtt/__pycache__
git rm -r --cached --ignore-unmatch tmp/ahtt_sim
git rm -r --cached --ignore-unmatch tmp/pdfs
```

这里只移除 Git tracking。

是否保留本地日志由你决定。

不要把有长期价值的板端证据直接删除。

需要长期保存的证据建议整理为：

```text
docs/ahtt/evidence/
```

或在板端验证清单 / 联调记录中保存脱敏摘要。

---

# 19. `AHTT-Bore模拟平台联调手册.md`

文件：

```text
docs/ahtt/AHTT-Bore模拟平台联调手册.md
```

不要再作为第二套运行手册。

改成：

```text
导航页
+
历史验证摘要
```

指向真正操作手册：

```text
.agents/skills/ahtt-bore-sim-validation/references/AHTT-Bore模拟联调运行手册.md
```

本文可以保留：

```text
历史上已经观察到：
独立流水号
0x81 静默
Offline
TCP backoff
Bore 恢复
0x04 关键成功路径
```

但必须明确：

```text
历史 COM / 域名 / 端口 / timeout
不能作为当前运行参数。
```

---

# 20. `tools/ahtt/README-ahtt-platform-sim.md`

更新为动态文档。

删除：

```text
固定“4个测试”
固定恢复 www.ahttcd.cn:8888
```

改成：

```text
测试数量以当前 test_*.py 实际结果为准。
```

运行：

```powershell
python tools\ahtt\test_ahtt_platform_sim.py
python tools\ahtt\ahtt_platform_sim.py --selftest
python tools\ahtt\ahtt_platform_sim.py --help
```

板端联调统一跳转：

```text
ahtt-bore-sim-validation
```

恢复地址统一使用：

```text
本次测试开始前确认的 RestoreTarget
```

---

# 21. M1 / M2 / M3 历史 Plan

在以下文件顶部增加统一说明：

```text
docs/ahtt/AHTT-M1基础框架实施计划.md
docs/ahtt/AHTT-M2签到与在线保持实施计划.md
docs/ahtt/AHTT-M3参数与NVM实施计划.md
```

增加：

```markdown
> **Workflow Notice：**
> 本文中的历史 Agent / Sub-skill / Superpowers 执行指令
> 不再作为当前 AHTT AI 工作流的权威来源。
>
> 当前 Planner / Executor / Reviewer 角色与 Gate
> 统一以：
>
> `docs/ai/AHTT_AI_Development_Prompts.md`
>
> 和根目录：
>
> `AGENTS.md`
>
> 为准。
>
> 本文继续保留技术方案、历史 checkbox 和验证证据价值。
```

不要删除历史技术内容。

---

# 22. `docs/superpowers`

新增：

```text
docs/superpowers/README.md
```

内容核心：

```text
Legacy only.

历史计划继续保留。

新的 AHTT Plan：
docs/ai/plans/

新的 AI Workflow：
docs/ai/AHTT_AI_Development_Prompts.md
```

不要为了整理历史目录批量删除旧 Plan。

---

# 23. 同步 `0x04` 当前事实

公司仓库已有的 2026-09-02 板端证据应在各文档保持一致。

当前允许同步为：

```text
[OBSERVED]
0x04 关键成功路径已有板端证据：
- 最新固件收到完整 0x04
- 关闭旧链路
- 连接候选端点
- 候选端点签到成功
- 重启后仍使用候选端点
- 测试结束恢复原确认端点
```

仍然不能写成已验证：

```text
DNS 失败
端口拒绝
NVM 写失败
掉电全过程
20ms WCET
看门狗
栈余量
真实生产平台全部 0x04 口径
```

---

# 24. `AHTT-V3.12-命令追踪矩阵.md`

将 `CMD-04` 旧描述：

```text
Bore已下发完整0x04但候选未建连，板端路径未通过
```

改成类似：

```text
[PARTIAL]
本地 M3 向量与 Keil 通过；
2026-09-02 Bore 板端已验证 0x04 候选端点切换、
候选签到成功、重启后仍使用候选端点并完成恢复；

仍需：
DNS失败
端口拒绝
NVM写失败
掉电
20ms任务/看门狗
等故障与实时性证据。
```

---

# 25. `AHTT-V3.12-事实台账.md`

增加一个新的 OBSERVED 条目，例如：

```text
F-P014
```

内容：

```text
2026-09-02 最新固件已经完成
0x04 → 候选端点 → 签到 → 重启保持 → 恢复原端点
关键成功路径板端验证。
```

影响：

```text
0x04 不能再写成“板端未通过”。

但异常回滚 / NVM / 掉电 / 实时性仍未关闭。
```

---

# 26. `AHTT-M3参数与NVM实施计划.md`

不要机械把完整：

```text
任务8 步骤4
```

直接勾选。

建议补充证据说明：

```text
已确认：
候选连接
候选签到
重启后继续连接候选
恢复原端点

尚缺：
明确 NVM 提交直接日志
成功路径绝无多余 0x04 应答的独立报文证据
```

所以：

```text
保持 - [ ]
```

直到所有该 checkbox 的验收条件都有证据。

---

# 27. `AHTT-板端验证清单.md`

增加：

```markdown
## M3 批次 2：0x04 域名切换
```

至少拆成：

```text
T10-01 最新固件收到 0x04                 通过
T10-02 候选端点连接并签到                通过
T10-03 重启后候选端点保持                通过
T10-04 测试结束恢复                      通过
T10-05 成功路径无额外 0x04 应答          待验证
T10-06 DNS / port / TCP / login failure  待验证
T10-07 NVM fail / power loss             待验证
T10-08 20ms / watchdog / stack           待验证
```

这样不会把“一次成功联调”错误扩大成整个 M3 完成。

---

# 28. `AHTT项目开发总纲.md`

在 Skill 入口附近增加：

```markdown
AHTT Planner / Executor / Reviewer 工作流位于：

`docs/ai/AHTT_AI_Development_Prompts.md`
```

让人工和 Web Planner 都能从总纲找到流程入口。

---

# 29. 新增 AI Workflow 自检脚本

新增：

```text
tools/ahtt/Validate-AHTTAIWorkflow.ps1
```

最少检查：

```text
AGENTS.md 存在
docs/ai/AHTT_AI_Development_Prompts.md 存在
docs/ai/README.md 存在
docs/ai/plans/README.md 存在

两个 project Skill 存在
两个 openai.yaml 存在

Skill 不再引用旧的
“AHTT AI Development Prompts.md”

AGENTS / SKILL 不出现：
- # ...
- ## ...

Bore 文档不把 COM3 / 历史生产地址当当前默认

Simulator README 不固定旧测试数量

GN reference 支持：
GN Mapping: N/A

命令矩阵不再写：
0x04 候选未建连

Git 不跟踪：
__pycache__
tmp/ahtt_sim
tmp/pdfs

能动态发现：
Validate-AHTTM*.ps1
test_*.py
*_sim.py
```

成功输出：

```text
AHTT AI WORKFLOW VALIDATION: PASS
```

---

# 30. 公司仓库修复执行顺序

推荐严格按以下顺序：

```text
Step 1
确认公司仓库 branch / HEAD / git status

Step 2
修 AGENTS.md

Step 3
修 ahtt-development

Step 4
修 ahtt-bore-sim-validation

Step 5
修 docs/ai

Step 6
修 Bore / simulator 文档

Step 7
修 M1/M2/M3 workflow notice

Step 8
同步 0x04 当前事实

Step 9
补 README / .gitignore

Step 10
补 Validate-AHTTAIWorkflow.ps1

Step 11
执行治理自检

Step 12
Review git diff

Step 13
人工确认后 commit

Step 14
同步到 GitHub mirror
```

---

# 31. 公司仓库验证

完成修改后执行：

```bash
git status --short
git diff --stat
git diff
```

然后：

```powershell
powershell -ExecutionPolicy Bypass -File tools\ahtt\Validate-AHTTAIWorkflow.ps1
```

本轮主要是文档和工作流治理，不要求因为本轮本身重新跑所有 AHTT 固件业务测试。

但如果修改了：

```text
tools/ahtt/ahtt_platform_sim.py
test_ahtt_platform_sim.py
Validate-AHTTM*.ps1
```

等实际可执行逻辑，则必须补相应测试。

如果只修改 README / Skill / Prompt：

```text
AI Workflow Validator
+
diff review
```

即可作为本轮主要验证。

---

# 32. 建议公司仓库提交边界

建议至少拆成两个 commit。

## Commit A

```text
chore(ai): align AHTT web-desktop workflow governance
```

包含：

```text
AGENTS.md
.agents/
docs/ai/
docs/ahtt/相关治理文档
README.md
tools/ahtt/README
Validate-AHTTAIWorkflow.ps1
docs/superpowers/README.md
```

## Commit B

```text
chore(repo): ignore generated AHTT runtime artifacts
```

包含：

```text
.gitignore
停止跟踪 __pycache__
停止跟踪 tmp/ahtt_sim
停止跟踪 tmp/pdfs
```

这样后续回退和同步更清晰。

---

# 33. 公司仓库 → GitHub 同步原则

公司仓库完成并确认 commit 后，再同步 GitHub。

禁止：

```text
复制整个 .git
直接覆盖 GitHub 仓库目录
把公司仓库 remote 配置同步过去
把公司私有凭据 / 日志 / 配置一起同步
```

只同步版本化文件内容。

---

# 34. 推荐同步 Manifest

建议把本轮文件列表作为同步 Manifest：

```text
README.md
AGENTS.md
.gitignore

.agents/skills/ahtt-development/SKILL.md
.agents/skills/ahtt-development/agents/openai.yaml
.agents/skills/ahtt-development/references/GN同构与校验边界.md
.agents/skills/ahtt-development/references/任务交付模板.md

.agents/skills/ahtt-bore-sim-validation/SKILL.md
.agents/skills/ahtt-bore-sim-validation/agents/openai.yaml
.agents/skills/ahtt-bore-sim-validation/references/AHTT-Bore模拟联调运行手册.md

docs/ai/AHTT_AI_Development_Prompts.md
docs/ai/README.md
docs/ai/plans/README.md

docs/ahtt/AHTT项目开发总纲.md
docs/ahtt/AHTT-Bore模拟平台联调手册.md
docs/ahtt/AHTT-V3.12-事实台账.md
docs/ahtt/AHTT-V3.12-命令追踪矩阵.md
docs/ahtt/AHTT-板端验证清单.md
docs/ahtt/AHTT-M1基础框架实施计划.md
docs/ahtt/AHTT-M2签到与在线保持实施计划.md
docs/ahtt/AHTT-M3参数与NVM实施计划.md

docs/superpowers/README.md

tools/ahtt/README-ahtt-platform-sim.md
tools/ahtt/Validate-AHTTAIWorkflow.ps1
```

如果公司仓库本轮实际没有改其中某个文件：

```text
不要为了同步清单而无意义覆盖。
```

只同步真实 diff。

---

# 35. PowerShell 同步示例

假设：

```powershell
$CompanyRepo = "公司仓库根目录"
$GithubRepo  = "GitHub镜像仓库根目录"
```

建立本轮实际文件清单：

```powershell
$files = @(
    "README.md",
    "AGENTS.md",
    ".gitignore",
    ".agents/skills/ahtt-development/SKILL.md",
    ".agents/skills/ahtt-development/agents/openai.yaml",
    ".agents/skills/ahtt-development/references/GN同构与校验边界.md",
    ".agents/skills/ahtt-development/references/任务交付模板.md",
    ".agents/skills/ahtt-bore-sim-validation/SKILL.md",
    ".agents/skills/ahtt-bore-sim-validation/agents/openai.yaml",
    ".agents/skills/ahtt-bore-sim-validation/references/AHTT-Bore模拟联调运行手册.md",
    "docs/ai/AHTT_AI_Development_Prompts.md",
    "docs/ai/README.md",
    "docs/ai/plans/README.md",
    "docs/ahtt/AHTT项目开发总纲.md",
    "docs/ahtt/AHTT-Bore模拟平台联调手册.md",
    "docs/ahtt/AHTT-V3.12-事实台账.md",
    "docs/ahtt/AHTT-V3.12-命令追踪矩阵.md",
    "docs/ahtt/AHTT-板端验证清单.md",
    "docs/ahtt/AHTT-M1基础框架实施计划.md",
    "docs/ahtt/AHTT-M2签到与在线保持实施计划.md",
    "docs/ahtt/AHTT-M3参数与NVM实施计划.md",
    "docs/superpowers/README.md",
    "tools/ahtt/README-ahtt-platform-sim.md",
    "tools/ahtt/Validate-AHTTAIWorkflow.ps1"
)
```

复制：

```powershell
foreach ($file in $files)
{
    $src = Join-Path $CompanyRepo $file
    $dst = Join-Path $GithubRepo $file

    if (Test-Path $src)
    {
        $dstDir = Split-Path -Parent $dst

        if (-not (Test-Path $dstDir))
        {
            New-Item -ItemType Directory -Path $dstDir -Force | Out-Null
        }

        Copy-Item $src $dst -Force
    }
}
```

注意：

```text
不要在 $files 中加入：
.git
tmp/
build output
公司私有配置
凭据
日志 dump
```

---

# 36. GitHub 镜像同步后验证

进入 GitHub 镜像仓库：

```bash
git status --short
git diff --stat
git diff
```

然后运行：

```powershell
powershell -ExecutionPolicy Bypass -File tools\ahtt\Validate-AHTTAIWorkflow.ps1
```

还应确认：

```bash
git ls-files | grep -E "__pycache__|^tmp/ahtt_sim/|^tmp/pdfs/"
```

预期：

```text
无结果
```

---

# 37. 公司私有信息同步检查

GitHub 是公开镜像时，提交前必须额外检查：

```text
公司内网地址
账号
密码
token
密钥
测试账号
内部服务器地址
私有 IP
真实客户信息
内部工单
个人绝对路径
原始敏感串口 dump
```

建议运行：

```bash
git diff
```

人工逐行检查所有新文件。

尤其关注：

```text
Bore 日志
联调记录
reference
.env
*.log
公司 Git 地址
本机用户目录
```

不要因为公司仓库允许保存，就直接同步到公开 GitHub。

---

# 38. 最终验收 Checklist

公司仓库修复完成前：

```text
[ ] AGENTS Markdown 正常
[ ] AGENTS 能找到 docs/ai Prompt
[ ] Web 端不依赖本机 Global 文件
[ ] ahtt-development Prompt 路径正确
[ ] Bore Skill Prompt 路径正确
[ ] Bore 运行参数动态化
[ ] docs/ai README 已建立
[ ] docs/ai/plans 已建立
[ ] Web → Desktop Handoff 已定义
[ ] Root README 已建立
[ ] .gitignore 已建立/更新
[ ] tmp / __pycache__ 不再进入新提交
[ ] Simulator README 已动态化
[ ] Bore docs 不再作为第二套运行 SSOT
[ ] M1/M2/M3 已标记旧 workflow
[ ] docs/superpowers 已标记 Legacy
[ ] GN Mapping 支持 N/A
[ ] 0x04 文档状态一致
[ ] AI Workflow Validator PASS
```

GitHub 镜像同步完成前：

```text
[ ] 只同步公司仓库最终版文件
[ ] 不同步 .git
[ ] 不同步 tmp / log / pycache
[ ] 不同步公司私有信息
[ ] GitHub git diff 已人工检查
[ ] GitHub AI Workflow Validator PASS
[ ] GitHub AGENTS / Skills / docs/ai 与公司仓库一致
```

---

# 39. 后续长期规则

以后推荐固定：

```text
公司仓库
=
规则和代码修改源

GitHub
=
公司仓库确认后的镜像
```

任何以下文件变化：

```text
AGENTS.md
.agents/skills/**
docs/ai/**
AHTT 权威文档
tools/ahtt AI/Simulator 文档
```

都采用：

```text
公司修改
→ 公司 Review
→ 公司 Commit
→ GitHub Sync
→ GitHub Validator
```

不要再同时在两边独立编辑同一个规则文件。

---

# 40. 最终工作流

```text
                     ┌─────────────────┐
                     │ 公司 Git 仓库   │
                     │ Source of Truth │
                     └────────┬────────┘
                              │
                    同步已确认版本化文件
                              │
                              ▼
                     ┌─────────────────┐
                     │ GitHub AHTT     │
                     │ Web AI Mirror   │
                     └────────┬────────┘
                              │
                              ▼
                       Web ChatGPT
                         Planner
                              │
                         Plan + HEAD
                              │
                         用户批准
                              │
                 ┌────────────┴────────────┐
                 │                         │
          Desktop 公司仓库          docs/ai/plans
              Executor                  Handoff
                 │
            Build / Test
                 │
             Reviewer
                 │
            用户批准 Git
                 │
             公司 Commit
                 │
          再同步 GitHub Mirror
```

这样公司仓库始终是开发主源，GitHub 始终是可供网页版 Planner 读取的同步镜像，不会形成两个互相漂移的“主仓库”。
