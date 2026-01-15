// ==WindhawkMod==
// @id              test-hook-explorer
// @name            Test Hook Explorer
// @description     UIA Tree Dump of XAML Islands
// @version         4.3
// @author          sb4ssman
// @include         explorer.exe
// @compilerOptions -luser32 -loleacc -loleaut32 -luuid -lole32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Test Hook Explorer v4.3 (UIA Tree Dump)

This version dumps the UIA children of the XAML windows found in the taskbar.
This helps us identify the structure and correct names of the elements.

**INSTRUCTIONS:**
1. Compile and Enable.
2. Check Logs.
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

void DumpChildren(IUIAutomation* pAutomation, IUIAutomationElement* pRoot, int depth) {
    if (depth > 3) return; // Limit depth to avoid massive logs

    IUIAutomationCondition* pTrueCondition = NULL;
    pAutomation->CreateTrueCondition(&pTrueCondition);
    
    IUIAutomationElementArray* pChildren = NULL;
    pRoot->FindAll(TreeScope_Children, pTrueCondition, &pChildren);
    
    if (pChildren) {
        int count = 0;
        pChildren->get_Length(&count);
        
        for (int i = 0; i < count; i++) {
            IUIAutomationElement* pChild = NULL;
            pChildren->GetElement(i, &pChild);
            if (pChild) {
                LogElement(pChild, depth);
                
                // Recurse to see structure
                DumpChildren(pAutomation, pChild, depth + 1);
                
                pChild->Release();
            }
        }
        pChildren->Release();
    }
    pTrueCondition->Release();
}

BOOL Wh_ModInit() {
    Wh_Log(L"=== Test Hook Explorer v4.3 (UIA Tree Dump) ===");
    
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
                Wh_Log(L"Checking XAML Window: %p (Class: %s)", hChild, className);
                
                IUIAutomationElement* pRoot = NULL;
                hr = pAutomation->ElementFromHandle(hChild, &pRoot);
                if (SUCCEEDED(hr) && pRoot) {
                    Wh_Log(L"  -> Connected. Dumping UIA Tree:");
                    DumpChildren(pAutomation, pRoot, 1);
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
