// ==WindhawkMod==
// @id              test-hook-explorer
// @name            Test Hook Explorer
// @description     Test mod to probe explorer.exe functions and taskbar behavior
// @version         0.1
// @author          sb4ssman
// @include         explorer.exe
// @compilerOptions
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Test Hook Explorer

This is a test mod to probe and log explorer.exe behavior, specifically
related to taskbar operations. Use this to identify function names and
understand how the taskbar works.

**WARNING**: This is a debugging/test mod. Use with caution.
*/
// ==/WindhawkModReadme==

#include <windows.h>
#include <commctrl.h>

// Hook CreateWindowExW to see what windows are being created
using CreateWindowExW_t = decltype(&CreateWindowExW);
CreateWindowExW_t CreateWindowExW_Original;

HWND WINAPI CreateWindowExW_Hook(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName,
                                  DWORD dwStyle, int X, int Y, int nWidth, int nHeight,
                                  HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    HWND hwnd = CreateWindowExW_Original(dwExStyle, lpClassName, lpWindowName, dwStyle,
                                        X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    
    // Log taskbar-related window creation
    if (lpClassName) {
        if (wcscmp(lpClassName, L"Shell_TrayWnd") == 0 ||
            wcscmp(lpClassName, L"ToolbarWindow32") == 0 ||
            wcscmp(lpClassName, L"TaskManagerWindow") == 0) {
            Wh_Log(L"CreateWindowExW: Class=%s, Title=%s, Handle=0x%p",
                   lpClassName, lpWindowName ? lpWindowName : L"(null)", hwnd);
        }
    }
    
    return hwnd;
}

// Hook SendMessageW to see taskbar messages
using SendMessageW_t = decltype(&SendMessageW);
SendMessageW_t SendMessageW_Original;

LRESULT WINAPI SendMessageW_Hook(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    // Check if this is a message to a toolbar (taskbar)
    wchar_t className[256] = {0};
    if (GetClassNameW(hWnd, className, 256) > 0) {
        if (wcscmp(className, L"ToolbarWindow32") == 0) {
            // Log interesting toolbar messages
            if (Msg == TB_BUTTONCOUNT || Msg == TB_GETBUTTON || 
                Msg == TB_MOVEBUTTON || Msg == TB_INSERTBUTTON ||
                Msg == TB_DELETEBUTTON || Msg == TB_ADDBUTTONS) {
                Wh_Log(L"SendMessageW to ToolbarWindow32: Msg=0x%04X, wParam=0x%p, lParam=0x%p",
                       Msg, wParam, lParam);
            }
        }
    }
    
    return SendMessageW_Original(hWnd, Msg, wParam, lParam);
}

BOOL Wh_ModInit() {
    Wh_Log(L"Test Hook Explorer mod initializing");
    
    // Hook CreateWindowExW to see window creation
    Wh_SetFunctionHook((void*)CreateWindowExW, (void*)CreateWindowExW_Hook,
                       (void**)&CreateWindowExW_Original);
    
    // Hook SendMessageW to see taskbar messages
    Wh_SetFunctionHook((void*)SendMessageW, (void*)SendMessageW_Hook,
                       (void**)&SendMessageW_Original);
    
    Wh_Log(L"Test Hook Explorer mod initialized - check logs for taskbar activity");
    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L"Test Hook Explorer mod unloading");
}
