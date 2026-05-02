// ==WindhawkMod==
// @id              taskbar-clock-spacer
// @name            Taskbar Clock Spacer
// @description     Adds a %s% elastic spacer token to clock format strings, distributing leftover space evenly between items. Works with Taskbar Clock Customization.
// @version         0.1
// @author          sb4ssman
// @github          https://github.com/sb4ssman
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lruntimeobject
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

- `%s%` only works meaningfully when the clock area has a fixed width (set via
  Max width in Taskbar Clock Customization, or via the Line width override here).
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
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>

#include <atomic>
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
// Globals
// ============================================================

static std::atomic<bool> g_unloading{false};
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

static bool TryUpdateSpacerGrid(Grid g, const std::vector<std::wstring>& segs,
                                 TextBlock styleSrc) {
    if (!g) return false;
    int tbIdx = 0;
    for (auto ch : g.Children()) {
        auto tb = ch.try_as<TextBlock>();
        if (!tb) continue;
        if (tbIdx >= (int)segs.size()) return false;
        tb.Text(segs[tbIdx]);
        CopyTextStyle(styleSrc, tb);  // re-copy style in case it changed
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
        // No spacer token — ensure original is visible, hide Grid if any.
        if (auto sg = state.spacerGridRef.get())
            sg.Visibility(Visibility::Collapsed);
        original.Visibility(Visibility::Visible);
        return;
    }

    // Fast path: existing Grid with correct segment count.
    if (auto existing = state.spacerGridRef.get()) {
        if (TryUpdateSpacerGrid(existing, segs, original)) {
            existing.Visibility(Visibility::Visible);
            original.Visibility(Visibility::Collapsed);
            return;
        }
    }

    // Build (or rebuild with new segment count).
    auto newGrid = BuildSpacerGrid(original.Name(), segs, original);
    state.spacerGridRef = winrt::make_weak(newGrid);

    // Remove old spacer Grid if present.
    if (auto existing = state.spacerGridRef.get()) {
        uint32_t oldIdx;
        if (parent.Children().IndexOf(existing, oldIdx))
            parent.Children().RemoveAt(oldIdx);
    }

    uint32_t origIdx = 0;
    parent.Children().IndexOf(original, origIdx);
    parent.Children().InsertAt(origIdx + 1, newGrid);

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

    // Run once with current text.
    HandleTextChange(state, std::wstring_view(tb.Text()));

    g_states.push_back(std::move(state));

    // Capture by weak ref so we look up state at callback time, not by index.
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
    auto containerGrid = FindChildByName(elem, L"ContainerGrid").try_as<Grid>();
    if (!containerGrid) { Wh_Log(L"[Spacer] ContainerGrid not found"); return; }

    if (containerGrid.Children().Size() == 0) return;
    auto sp = containerGrid.Children().GetAt(0).try_as<StackPanel>();
    if (!sp) { Wh_Log(L"[Spacer] StackPanel not found"); return; }

    for (const auto& child : sp.Children()) {
        auto tb = child.try_as<TextBlock>();
        if (!tb) continue;
        auto name = std::wstring(tb.Name());
        if (name == kDateBlock || name == kTimeBlock)
            SetupSpacerForTextBlock(sp, tb);
    }
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
    // SystemTray.dll / Taskbar.View.dll / ExplorerExtensions.dll
    WindhawkUtils::SYMBOL_HOOK systemTrayHooks[] = {{
        {LR"(public: void __cdecl winrt::SystemTray::implementation::DateTimeIconContent::OnApplyTemplate(void))"},
        &DateTimeIconContent_OnApplyTemplate_Original,
        DateTimeIconContent_OnApplyTemplate_Hook,
        true,  // optional — not all builds have this; mod is a no-op if missing
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
    Wh_Log(L"[Init] Clock Spacer v0.1");
    LoadSettings();

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
