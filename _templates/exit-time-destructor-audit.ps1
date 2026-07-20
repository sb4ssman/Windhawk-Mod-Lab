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

$clang = 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe'
$include = 'C:\Program Files\Windhawk\Compiler\include'
if (-not (Test-Path -LiteralPath $clang)) {
    throw "Windhawk clang not found at $clang"
}

& $clang -std=c++23 -target x86_64-w64-mingw32 -fsyntax-only `
    -Wexit-time-destructors -Werror=exit-time-destructors `
    -DUNICODE -D_UNICODE -DWH_MOD -DWH_EDITING `
    "-DWH_MOD_ID=L`"$modName`"" '-DWH_MOD_VERSION=L"0.0"' `
    -I $include -include windows.h -include windhawk_api.h `
    $sourceFile.FullName

if ($LASTEXITCODE -eq 0) {
    Write-Output "$modName EXIT_TIME_DESTRUCTOR_AUDIT_OK"
} else {
    Write-Error "$modName EXIT_TIME_DESTRUCTOR_REVIEW_REQUIRED"
    exit 1
}
