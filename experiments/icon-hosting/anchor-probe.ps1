param([ValidateSet('Omni','QuickSettings')][string]$Mode = 'Omni')
Add-Type -AssemblyName UIAutomationClient
Add-Type -AssemblyName UIAutomationTypes
Add-Type @'
using System;
using System.Runtime.InteropServices;
using System.Text;
public static class IconHostWindows {
  public delegate bool Callback(IntPtr hwnd, IntPtr parameter);
  [DllImport("user32.dll")] public static extern bool EnumWindows(Callback callback, IntPtr parameter);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetClassName(IntPtr hwnd, StringBuilder text, int count);
}
'@
$script:candidates = New-Object 'System.Collections.Generic.List[System.IntPtr]'
$callback = [IconHostWindows+Callback]{ param($hwnd,$parameter)
    $name = New-Object System.Text.StringBuilder(256)
    [void][IconHostWindows]::GetClassName($hwnd, $name, 256)
    $class = $name.ToString()
    if (($Mode -eq 'Omni' -and $class -eq 'Shell_TrayWnd') -or
        ($Mode -eq 'QuickSettings' -and $class -match 'ControlCenter|XamlExplorerHostIsland')) {
        $script:candidates.Add($hwnd)
    }
    return $true
}
[void][IconHostWindows]::EnumWindows($callback,[IntPtr]::Zero)
$rows = @()
foreach ($hwnd in $script:candidates) {
    $root = [System.Windows.Automation.AutomationElement]::FromHandle($hwnd)
    $queue = New-Object 'System.Collections.Generic.Queue[System.Windows.Automation.AutomationElement]'
    $queue.Enqueue($root); $visited = 0
    $walker = [System.Windows.Automation.TreeWalker]::ControlViewWalker
    while ($queue.Count -and $visited -lt 512) {
        $element = $queue.Dequeue(); $visited++
        try {
            $c = $element.Current; $r = $c.BoundingRectangle
            if (-not $c.IsOffscreen -and $r.Width -gt 0) {
                $rows += [pscustomobject]@{ name=$c.Name; automationId=$c.AutomationId; className=$c.ClassName;
                    controlType=$c.ControlType.ProgrammaticName; x=$r.X; y=$r.Y; width=$r.Width; height=$r.Height }
            }
            $child = $walker.GetFirstChild($element)
            while ($child -and $queue.Count -lt 512) { $queue.Enqueue($child); $child = $walker.GetNextSibling($child) }
        } catch { }
    }
}
$rows | ConvertTo-Json -Depth 4
