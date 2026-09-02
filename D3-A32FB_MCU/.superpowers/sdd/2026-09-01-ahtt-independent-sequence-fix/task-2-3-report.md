# Task 2 与 Task 3：AHTT 独立流水号收发修复报告

## 状态

完成。仅修改了任务指定的两个 AHTT 源文件；未提交、未暂存，未修改构建产物。

## 修改内容

1. `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTRecv.c`
   - 平台应答匹配仅保留“对应命令接收定时器已开启”的条件，删除 `recvSeq == Common_GetRecvSeq(...)` 比较。
   - 平台主动请求解析成功后，将已解析的 `recvSeq` 保存到接收控制项，使发送侧设备应答能原样使用平台请求帧的流水号。
2. `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTSend.c`
   - 删除设备主动请求成功入队后写入 `Common_SetRecvSeq(..., reqSeq)` 的逻辑。
   - 保留 `pIotAHTTCtx->reqSeq` 的范围回绕与递增逻辑。

未修改公共 `Common_*` 实现、CRC、超时、控制表、业务解析函数或其他协议。

## RED 证据

修改前源级检查命令：

```powershell
rg -n -C 3 "recvSeq\\s*==\\s*Common_GetRecvSeq\\(" -- 02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTRecv.c
```

输出命中接收侧第 138--139 行：在接收定时器已开启之外，还要求 `recvSeq == Common_GetRecvSeq(...)`。因此设备请求流水号 `01 00` 时，平台合法签到应答流水号 `87 04` 会因流水号不相等被拒绝，符合任务简报记录的 RED 现象。

## GREEN 证据

执行的源级检查及输出摘要：

```powershell
rg -n "recvSeq\\s*==\\s*Common_GetRecvSeq\\(" -- 02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTRecv.c
```

无匹配，检查通过。

```powershell
rg -n -U "Common_SetRecvSeq\\([\\s\\S]{0,180}recvSeq\\);" -- 02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTRecv.c
```

命中第 173--174 行，确认平台主动请求使用已解析的 `recvSeq` 保存。

```powershell
rg -n "Common_SetRecvSeq\\(" -- 02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTSend.c
```

无匹配，确认设备主动请求发送侧不再写接收流水号。

```powershell
rg -n "pIotAHTTCtx->reqSeq\\s*=\\s*\\(reqSeq >= IOT_AHTT_SEQ_MAX\\)" -- 02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTSend.c
git diff --check
```

前者命中第 213 行，确认请求流水号仍递增；后者退出码为 0。命令仅输出既有未关联文件的 LF/CRLF 警告，没有 whitespace 错误。

## 自审

- 应答分支仍按接收控制表的命令查找，并要求该命令对应的接收定时器已开启；只移除了错误的跨方向流水号相等约束。
- 主动请求分支使用同一帧已解析的 `recvSeq`，没有新增局部变量或运行期赋值。
- 发送侧应答继续从接收控制项读取流水号；设备主动请求仅推进自身 `reqSeq`，两方向序号不再互相覆盖。
- 未新增带 `U`、`L`、`UL`、`ULL` 后缀的数值字面量。

## 风险与限制

- 本次按简报执行源级验证，未运行构建或板端联调；仍需用题设向量（请求 `01 00`、签到应答 `87 04`）完成集成验证。
- 开始前，`Protocol_AHTT` 目录已处于未跟踪状态，故常规 `git diff` 不会展示这两个源文件；本报告以限定路径的内容检查和 `git diff --check` 退出码为依据。工作区还存在其他预先存在的未提交/未跟踪文件，未触碰。
