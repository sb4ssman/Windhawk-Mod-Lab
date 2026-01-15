// ==WindhawkMod==
// @id              test-hook-explorer
// @name            Test Hook Explorer
// @description     Task Manager Tail (v16.3 Optimized)
// @version         16.3.1
// @author          sb4ssman
// @include         explorer.exe
// @compilerOptions -luser32 -loleacc -loleaut32 -luuid -lole32
// ==/WindhawkMod==

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
2. Close them randomly.
3. Ensure Task Manager stays at the end.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- checkInterval: 50
  $name: Check Interval (ms)
  $description: How often to check the taskbar order.
*/
// ==/WindhawkModSettings==

#define _WIN32_WINNT 0x0600 // Ensure Vista+ APIs are visible
#include <windows.h>
#include <objbase.h>
#include <uiautomation.h>
#include <shobjidl.h> // For ITaskbarList
#include <stdio.h>
#include <vector>
#include <string>
#include <stdarg.h> // Required for va_list
#include <wchar.h>  // Required for wcs functions

struct UIAItem {
    IUIAutomationElement* element;
    RECT rect;
    BSTR name;
};

struct Settings {
    int checkInterval;
} g_settings;

// Global thread control
HANDLE g_hThread = NULL;
volatile bool g_stopThread = false;
DWORD g_lastAttemptTime = 0;

void LoadSettings() {
    g_settings.checkInterval = Wh_GetIntSetting(L"checkInterval");
    if (g_settings.checkInterval < 10) g_settings.checkInterval = 10; // Safety floor
}

void Log(const wchar_t* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    wchar_t buffer[1024];
    // Use standard vswprintf to ensure compatibility
    vswprintf(buffer, 1024, fmt, args);
    va_end(args);
    Wh_Log(L"%s", buffer);
}

// Helper to check if user is interacting
bool IsUserIdle() {
    LASTINPUTINFO lii = { sizeof(LASTINPUTINFO) };
    if (GetLastInputInfo(&lii)) {
        DWORD tick = GetTickCount();
        if (tick - lii.dwTime < 2000) return false;
    }
    return true;
}

void CycleTaskbarTab(HWND hwnd) {
    Log(L"Cycling Taskbar Tab for HWND %p via ITaskbarList", hwnd);

    ITaskbarList* pTaskbarList = NULL;
    HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList, (void**)&pTaskbarList);
    
    if (SUCCEEDED(hr) && pTaskbarList) {
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
        }
        pTaskbarList->Release();
    } else {
        Log(L"CoCreateInstance(CLSID_TaskbarList) failed: 0x%08X", (unsigned int)hr);
    }
    
    Log(L"Taskbar Tab Cycle Complete.");
}

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

    HWND hChild = GetWindow(hTray, GW_CHILD);
    while (hChild) {
        wchar_t text[256] = {0};
        GetWindowTextW(hChild, text, 255);
        
        if (wcsstr(text, L"DesktopWindowXamlSource")) {
            IUIAutomationElement* pRoot = NULL;
            HRESULT hr = pAutomation->ElementFromHandle(hChild, &pRoot);
            if (SUCCEEDED(hr) && pRoot) {
                IUIAutomationCondition* pTrueCondition = NULL;
                pAutomation->CreateTrueCondition(&pTrueCondition);
                
                IUIAutomationElementArray* pChildren = NULL;
                pRoot->FindAll(TreeScope_Descendants, pTrueCondition, &pChildren);
                
                if (pChildren) {
                    int count = 0;
                    pChildren->get_Length(&count);
                    
                    std::vector<UIAItem> buttons;
                    int taskMgrIndex = -1;

                    for (int i = 0; i < count; i++) {
                        IUIAutomationElement* pChild = NULL;
                        pChildren->GetElement(i, &pChild);
                        if (pChild) {
                            BSTR cls = NULL;
                            pChild->get_CurrentClassName(&cls);
                            
                            if (cls && wcsstr(cls, L"TaskListButtonAutomationPeer")) {
                                UIAItem item;
                                item.element = pChild;
                                pChild->AddRef();
                                pChild->get_CurrentName(&item.name);
                                pChild->get_CurrentBoundingRectangle(&item.rect);
                                
                                buttons.push_back(item);
                                
                                if (item.name && wcsstr(item.name, L"Task Manager")) {
                                    taskMgrIndex = (int)buttons.size() - 1;
                                }
                            }
                            if (cls) SysFreeString(cls);
                            pChild->Release();
                        }
                    }
                    
                    // Logic to move
                    if (taskMgrIndex != -1 && static_cast<size_t>(taskMgrIndex) < buttons.size() - 1) {
                        if (!IsUserIdle()) {
                            // Busy
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

                    // Cleanup
                    for (auto& b : buttons) {
                        if (b.name) SysFreeString(b.name);
                        if (b.element) b.element->Release();
                    }
                    pChildren->Release();
                }
                pTrueCondition->Release();
                pRoot->Release();
            }
        }
        hChild = GetWindow(hChild, GW_HWNDNEXT);
    }
}

DWORD WINAPI BackgroundThread(LPVOID) {
    Log(L"Background Thread Started");
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) return 1;

    IUIAutomation* pAutomation = NULL;
    hr = CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&pAutomation);
    
    if (SUCCEEDED(hr) && pAutomation) {
        Log(L"UIA Interface obtained. Monitoring...");
        while (!g_stopThread) {
            CheckAndMove(pAutomation);
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            Sleep(g_settings.checkInterval); // Use user setting for loop delay
        }
        pAutomation->Release();
    }
    CoUninitialize();
    Log(L"Background Thread Stopped");
    return 0;
}

BOOL Wh_ModInit() {
    Wh_Log(L"=== Test Hook Explorer v16.3 (Optimized) ===");
    LoadSettings();
    g_stopThread = false;
    g_hThread = CreateThread(NULL, 0, BackgroundThread, NULL, 0, NULL);
    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L"=== Test Hook Explorer Uninit (v16.3) ===");
    g_stopThread = true;
    if (g_hThread) {
        WaitForSingleObject(g_hThread, 3000);
        CloseHandle(g_hThread);
    }
}

void Wh_ModSettingsChanged() {
    Wh_Log(L"Settings Changed");
    LoadSettings();
}
