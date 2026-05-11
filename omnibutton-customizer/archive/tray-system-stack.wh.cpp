// ==WindhawkMod==
// @id              tray-system-stack
// @name            Tray System Icons — Stack (Probe & First Attempt)
// @description     Probe Shell_NotifyIconGetRect calls, log identifiers, and attempt to stack system icons vertically.
// @version         0.1
// @author          You
// @include         explorer.exe
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
This mod hooks Shell_NotifyIconGetRect to discover how explorer exposes tray icons.
It logs NOTIFYICONIDENTIFIER contents and attempts a simple vertical stack for
icons that appear to belong to the system tray (heuristic: identifier->hWnd is
owned by the taskbar or identifier->guidItem is zero and caller requests rects
inside explorer).

Run this mod and interact with the system icons (click wifi/volume/battery)
and paste the Windhawk debug logs. That will let us refine an accurate,
reliable hook that modifies icon positions at the correct callsite.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
{
  "pollIntervalMs": { "type": "int", "default": 1000 },
  "iconSize":        { "type": "int", "default": 32 },
  "iconSpacing":     { "type": "int", "default": 4 },
  "debugLogging":    { "type": "bool", "default": true }
}
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <shellapi.h>
#include <vector>
#include <string>
#include <mutex>

#include <windhawk_api.h>

// Storage for original function pointer
using Shell_NotifyIconGetRect_t = HRESULT(WINAPI*)(const NOTIFYICONIDENTIFIER*, RECT*);
static Shell_NotifyIconGetRect_t g_Shell_NotifyIconGetRect_Original = nullptr;

// Settings
static int g_iconSize = 32;
static int g_iconSpacing = 4;
static bool g_debugLogging = true;

// Small per-frame collection: store observed calls that likely belong to the system tray
struct IconCall {
    NOTIFYICONIDENTIFIER id;
    RECT rect;          // original rect returned by original function (if any)
    bool rectSet;       // whether original returned a rect
    bool markedSystem;  // heuristic that this is a system icon
};
static std::vector<IconCall> g_frameCalls;
static std::mutex g_frameMutex;

// Helpers
static std::wstring GuidToString(const GUID& g) {
    wchar_t buf[64];
    if (StringFromGUID2(g, buf, _countof(buf))) return std::wstring(buf);
    return L"{}";
}

static std::wstring HwndToClassName(HWND hwnd) {
    wchar_t buf[256] = {0};
    if (hwnd && GetClassNameW(hwnd, buf, _countof(buf))) return std::wstring(buf);
    return L"";
}

static bool IsTaskbarWindow(HWND hwnd) {
    if (!hwnd) return false;
    // Check ancestor chain for Shell_TrayWnd
    HWND cur = hwnd;
    while (cur) {
        wchar_t cls[64] = {0};
        GetClassNameW(cur, cls, _countof(cls));
        if (_wcsicmp(cls, L"Shell_TrayWnd") == 0) return true;
        cur = GetParent(cur);
    }
    return false;
}

// Compute stacked rects (1 column grid) centered where original group was
static void ComputeAndAssignStackedRects(std::vector<IconCall*>& calls) {
    if (calls.empty()) return;

    // Compute bounding box of original rects (fallback if none available: use taskbar rect)
    RECT bbox = {INT_MAX, INT_MAX, INT_MIN, INT_MIN};
    bool haveAny = false;
    for (auto c : calls) {
        if (c->rectSet) {
            haveAny = true;
            bbox.left = min(bbox.left, c->rect.left);
            bbox.top = min(bbox.top, c->rect.top);
            bbox.right = max(bbox.right, c->rect.right);
            bbox.bottom = max(bbox.bottom, c->rect.bottom);
        }
    }

    if (!haveAny) {
        // fallback: use Shell_TrayWnd rect
        HWND shell = FindWindowW(L"Shell_TrayWnd", nullptr);
        if (shell) GetWindowRect(shell, &bbox);
        else {
            bbox.left = 0; bbox.top = 0; bbox.right = bbox.left + 200; bbox.bottom = bbox.top + 200;
        }
    }

    int totalH = (int)calls.size() * g_iconSize + (int)calls.size() * g_iconSpacing - g_iconSpacing;
    int startY = bbox.top + ((bbox.bottom - bbox.top) / 2) - (totalH / 2);
    int centerX = bbox.left + ((bbox.right - bbox.left) / 2);

    for (size_t i = 0; i < calls.size(); ++i) {
        int x = centerX - (g_iconSize / 2);
        int y = startY + (int)i * (g_iconSize + g_iconSpacing);
        RECT r = { x, y, x + g_iconSize, y + g_iconSize };
        calls[i]->rect = r;
        calls[i]->rectSet = true;
        // We will overwrite the rect returned to callers later
    }
}

// Our hook for Shell_NotifyIconGetRect
HRESULT WINAPI Shell_NotifyIconGetRect_Hook(const NOTIFYICONIDENTIFIER* lpniid, RECT* lprcIcon) {
    // Call original first (to populate a default rect)
    HRESULT hr = S_OK;
    if (g_Shell_NotifyIconGetRect_Original) {
        hr = g_Shell_NotifyIconGetRect_Original(lpniid, lprcIcon);
    }

    // Defensive checks
    if (!lpniid) return hr;

    // Build a logged copy
    IconCall call = {};
    call.id = *lpniid;
    call.rectSet = false;
    call.markedSystem = false;
    if (lprcIcon && SUCCEEDED(hr)) {
        call.rect = *lprcIcon;
        call.rectSet = true;
    }

    // Heuristic: if identifier's hWnd is taskbar or a child of taskbar, mark as system
    HWND idHwnd = lpniid->hWnd;
    if (IsTaskbarWindow(idHwnd)) call.markedSystem = true;

    // Another heuristic: if GUID is zero (no guidItem) and hwnd==NULL or belongs to shell, treat as system
    GUID emptyGuid = {};
    if (lpniid->guidItem.Data1 == 0 && (idHwnd == NULL || IsTaskbarWindow(idHwnd))) {
        call.markedSystem = true;
    }

    // Store call
    {
        std::lock_guard<std::mutex> lock(g_frameMutex);
        g_frameCalls.push_back(call);
    }

    // Logging
    if (g_debugLogging) {
        std::wstring guidStr = GuidToString(lpniid->guidItem);
        std::wstring cls = HwndToClassName(idHwnd);
        Wh_Log(L"[Shell_NotifyIconGetRect_Hook] called: hWnd=%p class=%s uID=%u guid=%s rectSet=%d",
               idHwnd, cls.c_str(), lpniid->uID, guidStr.c_str(), call.rectSet ? 1 : 0);
    }

    // After collecting a small batch (synchronous heuristic), attempt to arrange system-marked calls
    bool doArrange = false;
    std::vector<IconCall*> toArrange;
    {
        std::lock_guard<std::mutex> lock(g_frameMutex);
        // If we've seen >= 3 calls OR the last N ms elapsed, attempt grouping
        if (g_frameCalls.size() >= 3) doArrange = true;

        if (doArrange) {
            // Build pointers for system-marked calls only
            for (auto &c : g_frameCalls) {
                if (c.markedSystem) toArrange.push_back(&c);
            }
            // Clear frame storage (we will reapply changes by directly writing to callers below)
            g_frameCalls.clear();
        }
    }

    if (doArrange && !toArrange.empty()) {
        // Compute stacked rects and then write them back to the system via a best-effort approach.
        ComputeAndAssignStackedRects(toArrange);

        // For each arranged call, attempt to apply the stacked RECT to the callers.
        // We cannot directly set the rect in other call contexts; but we can handle the case
        // where the current caller corresponds to one of these items and override its rect.
        // To cover other simultaneous callers, we rely on subsequent calls to this hook to return updated rects.
        // Here, if the current identifier matches an arranged entry, override the supplied output rect.
        if (lprcIcon && lpniid) {
            // Search in toArrange for a matching identifier
            for (auto c : toArrange) {
                bool match = false;
                // match by hWnd & uID if available
                if (lpniid->hWnd && (c->id.hWnd == lpniid->hWnd) && (c->id.uID == lpniid->uID)) match = true;
                // match by GUID if present
                if (!match && (memcmp(&c->id.guidItem, &lpniid->guidItem, sizeof(GUID)) == 0)) match = true;
                if (match) {
                    *lprcIcon = c->rect;
                    if (g_debugLogging) {
                        Wh_Log(L"[Shell_NotifyIconGetRect_Hook] Overriding rect for caller hWnd=%p uID=%u -> (%d,%d)-(%d,%d)",
                               lpniid->hWnd, lpniid->uID, c->rect.left, c->rect.top, c->rect.right, c->rect.bottom);
                    }
                    // done
                    break;
                }
            }
        }
    }

    return hr;
}

// Install the hook
BOOL InstallShellNotifyIconGetRectHook() {
    HMODULE hShell = GetModuleHandleW(L"shell32.dll");
    if (!hShell) return FALSE;
    FARPROC addr = GetProcAddress(hShell, "Shell_NotifyIconGetRect");
    if (!addr) {
        Wh_Log(L"[InstallHook] Shell_NotifyIconGetRect not found in shell32.dll");
        return FALSE;
    }

    // Save original and install
    g_Shell_NotifyIconGetRect_Original = (Shell_NotifyIconGetRect_t)addr;
    // Use Wh_SetFunctionHook to detour
    Wh_SetFunctionHook((void*)addr, (void*)Shell_NotifyIconGetRect_Hook, (void**)&g_Shell_NotifyIconGetRect_Original);
    Wh_Log(L"[InstallHook] Hooked Shell_NotifyIconGetRect at %p", addr);
    return TRUE;
}

void RemoveShellNotifyIconGetRectHook() {
    if (g_Shell_NotifyIconGetRect_Original) {
        // Unhook by setting original pointer back via Wh_SetFunctionHook with original? Windhawk unhooking occurs on unload.
        // Nothing special required here.
    }
}

// Windhawk callbacks
BOOL Wh_ModInit() {
    Wh_Log(L"[tray-system-stack] Init");
    g_iconSize = Wh_GetIntSetting(L"iconSize");
    if (g_iconSize <= 0) g_iconSize = 32;
    g_iconSpacing = Wh_GetIntSetting(L"iconSpacing");
    g_debugLogging = Wh_GetIntSetting(L"debugLogging");

    if (!InstallShellNotifyIconGetRectHook()) {
        Wh_Log(L"[tray-system-stack] Failed to install Shell_NotifyIconGetRect hook");
    }
    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L"[tray-system-stack] Uninit");
    g_frameCalls.clear();
    RemoveShellNotifyIconGetRectHook();
}

void Wh_ModSettingsChanged() {
    Wh_Log(L"[tray-system-stack] SettingsChanged");
    g_iconSize = Wh_GetIntSetting(L"iconSize");
    g_iconSpacing = Wh_GetIntSetting(L"iconSpacing");
    g_debugLogging = Wh_GetIntSetting(L"debugLogging");
}
