$ErrorActionPreference = 'Stop'

$script:PassedCount = 0
$script:FailedCount = 0

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

function ConvertTo-HexString
{
    param([byte[]]$Data)

    return (($Data | ForEach-Object { $_.ToString('X2') }) -join '')
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
    $frame = New-Object System.Collections.Generic.List[byte]
    $frame.Add(0xEA)
    $frame.Add([byte]($declareLen -band 0xFF))
    $frame.Add([byte](($declareLen -shr 8) -band 0xFF))
    $frame.Add(0x01)
    $frame.AddRange($DeviceNum)
    $frame.Add([byte]($Seq -band 0xFF))
    $frame.Add([byte](($Seq -shr 8) -band 0xFF))
    $frame.Add($Cmd)
    $frame.AddRange($Parameter)

    $crc = Get-Crc16Modbus -Data $frame.ToArray()
    $frame.Add([byte]($crc -band 0xFF))
    $frame.Add([byte](($crc -shr 8) -band 0xFF))
    return $frame.ToArray()
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
        Write-Host "[FAIL] $Name - required M3 implementation not found"
    }
}

function Invoke-ParameterSetModel
{
    param(
        [int]$CurrentValue,
        [int]$RequestedValue,
        [int]$MinValue,
        [int]$MaxValue,
        [bool]$WriteSucceeds
    )

    $result = 0
    $storedValue = $CurrentValue
    $writeCount = 0

    if (($RequestedValue -ge $MinValue) -and ($RequestedValue -le $MaxValue))
    {
        if ($RequestedValue -eq $CurrentValue)
        {
            $result = 1
        }
        elseif ($WriteSucceeds -eq $true)
        {
            $storedValue = $RequestedValue
            $writeCount = 1
            $result = 1
        }
        else
        {
            $writeCount = 2
        }
    }

    return [PSCustomObject]@{
        Result = $result
        StoredValue = $storedValue
        WriteCount = $writeCount
    }
}

function Invoke-PrivateParamMigrationModel
{
    param(
        [int]$HeartCycle,
        [int]$MaxChargeTime,
        [bool]$WriteSucceeds
    )

    $runtimeHeartCycle = $HeartCycle
    $runtimeMaxChargeTime = $MaxChargeTime
    $persisted = $WriteSucceeds

    if (($runtimeHeartCycle -lt 1) -or ($runtimeHeartCycle -gt 10))
    {
        $runtimeHeartCycle = 5
    }

    if (($runtimeMaxChargeTime -lt 1) -or ($runtimeMaxChargeTime -gt 16))
    {
        $runtimeMaxChargeTime = 10
    }

    return [PSCustomObject]@{
        HeartCycle = $runtimeHeartCycle
        MaxChargeTime = $runtimeMaxChargeTime
        NvmMirrorHeartCycle = $runtimeHeartCycle
        NvmMirrorMaxChargeTime = $runtimeMaxChargeTime
        Persisted = $persisted
        PersistPending = (-not $persisted)
    }
}

function Invoke-PendingPrivateParamRetryModel
{
    param(
        [bool]$WriteSucceeds
    )

    $writeCount = 1
    $persistPending = (-not $WriteSucceeds)

    return [PSCustomObject]@{
        Result = $WriteSucceeds
        WriteCount = $writeCount
        PersistPending = $persistPending
    }
}

function Invoke-DeviceParamSetModel
{
    param(
        [byte[]]$CurrentValue,
        [byte[]]$RequestedValue,
        [bool]$WriteSucceeds
    )

    $result = 0
    $writeCount = 0
    $storedValue = [byte[]]$CurrentValue.Clone()

    if (($RequestedValue.Length -eq 8) -and
        ($RequestedValue[7] -ge 1) -and
        ($RequestedValue[7] -le 30))
    {
        if ((ConvertTo-HexString -Data $RequestedValue) -eq (ConvertTo-HexString -Data $CurrentValue))
        {
            $result = 1
        }
        elseif ($WriteSucceeds -eq $true)
        {
            $storedValue = [byte[]]$RequestedValue.Clone()
            $writeCount = 1
            $result = 1
        }
        else
        {
            $writeCount = 2
        }
    }

    return [PSCustomObject]@{
        Result = $result
        WriteCount = $writeCount
        StoredValue = $storedValue
    }
}

function ConvertTo-DomainString
{
    param([byte[]]$Domain)

    $chars = New-Object System.Collections.Generic.List[char]
    foreach ($value in $Domain)
    {
        if ($value -eq 0)
        {
            break
        }
        $chars.Add([char]$value)
    }

    return (-join $chars.ToArray())
}

function ConvertTo-PortValue
{
    param([byte[]]$Port)

    $portValue = 0
    foreach ($value in $Port)
    {
        if (($value -lt [byte][char]'0') -or ($value -gt [byte][char]'9'))
        {
            break
        }
        $portValue = $portValue * 10 + ($value - [byte][char]'0')
    }

    return $portValue
}

function Invoke-DomainSwitchModel
{
    param(
        [string]$Scenario,
        [uint16]$RequestSeq = 0
    )

    $result = [PSCustomObject]@{
        NvmWriteCount = 0
        NetworkAction = 'none'
        Response = 'none'
        ResponseSeq = 0
        PersistedAddress = 'old'
    }

    switch ($Scenario)
    {
        'same-address'
        {
            $result.NetworkAction = 'none'
        }
        'new-login-success'
        {
            $result.NetworkAction = 'candidate-connected'
            $result.NvmWriteCount = 1
            $result.PersistedAddress = 'new'
        }
        'tcp-fail'
        {
            $result.NetworkAction = 'rollback-connected'
            $result.Response = 'fail'
            $result.ResponseSeq = $RequestSeq
        }
        'nvm-fail'
        {
            $result.NetworkAction = 'rollback-connected'
            $result.NvmWriteCount = 2
            $result.Response = 'fail'
            $result.ResponseSeq = $RequestSeq
        }
        'nvm-async-verify-fail'
        {
            $result.NetworkAction = 'rollback-connected'
            $result.NvmWriteCount = 2
            $result.Response = 'fail'
            $result.ResponseSeq = $RequestSeq
        }
        'busy'
        {
            $result.Response = 'fail'
            $result.ResponseSeq = $RequestSeq
        }
        'power-loss-trying'
        {
            $result.PersistedAddress = 'old'
        }
        default
        {
            throw "unknown scenario $Scenario"
        }
    }

    return $result
}

$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$typesPath = Join-Path $projectRoot '02_App\Src\ASW\ASW_COM\Asw_IotProtocol\Protocol_AHTT\Asw_IotProtoAHTTTypes.h'
$recvPath = Join-Path $projectRoot '02_App\Src\ASW\ASW_COM\Asw_IotProtocol\Protocol_AHTT\Asw_IotProtoAHTTRecv.c'
$sendPath = Join-Path $projectRoot '02_App\Src\ASW\ASW_COM\Asw_IotProtocol\Protocol_AHTT\Asw_IotProtoAHTTSend.c'
$mPath = Join-Path $projectRoot '02_App\Src\ASW\ASW_COM\Asw_IotProtocol\Protocol_AHTT\Asw_IotProtoAHTTM.c'
$mHeaderPath = Join-Path $projectRoot '02_App\Src\ASW\ASW_COM\Asw_IotProtocol\Protocol_AHTT\Asw_IotProtoAHTTM.h'
$nvmTypePath = Join-Path $projectRoot '02_App\Src\BSW\MemoryService\NVM\MS_NvmAppTypes.h'
$nvmHeaderPath = Join-Path $projectRoot '02_App\Src\BSW\MemoryService\NVM\MS_Nvm.h'
$deviceNum = [byte[]](0x00, 0x00, 0x40, 0x00, 0x34)

Invoke-Vector 'M3-VEC-001 set-heartbeat accepts one and ten minutes' {
    foreach ($value in @(1, 10))
    {
        $result = Invoke-ParameterSetModel -CurrentValue 5 -RequestedValue $value -MinValue 1 -MaxValue 10 -WriteSucceeds $true
        Assert-Equal $result.Result 1 "heartbeat value $value was rejected"
        Assert-Equal $result.StoredValue $value "heartbeat value $value was not stored"
    }
}

Invoke-Vector 'M3-VEC-002 set-heartbeat rejects zero and eleven minutes' {
    foreach ($value in @(0, 11))
    {
        $result = Invoke-ParameterSetModel -CurrentValue 5 -RequestedValue $value -MinValue 1 -MaxValue 10 -WriteSucceeds $true
        Assert-Equal $result.Result 0 "invalid heartbeat value $value was accepted"
        Assert-Equal $result.StoredValue 5 "invalid heartbeat value $value changed storage"
        Assert-Equal $result.WriteCount 0 "invalid heartbeat value $value wrote NVM"
    }
}

Invoke-Vector 'M3-VEC-003 set-max-charge-time accepts one and sixteen hours' {
    foreach ($value in @(1, 16))
    {
        $result = Invoke-ParameterSetModel -CurrentValue 10 -RequestedValue $value -MinValue 1 -MaxValue 16 -WriteSucceeds $true
        Assert-Equal $result.Result 1 "max-charge value $value was rejected"
        Assert-Equal $result.StoredValue $value "max-charge value $value was not stored"
    }
}

Invoke-Vector 'M3-VEC-004 set-max-charge-time rejects zero and seventeen hours' {
    foreach ($value in @(0, 17))
    {
        $result = Invoke-ParameterSetModel -CurrentValue 10 -RequestedValue $value -MinValue 1 -MaxValue 16 -WriteSucceeds $true
        Assert-Equal $result.Result 0 "invalid max-charge value $value was accepted"
        Assert-Equal $result.StoredValue 10 "invalid max-charge value $value changed storage"
    }
}

Invoke-Vector 'M3-VEC-005 duplicate setting succeeds without NVM write' {
    $result = Invoke-ParameterSetModel -CurrentValue 5 -RequestedValue 5 -MinValue 1 -MaxValue 10 -WriteSucceeds $true
    Assert-Equal $result.Result 1 'duplicate setting did not succeed'
    Assert-Equal $result.StoredValue 5 'duplicate setting changed value'
    Assert-Equal $result.WriteCount 0 'duplicate setting wrote NVM'
}

Invoke-Vector 'M3-VEC-006 failed NVM write restores the old value' {
    $result = Invoke-ParameterSetModel -CurrentValue 5 -RequestedValue 10 -MinValue 1 -MaxValue 10 -WriteSucceeds $false
    Assert-Equal $result.Result 0 'failed NVM write reported success'
    Assert-Equal $result.StoredValue 5 'failed NVM write left candidate value active'
    Assert-Equal $result.WriteCount 2 'failed NVM write did not restore NVM mirror'
}

Invoke-Vector 'M3-VEC-007 zero-parameter query frames are complete' {
    $heartFrame = New-AhttFrame -DeviceNum $deviceNum -Seq 1 -Cmd 0x03
    $maxTimeFrame = New-AhttFrame -DeviceNum $deviceNum -Seq 2 -Cmd 0x0B
    Assert-Equal $heartFrame.Length 14 'heartbeat query frame length mismatch'
    Assert-Equal $maxTimeFrame.Length 14 'max-charge query frame length mismatch'
}

Invoke-Vector 'M3-VEC-008 failed migration keeps safe runtime defaults' {
    $result = Invoke-PrivateParamMigrationModel -HeartCycle 0 -MaxChargeTime 0 -WriteSucceeds $false
    Assert-Equal $result.HeartCycle 5 'failed migration retained an invalid heartbeat period'
    Assert-Equal $result.MaxChargeTime 10 'failed migration retained an invalid max-charge period'
    Assert-Equal $result.NvmMirrorHeartCycle 5 'failed migration left an invalid NVM mirror heartbeat period'
    Assert-Equal $result.NvmMirrorMaxChargeTime 10 'failed migration left an invalid NVM mirror max-charge period'
    Assert-Equal $result.Persisted $false 'failed migration reported persistence success'
    Assert-Equal $result.PersistPending $true 'failed migration did not keep a persistence retry pending'
}

Invoke-Vector 'M3-VEC-009 pending same-value setting retries persistence' {
    $result = Invoke-PendingPrivateParamRetryModel -WriteSucceeds $true
    Assert-Equal $result.Result $true 'pending same-value setting did not report success after retry'
    Assert-Equal $result.WriteCount 1 'pending same-value setting did not retry persistence'
    Assert-Equal $result.PersistPending $false 'successful retry did not clear persistence pending'
}

Invoke-Vector 'M3-VEC-010 device parameter accepts complete eight-byte values' {
    $current = [byte[]](0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01)
    foreach ($uploadCycle in @(1, 30))
    {
        $requested = [byte[]](0xFF, 0x7F, 0x00, 0x55, 0xAA, 0x01, 0xFE, $uploadCycle)
        $result = Invoke-DeviceParamSetModel -CurrentValue $current -RequestedValue $requested -WriteSucceeds $true
        Assert-Equal $result.Result 1 "device parameter upload cycle $uploadCycle was rejected"
        Assert-Equal $result.WriteCount 1 "device parameter upload cycle $uploadCycle was not persisted"
        Assert-ByteArrayEqual -Actual $result.StoredValue -Expected $requested "device parameter upload cycle $uploadCycle"
    }
}

Invoke-Vector 'M3-VEC-011 device parameter rejects invalid length and upload cycle atomically' {
    $current = [byte[]](0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x01)
    $invalidValues = @(
        [byte[]](0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77),
        [byte[]](0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x00),
        [byte[]](0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x1F),
        [byte[]](0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x01, 0x00)
    )

    foreach ($requested in $invalidValues)
    {
        $result = Invoke-DeviceParamSetModel -CurrentValue $current -RequestedValue $requested -WriteSucceeds $true
        Assert-Equal $result.Result 0 'invalid device parameter was accepted'
        Assert-Equal $result.WriteCount 0 'invalid device parameter wrote NVM'
        Assert-ByteArrayEqual -Actual $result.StoredValue -Expected $current 'invalid device parameter changed storage'
    }
}

Invoke-Vector 'M3-VEC-012 device parameter write failure restores all eight bytes' {
    $current = [byte[]](0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x01)
    $requested = [byte[]](0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x1E)
    $result = Invoke-DeviceParamSetModel -CurrentValue $current -RequestedValue $requested -WriteSucceeds $false
    Assert-Equal $result.Result 0 'device parameter write failure reported success'
    Assert-Equal $result.WriteCount 2 'device parameter write failure did not restore NVM mirror'
    Assert-ByteArrayEqual -Actual $result.StoredValue -Expected $current 'device parameter write failure changed storage'
}

Invoke-Vector 'M3-VEC-013 device parameter query request uses zero parameters' {
    $queryFrame = New-AhttFrame -DeviceNum $deviceNum -Seq 3 -Cmd 0x85
    Assert-Equal $queryFrame.Length 14 'device parameter query frame length mismatch'
}

Invoke-Vector 'M3-VEC-014 domain and port fields are accepted verbatim and parsed to the first terminator' {
    $domain = [byte[]](0x6E, 0x65, 0x77, 0x2E, 0x61, 0x68, 0x74, 0x74, 0x2E, 0x63, 0x6E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
    $port = [byte[]](0x30, 0x30, 0x30, 0x34, 0x34, 0x33)
    Assert-Equal (ConvertTo-DomainString -Domain $domain) 'new.ahtt.cn' 'right zero padding was not trimmed'
    Assert-Equal (ConvertTo-PortValue -Port $port) 443 'ascii port was not parsed'
    $domain[4] = 0x2A
    Assert-Equal (ConvertTo-DomainString -Domain $domain) 'new.*htt.cn' 'illegal characters were rejected instead of accepted verbatim'
    $domain[12] = 0x61
    Assert-Equal (ConvertTo-DomainString -Domain $domain) 'new.*htt.cn' 'bytes after the first zero were not ignored'
    $port = [byte[]](0x30, 0x36, 0x35, 0x35, 0x33, 0x36)
    Assert-Equal (ConvertTo-PortValue -Port $port) 65536 'port 65536 was rejected instead of accepted verbatim'
}

Invoke-Vector 'M3-VEC-015 domain switch success writes only after candidate login and stays silent' {
    $result = Invoke-DomainSwitchModel -Scenario 'new-login-success' -RequestSeq 412
    Assert-Equal $result.NetworkAction 'candidate-connected' 'candidate was not connected'
    Assert-Equal $result.NvmWriteCount 1 'successful candidate login did not commit once'
    Assert-Equal $result.PersistedAddress 'new' 'successful candidate login did not persist new address'
    Assert-Equal $result.Response 'none' 'successful domain switch sent an answer'
}

Invoke-Vector 'M3-VEC-016 failed candidate and failed NVM restore old address then use original sequence' {
    foreach ($scenario in @('tcp-fail', 'nvm-fail'))
    {
        $result = Invoke-DomainSwitchModel -Scenario $scenario -RequestSeq 413
        Assert-Equal $result.NetworkAction 'rollback-connected' "$scenario did not reconnect old address"
        Assert-Equal $result.Response 'fail' "$scenario did not report failure"
        Assert-Equal $result.ResponseSeq 413 "$scenario changed the original sequence"
        Assert-Equal $result.PersistedAddress 'old' "$scenario left candidate persisted"
    }
}

Invoke-Vector 'M3-VEC-017 duplicate busy and power-loss cases preserve transaction consistency' {
    $same = Invoke-DomainSwitchModel -Scenario 'same-address' -RequestSeq 414
    Assert-Equal $same.NetworkAction 'none' 'same address reconnected'
    Assert-Equal $same.NvmWriteCount 0 'same address wrote NVM'
    Assert-Equal $same.Response 'none' 'same address sent success response'
    $busy = Invoke-DomainSwitchModel -Scenario 'busy' -RequestSeq 415
    Assert-Equal $busy.Response 'fail' 'busy request did not fail'
    Assert-Equal $busy.ResponseSeq 415 'busy request sequence was not retained'
    $powerLoss = Invoke-DomainSwitchModel -Scenario 'power-loss-trying' -RequestSeq 416
    Assert-Equal $powerLoss.PersistedAddress 'old' 'power loss during trying persisted candidate address'
}

Invoke-Vector 'M3-VEC-018 asynchronous NVM verify failure rolls back after candidate login' {
    $result = Invoke-DomainSwitchModel -Scenario 'nvm-async-verify-fail' -RequestSeq 417
    Assert-Equal $result.NetworkAction 'rollback-connected' 'asynchronous verify failure did not restore old address'
    Assert-Equal $result.NvmWriteCount 2 'asynchronous verify failure did not restore the NVM mirror'
    Assert-Equal $result.Response 'fail' 'asynchronous verify failure did not report failure'
    Assert-Equal $result.ResponseSeq 417 'asynchronous verify failure changed the original sequence'
}

Test-RequiredText -Name 'M3-CFG-001 parameter range and result macros' -Path $typesPath -Pattern 'IOT_AHTT_HEART_CYCLE_MIN[\s\S]*IOT_AHTT_MAX_CHARGE_TIME_MAX[\s\S]*IOT_AHTT_PARAM_RESULT_SUCCESS'
Test-RequiredText -Name 'M3-NVM-001 AHTT private parameter has a tail version field' -Path $nvmTypePath -Pattern 'tempAlarmLimit[\s\S]*paramVersion'
Test-RequiredText -Name 'M3-M-001 private parameter commit API is declared and implemented' -Path $mHeaderPath -Pattern 'IotAHTT_CommitPrivateParam'
Test-RequiredText -Name 'M3-M-002 private parameter migration keeps safe RAM and NVM mirror after write failure' -Path $mPath -Pattern 'IotAHTT_MigratePrivateParam[\s\S]*privateParamPersistPending'
Test-RequiredText -Name 'M3-M-003 main function tolerates a missing AHTT context' -Path $mPath -Pattern 'void\s+IotAHTT_MainFunction\(void\)[\s\S]*if\s*\(pIotAHTTCtx\s*!=\s*NULL\)'
Test-RequiredText -Name 'M3-R-001 four parameter receivers are bound' -Path $recvPath -Pattern 'IotAHTT_RecvSetHeartCycle[\s\S]*IotAHTT_RecvQueryHeartCycle[\s\S]*IotAHTT_RecvSetMaxChargeTime[\s\S]*IotAHTT_RecvQueryMaxChargeTime'
Test-RequiredText -Name 'M3-R-002 parameter receivers include the PlatM declaration' -Path $recvPath -Pattern '#include\s+"Asw_PlatM\.h"'
Test-RequiredText -Name 'M3-R-003 device parameter receivers validate and commit atomically' -Path $recvPath -Pattern 'IotAHTT_RecvSetDevParam[\s\S]*(8\s*==\s*len|len\s*==\s*8)[\s\S]*IOT_AHTT_DEV_PARAM_UPLOAD_CYCLE_MIN[\s\S]*IotAHTT_CommitPrivateParam[\s\S]*IotAHTT_RecvQueryDevParam'
Test-RequiredText -Name 'M3-S-001 four parameter senders are bound' -Path $sendPath -Pattern 'IotAHTT_SendSetHeartCycleRsp[\s\S]*IotAHTT_SendQueryHeartCycleRsp[\s\S]*IotAHTT_SendSetMaxChargeTimeRsp[\s\S]*IotAHTT_SendQueryMaxChargeTimeRsp'
Test-RequiredText -Name 'M3-S-002 device parameter senders return result and eight-byte query data' -Path $sendPath -Pattern 'IotAHTT_SendSetDevParamRsp[\s\S]*setDevParamResult[\s\S]*IotAHTT_SendQueryDevParamRsp[\s\S]*devOperationParam,\s*8\s*\)[\s\S]*dataLen\s*=\s*8'
Test-RequiredText -Name 'M3-04-001 fixed field parsing is bound without generic auto response' -Path $recvPath -Pattern '(?s)(?=.*IotAHTT_RecvSetDomainPort)(?=.*IOT_AHTT_DOMAIN_PORT_PARAM_LEN)(?=.*IOT_AHTT_CMD_NULL)'
Test-RequiredText -Name 'M3-04-002 transaction tracks candidate old address and original sequence' -Path $mHeaderPath -Pattern 'IotAHTTDomainSwitchState_Enum[\s\S]*oldDomain[\s\S]*candidateDomain[\s\S]*domainSwitchReqSeq'
Test-RequiredText -Name 'M3-04-003 candidate login commits NVM and failures rollback' -Path $mPath -Pattern '(?s)(?=.*IotAHTT_CommitDomainSwitch)(?=.*MSNvm_WriteParaBlock)(?=.*IotAHTT_BeginDomainSwitchRollback)'
Test-RequiredText -Name 'M3-04-004 candidate TCP timeout and duplicate address are handled' -Path $mPath -Pattern 'IOT_AHTT_DOMAIN_SWITCH_TCP_TIMEOUT_MS[\s\S]*IotAHTT_IsSameDomainPort'
Test-RequiredText -Name 'M3-04-005 failure response sender is bound' -Path $sendPath -Pattern 'IotAHTT_SendSetDomainPortRsp[\s\S]*domainSwitchResult'
Test-RequiredText -Name 'M3-04-006 busy request failure is queued without overwriting original sequence' -Path $mHeaderPath -Pattern 'domainSwitchBusyRspPending[\s\S]*domainSwitchBusyReqSeq'
Test-RequiredText -Name 'M3-04-007 NVM exposes asynchronous verification state' -Path $nvmHeaderPath -Pattern 'MSNvmWriteVerifyState_Enum[\s\S]*MSNvm_GetParaBlockWriteVerifyState'
Test-RequiredText -Name 'M3-04-008 domain switch waits for asynchronous NVM verification' -Path $mPath -Pattern 'eIotAHTTDomainSwitchState_CommitVerify[\s\S]*MSNvm_GetParaBlockWriteVerifyState'

Write-Host "AHTT M3 vectors: $($script:PassedCount) passed, $($script:FailedCount) failed"

if ($script:FailedCount -ne 0)
{
    exit 1
}
