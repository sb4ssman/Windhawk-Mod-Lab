param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$ModDirectory
)

$ErrorActionPreference = 'Stop'
$modPath = (Resolve-Path -LiteralPath $ModDirectory).Path
$modName = Split-Path -Leaf $modPath
$sourceFile = Get-ChildItem -LiteralPath $modPath -Filter '*.wh.cpp' |
    Select-Object -First 1

if (-not $sourceFile) {
    throw "No .wh.cpp source found in $modPath"
}

$readmePath = Join-Path $modPath 'README.md'
if (-not (Test-Path -LiteralPath $readmePath)) {
    throw "No README.md found in $modPath"
}

$source = Get-Content -Raw -LiteralPath $sourceFile.FullName
$match = [regex]::Match(
    $source,
    '(?s)// ==WindhawkModReadme==\s*/\*\r?\n(.*?)\r?\n\*/\s*// ==/WindhawkModReadme=='
)
if (-not $match.Success) {
    throw "WindhawkModReadme block not found in $($sourceFile.Name)"
}

$embedded = $match.Groups[1].Value
$folderReadme = Get-Content -Raw -LiteralPath $readmePath
$rawAssetPrefix =
    "https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/$modName/"
$repoBlobPrefix =
    'https://github.com/sb4ssman/Windhawk-Mod-Lab/blob/main/'

$embedded = $embedded.Replace($rawAssetPrefix, '')
$embedded = $embedded.Replace($repoBlobPrefix, '../') -replace "`r`n", "`n"
$folderReadme = $folderReadme -replace "`r`n", "`n"

if ($embedded.Trim() -cne $folderReadme.Trim()) {
    Write-Error "$modName README_MISMATCH"
    exit 1
}

Write-Output "$modName README_MATCH"
