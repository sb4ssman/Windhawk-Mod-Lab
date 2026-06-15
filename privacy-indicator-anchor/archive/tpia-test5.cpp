// ==WindhawkMod==
// @id              tray-privacy-indicator-anchor
// @name            Tray Privacy Indicator Anchor
// @description     Permanently shows location/microphone/camera/Copilot icons in the system tray — dim when idle, bright when in use — preventing taskbar layout shifts.
// @version         0.8
// @author          sb4ssman
// @github          https://github.com/sb4ssman
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lversion -lsetupapi
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
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Automation.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Shapes.h>

#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <setupapi.h>
#include <tlhelp32.h>

#include <algorithm>
#include <atomic>
#include <cwctype>
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
    int  idleOpacity  = 50;
    std::wstring itemOrder          = L"location,mic,camera";
    int  gridColumns                = 2;
    std::wstring gridFillOrder      = L"rowFirst";
    std::wstring shortGroupPosition = L"last";
    std::wstring shortGroupAlign    = L"center";
    int  iconSize     = 16;
    std::wstring position = L"beforeOmni";
    int  paddingLeft  = 0;
    int  paddingRight = 0;
    int  iconSpacing  = 4;
    int  barOffsetX   = 0;
    int  barOffsetY   = 0;
    int  locationOffsetX = 0;
    int  locationOffsetY = 0;
    int  micOffsetX   = 0;
    int  micOffsetY   = 0;
    int  cameraOffsetX = 0;
    int  cameraOffsetY = 0;
    int  copilotOffsetX = 0;
    int  copilotOffsetY = 0;
    int  activeColorR   = 255;
    int  activeColorG   = 180;
    int  activeColorB   = 60;
    bool glowEnabled    = true;
    int  glowOpacity    = 40;
    int  slashColor     = -1;  // -1 = system theme; otherwise 0x00RRGGBB
    std::wstring slashDirection = L"rising";
    int  slashOpacity   = 100;
};
static ModSettings g_settings;

static std::wstring GetStringSetting(PCWSTR name, PCWSTR fallback) {
    PCWSTR raw = Wh_GetStringSetting(name);
    std::wstring value = raw ? raw : fallback;
    Wh_FreeStringSetting(raw);
    return value;
}

static void LoadSettings() {
    auto clamp = [](int v, int lo, int hi) { return std::max(lo, std::min(hi, v)); };
    g_settings.idleOpacity          = clamp(Wh_GetIntSetting(L"idleOpacity", 50), 0, 100);
    g_settings.itemOrder            = GetStringSetting(L"itemOrder", L"location,mic,camera");
    g_settings.gridColumns          = clamp(Wh_GetIntSetting(L"gridColumns", 2), 1, 10);
    g_settings.gridFillOrder        = GetStringSetting(L"gridFillOrder", L"rowFirst");
    g_settings.shortGroupPosition   = GetStringSetting(L"shortGroupPosition", L"last");
    g_settings.shortGroupAlign      = GetStringSetting(L"shortGroupAlign", L"center");
    g_settings.iconSize             = clamp(Wh_GetIntSetting(L"iconSize", 16), 8, 48);
    g_settings.position             = GetStringSetting(L"position", L"beforeOmni");
    g_settings.paddingLeft          = clamp(Wh_GetIntSetting(L"paddingLeft",  0), -40, 40);
    g_settings.paddingRight         = clamp(Wh_GetIntSetting(L"paddingRight", 0), -40, 40);
    g_settings.iconSpacing          = clamp(Wh_GetIntSetting(L"iconSpacing", 4), 0, 40);
    g_settings.barOffsetX           = clamp(Wh_GetIntSetting(L"barOffsetX", 0), -40, 40);
    g_settings.barOffsetY           = clamp(Wh_GetIntSetting(L"barOffsetY", 0), -40, 40);
    g_settings.locationOffsetX      = clamp(Wh_GetIntSetting(L"locationOffsetX", 0), -40, 40);
    g_settings.locationOffsetY      = clamp(Wh_GetIntSetting(L"locationOffsetY", 0), -40, 40);
    g_settings.micOffsetX           = clamp(Wh_GetIntSetting(L"micOffsetX", 0), -40, 40);
    g_settings.micOffsetY           = clamp(Wh_GetIntSetting(L"micOffsetY", 0), -40, 40);
    g_settings.cameraOffsetX        = clamp(Wh_GetIntSetting(L"cameraOffsetX", 0), -40, 40);
    g_settings.cameraOffsetY        = clamp(Wh_GetIntSetting(L"cameraOffsetY", 0), -40, 40);
    g_settings.copilotOffsetX       = clamp(Wh_GetIntSetting(L"copilotOffsetX", 0), -40, 40);
    g_settings.copilotOffsetY       = clamp(Wh_GetIntSetting(L"copilotOffsetY", 0), -40, 40);
    g_settings.activeColorR         = clamp(Wh_GetIntSetting(L"activeColorR", 255), 0, 255);
    g_settings.activeColorG         = clamp(Wh_GetIntSetting(L"activeColorG", 180), 0, 255);
    g_settings.activeColorB         = clamp(Wh_GetIntSetting(L"activeColorB",  60), 0, 255);
    g_settings.glowEnabled          = Wh_GetIntSetting(L"glowEnabled", 1) != 0;
    g_settings.glowOpacity          = clamp(Wh_GetIntSetting(L"glowOpacity", 40), 0, 100);
    {
        std::wstring hex = GetStringSetting(L"slashColor", L"");
        const wchar_t* p = hex.c_str();
        if (*p == L'#') p++;
        wchar_t* end = nullptr;
        long val = wcstol(p, &end, 16);
        g_settings.slashColor = (end && end != p) ? (int)val : -1;
    }
    g_settings.slashDirection = GetStringSetting(L"slashDirection", L"rising");
    g_settings.slashOpacity   = clamp(Wh_GetIntSetting(L"slashOpacity", 100), 0, 100);
}

// ============================================================
// Globals
// ============================================================

static std::atomic<bool> g_unloading{false};
static HWND              g_taskbarWnd           = nullptr;
static bool              g_taskbarViewDllLoaded = false;
static HANDLE            g_retryThread          = nullptr;
static HANDLE            g_retryStopEvent       = nullptr;

static std::atomic<bool> g_locActive{false};
static std::atomic<bool> g_micActive{false};
static std::atomic<bool> g_camActive{false};
static std::atomic<bool> g_locDisabled{false};
static std::atomic<bool> g_micDisabled{false};
static std::atomic<bool> g_camDisabled{false};
static std::atomic<bool> g_copilotInstalled{false};
static std::atomic<bool> g_copilotActive{false};
static Grid              g_syntheticGrid   = nullptr;
static TextBlock         g_locIcon         = nullptr;
static TextBlock         g_micIcon         = nullptr;
static TextBlock         g_camIcon         = nullptr;
static TextBlock         g_copilotIcon     = nullptr;
static TextBlock         g_locGlowIcon     = nullptr;
static TextBlock         g_micGlowIcon     = nullptr;
static TextBlock         g_camGlowIcon     = nullptr;
static TextBlock         g_copilotGlowIcon = nullptr;
static FrameworkElement  g_locSlashIcon    = nullptr;
static FrameworkElement  g_micSlashIcon    = nullptr;
static FrameworkElement  g_camSlashIcon    = nullptr;
static FrameworkElement  g_copilotSlashIcon = nullptr;
static FrameworkElement  g_syntheticParent = nullptr;
static int               g_syntheticColumn = -1;

struct PrivacyState {
    enum class Type { Location, Mic, Camera, Both };
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
static void UpdateDisabledStates();
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
    size_t offset = 0x10;
#if defined(_M_X64)
    {
        const BYTE* b = (const BYTE*)TaskbarHost_FrameHeight_Original;
        if (b[0]==0x48 && b[1]==0x83 && b[2]==0xEC && b[4]==0x48 &&
            b[5]==0x83 && b[6]==0xC1 && b[7]<=0x7F)
            offset = b[7];
        else
            Wh_Log(L"Unsupported TaskbarHost::FrameHeight");
    }
#elif defined(_M_ARM64)
    {
        const DWORD* p = (const DWORD*)TaskbarHost_FrameHeight_Original;
        if (p[0] == 0xD503237F && (p[1] & 0xFFC07FFF) == 0xA9807BFD &&
            p[2] == 0x910003FD && (p[3] & 0xFFF00FE0) == 0xF8400C00)
            offset = (p[3] >> 12) & 0xFF;
        else
            Wh_Log(L"Unsupported TaskbarHost::FrameHeight");
    }
#else
#error "Unsupported architecture"
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

static PrivacyState::Type DetectPrivacyType(std::wstring_view text) {
    if (text.empty()) return PrivacyState::Type::Location;
    switch (text[0]) {
        case 0xE37A: return PrivacyState::Type::Location;
        case 0xF47F: return PrivacyState::Type::Both;
        case 0xE361:
        case 0xE720:
        case 0xEC71: return PrivacyState::Type::Mic;
        case 0xE722: return PrivacyState::Type::Camera;
        default:     return PrivacyState::Type::Location;
    }
}

static bool IsPrivacyGlyph(wchar_t c) {
    return c == 0xE37A || c == 0xF47F ||
           c == 0xE361 || c == 0xE720 || c == 0xEC71 ||
           c == 0xE722;
}

static bool IsPrivacyText(std::wstring_view text) {
    return text.empty() || (text.length() == 1 && IsPrivacyGlyph(text[0]));
}

// ============================================================
// Icon order parsing
// ============================================================

static std::vector<std::wstring> ParseItemOrder(std::wstring const& s) {
    std::vector<std::wstring> result;
    size_t start = 0;
    while (start <= s.size()) {
        size_t end = s.find(L',', start);
        if (end == std::wstring::npos) end = s.size();
        size_t ts = start, te = end;
        while (ts < te && std::iswspace(s[ts])) ts++;
        while (te > ts && std::iswspace(s[te - 1])) te--;
        if (ts < te) {
            std::wstring token = s.substr(ts, te - ts);
            for (auto& c : token) c = std::towlower(c);
            if (token == L"location" || token == L"mic" || token == L"camera" || token == L"copilot") {
                // Deduplicate
                bool already = false;
                for (auto& r : result) if (r == token) { already = true; break; }
                if (!already) result.push_back(token);
            }
        }
        if (end == s.size()) break;
        start = end + 1;
    }
    return result;
}

// ============================================================
// Grid placement helper (Option C)
// ============================================================

struct GridPlacement {
    int row, col, rowSpan, colSpan;
    HorizontalAlignment hAlign;
    VerticalAlignment   vAlign;
};

static GridPlacement ComputeIconPlacement(
    int i, int N, int cols,
    bool colFirst, bool shortFirst,
    const std::wstring& align)
{
    GridPlacement p;
    p.rowSpan = 1; p.colSpan = 1;
    p.hAlign  = HorizontalAlignment::Stretch;
    p.vAlign  = VerticalAlignment::Stretch;

    int rows = (N + cols - 1) / cols;

    if (!colFirst) {
        // Row-first: fill left-to-right, then down.
        int remainder = N % cols;  // items in the short row (0 = no short row)
        bool hasShort = remainder > 0;

        if (!shortFirst) {
            // Short row at end (default).
            p.row = i / cols;
            p.col = i % cols;
            bool inShortRow = hasShort && (p.row == rows - 1);
            if (inShortRow) {
                if (remainder == 1) {
                    if (align == L"center") { p.colSpan = cols; p.hAlign = HorizontalAlignment::Center; }
                    else if (align == L"end") p.col = cols - 1;
                }
                // Multi-item short row: leave as-is (start alignment).
            }
        } else {
            // Short row at start.
            if (hasShort && i < remainder) {
                p.row = 0;
                p.col = i;
                if (remainder == 1) {
                    if (align == L"center") { p.colSpan = cols; p.hAlign = HorizontalAlignment::Center; }
                    else if (align == L"end") p.col = cols - 1;
                }
            } else {
                int j = hasShort ? i - remainder : i;
                p.row = (hasShort ? 1 : 0) + j / cols;
                p.col = j % cols;
            }
        }
    } else {
        // Col-first: fill top-to-bottom, then right.
        // Each full column gets `rows` items. The short column has (N % rows) items.
        int itemsPerFullCol = rows;
        int fullCols        = N / itemsPerFullCol;
        int remainder       = N % itemsPerFullCol;  // items in short col (0 = no short col)
        bool hasShort       = remainder > 0;

        if (!shortFirst) {
            // Short col at end (default).
            if (i < fullCols * itemsPerFullCol) {
                p.col = i / itemsPerFullCol;
                p.row = i % itemsPerFullCol;
            } else {
                int j = i - fullCols * itemsPerFullCol;
                p.col = fullCols;
                p.row = j;
                if (remainder == 1) {
                    if (align == L"center") { p.rowSpan = rows; p.vAlign = VerticalAlignment::Center; }
                    else if (align == L"end") p.row = rows - 1;
                }
            }
        } else {
            // Short col at start.
            if (hasShort && i < remainder) {
                p.col = 0;
                p.row = i;
                if (remainder == 1) {
                    if (align == L"center") { p.rowSpan = rows; p.vAlign = VerticalAlignment::Center; }
                    else if (align == L"end") p.row = rows - 1;
                }
            } else {
                int j = hasShort ? i - remainder : i;
                p.col = (hasShort ? 1 : 0) + j / itemsPerFullCol;
                p.row = j % itemsPerFullCol;
            }
        }
    }

    return p;
}

// ============================================================
// Synthetic icon management
// ============================================================

static void UpdateSyntheticOpacity() {
    double idleOp = g_settings.idleOpacity / 100.0;
    winrt::Windows::UI::Color activeColor{255,
        (BYTE)g_settings.activeColorR,
        (BYTE)g_settings.activeColorG,
        (BYTE)g_settings.activeColorB};

    // Apply full visual state to one icon slot.
    // active   = feature in use  → colored foreground + glow
    // disabled = hardware off    → neutral foreground + slash, no glow
    // idle     = neither         → system-theme foreground, reduced opacity, no glow
    auto applySlot = [&](TextBlock icon, TextBlock glow, FrameworkElement slash,
                         bool active, bool disabled) {
        if (!icon) return;
        icon.Opacity(active ? 1.0 : idleOp);

        // Active: user's color. Idle/disabled: restore system theme (no explicit brush).
        if (active) {
            SolidColorBrush brush;
            brush.Color(activeColor);
            icon.Foreground(brush);
        } else {
            icon.ClearValue(TextBlock::ForegroundProperty());
        }

        // Glow: visible only when active
        if (glow) {
            bool show = active && g_settings.glowEnabled;
            glow.Visibility(show ? Visibility::Visible : Visibility::Collapsed);
            if (show) {
                // Refresh glow color in case settings changed
                if (auto b = glow.Foreground().try_as<SolidColorBrush>()) b.Color(activeColor);
            }
        }

        if (slash) {
            slash.Visibility(disabled ? Visibility::Visible : Visibility::Collapsed);
            // Sync Line stroke to the icon's computed foreground so slash matches icon color.
            // Only when no explicit user color — slashColor >= 0 was baked in at inject time.
            if (disabled && g_settings.slashColor < 0) {
                if (auto line = slash.try_as<winrt::Windows::UI::Xaml::Shapes::Line>()) {
                    line.Stroke(icon.Foreground());
                }
            }
        }
    };

    applySlot(g_locIcon, g_locGlowIcon, g_locSlashIcon,
              g_locActive.load(), g_locDisabled.load());
    applySlot(g_micIcon, g_micGlowIcon, g_micSlashIcon,
              g_micActive.load(), g_micDisabled.load());
    applySlot(g_camIcon, g_camGlowIcon, g_camSlashIcon,
              g_camActive.load(), g_camDisabled.load());
    applySlot(g_copilotIcon, g_copilotGlowIcon, g_copilotSlashIcon,
              g_copilotActive.load(), !g_copilotInstalled.load());
}

static void SetIconTooltip(TextBlock const& tb, PCWSTR label, bool active, bool disabled,
                           PCWSTR idleLabel     = L"Not requested",
                           PCWSTR disabledLabel = L"Hardware disabled") {
    if (!tb) return;
    const wchar_t* state = disabled ? disabledLabel
                         : active  ? L"In use"
                                   : idleLabel;
    winrt::hstring tooltip = winrt::hstring(label) + L":\n" + state;
    ToolTipService::SetToolTip(tb, winrt::box_value(tooltip));
    winrt::Windows::UI::Xaml::Automation::AutomationProperties::SetName(
        tb, winrt::hstring(label) + L": " + state);
}

static void UpdateSyntheticTooltips() {
    if (g_locIcon) SetIconTooltip(g_locIcon, L"Location",   g_locActive.load(), g_locDisabled.load());
    if (g_micIcon) SetIconTooltip(g_micIcon, L"Microphone", g_micActive.load(), g_micDisabled.load());
    if (g_camIcon) SetIconTooltip(g_camIcon, L"Camera",     g_camActive.load(), g_camDisabled.load());
    if (g_copilotIcon) SetIconTooltip(g_copilotIcon, L"Copilot",
        g_copilotActive.load(), !g_copilotInstalled.load(),
        L"Installed (not running)", L"Not installed");
}

static void UpdateSyntheticState() {
    UpdateSyntheticOpacity();
    UpdateSyntheticTooltips();
}

static void SetPrivacyActive(PrivacyState::Type type, bool active) {
    switch (type) {
        case PrivacyState::Type::Location: g_locActive.store(active); break;
        case PrivacyState::Type::Mic:      g_micActive.store(active); break;
        case PrivacyState::Type::Camera:   g_camActive.store(active); break;
        case PrivacyState::Type::Both:
            g_locActive.store(active);
            g_micActive.store(active);
            break;
    }
    UpdateSyntheticState();
}

// ============================================================
// Hardware-disabled detection (called from background thread, COM must be initialized)
// ============================================================

// Returns true if the default audio capture device appears hardware-disabled.
// Covers two mechanisms used by different laptops:
//   1. DEVICE_STATE_DISABLED / DEVICE_STATE_NOTPRESENT  (Device Manager / EC kill)
//   2. IAudioEndpointVolume master mute  (Fn-key soft mute on many ThinkPads / HPs)
static bool CheckMicDisabled() {
    IMMDeviceEnumerator* pEnum = nullptr;
    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                IID_PPV_ARGS(&pEnum))))
        return false;
    IMMDevice* pDev = nullptr;
    HRESULT hr = pEnum->GetDefaultAudioEndpoint(eCapture, eConsole, &pDev);
    pEnum->Release();
    if (FAILED(hr)) return false;  // E_NOTFOUND = no capture device at all

    bool disabled = false;
    DWORD state = 0;
    if (SUCCEEDED(pDev->GetState(&state))) {
        disabled = (state == DEVICE_STATE_DISABLED || state == DEVICE_STATE_NOTPRESENT);
    }
    if (!disabled) {
        // Check endpoint master mute (Fn-key path on many laptops)
        IAudioEndpointVolume* pVol = nullptr;
        if (SUCCEEDED(pDev->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr,
                                     reinterpret_cast<void**>(&pVol)))) {
            BOOL muted = FALSE;
            if (SUCCEEDED(pVol->GetMute(&muted))) disabled = (muted == TRUE);
            pVol->Release();
        }
    }
    pDev->Release();
    return disabled;
}

// Returns true if a camera device exists in the Windows device database but none is
// currently accessible — i.e. the physical kill switch has cut power to the camera.
static bool CheckCameraDisabled() {
    static const GUID GUID_DEVCLASS_CAMERA_LOCAL =
        {0xca3e7ab9, 0xb4c3, 0x4ae6, {0x82, 0x51, 0x57, 0x9e, 0xf9, 0x33, 0x89, 0x0f}};
    // Check if any camera device is registered in the system at all
    HDEVINFO allDevs = SetupDiGetClassDevs(&GUID_DEVCLASS_CAMERA_LOCAL, nullptr, nullptr, 0);
    if (allDevs == INVALID_HANDLE_VALUE) return false;
    SP_DEVINFO_DATA d{}; d.cbSize = sizeof(d);
    bool hasAny = SetupDiEnumDeviceInfo(allDevs, 0, &d) == TRUE;
    SetupDiDestroyDeviceInfoList(allDevs);
    if (!hasAny) return false; // no camera hardware in this machine
    // Check if any camera is present (powered on)
    HDEVINFO presentDevs = SetupDiGetClassDevs(&GUID_DEVCLASS_CAMERA_LOCAL, nullptr, nullptr, DIGCF_PRESENT);
    bool hasPresent = false;
    if (presentDevs != INVALID_HANDLE_VALUE) {
        SP_DEVINFO_DATA pd{}; pd.cbSize = sizeof(pd);
        hasPresent = SetupDiEnumDeviceInfo(presentDevs, 0, &pd) == TRUE;
        SetupDiDestroyDeviceInfoList(presentDevs);
    }
    if (!hasPresent) return true; // device known but not present = kill switch off
    // Check: app-level consent denied (Settings → Privacy → Camera toggle off)
    {
        HKEY hk = nullptr;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager"
                L"\\ConsentStore\\webcam",
                0, KEY_READ, &hk) == ERROR_SUCCESS) {
            wchar_t val[16] = {};
            DWORD valLen = sizeof(val), type = 0;
            LONG r = RegQueryValueExW(hk, L"Value", nullptr, &type, (LPBYTE)val, &valLen);
            RegCloseKey(hk);
            if (r == ERROR_SUCCESS && type == REG_SZ && _wcsicmp(val, L"Deny") == 0)
                return true;
        }
    }
    return false;
}

// Returns true if the Windows Web Experience Pack (Copilot) AppX package is registered in
// the AppModel repository for the current user or machine. The package data directory in
// %LOCALAPPDATA%\Packages is NOT used — that directory survives uninstall (it holds user
// data) and would give a false positive after removal.
static bool CheckCopilotInstalled() {
    // Sub-keys under Repository\Packages are named <PackageFullName> e.g.
    //   MicrosoftWindows.Client.WebExperience_<ver>_x64__<pub>
    // We enumerate and look for any key whose name starts with the package family prefix.
    static const wchar_t kPrefix[]    = L"MicrosoftWindows.Client.WebExperience_";
    static const DWORD   kPrefixLen   = (DWORD)(ARRAYSIZE(kPrefix) - 1);
    static const wchar_t* const kRoots[] = {
        L"Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion"
            L"\\AppModel\\Repository\\Packages",                     // per-user (HKCU)
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion"
            L"\\AppModel\\Repository\\Packages",                     // machine-wide (HKLM)
    };
    static const HKEY kHives[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };
    for (int h = 0; h < 2; h++) {
        HKEY hPkg = nullptr;
        if (RegOpenKeyExW(kHives[h], kRoots[h], 0,
                          KEY_READ | KEY_ENUMERATE_SUB_KEYS, &hPkg) != ERROR_SUCCESS)
            continue;
        wchar_t name[256]; DWORD nameLen;
        bool found = false;
        for (DWORD i = 0; !found; i++) {
            nameLen = ARRAYSIZE(name);
            LONG e = RegEnumKeyExW(hPkg, i, name, &nameLen,
                                   nullptr, nullptr, nullptr, nullptr);
            if (e == ERROR_NO_MORE_ITEMS) break;
            if (e != ERROR_SUCCESS)       continue;
            if (wcsncmp(name, kPrefix, kPrefixLen) == 0) found = true;
        }
        RegCloseKey(hPkg);
        if (found) return true;
    }
    return false;
}

// Returns true if a Copilot-related process is currently running.
static bool CheckCopilotActive() {
    static const wchar_t* const exes[] = {
        L"Copilot.exe", L"AIHost.exe", L"copilotwindows.exe", L"Microsoft.Copilot.exe"
    };
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return false;
    PROCESSENTRY32W pe{}; pe.dwSize = sizeof(pe);
    bool found = false;
    if (Process32FirstW(snap, &pe)) {
        do {
            for (const auto* exe : exes) {
                if (_wcsicmp(pe.szExeFile, exe) == 0) { found = true; break; }
            }
        } while (!found && Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return found;
}

// Returns true if location is disabled at either the OS service level or the app consent level.
static bool CheckLocationDisabled() {
    // Check 1: Geolocation service master switch (Settings → Privacy → Location services)
    // Status DWORD = 0 means the service is turned off system-wide.
    {
        HKEY hk = nullptr;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                L"SYSTEM\\CurrentControlSet\\Services\\lfsvc\\Service\\Configuration",
                0, KEY_READ, &hk) == ERROR_SUCCESS) {
            DWORD status = 0xFFFFFFFF, valLen = sizeof(status);
            bool off = (RegQueryValueExW(hk, L"Status", nullptr, nullptr,
                                         (LPBYTE)&status, &valLen) == ERROR_SUCCESS
                        && status == 0);
            RegCloseKey(hk);
            if (off) return true;
        }
    }
    // Check 2: App-level consent denied (Settings → Privacy → Let apps access your location)
    {
        HKEY hk = nullptr;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager"
                L"\\ConsentStore\\location",
                0, KEY_READ, &hk) != ERROR_SUCCESS)
            return false;
        wchar_t val[16] = {};
        DWORD valLen = sizeof(val), type = 0;
        LONG r = RegQueryValueExW(hk, L"Value", nullptr, &type, (LPBYTE)val, &valLen);
        RegCloseKey(hk);
        return r == ERROR_SUCCESS && type == REG_SZ && _wcsicmp(val, L"Deny") == 0;
    }
}

// Poll all privacy states and push UI update if anything changed.
// Must be called from a thread with COM initialized (COINIT_MULTITHREADED).
static void UpdateDisabledStates() {
    if (g_unloading) return;
    bool locDis  = CheckLocationDisabled();
    bool micDis  = CheckMicDisabled();
    bool camDis  = CheckCameraDisabled();
    bool copInst = CheckCopilotInstalled();
    bool copAct  = CheckCopilotActive();
    bool prevLoc  = g_locDisabled.load();
    bool prevMic  = g_micDisabled.load(),  prevCam  = g_camDisabled.load();
    bool prevCopI = g_copilotInstalled.load(), prevCopA = g_copilotActive.load();
    bool changed = (g_locDisabled.exchange(locDis)           != locDis)  ||
                   (g_micDisabled.exchange(micDis)           != micDis)  ||
                   (g_camDisabled.exchange(camDis)           != camDis)  ||
                   (g_copilotInstalled.exchange(copInst)     != copInst) ||
                   (g_copilotActive.exchange(copAct)         != copAct);
    if (changed)
        Wh_Log(L"[Poll] loc %d->%d  mic %d->%d  cam %d->%d  copInst %d->%d  copAct %d->%d",
               prevLoc, locDis, prevMic, micDis, prevCam, camDis, prevCopI, copInst, prevCopA, copAct);
    if (changed && !g_unloading && g_taskbarWnd) {
        RunFromWindowThread(g_taskbarWnd, [](void*) {
            if (!g_unloading) UpdateSyntheticState();
        }, nullptr);
    }
}

static TextBlock MakeIconTextBlock(const wchar_t* glyph) {
    TextBlock tb;
    tb.Text(glyph);
    tb.FontFamily(FontFamily(L"Segoe MDL2 Assets"));
    tb.FontSize((double)g_settings.iconSize);
    tb.VerticalAlignment(VerticalAlignment::Center);
    tb.HorizontalAlignment(HorizontalAlignment::Center);
    tb.TextWrapping(TextWrapping::NoWrap);
    return tb;
}

static void ApplyOffset(FrameworkElement const& fe, int x, int y) {
    if (!fe) return;
    if (x != 0 || y != 0) {
        TranslateTransform tt;
        tt.X((double)x); tt.Y((double)y);
        fe.RenderTransform(tt);
    } else {
        fe.ClearValue(UIElement::RenderTransformProperty());
    }
}

static bool InjectSyntheticIcons(FrameworkElement root) {
    auto gridElem = FindChildRecursive(root, [](FrameworkElement fe) {
        return fe.Name() == L"SystemTrayFrameGrid";
    });
    if (!gridElem) { Wh_Log(L"[Inject] SystemTrayFrameGrid not found"); return false; }
    auto gridParent = gridElem.try_as<Grid>();
    if (!gridParent) { Wh_Log(L"[Inject] SystemTrayFrameGrid not a Grid"); return false; }

    // Idempotent check.
    for (auto child : gridParent.Children()) {
        if (auto fe = child.try_as<FrameworkElement>(); fe && fe.Name() == L"PrivacyAnchorBar")
            return true;
    }

    auto activeItems = ParseItemOrder(g_settings.itemOrder);
    if (activeItems.empty()) {
        Wh_Log(L"[Inject] itemOrder has no valid tokens");
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
    if (insertAfterRef && refElem)  insertCol = Grid::GetColumn(refElem) + 1;
    else if (refElem)               insertCol = Grid::GetColumn(refElem);

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
        if (col >= insertCol)             Grid::SetColumn(fe, col + 1);
        else if (col + span > insertCol)  Grid::SetColumnSpan(fe, span + 1);
    }

    // ── Build the anchor bar ────────────────────────────────────
    Grid bar;
    bar.Name(L"PrivacyAnchorBar");
    bar.VerticalAlignment(VerticalAlignment::Center);
    bar.HorizontalAlignment(HorizontalAlignment::Center);
    bar.Margin({ (double)g_settings.paddingLeft, 0.0, (double)g_settings.paddingRight, 0.0 });
    ApplyOffset(bar, g_settings.barOffsetX, g_settings.barOffsetY);

    int N    = (int)activeItems.size();
    int cols = std::max(1, std::min(N, g_settings.gridColumns));
    int rows = (N + cols - 1) / cols;
    bool colFirst   = (g_settings.gridFillOrder      == L"colFirst");
    bool shortFirst = (g_settings.shortGroupPosition == L"first");
    const auto& align = g_settings.shortGroupAlign;

    for (int r = 0; r < rows; r++) {
        RowDefinition rd;
        rd.Height({ 1.0, GridUnitType::Auto });
        bar.RowDefinitions().Append(rd);
    }
    for (int c = 0; c < cols; c++) {
        ColumnDefinition bcd;
        bcd.Width({ 1.0, GridUnitType::Auto });
        bar.ColumnDefinitions().Append(bcd);
    }
    if (g_settings.iconSpacing > 0) {
        bar.ColumnSpacing((double)g_settings.iconSpacing);
        bar.RowSpacing((double)g_settings.iconSpacing);
    }

    g_locIcon = nullptr; g_micIcon = nullptr; g_camIcon = nullptr; g_copilotIcon = nullptr;
    g_locGlowIcon = nullptr; g_micGlowIcon = nullptr; g_camGlowIcon = nullptr; g_copilotGlowIcon = nullptr;
    g_locSlashIcon = nullptr; g_micSlashIcon = nullptr; g_camSlashIcon = nullptr; g_copilotSlashIcon = nullptr;

    for (int i = 0; i < N; i++) {
        const auto& token = activeItems[i];
        const wchar_t* glyph;
        const wchar_t* iconFont      = L"Segoe MDL2 Assets";
        bool  isActive, isDisabled;
        int   offX, offY;
        const wchar_t* label;
        const wchar_t* idleLabel     = L"Not requested";
        const wchar_t* disabledLabel = L"Hardware disabled";

        if (token == L"location") {
            glyph      = L"\xE37A";
            isActive   = g_locActive.load();
            isDisabled = g_locDisabled.load();
            offX       = g_settings.locationOffsetX;
            offY       = g_settings.locationOffsetY;
            label      = L"Location";
        } else if (token == L"mic") {
            glyph      = L"\xE720";
            isActive   = g_micActive.load();
            isDisabled = g_micDisabled.load();
            offX       = g_settings.micOffsetX;
            offY       = g_settings.micOffsetY;
            label      = L"Microphone";
        } else if (token == L"camera") {
            glyph      = L"\xE722";
            isActive   = g_camActive.load();
            isDisabled = g_camDisabled.load();
            offX       = g_settings.cameraOffsetX;
            offY       = g_settings.cameraOffsetY;
            label      = L"Camera";
        } else {  // copilot
            glyph        = L"\xF6FA";  // Segoe Fluent Icons: globe (best available Copilot placeholder — real glyph TBD)
            iconFont     = L"Segoe Fluent Icons";
            isActive     = g_copilotActive.load();
            isDisabled   = !g_copilotInstalled.load();  // slash = not installed
            offX         = g_settings.copilotOffsetX;
            offY         = g_settings.copilotOffsetY;
            label        = L"Copilot";
            idleLabel    = L"Installed (not running)";
            disabledLabel = L"Not installed";
        }

        // Wrap glow + icon + slash overlay in a 1-cell Grid so they overlap (back to front)
        Grid slot;
        slot.HorizontalAlignment(HorizontalAlignment::Center);
        slot.VerticalAlignment(VerticalAlignment::Center);

        // Glow layer — same glyph at 1.5× size, active color; behind the main icon
        TextBlock glowTb;
        glowTb.Text(glyph);
        glowTb.FontFamily(FontFamily(iconFont));
        glowTb.FontSize((double)g_settings.iconSize * 1.5);
        glowTb.HorizontalAlignment(HorizontalAlignment::Center);
        glowTb.VerticalAlignment(VerticalAlignment::Center);
        glowTb.IsHitTestVisible(false);
        glowTb.Opacity(g_settings.glowOpacity / 100.0);
        {
            SolidColorBrush glowBrush;
            winrt::Windows::UI::Color glowColor{255,
                (BYTE)g_settings.activeColorR,
                (BYTE)g_settings.activeColorG,
                (BYTE)g_settings.activeColorB};
            glowBrush.Color(glowColor);
            glowTb.Foreground(glowBrush);
        }
        glowTb.Visibility(Visibility::Collapsed);
        slot.Children().Append(glowTb);

        auto tb = MakeIconTextBlock(glyph);
        tb.FontFamily(FontFamily(iconFont));
        SetIconTooltip(tb, label, isActive, isDisabled, idleLabel, disabledLabel);
        slot.Children().Append(tb);

        // Slash overlay — diagonal line across the icon, direction from settings
        double sz = (double)g_settings.iconSize;
        bool falling = (g_settings.slashDirection == L"falling");
        winrt::Windows::UI::Xaml::Shapes::Line slashLine;
        if (falling) {
            slashLine.X1(sz * 0.1);  slashLine.Y1(sz * 0.1);  // top-left
            slashLine.X2(sz * 0.9);  slashLine.Y2(sz * 0.9);  // bottom-right
        } else {
            slashLine.X1(sz * 0.1);  slashLine.Y1(sz * 0.9);  // bottom-left
            slashLine.X2(sz * 0.9);  slashLine.Y2(sz * 0.1);  // top-right
        }
        slashLine.Width(sz);
        slashLine.Height(sz);
        slashLine.Opacity(g_settings.slashOpacity / 100.0);
        slashLine.StrokeThickness(std::max(1.5, sz * 0.09));
        slashLine.StrokeStartLineCap(PenLineCap::Round);
        slashLine.StrokeEndLineCap(PenLineCap::Round);
        slashLine.HorizontalAlignment(HorizontalAlignment::Center);
        slashLine.VerticalAlignment(VerticalAlignment::Center);
        slashLine.IsHitTestVisible(false);
        {
            SolidColorBrush slashBrush;
            if (g_settings.slashColor >= 0) {
                slashBrush.Color({255,
                    (BYTE)((g_settings.slashColor >> 16) & 0xFF),
                    (BYTE)((g_settings.slashColor >>  8) & 0xFF),
                    (BYTE)( g_settings.slashColor        & 0xFF)});
            } else {
                // Placeholder color; applySlot syncs this to icon foreground when shown.
                slashBrush.Color({255, 255, 255, 255});
            }
            slashLine.Stroke(slashBrush);
        }
        slashLine.Visibility(isDisabled ? Visibility::Visible : Visibility::Collapsed);
        slot.Children().Append(slashLine);

        ApplyOffset(slot, offX, offY);

        auto placement = ComputeIconPlacement(i, N, cols, colFirst, shortFirst, align);
        Grid::SetRow(slot, placement.row);
        Grid::SetColumn(slot, placement.col);
        if (placement.rowSpan > 1) Grid::SetRowSpan(slot, placement.rowSpan);
        if (placement.colSpan > 1) Grid::SetColumnSpan(slot, placement.colSpan);
        if (placement.hAlign != HorizontalAlignment::Stretch) slot.HorizontalAlignment(placement.hAlign);
        if (placement.vAlign != VerticalAlignment::Stretch)   slot.VerticalAlignment(placement.vAlign);

        if (token == L"location")      { g_locIcon     = tb; g_locGlowIcon     = glowTb; g_locSlashIcon     = slashLine; }
        else if (token == L"mic")      { g_micIcon     = tb; g_micGlowIcon     = glowTb; g_micSlashIcon     = slashLine; }
        else if (token == L"camera")   { g_camIcon     = tb; g_camGlowIcon     = glowTb; g_camSlashIcon     = slashLine; }
        else /* copilot */             { g_copilotIcon = tb; g_copilotGlowIcon = glowTb; g_copilotSlashIcon = slashLine; }

        bar.Children().Append(slot);
    }

    Grid::SetColumn(bar, insertCol);
    gridParent.Children().Append(bar);

    g_syntheticGrid   = bar;
    g_syntheticParent = gridElem;
    g_syntheticColumn = insertCol;

    UpdateSyntheticState();
    Wh_Log(L"[Inject] PrivacyAnchorBar: %d icons, %d cols, %d rows, colFirst=%d, shortFirst=%d",
           N, cols, rows, colFirst ? 1 : 0, shortFirst ? 1 : 0);
    return true;
}

static void RemoveSyntheticIcons() {
    auto gridParent = g_syntheticParent ? g_syntheticParent.try_as<Grid>() : nullptr;
    if (!gridParent) {
        g_syntheticGrid    = nullptr;
        g_locIcon = nullptr; g_micIcon = nullptr; g_camIcon = nullptr; g_copilotIcon = nullptr;
        g_locGlowIcon = nullptr; g_micGlowIcon = nullptr; g_camGlowIcon = nullptr; g_copilotGlowIcon = nullptr;
        g_locSlashIcon = nullptr; g_micSlashIcon = nullptr; g_camSlashIcon = nullptr; g_copilotSlashIcon = nullptr;
        g_syntheticParent  = nullptr; g_syntheticColumn = -1;
        return;
    }

    for (uint32_t i = 0; i < gridParent.Children().Size(); i++) {
        auto fe = gridParent.Children().GetAt(i).try_as<FrameworkElement>();
        if (fe && fe.Name() == L"PrivacyAnchorBar") {
            gridParent.Children().RemoveAt(i);
            break;
        }
    }

    int col = g_syntheticColumn;
    if (col >= 0 && (uint32_t)col < gridParent.ColumnDefinitions().Size())
        gridParent.ColumnDefinitions().RemoveAt((uint32_t)col);

    for (auto child : gridParent.Children()) {
        auto fe = child.try_as<FrameworkElement>();
        if (!fe) continue;
        int c    = Grid::GetColumn(fe);
        int span = Grid::GetColumnSpan(fe);
        if (c > col)                     Grid::SetColumn(fe, c - 1);
        else if (c < col && c + span > col) Grid::SetColumnSpan(fe, span - 1);
    }

    g_syntheticGrid    = nullptr;
    g_locIcon = nullptr; g_micIcon = nullptr; g_camIcon = nullptr; g_copilotIcon = nullptr;
    g_locGlowIcon = nullptr; g_micGlowIcon = nullptr; g_camGlowIcon = nullptr; g_copilotGlowIcon = nullptr;
    g_locSlashIcon = nullptr; g_micSlashIcon = nullptr; g_camSlashIcon = nullptr; g_copilotSlashIcon = nullptr;
    g_syntheticParent  = nullptr; g_syntheticColumn = -1;
    Wh_Log(L"[Remove] PrivacyAnchorBar removed");
}

// ============================================================
// Privacy indicator state tracking
// ============================================================

static void ApplyPrivacyIndicatorBehavior(FrameworkElement iconView) {
    for (auto& s : g_privacyStates)
        if (s.iconViewRef.get() == iconView) return;

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

    // Log unknown glyphs so new privacy indicator types can be identified during testing.
    if (!text.empty() && text.length() == 1 && !IsPrivacyGlyph(text[0])) {
        Wh_Log(L"[Privacy] Unknown MainStack glyph U+%04X — not a tracked privacy type", (unsigned)text[0]);
        return;
    }
    if (!IsPrivacyText(text)) return;

    PrivacyState::Type type = DetectPrivacyType(text);
    SetPrivacyActive(type, !text.empty());

    PrivacyState state;
    state.iconViewRef  = winrt::make_weak(iconView);
    state.textBlockRef = tb;
    state.type         = type;

    state.textToken = tb.RegisterPropertyChangedCallback(
        TextBlock::TextProperty(),
        [](DependencyObject sender, DependencyProperty) {
            if (g_unloading) return;
            auto tbRef = sender.try_as<TextBlock>();
            if (!tbRef) return;
            std::wstring_view newText = tbRef.Text();
            if (!newText.empty() && newText.length() == 1 && !IsPrivacyGlyph(newText[0])) {
                Wh_Log(L"[Privacy] Unknown glyph change: U+%04X", (unsigned)newText[0]);
                return;
            }
            if (!IsPrivacyText(newText)) return;
            if (newText.empty()) {
                for (auto& s : g_privacyStates) {
                    if (s.textBlockRef.get() == tbRef) {
                        SetPrivacyActive(s.type, false);
                        break;
                    }
                }
            } else {
                auto detectedType = DetectPrivacyType(newText);
                for (auto& s : g_privacyStates) {
                    if (s.textBlockRef.get() == tbRef) {
                        s.type = detectedType;
                        break;
                    }
                }
                SetPrivacyActive(detectedType, true);
            }
        });

    g_privacyStates.push_back(std::move(state));
    iconView.Visibility(Visibility::Collapsed);
    iconView.IsHitTestVisible(false);
    Wh_Log(L"[Privacy] Tracking indicator type=%d", (int)type);
}

static void ScanMainStack(FrameworkElement mainStack) {
    int count = 0;
    FindChildRecursive(mainStack, [&count](FrameworkElement fe) -> bool {
        if (winrt::get_class_name(fe) != L"SystemTray.IconView") return false;
        const std::wstring name = std::wstring(fe.Name());
        if (name != L"SystemTrayIcon") {
            // Log non-standard names — may correspond to screen capture, presence sensing, etc.
            Wh_Log(L"[Scan] Non-standard IconView name: %s", name.c_str());
            return false;
        }
        ApplyPrivacyIndicatorBehavior(fe);
        count++;
        return false;
    });
    Wh_Log(L"[Scan] MainStack scan complete, tracked %d icon(s)", count);
}

static void ClearPrivacyStates() {
    for (auto& state : g_privacyStates) {
        if (auto tb = state.textBlockRef.get())
            tb.UnregisterPropertyChangedCallback(TextBlock::TextProperty(), state.textToken);
        if (auto iv = state.iconViewRef.get())
            try { iv.Visibility(Visibility::Visible); } catch (...) {}
    }
    g_privacyStates.clear();
    g_locActive.store(false);
    g_micActive.store(false);
    g_camActive.store(false);
    g_locDisabled.store(false);
    g_micDisabled.store(false);
    g_camDisabled.store(false);
    g_copilotInstalled.store(false);
    g_copilotActive.store(false);
    UpdateSyntheticState();
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

    if (!g_syntheticGrid) InjectSyntheticIcons(root);

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
                if (!g_syntheticGrid) {
                    auto xamlRoot = fe.XamlRoot();
                    if (xamlRoot) {
                        auto root = xamlRoot.Content().try_as<FrameworkElement>();
                        if (root) InjectSyntheticIcons(root);
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
    Wh_Log(L"[Init] Privacy Anchor v0.8");
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
        // Phase 1: retry injection up to 5×
        for (int i = 0; i < 5 && !g_unloading; i++) {
            if (WaitForSingleObject(stop, 2000) != WAIT_TIMEOUT) return 0;
            if (g_syntheticGrid) break;
            Wh_Log(L"[AfterInit] Retry %d", i + 1);
            ApplyStyleOnWindowThread();
        }
        // Phase 2: poll hardware-disabled states every 3 seconds
        if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) return 0;
        Wh_Log(L"[Poll] Phase 2 starting — initial hardware state check");
        UpdateDisabledStates();
        Wh_Log(L"[Poll] Baseline: loc=%d mic=%d cam=%d copInst=%d copAct=%d",
               g_locDisabled.load(), g_micDisabled.load(), g_camDisabled.load(),
               g_copilotInstalled.load(), g_copilotActive.load());
        while (!g_unloading) {
            if (WaitForSingleObject(stop, 3000) != WAIT_TIMEOUT) break;
            UpdateDisabledStates();
        }
        CoUninitialize();
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
    Wh_Log(L"[Settings] order=%s cols=%d fill=%s shortPos=%s shortAlign=%s",
           g_settings.itemOrder.c_str(), g_settings.gridColumns,
           g_settings.gridFillOrder.c_str(), g_settings.shortGroupPosition.c_str(),
           g_settings.shortGroupAlign.c_str());

    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return;
    RunFromWindowThread(hWnd, [](void*) {
        ClearPrivacyStates();
        RemoveSyntheticIcons();
        ApplyStyle();
    }, nullptr);
}