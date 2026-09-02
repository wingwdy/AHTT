[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$Failures = [System.Collections.Generic.List[string]]::new()

function Add-Failure {
    param([string]$Message)

    $script:Failures.Add($Message)
}

function Assert-Exists {
    param([string]$RelativePath)

    if (-not (Test-Path -LiteralPath (Join-Path $ProjectRoot $RelativePath))) {
        Add-Failure "缺少必需文件：$RelativePath"
    }
}

function Assert-NoMatch {
    param(
        [string]$RelativePath,
        [string]$Pattern,
        [string]$Description
    )

    $Path = Join-Path $ProjectRoot $RelativePath
    if ((Test-Path -LiteralPath $Path) -and (Select-String -LiteralPath $Path -Pattern $Pattern -Quiet)) {
        Add-Failure "$RelativePath：$Description"
    }
}

$RequiredFiles = @(
    "AGENTS.md",
    ".agents/skills/ahtt-development/SKILL.md",
    ".agents/skills/ahtt-development/agents/openai.yaml",
    ".agents/skills/ahtt-bore-sim-validation/SKILL.md",
    ".agents/skills/ahtt-bore-sim-validation/agents/openai.yaml",
    ".agents/skills/ahtt-bore-sim-validation/references/AHTT-Bore模拟联调运行手册.md",
    "docs/ai/AHTT_AI_Development_Prompts.md",
    "docs/ai/README.md",
    "docs/ai/plans/README.md",
    "docs/superpowers/README.md",
    "tools/ahtt/README-ahtt-platform-sim.md"
)

foreach ($RelativePath in $RequiredFiles) {
    Assert-Exists $RelativePath
}

$RuleFiles = @(
    "AGENTS.md",
    ".agents/skills/ahtt-development/SKILL.md",
    ".agents/skills/ahtt-bore-sim-validation/SKILL.md"
)

foreach ($RelativePath in $RuleFiles) {
    Assert-NoMatch $RelativePath "(?m)^\s*-\s+#{1,6}\s" "存在被列表包装的 Markdown 标题"
}

$PromptFiles = @(
    ".agents/skills/ahtt-development/SKILL.md",
    ".agents/skills/ahtt-bore-sim-validation/SKILL.md",
    ".agents/skills/ahtt-bore-sim-validation/references/AHTT-Bore模拟联调运行手册.md"
)

foreach ($RelativePath in $PromptFiles) {
    Assert-NoMatch $RelativePath "(?<!docs/ai/)AHTT AI Development Prompts\.md" "仍引用旧 Prompt 路径"
}

Assert-NoMatch "docs/ahtt/AHTT-Bore模拟平台联调手册.md" "默认.*(COM\d+|www\.)" "把历史 COM 或地址写为当前默认"
Assert-NoMatch "tools/ahtt/README-ahtt-platform-sim.md" "预期：\s*4个" "固定历史测试数量"
Assert-NoMatch "tools/ahtt/README-ahtt-platform-sim.md" "www\.ahttcd\.cn" "硬编码历史恢复地址"

$GnReference = Join-Path $ProjectRoot ".agents/skills/ahtt-development/references/GN同构与校验边界.md"
if ((Test-Path -LiteralPath $GnReference) -and -not (Select-String -LiteralPath $GnReference -Pattern "GN Mapping: N/A" -Quiet)) {
    Add-Failure "GN 同构参考未支持 GN Mapping: N/A"
}

Assert-NoMatch "docs/ahtt/AHTT-V3.12-命令追踪矩阵.md" "候选未建连，板端路径未通过" "仍保留已过期的 CMD-04 结论"

$TrackedGenerated = @(& git -C $ProjectRoot ls-files | Where-Object {
    $_ -match "(^|/)(__pycache__/|tmp/ahtt_sim/|tmp/pdfs/)" -or $_ -match "\.pyc$"
})
if ($TrackedGenerated.Count -gt 0) {
    Add-Failure ("Git 仍跟踪运行时产物：" + ($TrackedGenerated -join ", "))
}

$ValidationScripts = @(Get-ChildItem -LiteralPath $PSScriptRoot -Filter "Validate-AHTTM*.ps1" -File -ErrorAction SilentlyContinue)
$PythonTests = @(Get-ChildItem -LiteralPath $PSScriptRoot -Filter "test_*.py" -File -ErrorAction SilentlyContinue)
$Simulators = @(Get-ChildItem -LiteralPath $PSScriptRoot -Filter "*_sim.py" -File -ErrorAction SilentlyContinue)

if ($ValidationScripts.Count -eq 0) {
    Add-Failure "未动态发现 Validate-AHTTM*.ps1"
}
if ($PythonTests.Count -eq 0) {
    Add-Failure "未动态发现 test_*.py"
}
if ($Simulators.Count -eq 0) {
    Add-Failure "未动态发现 *_sim.py"
}

if ($Failures.Count -gt 0) {
    Write-Host "AHTT AI WORKFLOW VALIDATION: FAIL"
    $Failures | ForEach-Object { Write-Host "- $_" }
    exit 1
}

Write-Host "AHTT AI WORKFLOW VALIDATION: PASS"
Write-Host ("Discovered validation scripts: " + (($ValidationScripts.Name) -join ", "))
Write-Host ("Discovered Python tests: " + (($PythonTests.Name) -join ", "))
Write-Host ("Discovered simulators: " + (($Simulators.Name) -join ", "))
