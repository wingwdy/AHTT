# AHTT M3 参数与 NVM 实施计划

> **面向代理执行者：** 实施时必须使用 `test-driven-development`，按本文复选框逐项执行和同步状态；完成代码后必须使用 `verification-before-completion`。若用户明确选择子代理执行，再使用 `superpowers:subagent-driven-development`。

**目标：** 在 M1/M2 基础上实现 `0x02/0x03/0x04/0x0A/0x0B/0x84/0x85` 参数闭环，保证合法性校验、设置/查询一致性、重复设置不重复写、NVM 写失败回滚、掉电恢复，以及域名/端口试连、提交和失败回退。

**架构：** 保持 AHTT Types/Recv/Send/M 四层和 `Init → Offline → Login → Normal` 四态主状态机。Recv 只完成不可信报文解析、全字段校验和事务发起；M 层持有域名切换事务状态并协调 `Cdd_NetM`；参数持久化统一通过 `MS_Nvm`，不新增 Flash 直写路径。`0x04`采用“候选地址先试连，签到成功后提交 NVM”的两阶段策略，避免错误地址提前持久化导致设备失联。

**技术栈：** C（ARMClang/Keil）、FreeRTOS 20ms 平台任务、`FrameQueue`、`Cdd_NetM`、`MS_Nvm`、`CommonSendCtrl/CommonRecvCtrl`、PowerShell 主机侧报文/NVM/状态推进测试。

**规格基线：** `docs/ahtt/AHTT-软件架构设计.md`、`docs/ahtt/AHTT-V3.12-事实台账.md`、`docs/ahtt/AHTT-V3.12-命令追踪矩阵.md`。

---

## 1. 当前基线与开始条件

### 1.1 已具备的 M1/M2 基线

- AHTT 七文件骨架、23 项收发控制槽位和平台描述符已存在。
- `0x01`签到、`0x81`心跳、同命令应答关联和命令级超时重试已经实现。
- `MSNvmAHTTParam_Struct`已预留心跳周期、最大充电时长、8 字节运维参数和温度阈值；两个 NVM 联合体仍保持 1280 字节。
- `0x02/0x03/0x04/0x0A/0x0B/0x84/0x85`在收发控制表中已有槽位，但解析函数和组包函数均为空。
- M2 本地向量和 Keil 构建已完成；M2 资源记录、完整板端重试/重连验证和文档收口仍未全部勾选。

### 1.2 M3 开始门禁

M3 可以先完成任务 1 的事实冻结和测试框架；以下三项未关闭前，不进入对应命令实现：

| 门禁 | 必须取得的证据 | 阻塞任务 |
|---|---|---|
| C-003 | 当前平台抓包或协议负责人确认：首期仅使用 `0x04`合并设置 24 字节域名和 6 字节 ASCII 端口，`0x05`不实现；同时确认同地址重复设置的应答规则 | 任务 6 |
| C-007 | 确认 `0x84/0x85`采用专节 8 个 1 字节字段，还是汇总表中的功率字段布局；给出每字段单位、范围和是否实际应用 | 任务 5 |
| D-003 | 明确 `0x84`中哪些字段仅保存/回显，哪些字段必须传递给充电或业务模块生效 | 任务 5 |

若证据仍未取得，允许完成 `0x02/0x03/0x0A/0x0B`及公共事务机制，但不得猜测 `0x04/0x84/0x85`字段，也不得把 M3 标记完成。

### 1.3 本计划不实现

- 不实现首期范围外的 `0x05/0x06～0x09/0x0C～0x0F`。
- 不实现 `0xC4/0xC5`温度阈值协议闭环；其 NVM 字段可保留，但命令归 M5。
- 不冻结 `MSNvmAHTTOrderInfo_Struct`；订单结构归 M4。
- 不修改 `MSNvm_WriteParaBlock()`同步写机制，不新增异步参数队列。
- 不修改 GN、YKC、XDT 等其他平台行为，不重构公共协议框架。
- 不把 Keil 编译通过等同于掉电恢复、真实平台试连或板端通过。

## 2. 已确认字段和响应规则

| 命令 | 请求参数 | 成功响应 | 失败响应 | 持久化 |
|---|---|---|---|---|
| `0x02` | 周期 1 字节，单位分钟，范围 1～10 | 结果 `0x01` | 结果 `0x00` | 私有参数块 |
| `0x03` | 0 字节 | 当前周期 1 字节 | 非零请求长度时不分发响应 | 不写 |
| `0x04` | 24 字节域名 + 6 字节 ASCII 端口；以 C-003 关闭结果为准 | 新地址签到成功后不发送设置应答 | 新地址失败并回到旧地址签到后发送结果 `0x00` | 成功试连后公共平台参数块 |
| `0x0A` | 时长 1 字节，单位小时，范围 1～16；`0`非法 | 结果 `0x01` | 结果 `0x00` | 私有参数块 |
| `0x0B` | 0 字节 | 当前时长 1 字节 | 非零请求长度时不分发响应 | 不写 |
| `0x84` | 以 C-007 关闭后的固定字段表为准 | 结果 `0x01` | 结果 `0x00` | 私有参数块，全部字段原子更新 |
| `0x85` | 0 字节 | 与最终 `0x84`请求字段布局完全一致 | 非零请求长度时不分发响应 | 不写 |

统一规则：

1. 设置类命令必须先完成长度和全部字段校验，再修改 RAM。
2. 值未变化时直接返回成功，但不得触发 NVM 实际写入。
3. NVM 写失败时响应失败，AHTT 可见 RAM 恢复旧值，并把 NVM 内部 RAM 镜像重新指向旧值，防止后台重试把失败候选值写入。
4. 查询命令只返回当前已生效 RAM 值；不得为了查询读取或写入 Flash。
5. 下行命令的响应流水号使用接收帧流水号，由现有 `Common_SetRecvSeq()`和发送控制项回传。

## 3. 文件边界和接口

| 文件 | M3 职责 | 禁止事项 |
|---|---|---|
| `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTTypes.h` | 参数长度、范围、结果码、域名切换状态和事务数据类型 | 不放业务动作和 NVM 调用 |
| `02_App/Src/BSW/MemoryService/NVM/MS_NvmAppTypes.h` | 冻结 AHTT 私有参数布局、字段注释、版本字段和容量断言 | 不改 1280 字节联合体容量，不改订单布局 |
| `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTRecv.c` | 7 个命令的长度/范围校验、临时值构造、事务调用 | 校验完成前不得改 RAM；不直接控制网络驱动 |
| `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTSend.c` | 设置结果和查询值组包 | 不写 NVM；不改变参数 |
| `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTM.h` | 对 Recv 暴露参数提交和域名切换请求接口 | 不暴露 AHTT 上下文内部指针 |
| `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTM.c` | 私有参数事务提交、默认值/版本迁移、域名切换状态机 | 不新增平行主状态机；不直接访问 Flash 驱动 |
| `02_App/Src/ASW/ASW_COM/Asw_PlatM/Asw_PlatM.c/.h` | 若现有 API 无法保证提交失败后的 RAM 回滚，增加最小的公共平台参数事务接口 | 不改变既有 setter 语义，不波及其他平台 |
| `tools/ahtt/Validate-AHTTM3.ps1` | 报文、边界、原子性、重复写和域名切换状态模型 | 不依赖板端硬件 |
| `docs/ahtt/AHTT-报文测试向量.md` | M3 黄金报文和非法边界 | 不提前写入未经平台确认的 `0x84`黄金值 |
| `docs/ahtt/AHTT-板端验证清单.md` | NVM、掉电、域名回滚和任务实时性步骤 | 不把未执行项标记通过 |

计划接口如下；实施时函数名和签名保持一致：

```c
uint8_t IotAHTT_CommitPrivateParam(const MSNvmAHTTParam_Struct *pNewParam);
uint8_t IotAHTT_RequestEndpointSwitch(const char *pDomain, uint8_t domainLen,
    uint16_t port, uint16_t recvSeq);
void IotAHTT_EndpointSwitchMainFunction(void);
```

若需要补充 PlatM 事务接口，限定为：

```c
uint8_t AswPlatM_CommitPlatMainIpPort(const char *pIp, uint8_t ipLen, uint16_t port);
```

该接口只能“候选完整写入成功后更新 PlatM 运行镜像”；失败时必须恢复 NVM 内部镜像和 PlatM 运行镜像。不得修改现有 `AswPlatM_SetPlatMainIpPort()`，避免改变其他平台调用语义。

## 4. 域名切换状态模型

```text
Idle
  └─ 收到合法且不同的0x04
      ├─ 保存旧地址、候选地址和下行流水号
      ├─ CddNetM_UpdateIpPort(候选)
      ├─ 断开当前链路
      └─ TryingNew

TryingNew
  ├─ 新地址TCP连接并0x01签到成功
  │   ├─ 提交公共平台参数NVM
  │   ├─ 成功：清事务，保持新连接，不发送0x04响应
  │   └─ 失败：恢复旧运行地址并断链 → RollingBack
  └─ 签到达到M2重试上限
      └─ 恢复旧运行地址并断链 → RollingBack

RollingBack
  ├─ 旧地址TCP连接并0x01签到成功
  │   ├─ 恢复保存的0x04下行流水号
  │   ├─ 使能0x04失败响应
  │   └─ 清事务 → Idle
  └─ 旧地址仍不可达
      └─ 保持旧地址和普通M2重连策略，不把候选写入NVM
```

边界规则：

- 收到与当前地址完全相同的 `0x04`时，不断链、不写 NVM；按 C-003 关闭时确认的固定应答规则处理并形成一条黄金向量。
- `TryingNew`和`RollingBack`期间再次收到 `0x04`时返回失败，不覆盖正在执行的事务。
- 候选域名只允许 ASCII 可打印字符，去除右侧 `0x00`填充后长度必须为 1～`CDD_NETM_CFG_IP_LEN - 1`；中间出现 `0x00`后不得再出现非零字符。
- 端口 6 字节必须全部为 ASCII 数字，转换值范围为 1～65535；禁止 `atoi()`无边界转换。
- 域名、候选端口和原请求流水号存放在 AHTT 上下文中，普通 Offline 清理不得清除活动事务；事务完成后必须清零。
- 设备在 `TryingNew`期间掉电时，旧地址仍在 NVM，重启后自动回到旧地址；这是选择“试连成功后提交”的核心恢复保证。

## 5. 分任务实施步骤

### 任务 1：冻结 M3 协议事实和测试清单

**文件：**

- 更新：`docs/ahtt/AHTT-V3.12-事实台账.md`
- 更新：`docs/ahtt/AHTT-V3.12-命令追踪矩阵.md`
- 更新：`docs/ahtt/AHTT-报文测试向量.md`
- 新建：`tools/ahtt/Validate-AHTTM3.ps1`

**产出：** M3 七个命令的唯一字段表、响应规则、最小/最大/非法向量和状态推进模型。

- [ ] **步骤 1：记录 C-003、C-007、D-003 的关闭证据**

把确认结论写成“命令、字段偏移、长度、单位、合法范围、应用行为、持久化行为”七列。证据不足的命令保持阻塞，不写假定值。

- [x] **步骤 2：建立 M3 帧构造和 CRC 复用函数**

`Validate-AHTTM3.ps1`按已通过的 M1/M2 帧构造和 CRC 规则实现独立向量，提供 `New-AhttFrame`和断言函数；本批不重构已通过的 M1/M2 脚本。

- [x] **步骤 3：写入先失败的参数向量**

本批覆盖：

```text
0x02: 0/1/10/11、长度0/1/2、重复设置
0x03: 请求长度0/1、默认5、设置后查询
0x0A: 0/1/16/17、长度0/1/2、重复设置
0x0B: 请求长度0/1、默认10、设置后查询
```

- [x] **步骤 4：运行并确认测试因 M3 功能缺失而失败**

运行：

```powershell
powershell -ExecutionPolicy Bypass -File tools\ahtt\Validate-AHTTM3.ps1
```

预期：脚本自身语法通过；M3 业务断言失败，失败原因指向未实现解析/组包/事务，不是 CRC 或 M1/M2 回归错误。

### 任务 2：冻结私有参数布局、默认值和升级兼容

**文件：**

- 修改：`02_App/Src/BSW/MemoryService/NVM/MS_NvmAppTypes.h`
- 修改：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTM.c`

**产出：** 固定布局的 `MSNvmAHTTParam_Struct`、默认值、版本识别和 M2→M3 迁移规则。

- [x] **步骤 1：为 AHTT 私有参数增加尾部版本字段**

版本字段追加在当前字段末尾，禁止插入前部导致 M2 已存的 `heartCycleMin`和`maxChargeTimeHour`偏移变化。所有新增宏和结构体成员在声明行末写中文注释。

- [ ] **步骤 2：将最终 `0x84`字段固化为有语义成员**

只有 C-007/D-003 关闭后才把 `devOperationParam[8]`替换为已确认成员；若平台确认仍按 8 字节原值保存，则保留数组并为每一偏移建立常量和中文说明，不把未知字段伪装成业务字段。

- [x] **步骤 3：实现版本 0 到版本 1 的原位迁移**

迁移规则固定为：保留范围合法的 M2 心跳周期和最大充电时长；非法值恢复为 5 分钟和 10 小时；其余 M3 新字段使用已确认默认值；设置当前版本号后仅在数据真实变化时请求一次私有参数块写入。迁移写失败时，运行参数与NVM内部镜像均保留安全值，并设置待持久化标志；后续同值设置或下次启动继续尝试持久化。

- [x] **步骤 4：保持 NVM 容量和既有平台兼容**

编译期继续满足：

```c
sizeof(MSNvmPlatPrivateParam_Union) == MSNVM_PLAT_PRIVATE_PARAM_LEN
sizeof(MSNvmPlatOrderInfo_Union) == MSNVM_ORDER_MAX_LEN
```

并确认 `eAswPlatType_AHTT`仍只追加在 DXL 之后；本任务不改任何已有平台结构体。

验证证据：2026-09-02 Keil 目标`D3_A32FB_GD32E503RE`全量构建为`0 Error(s), 0 Warning(s)`，现有两个联合体容量断言继续参与编译。

### 任务 3：实现私有参数原子提交公共路径

**文件：**

- 修改：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTM.h`
- 修改：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTM.c`

**接口：**

- 消费：完整且已校验的 `MSNvmAHTTParam_Struct`候选值。
- 产出：`IotAHTT_CommitPrivateParam()`；`TRUE`表示候选已生效或与当前值相同，`FALSE`表示写入失败且旧值继续生效。

- [x] **步骤 1：实现“无变化不写”判断**

比较范围只覆盖 `MSNvmAHTTParam_Struct`，相同则直接成功；不得调用 `MSNvm_WriteParaBlock()`。

- [x] **步骤 2：实现一次提交和失败回滚**

流程固定为：保存小尺寸旧 AHTT 结构→复制候选到 PlatM 私有参数镜像→调用 `MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, ...)`→失败时恢复旧 AHTT 结构并再次调用同一 API，让 NVM 内部镜像也回到旧值。第二次恢复写失败必须打印可观察错误并返回失败。

- [x] **步骤 3：验证不在 20ms 任务栈上复制 1280 字节联合体**

局部快照仅使用 `MSNvmAHTTParam_Struct`；不得声明 `MSNvmPlatPrivateParam_Union`局部副本。所有局部变量放在函数开头，函数保持单出口。

- [x] **步骤 4：加入写次数模型测试**

验证首次变化写 1 次、重复设置写 0 次、首次写失败时 RAM 恢复且恢复路径更新 NVM 内部镜像、查询写 0 次。

### 任务 4：实现 `0x02/0x03/0x0A/0x0B`

**文件：**

- 修改：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTRecv.c`
- 修改：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTSend.c`

**产出：** 四个命令的解析函数、组包函数和收发控制表绑定。

- [x] **步骤 1：绑定四个 Recv 解析函数**

解析函数严格检查长度；设置函数先复制当前私有参数到小候选结构，更新单字段后调用 `IotAHTT_CommitPrivateParam()`，并把成功/失败结果写入 AHTT 协议运行数据。

- [x] **步骤 2：绑定四个 Send 组包函数**

`0x02/0x0A`响应固定 1 字节结果；`0x03/0x0B`响应固定 1 字节当前值。组包函数只读取运行数据或 PlatM 私有参数，不写 NVM。

- [ ] **步骤 3：验证参数立即生效**

`0x02`成功后下一次心跳周期计算读取新值；不得重置已发送心跳的流水号。`0x0A`在 M3 只保存并回显，实际充电超时仲裁接口在 M4 接入前保持无副作用。

- [x] **步骤 4：运行四命令向量和 M1/M2 回归**

运行：

```powershell
powershell -ExecutionPolicy Bypass -File tools\ahtt\Validate-AHTTM1.ps1
powershell -ExecutionPolicy Bypass -File tools\ahtt\Validate-AHTTM2.ps1
powershell -ExecutionPolicy Bypass -File tools\ahtt\Validate-AHTTM3.ps1
```

预期：M1、M2 全部保持通过；M3 中四命令相关用例全部通过。

验证证据：2026-09-02 M1为9/9通过，M2为11项向量及17项源码一致性检查通过，M3为18/18通过。

### 任务 5：实现 `0x84/0x85`原子设置和查询

**文件：**

- 修改：`02_App/Src/BSW/MemoryService/NVM/MS_NvmAppTypes.h`
- 修改：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTTypes.h`
- 修改：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTRecv.c`
- 修改：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTSend.c`
- 按 D-003 结论可能修改：当前充电/业务参数接口文件

**开始条件：** 用户已确认本批采用参考项目的8字节兼容布局；C-007和D-003仍保持未关闭，未确认字段不得接入充电业务。

- [x] **步骤 1：定义参考兼容字段长度、偏移和范围宏**

每个宏行末写中文注释，不使用带 `U/L`后缀的数值字面量。请求长度必须由字段定义计算，不手写第二份魔法数字。

- [x] **步骤 2：解析到局部候选结构并一次性校验**

任何字段非法时：不修改运行参数、不写 NVM、响应 `0x00`。全部合法时才调用一次 `IotAHTT_CommitPrivateParam()`。

- [x] **步骤 3：按参考兼容边界保存字段**

本批不调用当前充电业务接口：`cardmoney`和`uploadcyc`分别留给M4刷卡/实时数据闭环读取，其余字段仅保存和回显；不得直接写充电全局变量。

- [x] **步骤 4：实现 `0x85`完整回显**

响应字段顺序、宽度和单位必须与最终 `0x84`请求完全一致；默认值、设置后值和掉电恢复值分别形成黄金向量。

- [x] **步骤 5：验证多字段原子性**

构造“前 7 字段合法、最后 1 字段非法”和“NVM 写失败”两类故障；确认所有字段均保持旧值，没有部分更新，没有业务接口获得候选值。

验证证据：M3主机向量覆盖长度7/8/9、`uploadcyc`为0/1/30/31、重复设置、写失败回滚及`0x85`零参数查询。

### 任务 6：实现 `0x04`域名/端口两阶段切换

**文件：**

- 修改：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTTypes.h`
- 修改：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTM.h`
- 修改：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTM.c`
- 修改：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTRecv.c`
- 修改：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTSend.c`
- 可能修改：`02_App/Src/ASW/ASW_COM/Asw_PlatM/Asw_PlatM.c`
- 可能修改：`02_App/Src/ASW/ASW_COM/Asw_PlatM/Asw_PlatM.h`

**开始条件：** C-003 已确认首期 `0x04`字段和响应时序。

- [x] **步骤 1：实现无库副作用的域名和端口解析**

端口逐字符执行十进制累积，并在每一步检查不超过 65535；不得使用会接受空串、符号或尾随字符的宽松转换函数。

- [x] **步骤 2：在 AHTT 上下文增加最小事务数据**

只保存状态、旧地址、候选地址、端口和原请求流水号；所有结构体成员行末写中文注释。活动事务字段不在普通 Offline 清理中重置。

- [x] **步骤 3：接入 20ms 主状态处理**

在 `IotAHTT_WSNormalHandle()`和登录成功/最终超时路径调用域名事务推进函数；不增加第五个主工作状态，不复制 TCP 重连计时。

- [x] **步骤 4：实现候选试连成功后的持久化提交**

新地址 `0x01`签到成功才提交 `MSNvmPlatParam_Struct`。提交成功后更新运行镜像并清事务；协议规定成功不应答时不得额外发送 `0x04 01`。

- [x] **步骤 5：实现试连或写入失败后的旧地址回滚**

恢复 `CddNetM`运行地址并断开候选链路；旧地址重新签到后，用保存的下行流水号发送 `0x04 00`。回滚期间不得启用第二条运营平台连接。

- [x] **步骤 6：覆盖掉电和重复命令状态模型**

验证 `TryingNew`掉电后从旧 NVM 地址启动；事务忙时新的 `0x04`不覆盖旧事务；同地址设置不写 NVM、不重建连接。

验证证据：2026-09-02 `Validate-AHTTM3.ps1`覆盖M3-VEC-014至M3-VEC-018，包括固定长度字段、候选签到成功静默提交、同步和异步NVM校验失败回滚及原流水号、同地址静默成功、事务忙和试连中掉电；Keil目标`D3_A32FB_GD32E503RE`为`0 Error(s), 0 Warning(s)`。候选TCP超时固定为60秒；连接后签到沿用现有3次×10秒机制；平台参数块异步校验成功前事务保持提交校验状态，最终失败会恢复旧地址。

### 任务 7：Keil 构建、静态影响面和资源验证

**文件：**

- 构建：`02_App/Prj/Project.uvprojx`
- 记录：`docs/ahtt/AHTT-板端验证清单.md`

- [x] **步骤 1：执行 M1/M2/M3 全部主机向量**

预期所有脚本 0 失败；测试输出单独列出边界、原子性、写次数、升级迁移和域名状态模型。

验证证据：2026-09-02 M1为9/9通过，M2为11项向量及17项源码一致性检查通过，M3为18/18通过。

- [x] **步骤 2：执行 Keil 全量构建**

```powershell
& 'C:\Keil_v5\UV4\UV4.exe' -b '02_App\Prj\Project.uvprojx' -t 'D3_A32FB_GD32E503RE' -j0 -o '02_App\Prj\ahtt_m3_build.log'
```

预期：0 Error(s)，AHTT 新增警告为 0；与 M2 基线比较 Flash、RAM 和任务栈变化。

验证证据：2026-09-02 Keil目标`D3_A32FB_GD32E503RE`返回码为0，日志为`0 Error(s), 0 Warning(s)`；当前程序尺寸为Code=330112、RO-data=59076、RW-data=5236、ZI-data=115340。未取得M2构建产物尺寸基线，不作资源增量结论。

- [x] **步骤 3：执行静态边界检查**

```powershell
rg -n "IotAHTT_CommitPrivateParam|IotAHTT_RequestEndpointSwitch|MSNvm_WriteParaBlock|CddNetM_UpdateIpPort" 02_App\Src\ASW\ASW_COM\Asw_IotProtocol\Protocol_AHTT 02_App\Src\ASW\ASW_COM\Asw_PlatM
rg -n "0x05|CMD_SET_PORT" 02_App\Src\ASW\ASW_COM\Asw_IotProtocol\Protocol_AHTT
```

确认 NVM 写入口集中、网络更新只在 M 层、未误实现 `0x05`、未修改其他协议行为。

验证证据：2026-09-02确认`0x04/0x84/0x85`已绑定收发回调；AHTT网络更新仅在M层，NVM写入口集中于AHTT M层和公共NVM API，`0x05`未实现；AHTT新增/修改代码未出现数值字面量后缀，未创建`MSNvmPlatPrivateParam_Union`局部副本。平台参数块异步校验状态仅由新增只读查询接口暴露，不改变既有写入、重试和备份机制。

- [ ] **步骤 4：检查 20ms 任务实时性**

记录最坏参数设置和 NVM 同步写期间的执行时间、`Task_20msB`最低栈余量和看门狗状态。若同步写导致周期超限，记录为阻断证据并单独提出 NVM 架构改造，不在 M3 内擅自改异步机制。

### 任务 8：板端、掉电和平台闭环

**文件：**

- 更新：`docs/ahtt/AHTT-板端验证清单.md`
- 更新：`docs/ahtt/AHTT-联调问题记录.md`
- 更新：`docs/ahtt/AHTT-V3.12-命令追踪矩阵.md`
- 更新：`docs/ahtt/AHTT-软件架构设计.md`

- [ ] **步骤 1：验证普通参数闭环**

逐项执行最小值、最大值、非法边界、重复设置、设置后查询、重启后查询；确认重复设置不产生 NVM 实际写日志。

- [ ] **步骤 2：执行 NVM 故障注入和掉电试验**

在写前、主块写期间、写完成后分别掉电；确认只出现完整旧值或完整新值，不出现混合字段。注入写失败时设备响应失败，重启后仍为旧值。

- [ ] **步骤 3：验证 M2 到 M3 升级**

使用保留 M2 参数块的设备升级 M3：合法的 5/10 或已设置值保留；新增字段采用确认默认值；迁移最多产生一次写入；M2 签到和心跳不回归。

- [ ] **步骤 4：验证 `0x04`成功路径**

旧平台下发新地址→只保留一个连接→新地址完成签到→NVM 提交→重启后仍连接新地址；确认成功路径没有多余 `0x04`应答。

- [ ] **步骤 5：验证 `0x04`失败和掉电路径**

分别注入 DNS 失败、端口拒绝、TCP 超时、签到超时、NVM 写失败和试连中掉电；确认设备最终使用旧地址、候选未落盘，并在能恢复旧连接时发送失败应答。

- [ ] **步骤 6：更新证据层级**

只把实际通过的命令更新为“已实现/本地通过/板端通过/平台通过”。未完成真实平台 `0x04/0x84`联调时不得标记 M3 完成。

## 6. 验收门禁

### 6.1 本地通过

- M1/M2/M3 PowerShell 向量全部通过，0 失败。
- Keil `D3_A32FB_GD32E503RE`全量构建 0 错误，AHTT 新增警告为 0。
- `0x02/0x0A/0x84`非法和写失败路径没有部分 RAM 更新。
- 重复设置不触发实际 NVM 写；查询命令不写 NVM。
- 两个 NVM 联合体仍为 1280 字节，M2 字段偏移保持兼容。
- `0x04`状态模型覆盖成功、试连失败、提交失败、回滚、事务忙和试连中掉电。
- 新增/修改 C 函数局部变量位于函数开头，保持单出口；新增宏和结构体成员有中文行末注释；数值字面量无 `U/L/UL/ULL`后缀。

### 6.2 板端通过

- 参数设置、查询、重启恢复与报文一致。
- NVM 写期间 20ms 任务和看门狗稳定，栈余量满足项目门槛。
- `0x04`成功后重启仍连接新地址；失败和掉电后回到旧地址。
- 切换期间最多一条有效运营平台连接，不影响本地充电安全功能。
- M2 签到、心跳、超时和默认重连路径无回归。

### 6.3 平台通过

- 当前安徽铁塔平台确认 `0x04`命令号、字段和成功不应答时序。
- 当前平台确认 `0x84/0x85`最终字段布局，并完成设置/查询一致性联调。
- 平台侧未因域名切换、重复设置、失败响应或流水号回传产生异常断链。

## 7. 风险与控制

| 风险 | 控制方式 |
|---|---|
| `0x84/0x85`字段冲突导致错误 NVM 布局 | 把 C-007/D-003 设为任务 5 硬门禁，证据不足不实现 |
| 新地址提前落盘导致设备失联 | 候选地址先试连并签到，成功后才提交 NVM |
| 写失败后 NVM 后台重试候选值 | 恢复 AHTT RAM 后再次调用 NVM API，用旧值覆盖 NVM 内部镜像 |
| 1280 字节联合体压栈 | 事务只保存小型 AHTT 结构，不创建联合体局部副本 |
| M3 修改公共 setter 影响其他平台 | 新增独立事务接口，保持既有 setter 语义不变 |
| `0x04`事务被 Offline 清理丢失 | 活动事务与普通收发控制清理分离，仅在提交或回滚完成后清零 |
| 同步 NVM 写阻塞 20ms 任务 | 保留 DEC-014，同步测量 WCET/栈/看门狗；超限后单独立项 |
| M2 板端验证尚未完全收口 | M3 本地工作可并行，M3 板端验收前必须补齐 M2 阻断项 |

## 8. 建议实施顺序和提交边界

1. 任务 1：协议事实与失败测试。
2. 任务 2～3：NVM 布局、迁移和原子提交。
3. 任务 4：`0x02/0x03/0x0A/0x0B`，形成第一个可独立验收提交。
4. 任务 5：`0x84/0x85`，仅在字段门禁关闭后实施，形成第二个提交。
5. 任务 6：`0x04`两阶段切换，单独评审和提交。
6. 任务 7～8：全量回归、板端、平台联调和文档收口。

推荐提交信息：

```text
test(ahtt): add M3 parameter and NVM vectors
feat(ahtt): add atomic private parameter persistence
feat(ahtt): implement parameter set and query commands
feat(ahtt): add transactional endpoint switching
docs(ahtt): record M3 verification evidence
```

## 9. 实施确认边界

本计划本身不授权修改代码。执行任务 2～8 前，必须按仓库规则再次展示具体代码修改方案并取得用户明确回复“同意”“开始实施”或等效确认。若实施中需要改变公共 NVM 写机制、增加首期外命令、修改四态主状态机数量或改变其他平台 setter 行为，必须停止并重新提交方案。
