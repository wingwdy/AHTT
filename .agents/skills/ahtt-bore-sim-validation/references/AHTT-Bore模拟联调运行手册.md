# AHTT Bore 模拟联调运行手册

本手册供 `ahtt-bore-sim-validation` 使用，负责提供 **可执行但不依赖本机固定路径、固定生产端点、固定 COM 口或历史 timeout 数值** 的联调操作模板。

本手册不定义项目级编码规则、Git 安全规则或 Planner / Executor / Reviewer 角色边界；这些分别由：

```text
AGENTS.md
AHTT AI Development Prompts.md
.agents/skills/ahtt-development/SKILL.md
.agents/skills/ahtt-bore-sim-validation/SKILL.md
```

维护。

---

## 0. 核心原则

Bore 联调必须遵守：

```text
Current Evidence > Historical Value
User-approved Device > Guessed Device
Runtime-discovered Path > Hardcoded Absolute Path
Confirmed Restore Target > Historical Production Endpoint
Host Gate > Board Test
```

不得因为本手册过去记录过某个：

- Python 路径。
- Bore 路径。
- COM 口。
- 串口参数。
- Bore 端口。
- 生产域名。
- retry / timeout 秒数。

就直接视为当前真实值。

发生冲突时标记：

```text
[CONFLICT]
```

缺少必要输入时：

```text
[TBD]
```

需要板端或平台验证时：

```text
[TBC]
```

---

# 1. 会话前置输入

每次 Bore 联调必须先建立本次会话参数，不复用旧会话值。

至少记录：

```text
测试目的
当前 branch / HEAD
固件版本
设备编号
目标测试设备
用户确认的 COM 口
当前恢复目标来源
当前验证切片 / M 阶段
当前 Implementation Plan
当前时间
```

建议建立会话变量表：

| 变量           | 本次值       | 来源                       |
| -------------- | ------------ | -------------------------- |
| `RepoRoot`     | 运行时发现   | 当前 Git 工作目录          |
| `PythonExe`    | 运行时发现   | `Get-Command` / 用户指定   |
| `BoreExe`      | 运行时发现   | `Get-Command` / 用户指定   |
| `LocalHost`    | 运行时确认   | simulator 参数             |
| `LocalPort`    | 运行时确认   | simulator 参数 / 当前脚本  |
| `RemoteHost`   | Bore 公告    | 当前 Bore 日志             |
| `RemotePort`   | Bore 公告    | 当前 Bore 日志             |
| `ComPort`      | 用户确认     | 当前会话                   |
| `SerialConfig` | 当前设备要求 | 当前文档 / 用户确认        |
| `RestoreHost`  | 当前确认     | 用户 / 当前配置 / 权威文档 |
| `RestorePort`  | 当前确认     | 用户 / 当前配置 / 权威文档 |

如果任何会影响设备写入或恢复的关键参数无法确认：

```text
STOP
SESSION-INPUT-INCOMPLETE
```

---

# 2. 仓库与环境发现

## 2.1 确认仓库根目录

在 Desktop / PowerShell 环境执行只读检查：

```powershell
git rev-parse --show-toplevel
git status --short
git branch --show-current
git log -1 --oneline
```

将：

```powershell
$RepoRoot = (git rev-parse --show-toplevel).Trim()
Set-Location $RepoRoot
```

作为本次会话仓库根目录。

不得依赖历史绝对路径，例如：

```text
E:\...
C:\Users\...
```

---

## 2.2 动态发现 Python

优先按当前环境发现 Python：

```powershell
$pythonCandidates = @(
    (Get-Command py -ErrorAction SilentlyContinue),
    (Get-Command python -ErrorAction SilentlyContinue),
    (Get-Command python3 -ErrorAction SilentlyContinue)
) | Where-Object { $_ -ne $null }

if ($pythonCandidates.Count -eq 0) {
    throw '未发现可用 Python，请由用户或当前环境提供 Python 可执行路径。'
}
```

如果使用 `py` launcher，应先确认实际版本：

```powershell
py -0p
```

再选择与当前项目测试兼容的解释器。

如果项目文档、虚拟环境或当前用户明确指定了 Python，则以当前有效配置为准。

不得把历史 Python 绝对路径写死进本手册。

---

## 2.3 动态发现 Bore

优先顺序：

1. 当前会话用户明确指定的 Bore 可执行文件。
2. 项目运行手册 / 环境变量提供的位置。
3. `Get-Command bore`。
4. 当前工具目录中已确认的 Bore 可执行文件。

示例：

```powershell
$boreCmd = Get-Command bore -ErrorAction SilentlyContinue

if ($boreCmd -ne $null) {
    $BoreExe = $boreCmd.Source
}
```

若无法确认：

```text
STOP
BORE-NOT-FOUND
```

不得从未知位置下载或执行未确认二进制文件。

---

# 3. Host 侧门禁

进入真实设备联调前，必须先完成当前切片适用的 Host 验证。

---

## 3.1 动态发现 Validation Scripts

不要固定认为只有 M1 / M2 / M3。

先发现：

```powershell
Get-ChildItem tools\ahtt\Validate-AHTTM*.ps1 -ErrorAction SilentlyContinue
```

根据：

- 当前 Plan。
- 当前 M 阶段。
- 当前变更文件。
- 当前命令矩阵。

选择与本次任务匹配的 validation script。

例如：

```powershell
powershell -ExecutionPolicy Bypass -File <当前切片脚本>
```

如果当前任务对应脚本不存在：

- 不伪造脚本名。
- 按 Plan 中定义的替代验证执行。
- 标记缺少自动化验证的风险。

---

## 3.2 Python 测试动态发现

先检查：

```powershell
Get-ChildItem tools\ahtt\test_*.py -ErrorAction SilentlyContinue
Get-ChildItem tools\ahtt\*_sim.py -ErrorAction SilentlyContinue
```

如果当前仓库存在：

```text
tools/ahtt/test_ahtt_platform_sim.py
tools/ahtt/ahtt_platform_sim.py
```

则可执行：

```powershell
& $PythonExe tools\ahtt\test_ahtt_platform_sim.py
& $PythonExe tools\ahtt\ahtt_platform_sim.py --selftest
```

前提是 `$PythonExe` 已经由当前会话确认。

---

## 3.3 Build Gate

如果当前 Plan 要求 Keil build，确认当前工程文件实际存在：

```powershell
Test-Path 02_App\Prj\Project.uvprojx
```

再按当前开发环境执行真实 build。

未实际 build 不得标记：

```text
[PASS-BUILD]
```

---

## 3.4 Host Gate 失败

任一与当前联调直接相关的 Host 验证失败：

```text
STOP
HOST-GATE-FAILED
```

记录：

```text
失败命令
失败输出
涉及文件
是否由当前 diff 引入
下一步根因定位
```

不得为了“先看看板端表现”绕过 Host Gate。

---

# 4. 联调授权门禁

## 4.1 设备授权

目标必须是用户允许操作的测试设备。

如果无法确定：

```text
STOP
DEVICE-AUTH-UNKNOWN
```

---

## 4.2 COM 口不等于写入授权

仅知道：

```text
COM3
```

或任何其他 COM 口，不等于允许：

- 修改平台端点。
- 写设备参数。
- reboot。

本次联调允许串口写入，必须同时满足：

1. 用户明确要求执行 Bore 板端联调。
2. 用户明确指定或确认目标设备 / COM 口。
3. 当前已批准 Plan 包含临时端点配置和必要 reboot。

三个条件缺一不可。

---

## 4.3 恢复授权

推荐在联调开始前把：

```text
测试结束后恢复当前已确认端点
```

写入已批准 Plan。

这样测试结束时，在目标值没有发生变化的情况下，不需要针对同一恢复动作再次重复确认。

如果恢复目标发生变化或来源不可靠：

```text
STOP
RESTORE-TARGET-UNCONFIRMED
```

---

# 5. 获取当前串口参数

串口参数必须来自当前有效证据：

优先级：

```text
1. 用户本次明确确认
2. 当前设备 / 项目权威文档
3. 当前已验证 CLI / 设备配置
4. 历史运行手册
```

需要确认：

```text
baud rate
data bits
parity
stop bits
flow control
DTR
RTS
line ending
read timeout
write timeout
```

如果当前确认结果恰好是：

```text
115200, 8N1, 无流控
```

可以按该值执行。

但该值不是本手册的永久事实。

---

# 6. 确认模拟器当前参数

在启动前先检查：

```powershell
& $PythonExe tools\ahtt\ahtt_platform_sim.py --help
```

确认当前版本支持的：

```text
--host
--port
--selftest
heartbeat reply behavior
其他当前参数
```

再为本次会话设置：

```powershell
$LocalHost = '<当前确认的本地监听地址>'
$LocalPort = <当前确认的本地监听端口>
```

如果当前工具默认值已明确，也应在会话记录中注明来源。

不得因为历史版本曾使用 `127.0.0.1:18888` 就跳过当前确认。

---

# 7. 创建会话日志目录

建议：

```powershell
$logDir = Join-Path $RepoRoot 'tmp\ahtt_sim'
$stamp = Get-Date -Format 'yyyyMMdd_HHmmss'

New-Item -ItemType Directory -Path $logDir -Force | Out-Null

$simOut = Join-Path $logDir "sim_$stamp.out.log"
$simErr = Join-Path $logDir "sim_$stamp.err.log"
$boreOut = Join-Path $logDir "bore_$stamp.out.log"
$boreErr = Join-Path $logDir "bore_$stamp.err.log"
$sessionFile = Join-Path $logDir "session_$stamp.txt"
```

会话文件至少记录：

```text
RepoRoot
branch
HEAD
firmware
device id
COM
PythonExe
BoreExe
LocalHost
LocalPort
RemoteHost
RemotePort
simPid
borePid
RestoreHost
RestorePort
```

禁止记录无关密钥或密码。

---

# 8. 启动本地模拟器

在确认当前 CLI 参数后执行，例如：

```powershell
$sim = Start-Process `
    -FilePath $PythonExe `
    -ArgumentList @(
        '-u',
        'tools\ahtt\ahtt_platform_sim.py',
        '--host', $LocalHost,
        '--port', "$LocalPort"
    ) `
    -WorkingDirectory $RepoRoot `
    -RedirectStandardOutput $simOut `
    -RedirectStandardError $simErr `
    -WindowStyle Hidden `
    -PassThru
```

启动后检查：

```powershell
Get-Process -Id $sim.Id -ErrorAction SilentlyContinue
```

如果模拟器立即退出：

```text
STOP
SIMULATOR-START-FAILED
```

先检查 stderr。

---

# 9. 启动 Bore

Bore 的具体参数必须与当前 Bore 版本一致。

先检查：

```powershell
& $BoreExe --help
```

确认当前语法。

如果当前版本支持与历史相同的 `local` 模式，可按当前变量启动：

```powershell
$env:RUST_LOG = 'info'

$boreProc = Start-Process `
    -FilePath $BoreExe `
    -ArgumentList @(
        'local',
        "$LocalPort",
        '--local-host', $LocalHost,
        '--to', 'bore.pub'
    ) `
    -WorkingDirectory $RepoRoot `
    -RedirectStandardOutput $boreOut `
    -RedirectStandardError $boreErr `
    -WindowStyle Hidden `
    -PassThru
```

如果当前 Bore CLI 已变化，应按当前 `--help` 调整，不得机械使用旧参数。

---

# 10. 动态提取 Bore 远端端口

从当前会话 Bore 日志提取。

示例：

```powershell
$deadline = (Get-Date).AddSeconds(20)

do {
    Start-Sleep -Milliseconds 500

    $boreText =
        (Get-Content $boreOut -Raw -ErrorAction SilentlyContinue) +
        (Get-Content $boreErr -Raw -ErrorAction SilentlyContinue)

    $match = [regex]::Match(
        $boreText,
        'listening at bore\.pub:(\d{1,5})'
    )

} while (
    ($match.Success -eq $false) -and
    ((Get-Date) -lt $deadline) -and
    (Get-Process -Id $boreProc.Id -ErrorAction SilentlyContinue)
)

if ($match.Success -eq $false) {
    throw 'Bore 未公告有效远端端口'
}

$RemoteHost = 'bore.pub'
$RemotePort = [int]$match.Groups[1].Value

if (($RemotePort -lt 1) -or ($RemotePort -gt 65535)) {
    throw 'Bore 远端端口非法'
}
```

如果当前 Bore 日志格式发生变化：

```text
[CONFLICT]
```

应根据当前版本日志重新确定提取规则。

不得复用旧会话 `$RemotePort`。

---

# 11. 修改设备测试端点前的最终门禁

只有同时满足：

```text
Host gate PASS
Simulator alive
Bore alive
RemotePort valid
Device authorized
COM confirmed
Serial config confirmed
Temporary endpoint write approved
Restore target confirmed
```

才能进入设备参数写入。

否则：

```text
STOP
DEVICE-WRITE-GATE-FAILED
```

---

# 12. 动态确定设备 CLI 命令

设备 CLI 命令格式必须以当前有效证据为准。

如果当前设备仍确认使用：

```text
set para domain:<host>,<port>
reboot
```

则可以按当前变量生成：

```text
set para domain:<RemoteHost>,<RemotePort>
reboot
```

如果 CLI 已变化：

- 以当前设备文档 / 设备回显 / 用户确认版本为准。
- 不使用历史命令格式强行写入。

---

# 13. 串口写入模板

以下仅为执行结构模板。

必须在 `$ComPort`、串口参数和命令格式均已确认后使用。

```powershell
$serial = New-Object System.IO.Ports.SerialPort

$serial.PortName = $ComPort
$serial.BaudRate = $SerialBaud
$serial.DataBits = $SerialDataBits
$serial.Parity = $SerialParity
$serial.StopBits = $SerialStopBits
$serial.Handshake = $SerialHandshake
$serial.DtrEnable = $SerialDtrEnable
$serial.RtsEnable = $SerialRtsEnable
$serial.ReadTimeout = $SerialReadTimeout
$serial.WriteTimeout = $SerialWriteTimeout

try {
    $serial.Open()

    $setDomainCmd = "<按当前确认 CLI 格式构造测试端点命令>"
    $serial.Write($setDomainCmd + $SerialLineEnding)

    Start-Sleep -Seconds 1
    $reply = $serial.ReadExisting()

    # 必须按当前固件真实成功回显规则判断。
    if (<当前成功回显判断未满足>) {
        throw "平台地址配置未取得明确成功证据：$reply"
    }

    $serial.Write("<当前确认 reboot 命令>" + $SerialLineEnding)
}
finally {
    if ($serial.IsOpen) {
        $serial.Close()
    }

    $serial.Dispose()
}
```

禁止把模板占位符直接发送到设备。

---

# 14. COM 被占用时

如果串口打开失败并显示占用 / 拒绝访问：

```text
STOP
COM-BUSY
```

不得：

- 杀死用户串口工具。
- 扫描并尝试其他 COM。
- 向其他 COM 盲发命令。
- 自动重试多台设备。

让用户释放目标串口后，再针对同一已确认 COM 重试。

---

# 15. 临时端点生效判定

不能只依赖单一证据。

至少应形成：

```text
串口成功回显
+
Bore 存活
+
模拟器收到新连接
+
模拟器收到预期 AHTT 帧
```

如果本次测试目标包含签到，则典型证据链为：

```text
设备配置成功
→ reboot
→ TCP connect
→ simulator client connected
→ 收到签到命令
→ 模拟器按当前协议正确回复
```

具体命令号和字段以当前 AHTT 代码 / 协议为准。

---

# 16. 心跳静默测试

## 16.1 先确定当前期望时序

不要固定使用历史：

```text
0 / 10 / 20 / 30 秒
```

或其他值。

必须从当前：

- Implementation Plan。
- 当前 AHTT 代码常量。
- 当前事实台账。
- 当前协议要求。

提取：

```text
heartbeat period
response wait timeout
retry count
offline transition
TCP close behavior
```

把本次期望值写入会话记录。

---

## 16.2 执行

让模拟器进入当前已确认的：

```text
heartbeat no-response
```

模式。

然后记录：

- 每次设备心跳时间戳。
- retry 次数。
- timeout。
- Offline 时间。
- TCP 关闭行为。
- 后续重连行为。

只有实测时序与当前期望匹配，才能标记该项通过。

---

# 17. TCP backoff 测试

同样不得硬编码历史：

```text
5 / 10 / 30 / 30 秒
```

必须先从当前代码确认实际退避策略。

记录：

```text
第 1 次连接失败时间
第 2 次尝试时间
第 3 次尝试时间
...
退避值
是否封顶
是否触发网络层恢复
```

如果观察到：

- CFUN。
- 模组重启。
- 模组掉电。
- 其他网络层恢复。

必须与 AHTT 协议层状态机分开记录。

---

# 18. Bore 恢复测试

如果本次测试要求恢复同一 Bore 远端端口：

1. 只停止本次会话记录的 Bore PID。
2. 检查当前 Bore 版本是否支持指定远端端口恢复。
3. 按当前 CLI 重启。
4. 确认是否真正获得原端口。

如果无法获得原端口：

```text
STOP
BORE-PORT-CHANGED
```

不得让设备无限尝试旧端口。

如需新端口：

- 获取新的有效 RemotePort。
- 确认重新配置设备仍属于已批准范围。
- 再执行设备端点更新。

---

# 19. 证据采集

本次联调至少保存：

```text
simulator stdout
simulator stderr
Bore stdout
Bore stderr
串口脱敏摘要
关键时间戳
PID
RemotePort
固件版本
设备编号
branch / HEAD
```

不要保存：

- 完整密码。
- 密钥。
- 无关敏感配置 dump。
- 与本次诊断无关的设备信息。

---

# 20. 通过等级

严格区分：

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

示例：

```text
Bore listening
≠ PASS-BOARD

Simulator 收到 TCP
≠ 协议功能 PASS

Host test PASS
≠ Board PASS

Board PASS
≠ Real Platform PASS
```

---

# 21. 文档回写

根据本次真实证据判断是否更新：

```text
docs/ahtt/AHTT-板端验证清单.md
docs/ahtt/AHTT-联调问题记录.md
docs/ahtt/AHTT-报文测试向量.md
docs/ahtt/AHTT-V3.12-事实台账.md
docs/ahtt/AHTT-V3.12-命令追踪矩阵.md
```

如果仓库已新增更权威的对应文档，也要使用当前真实文件。

不得因为本手册只列出以上历史文件名就忽略新文档。

---

# 22. 恢复目标的确定

测试结束前必须重新确认本次 `RestoreHost` / `RestorePort` 来源。

优先级：

```text
1. 用户本次明确确认
2. 测试前已读取并保存的设备原配置
3. 当前权威项目配置文档
4. 历史联调记录
```

历史曾使用过的：

```text
www.ahttcd.cn:8888
```

只能作为历史线索，不得作为永久恢复目标。

如果没有可靠恢复目标：

```text
STOP
RESTORE-TARGET-UNCONFIRMED
```

不得猜测并写回设备。

---

# 23. 执行恢复

如果当前已批准 Plan 包含测试结束后的恢复动作，并且 RestoreTarget 没有变化，则可以直接执行。

命令格式按当前设备 CLI 生成：

```text
set para domain:<RestoreHost>,<RestorePort>
reboot
```

前提是该 CLI 格式仍被当前设备确认。

恢复后必须取得：

```text
串口成功回显
+
设备 reboot / 新连接证据
+
设备已离开 Bore 测试端点
```

至少足以证明临时测试配置没有遗留。

---

# 24. 进程清理

恢复完成后：

- 只停止本次会话记录的 Bore PID。
- 只停止本次会话记录的 simulator PID。

示例：

```powershell
if (Get-Process -Id $boreProc.Id -ErrorAction SilentlyContinue) {
    Stop-Process -Id $boreProc.Id
}

if (Get-Process -Id $sim.Id -ErrorAction SilentlyContinue) {
    Stop-Process -Id $sim.Id
}
```

不得：

- `taskkill /IM bore.exe /F`
- 按名字批量结束所有 Python。
- 杀用户已有串口工具。
- 删除其他会话日志。

---

# 25. STOP 条件总表

出现任一情况停止扩大操作：

```text
SESSION-INPUT-INCOMPLETE
HOST-GATE-FAILED
DEVICE-AUTH-UNKNOWN
BORE-NOT-FOUND
SIMULATOR-START-FAILED
COM-BUSY
DEVICE-WRITE-GATE-FAILED
设备配置无明确成功回显
设备 reboot 无法确认
模拟器未收到预期连接 / 签到
BORE-PORT-CHANGED
RESTORE-TARGET-UNCONFIRMED
关键日志相互矛盾
当前代码与运行手册关键参数冲突
需要操作未批准设备 / 端口 / 进程
```

停止时输出：

```text
最后一个成功 Gate
失败位置
本次已执行动作
已保存证据
当前设备配置是否已经改变
是否已恢复
下一步需要用户确认什么
```

---

# 26. 会话记录模板

每次联调建议保存以下摘要：

```text
# AHTT Bore Session

Time:
Branch:
HEAD:
Firmware:
Device:
COM:

## Host Gate

Validation:
Python test:
Simulator selftest:
Build:

## Runtime

PythonExe:
BoreExe:
LocalHost:
LocalPort:
RemoteHost:
RemotePort:
SimulatorPID:
BorePID:

## Serial

Baud:
DataBits:
Parity:
StopBits:
FlowControl:
DTR:
RTS:
LineEnding:

## Test Target

Mode:
Expected timing source:
Expected heartbeat:
Expected timeout:
Expected retry:
Expected backoff:

## Evidence

Simulator log:
Bore log:
Serial log:
Board result:

## Restore

Restore target source:
RestoreHost:
RestorePort:
Restore result:

## Final Status

Host:
Build:
Simulator:
Board:
Platform:
Remaining risks:
```

---

# 27. 本手册维护规则

以后更新本手册时：

优先维护：

- 环境发现方法。
- 执行模板。
- Gate。
- 证据要求。
- Stop 条件。

不要重新硬编码：

- 某台机器绝对路径。
- 某个 COM 号。
- 某个永久生产域名。
- 某组 timeout / retry 数值。
- 某个历史 Bore 端口。

需要记录当前产品事实时，应放入：

```text
docs/ahtt/
当前配置文档
当前事实台账
当前 Implementation Plan
```

而不是把本运行手册变成新的事实数据库。
