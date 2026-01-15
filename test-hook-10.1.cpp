// ==WindhawkMod==
// @id              test-hook-explorer
// @name            Test Hook Explorer
// @description     Virtual Input Probe (v10.1)
// @version         10.1
// @author          sb4ssman
// @include         explorer.exe
// @compilerOptions -luser32 -loleacc -loleaut32 -luuid -lole32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Test Hook Explorer v10.1 (Virtual Input Probe)

This version attempts to move the Task Manager button using **Virtual Window Messages**.
It sends `WM_LBUTTONDOWN` and `WM_MOUSEMOVE` directly to the Taskbar window handle.

**Goal:** Move the button *without* moving your physical mouse cursor.

**INSTRUCTIONS:**
1. Compile and Enable.
2. Open Task Manager.
3. Drag it to the middle of the taskbar.
4. Wait 5 seconds.
5. See if it snaps back to the end.
*/
// ==/WindhawkModReadme==

#include <windows.h>
#include <objbase.h>
#include <uiautomation.h>
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

// Helper to send virtual mouse messages
void VirtualDrag(HWND hwnd, POINT startScreen, POINT endScreen) {
    Wh_Log(L"Executing Virtual Drag on HWND %p", hwnd);
    Wh_Log(L"  Screen: (%d, %d) -> (%d, %d)", startScreen.x, startScreen.y, endScreen.x, endScreen.y);

    // Convert screen coordinates to client coordinates relative to the XAML window
    POINT startClient = startScreen;
    POINT endClient = endScreen;
    ScreenToClient(hwnd, &startClient);
    ScreenToClient(hwnd, &endClient);

    Wh_Log(L"  Client: (%d, %d) -> (%d, %d)", startClient.x, startClient.y, endClient.x, endClient.y);

    // Helper macros for packing coordinates into LPARAM
    #define MAKELPARAM_PT(pt) (LPARAM)(MAKELONG((short)pt.x, (short)pt.y))

    // 1. Mouse Down at Start
    PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM_PT(startClient));
    Sleep(50);

    // 2. Drag in steps
    int steps = 10;
    for(int i=1; i<=steps; i++) {
        int x = startClient.x + (endClient.x - startClient.x) * i / steps;
        int y = startClient.y + (endClient.y - startClient.y) * i / steps;
        POINT pt = {x, y};
        PostMessage(hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM_PT(pt));
        Sleep(10);
    }

    // 3. Hold at End
    PostMessage(hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM_PT(endClient));
    Sleep(200);

    // 4. Mouse Up
    PostMessage(hwnd, WM_LBUTTONUP, 0, MAKELPARAM_PT(endClient));
    
    Wh_Log(L"Virtual Drag Messages Sent.");
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
                        UIAItem& taskMgr = buttons[taskMgrIndex];
                        UIAItem& lastBtn = buttons.back();
                        
                        Wh_Log(L"Task Manager found at index %d (of %d). Attempting Virtual Drag...", taskMgrIndex, buttons.size());

                        POINT start = { 
                            (taskMgr.rect.left + taskMgr.rect.right) / 2, 
                            (taskMgr.rect.top + taskMgr.rect.bottom) / 2 
                        };
                        
                        // Target: 75% across last button
                        int btnWidth = lastBtn.rect.right - lastBtn.rect.left;
                        POINT end = { 
                            lastBtn.rect.left + (int)(btnWidth * 0.75), 
                            (lastBtn.rect.top + lastBtn.rect.bottom) / 2 
                        };
                        
                        // Pass the HWND of the XAML window
                        VirtualDrag(hChild, start, end);
                        g_lastAttemptTime = GetTickCount();
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
    Wh_Log(L"=== Test Hook Explorer v10.1 (Virtual Input) ===");
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
