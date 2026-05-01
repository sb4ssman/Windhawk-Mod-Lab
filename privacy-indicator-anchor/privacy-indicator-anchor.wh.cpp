// ==WindhawkMod==
// @id              tray-privacy-indicator-anchor
// @name            Tray Privacy Indicator Anchor
// @description     Keeps the system tray privacy indicator (location/microphone) permanently visible — dim when idle, full brightness when in use — preventing taskbar layout shifts.
// @version         0.1
// @author          sb4ssman
// @github          https://github.com/sb4ssman
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lversion
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Tray Privacy Indicator Anchor

Windows 11 shows a location/microphone icon in the system tray when an app
accesses those privacy features, then hides it again — causing nearby icons to
shift position. On a centered taskbar this can be visibly distracting,
especially with apps that access location frequently (such as the Windows Web
Experience Pack / Widgets).

This mod keeps the privacy indicator permanently present in the tray:

- **Dim** when idle — no app is using location or microphone
- **Full brightness** when in use — location or microphone is active

No more sudden shifts. You can still tell at a glance when location is active.

## Settings
- **Idle opacity** — how opaque the icon is when nothing is using it (0–100).
  `0` = invisible but space reserved (no layout shifts); `100` = always full brightness.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- idleOpacity: 20
  $name: Idle opacity (0-100)
  $description: >-
    Opacity when no app is using location or microphone.
    0 = invisible but space reserved (no layout shift); 100 = always full brightness.
*/
// ==/WindhawkModSettings==

#undef GetCurrentTime

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>

#include <algorithm>
#include <atomic>
#include <functional>
#include <list>
#include <string>
#include <thread>
#include <vector>

#include <windhawk_utils.h>
#include <winver.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

// ============================================================
// Settings
// ============================================================

struct ModSettings {
    int idleOpacity = 20;
};
static ModSettings g_settings;

static void LoadSettings() {
    int v = Wh_GetIntSetting(L"idleOpacity", 20);
    g_settings.idleOpacity = std::max(0, std::min(100, v));
}

// ============================================================
// Globals
// ============================================================

static std::atomic<bool> g_unloading{false};
static HWND              g_taskbarWnd           = nullptr;
static bool              g_taskbarViewDllLoaded = false;

struct PrivacyState {
    winrt::weak_ref<FrameworkElement> iconViewRef;
    winrt::weak_ref<TextBlock>        textBlockRef;
    int64_t textToken       = 0;
    int64_t visibilityToken = 0;
};
static std::vector<PrivacyState> g_privacyStates;

using FrameworkElementLoadedRevoker = winrt::impl::event_revoker<
    IFrameworkElement,
    &winrt::impl::abi<IFrameworkElement>::type::remove_Loaded>;
static std::list<FrameworkElementLoadedRevoker> g_loadedRevokers;

// Forward declarations
static void ApplyStyle();
static void ApplyStyleOnWindowThread();
static void ClearPrivacyStates();

// ============================================================
// GetTaskbarXamlRoot — same boilerplate as other lab mods
// ============================================================

using CTaskBand_GetTaskbarHost_t = void* (WINAPI*)(void* pThis, void* result);
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
// Window thread marshalling / taskbar discovery
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

// ============================================================
// XAML helpers
// ============================================================

static FrameworkElement FindChildByName(FrameworkElement element, PCWSTR name) {
    int n = VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();
        if (child && child.Name() == name) return child;
    }
    return nullptr;
}

static FrameworkElement FindChildByClassName(FrameworkElement element, PCWSTR className) {
    int n = VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();
        if (child && winrt::get_class_name(child) == className) return child;
    }
    return nullptr;
}

static void EnumChildElements(FrameworkElement element,
    std::function<bool(FrameworkElement)> cb) {
    int n = VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();
        if (!child) continue;
        if (cb(child)) break;
    }
}

static FrameworkElement FindChildRecursive(FrameworkElement const& element,
    std::function<bool(FrameworkElement)> const& cb, int maxDepth = 20) {
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

static bool IsChildOfElementByName(FrameworkElement element, PCWSTR name) {
    auto parent = element;
    while (true) {
        parent = VisualTreeHelper::GetParent(parent).try_as<FrameworkElement>();
        if (!parent) return false;
        if (parent.Name() == name) return true;
    }
}

// ============================================================
// Privacy indicator logic
// ============================================================

// Unicode chars that appear in the privacy indicator TextBlock when active.
// Char values sourced from taskbar-tray-system-icon-tweaks.wh.cpp (m417z).
static bool IsPrivacyActiveText(std::wstring_view text) {
    if (text.empty()) return false;
    switch (text[0]) {
        case 0xE37A:  // Geolocation arrow
        case 0xF47F:  // Microphone + Geolocation (combined icon)
        case 0xE361:  // Microphone (private use area variant)
        case 0xE720:  // Microphone
        case 0xEC71:  // MicOn
            return true;
    }
    return false;
}

static void SetPrivacyIconOpacity(FrameworkElement iconView, bool active) {
    if (!iconView) return;
    iconView.Visibility(Visibility::Visible);
    iconView.Opacity(active ? 1.0 : (g_settings.idleOpacity / 100.0));
}

static void ApplyPrivacyIndicatorBehavior(FrameworkElement notifyIconViewElement) {
    // Avoid double-applying to the same element
    for (auto& s : g_privacyStates)
        if (s.iconViewRef.get() == notifyIconViewElement) return;

    // Path: SystemTrayIcon > ContainerGrid > ContentPresenter > ContentGrid >
    //       SystemTray.TextIconContent > ContainerGrid > Base > InnerTextBlock
    FrameworkElement child = notifyIconViewElement;
    if (!(child = FindChildByName(child, L"ContainerGrid")))    { Wh_Log(L"[PIB] ContainerGrid not found"); return; }
    if (!(child = FindChildByName(child, L"ContentPresenter"))) { Wh_Log(L"[PIB] ContentPresenter not found"); return; }
    if (!(child = FindChildByName(child, L"ContentGrid")))      { Wh_Log(L"[PIB] ContentGrid not found"); return; }
    child = FindChildByClassName(child, L"SystemTray.TextIconContent");
    if (!child) { Wh_Log(L"[PIB] TextIconContent not found — not a privacy icon, skipping"); return; }
    if (!(child = FindChildByName(child, L"ContainerGrid")))  { Wh_Log(L"[PIB] inner ContainerGrid not found"); return; }
    if (!(child = FindChildByName(child, L"Base")))           { Wh_Log(L"[PIB] Base not found"); return; }
    if (!(child = FindChildByName(child, L"InnerTextBlock"))) { Wh_Log(L"[PIB] InnerTextBlock not found"); return; }

    auto innerTextBlock = child.try_as<TextBlock>();
    if (!innerTextBlock) return;

    bool active = IsPrivacyActiveText(std::wstring_view(innerTextBlock.Text()));
    SetPrivacyIconOpacity(notifyIconViewElement, active);

    PrivacyState state;
    state.iconViewRef  = winrt::make_weak(notifyIconViewElement);
    state.textBlockRef = innerTextBlock;

    auto iconViewRef = winrt::make_weak(notifyIconViewElement);

    // Watch text changes to update opacity when privacy state changes.
    state.textToken = innerTextBlock.RegisterPropertyChangedCallback(
        TextBlock::TextProperty(),
        [iconViewRef](DependencyObject sender, DependencyProperty) {
            if (g_unloading) return;
            auto iconView = iconViewRef.get();
            auto tb       = sender.try_as<TextBlock>();
            if (!iconView || !tb) return;
            bool act = IsPrivacyActiveText(std::wstring_view(tb.Text()));
            SetPrivacyIconOpacity(iconView, act);
        });

    // Watch visibility changes to prevent the system from collapsing the element.
    // When the system sets Collapsed, we immediately override back to Visible.
    // This callback fires at most twice per transition (once for Collapsed, once
    // for our Visible override) — not an infinite loop.
    state.visibilityToken = notifyIconViewElement.RegisterPropertyChangedCallback(
        UIElement::VisibilityProperty(),
        [iconViewRef](DependencyObject sender, DependencyProperty) {
            if (g_unloading) return;
            auto iconView = sender.try_as<FrameworkElement>();
            if (!iconView || iconView.Visibility() != Visibility::Collapsed) return;
            iconView.Visibility(Visibility::Visible);
        });

    g_privacyStates.push_back(std::move(state));
    Wh_Log(L"[Privacy] Anchored element, initial active=%d", active);
}

static void ApplyMainStackStyle(FrameworkElement mainStack) {
    // Search recursively for all SystemTray.IconView elements named "SystemTrayIcon"
    // rather than relying on a fragile fixed-depth path that may vary across Windows builds.
    int count = 0;
    FindChildRecursive(mainStack, [&count](FrameworkElement fe) -> bool {
        if (winrt::get_class_name(fe) == L"SystemTray.IconView" &&
            fe.Name() == L"SystemTrayIcon") {
            Wh_Log(L"[MainStack] Found SystemTrayIcon, trying to anchor");
            ApplyPrivacyIndicatorBehavior(fe);
            count++;
        }
        return false; // always continue searching
    });
    Wh_Log(L"[MainStack] Scan complete, anchored %d icon(s)", count);
}

static void ApplyStyle() {
    Wh_Log(L"[Apply] enter");
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) { Wh_Log(L"[Apply] No taskbar window"); return; }
    g_taskbarWnd = hWnd;

    Wh_Log(L"[Apply] Getting XamlRoot");
    XamlRoot xamlRoot = nullptr;
    try { xamlRoot = GetTaskbarXamlRoot(hWnd); }
    catch (...) { Wh_Log(L"[Apply] Exception in GetTaskbarXamlRoot"); return; }
    if (!xamlRoot) { Wh_Log(L"[Apply] XamlRoot unavailable"); return; }

    auto root = xamlRoot.Content().try_as<FrameworkElement>();
    if (!root) { Wh_Log(L"[Apply] No root FrameworkElement"); return; }

    // Navigate: root → SystemTray.SystemTrayFrame → SystemTrayFrameGrid → MainStack
    // (Same path as taskbar-tray-system-icon-tweaks)
    FrameworkElement child = root;
    child = FindChildByClassName(child, L"SystemTray.SystemTrayFrame");
    if (!child) { Wh_Log(L"[Apply] SystemTray.SystemTrayFrame not found"); return; }
    child = FindChildByName(child, L"SystemTrayFrameGrid");
    if (!child) { Wh_Log(L"[Apply] SystemTrayFrameGrid not found"); return; }

    auto mainStack = FindChildByName(child, L"MainStack");
    if (!mainStack) {
        int n = VisualTreeHelper::GetChildrenCount(child);
        Wh_Log(L"[Apply] MainStack not found; SystemTrayFrameGrid has %d children", n);
        for (int i = 0; i < n && i < 10; i++) {
            auto c = VisualTreeHelper::GetChild(child, i).try_as<FrameworkElement>();
            if (c) Wh_Log(L"[Apply]   child[%d] name=%s", i, c.Name().c_str());
        }
        return;
    }

    Wh_Log(L"[Apply] Found MainStack, traversing");
    ApplyMainStackStyle(mainStack);
}

static void ApplyStyleOnWindowThread() {
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) { Wh_Log(L"[ASOWT] No taskbar HWND"); return; }
    Wh_Log(L"[ASOWT] Dispatching, hWnd=%p", hWnd);
    bool ok = RunFromWindowThread(hWnd, [](void*) {
        Wh_Log(L"[ASOWT] On window thread");
        g_loadedRevokers.clear();
        ClearPrivacyStates();
        ApplyStyle();
    }, nullptr);
    if (!ok) Wh_Log(L"[ASOWT] RunFromWindowThread failed");
}

// Restore elements to the state Windows shows without this mod.
static void ClearPrivacyStates() {
    for (auto& state : g_privacyStates) {
        if (auto tb = state.textBlockRef.get())
            tb.UnregisterPropertyChangedCallback(TextBlock::TextProperty(), state.textToken);

        if (auto iconView = state.iconViewRef.get()) {
            iconView.UnregisterPropertyChangedCallback(
                UIElement::VisibilityProperty(), state.visibilityToken);
            auto tb     = state.textBlockRef.get();
            bool active = tb && IsPrivacyActiveText(std::wstring_view(tb.Text()));
            iconView.Opacity(1.0);
            iconView.Visibility(active ? Visibility::Visible : Visibility::Collapsed);
        }
    }
    g_privacyStates.clear();
}

// ============================================================
// Hooks
// ============================================================

using IconView_IconView_t = void* (WINAPI*)(void* pThis);
IconView_IconView_t IconView_IconView_Original;

void* WINAPI IconView_IconView_Hook(void* pThis) {
    void* ret = IconView_IconView_Original(pThis);
    if (g_unloading) return ret;
    Wh_Log(L"[Hook] IconView created");

    FrameworkElement iconView = nullptr;
    ((IUnknown**)pThis)[1]->QueryInterface(winrt::guid_of<FrameworkElement>(),
                                            winrt::put_abi(iconView));
    if (!iconView) return ret;

    g_loadedRevokers.emplace_back();
    auto it = g_loadedRevokers.end();
    --it;

    *it = iconView.Loaded(
        winrt::auto_revoke_t{},
        [it](winrt::Windows::Foundation::IInspectable const& sender, auto const&) {
            g_loadedRevokers.erase(it);
            if (g_unloading) return;

            auto fe = sender.try_as<FrameworkElement>();
            if (!fe) return;

            if (winrt::get_class_name(fe) == L"SystemTray.IconView" &&
                fe.Name() == L"SystemTrayIcon" &&
                IsChildOfElementByName(fe, L"MainStack")) {
                ApplyPrivacyIndicatorBehavior(fe);
            }
        });

    return ret;
}

using LoadLibraryExW_t = HMODULE (WINAPI*)(LPCWSTR, HANDLE, DWORD);
LoadLibraryExW_t LoadLibraryExW_Original;

HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR path, HANDLE file, DWORD flags) {
    HMODULE h = LoadLibraryExW_Original(path, file, flags);
    if (h && path && !g_taskbarViewDllLoaded) {
        const wchar_t* base = wcsrchr(path, L'\\');
        base = base ? base + 1 : path;
        if (_wcsicmp(base, L"Taskbar.View.dll") == 0) {
            Wh_Log(L"[LLEW] Taskbar.View.dll loaded, hooking");
            WindhawkUtils::SYMBOL_HOOK hooks[] = {{
                {LR"(public: __cdecl winrt::SystemTray::implementation::IconView::IconView(void))"},
                &IconView_IconView_Original, IconView_IconView_Hook,
            }};
            if (WindhawkUtils::HookSymbols(h, hooks, ARRAYSIZE(hooks)))
                g_taskbarViewDllLoaded = true;
            Wh_Log(L"[LLEW] Hook result: %d", g_taskbarViewDllLoaded ? 1 : 0);
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
    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        { {LR"(const CTaskBand::`vftable'{for `ITaskListWndSite'})"},
          &CTaskBand_ITaskListWndSite_vftable },
        { {LR"(public: virtual class std::shared_ptr<class TaskbarHost> __cdecl CTaskBand::GetTaskbarHost(void)const )"},
          &CTaskBand_GetTaskbarHost_Original },
        { {LR"(public: int __cdecl TaskbarHost::FrameHeight(void)const )"},
          &TaskbarHost_FrameHeight_Original },
        { {LR"(public: void __cdecl std::_Ref_count_base::_Decref(void))"},
          &std__Ref_count_base__Decref_Original },
    };
    return WindhawkUtils::HookSymbols(h, hooks, ARRAYSIZE(hooks));
}

static bool HookTaskbarViewDllSymbols(HMODULE h) {
    WindhawkUtils::SYMBOL_HOOK hooks[] = {{
        {LR"(public: __cdecl winrt::SystemTray::implementation::IconView::IconView(void))"},
        &IconView_IconView_Original, IconView_IconView_Hook,
    }};
    if (!WindhawkUtils::HookSymbols(h, hooks, ARRAYSIZE(hooks))) return false;
    g_taskbarViewDllLoaded = true;
    return true;
}

static HMODULE GetTaskbarViewModule() {
    for (auto* name : { L"Taskbar.View.dll", L"taskbar.view.dll" })
        if (HMODULE h = GetModuleHandleW(name)) return h;
    return nullptr;
}

// ============================================================
// Windhawk lifecycle
// ============================================================

BOOL Wh_ModInit() {
    Wh_Log(L"[Init] Privacy Indicator Anchor v0.1");
    LoadSettings();

    if (!HookTaskbarDllSymbols())
        Wh_Log(L"[Init] taskbar.dll hooks failed — XamlRoot unavailable");

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
        if (HMODULE h = GetTaskbarViewModule()) {
            bool ok = HookTaskbarViewDllSymbols(h);
            Wh_Log(L"[AfterInit] HookTaskbarViewDllSymbols: %d", ok ? 1 : 0);
        } else {
            Wh_Log(L"[AfterInit] Taskbar.View.dll not yet loaded");
        }
    }
    Wh_Log(L"[AfterInit] g_taskbarViewDllLoaded=%d", g_taskbarViewDllLoaded ? 1 : 0);
    if (g_taskbarViewDllLoaded)
        ApplyStyleOnWindowThread();

    std::thread([]() {
        for (int i = 0; i < 5 && !g_unloading; i++) {
            Sleep(2000);
            if (!g_privacyStates.empty()) break;
            Wh_Log(L"[AfterInit] Retry %d", i + 1);
            ApplyStyleOnWindowThread();
        }
    }).detach();
}

void Wh_ModUninit() {
    g_unloading = true;
    Wh_Log(L"[Uninit]");

    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (hWnd) {
        RunFromWindowThread(hWnd, [](void*) {
            g_loadedRevokers.clear();
            ClearPrivacyStates();
        }, nullptr);
    }
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Wh_Log(L"[Settings] Changed, idleOpacity=%d", g_settings.idleOpacity);
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return;
    RunFromWindowThread(hWnd, [](void*) {
        for (auto& state : g_privacyStates) {
            auto iconView = state.iconViewRef.get();
            auto tb       = state.textBlockRef.get();
            if (!iconView || !tb) continue;
            bool active = IsPrivacyActiveText(std::wstring_view(tb.Text()));
            SetPrivacyIconOpacity(iconView, active);
        }
    }, nullptr);
}
