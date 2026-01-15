# PowerShell script to find Task Manager windows and their properties
# This helps us identify Task Manager windows correctly

Write-Host "=== Task Manager Window Detection Test ===" -ForegroundColor Cyan

Add-Type @"
using System;
using System.Runtime.InteropServices;
using System.Text;

public class WindowHelper {
    [DllImport("user32.dll")]
    public static extern bool EnumWindows(EnumWindowsProc enumProc, IntPtr lParam);
    
    [DllImport("user32.dll", SetLastError = true)]
    public static extern int GetClassName(IntPtr hWnd, StringBuilder lpClassName, int nMaxCount);
    
    [DllImport("user32.dll", SetLastError = true)]
    public static extern int GetWindowText(IntPtr hWnd, StringBuilder lpString, int nMaxCount);
    
    [DllImport("user32.dll")]
    public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint lpdwProcessId);
    
    [DllImport("kernel32.dll")]
    public static extern IntPtr OpenProcess(uint dwDesiredAccess, bool bInheritHandle, uint dwProcessId);
    
    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern bool QueryFullProcessImageName(IntPtr hProcess, uint dwFlags, StringBuilder lpExeName, ref int lpdwSize);
    
    [DllImport("kernel32.dll")]
    public static extern bool CloseHandle(IntPtr hObject);
    
    public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);
    
    public const uint PROCESS_QUERY_INFORMATION = 0x0400;
    public const uint PROCESS_VM_READ = 0x0010;
}

public class WindowInfo {
    public IntPtr Handle;
    public string ClassName;
    public string WindowText;
    public string ProcessName;
    public uint ProcessId;
}
"@

$windows = New-Object System.Collections.ArrayList

$callback = {
    param([IntPtr]$hWnd, [IntPtr]$lParam)
    
    $className = New-Object System.Text.StringBuilder 256
    [WindowHelper]::GetClassName($hWnd, $className, 256) | Out-Null
    
    $windowText = New-Object System.Text.StringBuilder 256
    [WindowHelper]::GetWindowText($hWnd, $windowText, 256) | Out-Null
    
    $processId = 0
    [WindowHelper]::GetWindowThreadProcessId($hWnd, [ref]$processId) | Out-Null
    
    $processName = ""
    $hProcess = [WindowHelper]::OpenProcess(
        [WindowHelper]::PROCESS_QUERY_INFORMATION -bor [WindowHelper]::PROCESS_VM_READ,
        $false,
        $processId
    )
    
    if ($hProcess -ne [IntPtr]::Zero) {
        $exeName = New-Object System.Text.StringBuilder 260
        $size = 260
        if ([WindowHelper]::QueryFullProcessImageName($hProcess, 0, $exeName, [ref]$size)) {
            $processName = [System.IO.Path]::GetFileName($exeName.ToString())
        }
        [WindowHelper]::CloseHandle($hProcess) | Out-Null
    }
    
    $info = New-Object WindowInfo
    $info.Handle = $hWnd
    $info.ClassName = $className.ToString()
    $info.WindowText = $windowText.ToString()
    $info.ProcessName = $processName
    $info.ProcessId = $processId
    
    [void]$windows.Add($info)
    
    return $true
}

$enumProc = [WindowHelper+EnumWindowsProc]$callback
[WindowHelper]::EnumWindows($enumProc, [IntPtr]::Zero) | Out-Null

Write-Host "`nSearching for Task Manager windows...`n" -ForegroundColor Yellow

$found = $false
foreach ($window in $windows) {
    $isTaskMgr = $false
    
    # Check by class name
    if ($window.ClassName -eq "TaskManagerWindow") {
        $isTaskMgr = $true
    }
    
    # Check by process name
    if ($window.ProcessName -eq "Taskmgr.exe") {
        $isTaskMgr = $true
    }
    
    if ($isTaskMgr) {
        $found = $true
        Write-Host "=== Task Manager Window Found ===" -ForegroundColor Green
        Write-Host "Handle: 0x$($window.Handle.ToString('X'))"
        Write-Host "Class: $($window.ClassName)"
        Write-Host "Title: $($window.WindowText)"
        Write-Host "Process: $($window.ProcessName) (PID: $($window.ProcessId))"
        Write-Host ""
    }
}

if (-not $found) {
    Write-Host "Task Manager is not currently running." -ForegroundColor Yellow
    Write-Host "Please open Task Manager and run this script again." -ForegroundColor Yellow
}

Write-Host "=== Test Complete ===" -ForegroundColor Cyan
