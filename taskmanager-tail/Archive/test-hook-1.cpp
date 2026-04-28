// ==WindhawkMod==
// @id              test-hook-explorer
// @name            Test Hook Explorer
// @description     Logs taskbar window structure to help identify hooking targets
// @version         1.9
// @author          sb4ssman
// @include         explorer.exe
// @compilerOptions -luser32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Test Hook Explorer

This is a diagnostic mod. It does not change the taskbar appearance.

**INSTRUCTIONS:**
1. Click "Preview" or "Save & Enable".
2. Immediately switch to the **"Logs"** tab in the Windhawk editor.
3. The window tree dump will appear immediately.
*/
// ==/WindhawkModReadme==

#include <windows.h>

// Ensure constant is defined if header is missing it
#ifndef TB_BUTTONCOUNT
#define TB_BUTTONCOUNT (WM_USER + 24)
#endif

// Helper to log window structure recursively
void EnumWindowsRecursive(HWND hwnd, int depth) {
    // Safety check to prevent infinite recursion or stack overflow on deep trees
    if (depth > 10 || !hwnd) return;

    wchar_t className[256] = {0};
    GetClassNameW(hwnd, className, 255);
    
    wchar_t windowText[256] = {0};
    GetWindowTextW(hwnd, windowText, 255);

    // Simple indentation
    wchar_t indent[32] = {0};
    for(int i=0; i<depth*2 && i<30; i++) indent[i] = L' ';

    // Log current window
    Wh_Log(L"%s[HWND: %p] Class: %s, Text: %s", indent, hwnd, className, windowText);

    // Check for ToolbarWindow32
    if (lstrcmpW(className, L"ToolbarWindow32") == 0) {
        LRESULT count = SendMessageW(hwnd, TB_BUTTONCOUNT, 0, 0);
        Wh_Log(L"%s^^^ FOUND LEGACY TOOLBAR ^^^ Buttons: %d", indent, (int)count);
    }

    // Iterate children
    HWND child = GetWindow(hwnd, GW_CHILD);
    while (child) {
        EnumWindowsRecursive(child, depth + 1);
        child = GetWindow(child, GW_HWNDNEXT);
    }
}

BOOL Wh_ModInit() {
    Wh_Log(L"=== Test Hook Explorer v1.9 Init ===");
    
    HWND tray = FindWindowW(L"Shell_TrayWnd", NULL);
    if (tray) {
        Wh_Log(L"Found Shell_TrayWnd: %p", tray);
        EnumWindowsRecursive(tray, 0);
    } else {
        Wh_Log(L"Shell_TrayWnd not found");
    }
    
    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L"=== Test Hook Explorer Uninit ===");
}