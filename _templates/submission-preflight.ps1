param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$ModDirectory,

    [string]$PrAuthor = 'sb4ssman',

    [string]$UpstreamRepository
)

# Runs the local checks that previously lived only in the submission checklist,
# then invokes the current validator from a sibling windhawk-mods checkout.
# PR-body/authorship and changed-file-count checks still require the PR checklist.

$ErrorActionPreference = 'Stop'
$modPath = (Resolve-Path -LiteralPath $ModDirectory).Path
$modName = Split-Path -Leaf $modPath
$repoRoot = Split-Path -Parent $PSScriptRoot
$sourceFile = Get-ChildItem -LiteralPath $modPath -Filter '*.wh.cpp'
if ($sourceFile.Count -ne 1) {
    throw "Expected exactly one .wh.cpp source in $modPath, found $($sourceFile.Count)"
}
$sourceFile = $sourceFile[0]

& (Join-Path $PSScriptRoot 'compile-check.ps1') $modPath
if ($LASTEXITCODE -ne 0) { throw "$modName compile check failed" }

& (Join-Path $PSScriptRoot 'exit-time-destructor-audit.ps1') $modPath
if ($LASTEXITCODE -ne 0) { throw "$modName exit-time destructor audit failed" }

& (Join-Path $PSScriptRoot 'verify-readme-sync.ps1') $modPath
if ($LASTEXITCODE -ne 0) { throw "$modName README parity check failed" }

$readmePath = Join-Path $modPath 'README.md'
$readme = Get-Content -Raw -LiteralPath $readmePath
$imagePattern = '!\[[^\]]*\]\((?<target><[^>]+>|[^)\s]+)(?:\s+["''][^)]*)?\)'
foreach ($match in [regex]::Matches($readme, $imagePattern)) {
    $target = $match.Groups['target'].Value.Trim('<', '>')
    if ($target -match '^(?i:https?://|data:)') { continue }
    $target = [System.Uri]::UnescapeDataString($target).Replace('/', [IO.Path]::DirectorySeparatorChar)
    $targetPath = Join-Path $modPath $target
    if (-not (Test-Path -LiteralPath $targetPath -PathType Leaf)) {
        throw "Broken local README image link: $target"
    }
}

$assetsPath = Join-Path $modPath 'assets'
if (Test-Path -LiteralPath $assetsPath -PathType Container) {
    $imageExtensions = @('.png', '.jpg', '.jpeg', '.gif', '.webp', '.bmp')
    $unreferenced = Get-ChildItem -LiteralPath $assetsPath -File |
        Where-Object { $imageExtensions -contains $_.Extension.ToLowerInvariant() } |
        Where-Object { $readme -notmatch [regex]::Escape("assets/$($_.Name)") }
    if ($unreferenced) {
        $names = ($unreferenced.Name | Sort-Object) -join ', '
        throw "Unreferenced assets must be included, archived, or deliberately removed: $names"
    }
}

# Catch the exact policy failure that escaped local Clang during the Folder
# Menus submission. The upstream validator below performs the complete check.
$source = Get-Content -Raw -LiteralPath $sourceFile.FullName
$idMatch = [regex]::Match($source, '(?m)^//\s*@id\s+(?<id>\S+)\s*$')
if (-not $idMatch.Success) { throw 'Windhawk @id metadata not found' }
$submissionFileName = $idMatch.Groups['id'].Value + '.wh.cpp'
$genericHookPattern = '(?m)^\s*(?:(?:static|const)\s+)*(?:WindhawkUtils::)?SYMBOL_HOOK\s+(hooks?|symbolHooks)\b'
$sourceLines = $source -split "`r?`n"
foreach ($match in [regex]::Matches($source, $genericHookPattern)) {
    $lineNumber = 1 + ($source.Substring(0, $match.Index) -split "`n").Count - 1
    $previousLine = if ($lineNumber -gt 1) { $sourceLines[$lineNumber - 2].Trim() } else { '' }
    if ($previousLine -notmatch '^//\s*.+\.(dll|exe|cpl)(\s*,|\s*$)') {
        throw 'Generic SYMBOL_HOOK array name found without an immediately preceding target-module comment.'
    }
}

& git -C $repoRoot diff --check
if ($LASTEXITCODE -ne 0) { throw 'git diff --check failed' }

if (-not $UpstreamRepository) {
    $UpstreamRepository = Join-Path (Split-Path -Parent $repoRoot) 'windhawk-mods'
}
$validator = Join-Path $UpstreamRepository '.github\pr_validation.py'
$validatorDirectory = Split-Path -Parent $validator
if (-not (Test-Path -LiteralPath $validator -PathType Leaf)) {
    throw "Current upstream validator not found at $validator"
}

$python = Get-Command python -ErrorAction Stop
$escapedValidator = $validator.Replace("'", "''")
$escapedValidatorDirectory = $validatorDirectory.Replace("'", "''")
$shim = @"
import enum, runpy, sys
sys.path.insert(0, r'$escapedValidatorDirectory')
if not hasattr(enum, 'StrEnum'):
    class StrEnum(str, enum.Enum):
        @staticmethod
        def _generate_next_value_(name, start, count, last_values):
            return name.lower()
    enum.StrEnum = StrEnum
runpy.run_path(r'$escapedValidator', run_name='__main__')
"@

$validationRoot = Join-Path ([IO.Path]::GetTempPath()) (
    'windhawk-submission-preflight-' + [guid]::NewGuid().ToString('N'))
$validationMods = Join-Path $validationRoot 'mods'
New-Item -ItemType Directory -Path $validationMods | Out-Null
$stagedSource = Join-Path $validationMods $submissionFileName
Copy-Item -LiteralPath $sourceFile.FullName -Destination $stagedSource

try {
    Push-Location $validationRoot
    try {
        $relativeSource = Join-Path 'mods' $submissionFileName
        $oldPythonIoEncoding = $env:PYTHONIOENCODING
        $env:PYTHONIOENCODING = 'utf-8'
        try {
            $validatorOutput = & $python.Source -c $shim $relativeSource $PrAuthor 2>&1
            $validatorExitCode = $LASTEXITCODE
        } finally {
            $env:PYTHONIOENCODING = $oldPythonIoEncoding
        }
    } finally {
        Pop-Location
    }

    $validatorOutput | ForEach-Object { Write-Output $_ }
    if ($validatorExitCode -ne 0) {
        throw "Upstream Windhawk PR validator failed with exit code $validatorExitCode"
    }
    if (($validatorOutput -join "`n") -match 'Got\s+[1-9][0-9]*\s+warnings') {
        throw 'Upstream Windhawk PR validator reported warnings'
    }
} finally {
    $resolvedTemp = [IO.Path]::GetFullPath([IO.Path]::GetTempPath())
    $resolvedValidationRoot = [IO.Path]::GetFullPath($validationRoot)
    if ($resolvedValidationRoot.StartsWith($resolvedTemp, [StringComparison]::OrdinalIgnoreCase) -and
        (Split-Path -Leaf $resolvedValidationRoot) -like 'windhawk-submission-preflight-*') {
        Remove-Item -LiteralPath $resolvedValidationRoot -Recurse -Force
    }
}

Write-Output "$modName SUBMISSION_PREFLIGHT_OK"
