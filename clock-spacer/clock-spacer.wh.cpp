// ==WindhawkMod==
// @id              taskbar-clock-spacer
// @name            Taskbar Clock Spacer
// @description     Adds a %s% elastic spacer token to clock format strings, distributing leftover space evenly between items. Works with Taskbar Clock Customization.
// @version         0.4
// @author          sb4ssman
// @github          https://github.com/sb4ssman
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Taskbar Clock Spacer

Companion mod for [Taskbar Clock Customization](https://windhawk.net/mods/taskbar-clock-customization).

Adds a `%s%` elastic spacer token to clock line formats. Place `%s%` between
items and the remaining width is distributed evenly as gaps.

**Example:** `%time%%s%%date%`
— time on the left, date on the right, gap between them fills the rest.

**Three items:** `%time%%s%%date%%s%%weekday%`
— equal gaps between three differently-sized items.

## Setup

1. Install and configure **Taskbar Clock Customization**. Set a **Max width**
   in its settings (e.g. 120 px) so the clock area has a fixed width.
2. Install this mod.
3. Add `%s%` between items in the Top Line or Bottom Line format of the
   clock mod. The token passes through the clock mod unchanged and is
   intercepted here for display.

## Notes

- **Max Width is required.** Set a fixed `Max width` (px) in Taskbar Clock
  Customization settings. This constrains the clock area so the gap columns
  have real space to fill. Without it the spacer columns are zero-width and
  `%s%` is silently removed from the display but no gap appears.
- As an alternative, set the `Line width override` here to the same pixel
  value if you prefer not to touch the clock mod settings.
- If no `%s%` is present the line renders exactly as before — this mod is a
  no-op for those lines.
- Font and color of inner text segments follow the original TextBlock's current
  style, so clock mod style settings apply automatically.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- lineWidth: 0
  $name: Line width override (px, 0 = auto)
  $description: >-
    Explicit width for the spacer grid. Usually 0 is correct — the clock area
    inherits its width from the Max width set in Taskbar Clock Customization.
    Set this only if the spacer doesn't expand as expected.
*/
// ==/WindhawkModSettings==

#undef GetCurrentTime

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>

#include <atomic>
#include <functional>
#include <string>
#include <vector>

#include <windhawk_utils.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

// ============================================================
// Settings
// ============================================================

struct ModSettings {
    int lineWidth = 0;
};
static ModSettings g_settings;

static void LoadSettings() {
    g_settings.lineWidth = Wh_GetIntSetting(L"lineWidth", 0);
}

// ============================================================
// GetTaskbarXamlRoot
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
        if (b[0] == 0x48 && b[1] == 0x83 && b[2] == 0xEC && b[4] == 0x48 &&
            b[5] == 0x83 && b[6] == 0xC1 && b[7] <= 0x7F)
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

// ============================================================
// Globals
// ============================================================

static std::atomic<bool> g_unloading{false};
static std::atomic<bool> g_scanDone{false};
static bool              g_systemTrayModuleHooked = false;

static constexpr PCWSTR kSpacerToken    = L"%s%";
static constexpr size_t kSpacerTokenLen = 3;
static constexpr PCWSTR kDateBlock      = L"DateInnerTextBlock";
static constexpr PCWSTR kTimeBlock      = L"TimeInnerTextBlock";

struct SpacerState {
    winrt::weak_ref<TextBlock>  originalRef;
    winrt::weak_ref<StackPanel> parentRef;
    winrt::weak_ref<Grid>       spacerGridRef;
    int64_t                     textToken = 0;
};
static std::vector<SpacerState> g_states;

// ============================================================
// XAML helpers
// ============================================================

static FrameworkElement FindChildByName(DependencyObject parent, PCWSTR name) {
    int n = VisualTreeHelper::GetChildrenCount(parent);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(parent, i).try_as<FrameworkElement>();
        if (child && child.Name() == name) return child;
    }
    return nullptr;
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

// ============================================================
// Spacer grid construction
// ============================================================

static std::vector<std::wstring> SplitOnSpacer(std::wstring_view text) {
    std::vector<std::wstring> segments;
    size_t pos = 0;
    while (true) {
        size_t found = text.find(kSpacerToken, pos);
        if (found == std::wstring_view::npos) {
            segments.emplace_back(text.substr(pos));
            break;
        }
        segments.emplace_back(text.substr(pos, found - pos));
        pos = found + kSpacerTokenLen;
    }
    return segments;
}

static void CopyTextStyle(TextBlock src, TextBlock dst) {
    dst.FontSize(src.FontSize());
    dst.FontFamily(src.FontFamily());
    dst.FontWeight(src.FontWeight());
    dst.FontStyle(src.FontStyle());
    dst.FontStretch(src.FontStretch());
    dst.CharacterSpacing(src.CharacterSpacing());
    dst.Foreground(src.Foreground());
    dst.TextWrapping(TextWrapping::NoWrap);
}

// Build: [Auto TB] [* spacer] [Auto TB] [* spacer] ... [Auto TB]
static Grid BuildSpacerGrid(winrt::hstring const& name,
                             const std::vector<std::wstring>& segs,
                             TextBlock styleSrc) {
    Grid g;
    g.Name(name + L"_Spacer");
    g.HorizontalAlignment(HorizontalAlignment::Stretch);
    g.VerticalAlignment(VerticalAlignment::Center);
    if (g_settings.lineWidth > 0)
        g.Width((double)g_settings.lineWidth);

    int nSegs = (int)segs.size();
    for (int i = 0; i < nSegs; i++) {
        ColumnDefinition cd;
        cd.Width({ 1.0, GridUnitType::Auto });
        g.ColumnDefinitions().Append(cd);
        if (i + 1 < nSegs) {
            ColumnDefinition cs;
            cs.Width({ 1.0, GridUnitType::Star });
            g.ColumnDefinitions().Append(cs);
        }
    }

    int gridCol = 0;
    for (int i = 0; i < nSegs; i++) {
        TextBlock tb;
        tb.Text(segs[i]);
        tb.VerticalAlignment(VerticalAlignment::Center);
        CopyTextStyle(styleSrc, tb);
        Grid::SetColumn(tb, gridCol);
        g.Children().Append(tb);
        gridCol += 2;
    }

    return g;
}

static void UpdateSpacerGridWidth(Grid g, TextBlock original, StackPanel parent) {
    if (!g) return;

    double width = 0.0;
    if (g_settings.lineWidth > 0) {
        width = (double)g_settings.lineWidth;
    } else if (parent && parent.ActualWidth() > 1.0) {
        width = parent.ActualWidth();
    } else if (original && original.ActualWidth() > 1.0) {
        width = original.ActualWidth();
    }

    if (width > 1.0)
        g.Width(width);
    else
        g.ClearValue(FrameworkElement::WidthProperty());
}

static bool TryUpdateSpacerGrid(Grid g, const std::vector<std::wstring>& segs,
                                 TextBlock styleSrc) {
    if (!g) return false;
    int tbIdx = 0;
    for (auto ch : g.Children()) {
        auto tb = ch.try_as<TextBlock>();
        if (!tb) continue;
        if (tbIdx >= (int)segs.size()) return false;
        tb.Text(segs[tbIdx]);
        CopyTextStyle(styleSrc, tb);
        tbIdx++;
    }
    return tbIdx == (int)segs.size();
}

// ============================================================
// Per-line update
// ============================================================

static void HandleTextChange(SpacerState& state, std::wstring_view newText) {
    auto original = state.originalRef.get();
    auto parent   = state.parentRef.get();
    if (!original || !parent) return;

    auto segs = SplitOnSpacer(newText);

    if (segs.size() <= 1) {
        if (auto sg = state.spacerGridRef.get())
            sg.Visibility(Visibility::Collapsed);
        original.Visibility(Visibility::Visible);
        return;
    }

    // Fast path: existing Grid with correct segment count.
    if (auto existing = state.spacerGridRef.get()) {
        if (TryUpdateSpacerGrid(existing, segs, original)) {
            UpdateSpacerGridWidth(existing, original, parent);
            existing.Visibility(Visibility::Visible);
            original.Visibility(Visibility::Collapsed);
            return;
        }
    }

    auto oldGrid = state.spacerGridRef.get();

    // Build (or rebuild with new segment count).
    auto newGrid = BuildSpacerGrid(original.Name(), segs, original);
    UpdateSpacerGridWidth(newGrid, original, parent);

    // Remove old spacer Grid if present.
    if (oldGrid) {
        uint32_t oldIdx;
        if (parent.Children().IndexOf(oldGrid, oldIdx))
            parent.Children().RemoveAt(oldIdx);
    }

    uint32_t origIdx = 0;
    parent.Children().IndexOf(original, origIdx);
    parent.Children().InsertAt(origIdx + 1, newGrid);
    state.spacerGridRef = winrt::make_weak(newGrid);

    original.Visibility(Visibility::Collapsed);
    newGrid.Visibility(Visibility::Visible);
}

// ============================================================
// Template hook logic
// ============================================================

static void SetupSpacerForTextBlock(StackPanel sp, TextBlock tb) {
    if (!sp || !tb) return;

    // Avoid double-apply.
    for (auto& s : g_states)
        if (s.originalRef.get() == tb) return;

    SpacerState state;
    state.originalRef = winrt::make_weak(tb);
    state.parentRef   = winrt::make_weak(sp);

    HandleTextChange(state, std::wstring_view(tb.Text()));

    g_states.push_back(std::move(state));

    auto tbWeak = winrt::make_weak(tb);
    g_states.back().textToken = tb.RegisterPropertyChangedCallback(
        TextBlock::TextProperty(),
        [tbWeak](DependencyObject sender, DependencyProperty) {
            if (g_unloading) return;
            auto tbRef = sender.try_as<TextBlock>();
            if (!tbRef) return;
            for (auto& s : g_states) {
                if (s.originalRef.get() == tbRef) {
                    HandleTextChange(s, std::wstring_view(tbRef.Text()));
                    break;
                }
            }
        });

    Wh_Log(L"[Spacer] Registered '%s'", tb.Name().c_str());
}

static void ApplySpacerToDateTimeContent(FrameworkElement elem) {
    PCWSTR blockNames[] = {kTimeBlock, kDateBlock};
    int found = 0;
    for (PCWSTR blockName : blockNames) {
        auto tbElem = FindChildRecursive(elem, [blockName](FrameworkElement fe) {
            return fe.Name() == blockName;
        });
        if (!tbElem) { Wh_Log(L"[Spacer] '%s' not found", blockName); continue; }
        auto tb = tbElem.try_as<TextBlock>();
        if (!tb) continue;
        auto parentDep = VisualTreeHelper::GetParent(tb);
        if (!parentDep) continue;
        auto sp = parentDep.try_as<StackPanel>();
        if (!sp) { Wh_Log(L"[Spacer] parent of '%s' not StackPanel", blockName); continue; }
        SetupSpacerForTextBlock(sp, tb);
        found++;
    }
    if (!found) Wh_Log(L"[Spacer] No TextBlocks found in DateTimeIconContent");
}

// ============================================================
// Initial scan (for elements rendered before mod load)
// ============================================================

static void ScanForSpacerTargets(FrameworkElement root) {
    if (!root) return;
    // Use class name — ContainerGrid appears in many system tray elements; class
    // name is the only reliable way to identify DateTimeIconContent specifically.
    try {
        if (winrt::get_class_name(root) == L"SystemTray.DateTimeIconContent") {
            ApplySpacerToDateTimeContent(root);
            return;
        }
    } catch (...) {}
    int n = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(root, i).try_as<FrameworkElement>();
        if (child) ScanForSpacerTargets(child);
    }
}

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
        DWORD pid = 0;
        WCHAR cls[32];
        if (GetWindowThreadProcessId(hWnd, &pid) && pid == GetCurrentProcessId() &&
            GetClassNameW(hWnd, cls, ARRAYSIZE(cls)) &&
            _wcsicmp(cls, L"Shell_TrayWnd") == 0) {
            *reinterpret_cast<HWND*>(lParam) = hWnd;
            return FALSE;
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&result));
    return result;
}

// ============================================================
// Hook
// ============================================================

using DateTimeIconContent_OnApplyTemplate_t = void(WINAPI*)(void* pThis);
DateTimeIconContent_OnApplyTemplate_t DateTimeIconContent_OnApplyTemplate_Original;

void WINAPI DateTimeIconContent_OnApplyTemplate_Hook(void* pThis) {
    DateTimeIconContent_OnApplyTemplate_Original(pThis);
    if (g_unloading) return;

    auto* iunk = *((IUnknown**)pThis + 1);
    if (!iunk) return;
    FrameworkElement elem = nullptr;
    iunk->QueryInterface(winrt::guid_of<FrameworkElement>(), winrt::put_abi(elem));
    if (!elem) return;

    try {
        ApplySpacerToDateTimeContent(elem);
    } catch (...) {
        Wh_Log(L"[Spacer] Exception");
    }
}

// ============================================================
// Symbol hook setup — SystemTray.dll (new) or Taskbar.View.dll / ExplorerExtensions.dll (old)
// ============================================================

static bool HookSystemTraySymbols(HMODULE h) {
    WindhawkUtils::SYMBOL_HOOK systemTrayHooks[] = {{
        {LR"(public: void __cdecl winrt::SystemTray::implementation::DateTimeIconContent::OnApplyTemplate(void))"},
        &DateTimeIconContent_OnApplyTemplate_Original,
        DateTimeIconContent_OnApplyTemplate_Hook,
        true,
    }};
    return WindhawkUtils::HookSymbols(h, systemTrayHooks, ARRAYSIZE(systemTrayHooks));
}

static HMODULE GetSystemTrayModule() {
    if (HMODULE h = GetModuleHandleW(L"SystemTray.dll"))      return h;
    if (HMODULE h = GetModuleHandleW(L"Taskbar.View.dll"))    return h;
    if (HMODULE h = GetModuleHandleW(L"ExplorerExtensions.dll")) return h;
    return nullptr;
}

// ============================================================
// Uninit
// ============================================================

static void ClearSpacerStates() {
    for (auto& state : g_states) {
        if (auto tb = state.originalRef.get()) {
            if (state.textToken)
                tb.UnregisterPropertyChangedCallback(TextBlock::TextProperty(), state.textToken);
            tb.Visibility(Visibility::Visible);
        }
        if (auto sp = state.parentRef.get()) {
            if (auto sg = state.spacerGridRef.get()) {
                uint32_t idx;
                if (sp.Children().IndexOf(sg, idx))
                    sp.Children().RemoveAt(idx);
            }
        }
    }
    g_states.clear();
}

// ============================================================
// Windhawk lifecycle
// ============================================================

BOOL Wh_ModInit() {
    Wh_Log(L"[Init] Clock Spacer v0.4");
    LoadSettings();

    if (!HookTaskbarDllSymbols())
        Wh_Log(L"[Init] taskbar.dll symbols failed — initial scan disabled");

    HMODULE h = GetSystemTrayModule();
    if (h) {
        if (HookSystemTraySymbols(h))
            g_systemTrayModuleHooked = true;
        else
            Wh_Log(L"[Init] Hook failed");
    } else {
        Wh_Log(L"[Init] SystemTray module not yet loaded");
    }

    return TRUE;
}

void Wh_ModAfterInit() {
    if (!g_systemTrayModuleHooked) {
        if (HMODULE h = GetSystemTrayModule()) {
            if (HookSystemTraySymbols(h))
                g_systemTrayModuleHooked = true;
        }
    }
    Wh_Log(L"[AfterInit] hooked=%d", g_systemTrayModuleHooked ? 1 : 0);

    // Scan for DateTimeIconContent elements already rendered before mod load.
    HANDLE thread = CreateThread(nullptr, 0, [](void*) -> DWORD {
        for (int i = 0; i < 5 && !g_unloading && !g_scanDone; i++) {
            if (i > 0) Sleep(2000);
            HWND hWnd = FindCurrentProcessTaskbarWnd();
            if (!hWnd) continue;
            RunFromWindowThread(hWnd, [](void* param) {
                HWND h = (HWND)param;
                auto xamlRoot = GetTaskbarXamlRoot(h);
                if (!xamlRoot) return;
                auto root = xamlRoot.Content().try_as<FrameworkElement>();
                if (!root) return;
                g_scanDone = true;
                ScanForSpacerTargets(root);
                Wh_Log(L"[Spacer] Scan done, states=%d", (int)g_states.size());
            }, hWnd);
            if (g_scanDone) break;
        }
        return 0;
    }, nullptr, 0, nullptr);
    if (thread) CloseHandle(thread);
}

void Wh_ModUninit() {
    g_unloading = true;
    Wh_Log(L"[Uninit]");
    ClearSpacerStates();
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Wh_Log(L"[Settings] lineWidth=%d", g_settings.lineWidth);
    for (auto& state : g_states) {
        if (auto tb = state.originalRef.get())
            HandleTextChange(state, std::wstring_view(tb.Text()));
    }
}
