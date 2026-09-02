# AHTT 平台独立流水号修复实施计划

> **给执行者：** 实施时必须逐项执行红灯验证、最小修改、Keil 编译和板端验证；不得把本方案扩展为公共协议框架重构。

**目标：** 使 AHTT 接收层按照协议规定处理设备与服务器各自独立的帧流水号，正确接受平台签到和心跳应答，同时保留服务器请求应答时的流水号回送能力。

**架构：** AHTT 设备主动上报使用 `pIotAHTTCtx->reqSeq` 生成本地方向流水号；平台下发使用平台自己的流水号。设备主动上报的应答按“合法帧、相同指令、对应接收定时器正在等待”确认，不再比较两个方向的帧流水号；服务器主动请求的流水号仍保存到接收控制块，供设备应答组包时原样回送。当前目标为单枪配置，未来增加多枪应答解析时，必须由各指令参数解析函数确定枪口，不能恢复帧流水号匹配。

**技术栈：** 嵌入式 C、FreeRTOS 20 ms 周期任务、AHTT TCP 私有协议、Keil MDK ARM Compiler 6。

---

## 文件结构与职责

- 修改：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTRecv.c`
  - 负责合法帧校验、按指令分派、等待状态确认、平台请求流水号保存和应答成功后的控制块清理。
- 修改：`02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTSend.c`
  - 负责设备本地方向流水号生成，以及服务器请求应答时读取服务器流水号。
- 不修改：`02_App/Src/Common/Common.c`
  - 公共收发控制接口继续保留，不影响 GN、OM、XDT 等协议。
- 参考：`reference/参考项目/D3-A32EB_SCU/iotModule_user/Src/Asw/ProtoLayer/iot_AHTT_Protocol_Code.c`
  - 参考项目的 `AHTTUpCtrlRecvDeal()` 按指令调用解析函数，不比较平台帧流水号与设备帧流水号。

### Task 1：建立失败样例和验收样例

**验证输入：**

```text
设备签到：EA 27 00 01 01 00 90 05 34 01 00 01 ... 10 69
平台应答：EA 0B 00 01 01 00 90 05 34 87 04 01 BB D5
```

- [ ] **步骤 1：记录修改前失败条件**

确认当前代码保存的设备等待流水号为 `1`，平台应答流水号解析为 `0x0487`，以下判断为假：

```c
recvSeq == Common_GetRecvSeq(pIotAHTTCtx->pFuncRecvCtrl,
    matchedPort, pCmdRecvCtrl->cmd)
```

预期结果：`responseMatched` 保持 `FALSE`，日志出现“签到应答处理失败”。

- [ ] **步骤 2：固定验收条件**

同一合法应答在接收定时器已开启时必须被接受；若对应指令的接收定时器未开启，则该应答仍必须被拒绝，避免无等待状态的旧报文改变协议状态。

### Task 2：按指令和等待状态匹配平台应答

**文件：** `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTRecv.c:132`

- [ ] **步骤 1：删除跨方向流水号比较**

修改前：

```c
if ((TRUE == Common_GetRecvTimerEnable(pIotAHTTCtx->pFuncRecvCtrl,
    matchedPort, pCmdRecvCtrl->cmd)) &&
    (recvSeq == Common_GetRecvSeq(pIotAHTTCtx->pFuncRecvCtrl,
    matchedPort, pCmdRecvCtrl->cmd)))
{
    port = matchedPort;
    responseMatched = TRUE;
    break;
}
```

修改后：

```c
if (TRUE == Common_GetRecvTimerEnable(pIotAHTTCtx->pFuncRecvCtrl,
    matchedPort, pCmdRecvCtrl->cmd))
{
    port = matchedPort;
    responseMatched = TRUE;
    break;
}
```

这样签到和心跳应答由“指令 + 正在等待”关联，平台的独立流水号不会导致误拒绝。

- [ ] **步骤 2：平台请求沿用已解析的流水号**

保留函数开头的 `recvSeq` 声明和第 130 行的统一解析。平台主动请求处理成功后，把原来的重复转换改为直接使用 `recvSeq`：

```c
Common_SetRecvSeq(pIotAHTTCtx->pFuncRecvCtrl, port,
    pFrameHead->cmd, recvSeq);
```

该值只用于服务器请求的设备应答组包，不用于匹配设备主动上报的服务器应答。

- [ ] **步骤 3：检查成功清理路径**

确认签到应答通过后仍执行：

```c
Common_SetRecvTimerEnable(pIotAHTTCtx->pFuncRecvCtrl,
    port, pFrameHead->cmd, FALSE);
Common_ClearRptCount(pIotAHTTCtx->pFuncRecvCtrl,
    port, pCmdRecvCtrl->cmd);
Common_SetSendFlag(pIotAHTTCtx->pFuncSendCtrl,
    port, pCmdRecvCtrl->matchCmd, FALSE);
```

预期结果：签到不再每隔 10 秒重发，三次超时关闭连接的路径不会触发。

### Task 3：停止保存设备请求流水号作为应答匹配键

**文件：** `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTSend.c:211`

- [ ] **步骤 1：删除设备请求发送后的 `Common_SetRecvSeq()`**

删除：

```c
if (IOT_AHTT_CMD_NULL != pCmdSendCtrl->matchCmd)
{
    Common_SetRecvSeq(pIotAHTTCtx->pFuncRecvCtrl, port,
        pCmdSendCtrl->matchCmd, reqSeq);
}
```

保留下方本地方向流水号递增：

```c
pIotAHTTCtx->reqSeq = (reqSeq >= IOT_AHTT_SEQ_MAX) ?
    IOT_AHTT_SEQ_MIN : (reqSeq + 1);
```

原因：平台不会回显该值，保存它会误导后续维护者再次把它用于响应匹配。

- [ ] **步骤 2：保留服务器请求应答的流水号读取**

设备发送 `IOT_AHTT_CMDTYPE_RESPONSE` 报文时，继续使用：

```c
reqSeq = (uint16_t)Common_GetRecvSeq(pIotAHTTCtx->pFuncRecvCtrl,
    port, pCmdSendCtrl->matchCmd);
```

该值来自 Task 2 保存的平台请求流水号，保证设备对平台主动请求的应答携带相同流水号。

### Task 4：源级影响面检查

- [ ] **步骤 1：搜索 AHTT 流水号读写点**

运行：

```powershell
rg -n "Common_SetRecvSeq|Common_GetRecvSeq|recvSeq|responseMatched" 02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT
```

预期：

- 设备请求发送路径不再保存本地请求流水号；
- 平台请求接收路径仍保存平台流水号；
- 设备应答发送路径仍读取平台流水号；
- 平台应答确认路径不再调用 `Common_GetRecvSeq()`。

- [ ] **步骤 2：检查参考项目一致性**

确认当前实现与参考项目保持以下行为一致：

- 合法帧按 `cmd` 分派；
- 签到/心跳不依赖对端回显帧流水号；
- 枪口相关指令由参数解析函数确定枪口；
- 超时仍由每个枪口、每个指令的接收控制块管理。

- [ ] **步骤 3：检查 C 代码规范**

确认没有新增中间语句块局部变量，没有新增带 `U`、`L`、`UL`、`ULL` 后缀的数值字面量，也没有修改与 AHTT 无关的既有代码。

### Task 5：编译和板端验证

- [ ] **步骤 1：执行 Keil 完整编译**

目标：`D3_A32FB_GD32E503RE`

预期：

```text
0 Error(s), 0 Warning(s)
```

- [ ] **步骤 2：验证当前签到样例**

板端应出现一次签到发送和一次签到成功接收：

```text
AHTT,[枪：0]发送[cmd: 0x01, 签到]
AHTT,[枪：0]接收[cmd: 0x01, 签到应答]
```

应同时满足：

- `pIotAHTTCtx->loginSucc == TRUE`；
- 签到接收定时器关闭；
- 不再在 10 秒后发送第二次签到；
- TCP 不再因三次签到超时主动关闭；
- 登录成功后立即启用心跳。

- [ ] **步骤 3：验证独立平台流水号**

使用平台应答流水号 `0x0487`，设备请求流水号保持 `1`。预期签到仍成功，证明修复不依赖两个方向流水号相等。

- [ ] **步骤 4：验证无等待应答保护**

在签到接收定时器已关闭后再次注入相同 `0x01` 应答。预期报文不进入登录成功处理，不重复清理控制块。

- [ ] **步骤 5：源级验证平台请求应答流水号回送路径**

当前 AHTT 控制表中的服务器主动请求解析函数和对应设备应答组包函数尚未实现，不能构造真实板端闭环。因此本次采用源级路径检查：

```text
平台请求帧流水号
  -> IotAHTT_DecodeData() 的 recvSeq
  -> Common_SetRecvSeq()
  -> IotAHTT_UpCtrlSendDeal() 的 Common_GetRecvSeq()
  -> IotAHTT_PackHead()
```

预期：平台下发流水号例如 `0x0488` 时，保存值为 `0x0488`；对应应答功能后续实现后，组包字节应为 `88 04`。本次不把尚未实现的业务指令伪装成已完成的板端验证。

## 多枪扩展约束

当前 `SYSCFG_CFG_GUN_NUM` 为 1，因此按接收定时器选择等待枪口没有歧义。后续实现刷卡鉴权、订单、实时数据等多枪应答时，应参照参考项目让 `pRecvParse()` 从通道号或单号中设置 `port`，并在访问数组前检查：

```c
if (port < SYSCFG_CFG_GUN_NUM)
{
    /* 访问对应枪口控制块 */
}
```

不得照抄参考项目中“先用枪口索引数组、后检查范围”的不安全顺序，也不得重新使用平台帧流水号推断枪口。

## 不在本次范围内

- 不修改 AHTT CRC 字节序；该问题已经单独修复。
- 不修改 `CommonRecvCtrl_Struct` 或公共 `Common_*` 接口。
- 不修改 GN、OM、XDT 等其他平台协议。
- 不实现当前控制表中尚为空的 AHTT 业务解析函数。
- 不调整签到、心跳超时周期和最大重试次数。
