// ==WindhawkMod==
// @id              test-hook-explorer
// @name            Test Hook Explorer
// @description     Deep Capability Scanner (v10.0)
// @version         10.0
// @author          sb4ssman
// @include         explorer.exe
// @compilerOptions -luser32 -loleacc -loleaut32 -luuid -lole32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Test Hook Explorer v10.0 (Deep Capability Scan)

This version does NOT move anything. It performs a deep scan of the Taskbar's
internal structure to find a "silent" way to move buttons.

It checks for:
1. All UIA Patterns on the Task Manager button.
2. All UIA Patterns on the **Parent List** (the container holding the buttons).

**INSTRUCTIONS:**
1. Compile and Enable.
2. Open Task Manager.
3. Check the Logs tab.
*/
// ==/WindhawkModReadme==

#include <windows.h>
#include <objbase.h>
#include <uiautomation.h>
#include <stdio.h>
#include <vector>

// Global thread control
HANDLE g_hThread = NULL;
volatile bool g_stopThread = false;
bool g_scanned = false;

struct PatternDef {
    int id;
    const wchar_t* name;
};

// List of standard UIA Patterns to check
PatternDef g_patterns[] = {
    {10000, L"Invoke"},
    {10001, L"Selection"},
    {10002, L"Value"},
    {10003, L"RangeValue"},
    {10004, L"Scroll"},
    {10005, L"ExpandCollapse"},
    {10006, L"Grid"},
    {10007, L"GridItem"},
    {10008, L"MultipleView"},
    {10009, L"Window"},
    {10010, L"SelectionItem"},
    {10011, L"Dock"},
    {10012, L"Table"},
    {10013, L"TableItem"},
    {10014, L"Text"},
    {10015, L"Toggle"},
    {10016, L"Transform"}, // <--- Critical: Allows Move()
    {10017, L"ScrollItem"},
    {10018, L"LegacyIAccessible"}, // <--- Critical: Exposes MSAA
    {10019, L"ItemContainer"}, // <--- Critical: Finding items
    {10020, L"VirtualizedItem"},
    {10021, L"SynchronizedInput"},
    {10030, L"Drag"},
    {10031, L"DropTarget"}
};

void LogPatterns(IUIAutomationElement* pElement, const wchar_t* context) {
    Wh_Log(L"--- Scanning Patterns for: %s ---", context);
    
    for (const auto& pat : g_patterns) {
        IUnknown* pUnk = NULL;
        HRESULT hr = pElement->GetCurrentPattern(pat.id, &pUnk);
        if (SUCCEEDED(hr) && pUnk) {
            Wh_Log(L"  [SUPPORTED] %s (%d)", pat.name, pat.id);
            pUnk->Release();
        }
    }
}

void ScanTaskbar(IUIAutomation* pAutomation) {
    if (g_scanned) return;

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
                    
                    for (int i = 0; i < count; i++) {
                        IUIAutomationElement* pChild = NULL;
                        pChildren->GetElement(i, &pChild);
                        if (pChild) {
                            BSTR name = NULL;
                            pChild->get_CurrentName(&name);
                            
                            if (name && wcsstr(name, L"Task Manager")) {
                                Wh_Log(L"=== FOUND TASK MANAGER BUTTON ===");
                                LogPatterns(pChild, L"Task Manager Button");

                                // Now get the PARENT (The List)
                                IUIAutomationTreeWalker* pWalker = NULL;
                                pAutomation->get_ControlViewWalker(&pWalker);
                                if (pWalker) {
                                    IUIAutomationElement* pParent = NULL;
                                    pWalker->GetParentElement(pChild, &pParent);
                                    if (pParent) {
                                        BSTR pName = NULL;
                                        BSTR pClass = NULL;
                                        pParent->get_CurrentName(&pName);
                                        pParent->get_CurrentClassName(&pClass);
                                        
                                        wchar_t parentInfo[256];
                                        swprintf_s(parentInfo, L"Parent Container (Name: %s, Class: %s)", 
                                            pName ? pName : L"null", pClass ? pClass : L"null");
                                        
                                        LogPatterns(pParent, parentInfo);
                                        
                                        if (pName) SysFreeString(pName);
                                        if (pClass) SysFreeString(pClass);
                                        pParent->Release();
                                    }
                                    pWalker->Release();
                                }
                                
                                g_scanned = true; // Stop scanning after finding it
                            }
                            if (name) SysFreeString(name);
                            pChild->Release();
                        }
                        if (g_scanned) break;
                    }
                    pChildren->Release();
                }
                pTrueCondition->Release();
                pRoot->Release();
            }
        }
        if (g_scanned) break;
        hChild = GetWindow(hChild, GW_HWNDNEXT);
    }
}

DWORD WINAPI BackgroundThread(LPVOID) {
    Wh_Log(L"Scanner Thread Started");
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) return 1;

    IUIAutomation* pAutomation = NULL;
    hr = CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&pAutomation);
    
    if (SUCCEEDED(hr) && pAutomation) {
        while (!g_stopThread && !g_scanned) {
            ScanTaskbar(pAutomation);
            Sleep(1000);
        }
        pAutomation->Release();
    }
    CoUninitialize();
    Wh_Log(L"Scanner Thread Stopped");
    return 0;
}

BOOL Wh_ModInit() {
    Wh_Log(L"=== Test Hook Explorer v10.0 (Deep Capability Scan) ===");
    g_stopThread = false;
    g_scanned = false;
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
