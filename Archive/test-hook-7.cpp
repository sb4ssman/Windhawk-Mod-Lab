//THIS ONE HIJACKS THE MOUSE: DO NOT USE!

// ==WindhawkMod==
// @id              test-hook-explorer
// @name            Test Hook Explorer
// @description     Auto-Tail Background Service (v7.3)
// @version         7.3
// @author          sb4ssman
// @include         explorer.exe
// @compilerOptions -luser32 -loleacc -loleaut32 -luuid -lole32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Test Hook Explorer v7.3 (Auto-Tail + Safety)

This version runs in the background and automatically moves Task Manager to the
end of the taskbar.

**CHANGES:**
- **Fixed:** Added missing `Wh_ModUninit` (critical for mod loading).
- **New:** Added "User Busy" check. It won't move the button if you are clicking.
- **New:** Uses `SendInput` (modern) instead of `mouse_event`.

**INSTRUCTIONS:**
1. Compile and Enable.
2. Open Task Manager.
3. The mod will automatically drag it to the end.
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

// Helper to check if user is interacting with mouse
bool IsUserBusy() {
    // If Left or Right mouse button is currently down, user is busy
    if (GetAsyncKeyState(VK_LBUTTON) < 0) return true;
    if (GetAsyncKeyState(VK_RBUTTON) < 0) return true;
    return false;
}

// Helper to simulate mouse drag using SendInput
void SimulateDrag(POINT start, POINT end) {
    Wh_Log(L"Simulating Drag: (%d, %d) -> (%d, %d)", start.x, start.y, end.x, end.y);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    // Helper to convert to absolute coordinates (0-65535)
    auto toAbs =  -> int {
        return (val * 65535) / max;
    };

    // 1. Move to Start
    SetCursorPos(start.x, start.y);
    Sleep(50);

    // 2. Mouse Down
    INPUT inputs[1] = {};
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, inputs, sizeof(INPUT));
    Sleep(50);

    // 3. Drag in steps
    int steps = 10;
    int dx = (end.x - start.x) / steps;
    int dy = (end.y - start.y) / steps;

    for (int i = 0; i < steps; i++) {
        if (g_stopThread) break;
        POINT cur;
        GetCursorPos(&cur);
        SetCursorPos(cur.x + dx, cur.y + dy);
        Sleep(5);
    }
    SetCursorPos(end.x, end.y);
    Sleep(50);

    // 4. Mouse Up
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, inputs, sizeof(INPUT));
    
    Wh_Log(L"Drag Complete.");
}

void CheckAndMove(IUIAutomation* pAutomation, bool verbose) {
    HWND hTray = FindWindowW(L"Shell_TrayWnd", NULL);
    if (!hTray) {
        if (verbose) Wh_Log(L"Shell_TrayWnd not found");
        return;
    }

    HWND hChild = GetWindow(hTray, GW_CHILD);
    while (hChild) {
        wchar_t text[256] = {0};
        GetWindowTextW(hChild, text, 255);
        
        if (wcsstr(text, L"DesktopWindowXamlSource")) {
            if (verbose) Wh_Log(L"Checking XAML Window: %p", hChild);

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
                    
                    if (verbose) Wh_Log(L"  -> Found %d buttons. TaskMgr Index: %d", buttons.size(), taskMgrIndex);

                    // Logic to move
                    if (taskMgrIndex != -1 && static_cast<size_t>(taskMgrIndex) < buttons.size() - 1) {
                        // Check if user is busy before we interrupt
                        if (IsUserBusy()) {
                            Wh_Log(L"Task Manager out of place, but user is busy (mouse down). Waiting...");
                        } else {
                            UIAItem& taskMgr = buttons[taskMgrIndex];
                            UIAItem& lastBtn = buttons.back();
                            
                            Wh_Log(L"Task Manager found at index %d (of %d). Moving...", taskMgrIndex, buttons.size());

                            POINT start = { 
                                (taskMgr.rect.left + taskMgr.rect.right) / 2, 
                                (taskMgr.rect.top + taskMgr.rect.bottom) / 2 
                            };
                            
                            // Target: Right edge of last button + 20 pixels (GAP)
                            POINT end = { 
                                lastBtn.rect.right + 20, 
                                (lastBtn.rect.top + lastBtn.rect.bottom) / 2 
                            };
                            
                            SimulateDrag(start, end);
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
            } else {
                if (verbose) Wh_Log(L"  -> ElementFromHandle failed: 0x%08X", (unsigned int)hr);
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
        Wh_Log(L"CoInitialize failed in thread: 0x%08X", (unsigned int)hr);
        return 1;
    }

    IUIAutomation* pAutomation = NULL;
    hr = CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&pAutomation);
    
    if (SUCCEEDED(hr) && pAutomation) {
        Wh_Log(L"UIA Interface obtained. Starting loop...");
        
        int tick = 0;
        while (!g_stopThread) {
            // Log verbose output every 5 seconds (approx 25 ticks)
            bool verbose = (tick % 25 == 0);
            
            CheckAndMove(pAutomation, verbose);
            
            // Message Pump (Required for STA COM)
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            // Check every 200ms
            Sleep(200);
            tick++;
            
            if (tick > 10000) tick = 0;
        }
        pAutomation->Release();
    } else {
        Wh_Log(L"Failed to create UIA in thread: 0x%08X", (unsigned int)hr);
    }

    CoUninitialize();
    Wh_Log(L"Background Thread Stopped");
    return 0;
}

BOOL Wh_ModInit() {
    Wh_Log(L"=== Test Hook Explorer v7.3 (Auto-Tail + Safety) ===");
    
    g_stopThread = false;
    g_hThread = CreateThread(NULL, 0, BackgroundThread, NULL, 0, NULL);
    
    if (!g_hThread) {
        Wh_Log(L"Failed to create background thread!");
    }
    
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
