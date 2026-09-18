$ErrorActionPreference = 'Stop'
$labRoot = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$output = Join-Path $labRoot '.agents/outputs/test-candidates-2026-09-11'
if (Test-Path -LiteralPath $output) { throw 'Candidate package already exists; do not overwrite a test baseline.' }
New-Item -ItemType Directory -Path $output | Out-Null
$sources = New-Item -ItemType Directory -Path (Join-Path $output 'candidates')
$backups = New-Item -ItemType Directory -Path (Join-Path $output 'installed-backups')
$mods = @(
    @{ Folder = 'taskbar-vd-switcher'; Installed = 'local@taskbar-vd-switcher2' },
    @{ Folder = 'taskbar-folder-menus'; Installed = 'taskbar-folder-menus' },
    @{ Folder = 'taskbar-clock-spacer'; Installed = 'local@taskbar-clock-spacer' },
    @{ Folder = 'tray-utility-customizer'; Installed = 'local@tray-utility-customizer2' },
    @{ Folder = 'privacy-indicator-anchor'; Installed = 'local@tray-privacy-indicator-anchor' }
)
$manifest = foreach ($mod in $mods) {
    $source = Get-ChildItem -LiteralPath (Join-Path $labRoot $mod.Folder) -Filter '*.wh.cpp'
    if (@($source).Count -ne 1) { throw "Expected one source for $($mod.Folder)" }
    $installed = Join-Path 'C:/ProgramData/Windhawk/ModsSource' ($mod.Installed + '.wh.cpp')
    Copy-Item -LiteralPath $source.FullName -Destination $sources.FullName
    Copy-Item -LiteralPath $installed -Destination $backups.FullName
    $settingsKey = Get-Item -LiteralPath ('HKLM:\SOFTWARE\Windhawk\Engine\Mods\' + $mod.Installed + '\Settings')
    $settings = [ordered]@{}
    foreach ($name in $settingsKey.GetValueNames()) {
        $settings[$name] = @{ Kind = $settingsKey.GetValueKind($name).ToString(); Value = $settingsKey.GetValue($name) }
    }
    $settings | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $backups.FullName ($mod.Installed + '.settings.json')) -Encoding utf8
    [ordered]@{
        Mod = $mod.Folder
        Candidate = 'candidates/' + $source.Name
        CandidateSHA256 = (Get-FileHash -LiteralPath $source.FullName -Algorithm SHA256).Hash
        InstalledId = $mod.Installed
        InstalledSourceSHA256 = (Get-FileHash -LiteralPath $installed -Algorithm SHA256).Hash
        LiveTest = 'Pending for this candidate'
    }
}
$manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $output 'manifest.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $labRoot '.agents/outputs/release-test-guide.md') -Destination (Join-Path $output 'TESTING.md')
Write-Output "Packaged five exact source candidates and installed backups at $output"
