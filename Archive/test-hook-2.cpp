// ==WindhawkMod==
// @id              test-hook-explorer
// @name            Test Hook Explorer
// @description     Logs taskbar window structure to help identify hooking targets
// @version         2.0
// @author          sb4ssman
// @include         explorer.exe
// @compilerOptions -luser32
// ==/WindhawkMod==

// ==WindhawkModReadme==
    // Log current window
    Wh_Log(L"%s[HWND: %p] Class: %s, Text: %s", indent, hwnd, className, windowText);

    // Check for ToolbarWindow32 OR MSTaskListWClass (The Win10/11 Taskbar Button List)
    if (lstrcmpW(className, L"ToolbarWindow32") == 0 || 
        lstrcmpW(className, L"MSTaskListWClass") == 0) {
        LRESULT count = SendMessageW(hwnd, TB_BUTTONCOUNT, 0, 0);
        Wh_Log(L"%s^^^ FOUND TARGET WINDOW (%s) ^^^ Buttons: %d", indent, className, (int)count);
    }

    // Iterate children
    HWND child = GetWindow(hwnd, GW_CHILD);
    while (child) {
        EnumWindowsRecursive(child, depth + 1);
        child = GetWindow(child, GW_HWNDNEXT);
    }
}

BOOL Wh_ModInit() {
    Wh_Log(L"=== Test Hook Explorer v2.0 Init ===");
    
    HWND tray = FindWindowW(L"Shell_TrayWnd", NULL);
    if (tray) {
        Wh_Log(L"Found Shell_TrayWnd: %p", tray);
        EnumWindowsRecursive(tray, 0);
    } else {
        Wh_Log(L"Shell_TrayWnd not found");
    }
    
    return TRUE;
}
