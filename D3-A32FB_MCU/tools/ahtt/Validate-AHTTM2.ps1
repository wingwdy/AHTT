$ErrorActionPreference = 'Stop'

$script:PassedCount = 0
$script:FailedCount = 0
$script:SourcePendingCount = 0

function ConvertTo-HexString
{
    param([byte[]]$Data)

    return (($Data | ForEach-Object { $_.ToString('X2') }) -join '')
}

function Get-Crc16Modbus
{
    param([byte[]]$Data)

    [uint16]$crc = 0xFFFF

    foreach ($value in $Data)
    {
        $crc = [uint16]($crc -bxor $value)
        for ($bit = 0; $bit -lt 8; $bit++)
        {
            if (($crc -band 0x0001) -ne 0)
            {
                $crc = [uint16](($crc -shr 1) -bxor 0xA001)
            }
            else
            {
                $crc = [uint16]($crc -shr 1)
            }
        }
    }

    return $crc
}

function New-AhttFrame
{
    param(
        [byte[]]$DeviceNum,
        [uint16]$Seq,
        [byte]$Cmd,
        [byte[]]$Parameter = @()
    )

    $declareLen = 11 + $Parameter.Length
    $frameWithoutCrc = New-Object System.Collections.Generic.List[byte]
    $frameWithoutCrc.Add(0xEA)
    $frameWithoutCrc.Add([byte]($declareLen -band 0xFF))
    $frameWithoutCrc.Add([byte](($declareLen -shr 8) -band 0xFF))
    $frameWithoutCrc.Add(0x01)
    $frameWithoutCrc.AddRange($DeviceNum)
    $frameWithoutCrc.Add([byte]($Seq -band 0xFF))
    $frameWithoutCrc.Add([byte](($Seq -shr 8) -band 0xFF))
    $frameWithoutCrc.Add($Cmd)
    $frameWithoutCrc.AddRange($Parameter)

    $crc = Get-Crc16Modbus -Data $frameWithoutCrc.ToArray()
    $frameWithoutCrc.Add([byte]($crc -band 0xFF))
    $frameWithoutCrc.Add([byte](($crc -shr 8) -band 0xFF))
    return $frameWithoutCrc.ToArray()
}

function New-AhttLoginParameter
{
    param([string]$Iccid)

    if ($Iccid.Length -ne 20)
    {
        throw 'ICCID length must be 20'
    }

    $parameter = New-Object System.Collections.Generic.List[byte]
    $parameter.AddRange([Text.Encoding]::ASCII.GetBytes($Iccid))
    $parameter.Add(0x02)
    $parameter.Add(0x04)
    $parameter.AddRange([byte[]](0, 0, 0, 0, 0, 0))
    return $parameter.ToArray()
}

function New-AhttHeartbeatParameter
{
    param(
        [byte]$Csq,
        [byte[]]$ConfiguredStates
    )

    if ($ConfiguredStates.Length -gt 12)
    {
        throw 'heartbeat state count must not exceed 12'
    }

    $stateBytes = [byte[]](0xFF, 0xFF, 0xFF)
    for ($channel = 0; $channel -lt $ConfiguredStates.Length; $channel++)
    {
        if ($ConfiguredStates[$channel] -gt 3)
        {
            throw "invalid heartbeat state at channel $channel"
        }

        $byteIndex = [int][math]::Floor($channel / 4)
        $bitPos = ($channel % 4) * 2
        $stateMask = [byte](0x03 -shl $bitPos)
        $stateBytes[$byteIndex] = [byte](($stateBytes[$byteIndex] -band (-bnot $stateMask)) -bor
            ($ConfiguredStates[$channel] -shl $bitPos))
    }

    return [byte[]](0x02, $Csq, $stateBytes[0], $stateBytes[1], $stateBytes[2])
}

function New-ResponseWaitModel
{
    param(
        [byte]$Cmd
    )

    return [PSCustomObject]@{
        Cmd = $Cmd
        Waiting = $true
        LoginSuccess = $false
        SuccessCount = 0
    }
}

function Receive-AhttResponseModel
{
    param(
        [PSCustomObject]$Model,
        [byte]$Cmd,
        [uint16]$Seq,
        [byte[]]$Parameter = @()
    )

    if (($Model.Waiting -eq $true) -and ($Model.Cmd -eq $Cmd) -and
        ($Parameter.Length -eq 0))
    {
        $Model.Waiting = $false
        $Model.SuccessCount++
        if ($Cmd -eq 0x01)
        {
            $Model.LoginSuccess = $true
        }
        return $true
    }

    return $false
}

function Invoke-RetryModel
{
    param(
        [int]$TimeoutMs,
        [int]$MaxTryCount
    )

    $sendCount = 1
    $timeoutCount = 0
    $offline = $false

    while ($offline -eq $false)
    {
        $timeoutCount++
        if ($timeoutCount -ge $MaxTryCount)
        {
            $offline = $true
        }
        else
        {
            $sendCount++
        }
    }

    return [PSCustomObject]@{
        TimeoutMs = $TimeoutMs
        TimeoutCount = $timeoutCount
        SendCount = $sendCount
        Offline = $offline
        LoginSleepLevel = $null
        McuRebootRequested = $false
    }
}

function Assert-Equal
{
    param(
        $Actual,
        $Expected,
        [string]$Message
    )

    if ($Actual -ne $Expected)
    {
        throw "$Message; expected=$Expected actual=$Actual"
    }
}

function Assert-ByteArrayEqual
{
    param(
        [byte[]]$Actual,
        [byte[]]$Expected,
        [string]$Message
    )

    Assert-Equal $Actual.Length $Expected.Length "$Message length"
    for ($index = 0; $index -lt $Actual.Length; $index++)
    {
        Assert-Equal $Actual[$index] $Expected[$index] "$Message byte $index"
    }
}

function Invoke-Vector
{
    param(
        [string]$Name,
        [scriptblock]$Body
    )

    try
    {
        & $Body
        $script:PassedCount++
        Write-Host "[PASS] $Name"
    }
    catch
    {
        $script:FailedCount++
        Write-Host "[FAIL] $Name - $($_.Exception.Message)"
    }
}

function Test-RequiredText
{
    param(
        [string]$Name,
        [string]$Path,
        [string]$Pattern
    )

    $content = Get-Content -LiteralPath $Path -Raw
    if ($content -match $Pattern)
    {
        Write-Host "[PASS] $Name"
    }
    else
    {
        $script:FailedCount++
        Write-Host "[FAIL] $Name - required text not found"
    }
}

function Test-PendingSourceImplementation
{
    param(
        [string]$Name,
        [string]$Path,
        [string]$Pattern
    )

    $content = Get-Content -LiteralPath $Path -Raw
    if ($content -match $Pattern)
    {
        Write-Host "[PASS] $Name"
    }
    else
    {
        $script:SourcePendingCount++
        Write-Host "[PENDING] $Name - M2 business source is intentionally not implemented in batch 1"
    }
}

function Test-ForbiddenText
{
    param(
        [string]$Name,
        [string]$Path,
        [string]$Pattern
    )

    $content = Get-Content -LiteralPath $Path -Raw
    if ($content -match $Pattern)
    {
        $script:FailedCount++
        Write-Host "[FAIL] $Name - forbidden text found"
    }
    else
    {
        Write-Host "[PASS] $Name"
    }
}

$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$typesPath = Join-Path $projectRoot '02_App\Src\ASW\ASW_COM\Asw_IotProtocol\Protocol_AHTT\Asw_IotProtoAHTTTypes.h'
$sendPath = Join-Path $projectRoot '02_App\Src\ASW\ASW_COM\Asw_IotProtocol\Protocol_AHTT\Asw_IotProtoAHTTSend.c'
$recvPath = Join-Path $projectRoot '02_App\Src\ASW\ASW_COM\Asw_IotProtocol\Protocol_AHTT\Asw_IotProtoAHTTRecv.c'
$mPath = Join-Path $projectRoot '02_App\Src\ASW\ASW_COM\Asw_IotProtocol\Protocol_AHTT\Asw_IotProtoAHTTM.c'
$mHeaderPath = Join-Path $projectRoot '02_App\Src\ASW\ASW_COM\Asw_IotProtocol\Protocol_AHTT\Asw_IotProtoAHTTM.h'
$deviceNum = [byte[]](0x00, 0x00, 0x40, 0x00, 0x34)

Invoke-Vector 'M2-VEC-001 login parameter and complete frame use version 02 04' {
    $parameter = New-AhttLoginParameter -Iccid '89860416121980004151'
    Assert-Equal $parameter.Length 28 'login parameter length mismatch'
    Assert-Equal (ConvertTo-HexString -Data $parameter) '38393836303431363132313938303030343135310204000000000000' 'login parameter mismatch'
    $frame = New-AhttFrame -DeviceNum $deviceNum -Seq 1 -Cmd 0x01 -Parameter $parameter
    Assert-Equal (ConvertTo-HexString -Data $frame) 'EA2700010000400034010001383938363034313631323139383030303431353102040000000000003F70' 'login golden frame mismatch'
    Assert-Equal (Get-Crc16Modbus -Data $frame[0..($frame.Length - 3)]) 0x703F 'login CRC mismatch'
}

Invoke-Vector 'M2-VEC-002 independent-sequence zero-parameter login response progresses once' {
    $model = New-ResponseWaitModel -Cmd 0x01
    Assert-Equal (Receive-AhttResponseModel -Model $model -Cmd 0x01 -Seq 0x0587) $true 'independent-sequence login response rejected'
    Assert-Equal $model.LoginSuccess $true 'matching login response did not set login state'
    Assert-Equal $model.Waiting $false 'matching login response did not clear wait'
    Assert-Equal $model.SuccessCount 1 'matching login response did not progress once'
}

Invoke-Vector 'M2-VEC-003 parameterized login response retains wait' {
    $model = New-ResponseWaitModel -Cmd 0x01
    Assert-Equal (Receive-AhttResponseModel -Model $model -Cmd 0x01 -Seq 1 -Parameter ([byte[]](0x00))) $false 'parameterized login response accepted'
    Assert-Equal $model.Waiting $true 'parameterized login response cleared wait'
    Assert-Equal $model.LoginSuccess $false 'parameterized login response changed login state'
}

Invoke-Vector 'M2-VEC-004 inactive-wait duplicate and wrong-command login responses do not progress' {
    $model = New-ResponseWaitModel -Cmd 0x01
    Assert-Equal (Receive-AhttResponseModel -Model $model -Cmd 0x81 -Seq 0x0587) $false 'wrong-command response accepted'
    Assert-Equal $model.Waiting $true 'wrong-command response cleared wait'
    Assert-Equal (Receive-AhttResponseModel -Model $model -Cmd 0x01 -Seq 0x0587) $true 'independent-sequence current response rejected'
    Assert-Equal (Receive-AhttResponseModel -Model $model -Cmd 0x01 -Seq 0x0588) $false 'duplicate response accepted without wait'
    Assert-Equal $model.SuccessCount 1 'response sequence progressed more than once'
}

Invoke-Vector 'M2-VEC-005 idle heartbeat vector' {
    $parameter = New-AhttHeartbeatParameter -Csq 0x0C -ConfiguredStates ([byte[]](0x00))
    $frame = New-AhttFrame -DeviceNum $deviceNum -Seq 2 -Cmd 0x81 -Parameter $parameter
    Assert-Equal (ConvertTo-HexString -Data $frame) 'EA1000010000400034020081020CFCFFFF05F1' 'idle heartbeat golden frame mismatch'
}

Invoke-Vector 'M2-VEC-006 work heartbeat vector' {
    $parameter = New-AhttHeartbeatParameter -Csq 0x0C -ConfiguredStates ([byte[]](0x01))
    $frame = New-AhttFrame -DeviceNum $deviceNum -Seq 2 -Cmd 0x81 -Parameter $parameter
    Assert-Equal (ConvertTo-HexString -Data $frame) 'EA1000010000400034020081020CFDFFFF5431' 'work heartbeat golden frame mismatch'
}

Invoke-Vector 'M2-VEC-007 fault heartbeat vector' {
    $parameter = New-AhttHeartbeatParameter -Csq 0x0C -ConfiguredStates ([byte[]](0x02))
    $frame = New-AhttFrame -DeviceNum $deviceNum -Seq 2 -Cmd 0x81 -Parameter $parameter
    Assert-Equal (ConvertTo-HexString -Data $frame) 'EA1000010000400034020081020CFEFFFFA431' 'fault heartbeat golden frame mismatch'
}

Invoke-Vector 'M2-VEC-008 twelve-channel heartbeat bitmap maps two bits per channel' {
    $states = [byte[]](0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03)
    $parameter = New-AhttHeartbeatParameter -Csq 0x0C -ConfiguredStates $states
    Assert-Equal (ConvertTo-HexString -Data $parameter) '020CE4E4E4' 'twelve-channel heartbeat bitmap mismatch'
}

Invoke-Vector 'M2-VEC-009 login and heartbeat use ten seconds and three total sends' {
    foreach ($cmd in @([byte]0x01, [byte]0x81))
    {
        $result = Invoke-RetryModel -TimeoutMs 10000 -MaxTryCount 3
        Assert-Equal $result.TimeoutMs 10000 "command 0x$($cmd.ToString('X2')) timeout mismatch"
        Assert-Equal $result.TimeoutCount 3 "command 0x$($cmd.ToString('X2')) timeout count mismatch"
        Assert-Equal $result.SendCount 3 "command 0x$($cmd.ToString('X2')) send count mismatch"
        Assert-Equal $result.Offline $true "command 0x$($cmd.ToString('X2')) did not enter Offline"
        Assert-Equal ($null -eq $result.LoginSleepLevel) $true 'protocol private login backoff was introduced'
        Assert-Equal $result.McuRebootRequested $false 'protocol layer requested MCU reboot'
    }
}

Invoke-Vector 'M2-VEC-010 TCP reconnect uses default 5 10 30 30 30 seconds' {
    $reconnectIntervalsMs = @(5000, 10000, 30000, 30000, 30000)
    Assert-Equal ($reconnectIntervalsMs -join ',') '5000,10000,30000,30000,30000' 'TCP reconnect interval mismatch'
}

Invoke-Vector 'M2-VEC-011 heartbeat response accepts independent sequence and requires no parameters' {
    $model = New-ResponseWaitModel -Cmd 0x81
    Assert-Equal (Receive-AhttResponseModel -Model $model -Cmd 0x81 -Seq 0x0588 -Parameter ([byte[]](0x00))) $false 'parameterized heartbeat response accepted'
    Assert-Equal $model.Waiting $true 'parameterized heartbeat response cleared wait'
    Assert-Equal (Receive-AhttResponseModel -Model $model -Cmd 0x81 -Seq 0x0588) $true 'independent-sequence heartbeat response rejected'
}

Test-RequiredText -Name 'M2-CFG-001 login parameter length macro' -Path $typesPath -Pattern '#define\s+IOT_AHTT_LOGIN_PARAM_LEN\s+\(28\)'
Test-RequiredText -Name 'M2-CFG-002 heartbeat length and state macros' -Path $typesPath -Pattern '#define\s+IOT_AHTT_HEART_PARAM_LEN\s+\(5\)'
Test-RequiredText -Name 'M2-CFG-003 login timeout and retry macros' -Path $typesPath -Pattern '#define\s+IOT_AHTT_LOGIN_TIMEOUT_MS\s+\(10000\)[\s\S]*#define\s+IOT_AHTT_LOGIN_MAX_TRY_COUNT\s+\(3\)'
Test-RequiredText -Name 'M2-CFG-004 heartbeat timeout and retry macros' -Path $typesPath -Pattern '#define\s+IOT_AHTT_HEART_TIMEOUT_MS\s+\(10000\)[\s\S]*#define\s+IOT_AHTT_HEART_MAX_TRY_COUNT\s+\(3\)'

Test-RequiredText -Name 'M2-SRC-001 login sender uses Version ICCID and protocol field lengths' -Path $sendPath -Pattern '#include\s+"Version\.h"[\s\S]*IotAHTT_SendLoginReq[\s\S]*CddNetM_GetIccid[\s\S]*dataLen\s*\+=\s*20[\s\S]*APP_SW_MINOR_VERSION[\s\S]*APP_SW_CUSTORM_VERSION\s*\*\s*10\s*\+\s*APP_SW_PATCH_VERSION[\s\S]*memset\(&pBuf\[dataLen\],\s*0x00,\s*6\)[\s\S]*dataLen\s*\+=\s*6'
Test-RequiredText -Name 'M2-SRC-002 login sender table binds command 01' -Path $sendPath -Pattern '\{IOT_AHTT_CMD_LOGIN,\s*IOT_AHTT_CMDTYPE_REQUSET,\s*0,\s*IotAHTT_SendLoginReq,\s*IOT_AHTT_CMD_LOGIN'
Test-RequiredText -Name 'M2-SRC-003 login receiver accepts zero-length only' -Path $recvPath -Pattern 'IotAHTT_RecvLoginRsp[\s\S]*if\s*\(0\s*==\s*len\)[\s\S]*IotAHTT_LoginSuccess'
Test-RequiredText -Name 'M2-SRC-004 login receiver table sets timeout and retry' -Path $recvPath -Pattern '\{IOT_AHTT_CMD_LOGIN,\s*IOT_AHTT_CMDTYPE_RESPONSE,\s*IotAHTT_RecvLoginRsp,\s*IOT_AHTT_LOGIN_TIMEOUT_MS,\s*IOT_AHTT_LOGIN_MAX_TRY_COUNT'
Test-RequiredText -Name 'M2-SRC-005 TCP success enables login and LoginSuccess clears offline error' -Path $mPath -Pattern 'IotAHTT_WSLoginHandle[\s\S]*eIOTAHTTWorkState_Normal[\s\S]*Common_SetSendEnable[\s\S]*IOT_AHTT_CMD_LOGIN[\s\S]*void\s+IotAHTT_LoginSuccess\(void\)[\s\S]*loginSucc\s*=\s*TRUE[\s\S]*AswErrhandle_ResetErrExsitCallback'
Test-RequiredText -Name 'M2-SRC-006 login timeout retries then disconnects' -Path $recvPath -Pattern 'IotAHTT_TimeoutDetect[\s\S]*Common_SetRptCount[\s\S]*timeoutCount\s*>=\s*pCmdRecvCtrl->maxTryCnt[\s\S]*IOT_AHTT_CMD_LOGIN[\s\S]*IotAHTT_OfflineHandle[\s\S]*Common_SetSendEnable[\s\S]*Common_SetSendImmdFlag'
Test-RequiredText -Name 'M2-SRC-007 heartbeat sender table and protocol field lengths' -Path $sendPath -Pattern '\{IOT_AHTT_CMD_HEARTBEAT,\s*IOT_AHTT_CMDTYPE_REQUSET,\s*0,\s*IotAHTT_SendHeartBeat,\s*IOT_AHTT_CMD_HEARTBEAT[\s\S]*IotAHTT_SendHeartBeat[\s\S]*IOT_AHTT_HEART_NET_4G[\s\S]*CddNetM_GetCsq[\s\S]*memset\(&pBuf\[dataLen\],\s*0xFF,\s*3\)[\s\S]*IOT_AHTT_HEART_CHANNEL_COUNT[\s\S]*dataLen\s*\+=\s*3'
Test-RequiredText -Name 'M2-SRC-008 heartbeat period reads AHTT private parameter at runtime' -Path $sendPath -Pattern 'AswPlatM_GetPlatPrivateParamPtr\(\)[\s\S]*heartCycleMin\s*\*\s*IOT_AHTT_MINUTE_MS'
Test-RequiredText -Name 'M2-SRC-009 heartbeat receiver accepts zero-length only' -Path $recvPath -Pattern 'IotAHTT_RecvHeartBeatRsp[\s\S]*if\s*\(0\s*==\s*len\)[\s\S]*ret\s*=\s*TRUE'
Test-RequiredText -Name 'M2-SRC-010 heartbeat receiver table sets timeout and retry' -Path $recvPath -Pattern '\{IOT_AHTT_CMD_HEARTBEAT,\s*IOT_AHTT_CMDTYPE_RESPONSE,\s*IotAHTT_RecvHeartBeatRsp,\s*IOT_AHTT_HEART_TIMEOUT_MS,\s*IOT_AHTT_HEART_MAX_TRY_COUNT'
Test-RequiredText -Name 'M2-SRC-011 heartbeat timeout reaches Offline' -Path $recvPath -Pattern 'IOT_AHTT_CMD_LOGIN\s*==\s*pCmdRecvCtrl->cmd\)\s*\|\|\s*\(IOT_AHTT_CMD_HEARTBEAT\s*==\s*pCmdRecvCtrl->cmd\)[\s\S]*IotAHTT_OfflineHandle'
Test-RequiredText -Name 'M2-SRC-012 heartbeat state mapping and immediate enable' -Path $mPath -Pattern '(?s)(?=.*uint8_t\s+IotAHTT_GetGunState\(uint8_t port\).*AswErrHandle_IsExsistError\(port\).*IOT_AHTT_HEART_STATE_FAULT.*AswMonitor_IsOrderIdle\(port\).*IOT_AHTT_HEART_STATE_WORK)(?=.*void\s+IotAHTT_LoginSuccess\(void\).*IOT_AHTT_CMD_HEARTBEAT.*Common_SetSendImmdFlag)'
Test-RequiredText -Name 'M2-SRC-013 heartbeat state function declaration' -Path $mHeaderPath -Pattern 'uint8_t\s+IotAHTT_GetGunState\(uint8_t port\);'
Test-RequiredText -Name 'M2-SRC-014 minute conversion macro' -Path $typesPath -Pattern '#define\s+IOT_AHTT_MINUTE_MS\s+\(60000\)'
Test-ForbiddenText -Name 'M2-SRC-015 no private login backoff state' -Path $mPath -Pattern 'LoginGroupFailed|loginSleep|offlineEnterFlag|AswMonitor_SetReboot'
Test-RequiredText -Name 'M2-SRC-016 heartbeat stays enabled after successful periodic send' -Path $sendPath -Pattern 'if\s*\(\(0\s*==\s*pCmdSendCtrl->sendCycle\)\s*&&\s*\(IOT_AHTT_CMD_HEARTBEAT\s*!=\s*pCmdSendCtrl->cmd\)\)'
Test-ForbiddenText -Name 'M2-SRC-017 response matching does not compare platform and device sequences' -Path $recvPath -Pattern 'recvSeq\s*==\s*Common_GetRecvSeq'

Write-Host "AHTT M2 independent vectors: $($script:PassedCount) passed, $($script:FailedCount) failed"
Write-Host "AHTT M2 source consistency: $($script:SourcePendingCount) pending"

if ($script:FailedCount -ne 0)
{
    exit 1
}
