param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$ModDirectory
)

# Compiles and links a temporary mod DLL with Windhawk's bundled clang. Linking
# is intentional: a syntax-only check cannot catch missing @compilerOptions
# libraries such as oleaut32 for C++/WinRT BSTR helpers.

$ErrorActionPreference = 'Stop'
$modPath = (Resolve-Path -LiteralPath $ModDirectory).Path
$modName = Split-Path -Leaf $modPath
$sourceFile = Get-ChildItem -LiteralPath $modPath -Filter '*.wh.cpp' | Select-Object -First 1
if (-not $sourceFile) { throw "No .wh.cpp source found in $modPath" }

$clang = 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe'
$include = 'C:\Program Files\Windhawk\Compiler\include'
if (-not (Test-Path $clang)) { throw "Windhawk clang not found at $clang" }

$source = Get-Content -Raw -LiteralPath $sourceFile.FullName
$compilerOptionsMatch = [regex]::Match(
    $source,
    '(?m)^//\s*@compilerOptions\s+(?<options>.*?)\s*$')
$compilerOptions = @()
if ($compilerOptionsMatch.Success) {
    $compilerOptions = $compilerOptionsMatch.Groups['options'].Value -split '\s+' |
        Where-Object { $_ }
}

$outputDll = Join-Path ([IO.Path]::GetTempPath()) (
    'windhawk-link-check-' + [guid]::NewGuid().ToString('N') + '.dll')

try {
    & $clang -std=c++23 -target x86_64-w64-mingw32 -shared `
        -DUNICODE -D_UNICODE -DWH_MOD -DWH_EDITING `
        "-DWH_MOD_ID=L`"$modName`"" "-DWH_MOD_VERSION=L`"0.0`"" `
        -I $include -include windows.h -include windhawk_api.h `
        $sourceFile.FullName @compilerOptions -o $outputDll
    $compileExitCode = $LASTEXITCODE
} finally {
    if (Test-Path -LiteralPath $outputDll) {
        Remove-Item -LiteralPath $outputDll -Force
    }
}

if ($compileExitCode -eq 0) { Write-Output "$modName COMPILE_OK" }
else { Write-Error "$modName COMPILE_FAILED"; exit 1 }
