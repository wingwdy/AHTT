# AHTT M1 基础框架实施计划

> **Workflow Notice：** 本文中的历史 Agent / Sub-skill / Superpowers 执行指令不再作为当前 AHTT AI 工作流的权威来源。当前 Planner / Executor / Reviewer 角色与 Gate 统一以 `docs/ai/AHTT_AI_Development_Prompts.md` 和根目录 `AGENTS.md` 为准。本文继续保留技术方案、历史 checkbox 和验证证据价值。

> **面向代理式执行者：** 实施本计划时必须使用任务驱动的分步执行流程；每完成一个任务先验证，再进入下一任务。所有代码修改仍需用户对本方案明确确认。

**目标：** 在 D3-A32FB_MCU 中建立可编译、可注册、可创建TCP链路、可组装AHTT基础帧并可处理单个FrameQueue接收块内粘包和异常恢复的M1框架，不实现签到、心跳及其他业务命令；M1不保证跨FrameQueue接收块半包恢复。

**架构：** 严格沿用 `Protocol_GN` 的 Types/Send/Recv/M 文件职责、公共收发控制、FrameQueue解码回调方式和 `Init → Offline → Login → Normal` 四态骨架。AHTT采用确认后的12字节固定头、CRC16-MODBUS和5字节BCD逆序设备号；当前不增加私有重组缓存，也不修改公共FrameQueue。

**技术栈：** C、FreeRTOS、Keil MDK、`Asw_PlatM`、`Cdd_NetM`、`FrameQueue`、`MS_Nvm`、PowerShell报文向量脚本。

---

## 1. M1范围

### 1.1 包含范围

- 创建 `Protocol_AHTT` 七个固定文件。
- 建立AHTT帧常量、类型、上下文、首期23命令常量和23项收发控制容量。
- 建立四态主状态机和 `Asw_PlatM` 生命周期接口。
- 从 `platPileDn`生成5字节BCD逆序设备号。
- 创建AHTT专用TCP FrameQueue通道。
- 建立基础组帧、CRC、流水号和FrameQueue发送调度骨架。
- 建立与GN同构的当前接收块内帧搜索、粘包、连续帧、异常恢复和基础合法性校验。
- 增加AHTT平台枚举、卡类型枚举、平台描述符和通用UUID卡描述符。
- 增加AHTT私有参数和订单NVM类型的初始结构，并绑定默认初始化函数。
- 将AHTT源文件和include path加入Keil工程。
- 建立独立报文向量脚本和M1验证文档。

### 1.2 不包含范围

- 不实现`0x01`签到参数组包或应答解析。
- 不实现`0x81`心跳组包、超时和重连。
- 不执行`0x02/0x03/0x04/0x0A/0x0B/0x84/0x85/0xC4/0xC5`参数业务。
- 不调用充电、订单、告警、时间同步、二维码或UCM OTA业务接口。
- 不实现M1之外命令的字段解析；23项表只建立命令元数据和运行控制槽位。
- 不修改`FrameQueue.c/.h`、GN或其他既有平台协议。
- 不保证分布在两个或多个FrameQueue接收块中的半包能够恢复；不完整数据按GN现有路径处理。
- 不修改`IotOM_GetPlatNo()`；M1板端测试关闭辅助运维平台，AHTT运维平台编号另行确认。
- 不宣称板端或安徽铁塔平台通过。

## 2. 创建和修改文件

### 2.1 新建文件

| 文件 | 职责 |
|---|---|
| `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTTypes.h` | 帧、命令、控制表类型和常量 |
| `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTSend.c` | 发送表、组帧和发送调度 |
| `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTSend.h` | 发送层公开入口 |
| `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTRecv.c` | TCP重组、解帧、校验、分发和超时骨架 |
| `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTRecv.h` | 接收层公开入口 |
| `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTM.c` | 四态状态机、平台适配和默认参数 |
| `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTM.h` | 上下文、状态和平台接口声明 |
| `tools/ahtt/Validate-AHTTM1.ps1` | 独立CRC、帧长和流式边界验证 |
| `docs/ahtt/AHTT-报文测试向量.md` | M1黄金帧和异常向量 |

### 2.2 修改文件

| 文件 | 修改内容 |
|---|---|
| `02_App/Src/ASW/ASW_COM/Asw_PlatM/Asw_PlatM.h` | 追加AHTT平台和卡类型枚举 |
| `02_App/Src/ASW/ASW_COM/Asw_PlatM/Asw_PlatMConfig.c` | 引入AHTT并注册协议/卡描述符 |
| `02_App/Src/BSW/MemoryService/NVM/MS_NvmAppTypes.h` | 增加AHTT私有参数、订单类型和两个联合体成员 |
| `02_App/Prj/Project.uvprojx` | 增加include path、AHTT分组和三个C源文件 |
| `docs/ahtt/AHTT-V3.12-命令追踪矩阵.md` | M1完成后更新基础链路状态 |
| `docs/ahtt/AHTT-软件架构设计.md` | 回写FrameQueue跨块半包限制和接收路径沿用GN的设计决策 |

## 3. GN→AHTT映射和非同构点

### 3.1 文件与核心符号映射

| GN | AHTT | M1处理 |
|---|---|---|
| `IotGNWorkState_Enum` | `IotAHTTWorkState_Enum` | 完全同构 |
| `IotGNCtx_Struct` | `IotAHTTCtx_Struct` | 公共控制同构，仅替换AHTT设备号等协议字段 |
| `IotGNSendCtrl_Struct` | `IotAHTTSendCtrl_Struct` | 完全同构 |
| `IotGNRecvCtrl_Struct` | `IotAHTTRecvCtrl_Struct` | 完全同构 |
| `IotGNFrameHead_Struct` | `IotAHTTFrameHead_Struct` | 协议字段不同 |
| `IotGN_GetSendCtrl` | `IotAHTT_GetSendCtrl` | 23命令槽位映射 |
| `IotGN_GetRecvCtrl` | `IotAHTT_GetRecvCtrl` | 23命令槽位映射 |
| `IotGN_WSInitHandle` | `IotAHTT_WSInitHandle` | 完全同构 |
| `IotGN_WSOfflineHandle` | `IotAHTT_WSOfflineHandle` | 重置AHTT上下文和FrameQueue |
| `IotGN_WSLoginHandle` | `IotAHTT_WSLoginHandle` | M1只处理TCP连接，不发送签到 |
| `IotGN_WSNormalHandle` | `IotAHTT_WSNormalHandle` | M1只推进基础收发，不运行业务周期 |
| `IotGN_FillLinkPara` | `IotAHTT_FillLinkPara` | 同构接口，增加失败返回 |
| `IotGN_InitMemory` | `IotAHTT_InitMemory` | 同构分配，失败时不解引用空指针 |
| `IotGN_MainFunction` | `IotAHTT_MainFunction` | 完全同构状态分发 |
| `IotGN_UpCtrlSendDeal` | `IotAHTT_UpCtrlSendDeal` | AHTT帧头、长度和CRC不同 |
| `IotGN_UpCtrlRecvDeal` | `IotAHTT_UpCtrlRecvDeal` | 严格沿用GN的FrameQueue回调模式 |
| `IotGN_TimeoutDetect` | `IotAHTT_TimeoutDetect` | M1条件不满足时无动作，后续命令方案再定义策略 |

### 3.2 必须明确批准的非同构点

1. AHTT固定头为12字节，不复用GN帧头。
2. AHTT流水号范围为1～60000，0不得发送。
3. `IotAHTT_FillLinkPara()`在上下文分配或FrameQueue创建失败时返回`FALSE`；GN当前始终返回`TRUE`。
4. `IotAHTT_InitMemory()`只在分配成功后写函数指针；GN当前存在分配失败后继续解引用的路径。
5. AHTT存在`0x95`等零参数上行请求，参数长度0是合法值；发送组包使用`0xFFFF`表示失败，不沿用GN的“长度0不发送”判断。

以下项目经用户确认严格沿用GN，不再列为非同构点：

- 不增加AHTT私有`rxCache`。
- `IotAHTT_DecodeData()`和`dealLen`处理沿用GN。
- 平台描述符绑定AHTT同名计费、订单、刷卡和历史订单转换回调；M1提供安全占位实现，不绑定GN业务函数。

## 4. 任务拆分

### 任务1：建立M1黄金向量和独立验证脚本

**文件：**

- 新建：`tools/ahtt/Validate-AHTTM1.ps1`
- 新建：`docs/ahtt/AHTT-报文测试向量.md`

- [x] **步骤1：建立CRC16-MODBUS函数**

脚本使用初值`0xFFFF`和多项式`0xA001`，输出CRC低字节在前。输入登录示例帧CRC前数据：

```text
EA270001000040003401000138393836303431363132313938303030343135310100000000000000
```

预期CRC：

```text
数值：0xA53A
线上：3A A5
```

- [x] **步骤2：建立无参数基础帧向量**

使用设备号`00 00 30 00 34`、流水号`01 00`、命令`0x95`、无参数：

```text
EA 0B00 01 0000300034 0100 95 4C3F
```

无空格形式：

```text
EA0B000100003000340100954C3F
```

预期总长14字节，声明长度11字节，CRC数值`0x3F4C`，线上`4C 3F`。

- [x] **步骤3：建立流式边界向量**

脚本必须验证以下输入模型：

| 用例 | 输入 | 期望 |
|---|---|---|
| M1-VEC-001 | 完整无参数帧 | 解析1帧 |
| M1-VEC-002 | 在1～13每个位置拆分无参数帧并按两个FrameQueue块输入 | 不越界、不误分发；M1不要求第二块到达后恢复完整帧 |
| M1-VEC-003 | 两个完整帧一次输入 | 解析2帧 |
| M1-VEC-004 | `12 34 56`加完整帧 | 丢弃3字节后解析1帧 |
| M1-VEC-005 | 错CRC帧加正常帧 | 错帧不分发，正常帧被解析 |
| M1-VEC-006 | 版本`0x02`帧加正常帧 | 错版本不分发，正常帧被解析 |
| M1-VEC-007 | 错设备号帧加正常帧 | 错设备号不分发，正常帧被解析 |
| M1-VEC-008 | 声明长度`0x000A` | 判定小于最小11，按1字节重新同步 |
| M1-VEC-009 | 声明长度导致总长大于512 | 判定超限，按1字节重新同步 |

- [x] **步骤4：运行脚本验证测试基线**

执行：

```powershell
powershell -ExecutionPolicy Bypass -File tools\ahtt\Validate-AHTTM1.ps1
```

预期：

```text
AHTT M1 vectors: 9 passed, 0 failed
```

### 任务2：建立AHTT类型、常量和上下文

**文件：**

- 新建：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTTypes.h`
- 新建：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTM.h`

- [x] **步骤1：定义基础常量**

```c
#define IOT_AHTT_HEAD                         (0xEA)
#define IOT_AHTT_PROTOCOL_VERSION             (0x01)
#define IOT_AHTT_FRAME_HEAD_LEN               (12)
#define IOT_AHTT_CRC_LEN                      (2)
#define IOT_AHTT_DECLARE_MIN_LEN              (11)
#define IOT_AHTT_FRAME_MIN_LEN                (14)
#define IOT_AHTT_FRAME_MAX_LEN                (512)
#define IOT_AHTT_FRAME_QUEUE_BUF_SIZE         (3072)
#define IOT_AHTT_DEVICE_NUM_LEN               (5)
#define IOT_AHTT_SIM_NUM_LEN                  (20)
#define IOT_AHTT_SEQ_MIN                      (1)
#define IOT_AHTT_SEQ_MAX                      (60000)
#define IOT_AHTT_CMD_SEND_COUNT               (23)        /* 发送命令控制项数量 */
#define IOT_AHTT_CMD_RECV_COUNT               (23)        /* 接收命令控制项数量 */
#define IOT_AHTT_CMDTYPE_REQUSET              (0x00)
#define IOT_AHTT_CMDTYPE_RESPONSE             (0x01)
#define IOT_AHTT_CMD_NULL                     (0xFF)
#define IOT_AHTT_PACK_INVALID_LEN             (0xFFFF)
```

保留`REQUSET`拼写以与GN公共风格一致，不在AHTT中单独修正。

- [x] **步骤2：定义首期23命令常量**

```c
#define IOT_AHTT_CMD_LOGIN                    (0x01)
#define IOT_AHTT_CMD_SET_HEART_CYCLE          (0x02)
#define IOT_AHTT_CMD_QUERY_HEART_CYCLE        (0x03)
#define IOT_AHTT_CMD_SET_DOMAIN_PORT          (0x04)
#define IOT_AHTT_CMD_SET_MAX_CHARGE_TIME      (0x0A)
#define IOT_AHTT_CMD_QUERY_MAX_CHARGE_TIME    (0x0B)
#define IOT_AHTT_CMD_STOP_CHARGE              (0x4B)
#define IOT_AHTT_CMD_CARD_AUTH                (0x4C)
#define IOT_AHTT_CMD_START_CHARGE             (0x4D)
#define IOT_AHTT_CMD_HEARTBEAT                (0x81)
#define IOT_AHTT_CMD_SET_DEV_PARAM            (0x84)
#define IOT_AHTT_CMD_QUERY_DEV_PARAM          (0x85)
#define IOT_AHTT_CMD_REPORT_REALDATA          (0x93)
#define IOT_AHTT_CMD_REPORT_ORDER             (0x94)
#define IOT_AHTT_CMD_QUERY_TIME               (0x95)
#define IOT_AHTT_CMD_REPORT_DEV_STATE         (0x96)
#define IOT_AHTT_CMD_DEV_ALARM                (0xC1)
#define IOT_AHTT_CMD_NET_ALARM                (0xC2)
#define IOT_AHTT_CMD_TEMP_ALARM               (0xC3)
#define IOT_AHTT_CMD_SET_TEMP_LIMIT           (0xC4)
#define IOT_AHTT_CMD_QUERY_TEMP_LIMIT         (0xC5)
#define IOT_AHTT_CMD_ELECTRIC_ALARM           (0xC7)      /* 上报过流、过压或欠压告警 */
#define IOT_AHTT_CMD_UPDATE                   (0xD1)
```

- [x] **步骤3：定义帧头和收发控制类型**

```c
typedef struct
{
    uint8_t head;
    uint8_t len[2];
    uint8_t ver;
    uint8_t deviceNum[5];
    uint8_t seq[2];
    uint8_t cmd;
}IotAHTTFrameHead_Struct;

typedef uint16_t (*IotAHTT_pSendPackFuncType)(uint8_t port, uint8_t *pBuf);
typedef uint8_t (*IotAHTT_pRecvParseFuncType)(uint8_t *port, uint8_t *pData, uint16_t len);

typedef struct
{
    uint16_t cmd;
    uint8_t cmdType;
    uint32_t sendCycle;
    IotAHTT_pSendPackFuncType pSendFunc;
    uint16_t matchCmd;
    uint8_t printFlag;
    char *cMeaning;
}IotAHTTSendCtrl_Struct;

typedef struct
{
    uint16_t cmd;
    uint8_t cmdType;
    IotAHTT_pRecvParseFuncType pRecvParse;
    uint16_t maxTimeout;
    uint16_t maxTryCnt;
    uint16_t matchCmd;
    uint8_t printFlag;
    char *cMeaning;
}IotAHTTRecvCtrl_Struct;

typedef char IotAHTTFrameHeadSizeCheck[(sizeof(IotAHTTFrameHead_Struct) == 12) ? 1 : -1];
```

- [x] **步骤4：定义四态和M1上下文**

```c
typedef enum
{
    eIOTAHTTWorkState_Init,
    eIOTAHTTWorkState_Offline,
    eIOTAHTTWorkState_Login,
    eIOTAHTTWorkState_Normal,
}IotAHTTWorkState_Enum;

typedef struct
{
    IotAHTTWorkState_Enum eWorkState;
    typeFuncSendCtrl pFuncSendCtrl;
    typeFuncRecvCtrl pFuncRecvCtrl;
    uint8_t frameQueueChannelID;
    uint8_t deviceNum[5];
    uint8_t loginSucc;
    uint8_t queueBusyFlag;
    uint32_t waitQueueIdleTick;
    uint8_t sendIndex;
    uint8_t sendPort;
    uint16_t reqSeq;
    CommonSendCtrl_Struct stSendCtrl[SYSCFG_CFG_GUN_NUM][IOT_AHTT_CMD_SEND_COUNT];
    CommonRecvCtrl_Struct stRecvCtrl[SYSCFG_CFG_GUN_NUM][IOT_AHTT_CMD_RECV_COUNT];
}IotAHTTCtx_Struct;
```

所有C函数局部变量在函数开头声明；本规则适用于后续全部任务。

### 任务3：建立发送表、基础组帧和发送调度

**文件：**

- 新建：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTSend.c`
- 新建：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTSend.h`

- [x] **步骤1：建立23项发送元数据表**

按2.2中的23命令顺序建立`c_stIotAHTTSendctrlTable`。M1仅填写`cmd`、方向、`matchCmd`、日志标志和含义，所有`pSendFunc`设为`NULL`，所有`sendCycle`设为0。设备主动命令为`0x01/0x4C/0x81/0x93/0x94/0x95/0x96/0xC1/0xC2/0xC3/0xC7`；其余12项为对平台请求的应答。

- [x] **步骤2：实现基础帧封装函数**

函数保持GN命名模式：

```c
static uint16_t IotAHTT_PackHead(uint8_t cmd, uint16_t seq, uint8_t *pBuf, uint16_t dataLen);
```

处理顺序：

```text
totalLen = 12 + dataLen
head = 0xEA
len = dataLen + 11，低字节在前
ver = 0x01
deviceNum = pIotAHTTCtx->deviceNum
seq = 低字节在前
cmd = cmd
crc = Common_CalcCRC16(pBuf, totalLen)
追加crc低字节、crc高字节
返回totalLen + 2
```

参数长度0表示合法零参数帧。如果`dataLen + 14 > 512`或组包输入无效，返回`IOT_AHTT_PACK_INVALID_LEN`且不写FrameQueue。

- [x] **步骤3：实现1～60000流水号推进**

发送设备主动请求时：

```text
当前reqSeq不在1～60000：本次使用1
当前reqSeq在1～59999：发送成功后加1
当前reqSeq为60000：发送成功后回绕到1
FrameQueue_PushTx失败：reqSeq保持不变
```

- [x] **步骤4：实现`IotAHTT_UpCtrlSendDeal()`**

沿用GN的表轮询、端口轮询、立即发送、周期发送、队列忙500ms保护、请求/应答流水号选择和公共收发计时器调用顺序。函数开头定义单个`uint8_t txBuf[IOT_AHTT_FRAME_MAX_LEN]`发送临时缓冲。`pSendFunc != NULL`且返回值不等于`IOT_AHTT_PACK_INVALID_LEN`时允许封帧，包括返回0的零参数帧；`pSendFunc == NULL`时不发送。M1表项没有组包回调，因此正常运行时不得产生AHTT业务帧。

- [x] **步骤5：Keil编译检查发送层**

暂不注册AHTT平台，只对新文件执行语法和依赖检查；完整工程编译在任务7执行。

### 任务4：建立GN同构解帧和异常恢复

**文件：**

- 新建：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTRecv.c`
- 新建：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTRecv.h`

- [x] **步骤1：建立23项接收元数据表**

按任务3相同命令顺序建立`c_stIotAHTTRecvctrlTable`。设备主动命令的入站方向是平台应答，其余12项的入站方向是平台请求。M1的`pRecvParse`全部为`NULL`；超时和重试先设0，M2以后逐命令填写。

- [x] **步骤2：实现命令查找**

```c
static const IotAHTTRecvCtrl_Struct *IotAHTT_GetRecvCtrlPtr(uint16_t cmd);
```

只遍历23项静态表。未知命令返回`NULL`。

- [x] **步骤3：实现设备号和CRC校验**

有效帧必须同时满足：

```text
head == 0xEA
declareLen >= 11
frameLen = declareLen + 3
frameLen <= 512
ver == 0x01
deviceNum与上下文5字节完全一致
Common_CalcCRC16(frame, frameLen - 2) == 线上低字节/高字节还原值
```

- [x] **步骤4：实现当前接收块内帧搜索函数**

函数保持GN命名模式：

```c
static IotAHTTFrameHead_Struct *IotAHTT_FindValidFrameLen(
    uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen);
```

规则：

1. 在当前`pData/dataLen`范围搜索`0xEA`。
2. 声明长度小于11或总长大于512时推进1字节继续搜索。
3. 当前块中数据不足完整帧时，不解析该候选帧，`dealLen`按GN搜索过程累计。
4. 版本、设备号或CRC失败时推进1字节继续搜索。
5. 找到完整合法帧后，`dealLen`包含前导丢弃长度和该帧长度。
6. 未知命令或M1空解析回调时消费完整帧但不执行业务。
7. 有解析回调且返回TRUE时按控制项更新公共收发状态。

不使用`memmove()`保存尾部数据，不在AHTT上下文保存跨块半包。

- [x] **步骤5：实现FrameQueue解码回调**

```c
static void IotAHTT_DecodeData(uint8_t *pData, uint16_t dataLen,
    uint16_t topicLen, uint8_t *pTopic, uint16_t *pDealLen);
```

回调严格沿用`IotGN_DecodeData()`控制流：调用`IotAHTT_FindValidFrameLen()`，找到完整合法帧后查表、调用解析回调并更新公共收发状态；不自行把`dealLen`强制设置为`dataLen`。

- [x] **步骤6：实现公开入口**

```c
void IotAHTT_UpCtrlRecvDeal(void)
{
    FrameQueue_ProcessRxData(pIotAHTTCtx->frameQueueChannelID, IotAHTT_DecodeData);
}
```

`eGlobalRet_NotEnoughData`属于正常空队列结果，不打印高频错误。

- [x] **步骤7：实现超时骨架**

`IotAHTT_TimeoutDetect()`遍历23项接收表，但只处理`cmdType == RESPONSE`且`maxTimeout > 0`、`maxTryCnt > 0`的条目。M1所有条目超时为0，因此函数无业务动作。

### 任务5：建立M层状态机和生命周期边界

**文件：**

- 新建：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTM.c`
- 更新：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTM.h`

- [x] **步骤1：实现23命令运行控制槽位映射**

`IotAHTT_GetSendCtrl()`和`IotAHTT_GetRecvCtrl()`分别把23个命令映射到索引0～22。输入端口来自内部调用；M1保持GN控制流，不在getter重复增加端口判断。

- [x] **步骤2：实现设备号转换**

新增静态函数：

```c
static uint8_t IotAHTT_InitDeviceNum(void);
```

检查`platPileDn`长度恰好为10且每个字符为`'0'～'9'`，调用`Common_AsciiToBCD()`生成5字节顺序BCD，再逐字节逆序写入`pIotAHTTCtx->deviceNum`。

示例：

```text
3400300000 → 34 00 30 00 00 → 00 00 30 00 34
```

- [x] **步骤3：实现四态处理函数**

```text
Init：转Offline
Offline：清loginSucc、队列忙、发送索引、流水号和收发控制；重置FrameQueue；初始化设备号；设置平台离线故障；转Login
Login：TCP连接成功后转Normal；M1不使能0x01
Normal：链路断开时转Offline；链路存在时执行Send、Recv、Timeout；不运行业务周期
```

- [x] **步骤4：实现生命周期失败处理**

`IotAHTT_InitMemory()`：

```text
myMalloc(sizeof(IotAHTTCtx_Struct))
成功：清零并设置pFuncSendCtrl/pFuncRecvCtrl，reqSeq初始化为1
失败：保持pIotAHTTCtx为NULL，不访问任何成员
```

`IotAHTT_FillLinkPara()`：

```text
pLinkPara或pIotAHTTCtx为空：返回FALSE
FrameQueue_Creat失败：返回FALSE
成功：填写TCP IP、端口和channelID，返回TRUE
```

FrameQueue大小固定为TX 3072、RX 3072；AHTT单帧上限仍为512。

- [x] **步骤5：实现主函数和离线入口**

`IotAHTT_MainFunction()`严格使用GN四态switch。`IotAHTT_OfflineHandle()`调用`CddNetM_SetLinkDisconnect(eCddNetMPlatType_O)`、清登录标志并转Offline。

- [x] **步骤6：实现并声明M1安全占位业务回调**

按GN描述符接口声明以下AHTT同名函数：

```c
void IotAHTT_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode);
void IotAHTT_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason);
uint8_t IotAHTT_SwipCardCharge(uint8_t port, uint8_t *pCardID);
void IotAHTT_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord,
    uint8_t *pProtocolRecord, uint16_t *pRecordLen);
```

M1行为固定为：

```text
TransformBillMode：不修改输出
PackChargeRecord：不写订单结构
SwipCardCharge：返回FALSE
TransformChargeRecord：将*pRecordLen置0，不写协议记录
```

占位回调不得复制GN计费、订单或卡逻辑。

### 任务6：增加NVM类型和AHTT默认参数

**文件：**

- 修改：`02_App/Src/BSW/MemoryService/NVM/MS_NvmAppTypes.h`
- 更新：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTM.c/.h`

- [x] **步骤1：增加初版AHTT私有参数类型**

```c
typedef struct
{
    uint8_t heartCycleMin;
    uint8_t maxChargeTimeHour;
    uint8_t devOperationParam[8];
    uint8_t tempAlarmLimit;
}MSNvmAHTTWorkParam_Struct;

typedef struct
{
    MSNvmAHTTWorkParam_Struct stWorkParam;
}MSNvmAHTTParam_Struct;
```

`devOperationParam[8]`只保存`0x84`原始运维参数，M1不解释、不应用。M3按最终字段表将其替换为具名字段并执行NVM兼容评审。

- [x] **步骤2：增加初版AHTT订单类型**

```c
typedef struct
{
    uint8_t deviceNum[5];
    uint8_t port;
    uint16_t orderNo;
    uint8_t cardNo[5];
    uint32_t startTime;
    uint32_t stopTime;
    uint32_t totalEnergy;
    uint16_t remainMoney;
    uint16_t elecFee;
    uint16_t serviceFee;
    uint8_t stopReason;
}MSNvmAHTTOrderInfo_Struct;
```

M1只建立类型，不读写订单块。M4按`0x4D/0x4C/0x4B/0x93/0x94`最终字段补充并冻结布局。

- [x] **步骤3：加入两个联合体**

在`MSNvmPlatOrderInfo_Union`中增加：

```c
MSNvmAHTTOrderInfo_Struct stAHTTOrderInfo;
```

在`MSNvmPlatPrivateParam_Union`中增加：

```c
MSNvmAHTTParam_Struct stAHTTParam;
```

现有`userData[1280]`和`paramArr[1280]`保持不变，两个联合体总大小不得变化。

增加编译期容量检查：

```c
typedef char MSNvmAHTTOrderUnionSizeCheck[
    (sizeof(MSNvmPlatOrderInfo_Union) == MSNVM_ORDER_MAX_LEN) ? 1 : -1];
typedef char MSNvmAHTTPrivateUnionSizeCheck[
    (sizeof(MSNvmPlatPrivateParam_Union) == MSNVM_PLAT_PRIVATE_PARAM_LEN) ? 1 : -1];
```

- [x] **步骤4：实现默认初始化函数**

```c
void IotAHTT_SetPrivateParam(MSNvmPlatPrivateParam_Union *pPrivateParam)
{
    MSNvmAHTTParam_Struct *pAHTTParam = &pPrivateParam->stAHTTParam;

    memset(pAHTTParam, 0x00, sizeof(MSNvmAHTTParam_Struct));
    pAHTTParam->stWorkParam.heartCycleMin = 5;
    pAHTTParam->stWorkParam.maxChargeTimeHour = 10;
}
```

M1不为`0x84`和温度阈值编造默认值，保持0且不应用。

- [x] **步骤5：验证联合体大小**

Keil构建后确认：

```text
sizeof(MSNvmPlatOrderInfo_Union) == 1280
sizeof(MSNvmPlatPrivateParam_Union) == 1280
```

若不等于1280，停止实施，不修改NVM块长度。

### 任务7：平台注册、卡类型和工程接入

**文件：**

- 修改：`02_App/Src/ASW/ASW_COM/Asw_PlatM/Asw_PlatM.h`
- 修改：`02_App/Src/ASW/ASW_COM/Asw_PlatM/Asw_PlatMConfig.c`
- 修改：`02_App/Prj/Project.uvprojx`

- [x] **步骤1：追加平台枚举**

```c
    eAswPlatType_DXL,
    eAswPlatType_AHTT,
    eAswPlatType_Count,
```

不得改变`eAswPlatType_DXL`及之前枚举值。

- [x] **步骤2：追加卡类型枚举**

```c
    eAswPlatCardType_DXL,
    eAswPlatCardType_AHTT,
    eAswPlatCardType_Count,
```

- [x] **步骤3：注册M1平台描述符**

```c
[eAswPlatType_AHTT] =
{
    .pName = "ahtt",
    .cProtoMeaning = "安徽铁塔",
    .eSocketType = eCddNetMSocketType_TCP,
    .pFuncFillLinkPara = IotAHTT_FillLinkPara,
    .pFuncInit = IotAHTT_InitMemory,
    .pMainFunction = IotAHTT_MainFunction,
    .pFuncTransformBillMode = IotAHTT_TransformBillMode,
    .pFuncPackChargeRecord = IotAHTT_PackChargeRecord,
    .pFuncSwipCardCharge = IotAHTT_SwipCardCharge,
    .pFuncTransformChargeRecord = IotAHTT_TransformChargeRecord,
    .pFuncSetPrivateParam = IotAHTT_SetPrivateParam,
},
```

M1绑定AHTT刷卡占位回调；该函数固定返回`FALSE`，不会触发AHTT刷卡业务。

- [x] **步骤4：注册通用UUID卡描述符**

```c
[eAswPlatCardType_AHTT] =
{
    .pName = "ahtt",
    .cMeaning = "通用卡",
    .cardType = eCddCardType_UUID,
},
```

- [x] **步骤5：修改Keil工程**

include path末尾增加：

```text
..\Src\ASW\ASW_COM\Asw_IotProtocol\Protocol_AHTT
```

新增`Protocol_AHTT`分组并加入：

```text
Asw_IotProtoAHTTM.c
Asw_IotProtoAHTTRecv.c
Asw_IotProtoAHTTSend.c
```

- [x] **步骤6：执行Keil全量构建**

```powershell
& 'C:\Keil_v5\UV4\UV4.exe' -b '02_App\Prj\Project.uvprojx' -t 'D3_A32FB_GD32E503RE' -j0 -o '02_App\Prj\ahtt_m1_build.log'
```

预期日志包含：

```text
0 Error(s)
```

警告必须与修改前基线比较；新增AHTT相关警告必须清零。

### 任务8：M1本地、板端和文档闭环

**文件：**

- 更新：`docs/ahtt/AHTT-报文测试向量.md`
- 更新：`docs/ahtt/AHTT-V3.12-命令追踪矩阵.md`
- 更新：`docs/ahtt/AHTT-软件架构设计.md`
- 建议新建：`docs/ahtt/AHTT-板端验证清单.md`

- [x] **步骤1：执行独立向量脚本**

```powershell
powershell -ExecutionPolicy Bypass -File tools\ahtt\Validate-AHTTM1.ps1
```

预期：9通过、0失败。

- [x] **步骤2：执行影响面搜索**

```powershell
rg -n "eAswPlatType_AHTT|eAswPlatCardType_AHTT|IotAHTT_|MSNvmAHTT" 02_App\Src 02_App\Prj\Project.uvprojx
```

检查：

- AHTT枚举仅追加，不改变既有值。
- 所有新符号使用AHTT同构命名。
- 不存在旧项目接口、固定凭据或直接Flash调用。
- `Protocol_GN`和公共`FrameQueue`无改动。

- [ ] **步骤3：检查资源增量**

从Keil map和任务栈信息记录：

- `sizeof(IotAHTTCtx_Struct)`；
- FrameQueue动态申请6144字节；
- AHTT发送临时栈缓冲512字节；
- `Task_20msB`当前2048字节栈的最低余量；
- Flash增量；
- 单次粘包/错帧压力下最坏执行时间。

- [ ] **步骤4：板端烟雾验证**

板端配置：

```text
platMainType = ahtt
platMainCardType = ahtt
AuxiliaryPlatDisableFlag = 1
platPileDn = 10位十进制设备号
```

验证：

1. 上电不HardFault。
2. AHTT上下文和FrameQueue创建成功。
3. 4G链路使用配置IP/端口发起TCP连接。
4. M1不主动发送`0x01`或其他AHTT业务帧，四个业务占位回调不产生业务动作。
5. 断网后状态回到Offline并能重新连接。
6. 看门狗不复位，20ms任务持续运行。

- [ ] **步骤5：更新完成状态**

命令矩阵中的23个业务命令仍保持“待实现”；只把公共基础链路标记为“已实现/本地通过”或“板端通过”，不得把任何命令标记为完成。

## 5. 验收门禁

### 5.1 本地通过

- PowerShell向量9/9通过。
- Keil全量构建0错误，AHTT新增警告为0。
- AHTT帧头编译期大小为12字节。
- 两个NVM联合体大小保持1280字节。
- 影响面搜索确认GN、FrameQueue和其他协议无行为修改。
- 代码评审确认所有局部变量位于C函数开头声明区。

### 5.2 板端通过

- 平台选择AHTT后不HardFault。
- TCP链路可创建、断开和重建。
- M1不发送未实现业务命令。
- FrameQueue无越界、无持续增长；跨FrameQueue块半包不作为M1通过条件。
- 20ms任务和看门狗稳定。

### 5.3 M1不证明的内容

- 不证明安徽铁塔平台签到成功。
- 不证明心跳或任何首期业务命令正确。
- 不证明OTA、订单和NVM业务闭环。
- 不证明长期运行和平台压力验收。

## 6. 风险和回退边界

| 风险 | 控制方式 |
|---|---|
| FrameQueue跨TCP块半包丢失 | 经用户确认M1沿用GN，不增加私有缓存；记录为已知限制，后续平台抓包发现实际发生时重新设计 |
| 动态内存不足 | InitMemory与FrameQueue创建失败阻止链路创建，不解引用空指针 |
| 512字节单帧上限不足 | M1按首期及已知V3.12最大字段验证；扩展命令前重新计算 |
| 平台枚举影响旧NVM | 只追加在DXL之后，禁止插入既有项之间 |
| 初版AHTT NVM结构后续变化 | M1不实际写AHTT订单；M3/M4使用前冻结并做兼容评审 |
| M1选择AHTT但无签到 | 明确为框架烟雾阶段，M2再使能`0x01` |
| 运维平台未知AHTT编号 | M1板端关闭辅助平台，不修改`IotOM_GetPlatNo()` |

## 7. 实施确认

本计划包含代码和工程文件修改。收到用户明确回复“同意”“开始实施”或等效确认后，才允许执行任务1～8。实施过程中若需要修改公共`FrameQueue`、改变四态主状态机、扩大NVM块或增加首期外命令，必须停止并重新提交方案。

请确认是否按此方案执行？
