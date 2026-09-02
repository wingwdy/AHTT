# AHTT M2 签到与在线保持实施计划

> **Workflow Notice：** 本文中的历史 Agent / Sub-skill / Superpowers 执行指令不再作为当前 AHTT AI 工作流的权威来源。当前 Planner / Executor / Reviewer 角色与 Gate 统一以 `docs/ai/AHTT_AI_Development_Prompts.md` 和根目录 `AGENTS.md` 为准。本文继续保留技术方案、历史 checkbox 和验证证据价值。

> **For agentic workers:** REQUIRED SUB-SKILL: Use `test-driven-development` to implement this plan task-by-task. 如用户明确要求使用子代理，再使用 subagent-driven development；否则在当前会话按任务逐项实施并在检查点复核。

**Goal:** 在M1基础链路上实现`0x01`签到、`0x81`十二路心跳、基于命令和有效等待状态的应答关联、命令级超时重试，以及与当前GN/YKC/AP一致的默认断链重连闭环。

**Architecture:** 保持`Init → Offline → Login → Normal`四态主状态机和GN表驱动收发骨架。登录或心跳达到命令级重试上限后调用`IotAHTT_OfflineHandle()`断开运营平台链路，TCP重连完全交给现有`Cdd_NetM → Cdd_Drv_EG800AK → AT_TCP`处理；AHTT不新增阶梯休眠、不请求MCU重启、不修改网络驱动。

**Tech Stack:** C（ARMClang/Keil）、FreeRTOS 20ms平台任务、`FrameQueue`、`Cdd_NetM`、`CommonSendCtrl/CommonRecvCtrl`、PowerShell主机侧报文和状态推进测试。

---

## 1. 已确认决策

### 1.1 签到版本字节

当前软件版本为`V1.2.0.4`，签到沿用参考项目的两字节压缩方式：

```c
pBuf[dataLen++] = APP_SW_MINOR_VERSION;
pBuf[dataLen++] = APP_SW_CUSTORM_VERSION * 10 + APP_SW_PATCH_VERSION;
```

因此M2发送：

```text
第1字节 = 2
第2字节 = 0 × 10 + 4 = 4
报文字节 = 02 04
```

这是数值字节，不是ASCII`32 34`，也不是BCD。第一段`APP_SW_MAJOR_VERSION`不发送，这是用户确认的参考项目兼容规则。

### 1.2 签到失败处理

当前项目GN、YKC16、YKC21和AP均未实现登录阶梯休眠或登录失败后重启MCU。用户确认AHTT不新增协议私有退避，采用当前项目默认规则：

```text
登录请求立即发送
  → 等待10秒
  → 超时后立即重发
  → 再等待10秒
  → 再次超时后立即重发
  → 第3次等待仍超时
  → IotAHTT_OfflineHandle
  → CddNetM_SetLinkDisconnect
  → AT_TCP按5秒、10秒、30秒重连，后续保持30秒间隔
```

`maxTryCnt=3`沿用GN现有语义：超时计数达到3时结束，因此总发送次数为3次，即初次发送加2次重发。

### 1.3 网络层恢复边界

- TCP进入`WaitReconnect`后，第1次等待5秒、第2次等待10秒、第3次及以后等待30秒。
- `reconectTimes`上限为5；满足网络驱动现有条件时执行模组CFUN恢复，连续同类异常时可升级为模组重新初始化/掉电重启路径。
- 上述是4G模组恢复，不是MCU系统重启。
- AHTT只负责断开当前运营平台链路，不复制这些计时和恢复状态。

## 2. 范围与退出条件

### 2.1 本计划实现

- `0x01`：20字节ASCII ICCID、版本`02 04`、6字节电表地址全0。
- `0x01`同命令无参数应答：仅接受当前有效等待项；平台应答流水号独立于设备上行流水号，不进行两者相等比较。
- 登录成功后设置`loginSucc`、清平台离线故障并立即使能`0x81`。
- `0x81`：固定`0x02`表示4G、`CddNetM_GetCsq()`低8位、12通道2bit状态。
- 心跳周期从AHTT私有参数`heartCycleMin`读取，当前默认5分钟。
- 登录和心跳均采用当前平台通用的10秒、`maxTryCnt=3`超时规则。
- 登录或心跳最终超时后断链，由TCP驱动执行默认重连。
- 无有效等待项、命令不符、参数非法的应答不得推进登录状态或清除等待项；在同一命令仍处于有效等待期间，协议没有可用于区分迟到同命令应答的关联字段。

### 2.2 本计划不实现

- V3.12 4.1/4.6的5/10/15/20/25分钟协议私有退避。
- 签到失败后调用`AswMonitor_SetReboot()`或直接重启MCU。
- `0x02/0x03`心跳周期设置与查询；M2只读取现有默认值。
- `0x95`校时、`0x96`设备状态、订单补报和其他在线业务。
- 跨两个`FrameQueue`接收块的半包重组。
- 公共网络驱动、NVM结构和平台注册机制修改。

### 2.3 M2退出条件

- M1九项向量继续通过，M2新增向量全部通过。
- Keil目标`D3_A32FB_GD32E503RE`构建0错误，AHTT新增警告为0。
- 板端抓包确认签到版本为`02 04`。
- 登录/心跳各自总发送3次后断开TCP，随后重连间隔符合5/10/30秒默认规则。
- 重连期间20ms任务、看门狗和充电安全功能持续正常。
- 只将`0x01/0x81`更新到实际证据对应层级，不提前标记其他命令完成。

## 3. 协议依据与当前项目差异

| 项目 | V3.12 | 当前M2决策 |
|---|---|---|
| 签到参数 | ICCID 20、版本2、表地址6 | 按专节实现，版本使用`02 04` |
| 签到应答 | `0x01`无参数 | 长度0、命令匹配且当前等待使能视为成功；不比较平台流水号 |
| 签到频率 | 4.1/4.6存在1分钟连续签到及阶梯休眠描述 | 用户确认采用当前项目默认10秒×3后断链重连 |
| 心跳命令 | 当前产品选择12路`0x81` | 按DEC-005实现 |
| 心跳周期 | 默认5分钟 | 读取`heartCycleMin`，默认5 |
| 心跳超时 | 10秒，协议称重试3次 | 用户确认沿用当前平台`maxTryCnt=3`语义，总发送3次 |
| 最终恢复 | 协议描述重新签到或设备重启 | AHTT断链，交给现有TCP/模组恢复，不重启MCU |

此处是经过用户确认的产品实现差异，必须在命令矩阵和事实台账中记录，不能写成V3.12原文要求。

## 4. 文件边界

### 4.1 修改文件

| 文件 | 具体职责 |
|---|---|
| `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTTypes.h` | M2字段长度、状态和超时宏 |
| `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTM.h` | M层状态映射与登录成功接口声明 |
| `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTSend.c` | `0x01/0x81`组包、运行期心跳周期和发送表绑定 |
| `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTRecv.c` | 无参数应答解析、接收表参数和GN同构超时处理 |
| `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTM.c` | TCP成功触发签到、登录成功展开、通道状态映射 |
| `docs/ahtt/AHTT-报文测试向量.md` | 增加M2黄金报文和超时/重连向量 |
| `docs/ahtt/AHTT-V3.12-命令追踪矩阵.md` | 回写`0x01/0x81`实现与验证状态 |
| `docs/ahtt/AHTT-V3.12-事实台账.md` | 记录版本规则和默认重连产品决策 |
| `docs/ahtt/AHTT-软件架构设计.md` | 将M2退避口径更新为默认重连 |

### 4.2 新建文件

| 文件 | 具体职责 |
|---|---|
| `tools/ahtt/Validate-AHTTM2.ps1` | 验证签到/心跳字节、CRC、双向独立流水号应答关联和10秒×3状态推进 |

### 4.3 明确不修改

- `FrameQueue.c/.h`
- `Cdd_NetM.c/.h`
- `Cdd_Drv_EG800AK.c/.h/Config.h`
- `AT_TCP.c/.h`
- `Asw_PlatM.c/.h/Config.c`
- `MS_NvmAppTypes.h`及NVM块长度
- `Project.uvprojx`
- `Protocol_GN`和其他平台协议

## 5. GN→AHTT同构表

| GN符号 | AHTT符号 | 完全同构 | 差异原因 |
|---|---|---:|---|
| `IotGN_SendLoginReq` | `IotAHTT_SendLoginReq` | 否 | AHTT参数为20字节ICCID、2字节版本和6字节表地址 |
| `IotGN_SendHeartBeat` | `IotAHTT_SendHeartBeat` | 否 | AHTT为设备级12通道2bit位图 |
| `IotGN_RecvLoginRsp` | `IotAHTT_RecvLoginRsp` | 否 | AHTT同命令无参数应答 |
| `IotGN_RecvHeartBeatRsp` | `IotAHTT_RecvHeartBeatRsp` | 否 | AHTT无端口参数 |
| `IotGN_WSLoginHandle` | `IotAHTT_WSLoginHandle` | 是 | TCP成功后转Normal并使能登录 |
| `IotGN_WSOfflineHandle` | `IotAHTT_WSOfflineHandle` | 是 | 清协议状态后立即进入Login等待TCP |
| `IotGN_WSNormalHandle` | `IotAHTT_WSNormalHandle` | 是 | 收发和超时持续运行，在线业务受`loginSucc`约束 |
| `IotGN_TimeoutDetect` | `IotAHTT_TimeoutDetect` | 是 | 使用`>= maxTryCnt`，登录/心跳耗尽后Offline |
| `IotGN_OfflineHandle` | `IotAHTT_OfflineHandle` | 是 | 调用`CddNetM_SetLinkDisconnect`并切换Offline |
| `IotYKC21_GetGunState`最近职责 | `IotAHTT_GetGunState` | 部分 | 状态来源一致，AHTT编码为00/01/10 |

M2不再存在`IotAHTT_LoginGroupFailed`、`loginSleepFlag`、`loginSleepLevel`或`offlineEnterFlag`等非GN字段。

## 6. 数据流与状态转换

```text
TCP连接成功
  → IotAHTT_WSLoginHandle使能0x01并转Normal
  → IotAHTT_SendLoginReq组装ICCID/02 04/表地址
→ 发送设备侧流水号，启动10秒接收计时
  → 超时计数1：立即重发
  → 超时计数2：立即重发
  → 超时计数3：IotAHTT_OfflineHandle
  → CddNetM_SetLinkDisconnect
  → AT_TCP进入WaitReconnect并按5/10/30秒恢复
```

成功路径：

```text
合法0x01无参数应答
  → IotAHTT_LoginSuccess
  → loginSucc=TRUE
  → 清平台离线故障
  → 立即使能0x81
  → 后续按heartCycleMin周期发送
  → 心跳10秒×3耗尽时走同一Offline/默认重连路径
```

## 7. 分任务实施步骤

### Task 1：建立失败的M2主机侧向量

**Files:**
- Create: `tools/ahtt/Validate-AHTTM2.ps1`
- Modify: `docs/ahtt/AHTT-报文测试向量.md`

- [x] **Step 1：复用M1的CRC和帧构造函数**

保留`Get-Crc16Modbus`与`New-AhttFrame`，新增：

```powershell
function New-AhttLoginParameter
{
    param([string]$Iccid)

    if ($Iccid.Length -ne 20)
    {
        throw 'ICCID length must be 20'
    }

    $parameter = New-Object System.Collections.Generic.List[byte]
    $parameter.AddRange([Text.Encoding]::ASCII.GetBytes($Iccid))
    $parameter.Add(2)
    $parameter.Add(4)
    $parameter.AddRange([byte[]](0, 0, 0, 0, 0, 0))
    return $parameter.ToArray()
}

function New-AhttHeartbeatParameter
{
    param([byte]$Csq, [byte[]]$ConfiguredStates)

    $stateBytes = [byte[]](0xFF, 0xFF, 0xFF)
    for ($channel = 0; $channel -lt $ConfiguredStates.Length; $channel++)
    {
        $byteIndex = [int]($channel / 4)
        $bitPos = ($channel % 4) * 2
        $stateBytes[$byteIndex] = [byte](($stateBytes[$byteIndex] -band
            (-bnot (3 -shl $bitPos))) -bor ($ConfiguredStates[$channel] -shl $bitPos))
    }

    return [byte[]](0x02, $Csq, $stateBytes[0], $stateBytes[1], $stateBytes[2])
}
```

- [x] **Step 2：加入字节级向量**

| 向量 | 输入 | 精确断言 |
|---|---|---|
| M2-VEC-001 | ICCID`89860416121980004151`、设备号`00 00 40 00 34`、流水号1 | 完整帧`EA2700010000400034010001383938363034313631323139383030303431353102040000000000003F70` |
| M2-VEC-002 | 设备流水号1、平台流水号`0x0587`的`0x01`无参数应答 | 登录成功只推进一次 |
| M2-VEC-003 | `0x01`应答多1字节参数 | 解析失败，等待计时不清除 |
| M2-VEC-004 | 无有效等待后的重复应答、命令不符的`0x01`应答 | 不重复推进登录状态；有效等待期间不以平台流水号拒绝应答 |
| M2-VEC-005 | 单端口空闲、CSQ=`0x0C`、流水号2 | `EA1000010000400034020081020CFCFFFF05F1` |
| M2-VEC-006 | 单端口工作、CSQ=`0x0C`、流水号2 | `EA1000010000400034020081020CFDFFFF5431` |
| M2-VEC-007 | 单端口故障、CSQ=`0x0C`、流水号2 | `EA1000010000400034020081020CFEFFFFA431` |
| M2-VEC-008 | 12通道`00/01/10/11`循环 | 每个状态位于`ch/4`字节和`(ch%4)*2`位 |

- [x] **Step 3：加入默认重试状态模型**

```powershell
$commandTimeoutMs = 10000
$maxTryCount = 3
$tcpReconnectIntervalsMs = @(5000, 10000, 30000, 30000, 30000)
```

断言登录和心跳均为总发送3次；第3次等待超时后调用Offline；TCP等待序列为5/10/30/30/30秒；协议层没有休眠级别和MCU重启事件。

- [x] **Step 4：运行并确认先失败**

```powershell
powershell -ExecutionPolicy Bypass -File tools\ahtt\Validate-AHTTM2.ps1
```

Expected: 脚本语法正确，但C源码一致性检查因M2函数尚未实现而失败。

### Task 2：增加M2常量和M层接口

**Files:**
- Modify: `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTTypes.h`
- Modify: `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTM.h`

- [x] **Step 1：增加精确宏**

```c
#define IOT_AHTT_LOGIN_PARAM_LEN               (28)
#define IOT_AHTT_METER_ADDR_LEN                (6)
#define IOT_AHTT_HEART_PARAM_LEN               (5)
#define IOT_AHTT_HEART_CHANNEL_COUNT           (12)
#define IOT_AHTT_HEART_STATE_LEN               (3)
#define IOT_AHTT_HEART_NET_4G                  (0x02)
#define IOT_AHTT_HEART_STATE_IDLE              (0x00)
#define IOT_AHTT_HEART_STATE_WORK              (0x01)
#define IOT_AHTT_HEART_STATE_FAULT             (0x02)
#define IOT_AHTT_HEART_STATE_OFFLINE           (0x03)
#define IOT_AHTT_LOGIN_TIMEOUT_MS              (10000)
#define IOT_AHTT_LOGIN_MAX_TRY_COUNT           (3)
#define IOT_AHTT_HEART_TIMEOUT_MS              (10000)
#define IOT_AHTT_HEART_MAX_TRY_COUNT           (3)
#define IOT_AHTT_MINUTE_MS                     (60000)
```

- [x] **Step 2：增加M层协作接口，不增加上下文字段**

```c
uint8_t IotAHTT_GetGunState(uint8_t port);
void IotAHTT_LoginSuccess(void);
```

### Task 3：实现`0x01`和`0x81`组包

**Files:**
- Modify: `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTSend.c`

- [x] **Step 1：加入依赖和静态声明**

```c
#include "Version.h"

static uint16_t IotAHTT_SendLoginReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotAHTT_SendHeartBeat(uint8_t port, uint8_t *pBuf);
```

- [x] **Step 2：绑定发送表**

```c
{IOT_AHTT_CMD_LOGIN, IOT_AHTT_CMDTYPE_REQUSET, 0,
 IotAHTT_SendLoginReq, IOT_AHTT_CMD_LOGIN, TRUE, "签到"},

{IOT_AHTT_CMD_HEARTBEAT, IOT_AHTT_CMDTYPE_REQUSET, 0,
 IotAHTT_SendHeartBeat, IOT_AHTT_CMD_HEARTBEAT, FALSE, "心跳上报"},
```

- [x] **Step 3：实现签到参数**

```c
static uint16_t IotAHTT_SendLoginReq(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    (void)port;
    CddNetM_GetIccid(&pBuf[dataLen]);
    dataLen += IOT_AHTT_SIM_NUM_LEN;
    pBuf[dataLen++] = APP_SW_MINOR_VERSION;
    pBuf[dataLen++] = APP_SW_CUSTORM_VERSION * 10 + APP_SW_PATCH_VERSION;
    memset(&pBuf[dataLen], 0x00, IOT_AHTT_METER_ADDR_LEN);
    dataLen += IOT_AHTT_METER_ADDR_LEN;

    return dataLen;
}
```

- [x] **Step 4：实现十二路心跳参数**

```c
static uint16_t IotAHTT_SendHeartBeat(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    uint16_t csq = 0;
    uint8_t channel = 0;
    uint8_t byteIndex = 0;
    uint8_t bitPos = 0;
    uint8_t channelState = 0;
    uint8_t stateMask = 0;

    (void)port;
    pBuf[dataLen++] = IOT_AHTT_HEART_NET_4G;
    csq = CddNetM_GetCsq();
    pBuf[dataLen++] = (uint8_t)(csq & 0xFF);
    memset(&pBuf[dataLen], 0xFF, IOT_AHTT_HEART_STATE_LEN);

    for (channel = 0;
        (channel < SYSCFG_CFG_GUN_NUM) && (channel < IOT_AHTT_HEART_CHANNEL_COUNT);
        channel++)
    {
        channelState = IotAHTT_GetGunState(channel);
        byteIndex = channel / 4;
        bitPos = (channel % 4) * 2;
        stateMask = (uint8_t)(0x03 << bitPos);
        pBuf[dataLen + byteIndex] &= (uint8_t)(~stateMask);
        pBuf[dataLen + byteIndex] |= (uint8_t)(channelState << bitPos);
    }

    dataLen += IOT_AHTT_HEART_STATE_LEN;
    return dataLen;
}
```

- [x] **Step 5：运行期读取心跳周期**

在`IotAHTT_UpCtrlSendDeal`函数开头变量区增加：

```c
MSNvmPlatPrivateParam_Union *pPrivateParam = NULL;
uint32_t sendCycle = 0;
```

取得发送表项后：

```c
sendCycle = pCmdSendCtrl->sendCycle;
if (IOT_AHTT_CMD_HEARTBEAT == pCmdSendCtrl->cmd)
{
    pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    sendCycle = pPrivateParam->stAHTTParam.stWorkParam.heartCycleMin *
        IOT_AHTT_MINUTE_MS;
}
```

把`IotAHTT_ReportCycleCheck`调用的周期参数改为`sendCycle`。

### Task 4：实现无参数应答和GN同构超时

**Files:**
- Modify: `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTRecv.c`

- [x] **Step 1：声明并绑定解析函数**

```c
static uint8_t IotAHTT_RecvLoginRsp(uint8_t *port, uint8_t *pData, uint16_t len);
static uint8_t IotAHTT_RecvHeartBeatRsp(uint8_t *port, uint8_t *pData, uint16_t len);
```

```c
{IOT_AHTT_CMD_LOGIN, IOT_AHTT_CMDTYPE_RESPONSE,
 IotAHTT_RecvLoginRsp, IOT_AHTT_LOGIN_TIMEOUT_MS, IOT_AHTT_LOGIN_MAX_TRY_COUNT,
 IOT_AHTT_CMD_LOGIN, TRUE, "签到应答"},

{IOT_AHTT_CMD_HEARTBEAT, IOT_AHTT_CMDTYPE_RESPONSE,
 IotAHTT_RecvHeartBeatRsp, IOT_AHTT_HEART_TIMEOUT_MS, IOT_AHTT_HEART_MAX_TRY_COUNT,
 IOT_AHTT_CMD_HEARTBEAT, FALSE, "心跳应答"},
```

- [x] **Step 2：严格解析无参数应答**

```c
static uint8_t IotAHTT_RecvLoginRsp(uint8_t *port, uint8_t *pData, uint16_t len)
{
    uint8_t ret = FALSE;

    (void)port;
    (void)pData;
    if (0 == len)
    {
        IotAHTT_LoginSuccess();
        ret = TRUE;
    }

    return ret;
}

static uint8_t IotAHTT_RecvHeartBeatRsp(uint8_t *port, uint8_t *pData, uint16_t len)
{
    uint8_t ret = FALSE;

    (void)port;
    (void)pData;
    if (0 == len)
    {
        ret = TRUE;
    }

    return ret;
}
```

- [x] **Step 3：按GN控制流补全超时函数**

```c
void IotAHTT_TimeoutDetect(void)
{
    const IotAHTTRecvCtrl_Struct *pCmdRecvCtrl = NULL;
    uint8_t index = 0;
    uint8_t port = 0;
    uint8_t timeoutCount = 0;

    for (index = 0; index < IOT_AHTT_CMD_RECV_COUNT; index++)
    {
        pCmdRecvCtrl = &c_stIotAHTTRecvctrlTable[index];
        if ((IOT_AHTT_CMDTYPE_RESPONSE != pCmdRecvCtrl->cmdType) ||
            (0 == pCmdRecvCtrl->maxTimeout))
        {
            continue;
        }

        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            if (TRUE != Common_GetRecvTimerEnable(pIotAHTTCtx->pFuncRecvCtrl,
                port, pCmdRecvCtrl->cmd))
            {
                continue;
            }

            if (TRUE == Common_JudgeTimeoutMs(Common_GetRecvTick(pIotAHTTCtx->pFuncRecvCtrl,
                port, pCmdRecvCtrl->cmd), pCmdRecvCtrl->maxTimeout))
            {
                Common_SetRptCount(pIotAHTTCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                timeoutCount = Common_GetRptCount(pIotAHTTCtx->pFuncRecvCtrl,
                    port, pCmdRecvCtrl->cmd);

                if (timeoutCount >= pCmdRecvCtrl->maxTryCnt)
                {
                    if ((IOT_AHTT_CMD_LOGIN == pCmdRecvCtrl->cmd) ||
                        (IOT_AHTT_CMD_HEARTBEAT == pCmdRecvCtrl->cmd))
                    {
                        IotAHTT_OfflineHandle();
                    }
                }
                else
                {
                    Common_SetRecvTimerEnable(pIotAHTTCtx->pFuncRecvCtrl,
                        port, pCmdRecvCtrl->cmd, FALSE);
                    Common_SetSendEnable(pIotAHTTCtx->pFuncSendCtrl,
                        port, pCmdRecvCtrl->matchCmd, TRUE);
                    Common_SetSendImmdFlag(pIotAHTTCtx->pFuncSendCtrl,
                        port, pCmdRecvCtrl->matchCmd, TRUE);
                    Common_SetSendFlag(pIotAHTTCtx->pFuncSendCtrl,
                        port, pCmdRecvCtrl->matchCmd, FALSE);
                }
            }
        }
    }
}
```

M2只有登录和心跳配置非0超时；后续命令加入超时时，应按各自最终失败动作扩展非关键命令分支，不能统一断链。

### Task 5：实现登录状态展开和心跳状态映射

**Files:**
- Modify: `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTM.c`

- [x] **Step 1：实现AHTT心跳状态来源**

```c
uint8_t IotAHTT_GetGunState(uint8_t port)
{
    uint8_t gunState = IOT_AHTT_HEART_STATE_IDLE;

    if (AswErrHandle_IsExsistError(port) == TRUE)
    {
        gunState = IOT_AHTT_HEART_STATE_FAULT;
    }
    else if (AswMonitor_IsOrderIdle(port) != TRUE)
    {
        gunState = IOT_AHTT_HEART_STATE_WORK;
    }
    else
    {
    }

    return gunState;
}
```

- [x] **Step 2：实现登录成功展开**

```c
void IotAHTT_LoginSuccess(void)
{
    pIotAHTTCtx->loginSucc = TRUE;
    AswErrhandle_ResetErrExsitCallback(0, eErr_PlatformOffline);
    Common_SetSendEnable(pIotAHTTCtx->pFuncSendCtrl, 0,
        IOT_AHTT_CMD_HEARTBEAT, TRUE);
    Common_SetSendImmdFlag(pIotAHTTCtx->pFuncSendCtrl, 0,
        IOT_AHTT_CMD_HEARTBEAT, TRUE);
}
```

- [x] **Step 3：TCP连接后立即触发签到**

```c
static void IotAHTT_WSLoginHandle(void)
{
    if (TRUE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        pIotAHTTCtx->eWorkState = eIOTAHTTWorkState_Normal;
        Common_SetSendEnable(pIotAHTTCtx->pFuncSendCtrl, 0,
            IOT_AHTT_CMD_LOGIN, TRUE);
    }
}
```

`IotAHTT_WSOfflineHandle`和`IotAHTT_OfflineHandle`保持M1现有实现，不增加休眠或重启字段。

### Task 6：运行主机侧验证

**Files:**
- Test: `tools/ahtt/Validate-AHTTM1.ps1`
- Test: `tools/ahtt/Validate-AHTTM2.ps1`

- [x] **Step 1：运行M1回归**

```powershell
powershell -ExecutionPolicy Bypass -File tools\ahtt\Validate-AHTTM1.ps1
```

Expected: `AHTT M1 vectors: 9 passed, 0 failed`。

- [x] **Step 2：运行M2向量**

```powershell
powershell -ExecutionPolicy Bypass -File tools\ahtt\Validate-AHTTM2.ps1
```

Expected: 所有M2向量通过，退出码0。

- [x] **Step 3：静态影响面检查**

```powershell
rg -n "IotAHTT_SendLoginReq|IotAHTT_SendHeartBeat|IotAHTT_RecvLoginRsp|IotAHTT_RecvHeartBeatRsp|IotAHTT_LoginSuccess" 02_App\Src\ASW\ASW_COM\Asw_IotProtocol\Protocol_AHTT
rg -n "LoginGroupFailed|loginSleep|offlineEnterFlag|AswMonitor_SetReboot" 02_App\Src\ASW\ASW_COM\Asw_IotProtocol\Protocol_AHTT
```

Expected: 第一条能定位M2符号；第二条无输出。

### Task 7：Keil全量构建和资源检查

**Files:**
- Build: `02_App/Prj/Project.uvprojx`
- Inspect: `02_App/Prj/ahtt_m2_build.log`

- [x] **Step 1：构建目标**

```powershell
& 'C:\Keil_v5\UV4\UV4.exe' -b '02_App\Prj\Project.uvprojx' -t 'D3_A32FB_GD32E503RE' -j0 -o '02_App\Prj\ahtt_m2_build.log'
```

Expected: `0 Error(s)`，AHTT新增警告为0。

- [x] **Step 2：检查文件边界**

```powershell
git diff --name-only
git diff --check
```

源码只能修改第4.1节列出的5个AHTT文件。若需要修改网络驱动、公共`FrameQueue`、NVM或工程文件，停止并重新提交方案。

- [ ] **Step 3：记录资源变化**

记录Flash/RAM增量、`sizeof(IotAHTTCtx_Struct)`、20ms任务栈最低余量和超时循环最坏执行时间。本方案不增加AHTT上下文字段，结构体大小原则上应保持M1值。

### Task 8：板端和平台联调

**Files:**
- Modify or create after confirmation: `docs/ahtt/AHTT-板端验证清单.md`
- Modify or create after confirmation: `docs/ahtt/AHTT-联调问题记录.md`

- [ ] **Step 1：验证正常签到**

TCP连接后抓取首帧，确认命令`0x01`、ICCID20字节、版本`02 04`、表地址6字节全0、设备侧流水号和CRC正确。合法的独立平台流水号应答后立即出现`0x81`。

- [ ] **Step 2：验证心跳状态**

分别构造空闲、充电和故障，确认端口1状态为`00/01/10`，端口2～12保持`11`；CSQ等于`CddNetM_GetCsq()`低8位。

状态快照（2026-09-02）：空闲`FC FF FF`和故障`FE FF FF`连同平台无参数应答已实测；充电功能尚未实现，因此本Task仍保持未完成。两帧报文中的CSQ均为`0x1B`，待与`CddNetM_GetCsq()`运行期打印值交叉确认。

- [x] **Step 3：验证命令级超时**

丢弃登录或心跳应答，确认约0秒、10秒、20秒共发送3帧；约30秒进入Offline并关闭当前TCP。无有效等待项时到达的重复或迟到应答不得恢复登录。

板端证据（2026-09-02）：Bore心跳静默后，`0x81`于`10:59:45.287`、`10:59:55.312`、`11:00:05.337`发送，`11:00:15.363`进入Offline，并于`11:00:15.384`执行`AT+QICLOSE=0`。旧应答注入仍作为T8-11保留在板端清单中。

- [x] **Step 4：验证默认TCP重连**

持续拒绝TCP连接，确认日志依次出现约5秒、10秒、30秒重连，后续维持30秒；AHTT不输出5/10/15/20分钟休眠，不请求MCU重启。

板端证据（2026-09-02）：Bore停止后，失败建连实测5.060/10.060/30.060/30.060秒退避；连续5次失败后发生CFUN或EG800AK模组恢复，符合第2节“网络层模组恢复、非MCU重启”的现有策略。

- [ ] **Step 5：验证断网恢复和唯一连接**

断网后恢复，确认旧运营平台Socket先关闭，恢复后重新签到；同时最多存在一个有效运营平台连接；20ms任务和看门狗持续正常。

板端证据（2026-09-02）：恢复Bore后，模组恢复并重新`QIOPEN`、签到和发送心跳；每次重连前日志均有`AT+QICLOSE=0`。20ms任务、看门狗和并发Socket计数尚未留存证据，因此本步骤保持未完成。

### Task 9：文档回写

**Files:**
- Modify: `docs/ahtt/AHTT-报文测试向量.md`
- Modify: `docs/ahtt/AHTT-V3.12-命令追踪矩阵.md`
- Modify: `docs/ahtt/AHTT-V3.12-事实台账.md`
- Modify: `docs/ahtt/AHTT-软件架构设计.md`

- [ ] **Step 1：记录产品决策**

明确记录“版本`02 04`为参考项目兼容规则”“AHTT不实现V3.12阶梯休眠，采用当前项目命令级10秒×3和TCP 5/10/30秒默认重连”。

- [ ] **Step 2：记录实际验证证据**

保存测试脚本通过数、Keil构建结果、板端日志和平台抓包。没有真实平台抓包时不得标记平台通过。

- [ ] **Step 3：更新命令状态**

只更新`0x01/0x81`；其他21项继续保持待实现状态。

## 8. 风险和兼容边界

| 风险 | 控制方式 |
|---|---|
| 版本压缩丢失主版本 | 用户已确认参考项目兼容规则；黄金帧锁定`02 04` |
| 平台期望`01 02` | 板端首轮平台联调重点核对；不静默兼容两种格式 |
| 当前默认重试与V3.12文字不同 | 作为用户确认的产品差异写入事实台账和命令矩阵 |
| 重连过于频繁 | 复用网络层5/10/30秒成熟规则，不在协议层叠加第二套计时器 |
| 多层同时重连 | AHTT仅调用一次`CddNetM_SetLinkDisconnect`；后续状态归网络层管理 |
| 未配置通道误报空闲 | 心跳状态先填`FF FF FF`，再覆盖实际端口 |
| 重复应答误推进状态 | 仅在有效等待项存在时接受同命令、零参数应答；成功后立即关闭等待项。协议不提供平台应答与设备请求的同序号关联字段。 |
| 模组恢复误认为整机重启 | 验证日志区分TCP重连、CFUN和MCU复位源 |
| M3修改心跳周期 | M2运行期读取`heartCycleMin`，后续无需重构发送表 |

## 9. 代码修改确认边界

本文件是更新后的实施方案，不代表已授权修改代码。执行Task 1～9前仍需用户明确回复“确认方案并开始实施”或等效表述。

实施中出现以下任一情况必须停止并重新提交方案：

- 平台明确要求`01 02`或其他版本字节；
- 平台要求执行V3.12阶梯退避或签到失败后重启MCU；
- 需要修改公共网络驱动、`FrameQueue`、NVM结构或工程文件；
- 平台签到应答出现未定义结果字段；
- 平台要求`0x80`或按端口数动态选择心跳；
- 需要启用M2范围外命令。

### 方案：M2签到与在线保持（默认重连版）
**目标**: 完成`02 04`版本签到、十二路心跳、10秒×3命令重试和5/10/30秒TCP默认重连闭环。
**改动文件**: 第4节列出的5个AHTT源码文件、1个测试脚本及4个追踪文档。

#### 改动详情:
先建立失败向量，再按GN同构方式实现Send、Recv和M，最后执行M1回归、M2向量、Keil构建、板端重连及文档门禁；不新增退避状态，不修改网络驱动，不重启MCU。

请确认是否按此更新后的方案执行？
