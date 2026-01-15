// ==WindhawkMod==
// @id              test-hook-explorer
// @name            Test Hook Explorer
// @description     UIA Pattern Check for Task Manager
// @version         5.0
// @author          sb4ssman
// @include         explorer.exe
// @compilerOptions -luser32 -loleacc -loleaut32 -luuid -lole32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Test Hook Explorer v5.0 (Pattern Check)

We found the Task Manager button! Now we need to see if it supports any
UIA patterns that allow us to move it (like Drag or Transform).

**INSTRUCTIONS:**
1. Compile and Enable.
2. **Ensure Task Manager is open.**
3. Check Logs.
*/
// ==/WindhawkModReadme==

#include <windows.h>
#include <objbase.h>
#include <uiautomation.h>
#include <stdio.h>

void CheckPatterns(IUIAutomationElement* pElement) {
    IUnknown* pPattern = NULL;
    HRESULT hr;

    // Check Drag Pattern (ID: 10030)
    hr = pElement->GetCurrentPattern(10030, &pPattern);
    Wh_Log(L"    Supports DragPattern: %s", (SUCCEEDED(hr) && pPattern) ? L"YES" : L"NO");
    if (pPattern) pPattern->Release();

    // Check Transform Pattern (ID: 10016)
    hr = pElement->GetCurrentPattern(10016, &pPattern);
    Wh_Log(L"    Supports TransformPattern: %s", (SUCCEEDED(hr) && pPattern) ? L"YES" : L"NO");
    if (pPattern) pPattern->Release();
    
    // Check Invoke Pattern (ID: 10000)
    hr = pElement->GetCurrentPattern(10000, &pPattern);
    Wh_Log(L"    Supports InvokePattern: %s", (SUCCEEDED(hr) && pPattern) ? L"YES" : L"NO");
    if (pPattern) pPattern->Release();
}

BOOL Wh_ModInit() {
    Wh_Log(L"=== Test Hook Explorer v5.0 (Pattern Check) ===");
    
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
            
            // Target the window we found in the logs
            if (wcsstr(text, L"DesktopWindowXamlSource")) {
                IUIAutomationElement* pRoot = NULL;
                hr = pAutomation->ElementFromHandle(hChild, &pRoot);
                if (SUCCEEDED(hr) && pRoot) {
                    // Search for Task Manager in Descendants
                    IUIAutomationCondition* pTrueCondition = NULL;
                    pAutomation->CreateTrueCondition(&pTrueCondition);
                    
                    IUIAutomationElementArray* pChildren = NULL;
                    pRoot->FindAll(TreeScope_Descendants, pTrueCondition, &pChildren);
                    
                    if (pChildren) {
                        int count = 0;
                        pChildren->get_Length(&count);
                        
                        for (int i = 0; i < count; i++) {
                            IUIAutomationElement* pChild = NULL;
                            pChildren->GetElement(i, &pChild);
                            if (pChild) {
                                BSTR name = NULL;
                                pChild->get_CurrentName(&name);
                                
                                if (name && wcsstr(name, L"Task Manager")) {
                                    Wh_Log(L"!!! FOUND TASK MANAGER BUTTON !!!");
                                    Wh_Log(L"    Name: %s", name);
                                    
                                    // Get Rect
                                    RECT rect;
                                    if (SUCCEEDED(pChild->get_CurrentBoundingRectangle(&rect))) {
                                        Wh_Log(L"    Rect: [%d, %d, %d, %d]", rect.left, rect.top, rect.right, rect.bottom);
                                    }

                                    // Check Patterns
                                    CheckPatterns(pChild);
                                }
                                if (name) SysFreeString(name);
                                pChild->Release();
                            }
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
