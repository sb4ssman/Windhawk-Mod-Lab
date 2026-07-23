param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$ModDirectory
)

# Windhawk injects mods into long-lived system processes, and process shutdown
# doesn't guarantee Wh_ModUninit. Make every namespace-scope destructor an
# explicit design decision instead of allowing CRT teardown to release XAML or
# WinRT state after the framework has already gone away.

$ErrorActionPreference = 'Stop'
$modPath = (Resolve-Path -LiteralPath $ModDirectory).Path
$modName = Split-Path -Leaf $modPath
$sourceFile = Get-ChildItem -LiteralPath $modPath -Filter '*.wh.cpp' | Select-Object -First 1
if (-not $sourceFile) { throw "No .wh.cpp source found in $modPath" }
$source = Get-Content -LiteralPath $sourceFile.FullName -Raw

# Clang detects unguarded exit-time destructors, but it can't tell whether
# no_destroy was applied too broadly or whether a no_destroy container frees
# its backing allocation on controlled unload. Enforce lifecycle template
# v1.3's ownership shapes before compiling. Strip // comment-only lines so the
# copy-source contract examples don't look like live declarations.
$declarationSource = (($source -split "`r?`n") | Where-Object {
    -not $_.TrimStart().StartsWith('//')
}) -join "`n"
$ownershipErrors = [System.Collections.Generic.List[string]]::new()

$containerKinds = 'vector|list|deque|map|multimap|unordered_map|set|multiset|unordered_set'
$bareContainerPattern = "(?s)\[\[clang::no_destroy\]\]\s*(?:static\s+)?std::(?:$containerKinds)\s*<.*?>\s+(?<name>g_[A-Za-z0-9_]+)\s*(?:\{.*?\}|=.*?)?;"
foreach ($match in [regex]::Matches($declarationSource, $bareContainerPattern)) {
    $ownershipErrors.Add(
        "$($match.Groups['name'].Value): bare no_destroy container; use no_destroy optional<container>")
}

$settingsPattern = '(?s)\[\[clang::no_destroy\]\]\s*(?:static\s+)?(?:Mod)?Settings\s+(?<name>g_[A-Za-z0-9_]+)\s*(?:\{.*?\}|=.*?)?;'
foreach ($match in [regex]::Matches($declarationSource, $settingsPattern)) {
    $ownershipErrors.Add(
        "$($match.Groups['name'].Value): settings are heap-only; remove no_destroy")
}

$heapOnlyPattern = '(?s)\[\[clang::no_destroy\]\]\s*(?:static\s+)?(?:std::(?:w?string)|lease_column::Lease)\s+(?<name>g_[A-Za-z0-9_]+)\s*(?:\{.*?\}|=.*?)?;'
foreach ($match in [regex]::Matches($declarationSource, $heapOnlyPattern)) {
    $ownershipErrors.Add(
        "$($match.Groups['name'].Value): heap-only owner; remove no_destroy")
}

$optionalContainerPattern = "(?s)\[\[clang::no_destroy\]\]\s*(?:static\s+)?std::optional\s*<\s*std::(?:$containerKinds)\s*<.*?>\s*>\s+(?<name>g_[A-Za-z0-9_]+)\s*(?:\{.*?\}|=.*?)?;"
foreach ($match in [regex]::Matches($declarationSource, $optionalContainerPattern)) {
    $name = $match.Groups['name'].Value
    if ($source -notmatch ([regex]::Escape($name) + '\s*\.\s*reset\s*\(')) {
        $ownershipErrors.Add("${name}: no controlled-unload reset() found")
    }
}

if ($ownershipErrors.Count -gt 0) {
    foreach ($errorText in $ownershipErrors) {
        Write-Output "$modName NO_DESTROY_OWNERSHIP_ERROR: $errorText"
    }
    exit 1
}

$clang = 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe'
$include = 'C:\Program Files\Windhawk\Compiler\include'
if (-not (Test-Path -LiteralPath $clang)) {
    throw "Windhawk clang not found at $clang"
}

$savedErrorActionPreference = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
$compilerOutput = & $clang -std=c++23 -target x86_64-w64-mingw32 -fsyntax-only `
    -Wexit-time-destructors `
    -DUNICODE -D_UNICODE -DWH_MOD -DWH_EDITING `
    "-DWH_MOD_ID=L`"$modName`"" '-DWH_MOD_VERSION=L"0.0"' `
    -I $include -include windows.h -include windhawk_api.h `
    $sourceFile.FullName 2>&1
$compilerExitCode = $LASTEXITCODE
$ErrorActionPreference = $savedErrorActionPreference

if ($compilerExitCode -ne 0) {
    $compilerOutput | ForEach-Object { Write-Output $_ }
    Write-Error "$modName EXIT_TIME_DESTRUCTOR_REVIEW_REQUIRED"
    exit 1
}

# Heap-only globals should destruct normally, but Clang still warns about them.
# Require an explicit same-line marker so each exception stays reviewable:
#   static ModSettings g_settings;  // exit-time-safe: heap-only
$sourceLines = $source -split "`r?`n"
$unapprovedDestructorWarnings = [System.Collections.Generic.List[string]]::new()
foreach ($diagnostic in $compilerOutput) {
    $text = "$diagnostic"
    if ($text -match ':(?<line>[0-9]+):[0-9]+: warning: declaration requires an exit-time destructor') {
        $lineNumber = [int]$Matches['line']
        $sourceLine = if ($lineNumber -ge 1 -and $lineNumber -le $sourceLines.Count) {
            $sourceLines[$lineNumber - 1]
        } else {
            ''
        }
        if ($sourceLine -notmatch 'exit-time-safe:\s*heap-only') {
            $unapprovedDestructorWarnings.Add($text)
        }
    }
}

if ($unapprovedDestructorWarnings.Count -gt 0) {
    $unapprovedDestructorWarnings | ForEach-Object { Write-Output $_ }
    Write-Error "$modName EXIT_TIME_DESTRUCTOR_REVIEW_REQUIRED"
    exit 1
}

Write-Output "$modName EXIT_TIME_DESTRUCTOR_AUDIT_OK"
