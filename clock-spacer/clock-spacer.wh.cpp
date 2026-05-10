// ==WindhawkMod==
// @id              taskbar-clock-spacer
// @name            Taskbar Clock Spacer
// @description     Adds a %s% elastic spacer token to clock format strings, distributing leftover space evenly between items. Works with Taskbar Clock Customization.
// @version         0.8
// @author          sb4ssman
// @github          https://github.com/sb4ssman
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lversion
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
- As an alternative, set `Max clock width` here to the same pixel value if
  you prefer not to touch the clock mod settings. This also applies MaxWidth to
  the generated spacer rows and the immediate clock panel.
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
- maxWidth: 0
  $name: Max clock width (px, 0 = off)
  $description: >-
    Applies a hard MaxWidth to the clock TextBlocks, generated spacer rows, and
    their immediate clock panel. Use this to stop changing clock text from
    resizing the taskbar/tray area. Set to the same width you use for the clock.
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
#include <winver.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

// ============================================================
// Settings
// ============================================================

struct ModSettings {
    int lineWidth = 0;
    int maxWidth = 0;
};
static ModSettings g_settings;

static void LoadSettings() {
    g_settings.lineWidth = Wh_GetIntSetting(L"lineWidth", 0);
    g_settings.maxWidth = Wh_GetIntSetting(L"maxWidth", 0);
    if (g_settings.lineWidth < 0) g_settings.lineWidth = 0;
    if (g_settings.maxWidth < 0) g_settings.maxWidth = 0;
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
static std::atomic<bool> g_systemTrayModuleHooked{false};

// Forward declaration
static void HandleLoadedModuleIfSystemTray(HMODULE hModule, LPCWSTR lpLibFileName);

static constexpr PCWSTR kSpacerToken    = L"%s%";
static constexpr size_t kSpacerTokenLen = 3;
static constexpr PCWSTR kDateBlock      = L"DateInnerTextBlock";
static constexpr PCWSTR kTimeBlock      = L"TimeInnerTextBlock";

struct SpacerState {
    winrt::weak_ref<TextBlock>  originalRef;
    winrt::weak_ref<StackPanel> parentRef;
    winrt::weak_ref<StackPanel> generatedRef;
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

static double EffectiveLineWidth(TextBlock original, StackPanel parent) {
    if (g_settings.lineWidth > 0)
        return (double)g_settings.lineWidth;
    if (g_settings.maxWidth > 0)
        return (double)g_settings.maxWidth;
    if (parent && parent.ActualWidth() > 1.0)
        return parent.ActualWidth();
    if (original && original.ActualWidth() > 1.0)
        return original.ActualWidth();
    return 0.0;
}

static void ApplyWidthConstraint(FrameworkElement fe) {
    if (!fe) return;
    if (g_settings.maxWidth > 0) {
        fe.MaxWidth((double)g_settings.maxWidth);
    } else {
        fe.ClearValue(FrameworkElement::MaxWidthProperty());
    }
}

static void ApplyClockWidthConstraints(TextBlock original, StackPanel parent) {
    ApplyWidthConstraint(original);
    ApplyWidthConstraint(parent);

    double width = g_settings.lineWidth > 0 ? (double)g_settings.lineWidth
                 : g_settings.maxWidth > 0 ? (double)g_settings.maxWidth
                 : 0.0;
    if (width <= 1.0) {
        original.ClearValue(FrameworkElement::WidthProperty());
        if (parent)
            parent.ClearValue(FrameworkElement::WidthProperty());
        return;
    }

    original.Width(width);
    if (parent)
        parent.Width(width);
}

static void CollapseSourceTextBlock(TextBlock original) {
    if (!original) return;
    original.Height(0.0);
    original.MinHeight(0.0);
    original.MaxHeight(0.0);
    original.Visibility(Visibility::Collapsed);
}

static void RestoreSourceTextBlock(TextBlock original) {
    if (!original) return;
    original.ClearValue(FrameworkElement::HeightProperty());
    original.ClearValue(FrameworkElement::MinHeightProperty());
    original.ClearValue(FrameworkElement::MaxHeightProperty());
    original.Visibility(Visibility::Visible);
}

// Build: [Auto TB] [* spacer] [Auto TB] [* spacer] ... [Auto TB]
static Grid BuildSpacerGrid(winrt::hstring const& name,
                             const std::vector<std::wstring>& segs,
                             TextBlock styleSrc,
                             double width) {
    Grid g;
    g.Name(name + L"_Spacer");
    g.HorizontalAlignment(HorizontalAlignment::Stretch);
    g.VerticalAlignment(VerticalAlignment::Center);
    ApplyWidthConstraint(g);
    if (width > 1.0)
        g.Width(width);

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

static std::vector<std::wstring> SplitLines(std::wstring_view text) {
    std::vector<std::wstring> lines;
    size_t pos = 0;
    while (pos <= text.size()) {
        size_t found = text.find(L'\n', pos);
        if (found == std::wstring_view::npos) {
            lines.emplace_back(text.substr(pos));
            break;
        }
        lines.emplace_back(text.substr(pos, found - pos));
        pos = found + 1;
    }
    return lines;
}

static FrameworkElement BuildLineElement(winrt::hstring const& baseName,
                                          std::wstring const& line,
                                          TextBlock styleSrc,
                                          StackPanel parent,
                                          int lineIndex) {
    auto segs = SplitOnSpacer(line);
    double width = EffectiveLineWidth(styleSrc, parent);

    if (segs.size() > 1)
        return BuildSpacerGrid(baseName + L"_Line" + winrt::to_hstring(lineIndex),
                               segs, styleSrc, width);

    TextBlock tb;
    tb.Name(baseName + L"_Line" + winrt::to_hstring(lineIndex));
    tb.Text(line);
    tb.VerticalAlignment(VerticalAlignment::Center);
    CopyTextStyle(styleSrc, tb);
    ApplyWidthConstraint(tb);
    if (width > 1.0)
        tb.Width(width);
    return tb;
}

// ============================================================
// Per-line update
// ============================================================

static void HandleTextChange(SpacerState& state, std::wstring_view fullText) {
    auto original = state.originalRef.get();
    auto parent   = state.parentRef.get();
    if (!original || !parent) return;

    ApplyClockWidthConstraints(original, parent);

    bool hasSpacer = fullText.find(kSpacerToken) != std::wstring_view::npos;
    if (!hasSpacer) {
        if (auto generated = state.generatedRef.get())
            generated.Visibility(Visibility::Collapsed);
        RestoreSourceTextBlock(original);
        return;
    }

    auto oldGenerated = state.generatedRef.get();
    if (oldGenerated) {
        uint32_t oldIdx;
        if (parent.Children().IndexOf(oldGenerated, oldIdx))
            parent.Children().RemoveAt(oldIdx);
    }

    StackPanel generated;
    generated.Name(original.Name() + L"_SpacerPanel");
    generated.Orientation(Orientation::Vertical);
    generated.HorizontalAlignment(HorizontalAlignment::Stretch);
    generated.VerticalAlignment(VerticalAlignment::Center);
    ApplyWidthConstraint(generated);

    double width = EffectiveLineWidth(original, parent);
    if (width > 1.0)
        generated.Width(width);

    auto lines = SplitLines(fullText);
    for (int i = 0; i < (int)lines.size(); i++) {
        auto lineElement = BuildLineElement(original.Name(), lines[i], original, parent, i);
        generated.Children().Append(lineElement);
    }

    uint32_t origIdx = 0;
    parent.Children().IndexOf(original, origIdx);
    parent.Children().InsertAt(origIdx + 1, generated);
    state.generatedRef = winrt::make_weak(generated);

    CollapseSourceTextBlock(original);
    generated.Visibility(Visibility::Visible);
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

static VS_FIXEDFILEINFO* GetModuleVersionInfo(HMODULE hModule, UINT* puPtrLen) {
    void* pFixedFileInfo = nullptr;
    UINT uPtrLen = 0;
    HRSRC hResource = FindResourceW(hModule, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
    if (hResource) {
        HGLOBAL hGlobal = LoadResource(hModule, hResource);
        if (hGlobal) {
            void* pData = LockResource(hGlobal);
            if (pData) {
                if (!VerQueryValueW(pData, L"\\", &pFixedFileInfo, &uPtrLen) || !uPtrLen)
                    pFixedFileInfo = nullptr;
            }
        }
    }
    if (puPtrLen) *puPtrLen = uPtrLen;
    return static_cast<VS_FIXEDFILEINFO*>(pFixedFileInfo);
}

// Order matters: SystemTray.dll is the new home (Win11 Insider 26200+);
// older builds have the symbols in Taskbar.View.dll.
static HMODULE GetSystemTrayModuleHandle() {
    if (HMODULE h = GetModuleHandleW(L"SystemTray.dll")) return h;
    if (HMODULE h = GetModuleHandleW(L"Taskbar.View.dll")) {
        // Starting with Taskbar.View.dll 2604.x, the SystemTray types moved
        // out into SystemTray.dll — don't hook this version.
        VS_FIXEDFILEINFO* fi = GetModuleVersionInfo(h, nullptr);
        WORD moduleMajor = fi ? HIWORD(fi->dwFileVersionMS) : 0;
        if (!moduleMajor || moduleMajor >= 2604) return nullptr;
        return h;
    }
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
            tb.ClearValue(FrameworkElement::WidthProperty());
            tb.ClearValue(FrameworkElement::MaxWidthProperty());
            RestoreSourceTextBlock(tb);
        }
        if (auto sp = state.parentRef.get()) {
            sp.ClearValue(FrameworkElement::WidthProperty());
            sp.ClearValue(FrameworkElement::MaxWidthProperty());
            if (auto generated = state.generatedRef.get()) {
                uint32_t idx;
                if (sp.Children().IndexOf(generated, idx))
                    sp.Children().RemoveAt(idx);
            }
        }
    }
    g_states.clear();
}

using LoadLibraryExW_t = HMODULE (WINAPI*)(LPCWSTR, HANDLE, DWORD);
LoadLibraryExW_t LoadLibraryExW_Original;

HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
    HMODULE hModule = LoadLibraryExW_Original(lpLibFileName, hFile, dwFlags);
    if (hModule && lpLibFileName)
        HandleLoadedModuleIfSystemTray(hModule, lpLibFileName);
    return hModule;
}

static void HandleLoadedModuleIfSystemTray(HMODULE hModule, LPCWSTR lpLibFileName) {
    if (!g_systemTrayModuleHooked && GetSystemTrayModuleHandle() == hModule &&
        !g_systemTrayModuleHooked.exchange(true)) {
        Wh_Log(L"[LoadLib] %s — hooking symbols", lpLibFileName);
        if (HookSystemTraySymbols(hModule))
            Wh_ApplyHookOperations();
    }
}

// ============================================================
// Windhawk lifecycle
// ============================================================

BOOL Wh_ModInit() {
    Wh_Log(L"[Init] Clock Spacer v0.8");
    LoadSettings();

    if (!HookTaskbarDllSymbols())
        Wh_Log(L"[Init] taskbar.dll symbols failed — initial scan disabled");

    if (HMODULE hSystemTray = GetSystemTrayModuleHandle()) {
        g_systemTrayModuleHooked = true;
        if (!HookSystemTraySymbols(hSystemTray))
            Wh_Log(L"[Init] System tray symbol hooks failed");
    } else {
        Wh_Log(L"[Init] System tray module not loaded yet");
        HMODULE kernelbase = GetModuleHandleW(L"kernelbase.dll");
        auto pLoadLibraryExW = kernelbase
            ? reinterpret_cast<LoadLibraryExW_t>(GetProcAddress(kernelbase, "LoadLibraryExW"))
            : nullptr;
        if (pLoadLibraryExW)
            WindhawkUtils::Wh_SetFunctionHookT(pLoadLibraryExW,
                                               LoadLibraryExW_Hook,
                                               &LoadLibraryExW_Original);
    }
    return TRUE;
}

void Wh_ModAfterInit() {
    if (!g_systemTrayModuleHooked) {
        if (HMODULE hSystemTray = GetSystemTrayModuleHandle()) {
            if (!g_systemTrayModuleHooked.exchange(true)) {
                Wh_Log(L"[AfterInit] System tray module found — hooking symbols");
                if (HookSystemTraySymbols(hSystemTray))
                    Wh_ApplyHookOperations();
            }
        }
    }
    Wh_Log(L"[AfterInit] hooked=%d", (int)g_systemTrayModuleHooked.load());

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
    // ClearSpacerStates touches WinRT objects — must run on the UI thread.
    if (HWND hWnd = FindCurrentProcessTaskbarWnd()) {
        RunFromWindowThread(hWnd, [](void*) { ClearSpacerStates(); }, nullptr);
    } else {
        ClearSpacerStates();
    }
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Wh_Log(L"[Settings] lineWidth=%d maxWidth=%d", g_settings.lineWidth, g_settings.maxWidth);
    HWND hWnd = FindCurrentProcessTaskbarWnd();
    if (!hWnd) {
        Wh_Log(L"[Settings] No taskbar window found");
        return;
    }

    RunFromWindowThread(hWnd, [](void*) {
        for (auto& state : g_states) {
            if (auto tb = state.originalRef.get())
                HandleTextChange(state, std::wstring_view(tb.Text()));
        }
    }, nullptr);
}
