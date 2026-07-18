Add-Type -AssemblyName System.Runtime.WindowsRuntime
$null = [Windows.Media.Capture.MediaCapture,Windows.Media.Capture,ContentType=WindowsRuntime]
$asTask = ([System.WindowsRuntimeSystemExtensions].GetMethods() | Where-Object {
    $_.Name -eq 'AsTask' -and $_.GetParameters().Count -eq 1 -and
    $_.GetParameters()[0].ParameterType.FullName -eq 'Windows.Foundation.IAsyncAction' })[0]
$mc = New-Object Windows.Media.Capture.MediaCapture
try {
    $t = $asTask.Invoke($null, @($mc.InitializeAsync()))
    if ($t.Wait(8000)) { 'INIT_OK — camera opened successfully' }
    else { 'TIMEOUT waiting for camera init' }
} catch {
    $e = $_.Exception
    while ($e.InnerException) { $e = $e.InnerException }
    'INIT_FAILED: ' + $e.Message + ' HRESULT=0x' + ('{0:X8}' -f $e.HResult)
} finally {
    $mc.Dispose()
}
