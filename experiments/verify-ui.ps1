$ErrorActionPreference = 'Stop'
foreach ($path in @('confirmation-lights/start.ps1','icon-hosting/anchor-probe.ps1','icon-hosting/overlay.ps1')) {
    $tokens=$null; $errors=$null
    [void][System.Management.Automation.Language.Parser]::ParseFile((Join-Path $PSScriptRoot $path),[ref]$tokens,[ref]$errors)
    if ($errors) { throw ($errors -join "`n") }
}
& (Join-Path $PSScriptRoot 'confirmation-lights/start.ps1') -SelfTest
Add-Type -AssemblyName System.Drawing
$iconPath = Join-Path ([System.IO.Path]::GetTempPath()) ('icon-overlay-test-'+[guid]::NewGuid()+'.ico')
$stream = [System.IO.File]::Create($iconPath)
try { [System.Drawing.SystemIcons]::Information.Save($stream) } finally { $stream.Dispose() }
try {
    & (Join-Path $PSScriptRoot 'icon-hosting/overlay.ps1') -IconPath $iconPath -AutomationId test -SelfTest
} finally { Remove-Item -LiteralPath $iconPath }
