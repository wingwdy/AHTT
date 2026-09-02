$ErrorActionPreference = 'Stop'

$script:PassedCount = 0
$script:FailedCount = 0

function ConvertFrom-HexString
{
    param([string]$Hex)

    $normalized = $Hex -replace '\s', ''
    $data = New-Object byte[] ($normalized.Length / 2)

    for ($index = 0; $index -lt $data.Length; $index++)
    {
        $data[$index] = [Convert]::ToByte($normalized.Substring($index * 2, 2), 16)
    }

    return $data
}

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
    $frame = New-Object System.Collections.Generic.List[byte]
    $frame.AddRange($frameWithoutCrc.ToArray())
    $frame.Add([byte]($crc -band 0xFF))
    $frame.Add([byte](($crc -shr 8) -band 0xFF))
    return $frame.ToArray()
}

function Update-AhttFrameCrc
{
    param([byte[]]$Frame)

    $crcData = $Frame[0..($Frame.Length - 3)]
    $crc = Get-Crc16Modbus -Data $crcData
    $Frame[$Frame.Length - 2] = [byte]($crc -band 0xFF)
    $Frame[$Frame.Length - 1] = [byte](($crc -shr 8) -band 0xFF)
}

function Test-ByteArrayEqual
{
    param(
        [byte[]]$Left,
        [byte[]]$Right
    )

    if ($Left.Length -ne $Right.Length)
    {
        return $false
    }

    for ($index = 0; $index -lt $Left.Length; $index++)
    {
        if ($Left[$index] -ne $Right[$index])
        {
            return $false
        }
    }

    return $true
}

function Get-CheckedByte
{
    param(
        [byte[]]$Data,
        [int]$Index,
        [ref]$MaxReadIndex
    )

    if (($Index -lt 0) -or ($Index -ge $Data.Length))
    {
        throw "byte access out of range: index=$Index length=$($Data.Length)"
    }

    if ($Index -gt $MaxReadIndex.Value)
    {
        $MaxReadIndex.Value = $Index
    }

    return $Data[$Index]
}

function Invoke-AhttBlockParser
{
    param(
        [byte[]]$Data,
        [byte[]]$ExpectedDeviceNum
    )

    $index = 0
    $parsedCount = 0
    $resyncCount = 0
    $lengthRejectCount = 0
    $versionRejectCount = 0
    $deviceRejectCount = 0
    $crcRejectCount = 0
    $maxReadIndex = -1
    $acceptedOffsets = New-Object System.Collections.Generic.List[int]
    $acceptedSeqs = New-Object System.Collections.Generic.List[int]

    while ($index -lt $Data.Length)
    {
        while ($index -lt $Data.Length)
        {
            $value = Get-CheckedByte -Data $Data -Index $index -MaxReadIndex ([ref]$maxReadIndex)
            if ($value -eq 0xEA)
            {
                break
            }
            $index++
        }

        if (($Data.Length - $index) -lt 12)
        {
            break
        }

        $declareLenLow = Get-CheckedByte -Data $Data -Index ($index + 1) -MaxReadIndex ([ref]$maxReadIndex)
        $declareLenHigh = Get-CheckedByte -Data $Data -Index ($index + 2) -MaxReadIndex ([ref]$maxReadIndex)
        $declareLen = $declareLenLow -bor ([int]$declareLenHigh -shl 8)
        $frameLen = $declareLen + 3
        if (($declareLen -lt 11) -or ($frameLen -gt 512))
        {
            $index++
            $resyncCount++
            $lengthRejectCount++
            continue
        }

        if (($Data.Length - $index) -lt $frameLen)
        {
            break
        }

        $version = Get-CheckedByte -Data $Data -Index ($index + 3) -MaxReadIndex ([ref]$maxReadIndex)
        if ($version -ne 0x01)
        {
            $index++
            $resyncCount++
            $versionRejectCount++
            continue
        }

        $deviceNum = New-Object byte[] 5
        for ($deviceIndex = 0; $deviceIndex -lt $deviceNum.Length; $deviceIndex++)
        {
            $deviceNum[$deviceIndex] = Get-CheckedByte -Data $Data -Index ($index + 4 + $deviceIndex) -MaxReadIndex ([ref]$maxReadIndex)
        }
        if (-not (Test-ByteArrayEqual -Left $deviceNum -Right $ExpectedDeviceNum))
        {
            $index++
            $resyncCount++
            $deviceRejectCount++
            continue
        }

        $crcOffset = $index + $frameLen - 2
        $crcLow = Get-CheckedByte -Data $Data -Index $crcOffset -MaxReadIndex ([ref]$maxReadIndex)
        $crcHigh = Get-CheckedByte -Data $Data -Index ($crcOffset + 1) -MaxReadIndex ([ref]$maxReadIndex)
        $onlineCrc = $crcLow -bor ([int]$crcHigh -shl 8)
        $crcData = New-Object byte[] ($crcOffset - $index)
        for ($crcIndex = 0; $crcIndex -lt $crcData.Length; $crcIndex++)
        {
            $crcData[$crcIndex] = Get-CheckedByte -Data $Data -Index ($index + $crcIndex) -MaxReadIndex ([ref]$maxReadIndex)
        }
        $calculatedCrc = Get-Crc16Modbus -Data $crcData

        if ($calculatedCrc -ne $onlineCrc)
        {
            $index++
            $resyncCount++
            $crcRejectCount++
            continue
        }

        $seqLow = Get-CheckedByte -Data $Data -Index ($index + 9) -MaxReadIndex ([ref]$maxReadIndex)
        $seqHigh = Get-CheckedByte -Data $Data -Index ($index + 10) -MaxReadIndex ([ref]$maxReadIndex)
        $seq = $seqLow -bor ([int]$seqHigh -shl 8)
        $acceptedOffsets.Add($index)
        $acceptedSeqs.Add($seq)
        $parsedCount++
        $index += $frameLen
    }

    return [PSCustomObject]@{
        ParsedCount = $parsedCount
        ResyncCount = $resyncCount
        LengthRejectCount = $lengthRejectCount
        VersionRejectCount = $versionRejectCount
        DeviceRejectCount = $deviceRejectCount
        CrcRejectCount = $crcRejectCount
        MaxReadIndex = $maxReadIndex
        AcceptedOffsets = $acceptedOffsets.ToArray()
        AcceptedSeqs = $acceptedSeqs.ToArray()
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

$deviceNum = [byte[]](0x00, 0x00, 0x30, 0x00, 0x34)
$normalFrame = New-AhttFrame -DeviceNum $deviceNum -Seq 1 -Cmd 0x95

Invoke-Vector 'M1-VEC-001 complete zero-parameter frame' {
    $loginData = ConvertFrom-HexString 'EA270001000040003401000138393836303431363132313938303030343135310100000000000000'
    Assert-Equal (Get-Crc16Modbus -Data $loginData) 0xA53A 'login example CRC mismatch'
    Assert-Equal (ConvertTo-HexString -Data $normalFrame) 'EA0B000100003000340100954C3F' 'golden frame mismatch'
    Assert-Equal $normalFrame.Length 14 'zero-parameter frame length mismatch'
    $result = Invoke-AhttBlockParser -Data $normalFrame -ExpectedDeviceNum $deviceNum
    Assert-Equal $result.ParsedCount 1 'complete frame was not parsed'
    Assert-Equal $result.AcceptedOffsets[0] 0 'complete frame accepted at wrong offset'
    Assert-Equal $result.AcceptedSeqs[0] 1 'complete frame sequence mismatch'
}

Invoke-Vector 'M1-VEC-002 split frame across two FrameQueue blocks' {
    for ($split = 1; $split -le 13; $split++)
    {
        $first = $normalFrame[0..($split - 1)]
        $second = $normalFrame[$split..($normalFrame.Length - 1)]
        $firstResult = Invoke-AhttBlockParser -Data $first -ExpectedDeviceNum $deviceNum
        $secondResult = Invoke-AhttBlockParser -Data $second -ExpectedDeviceNum $deviceNum
        Assert-Equal ($firstResult.ParsedCount + $secondResult.ParsedCount) 0 "split position $split was incorrectly dispatched"
        Assert-Equal ($firstResult.MaxReadIndex -lt $first.Length) $true "first block at split position $split read out of range"
        Assert-Equal ($secondResult.MaxReadIndex -lt $second.Length) $true "second block at split position $split read out of range"
    }
}

Invoke-Vector 'M1-VEC-003 two complete frames in one block' {
    $data = [byte[]]($normalFrame + $normalFrame)
    $result = Invoke-AhttBlockParser -Data $data -ExpectedDeviceNum $deviceNum
    Assert-Equal $result.ParsedCount 2 'two complete frames were not both parsed'
    Assert-Equal $result.AcceptedOffsets[0] 0 'first complete frame accepted at wrong offset'
    Assert-Equal $result.AcceptedOffsets[1] 14 'second complete frame accepted at wrong offset'
}

Invoke-Vector 'M1-VEC-004 leading noise before a complete frame' {
    $data = [byte[]](@(0x12, 0x34, 0x56) + $normalFrame)
    $result = Invoke-AhttBlockParser -Data $data -ExpectedDeviceNum $deviceNum
    Assert-Equal $result.ParsedCount 1 'frame after leading noise was not parsed'
    Assert-Equal $result.AcceptedOffsets[0] 3 'leading noise was not skipped exactly'
}

Invoke-Vector 'M1-VEC-005 bad CRC followed by a valid frame' {
    $badFrame = [byte[]]$normalFrame.Clone()
    $badFrame[$badFrame.Length - 1] = $badFrame[$badFrame.Length - 1] -bxor 0xFF
    $data = [byte[]]($badFrame + $normalFrame)
    $result = Invoke-AhttBlockParser -Data $data -ExpectedDeviceNum $deviceNum
    Assert-Equal $result.ParsedCount 1 'valid frame after bad CRC was not parsed'
    Assert-Equal $result.CrcRejectCount 1 'bad CRC frame was not rejected for CRC'
    Assert-Equal $result.AcceptedOffsets[0] 14 'wrong frame was accepted after bad CRC'
}

Invoke-Vector 'M1-VEC-006 bad version followed by a valid frame' {
    $badFrame = [byte[]]$normalFrame.Clone()
    $badFrame[3] = 0x02
    Update-AhttFrameCrc -Frame $badFrame
    $data = [byte[]]($badFrame + $normalFrame)
    $result = Invoke-AhttBlockParser -Data $data -ExpectedDeviceNum $deviceNum
    Assert-Equal $result.ParsedCount 1 'valid frame after bad version was not parsed'
    Assert-Equal $result.VersionRejectCount 1 'bad version frame was not rejected for version'
    Assert-Equal $result.AcceptedOffsets[0] 14 'wrong frame was accepted after bad version'
}

Invoke-Vector 'M1-VEC-007 bad device number followed by a valid frame' {
    $badFrame = [byte[]]$normalFrame.Clone()
    $badFrame[4] = 0x01
    Update-AhttFrameCrc -Frame $badFrame
    $data = [byte[]]($badFrame + $normalFrame)
    $result = Invoke-AhttBlockParser -Data $data -ExpectedDeviceNum $deviceNum
    Assert-Equal $result.ParsedCount 1 'valid frame after bad device number was not parsed'
    Assert-Equal $result.DeviceRejectCount 1 'bad device frame was not rejected for device number'
    Assert-Equal $result.AcceptedOffsets[0] 14 'wrong frame was accepted after bad device number'
}

Invoke-Vector 'M1-VEC-008 declared length below minimum' {
    $data = [byte[]](0xEA, 0x0A, 0x00, 0x01, 0x00, 0x00, 0x30, 0x00, 0x34, 0x01, 0x00, 0x95, 0x00, 0x00)
    $result = Invoke-AhttBlockParser -Data $data -ExpectedDeviceNum $deviceNum
    Assert-Equal $result.ParsedCount 0 'short declared length was incorrectly parsed'
    Assert-Equal $result.ResyncCount 1 'short declared length did not resynchronize by one byte'
    Assert-Equal $result.LengthRejectCount 1 'short declared length was not rejected for length'
}

Invoke-Vector 'M1-VEC-009 declared length above frame maximum' {
    $data = [byte[]](0xEA, 0xFE, 0x01, 0x01, 0x00, 0x00, 0x30, 0x00, 0x34, 0x01, 0x00, 0x95, 0x00, 0x00)
    $result = Invoke-AhttBlockParser -Data $data -ExpectedDeviceNum $deviceNum
    Assert-Equal $result.ParsedCount 0 'oversized declared length was incorrectly parsed'
    Assert-Equal $result.ResyncCount 1 'oversized declared length did not resynchronize by one byte'
    Assert-Equal $result.LengthRejectCount 1 'oversized declared length was not rejected for length'
}

Write-Host "AHTT M1 vectors: $($script:PassedCount) passed, $($script:FailedCount) failed"

if ($script:FailedCount -ne 0)
{
    exit 1
}
