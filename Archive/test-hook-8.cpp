// ==WindhawkMod==
// @id              test-hook-explorer
// @name            Test Hook Explorer
// @description     Auto-Tail (Slow & Steady v9.3)
// @version         9.3
// @author          sb4ssman
// @include         explorer.exe
// @compilerOptions -luser32 -loleacc -loleaut32 -luuid -lole32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Test Hook Explorer v9.3 (Slow & Steady)

This version automatically moves Task Manager to the end of the taskbar.

**FIXES vs v9.2:**
- **Hold Time:** Holds the mouse at the drop target for 300ms to allow taskbar animations to finish.
- **Trajectory:** Moves in 5 steps to simulate a real drag.
- **Safety:** Waits for 2 seconds of user inactivity before acting.

**INSTRUCTIONS:**
1. Compile and Enable.
2. Open Task Manager.
3. Drag it out of place and release the mouse.
4. Wait 2 seconds. It should move.
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

// Helper to check if user is interacting with mouse/keyboard
bool IsUserIdle() {
    LASTINPUTINFO lii = { sizeof(LASTINPUTINFO) };
    if (GetLastInputInfo(&lii)) {
        DWORD tick = GetTickCount();
        // If input happened in the last 2000ms (2s), user is busy
        if (tick - lii.dwTime < 2000) return false;
    }
    return true;
}

// Helper to perform a DRAG operation with HOLD time
void SteadyDrag(POINT start, POINT end) {
    Wh_Log(L"Executing Steady Drag: (%d, %d) -> (%d, %d)", start.x, start.y, end.x, end.y);

    // 1. Save original position
    POINT originalPos;
    GetCursorPos(&originalPos);

    // 2. Prepare Inputs
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_MOUSE;
    inputs[1].type = INPUT_MOUSE;

    // Step A: Move to Start & Click Down
    SetCursorPos(start.x, start.y);
    Sleep(50); 
    
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &inputs[0], sizeof(INPUT));
    Sleep(100); // Grab time

    // Step B: Move to End (Drag) in steps
    int steps = 5;
    for(int i=1; i<=steps; i++) {
        int x = start.x + (end.x - start.x) * i / steps;
        int y = start.y + (end.y - start.y) * i / steps;
        SetCursorPos(x, y);
        Sleep(20);
    }
    
    // IMPORTANT: Hold to let Windows UI animations catch up
    Sleep(300); 

    // Step C: Click Up
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &inputs[1], sizeof(INPUT));
    Sleep(50);

    // Step D: Restore Cursor
    SetCursorPos(originalPos.x, originalPos.y);
    
    Wh_Log(L"Drag Complete.");
}

void CheckAndMove(IUIAutomation* pAutomation) {
    // Cooldown check: Don't retry immediately if we just tried (5 seconds)
    if (GetTickCount() - g_lastAttemptTime < 5000) {
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
                        // Check if user is busy before we interrupt
                        if (!IsUserIdle()) {
                            // Silent wait
                        } else {
                            UIAItem& taskMgr = buttons[taskMgrIndex];
                            UIAItem& lastBtn = buttons.back();
                            
                            Wh_Log(L"Task Manager found at index %d (of %d). Moving...", taskMgrIndex, buttons.size());

                            POINT start = { 
                                (taskMgr.rect.left + taskMgr.rect.right) / 2, 
                                (taskMgr.rect.top + taskMgr.rect.bottom) / 2 
                            };
                            
                            // Target: Right edge of last button MINUS 15 pixels.
                            // This ensures we are inside the button's hit area, triggering the "insert after" logic.
                            POINT end = { 
                                lastBtn.rect.right - 15, 
                                (lastBtn.rect.top + lastBtn.rect.bottom) / 2 
                            };
                            
                            SteadyDrag(start, end);
                            g_lastAttemptTime = GetTickCount();
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
    
    // Use STA with message pump
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        Wh_Log(L"CoInitialize failed: 0x%08X", (unsigned int)hr);
        return 1;
    }

    IUIAutomation* pAutomation = NULL;
    hr = CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&pAutomation);
    
    if (SUCCEEDED(hr) && pAutomation) {
        Wh_Log(L"UIA Interface obtained. Monitoring...");
        
        while (!g_stopThread) {
            CheckAndMove(pAutomation);
            
            // Message Pump (Required for STA COM)
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            // Check every 500ms
            Sleep(500);
        }
        pAutomation->Release();
    }

    CoUninitialize();
    Wh_Log(L"Background Thread Stopped");
    return 0;
}

BOOL Wh_ModInit() {
    Wh_Log(L"=== Test Hook Explorer v9.3 (Slow & Steady) ===");
    
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
