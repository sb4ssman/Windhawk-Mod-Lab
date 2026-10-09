param(
    [Parameter(Mandatory=$true)][string]$IconPath,
    [ValidateSet('Omni','QuickSettings')][string]$Mode = 'Omni',
    [Parameter(Mandatory=$true)][string]$AutomationId,
    [int]$OffsetX = 0, [int]$OffsetY = 0, [int]$Size = 18,
    [string]$OnClickScript = '', [switch]$SelfTest
)
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
Add-Type -ReferencedAssemblies @('System.Runtime.dll','System.Windows.Forms.dll','System.Drawing.Primitives.dll','System.ComponentModel.Primitives.dll') -TypeDefinition @'
using System.Windows.Forms;
public class IconHostOverlay : Form {
    protected override bool ShowWithoutActivation { get { return true; } }
    protected override CreateParams CreateParams {
        get { var p=base.CreateParams; p.ExStyle |= 0x08000000 | 0x00000080; return p; }
    }
}
'@
$ErrorActionPreference = 'Stop'
$shellExecutable = Join-Path $PSHOME 'pwsh.exe'
if (-not (Test-Path -LiteralPath $shellExecutable)) { $shellExecutable = Join-Path $PSHOME 'powershell.exe' }
if ($Size -lt 8 -or $Size -gt 64) { throw 'Size must be 8..64 pixels' }
$iconFile = (Resolve-Path -LiteralPath $IconPath).Path
if ($OnClickScript) { $OnClickScript = (Resolve-Path -LiteralPath $OnClickScript).Path }
$form = New-Object IconHostOverlay
$form.FormBorderStyle = 'None'; $form.TopMost = $true; $form.ShowInTaskbar = $false
$form.BackColor = [System.Drawing.Color]::Magenta; $form.TransparencyKey = $form.BackColor
$form.Size = New-Object System.Drawing.Size($Size,$Size)
$picture = New-Object System.Windows.Forms.PictureBox
$picture.Dock = 'Fill'; $picture.SizeMode = 'Zoom'
$form.Controls.Add($picture)
$script:image = $null; $script:stamp = ''
function Refresh-Image {
    $stamp = (Get-Item -LiteralPath $iconFile).LastWriteTimeUtc.Ticks
    if ($stamp -eq $script:stamp) { return }
    $icon = New-Object System.Drawing.Icon($iconFile)
    $image = $icon.ToBitmap(); $icon.Dispose()
    $picture.Image = $image
    if ($script:image) { $script:image.Dispose() }
    $script:image = $image; $script:stamp = $stamp
}
Refresh-Image
if ($SelfTest) { $script:image.Dispose(); $form.Dispose(); 'ICON_OVERLAY_SELF_TEST_OK'; exit }
$menu = New-Object System.Windows.Forms.ContextMenuStrip
$exit = $menu.Items.Add('Exit icon-hosting experiment'); $exit.add_Click({$form.Close()})
$picture.ContextMenuStrip = $menu
$picture.add_Click({
    if ($_.Button -eq 'Left' -and $OnClickScript) {
        Start-Process $shellExecutable -WindowStyle Hidden -ArgumentList @('-NoProfile','-File',('"'+$OnClickScript+'"'))
    }
})
$state = Join-Path $env:TEMP ('icon-host-probe-' + $PID + '.json')
$script:worker = $null; $script:lastProbe = [datetime]::MinValue
$timer = New-Object System.Windows.Forms.Timer; $timer.Interval = 200
$timer.add_Tick({
    try {
        if ($script:worker) {
            if (-not $script:worker.HasExited) {
                if (([datetime]::Now - $script:lastProbe).TotalSeconds -gt 5) {
                    $script:worker.Kill(); $script:worker.Dispose(); $script:worker = $null; $form.Hide()
                }
                return
            }
            $ok = $script:worker.ExitCode -eq 0
            $script:worker.Dispose(); $script:worker = $null
            if ($ok -and (Test-Path -LiteralPath $state)) {
                $rows = @(Get-Content -Raw -LiteralPath $state | ConvertFrom-Json)
                $matches = @($rows | Where-Object automationId -eq $AutomationId)
                if ($matches.Count -eq 1) {
                    $anchor = $matches[0]
                    $form.Location = New-Object System.Drawing.Point(([int]$anchor.x+$OffsetX),([int]$anchor.y+$OffsetY))
                    Refresh-Image; $form.Show()
                } else { $form.Hide() }
            } else { $form.Hide() }
        }
        if (([datetime]::Now - $script:lastProbe).TotalSeconds -lt 1) { return }
        $script:lastProbe = [datetime]::Now
        $script:worker = Start-Process $shellExecutable -WindowStyle Hidden -PassThru -RedirectStandardOutput $state -ArgumentList @('-NoProfile','-File',('"'+(Join-Path $PSScriptRoot 'anchor-probe.ps1')+'"'),'-Mode',$Mode)
    } catch { $form.Hide() }
})
$timer.Start()
$form.add_Shown({ $form.Hide() })
try { [System.Windows.Forms.Application]::Run($form) }
finally {
    $timer.Stop(); $timer.Dispose()
    if ($script:worker) { if (-not $script:worker.HasExited) { $script:worker.Kill() }; $script:worker.Dispose() }
    $picture.Image=$null; if ($script:image) { $script:image.Dispose() }
    $menu.Dispose(); $form.Dispose()
}
