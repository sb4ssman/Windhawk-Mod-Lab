// @id              test-hook-explorer
// @name            Test Hook Explorer
// @description     Task Manager Tail (v16.3 Optimized)
// @version         16.3
// @author          sb4ssman
// @include         explorer.exe
// @compilerOptions -luser32 -loleacc -loleaut32 -luuid -lole32

// ==WindhawkModReadme==
/*
# Task Manager Tail (v16.3 Optimized)

Automatically keeps Task Manager (or a target application) as the last item on the taskbar.

**Method:**
Uses the `ITaskbarList` interface to programmatically remove and re-add the taskbar button,
which effectively moves it to the end of the list.

**Optimization:**
Checks if the Task Manager window exists *before* scanning the taskbar. This ensures near-zero
CPU usage when the application is not running.

**Stress Testing:**
1. Open many windows (e.g., 10 Notepads).

// ==WindhawkModSettings==
/*
- checkInterval: 50
  $name: Check Interval (ms)
  $description: How often to check the taskbar order.
*/
#include <vector>
#include <string>
#include <stdarg.h> // Required for va_list
#include <wchar.h>  // Required for wcs functions

struct UIAItem {
    IUIAutomationElement* element;

void LoadSettings() {
    g_settings.checkInterval = Wh_GetIntSetting(L"checkInterval");
    if (g_settings.checkInterval < 10) g_settings.checkInterval = 10; // Safety floor
}

void Log(const wchar_t* fmt, ...) {
        hr = pTaskbarList->HrInit();
        if (SUCCEEDED(hr)) {
            // 1. Delete Tab
            Log(L"Calling DeleteTab...");
            pTaskbarList->DeleteTab(hwnd);
            
            Sleep(200);
            
            // 2. Add Tab
            Log(L"Calling AddTab...");
            pTaskbarList->AddTab(hwnd);
            
            Sleep(50);
            
            // 3. Activate Tab
            Log(L"Calling ActivateTab...");
            pTaskbarList->ActivateTab(hwnd);
        } else {
            Log(L"ITaskbarList::HrInit failed: 0x%08X", (unsigned int)hr);
void CheckAndMove(IUIAutomation* pAutomation) {
    // Cooldown to prevent spamming if something goes wrong, separate from checkInterval
    if (GetTickCount() - g_lastAttemptTime < 1000) return;

    // OPTIMIZATION: Check if Task Manager is even open before scanning the taskbar.
    // This saves massive CPU when the app is not running.
    HWND hTaskMgr = FindWindowW(L"TaskManagerWindow", NULL);
    if (!hTaskMgr) {
        return;
    }

    HWND hTray = FindWindowW(L"Shell_TrayWnd", NULL);
    if (!hTray) return;
                        } else {
                            Log(L"Task Manager found at index %d (of %d). Attempting ITaskbarList Cycle...", taskMgrIndex, buttons.size());
                            
                            // Use the handle we found at the start of the function
                            if (hTaskMgr && IsWindow(hTaskMgr)) {
                                CycleTaskbarTab(hTaskMgr);
                                g_lastAttemptTime = GetTickCount();
                            } else {
                                Log(L"Could not find TaskManagerWindow handle.");
                            }
                        }
                    }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            Sleep(g_settings.checkInterval); // Use user setting for loop delay
        }
        pAutomation->Release();
    }
}

BOOL Wh_ModInit() {
    Wh_Log(L"=== Test Hook Explorer v16.3 (Optimized) ===");
    LoadSettings();
    g_stopThread = false;
    g_hThread = CreateThread(NULL, 0, BackgroundThread, NULL, 0, NULL);
}

void Wh_ModUninit() {
    Wh_Log(L"=== Test Hook Explorer Uninit (v16.3) ===");
    g_stopThread = true;
    if (g_hThread) {
        WaitForSingleObject(g_hThread, 3000);
    }
}

