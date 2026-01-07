// ==WindhawkMod==
// @id              test-hook-explorer
// @name            Test Hook Explorer
// @description     Multi-Target UIA Scan of XAML Islands
// @version         4.2
// @author          sb4ssman
// @include         explorer.exe
// @compilerOptions -luser32 -loleacc -loleaut32 -luuid -lole32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Test Hook Explorer v4.2 (Multi-Target UIA)

This version scans ALL windows named "DesktopWindowXamlSource" inside the taskbar.
It attempts to find "Task Manager" using UIA Descendant search on each of them.

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

void LogElement(IUIAutomationElement* pElement, int depth) {
    BSTR name = NULL;
    pElement->get_CurrentName(&name);
    
    BSTR className = NULL;
    pElement->get_CurrentClassName(&className);
    
    CONTROLTYPEID typeId;
    pElement->get_CurrentControlType(&typeId);
    
    wchar_t indent[64] = {0};
    for(int i=0; i<depth*2 && i<60; i++) indent[i] = L' ';
    
    Wh_Log(L"%s[UIA] Name: '%s', Class: '%s', Type: %d", indent, name ? name : L"(null)", className ? className : L"(null)", typeId);
    
    if (name) SysFreeString(name);
    if (className) SysFreeString(className);
}

BOOL Wh_ModInit() {
    Wh_Log(L"=== Test Hook Explorer v4.2 (Multi-Target UIA) ===");
    
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
            
            // Check if it looks like a XAML source
            if (wcsstr(text, L"DesktopWindowXamlSource")) {
                Wh_Log(L"Checking XAML Window: %p (Class: %s)", hChild, className);
                
                IUIAutomationElement* pRoot = NULL;
                hr = pAutomation->ElementFromHandle(hChild, &pRoot);
                if (SUCCEEDED(hr) && pRoot) {
                    // 1. Count children
                    IUIAutomationCondition* pTrueCondition = NULL;
                    pAutomation->CreateTrueCondition(&pTrueCondition);
                    IUIAutomationElementArray* pChildren = NULL;
                    pRoot->FindAll(TreeScope_Children, pTrueCondition, &pChildren);
                    int count = 0;
                    if (pChildren) {
                        pChildren->get_Length(&count);
                        pChildren->Release();
                    }
                    pTrueCondition->Release();
                    
                    Wh_Log(L"  -> Connected. UIA Children count: %d", count);

                    // 2. Search for Task Manager in Descendants
                    Wh_Log(L"  -> Searching for 'Task Manager' in descendants...");
                    
                    VARIANT varName;
                    varName.vt = VT_BSTR;
                    varName.bstrVal = SysAllocString(L"Task Manager");
                    
                    IUIAutomationCondition* pNameCondition = NULL;
                    pAutomation->CreatePropertyCondition(UIA_NamePropertyId, varName, &pNameCondition);
                    
                    if (pNameCondition) {
                        IUIAutomationElement* pTaskMgr = NULL;
                        pRoot->FindFirst(TreeScope_Descendants, pNameCondition, &pTaskMgr);
                        
                        if (pTaskMgr) {
                            Wh_Log(L"!!! FOUND TASK MANAGER !!!");
                            LogElement(pTaskMgr, 1);
                            
                            // Try to find a parent that supports Drag/Drop or is a list item
                            IUIAutomationTreeWalker* pWalker = NULL;
                            pAutomation->get_ControlViewWalker(&pWalker);
                            if (pWalker) {
                                IUIAutomationElement* pParent = NULL;
                                pWalker->GetParentElement(pTaskMgr, &pParent);
                                if (pParent) {
                                    Wh_Log(L"    Parent:");
                                    LogElement(pParent, 2);
                                    pParent->Release();
                                }
                                pWalker->Release();
                            }
                            pTaskMgr->Release();
                        } else {
                            Wh_Log(L"  -> Not found in this window.");
                        }
                        pNameCondition->Release();
                    }
                    SysFreeString(varName.bstrVal);
                    pRoot->Release();
                } else {
                    Wh_Log(L"  -> Failed to connect UIA: 0x%08X", (unsigned int)hr);
                }
            }
            
            hChild = GetWindow(hChild, GW_HWNDNEXT);
        }
    } else {
        Wh_Log(L"Shell_TrayWnd not found.");
    }

    pAutomation->Release();
    CoUninitialize();
    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L"=== Test Hook Explorer Uninit ===");
}
