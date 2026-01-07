// ==WindhawkMod==
// @id              test-hook-explorer
// @name            Test Hook Explorer
// @description     Auto-Tail Background Service
// @version         7.0
// @author          sb4ssman
// @include         explorer.exe
// @compilerOptions -luser32 -loleacc -loleaut32 -luuid -lole32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Test Hook Explorer v7.0 (Auto-Tail)

This version runs in the background and automatically moves Task Manager to the
end of the taskbar whenever it detects it is out of place.

**CHANGES:**
- Runs in a background thread (fixes "Initializing..." hang).
- Adjusted drag coordinates to avoid grouping.
- Fixed compiler warnings.

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

// Helper to simulate mouse drag
void SimulateDrag(POINT start, POINT end) {
    Wh_Log(L"Simulating Drag: (%d, %d) -> (%d, %d)", start.x, start.y, end.x, end.y);

    // Save original position
    POINT originalPos;
    GetCursorPos(&originalPos);

    // 1. Move to Start
    SetCursorPos(start.x, start.y);
    Sleep(50);

    // 2. Mouse Down
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    Sleep(50);

    // 3. Drag in steps
    int steps = 10; // Faster than before
    int dx = (end.x - start.x) / steps;
    int dy = (end.y - start.y) / steps;

    for (int i = 0; i < steps; i++) {
        POINT cur;
        GetCursorPos(&cur);
        SetCursorPos(cur.x + dx, cur.y + dy);
        Sleep(5);
    }
    SetCursorPos(end.x, end.y);
    Sleep(50);

    // 4. Mouse Up
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    
    // Optional: Restore mouse? Might be disorienting if user was moving it.
    // SetCursorPos(originalPos.x, originalPos.y);
    
    Wh_Log(L"Drag Complete.");
}

void CheckAndMove(IUIAutomation* pAutomation) {
    HWND hTray = FindWindowW(L"Shell_TrayWnd", NULL);
    if (!hTray) return;

    HWND hChild = GetWindow(hTray, GW_CHILD);
    while (hChild) {
        wchar_t text[256] = {0};
        GetWindowTextW(hChild, text, 255);
        
        if (wcsstr(text, L"DesktopWindowXamlSource")) {
            IUIAutomationElement* pRoot = NULL;
            if (SUCCEEDED(pAutomation->ElementFromHandle(hChild, &pRoot)) && pRoot) {
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
                    // Fix warning: cast to size_t
                    if (taskMgrIndex != -1 && static_cast<size_t>(taskMgrIndex) < buttons.size() - 1) {
                        UIAItem& taskMgr = buttons[taskMgrIndex];
                        UIAItem& lastBtn = buttons.back();
                        
                        Wh_Log(L"Task Manager found at index %d (of %d). Moving...", taskMgrIndex, buttons.size());

                        POINT start = { 
                            (taskMgr.rect.left + taskMgr.rect.right) / 2, 
                            (taskMgr.rect.top + taskMgr.rect.bottom) / 2 
                        };
                        
                        // Target: Right edge of last button + 5 pixels (GAP)
                        // This prevents dropping ON the button (grouping)
                        POINT end = { 
                            lastBtn.rect.right + 5, 
                            (lastBtn.rect.top + lastBtn.rect.bottom) / 2 
                        };
                        
                        SimulateDrag(start, end);
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
    if (FAILED(hr)) {
        Wh_Log(L"CoInitialize failed in thread");
        return 1;
    }

    IUIAutomation* pAutomation = NULL;
    hr = CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&pAutomation);
    
    if (SUCCEEDED(hr) && pAutomation) {
        while (!g_stopThread) {
            CheckAndMove(pAutomation);
            
            // Check every 2 seconds
            for (int i = 0; i < 20; i++) {
                if (g_stopThread) break;
                Sleep(100);
            }
        }
        pAutomation->Release();
    } else {
        Wh_Log(L"Failed to create UIA in thread");
    }

    CoUninitialize();
    Wh_Log(L"Background Thread Stopped");
    return 0;
}

BOOL Wh_ModInit() {
    Wh_Log(L"=== Test Hook Explorer v7.0 (Auto-Tail) ===");
    
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
