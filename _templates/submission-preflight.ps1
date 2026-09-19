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

# ---- Lessons from upstream AI reviews -------------------------------------
# Each of these encodes a finding that reached a real pull request. Only checks
# that can be decided mechanically live here; judgement calls that need the call
# graph stay in submission-checklist.md, because a preflight that cries wolf
# gets ignored.

# PR #4855: a declared setting no code reads. Windhawk has no read-only control,
# so an inert box silently does nothing when the user edits it. This also
# catches the far more dangerous case of a setting renamed in the block but not
# in the loader, which Windhawk answers with a default rather than an error.
$pythonCommand = Get-Command python -ErrorAction Stop
& $pythonCommand (Join-Path $PSScriptRoot 'verify-settings-used.py') $sourceFile.FullName
if ($LASTEXITCODE -ne 0) { throw "$modName has settings that nothing reads" }

# PR #4855: preferring a cached Shell_TrayWnd without validating it. The window
# can be recreated in-process, and the stale handle then wins forever — on the
# unload path that skips teardown entirely and leaves callbacks pointing into a
# freed image. taskbar-host.h::ResolveTaskbarWnd is the validated form.
$staleHandlePattern = '(?<var>g_\w*[Tt]askbar\w*)\s*\?\s*\k<var>\s*:\s*FindCurrentProcessTaskbarWnd\s*\('
foreach ($match in [regex]::Matches($source, $staleHandlePattern)) {
    # The template documents the anti-pattern in a comment; that is not a use.
    $lineStart = $source.LastIndexOf("`n", $match.Index) + 1
    $line = $source.Substring($lineStart, $match.Index - $lineStart)
    if ($line.TrimStart().StartsWith('//')) { continue }
    throw ("Cached taskbar handle used without validation. " +
           "Use tbh::ResolveTaskbarWnd(...) so a recreated Shell_TrayWnd " +
           "cannot leave a dead handle winning forever.")
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

# The upstream validator's dependencies now use PEP 695 `type` alias statements
# (.github/preprocessor.py), which are a SYNTAX ERROR before Python 3.12 — the
# whole step fails to import on an older interpreter and reports itself as a
# validation failure, which reads as "the mod is broken" when nothing is wrong
# with it. Pick a new enough interpreter explicitly rather than whatever
# `python` happens to be on PATH.
# A version match alone is not enough: the validator also imports pyyaml, and
# the newest interpreter on a machine is often the one with nothing installed
# in it. Probe for BOTH, so this picks an interpreter that can actually run.
$probe = 'import sys, yaml; print(sys.version_info[:2] >= (3, 12))'
$python = $null
foreach ($version in @('3.12', '3.13', '3.14')) {
    $candidate = & py "-$version" -c 'import sys; print(sys.executable)' 2>$null
    if ($LASTEXITCODE -ne 0 -or -not $candidate) { continue }
    $candidate = $candidate.Trim()
    $usable = & $candidate -c $probe 2>$null
    if ($LASTEXITCODE -eq 0 -and $usable -eq 'True') {
        $python = $candidate
        break
    }
}
if (-not $python) {
    $onPath = Get-Command python -ErrorAction SilentlyContinue
    if ($onPath) {
        $usable = & $onPath.Source -c $probe 2>$null
        if ($LASTEXITCODE -eq 0 -and $usable -eq 'True') { $python = $onPath.Source }
    }
}
if (-not $python) {
    throw ('The upstream PR validator needs Python 3.12 or newer (its ' +
           'dependencies use PEP 695 `type` aliases) WITH pyyaml installed. ' +
           'No interpreter satisfying both was found — install pyyaml into a ' +
           '3.12+ interpreter, e.g. "py -3.12 -m pip install pyyaml".')
}
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
            $validatorOutput = & $python -c $shim $relativeSource $PrAuthor 2>&1
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
