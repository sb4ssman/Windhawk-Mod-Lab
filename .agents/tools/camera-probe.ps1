if ($PSVersionTable.PSEdition -ne 'Desktop') {
    throw 'Run this probe with Windows PowerShell 5.1 (powershell.exe), not PowerShell 7/pwsh; the newer host cannot project these WinRT camera types.'
}

Add-Type -AssemblyName System.Runtime.WindowsRuntime
$null = [Windows.Media.Capture.MediaCapture,Windows.Media.Capture,ContentType=WindowsRuntime]
$null = [Windows.Media.Capture.MediaCaptureInitializationSettings,Windows.Media.Capture,ContentType=WindowsRuntime]
$null = [Windows.Media.Capture.MediaCaptureSharingMode,Windows.Media.Capture,ContentType=WindowsRuntime]
$null = [Windows.Media.Capture.StreamingCaptureMode,Windows.Media.Capture,ContentType=WindowsRuntime]
$null = [Windows.Media.Devices.CameraOcclusionKind,Windows.Media.Devices,ContentType=WindowsRuntime]
$asTask = ([System.WindowsRuntimeSystemExtensions].GetMethods() | Where-Object {
    $_.Name -eq 'AsTask' -and $_.GetParameters().Count -eq 1 -and
    $_.GetParameters()[0].ParameterType.FullName -eq 'Windows.Foundation.IAsyncAction' })[0]
$mc = New-Object Windows.Media.Capture.MediaCapture
try {
    $settings = New-Object Windows.Media.Capture.MediaCaptureInitializationSettings
    $settings.StreamingCaptureMode = [Windows.Media.Capture.StreamingCaptureMode]::Video
    $settings.SharingMode = [Windows.Media.Capture.MediaCaptureSharingMode]::SharedReadOnly

    $t = $asTask.Invoke($null, @($mc.InitializeAsync($settings)))
    if (-not $t.Wait(8000)) {
        'TIMEOUT waiting for camera init'
        return
    }

    'INIT_OK - camera opened successfully in SharedReadOnly mode'

    # Windows 11 cameras can expose their physical shutter/kill-switch state
    # through VideoDeviceController.CameraOcclusionInfo. This first-stage probe
    # identifies driver support and reads the current state. Repeat the probe
    # with the physical control in both positions before treating it as proven
    # for a particular camera/driver.
    $occlusion = $mc.VideoDeviceController.CameraOcclusionInfo
    if ($null -eq $occlusion) {
        'OCCLUSION_INFO=unavailable'
    } else {
        $kind = [Windows.Media.Devices.CameraOcclusionKind]::CameraHardware
        $supported = $occlusion.IsOcclusionKindSupported($kind)
        $state = $occlusion.GetState()
        'OCCLUSION_CAMERA_HARDWARE_SUPPORTED=' + $supported
        'OCCLUSION_IS_OCCLUDED=' + $state.IsOccluded
        'OCCLUSION_IS_CAMERA_HARDWARE=' + $state.IsOcclusionKind($kind)
        'NOTE=compare both physical switch positions; event delivery still needs a persistent-monitor test'
    }
} catch {
    $e = $_.Exception
    while ($e.InnerException) { $e = $e.InnerException }
    'INIT_FAILED: ' + $e.Message + ' HRESULT=0x' + ('{0:X8}' -f $e.HResult)
} finally {
    $mc.Dispose()
}
