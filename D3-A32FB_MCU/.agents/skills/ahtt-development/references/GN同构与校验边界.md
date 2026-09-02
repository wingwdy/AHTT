# GN 同构与校验边界

## 1. 同构基准

涉及 `Protocol_AHTT` 运行时代码，或者当前任务存在合理 GN 同职责实现时，先在 `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_GN/` 找到同职责实现。AHTT 以 GN 为代码骨架，只替换平台标识和协议字段。

如果当前任务没有合理 GN 对应实现，Plan 或 Review 必须写明：

```text
GN Mapping: N/A
```

不得为了模板完整制造虚假映射。

| GN | AHTT |
|---|---|
| `Protocol_GN` | `Protocol_AHTT` |
| `Asw_IotProtoGNM.c/.h` | `Asw_IotProtoAHTTM.c/.h` |
| `Asw_IotProtoGNSend.c/.h` | `Asw_IotProtoAHTTSend.c/.h` |
| `Asw_IotProtoGNRecv.c/.h` | `Asw_IotProtoAHTTRecv.c/.h` |
| `Asw_IotProtoGNTypes.h` | `Asw_IotProtoAHTTTypes.h` |
| `IotGN_` | `IotAHTT_` |
| `IotGN..._Struct/Enum` | `IotAHTT..._Struct/Enum` |
| `IOT_GN_...` | `IOT_AHTT_...` |
| `pIotGNCtx` | `pIotAHTTCtx` |
| `stIotGN...` | `stIotAHTT...` |

大小写、下划线、缩写、后缀、单复数和词序均沿用 GN，不自行“优化”。新增协议概念没有直接对应项时，搜索 GN 中同类职责的命名形式后再决定。

## 2. 固定状态机

AHTT 使用与 GN 相同的四态主状态机：

```text
eIOTAHTTWorkState_Init
  → eIOTAHTTWorkState_Offline
  → eIOTAHTTWorkState_Login
  → eIOTAHTTWorkState_Normal
```

对应类型与函数：

| GN | AHTT |
|---|---|
| `IotGNWorkState_Enum` | `IotAHTTWorkState_Enum` |
| `IotGN_WSInitHandle` | `IotAHTT_WSInitHandle` |
| `IotGN_WSOfflineHandle` | `IotAHTT_WSOfflineHandle` |
| `IotGN_WSLoginHandle` | `IotAHTT_WSLoginHandle` |
| `IotGN_WSNormalHandle` | `IotAHTT_WSNormalHandle` |
| `IotGN_MainFunction` | `IotAHTT_MainFunction` |
| `IotGN_OfflineHandle` | `IotAHTT_OfflineHandle` |

协议签到、重连和在线业务应放入对应状态处理函数，不为 AHTT 另造平行主状态机。若协议行为需要子状态，在不改变四态主骨架的前提下设计，并先取得方案确认。

## 3. 函数与数据命名

有直接对应关系时只替换平台标识，例如：

```text
IotGN_SendLoginReq       → IotAHTT_SendLoginReq
IotGN_RecvLoginRsp       → IotAHTT_RecvLoginRsp
IotGN_GetSendCtrl        → IotAHTT_GetSendCtrl
IotGN_UpCtrlSendDeal     → IotAHTT_UpCtrlSendDeal
IotGN_TimeoutDetect      → IotAHTT_TimeoutDetect
IotGNSendCtrl_Struct     → IotAHTTSendCtrl_Struct
IotGNRecvCtrl_Struct     → IotAHTTRecvCtrl_Struct
IotGNFrameHead_Struct    → IotAHTTFrameHead_Struct
```

每次代码方案附带最小映射表：GN 符号、AHTT 符号、是否完全同构、差异原因。没有协议差异时，“完全同构”应为是。

## 4. 防御性校验边界

### 必须校验

平台和网络输入属于不可信边界，必须按协议校验：

- 接收缓冲区与声明长度；
- 帧头、版本、设备号、CRC 和命令；
- 平台下发字段的范围、单位、枚举和值域；
- 平台下发端口号、单号、重复命令和业务前置状态；
- 可变长度字段及其缓冲区上限。

### 按 GN 对应位置处理

模块内部可信对象由初始化和调用契约保证，不重复增加防御性代码：

- `pIotAHTTCtx` 是否判空，严格查看 `pIotGNCtx` 的对应位置；
- GN 普通状态机、周期发送、接收和超时路径未判断 `pIotGNCtx`，AHTT 对应路径也不判断；
- GN 在 `InitMemory` 分配后检查、在 `FillLinkPara` 等生命周期边界组合检查，AHTT 保持相同位置和控制流；
- GN 对内部 getter 返回值、内部端口或函数指针有检查时，AHTT 才在对应位置保留同类检查；平台报文派生的端口仍属于不可信输入，必须校验。

### 新增判断的门禁

没有 GN 对应项时，不默认添加 NULL/范围/状态判断。只有能说明实际可达的失败来源、未检查的具体后果以及为何不能在输入边界或初始化阶段消除时，才在代码方案中提出；取得用户确认后实施。

## 5. 评审清单

- 文件、函数、类型、变量是否存在 GN 对应项？
- 能否仅通过 `GN/gn` 替换得到 AHTT 名称？
- 四态主状态机及处理顺序是否保持一致？
- 是否无理由增加包装层、辅助状态或控制分支？
- 每个新增防御判断对应的 GN 位置在哪里？
- 若 GN 没有该判断，是否提供了已确认的实际失败路径？
- 平台下发参数是否完成长度、范围和业务状态校验？
