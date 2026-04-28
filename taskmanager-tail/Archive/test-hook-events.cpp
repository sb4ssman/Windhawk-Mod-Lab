// ==WindhawkMod==
// @id              test-hook-events
// @name            Test Hook Explorer (Event Driven)
// @description     Task Manager Tail (v17.0 Experimental)
// @version         17.0
// @author          sb4ssman
// @include         explorer.exe
// @compilerOptions -luser32 -loleacc -loleaut32 -luuid -lole32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Task Manager Tail (v17.0 Event Driven)

This is an experimental version that replaces the polling timer with **UI Automation Events**.

**How it works:**
Instead of checking every 50ms, this script registers a `StructureChangedEventHandler` 
on the Taskbar. It sleeps until Windows notifies it that the Taskbar layout has changed 
(e.g., a button was added or removed).

**Advantages:**
- Zero CPU usage when idle.
- Instant reaction to new windows.
*/
// ==/WindhawkModReadme==

#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <objbase.h>
#include <uiautomation.h>
#include <shobjidl.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <stdarg.h>
#include <wchar.h>

// Custom Message to wake up the thread
#define WM_TRIGGER_CHECK (WM_USER + 1)

struct UIAItem {
    IUIAutomationElement* element;
    RECT rect;
    BSTR name;
};

// Global thread control
HANDLE g_hThread = NULL;
DWORD g_dwThreadId = 0;
volatile bool g_stopThread = false;
DWORD g_lastAttemptTime = 0;

void Log(const wchar_t* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    wchar_t buffer[1024];
    vswprintf(buffer, 1024, fmt, args);
    va_end(args);
    Wh_Log(L"%s", buffer);
}

// --- WinEvent Hook Callback ---
void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, 
                           LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
    // Filter: We only care about top-level window events (OBJID_WINDOW)
    // This reduces noise significantly compared to UIA events.
    if (idObject == OBJID_WINDOW && idChild == CHILDID_SELF && g_dwThreadId != 0) {
        // Post message to trigger check (debounced in main loop)
        PostThreadMessage(g_dwThreadId, WM_TRIGGER_CHECK, 0, 0);
    }
}

void CycleTaskbarTab(HWND hwnd) {
    Log(L"Cycling Taskbar Tab for HWND %p via ITaskbarList", hwnd);

    ITaskbarList* pTaskbarList = NULL;
    HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList, (void**)&pTaskbarList);
    
    if (SUCCEEDED(hr) && pTaskbarList) {
        hr = pTaskbarList->HrInit();
        if (SUCCEEDED(hr)) {
            pTaskbarList->DeleteTab(hwnd);
            Sleep(200);
            pTaskbarList->AddTab(hwnd);
            Sleep(50);
            pTaskbarList->ActivateTab(hwnd);
        }
        pTaskbarList->Release();
    }
}

void CheckAndMove(IUIAutomation* pAutomation) {
    // Note: Cooldown removed here because we use a debounce timer in the message loop.
    // We want to check exactly when the timer fires.

    HWND hTaskMgr = FindWindowW(L"TaskManagerWindow", NULL);
    if (!hTaskMgr) return;

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
                                
                                buttons.push_back(item);
                                
                                if (item.name && wcsstr(item.name, L"Task Manager")) {
                                    taskMgrIndex = (int)buttons.size() - 1;
                                }
                            }
                            if (cls) SysFreeString(cls);
                            pChild->Release();
                        }
                    }
                    
                    if (taskMgrIndex != -1 && static_cast<size_t>(taskMgrIndex) < buttons.size() - 1) {
                        // Only move if we haven't moved recently (safety cooldown)
                        if (GetTickCount() - g_lastAttemptTime > 1000) {
                            Log(L"Event Triggered: Task Manager found at index %d (of %d). Moving...", taskMgrIndex, buttons.size());
                            if (hTaskMgr && IsWindow(hTaskMgr)) {
                                Sleep(100); // Short delay to be less aggressive
                                CycleTaskbarTab(hTaskMgr);
                                g_lastAttemptTime = GetTickCount();
                            }
                        }
                    }

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
    Log(L"Event-Driven Thread Started");
    g_dwThreadId = GetCurrentThreadId();

    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) return 1;

    IUIAutomation* pAutomation = NULL;
    hr = CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&pAutomation);
    
    if (SUCCEEDED(hr) && pAutomation) {
        // Register WinEventHook for Window Creation/Destruction/Show/Hide
        // This is much lighter than UIA events and won't deadlock Explorer.
        HWINEVENTHOOK hHook = SetWinEventHook(
            EVENT_OBJECT_CREATE, EVENT_OBJECT_HIDE, // Range: Create, Destroy, Show, Hide
            NULL,
            WinEventProc,
            0, 0, // Global hook (all processes/threads) to catch new apps opening
            WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS
        );

        if (hHook) {
            Log(L"WinEventHook Registered. Waiting for window events...");
            
            MSG msg;
            UINT_PTR debounceTimer = 0;

            while (!g_stopThread && GetMessage(&msg, NULL, 0, 0)) {
                if (msg.message == WM_TRIGGER_CHECK) {
                    // Debounce: Reset timer to fire in 300ms
                    if (debounceTimer) KillTimer(NULL, debounceTimer);
                    debounceTimer = SetTimer(NULL, 0, 300, NULL);
                }
                else if (msg.message == WM_TIMER && msg.wParam == debounceTimer) {
                    KillTimer(NULL, debounceTimer);
                    debounceTimer = 0;
                    CheckAndMove(pAutomation);
                }
                
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            
            UnhookWinEvent(hHook);
        } else {
            Log(L"Failed to register WinEventHook: %d", GetLastError());
        }
        pAutomation->Release();
    }
    CoUninitialize();
    Log(L"Thread Stopped");
    return 0;
}

BOOL Wh_ModInit() {
    Wh_Log(L"=== Test Hook Explorer v17.0 (Event Driven) ===");
    g_stopThread = false;
    g_hThread = CreateThread(NULL, 0, BackgroundThread, NULL, 0, NULL);
    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L"=== Test Hook Explorer Uninit ===");
    g_stopThread = true;
    if (g_dwThreadId) PostThreadMessage(g_dwThreadId, WM_QUIT, 0, 0);
    if (g_hThread) {
        WaitForSingleObject(g_hThread, 3000);
        CloseHandle(g_hThread);
    }
}