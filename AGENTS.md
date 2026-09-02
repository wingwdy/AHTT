- # AHTT 项目 AGENTS.md

  ## 1. 适用范围与继承关系

  本文件定义 `wingwdy/AHTT` 项目的项目级开发规则。

  本项目继承用户全局 `AGENTS.md` 中的：

  - 默认语言。
  - 通用代码修改确认。
  - Git 操作安全。
  - 工作区保护。
  - 最小修改原则。
  - 正确性优先。
  - 验证证据要求。
  - 通用 Skill 使用规则。

  本文件只维护 AHTT 项目专用规则。

  如果本文件与全局规则在 AHTT 项目特定事项上存在冲突，以本文件为准；但不得借此绕过用户明确要求、Git 写操作门禁或其他安全确认。

  ---

  ## 2. AHTT AI 工作流

  AHTT 项目默认采用：

  ```text
  Planner
    ↓
  用户批准 Implementation Plan
    ↓
  Executor
    ↓
  独立 Reviewer
    ↓
  用户确认
    ↓
  Git add / commit / push / PR
  ```

  ### 2.1 Planner

  Planner 只负责：

  - 读取真实仓库。
  - 查协议。
  - 查当前架构和调用链。
  - 对照 GN。
  - 确认状态和数据 owner。
  - 设计验证方案。
  - 输出 Implementation Plan。

  Planner 阶段不得修改代码或执行 Git 写操作。

  ### 2.2 Executor

  当用户已经明确批准当前 Implementation Plan，并明确要求开始实施时：

  - 视为已经满足全局“代码修改前置确认”。
  - Executor 可直接执行已批准范围。
  - 不需要针对同一份 Plan 再次重复确认。

  出现以下情况时必须停止：

  ```text
  PLAN STALE
  计划外文件
  修改范围扩大
  设计偏离
  新增显著风险
  需要改变已批准行为
  ```

  停止后应说明差异并等待重新规划或用户确认。

  ### 2.3 Reviewer

  Reviewer 默认只读：

  - 审查当前 diff。
  - 审查协议符合性。
  - 审查架构边界。
  - 审查状态机和数据所有权。
  - 审查边界条件和异常路径。
  - 审查测试证据。

  Reviewer 不因为代码由 AI 自己生成就默认正确。

  ---

  ## 3. 项目级 Skills

  AHTT 项目版本化 Skills 位于：

  ```text
  .agents/skills/
  ```

  当前项目级 Skill 至少包括：

  ```text
  ahtt-development
  ahtt-bore-sim-validation
  ```

  使用原则：

  - AHTT 协议分析、设计、实现、调试和评审优先读取 `ahtt-development`。
  - Bore / TCP 模拟器 / 4G 桩板端联调任务按需读取 `ahtt-bore-sim-validation`。
  - 全局 Skills 只提供通用方法论，不覆盖本项目专用 Planner / Executor / Reviewer 的角色边界。
  - 项目 Skill 不应重复维护本文件已经定义的 C 编码规范和 Git 安全规则。
  - 项目 Skill 中的历史架构基线不得覆盖当前代码和当前权威文档。

  ---

  ## 4. AHTT C 代码通用规则

  本节是 AHTT 项目 C 编码规范的项目级 SSOT。

  只约束本次新增或修改范围，不因规则存在而批量重写无关旧代码。

  ---

  ## 5. C 函数局部变量声明

  新增或修改 C 函数时：

  - 局部变量统一放在函数开头的变量声明区。
  - 不得在函数中间语句块、`if/else`、`switch/case`、循环体中新增局部变量声明。
  - 不在 `for (...)` 初始化语句中新增局部变量声明。
  - 变量依赖运行期上下文时，只上移声明，不提前执行赋值或函数调用。
  - 赋值和函数调用应保留在原有语义正确的位置。
  - 检查上移后的作用域扩大、未使用变量、初始化副作用和变量复用风险。

  不得为了满足本规则改变原有执行语义。

  ---

  ## 6. C 函数单出口

  新增或修改 C 函数时采用单出口结构：

  - 有返回值函数在函数开头声明返回值变量。
  - 中间分支不得通过提前 `return` 退出。
  - 在函数末尾保留统一 `return`。
  - `void` 函数不得通过中间 `return;` 提前退出，应使用条件保护或状态控制组织流程。
  - 不得为了单出口引入无必要 `goto`。
  - 不得引入重复副作用。
  - 不得改变资源释放、时序或错误处理语义。

  如果现有函数无法在保持行为等价的情况下满足单出口：

  ```text
  STYLE-CONFLICT-WITH-CORRECTNESS
  ```

  停止机械修改并说明原因。

  ---

  ## 7. 数值字面量后缀

  新增或修改 AHTT C 代码时，默认不得使用：

  ```text
  U
  L
  UL
  ULL
  ```

  等数值字面量后缀。

  例如：

  ```c
  0xEA
  12
  ```

  而不是：

  ```c
  0xEAU
  12U
  ```

  适用于：

  - 宏。
  - 枚举。
  - 赋值。
  - 比较。
  - 方案文档中的 AHTT C 代码示例。

  除非用户针对具体位置明确允许，不得新增上述后缀。

  但本规则不得机械用于改变：

  - C 表达式类型语义。
  - 整数提升。
  - 位移结果。
  - 溢出行为。
  - ABI。
  - 硬件寄存器访问语义。

  如果正确性需要特定整数类型，必须停止并说明，不得为了风格制造行为变化。

  ---

  ## 8. 日志语言

  新增、生成或修改的代码打印日志必须使用简体中文，包括但不限于：

  ```text
  InfoPrint
  DebugPrint
  ErrorPrint
  printf
  LOG
  协议调试输出
  ```

  不因本规则批量改写当前任务未涉及的旧日志。

  ---

  ## 9. C 函数 Doxygen 注释

  新建、生成或修改 C 函数时使用简体中文 Doxygen 函数头。

  至少包含：

  ```text
  @brief
  ```

  存在参数时使用：

  ```text
  @param[in]
  @param[out]
  @param[in,out]
  ```

  存在返回值时使用：

  ```text
  @retval
  ```

  涉及以下复杂逻辑时必须补充 `@note`：

  - 状态切换。
  - 协议字段处理。
  - 持久化。
  - 硬件输出。
  - 异常路径。
  - 兼容性约束。
  - 关键业务决策。

  复杂函数体关键阶段使用：

  ```c
  /* Step1: ... */
  /* Step2: ... */
  /* Step3: ... */
  ```

  中文注释说明业务决策、原因和副作用，不逐行翻译代码。

  ---

  ## 10. C 宏与结构体中文注释

  新增或修改 C 宏时：

  - 宏定义行末添加简洁准确的中文注释。
  - 存在单位、范围、生命周期或特殊语义时一并说明。

  新增或修改结构体、联合体时：

  - 每个新增或修改成员在声明行末添加中文注释。
  - 涉及单位、字节序、有效范围或生命周期时一并说明。

  不得因为补注释改变：

  - 命名。
  - 数据布局。
  - 协议语义。
  - ABI。

  ---

  ## 11. 新建文件作者

  新创建文件中，如果模板、文件头或元数据存在：

  ```text
  Author
  author
  作者
  ```

  字段，统一填写：

  ```text
  wdy
  ```

  不得填写：

  - codex。
  - 模型名称。
  - Agent 名称。

  本规则只约束新创建文件，不批量修改既有文件作者信息。

  ---

  ## 12. Protocol_AHTT 未使用形参

  在 `Protocol_AHTT` 目录中：

  - 不新增或保留 `(void)变量;` 形式的未使用形参标记。
  - 未使用形参仍保留在函数声明和定义中。
  - 不删除函数形参。
  - 不改变函数原型。
  - 不改变函数指针、回调或公开接口兼容性。

  修改相关代码后，应检索当前 `Protocol_AHTT` 修改范围，确认不存在因本次修改新增的 `(void)标识符;`。

  如果 Plan 要求全目录清理，则按 Plan 执行并进行构建验证。

  ---

  ## 13. AHTT 协议报文长度

  后续新增或修改 AHTT 协议报文组包代码时：

  - `dataLen` 对协议固定字段长度的直接累加、赋值或固定长度运算使用协议原始数值。
  - 不为这类固定字段长度新增 AHTT 私有长度宏。
  - 如果当前任务明确移除了既有 AHTT 私有固定长度宏，应同步替换本任务范围内全部引用，避免无效定义或未定义引用。

  例如：

  ```c
  dataLen += 20;
  ```

  而不是为固定协议长度专门引入：

  ```c
  dataLen += AHTT_XXX_LEN;
  ```

  其他模块中的：

  - 版本号。
  - 配置项。
  - 公共平台宏。
  - 非 AHTT 私有长度宏。

  不因本规则自动删除。

  ---

  ## 14. 防御性保护边界

  AHTT 防御性校验重点放在真实边界：

  - 平台下发。
  - TCP / 网络输入。
  - 协议报文。
  - 外部配置参数。
  - NVM 数据恢复。
  - 资源创建和生命周期边界。

  对已经由：

  - 系统初始化顺序。
  - 模块生命周期。
  - 模块私有状态。
  - 内部调用契约。

  保证有效的内部对象，不重复增加：

  - NULL 检查。
  - 状态检查。
  - 端口检查。
  - 索引检查。
  - 资源已创建标志。
  - 全局门控 flag。

  修改或新增 AHTT 代码时，优先对照同职责 GN 控制流。

  如果 GN 在相同内部调用位置未做防御，而当前调用契约能证明安全：

  - AHTT 默认保持同构。
  - 不自行增加 wrapper 或额外保护层。

  如果确实需要增加 GN 没有的内部防御：

  1. 说明真实可达的失败来源。
  2. 说明缺少保护的具体后果。
  3. 说明为什么不能在输入边界或初始化边界解决。
  4. 纳入 Implementation Plan。
  5. 取得用户批准后实施。

  用户明确要求移除的内部防御不得以“更安全”为由自动加回。

  ---

  ## 15. GN 同构原则

  `Protocol_GN` 是 AHTT 运行时代码的重要架构和控制流参考。

  当任务涉及以下内容时优先执行 GN → AHTT 对照：

  ```text
  Protocol_AHTT 运行时代码
  协议状态机
  Send / Recv control flow
  PlatM 接口
  网络 / 业务交互
  与 GN 存在同职责实现的功能
  ```

  对照原则：

  - 优先保持文件职责同构。
  - 优先保持函数层级同构。
  - 优先保持状态控制同构。
  - 优先保持 send / recv control flow 同构。
  - 优先保持错误处理位置同构。
  - 不因个人偏好重新分层、包装或重命名。

  如果协议语义或产品需求明确不同：

  - 可以偏离。
  - 必须在 Plan 中写清差异和原因。

  如果当前任务不存在合理 GN 对应实现：

  ```text
  GN Mapping: N/A
  ```

  不得为了形式强行建立映射。

  GN 只是当前实现的重要架构模板，不得覆盖：

  - 用户确认的当前产品需求。
  - AHTT 原始协议。
  - 当前真实代码。
  - 当前 build / test / 抓包 / 板端证据。

  ---

  ## 16. 状态和数据所有权

  任何设计和实现都必须先确认当前 owner。

  重点确认：

  ```text
  协议运行态
  网络状态
  充电业务状态
  持久化参数
  报文字节
  transaction / command runtime
  ```

  不得创建第二事实源。

  当前架构中的 owner 应从：

  - 当前代码。
  - 当前权威架构文档。
  - 当前事实台账。

  重新确认。

  Skill 或 Prompt 中记录的历史 owner 只能作为核对基线，不得覆盖当前证据。

  ---

  ## 17. 网络输入与 Recv 安全

  涉及平台下行或 TCP 输入时，根据任务适用性检查：

  ```text
  帧头
  声明长度
  真实长度
  最小帧
  超长帧
  版本
  设备号
  CRC
  命令
  参数长度
  端口
  索引
  枚举
  范围
  流水号
  transaction id
  业务前置状态
  ```

  同时分析适用的 TCP 情况：

  ```text
  半包
  粘包
  连续多帧
  异常帧恢复
  重复请求
  重复应答
  超时
  断链
  重连
  ```

  必须确保坏帧不会无故破坏后续正常帧处理。

  ---

  ## 18. Send 侧检查

  修改 AHTT 发送逻辑时，根据任务适用性检查：

  ```text
  payload length
  buffer limit
  dataLen
  max frame length
  command
  device number
  sequence
  response sequence source
  CRC
  endianness
  parameter unit
  send control
  retry control
  ```

  特别注意：

  - 设备流水号。
  - 平台流水号。
  - 请求 / 应答流水号关系。

  不得混用语义。

  ---

  ## 19. NVM 修改

  如果 Plan 涉及 `MS_Nvm` 或 AHTT 私有持久化数据，必须分析：

  ```text
  default
  version
  migration
  旧数据兼容
  结构体布局
  union size
  write failure
  restore
  重复写
  写频率
  RAM / NVM 一致性
  掉电一致性
  ```

  不得擅自改变其他协议或模块的 NVM 布局。

  ---

  ## 20. 实施计划复选框同步

  执行包含 Markdown checkbox 的 Implementation Plan 时：

  - 完成一个步骤后先取得与验收条件匹配的证据。
  - 只有证据充分才能把：

  ```text
  - [ ]
  ```

  更新为：

  ```text
  - [x]
  ```

  - 未验证、部分完成或验证失败的步骤保持未勾选。
  - 不等整个任务结束后再集中补勾。
  - 每次继续执行旧 Plan 前，先核对 checkbox 与当前真实状态。
  - 已勾选但证据不足时应纠正。
  - 已完成且证据充分但未勾选时应同步。

  板端或平台尚未验证的步骤不得因为本地测试通过而勾选为完全完成。

  ---

  ## 21. 验证原则

  AHTT 任务根据当前仓库实际存在的工具动态发现验证入口。

  优先检查：

  ```text
  tools/ahtt/Validate-AHTTM*.ps1
  tools/ahtt/test_*.py
  tools/ahtt/*_sim.py
  02_App/Prj/Project.uvprojx
  ```

  不得把历史 Prompt 或 Skill 中列出的脚本清单当作永久完整事实。

  根据任务实际需要区分：

  ```text
  [PASS-LOCAL]
  [PASS-BUILD]
  [PASS-SIM]
  [PASS-BOARD]
  [PASS-PLATFORM]
  [PENDING-BOARD]
  [PENDING-PLATFORM]
  [FAILED]
  ```

  禁止：

  ```text
  应该能通过
  理论上没有问题
  ```

  代替真实验证。

  Host test、Simulator、Keil build、Board、真实 Platform 的证据等级不得混淆。

  ---

  ## 22. 文档同步

  完成 AHTT 功能修改后，根据任务影响检查是否需要同步：

  - 项目开发总纲。
  - 软件架构设计。
  - V3.12 事实台账。
  - V3.12 命令追踪矩阵。
  - 报文测试向量。
  - 板端验证清单。
  - 联调问题记录。
  - 当前 M* Implementation Plan。

  不得出现代码已经实现，但权威状态文档长期仍写“未实现”的明显漂移。

  只同步与当前任务实际相关的文档。

  ---

  ## 23. 本机环境信息

  本项目 `AGENTS.md` 不保存本机绝对路径。

  禁止把以下类型信息当作仓库事实：

  ```text
  C:\Users\...
  D:\...
  E:\...
  固定本地 workspace 路径
  固定用户级 Skill 安装路径
  ```

  Desktop Codex 使用当前实际工作目录。

  Web / Cloud 环境不得假设本机路径存在。

  项目版本化资源统一使用仓库相对路径。

  ---

  ## 24. 正确性兜底

  AHTT 项目所有风格规则都受以下原则约束：

  ```text
  Correctness > Style
  Current Evidence > Historical Prompt
  Current Code > Stale Plan
  Approved Scope > Unrelated Cleanup
  ```

  任何规范都不得机械用于改变：

  - 协议语义。
  - C 类型语义。
  - ABI。
  - 数据布局。
  - Flash / NVM 布局。
  - 执行时序。
  - RTOS 调度行为。
  - 硬件访问。
  - 错误恢复。
  - 资源生命周期。

  发生冲突时停止并说明，不得为了“规则一致”制造功能错误。

  ---

  ## 25. Git 门禁

  AHTT 项目继承全局 Git 安全规则。

  即使：

  - Executor 已完成。
  - 所有本地测试通过。
  - Reviewer 给出 APPROVE。

  也不得自动：

  ```text
  git add
  git commit
  git push
  ```

  不得自动创建或合并 PR。

  下一步必须先向用户展示：

  - 变更摘要。
  - 验证结果。
  - 剩余风险。
  - Commit / PR Preview。
  - 拟执行 Git 命令。

  取得用户明确确认后才能执行 Git 写操作。
