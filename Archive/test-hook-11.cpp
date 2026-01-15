// ==WindhawkMod==
// @id              test-hook-explorer
// @name            Test Hook Explorer
// @description     Touch Injection Probe (v11.0)
// @version         11.0
// @author          sb4ssman
// @include         explorer.exe
// @compilerOptions -luser32 -loleacc -loleaut32 -luuid -lole32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Test Hook Explorer v11.0 (Touch Injection)

This version attempts to move the Task Manager button using **Touch Injection**.
It simulates a finger touching the screen and dragging the button.

**Why?**
- Touch input does NOT move the mouse cursor.
- The Taskbar is designed for touch (tablets).
- It might bypass the limitations of legacy mouse messages.

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
bool g_touchInitialized = false;

// Helper to check if user is interacting with mouse/keyboard
bool IsUserIdle() {
    LASTINPUTINFO lii = { sizeof(LASTINPUTINFO) };
    if (GetLastInputInfo(&lii)) {
        DWORD tick = GetTickCount();
        if (tick - lii.dwTime < 2000) return false;
    }
    return true;
}

// Helper to simulate TOUCH drag
void TouchDrag(POINT start, POINT end) {
    Wh_Log(L"Executing Touch Drag: (%d, %d) -> (%d, %d)", start.x, start.y, end.x, end.y);

    if (!g_touchInitialized) {
        if (!InitializeTouchInjection(1, TOUCH_FEEDBACK_DEFAULT)) {
            Wh_Log(L"Failed to initialize touch injection: %d", GetLastError());
            return;
        }
        g_touchInitialized = true;
    }

    POINTER_TOUCH_INFO contact = {0};
    contact.pointerInfo.pointerType = PT_TOUCH;
    contact.pointerInfo.pointerId = 1; // Arbitrary ID
    contact.pointerInfo.ptPixelLocation = start;
    contact.pointerInfo.pointerFlags = POINTER_FLAG_DOWN | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
    contact.touchFlags = TOUCH_FLAG_NONE;
    contact.touchMask = TOUCH_MASK_CONTACTAREA | TOUCH_MASK_ORIENTATION | TOUCH_MASK_PRESSURE;
    contact.rcContact.left = start.x - 2;
    contact.rcContact.right = start.x + 2;
    contact.rcContact.top = start.y - 2;
    contact.rcContact.bottom = start.y + 2;
    contact.orientation = 90;
    contact.pressure = 1024;

    // 1. Touch Down
    if (!InjectTouchInput(1, &contact)) {
        Wh_Log(L"InjectTouchInput (Down) failed: %d", GetLastError());
        return;
    }
    Sleep(100); // Hold

    // 2. Drag
    contact.pointerInfo.pointerFlags = POINTER_FLAG_UPDATE | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
    
    int steps = 20;
    for(int i=1; i<=steps; i++) {
        int x = start.x + (end.x - start.x) * i / steps;
        int y = start.y + (end.y - start.y) * i / steps;
        
        contact.pointerInfo.ptPixelLocation.x = x;
        contact.pointerInfo.ptPixelLocation.y = y;
        contact.rcContact.left = x - 2;
        contact.rcContact.right = x + 2;
        contact.rcContact.top = y - 2;
        contact.rcContact.bottom = y + 2;

        InjectTouchInput(1, &contact);
        Sleep(10);
    }

    // 3. Hold at destination
    Sleep(200);

    // 4. Touch Up
    contact.pointerInfo.pointerFlags = POINTER_FLAG_UP;
    InjectTouchInput(1, &contact);
    
    Wh_Log(L"Touch Drag Complete.");
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
                            UIAItem& taskMgr = buttons[taskMgrIndex];
                            UIAItem& lastBtn = buttons.back();
                            
                            Wh_Log(L"Task Manager found at index %d (of %d). Attempting Touch Drag...", taskMgrIndex, buttons.size());

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
                            
                            TouchDrag(start, end);
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
    Wh_Log(L"=== Test Hook Explorer v11.0 (Touch Injection) ===");
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
