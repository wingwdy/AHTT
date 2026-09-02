# AHTT Bore 模拟联调运行手册

本手册供 `ahtt-bore-sim-validation` 使用。所有时间采用同一台主机日志时间；测试对象必须是明确授权的测试桩。

## 0. 会话输入与产物

开始前记录：固件版本、设备编号、用户指定的 COM 口、生产端点 `www.ahttcd.cn,8888`、测试目的和时间。不要把 `get all` 的完整输出写入日志，因为它可能包含密钥或密码。

会话产物目录：`tmp/ahtt_sim/`。保存以下文件：模拟器 stdout/stderr、Bore stdout/stderr、串口脱敏摘要和本次 PID/远端端口。

## 1. 命令帧与主机侧门禁

1. 定位本切片脚本并运行，例如：

   ```powershell
   powershell -ExecutionPolicy Bypass -File tools\ahtt\Validate-AHTTM2.ps1
   ```

2. 使用可执行 Python，不使用 WindowsApps 占位 `python.exe`：

   ```powershell
   $py = 'C:\Users\Administrator\AppData\Local\Programs\Python\Python312\python.exe'
   & $py tools\ahtt\test_ahtt_platform_sim.py
   & $py tools\ahtt\ahtt_platform_sim.py --selftest
   ```

3. 任一命令失败则停止；先修复帧、CRC、长度、流水号或脚本问题。

## 2. 启动模拟器与 Bore

在 `D3-A32FB_MCU` 根目录执行。模拟器默认回复`0x01`，不回复`0x81`，不得添加 `--reply-heartbeat`。

```powershell
$py = 'C:\Users\Administrator\AppData\Local\Programs\Python\Python312\python.exe'
$bore = "$env:TEMP\bore\bore.exe"
$logDir = Join-Path $PWD 'tmp\ahtt_sim'
$stamp = Get-Date -Format 'yyyyMMdd_HHmmss'
New-Item -ItemType Directory -Path $logDir -Force | Out-Null

$simOut = Join-Path $logDir "sim_$stamp.out.log"
$simErr = Join-Path $logDir "sim_$stamp.err.log"
$boreOut = Join-Path $logDir "bore_$stamp.out.log"
$boreErr = Join-Path $logDir "bore_$stamp.err.log"

$sim = Start-Process -FilePath $py `
    -ArgumentList @('-u','tools\ahtt\ahtt_platform_sim.py','--host','127.0.0.1','--port','18888') `
    -WorkingDirectory $PWD -RedirectStandardOutput $simOut -RedirectStandardError $simErr `
    -WindowStyle Hidden -PassThru

$env:RUST_LOG = 'info'
$boreProc = Start-Process -FilePath $bore `
    -ArgumentList @('local','18888','--local-host','127.0.0.1','--to','bore.pub') `
    -WorkingDirectory $PWD -RedirectStandardOutput $boreOut -RedirectStandardError $boreErr `
    -WindowStyle Hidden -PassThru
```

最多等待20秒提取端口：

```powershell
$deadline = (Get-Date).AddSeconds(20)
do {
    Start-Sleep -Milliseconds 500
    $boreText = (Get-Content $boreOut -Raw -ErrorAction SilentlyContinue) +
                (Get-Content $boreErr -Raw -ErrorAction SilentlyContinue)
    $match = [regex]::Match($boreText, 'listening at bore\.pub:(\d{1,5})')
} while (($match.Success -eq $false) -and ((Get-Date) -lt $deadline) -and
         (Get-Process -Id $boreProc.Id -ErrorAction SilentlyContinue))

if ($match.Success -eq $false) { throw 'Bore未在20秒内公告远端端口' }
$remotePort = [int]$match.Groups[1].Value
if (($remotePort -lt 1) -or ($remotePort -gt 65535)) { throw 'Bore端口非法' }
"Bore endpoint: bore.pub:$remotePort; simPid=$($sim.Id); borePid=$($boreProc.Id)"
```

仅当模拟器和 Bore 均存活、`$remotePort`有效时，才允许修改桩参数。

## 3. 串口配置与重启

用户明确给出 COM 口即构成本次会话的写入和重启授权。默认串口参数：`115200, 8N1, 无流控, DTR/RTS=false`。

```powershell
$portName = 'COM3' # 必须替换为用户明确指定的端口
$serial = New-Object System.IO.Ports.SerialPort $portName,115200,'None',8,'One'
$serial.Handshake = [System.IO.Ports.Handshake]::None
$serial.DtrEnable = $false
$serial.RtsEnable = $false
$serial.ReadTimeout = 1500
$serial.WriteTimeout = 1500

try {
    $serial.Open()
    $serial.Write("set para domain:bore.pub,$remotePort`r`n")
    Start-Sleep -Seconds 1
    $reply = $serial.ReadExisting()
    if ($reply -notmatch "Set Plat Main domain port: \"bore.pub\", port=\s*$remotePort\s*ok!") {
        throw "平台地址配置未得到成功回显：$reply"
    }
    $serial.Write("reboot`r`n")
}
finally {
    if ($serial.IsOpen) { $serial.Close() }
    $serial.Dispose()
}
```

若端口提示拒绝访问，表示被串口工具占用。停止，不得杀进程或向其他 COM 口试探。用户释放后再重试一次。

重启后，以模拟器出现以下记录作为最终生效证据：

```text
[SIM] client connected
[RX] cmd=0x01, ... paramLen=28
[TX] cmd=0x01, ... paramLen=0
```

## 4. 三种板端测试模式

| 模式 | 操作 | 必须记录的结果 |
|---|---|---|
| 心跳静默 | 保持模拟器和 Bore | `0x81`约0/10/20秒三次；约30秒Offline与`QICLOSE` |
| TCP退避 | 在桩已关闭当前TCP后，只终止 `$boreProc.Id` | `QIOPEN`失败、5/10/30/30秒等待；CFUN或模组掉电恢复归网络层 |
| Bore恢复 | 用 `--port $remotePort` 重新启动 Bore | 新连接、重新`0x01`、首个`0x81` |

停止 Bore 后恢复同一端口：

```powershell
Stop-Process -Id $boreProc.Id
$env:RUST_LOG = 'info'
$boreProc = Start-Process -FilePath $bore `
    -ArgumentList @('local','18888','--local-host','127.0.0.1','--to','bore.pub','--port',"$remotePort") `
    -WorkingDirectory $PWD -RedirectStandardOutput $boreOut -RedirectStandardError $boreErr `
    -WindowStyle Hidden -PassThru
```

若指定端口无法重新申请，停止板端测试；必须以新端口重新配置并重启设备，不能让桩继续尝试旧端口。

## 5. 回写与恢复

仅以原始桩端日志、模拟器日志和 Bore 日志共同判定。回写：

- `docs/ahtt/AHTT-板端验证清单.md`
- `docs/ahtt/AHTT-联调问题记录.md`
- `docs/ahtt/AHTT-报文测试向量.md`
- `docs/ahtt/AHTT-V3.12-事实台账.md`
- `docs/ahtt/AHTT-V3.12-命令追踪矩阵.md`

最后固定恢复用户确认的默认历史端点；无需再次征求同意：

```text
set para domain:www.ahttcd.cn,8888
reboot
```

确认恢复成功后，仅终止本会话记录的 `$boreProc.Id` 与 `$sim.Id`。未验证的旧应答注入、并发Socket计数、20ms任务/看门狗和充电状态不得标为通过。
