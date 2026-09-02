$ErrorActionPreference = "Stop"
$toolRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$python = "C:\Users\Administrator\AppData\Local\Programs\Python\Python312\python.exe"

Set-Location $toolRoot
if (-not (Test-Path $python)) {
    $python = "python"
}
& $python -m PyInstaller --noconfirm --clean --onefile --windowed --name "AP_Frame_Parser" app.py
Write-Host "Build complete: $toolRoot\dist\AP_Frame_Parser.exe"
