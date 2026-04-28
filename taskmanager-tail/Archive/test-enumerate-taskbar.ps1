# PowerShell script to enumerate taskbar buttons and their properties
# This helps us understand the taskbar structure

Write-Host "=== Taskbar Enumeration Test ===" -ForegroundColor Cyan

# Find the taskbar window
$taskbar = [System.Runtime.InteropServices.Marshal]::FindWindow("Shell_TrayWnd", $null)
if ($taskbar -eq 0) {
    Write-Host "ERROR: Could not find taskbar window" -ForegroundColor Red
    exit 1
}

Write-Host "Taskbar window handle: 0x$($taskbar.ToString('X'))" -ForegroundColor Green

# Find toolbar windows
Add-Type @"
using System;
using System.Runtime.InteropServices;

public class TaskbarHelper {
    [DllImport("user32.dll", SetLastError = true)]
    public static extern IntPtr FindWindowEx(IntPtr hwndParent, IntPtr hwndChildAfter, string lpszClass, string lpszWindow);
    
    [DllImport("user32.dll", SetLastError = true)]
    public static extern int SendMessage(IntPtr hWnd, uint Msg, int wParam, int lParam);
    
    [DllImport("user32.dll", SetLastError = true)]
    public static extern int GetClassName(IntPtr hWnd, System.Text.StringBuilder lpClassName, int nMaxCount);
    
    public const uint TB_BUTTONCOUNT = 0x0418;
    public const uint TB_GETBUTTON = 0x0417;
}
"@

$toolbar = [TaskbarHelper]::FindWindowEx($taskbar, [IntPtr]::Zero, "ToolbarWindow32", $null)
$toolbarIndex = 0

while ($toolbar -ne [IntPtr]::Zero) {
    $className = New-Object System.Text.StringBuilder 256
    [TaskbarHelper]::GetClassName($toolbar, $className, 256) | Out-Null
    
    Write-Host "`nToolbar #$toolbarIndex" -ForegroundColor Yellow
    Write-Host "  Handle: 0x$($toolbar.ToString('X'))"
    Write-Host "  Class: $($className.ToString())"
    
    $buttonCount = [TaskbarHelper]::SendMessage($toolbar, [TaskbarHelper]::TB_BUTTONCOUNT, 0, 0)
    Write-Host "  Button count: $buttonCount"
    
    if ($buttonCount -gt 0) {
        Write-Host "  This appears to be the main taskbar toolbar" -ForegroundColor Green
    }
    
    $toolbar = [TaskbarHelper]::FindWindowEx($taskbar, $toolbar, "ToolbarWindow32", $null)
    $toolbarIndex++
}

Write-Host "`n=== Test Complete ===" -ForegroundColor Cyan
