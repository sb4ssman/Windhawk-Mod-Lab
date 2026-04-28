// ==WindhawkMod==
// @id              test-hook-explorer
// @name            Test Hook Explorer
// @description     Simulate Drag & Drop to Move Button
// @version         6.0
// @author          sb4ssman
// @include         explorer.exe
// @compilerOptions -luser32 -loleacc -loleaut32 -luuid -lole32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Test Hook Explorer v6.0 (Drag Simulation)

This version attempts to **MOVE** the Task Manager button!

It uses UIA to find the button coordinates and the coordinates of the last item
in the list, then simulates a mouse drag operation to move Task Manager to the end.

**INSTRUCTIONS:**
1. Compile and Enable.
2. Ensure Task Manager is open.
3. Wait for the Message Box asking for permission to move the mouse.
4. Click OK and **HANDS OFF THE MOUSE**.
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

// Helper to simulate mouse drag
void SimulateDrag(POINT start, POINT end) {
    Wh_Log(L"Simulating Drag: (%d, %d) -> (%d, %d)", start.x, start.y, end.x, end.y);

    // 1. Move to Start
    SetCursorPos(start.x, start.y);
    Sleep(100);

    // 2. Mouse Down
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    Sleep(100);

    // 3. Drag in steps (smoothness helps Windows recognize the drag)
    int steps = 20;
    int dx = (end.x - start.x) / steps;
    int dy = (end.y - start.y) / steps;

    for (int i = 0; i < steps; i++) {
        POINT cur;
        GetCursorPos(&cur);
        SetCursorPos(cur.x + dx, cur.y + dy);
        Sleep(10);
    }
    SetCursorPos(end.x, end.y);
    Sleep(100);

    // 4. Mouse Up
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    Wh_Log(L"Drag Complete.");
}

BOOL Wh_ModInit() {
    Wh_Log(L"=== Test Hook Explorer v6.0 (Drag Simulation) ===");
    
    HRESULT hr = CoInitialize(NULL);
    
    IUIAutomation* pAutomation = NULL;
    hr = CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&pAutomation);
    if (FAILED(hr) || !pAutomation) {
        Wh_Log(L"Failed to create UIA: 0x%08X", (unsigned int)hr);
        return FALSE;
    }

    HWND hTray = FindWindowW(L"Shell_TrayWnd", NULL);
    if (hTray) {
        HWND hChild = GetWindow(hTray, GW_CHILD);
        while (hChild) {
            wchar_t className[256] = {0};
            GetClassNameW(hChild, className, 255);
            wchar_t text[256] = {0};
            GetWindowTextW(hChild, text, 255);
            
            if (wcsstr(text, L"DesktopWindowXamlSource")) {
                IUIAutomationElement* pRoot = NULL;
                hr = pAutomation->ElementFromHandle(hChild, &pRoot);
                if (SUCCEEDED(hr) && pRoot) {
                    // Find all TaskListButtonAutomationPeer elements
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
                                
                                // Filter for taskbar buttons
                                if (cls && wcsstr(cls, L"TaskListButtonAutomationPeer")) {
                                    UIAItem item;
                                    item.element = pChild;
                                    pChild->AddRef(); // Keep alive
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
                        
                        Wh_Log(L"Found %d buttons. Task Manager Index: %d", buttons.size(), taskMgrIndex);

                        // Logic to move
                        if (taskMgrIndex != -1 && taskMgrIndex < buttons.size() - 1) {
                            UIAItem& taskMgr = buttons[taskMgrIndex];
                            UIAItem& lastBtn = buttons.back();
                            
                            Wh_Log(L"Preparing to move Task Manager...");
                            Wh_Log(L"  From: %s [%d, %d]", taskMgr.name, taskMgr.rect.left, taskMgr.rect.top);
                            Wh_Log(L"  To After: %s [%d, %d]", lastBtn.name, lastBtn.rect.right, lastBtn.rect.top);

                            // Ask user permission (since we are hijacking the mouse)
                            int result = MessageBoxW(NULL, 
                                L"Windhawk Mod is about to simulate a mouse drag to move Task Manager.\n\n"
                                L"Click OK and DO NOT TOUCH YOUR MOUSE.", 
                                L"Test Hook Explorer", MB_OKCANCEL | MB_ICONWARNING | MB_TOPMOST);
                            
                            if (result == IDOK) {
                                POINT start = { 
                                    (taskMgr.rect.left + taskMgr.rect.right) / 2, 
                                    (taskMgr.rect.top + taskMgr.rect.bottom) / 2 
                                };
                                
                                // Target: Right edge of last button + small offset
                                POINT end = { 
                                    lastBtn.rect.right - 5, // Slightly inside the last button to trigger "insert after" usually
                                    (lastBtn.rect.top + lastBtn.rect.bottom) / 2 
                                };
                                
                                SimulateDrag(start, end);
                            } else {
                                Wh_Log(L"User cancelled drag operation.");
                            }
                        } else {
                            Wh_Log(L"Task Manager not found or already at the end.");
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

    pAutomation->Release();
    CoUninitialize();
    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L"=== Test Hook Explorer Uninit ===");
}
