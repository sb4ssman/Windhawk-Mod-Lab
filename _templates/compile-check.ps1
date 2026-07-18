param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$ModDirectory
)

# Syntax-checks a mod with Windhawk's bundled clang, mimicking the real build
# closely enough to catch compile errors before spending an Explorer restart.

$ErrorActionPreference = 'Stop'
$modPath = (Resolve-Path -LiteralPath $ModDirectory).Path
$modName = Split-Path -Leaf $modPath
$sourceFile = Get-ChildItem -LiteralPath $modPath -Filter '*.wh.cpp' | Select-Object -First 1
if (-not $sourceFile) { throw "No .wh.cpp source found in $modPath" }

$clang = 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe'
$include = 'C:\Program Files\Windhawk\Compiler\include'
if (-not (Test-Path $clang)) { throw "Windhawk clang not found at $clang" }

& $clang -std=c++23 -target x86_64-w64-mingw32 -fsyntax-only `
    -DUNICODE -D_UNICODE -DWH_MOD -DWH_EDITING `
    "-DWH_MOD_ID=L`"$modName`"" "-DWH_MOD_VERSION=L`"0.0`"" `
    -I $include -include windows.h -include windhawk_api.h `
    $sourceFile.FullName

if ($LASTEXITCODE -eq 0) { Write-Output "$modName COMPILE_OK" }
else { Write-Error "$modName COMPILE_FAILED"; exit 1 }
