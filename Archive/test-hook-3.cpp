// ==WindhawkMod==
// @id              test-hook-explorer
// @name            Test Hook Explorer
// @description     Scans Accessibility Tree to find Task Manager coordinates
// @version         3.0
// @author          sb4ssman
// @include         explorer.exe
// @compilerOptions -luser32 -loleacc -loleaut32 -luuid
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Test Hook Explorer v3.0 (Accessibility Scan)

Since the legacy toolbar messages failed, this version uses the **Accessibility API**
to look inside the modern XAML taskbar.

It attempts to find the "Task Manager" button and report its screen coordinates.

**INSTRUCTIONS:**
1. Compile and Enable.
2. **Ensure Task Manager is open.**
3. Check Logs.
*/
// ==/WindhawkModReadme==

#include <windows.h>
#include <oleacc.h>
#include <wchar.h>

// Recursive search for "Task Manager" in the Accessibility Tree
bool FindTaskManagerInAccTree(IAccessible* pAcc, int depth) {
    // Safety limit
    if (depth > 15 || !pAcc) return false;

    long childCount = 0;
    if (FAILED(pAcc->get_accChildCount(&childCount))) return false;
    
    // Don't scan massive trees (optimization)
    if (childCount > 50) childCount = 50;

    VARIANT* pChildren = new VARIANT[childCount];
    LONG obtained = 0;
    bool found = false;

    if (AccessibleChildren(pAcc, 0, childCount, pChildren, &obtained) == S_OK) {
        for (int i = 0; i < obtained; i++) {
            if (found) break; // Stop if already found in a sibling branch

            VARIANT& v = pChildren[i];
            
            // 1. Check Name of current element
            BSTR bstrName = NULL;
            if (SUCCEEDED(pAcc->get_accName(v, &bstrName)) && bstrName) {
                // Log for debugging (first few levels only)
                if (depth < 2) {
                    Wh_Log(L"  [ACC Depth %d] Found: %s", depth, bstrName);
                }

                if (wcsstr(bstrName, L"Task Manager")) {
                    Wh_Log(L"!!! FOUND TASK MANAGER via ACCESSIBILITY !!!");
                    
                    // Get Location
                    long x, y, w, h;
                    if (SUCCEEDED(pAcc->accLocation(&x, &y, &w, &h, v))) {
                        Wh_Log(L"    Coordinates: X=%ld, Y=%ld, Width=%ld, Height=%ld", x, y, w, h);
                    }
                    found = true;
                }
                SysFreeString(bstrName);
            }
            
            // 2. Recurse if it's an object (and we haven't found it yet)
            if (!found && v.vt == VT_DISPATCH && v.pdispVal) {
                IAccessible* pChild = NULL;
                v.pdispVal->QueryInterface(IID_IAccessible, (void**)&pChild);
                if (pChild) {
                    if (FindTaskManagerInAccTree(pChild, depth + 1)) {
                        found = true;
                    }
                    pChild->Release();
                }
            }
            VariantClear(&v);
        }
    }
    delete[] pChildren;
    return found;
}

BOOL Wh_ModInit() {
    Wh_Log(L"=== Test Hook Explorer v3.0 (Accessibility) ===");
    
    // Start scan from the Main Taskbar Window
    HWND mainTray = FindWindowW(L"Shell_TrayWnd", NULL);
    if (mainTray) {
        Wh_Log(L"Scanning Accessibility Tree for 'Task Manager'...");
        
        IAccessible* pAcc = NULL;
        if (S_OK == AccessibleObjectFromWindow(mainTray, OBJID_CLIENT, IID_IAccessible, (void**)&pAcc)) {
            if (!FindTaskManagerInAccTree(pAcc, 0)) {
                Wh_Log(L"Scanned tree but did not find 'Task Manager'. Is it running?");
            }
            pAcc->Release();
        } else {
            Wh_Log(L"Failed to get IAccessible from Shell_TrayWnd");
        }
    } else {
        Wh_Log(L"ERROR: Shell_TrayWnd not found");
    }

    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L"=== Test Hook Explorer Uninit ===");
}