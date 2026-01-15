// ==WindhawkMod==
// @id              test-hook-explorer
// @name            Test Hook Explorer
// @description     ITaskbarList Probe (v16.0)
// @version         16.0
// @author          sb4ssman
// @include         explorer.exe
// @compilerOptions -luser32 -loleacc -loleaut32 -luuid -lole32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Test Hook Explorer v16.0 (ITaskbarList)

This version attempts to move the Task Manager button using the **ITaskbarList** interface.
This is the official API used by applications to manage their taskbar buttons.

**Mechanism:**
1. Detects if Task Manager is not at the end.
2. Creates an `ITaskbarList` instance.
3. Calls `DeleteTab(hwnd)` to remove the button.
4. Calls `AddTab(hwnd)` to re-add the button (hopefully at the end).
5. Calls `ActivateTab(hwnd)` to ensure it's active.

**Hypothesis:**
Since we are injected into `explorer.exe`, this API might allow us to manipulate buttons
even for elevated windows, or at least trigger the internal logic to refresh the button.

**INSTRUCTIONS:**
1. Compile and Enable.
2. Open Task Manager.
3. Drag it to the middle of the taskbar.
4. Wait 5 seconds.
*/
// ==/WindhawkModReadme==

#include <windows.h>
#include <objbase.h>
#include <uiautomation.h>
#include <shobjidl.h> // For ITaskbarList
#include <stdio.h>
#include <vector>

struct UIAItem {
    IUIAutomationElement* element;
    RECT rect;
    BSTR name;
};

// Global thread control
HANDLE g_hThread = NULL;
volatile bool g_stopThread = false;
DWORD g_lastAttemptTime = 0;

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
    Wh_Log(L"Cycling Taskbar Tab for HWND %p via ITaskbarList", hwnd);

    ITaskbarList* pTaskbarList = NULL;
    HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList, (void**)&pTaskbarList);
    
    if (SUCCEEDED(hr) && pTaskbarList) {
        hr = pTaskbarList->HrInit();
        if (SUCCEEDED(hr)) {
            // 1. Delete Tab
            Wh_Log(L"Calling DeleteTab...");
            pTaskbarList->DeleteTab(hwnd);
            
            Sleep(200);
            
            // 2. Add Tab
            Wh_Log(L"Calling AddTab...");
            pTaskbarList->AddTab(hwnd);
            
            Sleep(50);
            
            // 3. Activate Tab
            Wh_Log(L"Calling ActivateTab...");
            pTaskbarList->ActivateTab(hwnd);
        } else {
            Wh_Log(L"ITaskbarList::HrInit failed: 0x%08X", (unsigned int)hr);
        }
        pTaskbarList->Release();
    } else {
        Wh_Log(L"CoCreateInstance(CLSID_TaskbarList) failed: 0x%08X", (unsigned int)hr);
    }
    
    Wh_Log(L"Taskbar Tab Cycle Complete.");
}

void CheckAndMove(IUIAutomation* pAutomation) {
    if (GetTickCount() - g_lastAttemptTime < 5000) return;

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
                            Wh_Log(L"Task Manager found at index %d (of %d). Attempting ITaskbarList Cycle...", taskMgrIndex, buttons.size());
                            
                            // Find the actual Task Manager window
                            HWND hTaskMgr = FindWindowW(L"TaskManagerWindow", NULL);
                            if (hTaskMgr) {
                                CycleTaskbarTab(hTaskMgr);
                                g_lastAttemptTime = GetTickCount();
                            } else {
                                Wh_Log(L"Could not find TaskManagerWindow handle.");
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
    Wh_Log(L"Background Thread Started");
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) return 1;

    IUIAutomation* pAutomation = NULL;
    hr = CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&pAutomation);
    
    if (SUCCEEDED(hr) && pAutomation) {
        Wh_Log(L"UIA Interface obtained. Monitoring...");
        while (!g_stopThread) {
            CheckAndMove(pAutomation);
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            Sleep(1000);
        }
        pAutomation->Release();
    }
    CoUninitialize();
    Wh_Log(L"Background Thread Stopped");
    return 0;
}

BOOL Wh_ModInit() {
    Wh_Log(L"=== Test Hook Explorer v16.0 (ITaskbarList) ===");
    g_stopThread = false;
    g_hThread = CreateThread(NULL, 0, BackgroundThread, NULL, 0, NULL);
    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L"=== Test Hook Explorer Uninit ===");
    g_stopThread = true;
    if (g_hThread) {
        WaitForSingleObject(g_hThread, 3000);
        CloseHandle(g_hThread);
    }
}
