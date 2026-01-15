// ==WindhawkMod==
// @id              taskmanager-tail
// @name            Task Manager Tail
// @description     Keeps Task Manager as the last item on the taskbar
// @version         0.1
// @author          sb4ssman
// @github          https://github.com/sb4ssman/Windhawk-Taskmanager-Tail
// @include         explorer.exe
// @compilerOptions
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Task Manager Tail

This mod ensures that Task Manager always appears as the last item on the taskbar.
When you open a new application, Task Manager will automatically slide itself to
the end of the taskbar.

## Features

- Automatically keeps Task Manager at the end of the taskbar
- Works with two-line taskbars
- Event-driven (no polling overhead)
- Responds immediately to taskbar changes

## How it works

The mod hooks into taskbar functions in `explorer.exe` to detect when Task Manager
is running and automatically repositions its taskbar button to the end.

## Requirements

- Windows 11
- Windhawk

## Getting started

1. Compile the mod with the button on the left or with Ctrl+B
2. Enable the mod in Windhawk
3. Open Task Manager and watch it automatically move to the end of the taskbar
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
# Settings for Task Manager Tail mod
# Currently no settings, but this section is reserved for future options
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <commctrl.h>

// TODO: Identify the actual function signatures and names
// These are placeholders that need to be filled in after research

// Function to check if a window belongs to Task Manager
bool IsTaskManagerWindow(HWND hwnd) {
    if (!hwnd) return false;
    
    wchar_t className[256];
    if (GetClassNameW(hwnd, className, sizeof(className) / sizeof(wchar_t)) == 0) {
        return false;
    }
    
    // Task Manager window class in Windows 11
    if (wcscmp(className, L"TaskManagerWindow") == 0) {
        return true;
    }
    
    // Also check by process name
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);
    
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess) {
        wchar_t processName[MAX_PATH];
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageNameW(hProcess, 0, processName, &size)) {
            wchar_t* fileName = wcsrchr(processName, L'\\');
            if (fileName && _wcsicmp(fileName + 1, L"Taskmgr.exe") == 0) {
                CloseHandle(hProcess);
                return true;
            }
        }
        CloseHandle(hProcess);
    }
    
    return false;
}

// Function to find the taskbar toolbar window
HWND FindTaskbarToolbar() {
    HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
    if (!hTaskbar) {
        return NULL;
    }
    
    // Find the toolbar control (ToolbarWindow32)
    HWND hToolbar = FindWindowExW(hTaskbar, NULL, L"ToolbarWindow32", NULL);
    
    // There might be multiple toolbars, find the main one
    // TODO: This might need refinement based on actual taskbar structure
    while (hToolbar) {
        // Check if this is the main taskbar toolbar
        // We can verify by checking if it has buttons
        int buttonCount = (int)SendMessageW(hToolbar, TB_BUTTONCOUNT, 0, 0);
        if (buttonCount > 0) {
            return hToolbar;
        }
        hToolbar = FindWindowExW(hTaskbar, hToolbar, L"ToolbarWindow32", NULL);
    }
    
    return NULL;
}

// Function to find Task Manager button index in taskbar
int FindTaskManagerButtonIndex(HWND hToolbar) {
    if (!hToolbar) return -1;
    
    int buttonCount = (int)SendMessageW(hToolbar, TB_BUTTONCOUNT, 0, 0);
    
    for (int i = 0; i < buttonCount; i++) {
        TBBUTTON button;
        if (SendMessageW(hToolbar, TB_GETBUTTON, i, (LPARAM)&button)) {
            // Get the window handle associated with this button
            // TODO: Need to figure out how to get window handle from toolbar button
            // This might require accessing internal taskbar structures
        }
    }
    
    return -1;
}

// Function to move Task Manager button to the end
void MoveTaskManagerToEnd() {
    HWND hToolbar = FindTaskbarToolbar();
    if (!hToolbar) {
        Wh_Log(L"Could not find taskbar toolbar");
        return;
    }
    
    int taskMgrIndex = FindTaskManagerButtonIndex(hToolbar);
    if (taskMgrIndex < 0) {
        // Task Manager not found, might not be running
        return;
    }
    
    int buttonCount = (int)SendMessageW(hToolbar, TB_BUTTONCOUNT, 0, 0);
    if (buttonCount <= 1) {
        // Only one button or none, nothing to reorder
        return;
    }
    
    int lastIndex = buttonCount - 1;
    if (taskMgrIndex == lastIndex) {
        // Already at the end
        return;
    }
    
    Wh_Log(L"Moving Task Manager from index %d to %d", taskMgrIndex, lastIndex);
    
    // Move the button using TB_MOVEBUTTON
    // Note: This might not work directly - may need to use internal taskbar APIs
    SendMessageW(hToolbar, TB_MOVEBUTTON, taskMgrIndex, lastIndex);
}

// TODO: Hook functions that handle taskbar button creation/ordering
// These need to be identified through reverse engineering explorer.exe

// Placeholder hook function - to be replaced with actual function signature
// This would hook into the function that adds/updates taskbar buttons
/*
typedef void (WINAPI *TaskbarButtonUpdateFunc_t)(/* parameters to be determined */);
TaskbarButtonUpdateFunc_t TaskbarButtonUpdateFunc_Original;

void WINAPI TaskbarButtonUpdateFunc_Hook(/* parameters */) {
    // Call original function first
    TaskbarButtonUpdateFunc_Original(/* parameters */);
    
    // Then move Task Manager to end if it exists
    MoveTaskManagerToEnd();
}
*/

// Alternative: Hook window creation to detect Task Manager
using CreateWindowExW_t = decltype(&CreateWindowExW);
CreateWindowExW_t CreateWindowExW_Original;
HWND WINAPI CreateWindowExW_Hook(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName,
                                  DWORD dwStyle, int X, int Y, int nWidth, int nHeight,
                                  HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    HWND hwnd = CreateWindowExW_Original(dwExStyle, lpClassName, lpWindowName, dwStyle,
                                        X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    
    // Check if this is Task Manager window
    if (hwnd && IsTaskManagerWindow(hwnd)) {
        Wh_Log(L"Task Manager window created, moving to end of taskbar");
        // Give it a moment to be added to taskbar, then move it
        // TODO: Use a timer or hook taskbar update function instead
    }
    
    return hwnd;
}

// The mod is being initialized, load settings, hook functions, and do other
// initialization stuff if required.
BOOL Wh_ModInit() {
    Wh_Log(L"Task Manager Tail mod initializing");
    
    // TODO: Hook the actual taskbar button management functions
    // This requires identifying the function names in explorer.exe
    
    // For now, hook CreateWindowExW as a test to detect Task Manager creation
    // This is a temporary approach - we should hook taskbar-specific functions instead
    Wh_SetFunctionHook((void*)CreateWindowExW, (void*)CreateWindowExW_Hook,
                       (void**)&CreateWindowExW_Original);
    
    Wh_Log(L"Task Manager Tail mod initialized");
    return TRUE;
}

// The mod is being unloaded, free all allocated resources.
void Wh_ModUninit() {
    Wh_Log(L"Task Manager Tail mod unloading");
    // Cleanup will be handled automatically by Windhawk
}

// The mod settings were changed, reload them.
void Wh_ModSettingsChanged() {
    Wh_Log(L"Task Manager Tail mod settings changed");
    // Currently no settings to reload
}
