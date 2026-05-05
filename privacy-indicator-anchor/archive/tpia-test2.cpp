// ==WindhawkMod==
// @id              tray-privacy-indicator-anchor
// @name            Tray Privacy Indicator Anchor
// @description     Permanently shows location/microphone icons in the system tray — dim when idle, bright when in use — preventing taskbar layout shifts.
// @version         0.5
// @author          sb4ssman
// @github          https://github.com/sb4ssman
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lversion
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*...*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*...*/
// ==/WindhawkModSettings==

#undef GetCurrentTime

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Automation.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>

#include <algorithm>
#include <atomic>
#include <functional>
#include <list>
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
    int  idleOpacity   = 50;
    bool showLocation  = true;
    bool showMic       = true;
    int  iconSize      = 16;
    std::wstring layoutMode = L"row";
    std::wstring position = L"beforeOmni";
    bool hideNativeIndicator = true;
    int  paddingLeft  = 2;
    int  paddingRight = 2;
    int  iconSpacing  = 2;
    int  barOffsetX   = 0;
    int  barOffsetY   = 0;
    int  locationOffsetX = 0;
    int  locationOffsetY = 0;
    int  micOffsetX   = 0;
    int  micOffsetY   = 0;
};
static ModSettings g_settings;

static std::wstring GetStringSetting(PCWSTR name, PCWSTR fallback) {
    PCWSTR raw = Wh_GetStringSetting(name);
    std::wstring value = raw ? raw : fallback;
    Wh_FreeStringSetting(raw);
    return value;
}

static void LoadSettings() {
    auto clampPx = [](int v, int minValue, int maxValue) {
        return std::max(minValue, std::min(maxValue, v));
    };
    g_settings.idleOpacity  = std::max(0, std::min(100, Wh_GetIntSetting(L"idleOpacity", 50)));
    g_settings.showLocation = Wh_GetIntSetting(L"showLocation", 1) != 0;
    g_settings.showMic      = Wh_GetIntSetting(L"showMic", 1) != 0;
    g_settings.iconSize     = std::max(8, std::min(32, Wh_GetIntSetting(L"iconSize", 16)));
    g_settings.layoutMode   = GetStringSetting(L"layoutMode", L"row");
    g_settings.position     = GetStringSetting(L"position", L"beforeOmni");
    g_settings.hideNativeIndicator = Wh_GetIntSetting(L"hideNativeIndicator", 1) != 0;
    g_settings.paddingLeft  = clampPx(Wh_GetIntSetting(L"paddingLeft",  2), -40, 40);
    g_settings.paddingRight = clampPx(Wh_GetIntSetting(L"paddingRight", 2), -40, 40);
    g_settings.iconSpacing  = clampPx(Wh_GetIntSetting(L"iconSpacing", 2), 0, 40);
    g_settings.barOffsetX   = clampPx(Wh_GetIntSetting(L"barOffsetX", 0), -40, 40);
    g_settings.barOffsetY   = clampPx(Wh_GetIntSetting(L"barOffsetY", 0), -40, 40);
    g_settings.locationOffsetX = clampPx(Wh_GetIntSetting(L"locationOffsetX", 0), -40, 40);
    g_settings.locationOffsetY = clampPx(Wh_GetIntSetting(L"locationOffsetY", 0), -40, 40);
    g_settings.micOffsetX   = clampPx(Wh_GetIntSetting(L"micOffsetX", 0), -40, 40);
    g_settings.micOffsetY   = clampPx(Wh_GetIntSetting(L"micOffsetY", 0), -40, 40);
}

// ============================================================
// Globals
// ============================================================

static std::atomic<bool> g_unloading{false};
static HWND              g_taskbarWnd           = nullptr;
static bool              g_taskbarViewDllLoaded = false;
static HANDLE            g_retryThread          = nullptr;
static HANDLE            g_retryStopEvent       = nullptr;

// Synthetic icon state
static std::atomic<bool> g_locActive{false};
static std::atomic<bool> g_micActive{false};
static Grid              g_syntheticGrid   = nullptr;
static TextBlock         g_locIcon         = nullptr;
static TextBlock         g_micIcon         = nullptr;
static FrameworkElement  g_syntheticParent = nullptr;
static int               g_syntheticColumn = -1;

// Tracks real privacy indicator elements so we can read their active/idle state.
struct PrivacyState {
    enum class Type { Location, Mic, Both };
    winrt::weak_ref<FrameworkElement> iconViewRef;
    winrt::weak_ref<TextBlock>        textBlockRef;
    int64_t textToken = 0;
    Type    type      = Type::Location;
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
static void RemoveSyntheticIcons();
static void StopRetryThread();
static bool HookTaskbarViewDllSymbols(HMODULE h);

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
// Privacy type detection
// ============================================================

// Unicode chars from taskbar-tray-system-icon-tweaks.wh.cpp (m417z).
static PrivacyState::Type DetectPrivacyType(std::wstring_view text) {
    if (text.empty()) return PrivacyState::Type::Location; // default fallback
    switch (text[0]) {
        case 0xE37A: return PrivacyState::Type::Location;
        case 0xF47F: return PrivacyState::Type::Both;
        case 0xE361:
        case 0xE720:
        case 0xEC71: return PrivacyState::Type::Mic;
        default:     return PrivacyState::Type::Location;
    }
}

static bool IsPrivacyGlyph(wchar_t c) {
    return c == 0xE37A || c == 0xF47F ||
           c == 0xE361 || c == 0xE720 || c == 0xEC71;
}

// Returns true only for known privacy indicator text (glyph or empty/idle).
// Guards against false matches on battery %, volume glyph, etc.
static bool IsPrivacyText(std::wstring_view text) {
    return text.empty() || (text.length() == 1 && IsPrivacyGlyph(text[0]));
}

// ============================================================
// Synthetic icon management
// ============================================================

static void UpdateSyntheticOpacity() {
    double idleOp = g_settings.idleOpacity / 100.0;
    if (g_locIcon)
        g_locIcon.Opacity(g_locActive.load() ? 1.0 : idleOp);
    if (g_micIcon)
        g_micIcon.Opacity(g_micActive.load() ? 1.0 : idleOp);
}

static void SetPrivacyActive(PrivacyState::Type type, bool active) {
    switch (type) {
        case PrivacyState::Type::Location:
            g_locActive.store(active);
            break;
        case PrivacyState::Type::Mic:
            g_micActive.store(active);
            break;
        case PrivacyState::Type::Both:
            g_locActive.store(active);
            g_micActive.store(active);
            break;
    }
    UpdateSyntheticOpacity();
}

static TextBlock MakeIconTextBlock(const wchar_t* glyph) {
    TextBlock tb;
    tb.Text(glyph);
    tb.FontFamily(FontFamily(L"Segoe MDL2 Assets"));
    tb.FontSize((double)g_settings.iconSize);
    tb.VerticalAlignment(VerticalAlignment::Center);
    tb.HorizontalAlignment(HorizontalAlignment::Center);
    tb.TextWrapping(TextWrapping::NoWrap);
    tb.Opacity(g_settings.idleOpacity / 100.0);
    return tb;
}

static void ApplyOffset(FrameworkElement const& fe, int x, int y) {
    if (!fe) return;
    if (x != 0 || y != 0) {
        TranslateTransform tt;
        tt.X((double)x);
        tt.Y((double)y);
        fe.RenderTransform(tt);
    } else {
        fe.ClearValue(UIElement::RenderTransformProperty());
    }
}

static void ApplyIconSpacing(FrameworkElement const& fe, bool vertical, bool hasNextIcon) {
    if (!fe) return;
    if (!hasNextIcon || g_settings.iconSpacing <= 0) {
        fe.ClearValue(FrameworkElement::MarginProperty());
        return;
    }

    if (vertical)
        fe.Margin({ 0.0, 0.0, 0.0, (double)g_settings.iconSpacing });
    else
        fe.Margin({ 0.0, 0.0, (double)g_settings.iconSpacing, 0.0 });
}

static bool InjectSyntheticIcons(FrameworkElement root) {
    // Find SystemTrayFrameGrid by recursive name search (same approach as VDS).
    // FindChildByClassName is shallow; SystemTrayFrameGrid is nested several levels deep.
    auto gridElem = FindChildRecursive(root, [](FrameworkElement fe) {
        return fe.Name() == L"SystemTrayFrameGrid";
    });
    if (!gridElem) { Wh_Log(L"[Inject] SystemTrayFrameGrid not found"); return false; }
    auto gridParent = gridElem.try_as<Grid>();
    if (!gridParent) { Wh_Log(L"[Inject] SystemTrayFrameGrid not a Grid"); return false; }

    // Already injected?
    for (auto child : gridParent.Children()) {
        if (auto fe = child.try_as<FrameworkElement>(); fe && fe.Name() == L"PrivacyAnchorBar")
            return true;
    }

    if (!g_settings.showLocation && !g_settings.showMic) {
        Wh_Log(L"[Inject] Both icons disabled — nothing to inject");
        return true;
    }

    auto findNamedDirect = [&](const wchar_t* name) -> FrameworkElement {
        for (auto child : gridParent.Children()) {
            if (auto fe = child.try_as<FrameworkElement>(); fe && fe.Name() == name)
                return fe;
        }
        return nullptr;
    };

    FrameworkElement refElem = nullptr;
    bool insertAfterRef = false;
    if      (g_settings.position == L"beforeOmni")
        refElem = findNamedDirect(L"ControlCenterButton");
    else if (g_settings.position == L"beforeClock")
        refElem = findNamedDirect(L"NotificationCenterButton");
    else if (g_settings.position == L"afterClock")
        refElem = findNamedDirect(L"ShowDesktopStack");
    else if (g_settings.position == L"afterShowDesktop") {
        refElem = findNamedDirect(L"ShowDesktopStack");
        insertAfterRef = true;
    }

    int insertCol = 0;
    if (insertAfterRef && refElem)
        insertCol = Grid::GetColumn(refElem) + 1;
    else if (refElem)
        insertCol = Grid::GetColumn(refElem);

    // Insert a new Auto column at the selected tray position.
    ColumnDefinition cd;
    cd.Width({ 1.0, GridUnitType::Auto });
    if ((uint32_t)insertCol < gridParent.ColumnDefinitions().Size())
        gridParent.ColumnDefinitions().InsertAt(insertCol, cd);
    else
        gridParent.ColumnDefinitions().Append(cd);

    for (auto child : gridParent.Children()) {
        auto fe = child.try_as<FrameworkElement>();
        if (!fe) continue;
        int col  = Grid::GetColumn(fe);
        int span = Grid::GetColumnSpan(fe);
        if (col >= insertCol)
            Grid::SetColumn(fe, col + 1);
        else if (col + span > insertCol)
            Grid::SetColumnSpan(fe, span + 1);
    }

    // Build the anchor bar.
    Grid bar;
    bar.Name(L"PrivacyAnchorBar");
    bar.VerticalAlignment(VerticalAlignment::Center);
    bar.HorizontalAlignment(HorizontalAlignment::Center);
    bar.Margin({ (double)g_settings.paddingLeft, 0.0, (double)g_settings.paddingRight, 0.0 });
    ApplyOffset(bar, g_settings.barOffsetX, g_settings.barOffsetY);

    double idleOp = g_settings.idleOpacity / 100.0;
    bool vertical = g_settings.layoutMode == L"column";
    int visibleIconCount = (g_settings.showLocation ? 1 : 0) + (g_settings.showMic ? 1 : 0);
    int itemIdx = 0;

    if (g_settings.showLocation) {
        if (vertical) {
            RowDefinition rd;
            rd.Height({ 1.0, GridUnitType::Auto });
            bar.RowDefinitions().Append(rd);
        } else {
            ColumnDefinition lc;
            lc.Width({ 1.0, GridUnitType::Auto });
            bar.ColumnDefinitions().Append(lc);
        }
        auto loc = MakeIconTextBlock(L"\xE37A");
        loc.Opacity(g_locActive.load() ? 1.0 : idleOp);
        ApplyOffset(loc, g_settings.locationOffsetX, g_settings.locationOffsetY);
        ApplyIconSpacing(loc, vertical, itemIdx + 1 < visibleIconCount);
        if (vertical)
            Grid::SetRow(loc, itemIdx++);
        else
            Grid::SetColumn(loc, itemIdx++);
        bar.Children().Append(loc);
        g_locIcon = loc;
    }
    if (g_settings.showMic) {
        if (vertical) {
            RowDefinition rd;
            rd.Height({ 1.0, GridUnitType::Auto });
            bar.RowDefinitions().Append(rd);
        } else {
            ColumnDefinition mc;
            mc.Width({ 1.0, GridUnitType::Auto });
            bar.ColumnDefinitions().Append(mc);
        }
        auto mic = MakeIconTextBlock(L"\xE720");
        mic.Opacity(g_micActive.load() ? 1.0 : idleOp);
        ApplyOffset(mic, g_settings.micOffsetX, g_settings.micOffsetY);
        ApplyIconSpacing(mic, vertical, itemIdx + 1 < visibleIconCount);
        if (vertical)
            Grid::SetRow(mic, itemIdx++);
        else
            Grid::SetColumn(mic, itemIdx++);
        bar.Children().Append(mic);
        g_micIcon = mic;
    }

    Grid::SetColumn(bar, insertCol);
    gridParent.Children().Append(bar);

    g_syntheticGrid   = bar;
    g_syntheticParent = gridElem;
    g_syntheticColumn = insertCol;

    Wh_Log(L"[Inject] PrivacyAnchorBar injected at col=%d (loc=%d mic=%d)",
           insertCol, g_settings.showLocation ? 1 : 0, g_settings.showMic ? 1 : 0);
    return true;
}

static void RemoveSyntheticIcons() {
    auto gridParent = g_syntheticParent ? g_syntheticParent.try_as<Grid>() : nullptr;
    if (!gridParent) {
        g_syntheticGrid = nullptr; g_locIcon = nullptr; g_micIcon = nullptr;
        g_syntheticParent = nullptr; g_syntheticColumn = -1;
        return;
    }

    // Remove the grid child.
    for (uint32_t i = 0; i < gridParent.Children().Size(); i++) {
        auto fe = gridParent.Children().GetAt(i).try_as<FrameworkElement>();
        if (fe && fe.Name() == L"PrivacyAnchorBar") {
            gridParent.Children().RemoveAt(i);
            break;
        }
    }

    // Remove the column and shift other elements back.
    int col = g_syntheticColumn;
    if (col >= 0 && (uint32_t)col < gridParent.ColumnDefinitions().Size())
        gridParent.ColumnDefinitions().RemoveAt((uint32_t)col);

    for (auto child : gridParent.Children()) {
        auto fe = child.try_as<FrameworkElement>();
        if (!fe) continue;
        int c    = Grid::GetColumn(fe);
        int span = Grid::GetColumnSpan(fe);
        if (c > col)
            Grid::SetColumn(fe, c - 1);
        else if (c < col && c + span > col)
            Grid::SetColumnSpan(fe, span - 1);
    }

    g_syntheticGrid = nullptr; g_locIcon = nullptr; g_micIcon = nullptr;
    g_syntheticParent = nullptr; g_syntheticColumn = -1;
    Wh_Log(L"[Remove] PrivacyAnchorBar removed");
}

// ============================================================
// Privacy indicator state tracking
// ============================================================

static void ApplyPrivacyIndicatorBehavior(FrameworkElement iconView) {
    // Avoid double-tracking.
    for (auto& s : g_privacyStates)
        if (s.iconViewRef.get() == iconView) return;

    // Navigate to InnerTextBlock to read the privacy icon character.
    FrameworkElement child = iconView;
    if (!(child = FindChildByName(child, L"ContainerGrid")))    return;
    if (!(child = FindChildByName(child, L"ContentPresenter"))) return;
    if (!(child = FindChildByName(child, L"ContentGrid")))      return;
    child = FindChildByClassName(child, L"SystemTray.TextIconContent");
    if (!child) return;
    if (!(child = FindChildByName(child, L"ContainerGrid")))  return;
    if (!(child = FindChildByName(child, L"Base")))           return;
    if (!(child = FindChildByName(child, L"InnerTextBlock"))) return;

    auto tb = child.try_as<TextBlock>();
    if (!tb) return;

    std::wstring_view text = tb.Text();
    if (!IsPrivacyText(text)) return;  // not a privacy glyph (battery %, volume, etc.)

    PrivacyState::Type type = DetectPrivacyType(text);
    SetPrivacyActive(type, !text.empty());

    PrivacyState state;
    state.iconViewRef  = winrt::make_weak(iconView);
    state.textBlockRef = tb;
    state.type         = type;

    // When text changes, update active state using the state's own type.
    // DetectPrivacyType("") returns Location as a fallback, which is wrong for mic
    // indicators — so when text goes empty we look up the state to get the real type.
    state.textToken = tb.RegisterPropertyChangedCallback(
        TextBlock::TextProperty(),
        [](DependencyObject sender, DependencyProperty) {
            if (g_unloading) return;
            auto tbRef = sender.try_as<TextBlock>();
            if (!tbRef) return;
            std::wstring_view newText = tbRef.Text();
            if (!IsPrivacyText(newText)) return;
            if (newText.empty()) {
                // Indicator went idle — use the state's tracked type, not re-detect.
                for (auto& s : g_privacyStates) {
                    if (s.textBlockRef.get() == tbRef) {
                        SetPrivacyActive(s.type, false);
                        break;
                    }
                }
            } else {
                auto type = DetectPrivacyType(newText);
                // Update the state's type in case it changed (e.g., Location→Mic).
                for (auto& s : g_privacyStates) {
                    if (s.textBlockRef.get() == tbRef) {
                        s.type = type;
                        break;
                    }
                }
                SetPrivacyActive(type, true);
            }
        });

    g_privacyStates.push_back(std::move(state));
    if (g_settings.hideNativeIndicator) {
        // Collapse the element so it takes no layout space (no spacer artefact).
        iconView.Visibility(Visibility::Collapsed);
        iconView.IsHitTestVisible(false);
    }
    Wh_Log(L"[Privacy] Tracking indicator type=%d", (int)type);
}

static void ScanMainStack(FrameworkElement mainStack) {
    int count = 0;
    FindChildRecursive(mainStack, [&count](FrameworkElement fe) -> bool {
        if (winrt::get_class_name(fe) == L"SystemTray.IconView" &&
            fe.Name() == L"SystemTrayIcon") {
            ApplyPrivacyIndicatorBehavior(fe);
            count++;
        }
        return false;
    });
    Wh_Log(L"[Scan] MainStack scan complete, tracked %d icon(s)", count);
}

static void ClearPrivacyStates() {
    for (auto& state : g_privacyStates) {
        if (auto tb = state.textBlockRef.get())
            tb.UnregisterPropertyChangedCallback(TextBlock::TextProperty(), state.textToken);
        if (auto iv = state.iconViewRef.get(); iv && g_settings.hideNativeIndicator) {
            // Restore visibility so the element works normally after mod unloads.
            try { iv.Visibility(Visibility::Visible); } catch (...) {}
        }
    }
    g_privacyStates.clear();
    g_locActive.store(false);
    g_micActive.store(false);
}

// ============================================================
// Apply
// ============================================================

static void ApplyStyle() {
    Wh_Log(L"[Apply] enter");
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) { Wh_Log(L"[Apply] No taskbar window"); return; }
    g_taskbarWnd = hWnd;

    XamlRoot xamlRoot = nullptr;
    try { xamlRoot = GetTaskbarXamlRoot(hWnd); } catch (...) { return; }
    if (!xamlRoot) { Wh_Log(L"[Apply] XamlRoot unavailable"); return; }

    auto root = xamlRoot.Content().try_as<FrameworkElement>();
    if (!root) return;

    // Inject synthetic icons (idempotent).
    if (!g_syntheticGrid)
        InjectSyntheticIcons(root);

    // Scan MainStack for existing real privacy indicators.
    auto sysGrid = FindChildRecursive(root, [](FrameworkElement fe) {
        return fe.Name() == L"SystemTrayFrameGrid";
    });
    if (sysGrid) {
        auto mainStack = FindChildByName(sysGrid, L"MainStack");
        if (mainStack) ScanMainStack(mainStack);
    }
}

static void ApplyStyleOnWindowThread() {
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return;
    RunFromWindowThread(hWnd, [](void*) {
        g_loadedRevokers.clear();
        ClearPrivacyStates();
        ApplyStyle();
    }, nullptr);
}

static void StopRetryThread() {
    if (g_retryStopEvent) SetEvent(g_retryStopEvent);
    if (g_retryThread) { WaitForSingleObject(g_retryThread, 3000); CloseHandle(g_retryThread); }
    if (g_retryStopEvent) CloseHandle(g_retryStopEvent);
    g_retryThread = nullptr; g_retryStopEvent = nullptr;
}

// ============================================================
// Hooks
// ============================================================

using IconView_IconView_t = void* (WINAPI*)(void* pThis);
IconView_IconView_t IconView_IconView_Original;

void* WINAPI IconView_IconView_Hook(void* pThis) {
    void* ret = IconView_IconView_Original(pThis);
    if (g_unloading) return ret;

    FrameworkElement iconView = nullptr;
    ((IUnknown**)pThis)[1]->QueryInterface(winrt::guid_of<FrameworkElement>(),
                                            winrt::put_abi(iconView));
    if (!iconView) return ret;

    g_loadedRevokers.emplace_back();
    auto it = g_loadedRevokers.end(); --it;
    *it = iconView.Loaded(winrt::auto_revoke_t{},
        [it](winrt::Windows::Foundation::IInspectable const& sender, auto const&) {
            g_loadedRevokers.erase(it);
            if (g_unloading) return;
            auto fe = sender.try_as<FrameworkElement>();
            if (!fe) return;
            if (winrt::get_class_name(fe) == L"SystemTray.IconView" &&
                fe.Name() == L"SystemTrayIcon") {
                // Ensure synthetic bar exists.
                if (!g_syntheticGrid) {
                    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
                    if (hWnd) {
                        auto xamlRoot = fe.XamlRoot();
                        if (xamlRoot) {
                            auto root = xamlRoot.Content().try_as<FrameworkElement>();
                            if (root) InjectSyntheticIcons(root);
                        }
                    }
                }
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
            g_taskbarViewDllLoaded = true;
            HookTaskbarViewDllSymbols(h);
            ApplyStyleOnWindowThread();
        }
    }
    return h;
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

static bool HookTaskbarViewDllSymbols(HMODULE h) {
    WindhawkUtils::SYMBOL_HOOK hooks[] = {{
        {LR"(public: __cdecl winrt::SystemTray::implementation::IconView::IconView(void))"},
        &IconView_IconView_Original,
        IconView_IconView_Hook,
        false,
    }};
    return WindhawkUtils::HookSymbols(h, hooks, ARRAYSIZE(hooks));
}

// ============================================================
// Windhawk lifecycle
// ============================================================

BOOL Wh_ModInit() {
    Wh_Log(L"[Init] Privacy Anchor v0.5");
    LoadSettings();

    if (!HookTaskbarDllSymbols())
        Wh_Log(L"[Init] taskbar.dll symbols failed");

    HMODULE kb = GetModuleHandleW(L"kernelbase.dll");
    auto pLLEW = kb ? (LoadLibraryExW_t)GetProcAddress(kb, "LoadLibraryExW") : nullptr;
    if (pLLEW)
        Wh_SetFunctionHook((void*)pLLEW, (void*)LoadLibraryExW_Hook, (void**)&LoadLibraryExW_Original);

    HMODULE tvDll = GetModuleHandleW(L"Taskbar.View.dll");
    if (tvDll) {
        g_taskbarViewDllLoaded = true;
        if (!HookTaskbarViewDllSymbols(tvDll))
            Wh_Log(L"[Init] Taskbar.View.dll hook failed");
    }

    return TRUE;
}

void Wh_ModAfterInit() {
    if (!g_taskbarViewDllLoaded) {
        HMODULE h = GetModuleHandleW(L"Taskbar.View.dll");
        if (h) { g_taskbarViewDllLoaded = true; HookTaskbarViewDllSymbols(h); }
    }

    if (g_taskbarViewDllLoaded)
        ApplyStyleOnWindowThread();

    g_retryStopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!g_retryStopEvent) return;
    HANDLE stopEvent = g_retryStopEvent;
    g_retryThread = CreateThread(nullptr, 0, [](void* param) -> DWORD {
        HANDLE stop = static_cast<HANDLE>(param);
        for (int i = 0; i < 5 && !g_unloading; i++) {
            if (WaitForSingleObject(stop, 2000) != WAIT_TIMEOUT) break;
            if (g_syntheticGrid) break;
            Wh_Log(L"[AfterInit] Retry %d", i + 1);
            ApplyStyleOnWindowThread();
        }
        return 0;
    }, stopEvent, 0, nullptr);
    if (!g_retryThread) { CloseHandle(g_retryStopEvent); g_retryStopEvent = nullptr; }
}

void Wh_ModUninit() {
    g_unloading = true;
    Wh_Log(L"[Uninit]");
    StopRetryThread();
    g_loadedRevokers.clear();
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (hWnd) {
        RunFromWindowThread(hWnd, [](void*) {
            ClearPrivacyStates();
            RemoveSyntheticIcons();
        }, nullptr);
    } else {
        ClearPrivacyStates();
        RemoveSyntheticIcons();
    }
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Wh_Log(L"[Settings] idleOpacity=%d showLoc=%d showMic=%d iconSize=%d layout=%s position=%s spacing=%d bar=(%d,%d) loc=(%d,%d) mic=(%d,%d)",
           g_settings.idleOpacity, g_settings.showLocation ? 1 : 0, g_settings.showMic ? 1 : 0,
           g_settings.iconSize, g_settings.layoutMode.c_str(), g_settings.position.c_str(),
           g_settings.iconSpacing, g_settings.barOffsetX, g_settings.barOffsetY,
           g_settings.locationOffsetX, g_settings.locationOffsetY,
           g_settings.micOffsetX, g_settings.micOffsetY);

    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return;
    RunFromWindowThread(hWnd, [](void*) {
        ClearPrivacyStates();
        RemoveSyntheticIcons();
        ApplyStyle();
    }, nullptr);
}