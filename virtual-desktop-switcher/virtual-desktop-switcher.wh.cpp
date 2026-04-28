// ==WindhawkMod==
// @id              taskbar-vd-switcher
// @name            Taskbar Virtual Desktop Switcher
// @description     Injects clickable buttons into the taskbar — one per virtual desktop — for direct switching. Grid layout adapts to taskbar height.
// @version         1.0
// @author          sb4ssman
// @github          https://github.com/sb4ssman
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lversion -luuid
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Taskbar Virtual Desktop Switcher

Adds numbered buttons to the system tray — one per virtual desktop. Click to switch directly.

![Default taskbar — three numbered buttons, first active](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/virtual-desktop-switcher/vds-screenshot1.png)

Buttons auto-arrange into a grid when the taskbar is tall enough for multiple rows.

![Taller taskbar — auto-rows stacks into a column](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/virtual-desktop-switcher/vds-screenshot2.png)

Works alongside other mods.

![Complex setup with other mods active](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/virtual-desktop-switcher/vds-screenshot3.png)

## Settings
- **Position** — five positions within the system tray
- **Size** — button width × height in pixels; spacing between buttons
- **Rows** — 0 = auto (detected from taskbar height), or a fixed count
- **Active color** — hex color for the current-desktop button (e.g. `#4488FF`)
- **Inactive color** — hex color for other buttons (empty = system default)
- **Opacity** — 0–100; lower values let the taskbar show through
- **Shine effect** — subtle gradient highlight (applies when a color is set)
- **Label format** — numbers, roman numerals, dots, or custom comma-separated labels
- **Padding** — left and right margin around the button grid (px)
- **Text color** — foreground color for active and inactive buttons
- **Font size** — button label size in pt
- **Corner radius** — rounded corners (px)
- **Active bold** — bold the current desktop's label
- **Border** — border color and thickness
- **Hide when single** — hide the bar when only one desktop exists
- **Tooltips** — hover a button to see the desktop name

## Known limitations
- Multi-monitor: only the primary taskbar gets buttons.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- position: "afterClock"
  $name: Position
  $description: Where in the system tray to inject the VD buttons
  $options:
  - "afterClock": "After clock (before Show Desktop)"
  - "beforeClock": "Before clock (after OmniButton)"
  - "beforeOmni": "Before OmniButton (wifi/vol/bat)"
  - "beforeIcons": "Before notification icons"
  - "afterShowDesktop": "After Show Desktop strip"

- buttonWidth: 20
  $name: Button width (px)

- buttonHeight: 22
  $name: Button height (px)

- buttonSpacing: 2
  $name: Button spacing (px)
  $description: Gap between buttons in the grid

- buttonRows: 0
  $name: Button rows (0 = auto from taskbar height)

- activeColor: "#4488FF"
  $name: Active desktop color (hex, empty = system default)

- inactiveColor: ""
  $name: Inactive button color (hex, empty = system default)

- buttonOpacity: 100
  $name: Button opacity (0–100)
  $description: 100 = fully opaque; lower values let the taskbar show through

- shineEffect: false
  $name: Shine effect
  $description: Adds a subtle gradient highlight. Applies when a custom color is set.

- labelFormat: "number"
  $name: Label format
  $options:
  - "number": "Numbers  1  2  3"
  - "roman": "Roman numerals  I  II  III"
  - "dot": "Dots  ●  ○  ○"
  - "custom": "Custom labels"

- customLabels: ""
  $name: Custom labels (comma-separated, e.g. "H,W,M")
  $description: Used when label format is Custom. Falls back to numbers if labels run out.

- activeTextColor: ""
  $name: Active desktop text color (hex, empty = system default)

- inactiveTextColor: ""
  $name: Inactive button text color (hex, empty = system default)

- fontSize: 10
  $name: Font size (pt)

- cornerRadius: 4
  $name: Corner radius (px)
  $description: Rounded corners on buttons (0 = square, 4 = Windows default)

- activeBold: false
  $name: Bold active desktop label

- borderThickness: 0
  $name: Button border thickness (px)

- borderColor: ""
  $name: Button border color (hex, empty = system default)

- hideWhenSingle: false
  $name: Hide when only one desktop
  $description: Don't show the button bar when there is only one virtual desktop

- paddingLeft: 0
  $name: Padding left (px)
  $description: Extra space to the left of the button grid

- paddingRight: 2
  $name: Padding right (px)
  $description: Extra space to the right of the button grid
*/
// ==/WindhawkModSettings==

#undef GetCurrentTime

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>

#include <atomic>
#include <string>
#include <vector>
#include <sstream>
#include <thread>
#include <functional>
#include <algorithm>

#include <windhawk_utils.h>
#include <combaseapi.h>
#include <winver.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

// ============================================================
// Settings
// ============================================================

struct ModSettings {
    std::wstring position      = L"afterClock";
    int buttonWidth            = 20;
    int buttonHeight           = 22;
    int buttonSpacing          = 2;
    int buttonRows             = 0;
    std::wstring activeColor   = L"#4488FF";
    std::wstring inactiveColor = L"";
    int buttonOpacity          = 100;
    bool shineEffect           = false;
    std::wstring labelFormat      = L"number";
    std::wstring customLabels     = L"";
    std::wstring activeTextColor  = L"";
    std::wstring inactiveTextColor= L"";
    int fontSize                  = 10;
    int cornerRadius              = 4;
    bool activeBold               = false;
    int borderThickness           = 0;
    std::wstring borderColor      = L"";
    bool hideWhenSingle           = false;
    int paddingLeft               = 0;
    int paddingRight              = 2;
};
ModSettings g_settings;

static void LoadSettings() {
    auto Str = [](const wchar_t* k, const wchar_t* d) {
        PCWSTR p = Wh_GetStringSetting(k);
        std::wstring r = p ? p : d;
        Wh_FreeStringSetting(p);
        return r;
    };
    g_settings.position       = Str(L"position",      L"afterClock");
    g_settings.buttonWidth    = Wh_GetIntSetting(L"buttonWidth",   20);
    g_settings.buttonHeight   = Wh_GetIntSetting(L"buttonHeight",  22);
    g_settings.buttonSpacing  = Wh_GetIntSetting(L"buttonSpacing", 2);
    g_settings.buttonRows     = Wh_GetIntSetting(L"buttonRows",    0);
    g_settings.activeColor    = Str(L"activeColor",   L"#4488FF");
    g_settings.inactiveColor  = Str(L"inactiveColor", L"");
    g_settings.buttonOpacity  = Wh_GetIntSetting(L"buttonOpacity", 100);
    g_settings.shineEffect    = Wh_GetIntSetting(L"shineEffect",   0) != 0;
    g_settings.labelFormat       = Str(L"labelFormat",       L"number");
    g_settings.customLabels      = Str(L"customLabels",      L"");
    g_settings.activeTextColor   = Str(L"activeTextColor",   L"");
    g_settings.inactiveTextColor = Str(L"inactiveTextColor", L"");
    g_settings.fontSize          = Wh_GetIntSetting(L"fontSize",         10);
    g_settings.cornerRadius      = Wh_GetIntSetting(L"cornerRadius",      4);
    g_settings.activeBold        = Wh_GetIntSetting(L"activeBold",        0) != 0;
    g_settings.borderThickness   = Wh_GetIntSetting(L"borderThickness",   0);
    g_settings.borderColor       = Str(L"borderColor",       L"");
    g_settings.hideWhenSingle    = Wh_GetIntSetting(L"hideWhenSingle",    0) != 0;
    g_settings.paddingLeft       = Wh_GetIntSetting(L"paddingLeft",       0);
    g_settings.paddingRight      = Wh_GetIntSetting(L"paddingRight",      2);
}

// ============================================================
// Globals
// ============================================================

static std::atomic<bool> g_unloading{false};
static HWND              g_taskbarWnd      = nullptr;
static Grid              g_buttonGrid      = nullptr;
static FrameworkElement  g_injectionParent = nullptr;
static int               g_injectedColumn  = -1;
static std::atomic<int>  g_currentDesktop{0};
static std::atomic<int>  g_desktopCount{1};

static HANDLE g_notificationThread    = nullptr;
static HANDLE g_notificationStopEvent = nullptr;
static DWORD  g_notificationCookie    = 0;

static bool g_taskbarViewDllLoaded = false;

// Forward declarations
static void ApplyAllSettings();
static void ApplyAllSettingsOnWindowThread();
static void RebuildOrUpdate(bool fullRebuild);
static void RemoveButtonGrid();
static void StopNotificationThread();

// ============================================================
// Explorer / twinui build detection
// ============================================================

static WORD g_explorerBuild    = 0;
static WORD g_explorerRevision = 0;
static WORD g_twinuiBuild      = 0;

static void DetectExplorerBuild() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    DWORD dummy;
    DWORD sz = GetFileVersionInfoSizeW(path, &dummy);
    if (!sz) return;
    std::vector<BYTE> buf(sz);
    if (!GetFileVersionInfoW(path, 0, sz, buf.data())) return;
    VS_FIXEDFILEINFO* fi = nullptr; UINT fs = 0;
    if (!VerQueryValueW(buf.data(), L"\\", (void**)&fi, &fs)) return;
    g_explorerBuild    = HIWORD(fi->dwFileVersionLS);
    g_explorerRevision = LOWORD(fi->dwFileVersionLS);
    Wh_Log(L"[Init] Explorer build %u rev %u", g_explorerBuild, g_explorerRevision);
}

static bool LoadTwinuiBuild() {
    if (g_twinuiBuild) return true;
    HMODULE h = GetModuleHandleW(L"twinui.pcshell.dll");
    if (!h) return false;
    wchar_t path[MAX_PATH];
    if (!GetModuleFileNameW(h, path, MAX_PATH)) return false;
    DWORD dummy;
    DWORD sz = GetFileVersionInfoSizeW(path, &dummy);
    if (!sz) return false;
    std::vector<BYTE> buf(sz);
    if (!GetFileVersionInfoW(path, 0, sz, buf.data())) return false;
    VS_FIXEDFILEINFO* fi = nullptr; UINT fs = 0;
    if (!VerQueryValueW(buf.data(), L"\\", (void**)&fi, &fs)) return false;
    g_twinuiBuild = HIWORD(fi->dwFileVersionLS);
    Wh_Log(L"[VD] twinui.pcshell.dll build %u", g_twinuiBuild);
    return true;
}

// ============================================================
// GetTaskbarXamlRoot boilerplate (from vertical-omnibutton)
// ============================================================

using RunFromWindowThreadProc_t = void (*)(void*);

static bool RunFromWindowThread(HWND hWnd, RunFromWindowThreadProc_t proc, void* procParam) {
    static const UINT kMsg = RegisterWindowMessage(L"Windhawk_RunFromWindowThread_" WH_MOD_ID);
    struct Param { RunFromWindowThreadProc_t proc; void* procParam; };
    DWORD dwThreadId = GetWindowThreadProcessId(hWnd, nullptr);
    if (!dwThreadId) return false;
    if (dwThreadId == GetCurrentThreadId()) { proc(procParam); return true; }
    HHOOK hook = SetWindowsHookEx(WH_CALLWNDPROC, [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
        if (nCode == HC_ACTION) {
            const CWPSTRUCT* cwp = (const CWPSTRUCT*)lParam;
            if (cwp->message == RegisterWindowMessageW(L"Windhawk_RunFromWindowThread_" WH_MOD_ID)) {
                auto* p = (Param*)cwp->lParam;
                p->proc(p->procParam);
            }
        }
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }, nullptr, dwThreadId);
    if (!hook) return false;
    Param param{ proc, procParam };
    SendMessage(hWnd, kMsg, 0, (LPARAM)&param);
    UnhookWindowsHookEx(hook);
    return true;
}

static HWND FindCurrentProcessTaskbarWnd() {
    HWND result = nullptr;
    EnumWindows([](HWND hWnd, LPARAM lParam) -> BOOL {
        DWORD pid; WCHAR cls[32];
        if (GetWindowThreadProcessId(hWnd, &pid) && pid == GetCurrentProcessId() &&
            GetClassName(hWnd, cls, ARRAYSIZE(cls)) && _wcsicmp(cls, L"Shell_TrayWnd") == 0) {
            *reinterpret_cast<HWND*>(lParam) = hWnd; return FALSE;
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&result));
    return result;
}

using CTaskBand_GetTaskbarHost_t = void* (WINAPI*)(void* pThis, void* taskbarHostSharedPtr);
CTaskBand_GetTaskbarHost_t CTaskBand_GetTaskbarHost_Original;

using TaskbarHost_FrameHeight_t = int (WINAPI*)(void* pThis);
TaskbarHost_FrameHeight_t TaskbarHost_FrameHeight_Original;

using std__Ref_count_base__Decref_t = void (WINAPI*)(void* pThis);
std__Ref_count_base__Decref_t std__Ref_count_base__Decref_Original;

static void* CTaskBand_ITaskListWndSite_vftable = nullptr;

static XamlRoot GetTaskbarXamlRoot(HWND hTaskbarWnd) {
    HWND hTaskSwWnd = (HWND)GetProp(hTaskbarWnd, L"TaskbandHWND");
    if (!hTaskSwWnd) return nullptr;
    void* taskBand = (void*)GetWindowLongPtr(hTaskSwWnd, 0);
    void* taskBandForSite = taskBand;
    for (int i = 0; *(void**)taskBandForSite != CTaskBand_ITaskListWndSite_vftable; i++) {
        if (i == 20) return nullptr;
        taskBandForSite = (void**)taskBandForSite + 1;
    }
    void* taskbarHostSharedPtr[2]{};
    CTaskBand_GetTaskbarHost_Original(taskBandForSite, taskbarHostSharedPtr);
    if (!taskbarHostSharedPtr[0] && !taskbarHostSharedPtr[1]) return nullptr;
    size_t offset = 0x48;
#if defined(_M_X64)
    {
        const BYTE* b = (const BYTE*)TaskbarHost_FrameHeight_Original;
        if (b[0]==0x48 && b[1]==0x83 && b[2]==0xEC && b[4]==0x48 &&
            b[5]==0x83 && b[6]==0xC1 && b[7]<=0x7F)
            offset = b[7];
        else
            Wh_Log(L"Unsupported TaskbarHost::FrameHeight");
    }
#endif
    auto* iunk = *(IUnknown**)((BYTE*)taskbarHostSharedPtr[0] + offset);
    FrameworkElement taskbarElem = nullptr;
    iunk->QueryInterface(winrt::guid_of<FrameworkElement>(), winrt::put_abi(taskbarElem));
    auto result = taskbarElem ? taskbarElem.XamlRoot() : nullptr;
    std__Ref_count_base__Decref_Original(taskbarHostSharedPtr[1]);
    return result;
}

// ============================================================
// XAML helpers
// ============================================================

static FrameworkElement FindChildRecursive(FrameworkElement const& element,
    std::function<bool(FrameworkElement)> const& cb, int maxDepth = 20)
{
    int n = VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < n && maxDepth > 0; i++) {
        auto child = VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();
        if (!child) continue;
        if (cb(child)) return child;
        auto found = FindChildRecursive(child, cb, maxDepth - 1);
        if (found) return found;
    }
    return nullptr;
}

// ============================================================
// VD COM notification infrastructure
// ============================================================

const CLSID CLSID_ImmersiveShell = {
    0xc2f03a33,0x21f5,0x47fa,{0xb4,0xbb,0x15,0x63,0x62,0xa2,0xf2,0x39}
};
const GUID SID_VirtualDesktopNotificationService = {
    0xa501fdec,0x4a09,0x464c,{0xae,0x4e,0x1b,0x9c,0x21,0xb8,0x49,0x18}
};
const GUID IID_IVirtualDesktopNotificationService_G = {
    0x0cd45e71,0xd927,0x4f15,{0x8b,0x0a,0x8f,0xef,0x52,0x53,0x37,0xbf}
};

MIDL_INTERFACE("0CD45E71-D927-4F15-8B0A-8FEF525337BF")
IVirtualDesktopNotificationService_I : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE Register(IUnknown*, DWORD*) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unregister(DWORD) = 0;
};

struct NotifConfig {
    int64_t iidPart1 = 0, iidPart2 = 0;
    int methodCount = 0, createdIdx = -1, destroyedIdx = -1, currentChangedIdx = -1;
    bool hasMonitors = false;
};

struct NotifObject {
    void** vtable  = nullptr;
    LONG refCount  = 1;
};

static NotifConfig GetNotifConfig() {
    if (g_explorerBuild < 22000) return {};
    if (g_explorerBuild < 22483 || (g_explorerBuild == 22621 && g_explorerRevision < 2215))
        return { 5481970284372180562ll, -1679294552252794956ll, 13, 7, 9, 11, true };
    if (g_explorerBuild < 22631 || (g_explorerBuild == 22631 && g_explorerRevision < 3085))
        return { 5123538856297626140ll,  8491238173783613346ll, 14, 6, 8, 10, false };
    return     { 5308375338100058445ll, -2401892766147978065ll, 14, 6, 8, 10, false };
}

static bool IsOurNotifIface(REFIID riid) {
    auto cfg = GetNotifConfig();
    if (!cfg.methodCount) return false;
    auto p = reinterpret_cast<const int64_t*>(&riid);
    return p[0] == cfg.iidPart1 && p[1] == cfg.iidPart2;
}

static HRESULT STDMETHODCALLTYPE Notif_QI(NotifObject* p, REFIID riid, void** ppv) {
    if (!ppv) return E_POINTER; *ppv = nullptr;
    static const GUID IID_IUnknown_ = {0,0,0,{0xc0,0,0,0,0,0,0,0x46}};
    if (InlineIsEqualGUID(riid, IID_IUnknown_) || IsOurNotifIface(riid)) {
        *ppv = p; InterlockedIncrement(&p->refCount); return S_OK;
    }
    return E_NOINTERFACE;
}
static ULONG STDMETHODCALLTYPE Notif_AddRef(NotifObject* p) {
    return (ULONG)InterlockedIncrement(&p->refCount);
}
static ULONG STDMETHODCALLTYPE Notif_Release(NotifObject* p) {
    LONG r = InterlockedDecrement(&p->refCount);
    if (r == 0) { delete[] p->vtable; delete p; }
    return (ULONG)std::max(r, 0L);
}
static HRESULT STDMETHODCALLTYPE Notif_HandleUpdate(bool fullRebuild) {
    if (g_unloading || !g_taskbarWnd) return S_OK;
    RunFromWindowThread(g_taskbarWnd, [](void* p) {
        if (!g_unloading) RebuildOrUpdate((bool)(intptr_t)p);
    }, (void*)fullRebuild);
    return S_OK;
}
static HRESULT STDMETHODCALLTYPE Notif_NoOp() { return S_OK; }
static HRESULT STDMETHODCALLTYPE Notif_CountChanged(NotifObject*) { return Notif_HandleUpdate(true); }
static HRESULT STDMETHODCALLTYPE Notif_CurrentChanged(NotifObject*) { return Notif_HandleUpdate(false); }
static HRESULT STDMETHODCALLTYPE Notif_CurrentChangedWithMonitors(NotifObject*, void*, void*, void*) {
    return Notif_HandleUpdate(false);
}

static NotifObject* CreateNotifObject() {
    auto cfg = GetNotifConfig();
    if (cfg.methodCount == 0 || cfg.currentChangedIdx < 0) return nullptr;
    auto* obj = new (std::nothrow) NotifObject();
    if (!obj) return nullptr;
    obj->vtable = new (std::nothrow) void*[cfg.methodCount];
    if (!obj->vtable) { delete obj; return nullptr; }
    for (int i = 0; i < cfg.methodCount; i++) obj->vtable[i] = (void*)&Notif_NoOp;
    obj->vtable[0] = (void*)&Notif_QI;
    obj->vtable[1] = (void*)&Notif_AddRef;
    obj->vtable[2] = (void*)&Notif_Release;
    if (cfg.createdIdx >= 0)   obj->vtable[cfg.createdIdx]   = (void*)&Notif_CountChanged;
    if (cfg.destroyedIdx >= 0) obj->vtable[cfg.destroyedIdx] = (void*)&Notif_CountChanged;
    obj->vtable[cfg.currentChangedIdx] = cfg.hasMonitors
        ? (void*)&Notif_CurrentChangedWithMonitors
        : (void*)&Notif_CurrentChanged;
    return obj;
}

static NotifObject* g_notifObject = nullptr;

static DWORD WINAPI NotificationThreadProc(void*) {
    auto cfg = GetNotifConfig();
    if (cfg.methodCount == 0) {
        Wh_Log(L"[Notif] Unsupported build (explorer %u)", g_explorerBuild);
        return 0;
    }
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) return 0;

    IServiceProvider* svc = nullptr;
    if (FAILED(CoCreateInstance(CLSID_ImmersiveShell, nullptr, CLSCTX_LOCAL_SERVER,
                                IID_IServiceProvider, (void**)&svc)) || !svc) {
        CoUninitialize(); return 0;
    }
    IVirtualDesktopNotificationService_I* notifSvc = nullptr;
    svc->QueryService(SID_VirtualDesktopNotificationService,
                      IID_IVirtualDesktopNotificationService_G, (void**)&notifSvc);
    svc->Release();
    if (!notifSvc) { CoUninitialize(); return 0; }

    g_notifObject = CreateNotifObject();
    if (!g_notifObject) { notifSvc->Release(); CoUninitialize(); return 0; }

    HRESULT hr = notifSvc->Register(reinterpret_cast<IUnknown*>(g_notifObject), &g_notificationCookie);
    if (FAILED(hr)) {
        Wh_Log(L"[Notif] Register failed: 0x%08X", hr);
        Notif_Release(g_notifObject); g_notifObject = nullptr;
        notifSvc->Release(); CoUninitialize(); return 0;
    }
    Wh_Log(L"[Notif] Registered, cookie=%lu", g_notificationCookie);

    MSG msg;
    while (!g_unloading) {
        DWORD w = MsgWaitForMultipleObjects(1, &g_notificationStopEvent, FALSE, INFINITE, QS_ALLINPUT);
        if (w == WAIT_OBJECT_0) break;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
    }

    if (g_notificationCookie) { notifSvc->Unregister(g_notificationCookie); g_notificationCookie = 0; }
    if (g_notifObject)        { Notif_Release(g_notifObject); g_notifObject = nullptr; }
    notifSvc->Release();
    CoUninitialize();
    return 0;
}

static void StartNotificationThread() {
    if (g_notificationThread) return;
    g_notificationStopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_notificationThread    = CreateThread(nullptr, 0, NotificationThreadProc, nullptr, 0, nullptr);
    if (!g_notificationThread) {
        CloseHandle(g_notificationStopEvent); g_notificationStopEvent = nullptr;
    }
}

static void StopNotificationThread() {
    if (g_notificationStopEvent) SetEvent(g_notificationStopEvent);
    if (g_notificationThread) {
        WaitForSingleObject(g_notificationThread, 3000);
        CloseHandle(g_notificationThread); g_notificationThread = nullptr;
    }
    if (g_notificationStopEvent) {
        CloseHandle(g_notificationStopEvent); g_notificationStopEvent = nullptr;
    }
}

// ============================================================
// Desktop state — registry
// ============================================================

static std::vector<BYTE> ReadRegBinary(const wchar_t* path, const wchar_t* name) {
    DWORD type = 0, size = 0;
    if (RegGetValueW(HKEY_CURRENT_USER, path, name, RRF_RT_REG_BINARY, &type, nullptr, &size) != ERROR_SUCCESS || !size)
        return {};
    std::vector<BYTE> buf(size);
    if (RegGetValueW(HKEY_CURRENT_USER, path, name, RRF_RT_REG_BINARY, &type, buf.data(), &size) != ERROR_SUCCESS)
        return {};
    buf.resize(size);
    return buf;
}

static int ReadDesktopCount() {
    DWORD sessionId = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &sessionId);
    wchar_t sessionPath[256];
    swprintf_s(sessionPath, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SessionInfo\\%lu\\VirtualDesktops", sessionId);
    for (auto* path : { (const wchar_t*)sessionPath, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops" }) {
        auto buf = ReadRegBinary(path, L"VirtualDesktopIDs");
        if (buf.size() >= 16) return (int)(buf.size() / 16);
    }
    return 1;
}

static int ReadCurrentDesktop() {
    DWORD sessionId = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &sessionId);
    wchar_t sessionPath[256];
    swprintf_s(sessionPath, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SessionInfo\\%lu\\VirtualDesktops", sessionId);

    std::vector<BYTE> ids;
    for (auto* path : { (const wchar_t*)sessionPath, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops" }) {
        ids = ReadRegBinary(path, L"VirtualDesktopIDs");
        if (ids.size() >= 16) break;
    }
    if (ids.empty()) return 0;

    GUID currentGuid{};
    bool gotCurrent = false;
    for (auto* path : { (const wchar_t*)sessionPath, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops" }) {
        auto buf = ReadRegBinary(path, L"CurrentVirtualDesktop");
        if (buf.size() >= 16) { memcpy(&currentGuid, buf.data(), 16); gotCurrent = true; break; }
        // Try REG_SZ form
        wchar_t strBuf[64]; DWORD sz = sizeof(strBuf), type;
        if (RegGetValueW(HKEY_CURRENT_USER, path, L"CurrentVirtualDesktop",
                         RRF_RT_REG_SZ, &type, strBuf, &sz) == ERROR_SUCCESS &&
            SUCCEEDED(CLSIDFromString(strBuf, &currentGuid))) { gotCurrent = true; break; }
    }
    if (!gotCurrent) return 0;

    int count = (int)(ids.size() / 16);
    for (int i = 0; i < count; i++) {
        GUID g; memcpy(&g, ids.data() + i * 16, 16);
        if (memcmp(&g, &currentGuid, 16) == 0) return i;
    }
    return 0;
}

// Read Windows display names for all desktops (registry Desktops\{GUID}\Name).
// Falls back to "Desktop N" when a desktop has no custom name.
static std::vector<std::wstring> ReadDesktopNames(int count) {
    DWORD sessionId = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &sessionId);
    wchar_t sessionPath[256];
    swprintf_s(sessionPath, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SessionInfo\\%lu\\VirtualDesktops", sessionId);

    std::vector<BYTE> ids;
    for (auto* path : { (const wchar_t*)sessionPath, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops" }) {
        ids = ReadRegBinary(path, L"VirtualDesktopIDs");
        if (ids.size() >= 16) break;
    }

    std::vector<std::wstring> names(count);
    for (int i = 0; i < count; i++) {
        names[i] = L"Desktop " + std::to_wstring(i + 1);
        if ((int)ids.size() >= (i + 1) * 16) {
            GUID g; memcpy(&g, ids.data() + i * 16, 16);
            wchar_t guidStr[64];
            StringFromGUID2(g, guidStr, ARRAYSIZE(guidStr));
            wchar_t regPath[300];
            swprintf_s(regPath, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops\\Desktops\\%ls", guidStr);
            wchar_t name[256]; DWORD sz = sizeof(name);
            if (RegGetValueW(HKEY_CURRENT_USER, regPath, L"Name", RRF_RT_REG_SZ, nullptr, name, &sz) == ERROR_SUCCESS && name[0])
                names[i] = name;
        }
    }
    return names;
}

// ============================================================
// Virtual desktop switching
// ============================================================

struct IVirtualDesktopManagerInternal_S : IUnknown {};
struct IVirtualDesktop_S : IUnknown {};

MIDL_INTERFACE("92CA9DCD-5622-4bba-A805-5E9F541BD8C9")
IObjectArray_Local : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetCount(UINT* pcObjects) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAt(UINT i, REFIID riid, void** ppv) = 0;
};

const CLSID CLSID_VirtualDesktopManagerInternal = {
    0xC5E0CDCA,0x7B6E,0x41B2,{0x9F,0xC4,0xD9,0x39,0x75,0xCC,0x46,0x7B}
};

void SwitchToDesktop(int targetIndex) {
    if (!LoadTwinuiBuild()) { Wh_Log(L"[VD] twinui.pcshell.dll not loaded"); return; }

    IID IID_VDMI, IID_VD;
    bool usesHMonitor;
    if      (g_twinuiBuild >= 26100) {
        IID_VDMI = {0x53F5CA0B,0x158F,0x4124,{0x90,0x0C,0x05,0x71,0x58,0x06,0x0B,0x27}};
        IID_VD   = {0x3F07F4BE,0xB107,0x441A,{0xAF,0x0F,0x39,0xD8,0x25,0x29,0x07,0x2C}};
        usesHMonitor = false;
    } else if (g_twinuiBuild >= 22621) {
        IID_VDMI = {0xA3175F2D,0x239C,0x4BD2,{0x8A,0xA0,0xEE,0xBA,0x8B,0x0B,0x13,0x8E}};
        IID_VD   = {0x3F07F4BE,0xB107,0x441A,{0xAF,0x0F,0x39,0xD8,0x25,0x29,0x07,0x2C}};
        usesHMonitor = false;
    } else if (g_twinuiBuild >= 22000) {
        IID_VDMI = {0xB2F925B9,0x5A0F,0x4D2E,{0x9F,0x4D,0x2B,0x15,0x07,0x59,0x3C,0x10}};
        IID_VD   = {0x536D3495,0xB208,0x4CC9,{0xAE,0x26,0xDE,0x81,0x11,0x27,0x5B,0xF8}};
        usesHMonitor = true;
    } else if (g_twinuiBuild >= 20348) {
        IID_VDMI = {0x094AFE11,0x44F2,0x4BA0,{0x97,0x6F,0x29,0xA9,0x7E,0x26,0x3E,0xE0}};
        IID_VD   = {0x62FDF88B,0x11CA,0x4AFB,{0x8B,0xD8,0x22,0x96,0xDF,0xAE,0x49,0xE2}};
        usesHMonitor = true;
    } else {
        IID_VDMI = {0xF31574D6,0xB682,0x4CDC,{0xBD,0x56,0x18,0x27,0x86,0x0A,0xBE,0xC6}};
        IID_VD   = {0xFF72FFDD,0xBE7E,0x43FC,{0x9C,0x03,0xAD,0x81,0x68,0x1E,0x88,0xE4}};
        usesHMonitor = false;
    }

    IServiceProvider* svc = nullptr;
    if (FAILED(CoCreateInstance(CLSID_ImmersiveShell, nullptr, CLSCTX_LOCAL_SERVER,
                                IID_IServiceProvider, (void**)&svc)) || !svc)
        { Wh_Log(L"[VD] CoCreateInstance failed"); return; }

    IVirtualDesktopManagerInternal_S* mgr = nullptr;
    svc->QueryService(CLSID_VirtualDesktopManagerInternal, IID_VDMI, (void**)&mgr);
    svc->Release();
    if (!mgr) { Wh_Log(L"[VD] QueryService VDMI failed"); return; }

    IObjectArray_Local* arr = nullptr;
    if (usesHMonitor) {
        typedef HRESULT(STDMETHODCALLTYPE* FnM)(IVirtualDesktopManagerInternal_S*, HMONITOR, IObjectArray_Local**);
        ((FnM)(*(void***)mgr)[7])(mgr, nullptr, &arr);
    } else {
        typedef HRESULT(STDMETHODCALLTYPE* Fn)(IVirtualDesktopManagerInternal_S*, IObjectArray_Local**);
        ((Fn)(*(void***)mgr)[7])(mgr, &arr);
    }
    if (!arr) { mgr->Release(); Wh_Log(L"[VD] GetDesktops failed"); return; }

    UINT count = 0;
    arr->GetCount(&count);
    if (targetIndex < 0 || (UINT)targetIndex >= count) { arr->Release(); mgr->Release(); return; }

    IVirtualDesktop_S* target = nullptr;
    arr->GetAt((UINT)targetIndex, IID_VD, (void**)&target);
    arr->Release();
    if (!target) { mgr->Release(); Wh_Log(L"[VD] GetAt failed"); return; }

    if (usesHMonitor) {
        typedef HRESULT(STDMETHODCALLTYPE* FnM)(IVirtualDesktopManagerInternal_S*, HMONITOR, IVirtualDesktop_S*);
        ((FnM)(*(void***)mgr)[9])(mgr, nullptr, target);
    } else {
        typedef HRESULT(STDMETHODCALLTYPE* Fn)(IVirtualDesktopManagerInternal_S*, IVirtualDesktop_S*);
        ((Fn)(*(void***)mgr)[9])(mgr, target);
    }
    target->Release();
    mgr->Release();
    Wh_Log(L"[VD] Switched to desktop %d", targetIndex);
}

// ============================================================
// Button grid building
// ============================================================

static Brush ParseColorBrush(const std::wstring& hex) {
    if (hex.empty() || hex[0] != L'#') return nullptr;
    std::wstring h = hex.substr(1);
    if (h.size() == 6) h = L"FF" + h;
    if (h.size() != 8) return nullptr;
    UINT32 val = 0;
    for (wchar_t c : h) {
        val <<= 4;
        if      (c >= L'0' && c <= L'9') val |= (UINT32)(c - L'0');
        else if (c >= L'A' && c <= L'F') val |= (UINT32)(10 + c - L'A');
        else if (c >= L'a' && c <= L'f') val |= (UINT32)(10 + c - L'a');
        else return nullptr;
    }
    winrt::Windows::UI::Color color;
    color.A = (BYTE)(val >> 24); color.R = (BYTE)(val >> 16);
    color.G = (BYTE)(val >> 8);  color.B = (BYTE)(val);
    SolidColorBrush brush; brush.Color(color); return brush;
}

static std::wstring ToRoman(int n) {
    if (n <= 0 || n > 3999) return std::to_wstring(n);
    static const struct { int v; const wchar_t* s; } t[] = {
        {1000,L"M"},{900,L"CM"},{500,L"D"},{400,L"CD"},
        {100,L"C"},{90,L"XC"},{50,L"L"},{40,L"XL"},
        {10,L"X"},{9,L"IX"},{5,L"V"},{4,L"IV"},{1,L"I"}
    };
    std::wstring r;
    for (auto& [v, s] : t) { while (n >= v) { r += s; n -= v; } }
    return r;
}

static std::wstring GetButtonLabel(int idx, int current) {
    if (g_settings.labelFormat == L"dot")
        return (idx == current) ? L"●" : L"○";
    if (g_settings.labelFormat == L"roman")
        return ToRoman(idx + 1);
    if (g_settings.labelFormat == L"custom" && !g_settings.customLabels.empty()) {
        std::wistringstream ss(g_settings.customLabels);
        std::wstring token; int i = 0;
        while (std::getline(ss, token, L',')) { if (i++ == idx) return token; }
    }
    return std::to_wstring(idx + 1);
}

// Apply shine gradient to a base brush. Returns brush unchanged if no base color or shine off.
static Brush MakeShineBrush(Brush base) {
    if (!g_settings.shineEffect) return base;
    auto solid = base ? base.try_as<SolidColorBrush>() : nullptr;
    if (!solid) return base;
    auto c = solid.Color();

    LinearGradientBrush b;
    b.StartPoint({0.5, 0.0});
    b.EndPoint({0.5, 1.0});

    // Top: semi-transparent white highlight
    GradientStop g0; winrt::Windows::UI::Color shine{180,255,255,255};
    g0.Color(shine); g0.Offset(0.0); b.GradientStops().Append(g0);

    // Upper-mid: base color lightened slightly
    GradientStop g1;
    winrt::Windows::UI::Color light{c.A,
        (BYTE)std::min(255, (int)c.R + 35),
        (BYTE)std::min(255, (int)c.G + 35),
        (BYTE)std::min(255, (int)c.B + 35)};
    g1.Color(light); g1.Offset(0.42); b.GradientStops().Append(g1);

    // Lower: base color
    GradientStop g2; g2.Color(c); g2.Offset(0.52); b.GradientStops().Append(g2);

    // Bottom: slightly darker
    GradientStop g3;
    winrt::Windows::UI::Color dark{c.A,
        (BYTE)(c.R * 7 / 10), (BYTE)(c.G * 7 / 10), (BYTE)(c.B * 7 / 10)};
    g3.Color(dark); g3.Offset(1.0); b.GradientStops().Append(g3);

    return b;
}

// Compute how many button rows fit in the taskbar.
// Column-major fill: desktop 1 = top-left, 2 = bottom-left (if 2 rows), 3 = top-right, etc.
static int ComputeRows(int count) {
    if (g_settings.buttonRows > 0) return std::min(g_settings.buttonRows, count);
    RECT r{};
    if (g_taskbarWnd && GetWindowRect(g_taskbarWnd, &r)) {
        int tbH = r.bottom - r.top;
        int rows = std::max(1, tbH / (g_settings.buttonHeight + 6));        return std::min(rows, count);
    }
    return 1;
}

static Grid BuildButtonGrid(int count, int current) {
    int rows = ComputeRows(count);
    int cols = (count + rows - 1) / rows;

    Grid grid;
    grid.Name(L"VdSwitcherBar");
    grid.VerticalAlignment(VerticalAlignment::Center);
    if (g_settings.buttonSpacing > 0) {
        grid.ColumnSpacing((double)g_settings.buttonSpacing);
        grid.RowSpacing((double)g_settings.buttonSpacing);
    }
    if (g_settings.buttonOpacity < 100)
        grid.Opacity(std::max(0.0, std::min(1.0, g_settings.buttonOpacity / 100.0)));
    if (g_settings.paddingLeft > 0 || g_settings.paddingRight > 0)
        grid.Margin({ (double)g_settings.paddingLeft, 0.0, (double)g_settings.paddingRight, 0.0 });

    for (int r = 0; r < rows; r++) {
        RowDefinition rd;
        rd.Height({ (double)g_settings.buttonHeight, GridUnitType::Pixel });
        grid.RowDefinitions().Append(rd);
    }
    for (int c = 0; c < cols; c++) {
        ColumnDefinition cd;
        cd.Width({ (double)g_settings.buttonWidth, GridUnitType::Pixel });
        grid.ColumnDefinitions().Append(cd);
    }

    auto activeBrush       = MakeShineBrush(ParseColorBrush(g_settings.activeColor));
    auto inactiveBrush     = MakeShineBrush(ParseColorBrush(g_settings.inactiveColor));
    auto activeTextBrush   = ParseColorBrush(g_settings.activeTextColor);
    auto inactiveTextBrush = ParseColorBrush(g_settings.inactiveTextColor);
    auto borderBrush       = ParseColorBrush(g_settings.borderColor);
    auto desktopNames      = ReadDesktopNames(count);

    for (int i = 0; i < count; i++) {
        int col = i / rows;   // column-major: desktops fill top-to-bottom per column
        int row = i % rows;
        bool isActive = (i == current);

        Button btn;
        btn.Content(winrt::box_value(GetButtonLabel(i, current)));
        btn.Padding({ 1.0, 0.0, 1.0, 0.0 });
        btn.FontSize((double)g_settings.fontSize);
        btn.HorizontalAlignment(HorizontalAlignment::Stretch);
        btn.VerticalAlignment(VerticalAlignment::Stretch);

        if (isActive && activeBrush)
            btn.Background(activeBrush);
        else if (!isActive && inactiveBrush)
            btn.Background(inactiveBrush);

        if (isActive && activeTextBrush)
            btn.Foreground(activeTextBrush);
        else if (!isActive && inactiveTextBrush)
            btn.Foreground(inactiveTextBrush);

        if (g_settings.activeBold)
            btn.FontWeight(isActive
                ? winrt::Windows::UI::Text::FontWeights::Bold()
                : winrt::Windows::UI::Text::FontWeights::Normal());

        {
            double r = (double)g_settings.cornerRadius;
            btn.CornerRadius({ r, r, r, r });
        }

        if (borderBrush) {
            btn.BorderBrush(borderBrush);
            if (g_settings.borderThickness > 0) {
                double t = (double)g_settings.borderThickness;
                btn.BorderThickness({ t, t, t, t });
            }
        }

        ToolTipService::SetToolTip(btn, winrt::box_value(winrt::hstring(desktopNames[i])));

        int capturedIdx = i;
        btn.Click([capturedIdx](auto const&, auto const&) {
            SwitchToDesktop(capturedIdx);
        });

        Grid::SetRow(btn, row);
        Grid::SetColumn(btn, col);
        grid.Children().Append(btn);
    }
    return grid;
}

// Update button highlights and labels in-place (no rebuild).
static void UpdateHighlights(int current) {
    if (!g_buttonGrid) return;
    auto activeBrush       = MakeShineBrush(ParseColorBrush(g_settings.activeColor));
    auto inactiveBrush     = MakeShineBrush(ParseColorBrush(g_settings.inactiveColor));
    auto activeTextBrush   = ParseColorBrush(g_settings.activeTextColor);
    auto inactiveTextBrush = ParseColorBrush(g_settings.inactiveTextColor);
    int n = (int)g_buttonGrid.Children().Size();
    for (int i = 0; i < n; i++) {
        auto btn = g_buttonGrid.Children().GetAt(i).try_as<Button>();
        if (!btn) continue;
        bool isActive = (i == current);
        if (g_settings.labelFormat == L"dot")
            btn.Content(winrt::box_value(std::wstring(isActive ? L"●" : L"○")));
        Brush bg = isActive ? activeBrush : inactiveBrush;
        btn.Background(bg ? bg : nullptr);
        if (activeTextBrush || inactiveTextBrush) {
            Brush fg = isActive ? activeTextBrush : inactiveTextBrush;
            if (fg)
                btn.Foreground(fg);
            else
                btn.ClearValue(Control::ForegroundProperty());
        }
        if (g_settings.activeBold)
            btn.FontWeight(isActive
                ? winrt::Windows::UI::Text::FontWeights::Bold()
                : winrt::Windows::UI::Text::FontWeights::Normal());
    }
}

// ============================================================
// Injection into XAML tree
// ============================================================

static bool InjectButtonGrid(FrameworkElement root) {
    const auto& pos = g_settings.position;

    FrameworkElement parent = FindChildRecursive(root, [](FrameworkElement fe) {
        return fe.Name() == L"SystemTrayFrameGrid";
    });
    if (!parent) {
        Wh_Log(L"[Inject] SystemTrayFrameGrid not found");
        return false;
    }

    // SystemTrayFrameGrid is a Grid with column-based layout. We must insert a new
    // ColumnDefinition and shift existing elements rather than relying on Children order.
    auto gridParent = parent.try_as<Grid>();
    if (!gridParent) { Wh_Log(L"[Inject] Parent is not a Grid"); return false; }

    // Already injected?
    for (auto child : gridParent.Children()) {
        if (auto fe = child.try_as<FrameworkElement>(); fe && fe.Name() == L"VdSwitcherBar")
            return true;
    }

    int count   = ReadDesktopCount();
    int current = ReadCurrentDesktop();
    g_desktopCount.store(count);
    g_currentDesktop.store(current);

    if (g_settings.hideWhenSingle && count <= 1) {
        Wh_Log(L"[Inject] Skipping — hideWhenSingle, count=%d", count);
        return true;  // notification thread will watch for desktop additions
    }

    auto grid = BuildButtonGrid(count, current);

    // Find a named direct child of the tray grid.
    auto findNamedDirect = [&](const wchar_t* name) -> FrameworkElement {
        for (auto child : gridParent.Children()) {
            if (auto fe = child.try_as<FrameworkElement>(); fe && fe.Name() == name)
                return fe;
        }
        return nullptr;
    };

    // Map position setting → reference element + whether to insert after it.
    FrameworkElement refElem = nullptr;
    bool insertAfterRef = false;

    if      (pos == L"beforeOmni")
        refElem = findNamedDirect(L"ControlCenterButton");
    else if (pos == L"beforeClock")
        refElem = findNamedDirect(L"NotificationCenterButton");
    else if (pos == L"afterClock")
        refElem = findNamedDirect(L"ShowDesktopStack");
    else if (pos == L"afterShowDesktop") {
        refElem = findNamedDirect(L"ShowDesktopStack");
        insertAfterRef = true;
    }
    // beforeIcons → column 0 (refElem stays nullptr)

    int insertCol;
    if (insertAfterRef && refElem)
        insertCol = Grid::GetColumn(refElem) + 1;
    else if (refElem)
        insertCol = Grid::GetColumn(refElem);
    else
        insertCol = 0;  // beforeIcons: leftmost column in tray

    // Insert a new Auto-width column at insertCol.
    ColumnDefinition cd;
    cd.Width({ 1.0, GridUnitType::Auto });
    if ((uint32_t)insertCol < gridParent.ColumnDefinitions().Size())
        gridParent.ColumnDefinitions().InsertAt((uint32_t)insertCol, cd);
    else
        gridParent.ColumnDefinitions().Append(cd);

    // Shift every existing child whose column is >= insertCol to make room.
    for (auto child : gridParent.Children()) {
        auto fe = child.try_as<FrameworkElement>();
        if (!fe) continue;
        int col = Grid::GetColumn(fe);
        if (col >= insertCol)
            Grid::SetColumn(fe, col + 1);
    }

    Grid::SetColumn(grid, insertCol);
    gridParent.Children().Append(grid);
    g_buttonGrid      = grid;
    g_injectionParent = parent;
    g_injectedColumn  = insertCol;

    Wh_Log(L"[Inject] VdSwitcherBar at column=%d in %ls (%d desktops, current=%d)",
           insertCol, parent.Name().c_str(), count, current);
    return true;
}

static void RemoveButtonGrid() {
    if (!g_buttonGrid) return;
    auto gridParent = g_injectionParent ? g_injectionParent.try_as<Grid>() : nullptr;
    if (gridParent) {
        uint32_t idx;
        if (gridParent.Children().IndexOf(g_buttonGrid, idx))
            gridParent.Children().RemoveAt(idx);

        if (g_injectedColumn >= 0) {
            uint32_t col = (uint32_t)g_injectedColumn;
            if (col < gridParent.ColumnDefinitions().Size())
                gridParent.ColumnDefinitions().RemoveAt(col);
            for (auto child : gridParent.Children()) {
                auto fe = child.try_as<FrameworkElement>();
                if (!fe) continue;
                int c = Grid::GetColumn(fe);
                if (c > g_injectedColumn)
                    Grid::SetColumn(fe, c - 1);
            }
        }
    }
    g_buttonGrid      = nullptr;
    g_injectionParent = nullptr;
    g_injectedColumn  = -1;
}

// Rebuild button grid (full or highlight-only) on the UI thread.
static void RebuildOrUpdate(bool fullRebuild) {
    int count   = ReadDesktopCount();
    int current = ReadCurrentDesktop();
    bool countChanged = (count != g_desktopCount.load());
    g_desktopCount.store(count);
    g_currentDesktop.store(current);

    if (g_settings.hideWhenSingle) {
        if (count <= 1) {
            if (g_buttonGrid) RemoveButtonGrid();
            return;
        }
        if (!g_buttonGrid) {
            ApplyAllSettings();
            return;
        }
    }

    if (fullRebuild || countChanged) {
        if (!g_buttonGrid || !g_injectionParent) return;
        auto gridParent = g_injectionParent.try_as<Grid>();
        if (!gridParent) return;
        uint32_t idx;
        if (!gridParent.Children().IndexOf(g_buttonGrid, idx)) return;
        gridParent.Children().RemoveAt(idx);
        g_buttonGrid = BuildButtonGrid(count, current);
        if (g_injectedColumn >= 0)
            Grid::SetColumn(g_buttonGrid, g_injectedColumn);
        gridParent.Children().InsertAt(idx, g_buttonGrid);
    } else {
        UpdateHighlights(current);
    }
}

// ============================================================
// Apply / cleanup
// ============================================================

static void ApplyAllSettings() {
    HWND hWnd = FindCurrentProcessTaskbarWnd();
    if (!hWnd) { Wh_Log(L"[Apply] No taskbar window"); return; }
    g_taskbarWnd = hWnd;

    auto xamlRoot = GetTaskbarXamlRoot(hWnd);
    if (!xamlRoot) { Wh_Log(L"[Apply] GetTaskbarXamlRoot failed"); return; }
    auto root = xamlRoot.Content().try_as<FrameworkElement>();
    if (!root) { Wh_Log(L"[Apply] No XAML root content"); return; }

    if (InjectButtonGrid(root))
        StartNotificationThread();
    else
        Wh_Log(L"[Apply] Injection failed");
}

static void ApplyAllSettingsOnWindowThread() {
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return;
    RunFromWindowThread(hWnd, [](void*) { ApplyAllSettings(); }, nullptr);
}

// ============================================================
// Hooks
// ============================================================

using IconView_IconView_t = void* (WINAPI*)(void* pThis);
IconView_IconView_t IconView_IconView_Original;

void* WINAPI IconView_IconView_Hook(void* pThis) {
    auto result = IconView_IconView_Original(pThis);
    if (!g_unloading && !g_buttonGrid)
        ApplyAllSettingsOnWindowThread();
    return result;
}

using LoadLibraryExW_t = HMODULE (WINAPI*)(LPCWSTR, HANDLE, DWORD);
LoadLibraryExW_t LoadLibraryExW_Original;

HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR path, HANDLE file, DWORD flags) {
    HMODULE h = LoadLibraryExW_Original(path, file, flags);
    if (h && path && !g_taskbarViewDllLoaded) {
        const wchar_t* base = wcsrchr(path, L'\\');
        base = base ? base + 1 : path;
        if (_wcsicmp(base, L"Taskbar.View.dll") == 0) {
            // Taskbar.View.dll
            WindhawkUtils::SYMBOL_HOOK taskbarViewHooks[] = {{
                {LR"(public: __cdecl winrt::SystemTray::implementation::IconView::IconView(void))"},
                &IconView_IconView_Original, IconView_IconView_Hook,
            }};
            if (WindhawkUtils::HookSymbols(h, taskbarViewHooks, ARRAYSIZE(taskbarViewHooks)))
                g_taskbarViewDllLoaded = true;
        }
    }
    return h;
}

// ============================================================
// Symbol hook setup
// ============================================================

static bool HookTaskbarDllSymbols() {
    HMODULE h = LoadLibraryExW(L"taskbar.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!h) return false;
    WindhawkUtils::SYMBOL_HOOK taskbarDllHooks[] = {
        { {LR"(const CTaskBand::`vftable'{for `ITaskListWndSite'})"},
          &CTaskBand_ITaskListWndSite_vftable },
        { {LR"(public: virtual class std::shared_ptr<class TaskbarHost> __cdecl CTaskBand::GetTaskbarHost(void)const )"},
          &CTaskBand_GetTaskbarHost_Original },
        { {LR"(public: int __cdecl TaskbarHost::FrameHeight(void)const )"},
          &TaskbarHost_FrameHeight_Original },
        { {LR"(public: void __cdecl std::_Ref_count_base::_Decref(void))"},
          &std__Ref_count_base__Decref_Original },
    };
    return WindhawkUtils::HookSymbols(h, taskbarDllHooks, ARRAYSIZE(taskbarDllHooks));
}

static bool HookTaskbarViewDllSymbols(HMODULE h) {
    // Taskbar.View.dll
    WindhawkUtils::SYMBOL_HOOK taskbarViewHooks[] = {{
        {LR"(public: __cdecl winrt::SystemTray::implementation::IconView::IconView(void))"},
        &IconView_IconView_Original, IconView_IconView_Hook,
    }};
    if (!WindhawkUtils::HookSymbols(h, taskbarViewHooks, ARRAYSIZE(taskbarViewHooks))) return false;
    g_taskbarViewDllLoaded = true;
    return true;
}

static HMODULE GetTaskbarViewModule() {
    for (auto* name : { L"Taskbar.View.dll", L"taskbar.view.dll" }) {
        if (HMODULE h = GetModuleHandleW(name)) return h;
    }
    return nullptr;
}

// ============================================================
// Windhawk lifecycle
// ============================================================

BOOL Wh_ModInit() {
    Wh_Log(L"[Init] VD Switcher v1.0");
    LoadSettings();
    DetectExplorerBuild();

    if (!HookTaskbarDllSymbols())
        Wh_Log(L"[Init] taskbar.dll hooks failed — GetTaskbarXamlRoot unavailable");

    if (HMODULE h = GetTaskbarViewModule()) {
        if (!HookTaskbarViewDllSymbols(h))
            Wh_Log(L"[Init] Taskbar.View.dll hooks failed");
    } else {
        HMODULE kb = GetModuleHandleW(L"kernelbase.dll");
        auto pLLEW = kb ? (LoadLibraryExW_t)GetProcAddress(kb, "LoadLibraryExW") : nullptr;
        if (pLLEW)
            Wh_SetFunctionHook((void*)pLLEW, (void*)LoadLibraryExW_Hook, (void**)&LoadLibraryExW_Original);
    }
    return TRUE;
}

void Wh_ModAfterInit() {
    if (!g_taskbarViewDllLoaded) {
        if (HMODULE h = GetTaskbarViewModule())
            HookTaskbarViewDllSymbols(h);
    }
    if (g_taskbarViewDllLoaded)
        ApplyAllSettingsOnWindowThread();

    std::thread([]() {
        for (int i = 0; i < 5 && !g_unloading; i++) {
            Sleep(2000);
            if (g_buttonGrid) break;
            Wh_Log(L"[AfterInit] Retry %d", i + 1);
            ApplyAllSettingsOnWindowThread();
        }
    }).detach();
}

static void DoUninitRemove(FrameworkElement const& parent, Grid const& grid, int col) {
    auto gp = parent ? parent.try_as<Grid>() : nullptr;
    if (!gp || !grid) return;
    uint32_t idx;
    if (gp.Children().IndexOf(grid, idx))
        gp.Children().RemoveAt(idx);
    if (col >= 0) {
        uint32_t colU = (uint32_t)col;
        if (colU < gp.ColumnDefinitions().Size())
            gp.ColumnDefinitions().RemoveAt(colU);
        for (auto child : gp.Children()) {
            auto fe = child.try_as<FrameworkElement>();
            if (!fe) continue;
            int c = Grid::GetColumn(fe);
            if (c > col) Grid::SetColumn(fe, c - 1);
        }
    }
}

void Wh_ModUninit() {
    g_unloading = true;
    Wh_Log(L"[Uninit]");

    StopNotificationThread();

    auto grid   = g_buttonGrid;
    auto parent = g_injectionParent;
    int  col    = g_injectedColumn;
    g_buttonGrid      = nullptr;
    g_injectionParent = nullptr;
    g_injectedColumn  = -1;

    if (!grid) return;

    // Attempt sync removal (may or may not be on UI thread).
    DoUninitRemove(parent, grid, col);

    // Async backup to ensure removal on UI thread.
    HMODULE hSelf = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)&Wh_ModUninit, &hSelf);
    try {
        auto disp = grid.Dispatcher();
        if (disp) {
            auto _ = disp.RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
                [grid, parent, col, hSelf]() {
                    DoUninitRemove(parent, grid, col);
                    if (hSelf) FreeLibrary(hSelf);
                });
            return;
        }
    } catch (...) {}
    if (hSelf) FreeLibrary(hSelf);
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Wh_Log(L"[Settings] Changed");

    // Signal notification thread to stop; it will be restarted by ApplyAllSettings.
    if (g_notificationStopEvent) SetEvent(g_notificationStopEvent);

    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return;

    RunFromWindowThread(hWnd, [](void*) {
        if (g_notificationThread) {
            WaitForSingleObject(g_notificationThread, 1000);
            CloseHandle(g_notificationThread); g_notificationThread = nullptr;
        }
        if (g_notificationStopEvent) {
            CloseHandle(g_notificationStopEvent); g_notificationStopEvent = nullptr;
        }
        RemoveButtonGrid();
        ApplyAllSettings();
    }, nullptr);
}
