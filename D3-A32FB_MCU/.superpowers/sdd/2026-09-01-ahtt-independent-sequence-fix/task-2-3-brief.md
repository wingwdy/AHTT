### Task 2 与 Task 3：AHTT 独立流水号收发修复

**要求：**

1. 只修改：
   - `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTRecv.c`
   - `02_App/Src/ASW/ASW_COM/Asw_IotProtocol/Protocol_AHTT/Asw_IotProtoAHTTSend.c`
2. 平台应答确认只依赖“接收定时器已开启 + 指令”，删除 `recvSeq == Common_GetRecvSeq(...)` 比较。
3. 在平台主动请求分支中，保存已经解析的 `recvSeq`，供设备应答原样回送。
4. 删除设备主动请求发送后写入 `Common_SetRecvSeq(..., reqSeq)` 的代码；保留 `reqSeq` 递增。
5. 不修改公共 `Common_*` 函数、CRC、超时策略、其他协议、控制表和未实现业务解析函数。
6. 遵循 C 规范：不新增中间声明；不新增带 `U`、`L`、`UL`、`ULL` 后缀的数值字面量。
7. 先记录红灯检查结果，再使用 `apply_patch` 修改。运行源级绿灯检查、`git diff --check`。不要提交，不要生成或修改构建产物。

**验证向量：**

```text
设备请求流水号：01 00
平台合法签到应答流水号：87 04
期望：应答被接受，不能依赖两个方向流水号相等。
```

**参考：** 参考项目 `iot_AHTT_Protocol_Code.c` 的 `AHTTUpCtrlRecvDeal()` 仅按 `cmd` 分派并执行解析/成功回调，不比较对端帧流水号。
