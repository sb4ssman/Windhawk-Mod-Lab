param([string]$Config = (Join-Path $PSScriptRoot 'panel.json'), [switch]$DcgOnly, [switch]$SelfTest)
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
$ErrorActionPreference = 'Stop'
$configuration = Get-Content -Raw -LiteralPath $Config | ConvertFrom-Json
$python = (Get-Command python -ErrorAction Stop).Source
$stateDirectory = Join-Path $env:LOCALAPPDATA 'WindhawkModLabExperiments/ConfirmationLights'
if (-not $SelfTest) { New-Item -ItemType Directory -Force -Path $stateDirectory | Out-Null }
$output = Join-Path $stateDirectory ('panel-' + $PID + '.json')
$form = New-Object System.Windows.Forms.Form
$form.Text = $configuration.title
$form.Size = New-Object System.Drawing.Size(560, 340)
$form.BackColor = [System.Drawing.Color]::FromArgb(15, 20, 23)
$form.ForeColor = [System.Drawing.Color]::Gainsboro
$form.Font = New-Object System.Drawing.Font('Consolas', 11)
$list = New-Object System.Windows.Forms.ListView
$list.Dock = 'Fill'; $list.View = 'Details'; $list.FullRowSelect = $true
$list.BackColor = $form.BackColor; $list.ForeColor = $form.ForeColor
[void]$list.Columns.Add('SYSTEM', 100); [void]$list.Columns.Add('STATE', 90); [void]$list.Columns.Add('EVIDENCE', 330)
$form.Controls.Add($list)
$lamps = New-Object System.Windows.Forms.ImageList
$lamps.ImageSize = New-Object System.Drawing.Size(16,16)
$lamps.ColorDepth = 'Depth32Bit'
$list.SmallImageList = $lamps
function Refresh-PanelLamps {
    $lamps.Images.Clear()
    foreach ($state in @('good','working','bad','unknown')) {
        $bitmap = New-Object System.Drawing.Bitmap(16,16)
        $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
        $graphics.SmoothingMode = 'AntiAlias'
        $brush = New-Object System.Drawing.SolidBrush([System.Drawing.ColorTranslator]::FromHtml($configuration.colors.$state))
        $graphics.FillEllipse($brush,2,2,12,12)
        $lamps.Images.Add($state,$bitmap)
        $brush.Dispose(); $graphics.Dispose(); $bitmap.Dispose()
    }
}
Refresh-PanelLamps
$tray = New-Object System.Windows.Forms.NotifyIcon
$tray.Visible = -not $SelfTest; $tray.Text = 'Confirmation lights: starting'
$menu = New-Object System.Windows.Forms.ContextMenuStrip
$open = $menu.Items.Add('Open panel'); $open.add_Click({$form.Show(); $form.Activate()})
$edit = $menu.Items.Add('Edit detectors...'); $edit.add_Click({Start-Process notepad.exe -ArgumentList ('"' + $Config + '"')})
$palette = New-Object System.Windows.Forms.ToolStripMenuItem('Light colors')
foreach ($state in @('good', 'working', 'bad', 'unknown')) {
    $item = $palette.DropDownItems.Add($state); $item.Tag = $state
    $item.add_Click({
        $dialog = New-Object System.Windows.Forms.ColorDialog
        if ($dialog.ShowDialog() -eq 'OK') {
            $configuration.colors.($this.Tag) = '#' + $dialog.Color.R.ToString('X2') + $dialog.Color.G.ToString('X2') + $dialog.Color.B.ToString('X2')
            $configuration | ConvertTo-Json -Depth 20 | Set-Content -LiteralPath $Config -Encoding UTF8
        }
        $dialog.Dispose()
    })
}
[void]$menu.Items.Add($palette)
$quit = $menu.Items.Add('Exit'); $quit.add_Click({$script:exiting = $true; $form.Close()})
$tray.ContextMenuStrip = $menu
$tray.add_DoubleClick({$form.Show(); $form.Activate()})
$script:icon = $null; $script:worker = $null; $script:nextRun = [datetime]::MinValue; $script:exiting = $false
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class ConfirmationNative { [DllImport("user32.dll")] public static extern bool DestroyIcon(IntPtr handle); }
'@
function Show-Light([string]$State) {
    $color = [System.Drawing.ColorTranslator]::FromHtml($configuration.colors.$State)
    $bitmap = New-Object System.Drawing.Bitmap(32,32)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $graphics.SmoothingMode = 'AntiAlias'
    $brush = New-Object System.Drawing.SolidBrush($color)
    $graphics.FillEllipse($brush, 4, 4, 24, 24)
    $handle = $bitmap.GetHicon()
    $temporary = [System.Drawing.Icon]::FromHandle($handle)
    $newIcon = $temporary.Clone()
    $tray.Icon = $newIcon
    if ($script:icon) { $script:icon.Dispose() }
    $script:icon = $newIcon
    $temporary.Dispose(); [void][ConfirmationNative]::DestroyIcon($handle)
    $brush.Dispose(); $graphics.Dispose(); $bitmap.Dispose()
}
$timer = New-Object System.Windows.Forms.Timer
$timer.Interval = 250
$timer.add_Tick({
    if ($script:worker) {
        if (-not $script:worker.HasExited) { return }
        try {
            if ($script:worker.ExitCode -ne 0) { throw 'Checker failed' }
            $report = Get-Content -Raw -LiteralPath $output | ConvertFrom-Json
            $lights = @($report.lights)
            if ($DcgOnly) { $lights = @($lights | Where-Object id -eq 'guard') }
            $state = $report.state
            if ($DcgOnly) { $state = if ($lights.Count) { $lights[0].state } else { 'unknown' } }
            Show-Light $state
            $tray.Text = ('Confirmation: ' + $state)
            $list.Items.Clear()
            Refresh-PanelLamps
            foreach ($light in $lights) {
                $row = New-Object System.Windows.Forms.ListViewItem($light.label)
                $row.ImageKey = $light.state
                [void]$row.SubItems.Add($light.state); [void]$row.SubItems.Add($light.reason)
                $row.ForeColor = [System.Drawing.ColorTranslator]::FromHtml($configuration.colors.($light.state))
                [void]$list.Items.Add($row)
            }
        } catch { Show-Light 'unknown'; $tray.Text = 'Confirmation: checker unavailable' }
        $script:worker.Dispose(); $script:worker = $null
        $script:nextRun = [datetime]::Now.AddSeconds([Math]::Max(5, $configuration.intervalSeconds))
    }
    if ([datetime]::Now -lt $script:nextRun) { return }
    try {
        $configuration = Get-Content -Raw -LiteralPath $Config | ConvertFrom-Json
        Show-Light 'working'
        $info = New-Object System.Diagnostics.ProcessStartInfo
        $info.FileName = $python
        $info.Arguments = '"' + (Join-Path $PSScriptRoot 'checker.py') + '" --config "' + $Config + '" --output "' + $output + '"'
        $info.UseShellExecute = $false; $info.CreateNoWindow = $true
        $script:worker = [System.Diagnostics.Process]::Start($info)
    } catch { Show-Light 'unknown'; $script:nextRun = [datetime]::Now.AddSeconds(30) }
})
$form.add_FormClosing({ if (-not $script:exiting) { $_.Cancel = $true; $form.Hide() } })
Show-Light 'working'
if ($SelfTest) {
    foreach ($state in @('good','working','bad','unknown')) { Show-Light $state }
    $tray.Visible = $false; $tray.Dispose(); $menu.Dispose()
    $script:icon.Dispose(); $timer.Dispose(); $form.Dispose(); $lamps.Dispose()
    'CONFIRMATION_UI_SELF_TEST_OK'; exit
}
$timer.Start()
try { [System.Windows.Forms.Application]::Run($form) }
finally {
    $timer.Stop(); $timer.Dispose()
    if ($script:worker) { if (-not $script:worker.HasExited) { $script:worker.Kill() }; $script:worker.Dispose() }
    $tray.Visible = $false; $tray.Dispose(); $menu.Dispose()
    if ($script:icon) { $script:icon.Dispose() }
    $form.Dispose(); $lamps.Dispose()
}
