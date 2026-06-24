#Requires -Version 5.1
param(
    [switch]$Watch,
    [int]$WatchIntervalSeconds = 2
)
<#
.SYNOPSIS
    Privacy Indicator Anchor — diagnostic script
    Mirrors every check the mod makes, plus extras worth considering as future icons.
    Run as the current user (no elevation needed for most checks).
#>
Set-StrictMode -Version Latest
$ErrorActionPreference = 'SilentlyContinue'

# ── helpers ──────────────────────────────────────────────────────────────────

function RegVal($hiveLetter, $path, $name) {
    try {
        $props = Get-ItemProperty -Path "${hiveLetter}:\$path" -Name $name -ErrorAction Stop
        return $props.$name
    } catch { return $null }
}

function RegSubkeys($hiveLetter, $path) {
    try {
        return Get-ChildItem -Path "${hiveLetter}:\$path" -ErrorAction Stop
    } catch { return @() }
}

function Section($title) {
    Write-Host ""
    Write-Host ("── {0} {1}" -f $title, ('─' * [Math]::Max(0, 60 - $title.Length))) -ForegroundColor Cyan
}

function Show($label, $value, [switch]$Bad) {
    $color = if ($Bad) { 'Red' } else { 'Green' }
    $tag   = if ($Bad) { 'DISABLED' } else { 'enabled ' }
    Write-Host ("  [{0}] {1,-42} {2}" -f $tag, $label, $value) -ForegroundColor $color
}

function Info($label, $value) {
    Write-Host ("  [info   ] {0,-42} {1}" -f $label, $value) -ForegroundColor Gray
}

function Verdict($feature, [bool]$disabled) {
    $color = if ($disabled) { 'Yellow' } else { 'DarkGray' }
    $text  = if ($disabled) { "=> $feature : DISABLED (mod should show slash)" } `
                            else { "=> $feature : enabled  (mod shows no slash)" }
    Write-Host "  $text" -ForegroundColor $color
}

# Load WinRT type for DeviceAccessInformation (used in Camera section)
$winrtLoaded = $false
try {
    $null = [Windows.Devices.Enumeration.DeviceAccessInformation, Windows.Devices.Enumeration, ContentType=WindowsRuntime]
    $winrtLoaded = $true
} catch {}

function Get-CameraAccessStatus {
    if (-not $winrtLoaded) { return $null }
    try {
        $guid = [System.Guid]::new("ca3e7ab9-b4c3-4ae6-8251-579ef933890f")
        $dai  = [Windows.Devices.Enumeration.DeviceAccessInformation]::CreateFromDeviceClassId($guid)
        return $dai.CurrentStatus   # DeviceAccessStatus: Unknown=0 Allowed=1 DeniedByUser=2 DeniedBySystem=3
    } catch { return $null }
}

# ── LOCATION ─────────────────────────────────────────────────────────────────
Section "LOCATION"

$locGP = RegVal HKLM 'SOFTWARE\Policies\Microsoft\Windows\LocationAndSensors' 'DisableLocation'
if ($null -ne $locGP -and $locGP -ne 0) {
    Show "Group Policy DisableLocation" $locGP -Bad
} else {
    Info "Group Policy DisableLocation" "not set / 0 (no policy)"
}

$lfsvcSt = RegVal HKLM 'SYSTEM\CurrentControlSet\Services\lfsvc\Service\Configuration' 'Status'
if ($lfsvcSt -eq 0) {
    Show "lfsvc Status (0 = disabled)" $lfsvcSt -Bad
} else {
    Info "lfsvc Status" "$lfsvcSt (non-zero = service running)"
}

$locHKCU = RegVal HKCU 'Software\Microsoft\Windows\CurrentVersion\CapabilityAccessManager\ConsentStore\location' 'Value'
if ($locHKCU -eq 'Deny') {
    Show "HKCU ConsentStore\location" $locHKCU -Bad
} else {
    Info "HKCU ConsentStore\location" "$locHKCU"
}

$locHKLM = RegVal HKLM 'SOFTWARE\Microsoft\Windows\CurrentVersion\CapabilityAccessManager\ConsentStore\location' 'Value'
if ($locHKLM -eq 'Deny') {
    Show "HKLM ConsentStore\location" $locHKLM -Bad
} else {
    Info "HKLM ConsentStore\location" "$locHKLM"
}

$locDisabled = ($null -ne $locGP -and $locGP -ne 0) -or ($lfsvcSt -eq 0) -or
               ($locHKCU -eq 'Deny') -or ($locHKLM -eq 'Deny')
Verdict "Location" $locDisabled

# ── MICROPHONE ───────────────────────────────────────────────────────────────
Section "MICROPHONE"

# Mic hardware / endpoint state via MMDeviceAPI inline C#
$micEndpointState = 'unknown'
try {
    Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;

[ComImport, Guid("BCDE0395-E52F-467C-8E3D-C4579291692E")]
[ClassInterface(ClassInterfaceType.None)]
class CMMDeviceEnumerator {}

[ComImport, Guid("A95664D2-9614-4F35-A746-DE8DB63617E6"),
 InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
interface IMMDeviceEnumerator {
    [PreserveSig] int EnumAudioEndpoints(int f, uint m, out IntPtr p);
    [PreserveSig] int GetDefaultAudioEndpoint(int f, int r, out IntPtr p);
    [PreserveSig] int GetDevice([MarshalAs(UnmanagedType.LPWStr)] string id, out IntPtr p);
    [PreserveSig] int RegisterEndpointNotificationCallback(IntPtr c);
    [PreserveSig] int UnregisterEndpointNotificationCallback(IntPtr c);
}

[ComImport, Guid("D666063F-1587-4E43-81F1-B948E807363F"),
 InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
interface IMMDevice {
    [PreserveSig] int Activate(ref Guid iid, uint ctx, IntPtr p, out IntPtr pp);
    [PreserveSig] int OpenPropertyStore(uint a, out IntPtr p);
    [PreserveSig] int GetId([MarshalAs(UnmanagedType.LPWStr)] out string id);
    [PreserveSig] int GetState(out uint state);
}

[ComImport, Guid("5CDF2C82-841E-4546-9722-0CF74078229A"),
 InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
interface IAudioEndpointVolume {
    [PreserveSig] int RegisterControlChangeNotify(IntPtr p);
    [PreserveSig] int UnregisterControlChangeNotify(IntPtr p);
    [PreserveSig] int GetChannelCount(out uint n);
    [PreserveSig] int SetMasterVolumeLevel(float db, ref Guid g);
    [PreserveSig] int SetMasterVolumeLevelScalar(float v, ref Guid g);
    [PreserveSig] int GetMasterVolumeLevel(out float db);
    [PreserveSig] int GetMasterVolumeLevelScalar(out float v);
    [PreserveSig] int SetChannelVolumeLevel(uint ch, float db, ref Guid g);
    [PreserveSig] int SetChannelVolumeLevelScalar(uint ch, float v, ref Guid g);
    [PreserveSig] int GetChannelVolumeLevel(uint ch, out float db);
    [PreserveSig] int GetChannelVolumeLevelScalar(uint ch, out float v);
    [PreserveSig] int SetMute([MarshalAs(UnmanagedType.Bool)] bool b, ref Guid g);
    [PreserveSig] int GetMute([MarshalAs(UnmanagedType.Bool)] out bool b);
}

public static class AudioDiag {
    static readonly Guid IID_Vol = new Guid("5CDF2C82-841E-4546-9722-0CF74078229A");
    const uint CLSCTX_ALL = 0x17;

    public static string CheckMicEndpoint() {
        try {
            var en = (IMMDeviceEnumerator)new CMMDeviceEnumerator();
            IntPtr devPtr;
            int hr = en.GetDefaultAudioEndpoint(1, 0, out devPtr); // eCapture, eConsole
            if (hr != 0) return hr == unchecked((int)0x80070490) ? "E_NOTFOUND" : string.Format("hr=0x{0:X8}", hr);
            if (devPtr == IntPtr.Zero) return "null device";
            var dev = (IMMDevice)Marshal.GetObjectForIUnknown(devPtr);
            uint state; dev.GetState(out state);
            if (state != 1) {
                Marshal.ReleaseComObject(dev);
                return state == 2 ? "DISABLED(2)" : state == 4 ? "NOTPRESENT(4)" :
                       state == 8 ? "UNPLUGGED(8)" : string.Format("state={0}", state);
            }
            IntPtr volPtr; var iid = IID_Vol;
            hr = dev.Activate(ref iid, CLSCTX_ALL, IntPtr.Zero, out volPtr);
            if (hr == 0 && volPtr != IntPtr.Zero) {
                var vol = (IAudioEndpointVolume)Marshal.GetObjectForIUnknown(volPtr);
                bool muted; vol.GetMute(out muted);
                Marshal.ReleaseComObject(vol);
                if (muted) { Marshal.ReleaseComObject(dev); return "MUTED"; }
            }
            Marshal.ReleaseComObject(dev);
            return "Active/Unmuted";
        } catch (Exception ex) { return "Error: " + ex.Message; }
    }
}
'@ -ErrorAction Stop
    $micEndpointState = [AudioDiag]::CheckMicEndpoint()
} catch {
    $micEndpointState = "Add-Type failed: $_"
}

$micEndpointBad = $micEndpointState -match 'DISABLED|NOTPRESENT|MUTED'
if ($micEndpointBad) {
    Show "Default capture endpoint" $micEndpointState -Bad
} else {
    Info "Default capture endpoint" $micEndpointState
}

$micHKCU = RegVal HKCU 'Software\Microsoft\Windows\CurrentVersion\CapabilityAccessManager\ConsentStore\microphone' 'Value'
if ($micHKCU -eq 'Deny') {
    Show "HKCU ConsentStore\microphone" $micHKCU -Bad
} else {
    Info "HKCU ConsentStore\microphone" "$micHKCU"
}

$micHKLM = RegVal HKLM 'SOFTWARE\Microsoft\Windows\CurrentVersion\CapabilityAccessManager\ConsentStore\microphone' 'Value'
if ($micHKLM -eq 'Deny') {
    Show "HKLM ConsentStore\microphone" $micHKLM -Bad
} else {
    Info "HKLM ConsentStore\microphone" "$micHKLM"
}

$micDisabled = $micEndpointBad -or ($micHKCU -eq 'Deny') -or ($micHKLM -eq 'Deny')
Verdict "Microphone" $micDisabled

# ── CAMERA ───────────────────────────────────────────────────────────────────
Section "CAMERA"

# ── 1. WinRT DeviceAccessInformation (what the mod checks first) ──────────
if ($winrtLoaded) {
    $daiStatus = Get-CameraAccessStatus
    $daiInt    = if ($null -ne $daiStatus) { [int]$daiStatus } else { -1 }
    # 0=Unknown 1=Allowed 2=DeniedByUser 3=DeniedBySystem
    if ($daiInt -ge 2) {
        Show "WinRT DeviceAccessInformation" "$daiStatus ($daiInt)" -Bad
    } elseif ($daiInt -eq 1) {
        Info "WinRT DeviceAccessInformation" "Allowed (1)"
    } else {
        Info "WinRT DeviceAccessInformation" "Unknown/unavailable ($daiInt)"
    }
} else {
    Info "WinRT DeviceAccessInformation" "WinRT not available in this PS session"
}

# ── 2. PnP enumeration with problem codes ─────────────────────────────────
try {
    $allCams = Get-PnpDevice -Class 'Camera' -ErrorAction Stop

    if ($allCams.Count -eq 0) {
        Info "PnP Camera class" "No devices registered at all"
    } else {
        Info "PnP Camera class" ("$($allCams.Count) device(s) in database:")
        foreach ($cam in $allCams) {
            $present = $cam.Status -eq 'OK'
            $isIR    = $cam.FriendlyName -match '\bIR\b|Hello|Face'
            $probProp = $cam | Get-PnpDeviceProperty -KeyName 'DEVPKEY_Device_ProblemCode' -ErrorAction SilentlyContinue
            $probCode = if ($probProp -and $probProp.Data) { "Problem=$($probProp.Data)" } else { "" }
            $tag = @()
            if ($present)  { $tag += 'PRESENT' }
            if ($isIR)     { $tag += 'IR/Hello-filtered' }
            if ($probCode) { $tag += $probCode }
            $tagStr = if ($tag) { " [" + ($tag -join ', ') + "]" } else { " [not present]" }
            if ($cam.Status -ne 'OK' -and -not $isIR) {
                Show ("    " + $cam.FriendlyName) ($cam.Status + $tagStr) -Bad
            } else {
                Info ("    " + $cam.FriendlyName) ($cam.Status + $tagStr)
            }
        }

        $nonIRPresent = $allCams | Where-Object { $_.Status -eq 'OK' -and $_.FriendlyName -notmatch '\bIR\b|Hello|Face' }
        if ($nonIRPresent.Count -eq 0) {
            Show "Non-IR camera present (DIGCF_PRESENT)" "NONE — hardware kill switch active?" -Bad
        } else {
            Info "Non-IR cameras present" "$($nonIRPresent.Count)"
        }
    }
} catch {
    Info "PnP Camera enumeration" "failed: $_"
}

# ── 3. ConsentStore ───────────────────────────────────────────────────────
$camHKCU = RegVal HKCU 'Software\Microsoft\Windows\CurrentVersion\CapabilityAccessManager\ConsentStore\webcam' 'Value'
if ($camHKCU -eq 'Deny') {
    Show "HKCU ConsentStore\webcam" $camHKCU -Bad
} else {
    Info "HKCU ConsentStore\webcam" "$camHKCU"
}

$camHKLM = RegVal HKLM 'SOFTWARE\Microsoft\Windows\CurrentVersion\CapabilityAccessManager\ConsentStore\webcam' 'Value'
if ($camHKLM -eq 'Deny') {
    Show "HKLM ConsentStore\webcam" $camHKLM -Bad
} else {
    Info "HKLM ConsentStore\webcam" "$camHKLM"
}

# ── 4. WMI root\WMI scan for camera / privacy / shutter / Lenovo classes ─
Info "WMI root\WMI scan" "scanning for camera/privacy/shutter/lenovo classes..."
try {
    $wmiPrivacy = Get-WmiObject -Namespace root\WMI -List -ErrorAction Stop |
                  Where-Object { $_.Name -match 'camera|Camera|privacy|Privacy|shutter|Shutter|lenovo|Lenovo|LENOVO' }
    if ($wmiPrivacy) {
        foreach ($cls in $wmiPrivacy) {
            Info "  WMI class found" $cls.Name
            # Try to instantiate and dump properties
            try {
                $inst = Get-WmiObject -Namespace root\WMI -Class $cls.Name -ErrorAction Stop
                if ($inst) {
                    $inst | Get-Member -MemberType Property | Where-Object { $_.Name -notmatch '^__' } | ForEach-Object {
                        $propName = $_.Name
                        try { Info "    .$propName" "$($inst.$propName)" } catch {}
                    }
                }
            } catch {}
        }
    } else {
        Info "  WMI camera/privacy/shutter/Lenovo classes" "none found in root\WMI"
    }
} catch {
    Info "  WMI root\WMI scan" "failed: $_"
}

$camDisabled = ($daiInt -ge 2) -or ($camHKCU -eq 'Deny') -or ($camHKLM -eq 'Deny')
Verdict "Camera" $camDisabled

# ── CAMERA SWITCH TEST ────────────────────────────────────────────────────
Section "CAMERA SWITCH TEST"
Write-Host "  This section checks whether your hardware switch changes any detectable state." -ForegroundColor White
Write-Host "  Run the script twice: once with switch OPEN, once with switch CLOSED." -ForegroundColor White
Write-Host ""

$switchDai = Get-CameraAccessStatus
$switchPnp = (Get-PnpDevice -Class 'Camera' -ErrorAction SilentlyContinue) |
             Where-Object { $_.FriendlyName -notmatch '\bIR\b|Hello|Face' } |
             Select-Object FriendlyName, Status

Info "DeviceAccessInformation NOW" $(if ($switchDai) { "$switchDai ($([int]$switchDai))" } else { "unavailable" })
foreach ($c in $switchPnp) {
    Info "  PnP status NOW" "$($c.FriendlyName): $($c.Status)"
}
Write-Host "  => Flip your switch, then re-run the script and compare these values." -ForegroundColor Yellow

# ── COPILOT ──────────────────────────────────────────────────────────────────
Section "COPILOT"

# ── 1. Get-AppxPackage — authoritative installed state ────────────────────
# Registry keys in AppModel linger after uninstall; Get-AppxPackage queries the
# deployment service and returns nothing if the package is truly removed.
$webExpPkg   = Get-AppxPackage -Name '*MicrosoftWindows.Client.WebExperience*' -ErrorAction SilentlyContinue
$copilotPkg  = Get-AppxPackage -Name '*Microsoft.Copilot*'                     -ErrorAction SilentlyContinue
$copilotPkg2 = Get-AppxPackage -Name '*Microsoft.Windows.Ai.Copilot*'          -ErrorAction SilentlyContinue

$anyPkg = $false
if ($webExpPkg) {
    $anyPkg = $true
    Info "Get-AppxPackage WebExperience" "$($webExpPkg.Version)"
    $loc = $webExpPkg.InstallLocation
    if ($loc -and (Test-Path $loc)) {
        Info "  Install location on disk" "EXISTS: $loc"
    } else {
        Show "  Install location on disk" "MISSING — registry artifact only (truly uninstalled)" -Bad
    }
}
if ($copilotPkg) {
    $anyPkg = $true
    Info "Get-AppxPackage Microsoft.Copilot" "$($copilotPkg.Version)"
}
if ($copilotPkg2) {
    $anyPkg = $true
    Info "Get-AppxPackage Ai.Copilot" "$($copilotPkg2.Version)"
}
if (-not $anyPkg) {
    Show "Get-AppxPackage (all Copilot variants)" "NONE — package truly not installed" -Bad
}

# ── 2. AppModel registry (may linger after uninstall) ────────────────────
$copPrefix = 'MicrosoftWindows.Client.WebExperience_'
$copRoots = @(
    [pscustomobject]@{ Hive='HKCU'; Path='Software\Classes\Local Settings\Software\Microsoft\Windows\CurrentVersion\AppModel\Repository\Packages' },
    [pscustomobject]@{ Hive='HKLM'; Path='SOFTWARE\Microsoft\Windows\CurrentVersion\AppModel\Repository\Packages' }
)

$regFound = $false
foreach ($root in $copRoots) {
    $subs = RegSubkeys $root.Hive $root.Path
    $hits = $subs | Where-Object { $_.PSChildName.StartsWith($copPrefix) }
    foreach ($h in $hits) {
        Info "AppModel registry ($($root.Hive)) [may linger]" $h.PSChildName
        $regFound = $true
    }
    if (-not $hits) {
        Info "AppModel registry ($($root.Hive))" "no WebExperience key found"
    }
}
if ($regFound -and -not $anyPkg) {
    Write-Host "  NOTE: AppModel key exists but Get-AppxPackage returned nothing." -ForegroundColor DarkYellow
    Write-Host "        Registry is a stale artifact. Package is NOT installed." -ForegroundColor DarkYellow
}

# ── 3. ShowCopilotButton (Settings > Personalization > Taskbar) ───────────
$showBtn = RegVal HKCU 'Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced' 'ShowCopilotButton'
if ($null -eq $showBtn) {
    Info "ShowCopilotButton" "not set (default = shown)"
} elseif ($showBtn -eq 0) {
    Show "ShowCopilotButton" "0 = explicitly disabled via Settings" -Bad
} else {
    Info "ShowCopilotButton" "$showBtn (shown)"
}

# ── 4. Process running ────────────────────────────────────────────────────
$copExes  = @('Copilot','AIHost','copilotwindows','Microsoft.Copilot')
$copProcs = Get-Process | Where-Object { $copExes -contains $_.Name }
if ($copProcs) {
    foreach ($p in $copProcs) { Show "Copilot process running" "$($p.Name) (PID $($p.Id))" }
} else {
    Info "Copilot process" "not running"
}

# Verdict: disabled if package is truly gone OR user explicitly hid the button
$copDisabled = (-not $anyPkg) -or ($null -ne $showBtn -and $showBtn -eq 0)
Verdict "Copilot" $copDisabled

if ($regFound -and -not $anyPkg) {
    Write-Host "  NOTE: The mod's current AppModel registry check will show NO SLASH" -ForegroundColor DarkYellow
    Write-Host "        because the registry key exists. The mod needs Get-AppxPackage" -ForegroundColor DarkYellow
    Write-Host "        equivalent (GetPackagesByPackageFamily Win32 API) to detect" -ForegroundColor DarkYellow
    Write-Host "        truly-uninstalled state." -ForegroundColor DarkYellow
}

# ── ADDITIONAL PRIVACY STATES (future icon candidates) ────────────────────────
Section "OTHER CONSENT-STORE CAPABILITIES (future icon candidates)"

$consentRoot = 'Software\Microsoft\Windows\CurrentVersion\CapabilityAccessManager\ConsentStore'
$candidates = [ordered]@{
    'graphicsCaptureProgrammatic' = 'Screen capture'
    'userNotificationListener'   = 'Notifications access'
    'contacts'                   = 'Contacts'
    'appointments'               = 'Calendar'
    'phoneCallHistory'           = 'Call history'
    'chat'                       = 'Messaging (SMS)'
    'email'                      = 'Email'
    'radios'                     = 'Radios (Bluetooth/WiFi control)'
    'bluetoothSync'              = 'Bluetooth sync'
    'activity'                   = 'Activity history'
    'trackerUserInteraction'     = 'Account info'
    'userDataTasks'              = 'Tasks'
    'gazeInput'                  = 'Eye tracking'
    'cellularData'               = 'Cellular data'
}

foreach ($kv in $candidates.GetEnumerator()) {
    $val = RegVal HKCU "$consentRoot\$($kv.Key)" 'Value'
    if ($val -eq 'Deny') {
        Show $kv.Value "Deny  ($($kv.Key))" -Bad
    } else {
        Info $kv.Value "Allow ($($kv.Key)): $val"
    }
}

Write-Host ""
Write-Host "Done." -ForegroundColor White

function Get-CompactCameraState {
    $dai = Get-CameraAccessStatus
    $daiText = if ($null -ne $dai) { "$dai/$([int]$dai)" } else { "unavailable" }
    $cams = @(Get-PnpDevice -Class 'Camera' -ErrorAction SilentlyContinue |
              Where-Object { $_.FriendlyName -notmatch '\bIR\b|Hello|Face' })
    if ($cams.Count -eq 0) {
        return "DeviceAccess=$daiText; non-IR camera=none"
    }
    $parts = foreach ($cam in $cams) {
        $probProp = $cam | Get-PnpDeviceProperty -KeyName 'DEVPKEY_Device_ProblemCode' -ErrorAction SilentlyContinue
        $probCode = if ($probProp -and $null -ne $probProp.Data) { $probProp.Data } else { "" }
        $problemText = if ($probCode -ne '') { "/Problem=$probCode" } else { "" }
        "{0}:{1}{2}" -f $cam.FriendlyName, $cam.Status, $problemText
    }
    return "DeviceAccess=$daiText; " + ($parts -join '; ')
}

function Get-CompactMicState {
    try {
        $endpoint = [AudioDiag]::CheckMicEndpoint()
    } catch {
        $endpoint = "AudioDiag unavailable"
    }
    $micHKCU = RegVal HKCU 'Software\Microsoft\Windows\CurrentVersion\CapabilityAccessManager\ConsentStore\microphone' 'Value'
    return "Endpoint=$endpoint; Consent=$micHKCU"
}

if ($Watch) {
    Write-Host ""
    Write-Host "Watching compact mic/camera state. Press Ctrl+C to stop." -ForegroundColor Cyan
    while ($true) {
        $now = Get-Date -Format 'HH:mm:ss'
        Write-Host "[$now] Mic:    $(Get-CompactMicState)"
        Write-Host "[$now] Camera: $(Get-CompactCameraState)"
        Start-Sleep -Seconds ([Math]::Max(1, $WatchIntervalSeconds))
    }
}
