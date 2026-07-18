// ==WindhawkMod==
// @id              tray-privacy-indicator-anchor3
// @name            Tray Privacy Indicator Anchor3
// @description     Permanently shows location/microphone/camera/Copilot icons in the system tray — dim when idle, bright when in use — preventing taskbar layout shifts.
// @version         0.9
// @author          sb4ssman
// @github          https://github.com/sb4ssman
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lversion -lsetupapi -lcfgmgr32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*...*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*...*/
// ==/WindhawkModSettings==

#undef GetCurrentTime

#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Automation.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Shapes.h>

#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <cfgmgr32.h>
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
// Smart grid layout
// Template block: _templates/smart-grid-layout.h v1.0 (verbatim copy — keep
// in sync with the template; Windhawk mods are single-file).
// ============================================================

#include <climits>

namespace windhawk_mod_templates::smart_grid {

enum class GridMode {
    AutoSmart,
    SingleRow,
    SingleColumn,
    FixedRows,
    FixedColumns,
    FixedGrid,
};

enum class SmartLayout { Balanced, PackVertical, PackHorizontal };
enum class FillOrder { RowFirst, ColumnFirst };
enum class ShortGroupPosition { First, Last };
enum class ShortGroupAlign { Start, Center, End };

struct Config {
    GridMode mode = GridMode::AutoSmart;
    SmartLayout smartLayout = SmartLayout::Balanced;
    FillOrder fillOrder = FillOrder::RowFirst;
    ShortGroupPosition shortGroupPosition = ShortGroupPosition::Last;
    ShortGroupAlign shortGroupAlign = ShortGroupAlign::Center;
    int rows = 0;          // exact in fixed modes; maximum in AutoSmart
    int columns = 0;       // exact in fixed modes; maximum in AutoSmart
    int availableRows = 1; // derive from host height / item pitch
};

struct Layout {
    int rows = 1;
    int columns = 1;
};

// A short group may need to span its complete axis so a half-cell offset can
// be expressed with Margin. Multiply offsetUnits by item-size-plus-spacing.
struct Cell {
    int row = 0;
    int column = 0;
    int rowSpan = 1;
    int columnSpan = 1;
    double topOffsetUnits = 0.0;
    double leftOffsetUnits = 0.0;
};

inline int ScoreCandidate(int rows, int columns, int count,
                          SmartLayout preference) {
    int waste = rows * columns - count;
    int widePenalty = columns > rows ? (columns - rows) * 2 : 0;
    int score = waste * 10 + widePenalty;

    if (preference == SmartLayout::PackVertical)
        score -= rows * 20;
    else if (preference == SmartLayout::PackHorizontal)
        score += rows * 20;
    else
        score -= rows * 3;

    return score;
}

inline Layout ComputeLayout(int count, Config const& config) {
    count = std::max(1, count);
    Layout result;
    int availableRows = std::clamp(config.availableRows, 1, count);
    if (config.rows > 0 && config.mode == GridMode::AutoSmart)
        availableRows = std::min(availableRows, config.rows);

    switch (config.mode) {
        case GridMode::SingleRow:
            result = {1, count};
            break;
        case GridMode::SingleColumn:
            result = {count, 1};
            break;
        case GridMode::FixedRows:
            result.rows = std::clamp(config.rows, 1, count);
            result.columns = (count + result.rows - 1) / result.rows;
            break;
        case GridMode::FixedColumns:
            result.columns = std::clamp(config.columns, 1, count);
            result.rows = (count + result.columns - 1) / result.columns;
            break;
        case GridMode::FixedGrid:
            result.rows = std::clamp(config.rows, 1, count);
            result.columns = config.columns > 0
                ? std::clamp(config.columns, 1, count)
                : (count + result.rows - 1) / result.rows;
            if (result.rows * result.columns < count)
                result.rows = (count + result.columns - 1) / result.columns;
            break;
        case GridMode::AutoSmart: {
            int bestScore = INT_MAX;
            int firstRows = availableRows > 1 && count > 1 &&
                            config.smartLayout != SmartLayout::PackHorizontal
                ? 2 : 1;
            for (int rows = firstRows; rows <= availableRows; ++rows) {
                int columns = (count + rows - 1) / rows;
                if (config.columns > 0 && columns > config.columns)
                    continue;
                int score = ScoreCandidate(rows, columns, count,
                                           config.smartLayout);
                if (score < bestScore) {
                    bestScore = score;
                    result = {rows, columns};
                }
            }
            if (bestScore == INT_MAX) {
                result.columns = std::clamp(config.columns, 1, count);
                result.rows = (count + result.columns - 1) / result.columns;
            }
            break;
        }
    }

    result.rows = std::clamp(result.rows, 1, count);
    result.columns = std::max(1, result.columns);
    while (result.rows * result.columns < count) {
        if (config.mode == GridMode::FixedColumns)
            ++result.rows;
        else
            ++result.columns;
    }
    return result;
}

inline double AlignOffset(int capacity, int itemCount,
                          ShortGroupAlign alignment) {
    int unused = std::max(0, capacity - itemCount);
    if (alignment == ShortGroupAlign::Center)
        return unused / 2.0;
    if (alignment == ShortGroupAlign::End)
        return static_cast<double>(unused);
    return 0.0;
}

inline Cell GetCell(int index, int count, Layout const& layout,
                    Config const& config) {
    Cell cell;
    index = std::clamp(index, 0, std::max(0, count - 1));

    if (config.fillOrder == FillOrder::RowFirst) {
        int groupCount = (count + layout.columns - 1) / layout.columns;
        int shortCount = count % layout.columns;
        if (!shortCount) shortCount = layout.columns;
        int group;
        int itemInGroup;
        if (shortCount < layout.columns &&
            config.shortGroupPosition == ShortGroupPosition::First) {
            if (index < shortCount) {
                group = 0;
                itemInGroup = index;
            } else {
                int adjusted = index - shortCount;
                group = 1 + adjusted / layout.columns;
                itemInGroup = adjusted % layout.columns;
            }
        } else {
            group = index / layout.columns;
            itemInGroup = index % layout.columns;
        }
        int shortGroup = config.shortGroupPosition == ShortGroupPosition::First
            ? 0 : groupCount - 1;
        bool isShort = shortCount < layout.columns && group == shortGroup;

        cell.row = group;
        cell.column = itemInGroup;
        if (isShort && config.shortGroupAlign != ShortGroupAlign::Start) {
            cell.column = 0;
            cell.columnSpan = layout.columns;
            cell.leftOffsetUnits = AlignOffset(layout.columns, shortCount,
                                               config.shortGroupAlign) +
                                   itemInGroup;
        }
    } else {
        int groupCount = (count + layout.rows - 1) / layout.rows;
        int shortCount = count % layout.rows;
        if (!shortCount) shortCount = layout.rows;
        int group;
        int itemInGroup;
        if (shortCount < layout.rows &&
            config.shortGroupPosition == ShortGroupPosition::First) {
            if (index < shortCount) {
                group = 0;
                itemInGroup = index;
            } else {
                int adjusted = index - shortCount;
                group = 1 + adjusted / layout.rows;
                itemInGroup = adjusted % layout.rows;
            }
        } else {
            group = index / layout.rows;
            itemInGroup = index % layout.rows;
        }
        int shortGroup = config.shortGroupPosition == ShortGroupPosition::First
            ? 0 : groupCount - 1;
        bool isShort = shortCount < layout.rows && group == shortGroup;

        cell.row = itemInGroup;
        cell.column = group;
        if (isShort && config.shortGroupAlign != ShortGroupAlign::Start) {
            cell.row = 0;
            cell.rowSpan = layout.rows;
            cell.topOffsetUnits = AlignOffset(layout.rows, shortCount,
                                              config.shortGroupAlign) +
                                  itemInGroup;
        }
    }
    return cell;
}

} // namespace windhawk_mod_templates::smart_grid

namespace grid = windhawk_mod_templates::smart_grid;

// ============================================================
// Settings
// ============================================================

struct ModSettings {
    std::wstring position = L"beforeOmni";
    std::wstring itemOrder = L"location,mic,camera,copilot";
    int  gridRows    = 0;
    int  gridColumns = 0;
    grid::FillOrder          fillOrder          = grid::FillOrder::RowFirst;
    grid::ShortGroupPosition shortGroupPosition = grid::ShortGroupPosition::Last;
    grid::ShortGroupAlign    shortGroupAlign    = grid::ShortGroupAlign::Center;
    int  iconSize      = 16;
    int  buttonSpacing = 4;
    int  idleOpacity   = 50;
    int  groupPaddingLeft = 0;
    int  groupPaddingRight = 0;
    int  groupOffsetX = 0;
    int  groupOffsetY = 0;
    int  locationOffsetX = 0;
    int  locationOffsetY = 0;
    int  micOffsetX   = 0;
    int  micOffsetY   = 0;
    int  cameraOffsetX = 0;
    int  cameraOffsetY = 0;
    int  copilotOffsetX = 0;
    int  copilotOffsetY = 0;
    bool activeColorSet = false;
    winrt::Windows::UI::Color activeColorValue{};
    bool glowEnabled    = false;
    int  glowOpacity    = 40;
    bool slashColorSet  = false;   // false = system theme
    winrt::Windows::UI::Color slashColorValue{};
    std::wstring slashDirection = L"rising";
    int  slashOpacity   = 100;
    bool suppressNativeIndicators = true;
};
static ModSettings g_settings;

static std::wstring GetStringSetting(PCWSTR name) {
    PCWSTR raw = Wh_GetStringSetting(name);
    std::wstring value = raw;
    Wh_FreeStringSetting(raw);
    return value;
}

// Color-returning variant of the canonical token parser (_templates/button-surface.h):
// "#RRGGBB" / "#AARRGGBB" hex (alpha honored, "#" required), the generics
// "accent" / "accentLight" / "accentDark" / "transparent", and the numbered
// Windows shades "accentLight1"-"3" / "accentDark1"-"3" (accepted silently,
// undocumented). Empty/unparseable returns false = keep the native behavior.
static bool ParseColorToken(const wchar_t* s, winrt::Windows::UI::Color& out) {
    using winrt::Windows::UI::ViewManagement::UIColorType;
    if (!s || !*s) return false;

    if (_wcsicmp(s, L"transparent") == 0) {
        out = {0, 0, 0, 0};
        return true;
    }

    static const struct { const wchar_t* token; UIColorType type; } kAccentTokens[] = {
        {L"accent",       UIColorType::Accent},
        {L"accentLight",  UIColorType::AccentLight2},
        {L"accentDark",   UIColorType::AccentDark1},
        {L"accentLight1", UIColorType::AccentLight1},
        {L"accentLight2", UIColorType::AccentLight2},
        {L"accentLight3", UIColorType::AccentLight3},
        {L"accentDark1",  UIColorType::AccentDark1},
        {L"accentDark2",  UIColorType::AccentDark2},
        {L"accentDark3",  UIColorType::AccentDark3},
    };
    for (auto const& entry : kAccentTokens) {
        if (_wcsicmp(s, entry.token) == 0) {
            try {
                winrt::Windows::UI::ViewManagement::UISettings settings;
                out = settings.GetColorValue(entry.type);
                return true;
            } catch (...) {
                Wh_Log(L"[Color] Failed to read the Windows accent color");
                return false;
            }
        }
    }

    if (*s != L'#') return false;
    const wchar_t* p = s + 1;
    size_t len = wcslen(p);
    if (len != 6 && len != 8) return false;
    for (size_t i = 0; i < len; i++)
        if (!iswxdigit(p[i])) return false;
    unsigned long v = wcstoul(p, nullptr, 16);
    if (len == 6) { out = {255, BYTE(v>>16), BYTE(v>>8), BYTE(v)}; }
    else          { out = {BYTE(v>>24), BYTE(v>>16), BYTE(v>>8), BYTE(v)}; }
    return true;
}

static void LoadSettings() {
    auto clamp = [](int v, int lo, int hi) { return std::max(lo, std::min(hi, v)); };
    g_settings.position  = GetStringSetting(L"position");
    g_settings.itemOrder = GetStringSetting(L"itemOrder");
    g_settings.gridRows    = clamp(Wh_GetIntSetting(L"gridRows"), 0, 10);
    g_settings.gridColumns = clamp(Wh_GetIntSetting(L"gridColumns"), 0, 10);
    { std::wstring s = GetStringSetting(L"fillOrder");
      // "colFirst" accepted as a legacy spelling of "columnFirst"
      g_settings.fillOrder = (s == L"columnFirst" || s == L"colFirst")
                           ? grid::FillOrder::ColumnFirst : grid::FillOrder::RowFirst; }
    { std::wstring s = GetStringSetting(L"shortGroupPosition");
      g_settings.shortGroupPosition = (s == L"first")
                                    ? grid::ShortGroupPosition::First
                                    : grid::ShortGroupPosition::Last; }
    { std::wstring s = GetStringSetting(L"shortGroupAlign");
      if      (s == L"start") g_settings.shortGroupAlign = grid::ShortGroupAlign::Start;
      else if (s == L"end")   g_settings.shortGroupAlign = grid::ShortGroupAlign::End;
      else                    g_settings.shortGroupAlign = grid::ShortGroupAlign::Center; }
    g_settings.iconSize             = clamp(Wh_GetIntSetting(L"iconSize"), 8, 48);
    g_settings.idleOpacity          = clamp(Wh_GetIntSetting(L"idleOpacity"), 0, 100);
    g_settings.groupPaddingLeft     = clamp(Wh_GetIntSetting(L"groupPaddingLeft"), -40, 40);
    g_settings.groupPaddingRight    = clamp(Wh_GetIntSetting(L"groupPaddingRight"), -40, 40);
    g_settings.buttonSpacing        = clamp(Wh_GetIntSetting(L"buttonSpacing"), 0, 40);
    g_settings.groupOffsetX         = clamp(Wh_GetIntSetting(L"groupOffsetX"), -40, 40);
    g_settings.groupOffsetY         = clamp(Wh_GetIntSetting(L"groupOffsetY"), -40, 40);
    g_settings.locationOffsetX      = clamp(Wh_GetIntSetting(L"locationOffsetX"), -40, 40);
    g_settings.locationOffsetY      = clamp(Wh_GetIntSetting(L"locationOffsetY"), -40, 40);
    g_settings.micOffsetX           = clamp(Wh_GetIntSetting(L"micOffsetX"), -40, 40);
    g_settings.micOffsetY           = clamp(Wh_GetIntSetting(L"micOffsetY"), -40, 40);
    g_settings.cameraOffsetX        = clamp(Wh_GetIntSetting(L"cameraOffsetX"), -40, 40);
    g_settings.cameraOffsetY        = clamp(Wh_GetIntSetting(L"cameraOffsetY"), -40, 40);
    g_settings.copilotOffsetX       = clamp(Wh_GetIntSetting(L"copilotOffsetX"), -40, 40);
    g_settings.copilotOffsetY       = clamp(Wh_GetIntSetting(L"copilotOffsetY"), -40, 40);
    g_settings.glowEnabled          = Wh_GetIntSetting(L"glowEnabled") != 0;
    g_settings.glowOpacity          = clamp(Wh_GetIntSetting(L"glowOpacity"), 0, 100);
    g_settings.activeColorSet = ParseColorToken(
        GetStringSetting(L"activeColor").c_str(), g_settings.activeColorValue);
    g_settings.slashColorSet = ParseColorToken(
        GetStringSetting(L"slashColor").c_str(), g_settings.slashColorValue);
    g_settings.slashDirection = GetStringSetting(L"slashDirection");
    g_settings.slashOpacity   = clamp(Wh_GetIntSetting(L"slashOpacity"), 0, 100);
    g_settings.suppressNativeIndicators = Wh_GetIntSetting(L"suppressNativeIndicators") != 0;
}

// ============================================================
// Globals
// ============================================================

static std::atomic<bool> g_unloading{false};
static HWND              g_taskbarWnd           = nullptr;
static std::atomic<bool>  g_systemTrayModuleHooked{false};
static HANDLE            g_retryThread          = nullptr;
static HANDLE            g_retryStopEvent       = nullptr;
static HANDLE            g_stateRefreshEvent    = nullptr;

static std::atomic<bool> g_locActive{false};
static std::atomic<bool> g_micActive{false};
static std::atomic<bool> g_camActive{false};
// Usage-record detection (ConsentStore LastUsedTimeStop==0) — covers hardware
// camera/mic/location that never get a native tray glyph. ORed with the
// glyph-driven *Active flags above when rendering.
static std::atomic<bool> g_locUsage{false};
static std::atomic<bool> g_micUsage{false};
static std::atomic<bool> g_camUsage{false};
static std::atomic<bool> g_locDisabled{false};
static std::atomic<bool> g_micDisabled{false};
static std::atomic<bool> g_camDisabled{false};
static std::atomic<bool> g_copilotInstalled{false};
static std::atomic<bool> g_copilotActive{false};
static std::atomic<bool> g_copilotDisabled{true};
static Grid              g_syntheticGrid   = nullptr;
static FrameworkElement  g_locIcon         = nullptr;
static FrameworkElement  g_micIcon         = nullptr;
static FrameworkElement  g_camIcon         = nullptr;
static FrameworkElement  g_copilotIcon     = nullptr;
static FrameworkElement  g_locGlowIcon     = nullptr;
static FrameworkElement  g_micGlowIcon     = nullptr;
static FrameworkElement  g_camGlowIcon     = nullptr;
static FrameworkElement  g_copilotGlowIcon = nullptr;
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
    int64_t textToken       = 0;
    int64_t visibilityToken = 0;
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
static bool HookSystemTraySymbols(HMODULE h);
static void HandleLoadedModuleIfSystemTray(HMODULE module,
                                            LPCWSTR fileName);

static VS_FIXEDFILEINFO* GetModuleVersionInfo(HMODULE module) {
    void* fixedFileInfo = nullptr;
    UINT length = 0;
    HRSRC resource = FindResourceW(
        module, MAKEINTRESOURCEW(VS_VERSION_INFO), RT_VERSION);
    if (!resource) return nullptr;
    HGLOBAL loaded = LoadResource(module, resource);
    void* data = loaded ? LockResource(loaded) : nullptr;
    if (!data || !VerQueryValueW(data, L"\\", &fixedFileInfo, &length) ||
        !length)
        return nullptr;
    return static_cast<VS_FIXEDFILEINFO*>(fixedFileInfo);
}

static HMODULE GetSystemTrayModuleHandle() {
    HMODULE module = GetModuleHandleW(L"SystemTray.dll");
    if (!module) {
        module = GetModuleHandleW(L"Taskbar.View.dll");
        if (module) {
            auto version = GetModuleVersionInfo(module);
            WORD major = version ? HIWORD(version->dwFileVersionMS) : 0;
            if (!major || major >= 2604)
                module = nullptr;
        }
    }
    if (!module)
        module = GetModuleHandleW(L"ExplorerExtensions.dll");
    return module;
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
    if (!CTaskBand_GetTaskbarHost_Original ||
        !TaskbarHost_FrameHeight_Original ||
        !std__Ref_count_base__Decref_Original ||
        !CTaskBand_ITaskListWndSite_vftable)
        return nullptr;

    HWND hTaskSwWnd = (HWND)GetProp(hTaskbarWnd, L"TaskbandHWND");
    if (!hTaskSwWnd) return nullptr;
    void* taskBand = (void*)GetWindowLongPtr(hTaskSwWnd, 0);
    if (!taskBand) return nullptr;
    void* taskBandForSite = taskBand;
    for (int i = 0; *(void**)taskBandForSite != CTaskBand_ITaskListWndSite_vftable; i++) {
        if (i == 20) return nullptr;
        taskBandForSite = (void**)taskBandForSite + 1;
    }
    void* taskbarHostSharedPtr[2]{};
    CTaskBand_GetTaskbarHost_Original(taskBandForSite, taskbarHostSharedPtr);
    if (!taskbarHostSharedPtr[0] || !taskbarHostSharedPtr[1]) {
        if (taskbarHostSharedPtr[1])
            std__Ref_count_base__Decref_Original(taskbarHostSharedPtr[1]);
        return nullptr;
    }
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
    if (!iunk) {
        std__Ref_count_base__Decref_Original(taskbarHostSharedPtr[1]);
        return nullptr;
    }
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
// Synthetic icon management
// ============================================================

static void UpdateSyntheticOpacity() {
    double idleOp = g_settings.idleOpacity / 100.0;
    winrt::Windows::UI::Color activeColor = g_settings.activeColorValue;

    // active   = feature in use → full opacity, optional custom color + glow
    // disabled = service off   → idle opacity, slash visible
    // idle     = neither       → idle opacity, system foreground, no slash
    //
    // setShapeFill: sets Fill on icon. Shape branch handles TextBlock-based icons;
    // VisualTreeHelper fallback handles Viewbox/Path icons once mounted in the tree.
    auto setShapeFill = [](FrameworkElement fe, winrt::Windows::UI::Color color) {
        SolidColorBrush br; br.Color(color);
        if (auto shape = fe.try_as<winrt::Windows::UI::Xaml::Shapes::Shape>()) {
            shape.Fill(br); return;
        }
        // VisualTreeHelper fallback (requires mounted tree — works during runtime updates)
        auto child = FindChildRecursive(fe,
            [](FrameworkElement e) { return e.try_as<winrt::Windows::UI::Xaml::Shapes::Shape>() != nullptr; });
        if (auto cs = child.try_as<winrt::Windows::UI::Xaml::Shapes::Shape>()) cs.Fill(br);
    };

    auto applySlot = [&](FrameworkElement icon, FrameworkElement glow, FrameworkElement slash,
                         bool active, bool disabled) {
        if (!icon) return;
        bool effectiveActive = active && !disabled;
        icon.Opacity(effectiveActive ? 1.0 : idleOp);

        // Custom active tint — only when the user sets an active color.
        // Default: clear any explicit brush so system theme foreground applies.
        if (g_settings.activeColorSet && effectiveActive) {
            if (auto tb = icon.try_as<TextBlock>()) {
                SolidColorBrush activeBrush; activeBrush.Color(activeColor);
                tb.Foreground(activeBrush);
            } else {
                setShapeFill(icon, activeColor);
            }
        } else {
            if (auto tb = icon.try_as<TextBlock>()) {
                tb.ClearValue(TextBlock::ForegroundProperty());
            } else {
                // Shapes and Viewbox containers: restore neutral system-foreground color.
                // (Shape.Fill default is null/transparent, so ClearValue would hide it.)
                bool isDark = (Application::Current().RequestedTheme() == ApplicationTheme::Dark);
                setShapeFill(icon, isDark ? winrt::Windows::UI::Color{255, 255, 255, 255}
                                          : winrt::Windows::UI::Color{255,  30,  30,  30});
            }
        }

        if (glow) {
            glow.Visibility((effectiveActive && g_settings.glowEnabled) ? Visibility::Visible : Visibility::Collapsed);
        }

        if (slash) {
            slash.Visibility(disabled ? Visibility::Visible : Visibility::Collapsed);
        }
    };

    applySlot(g_locIcon, g_locGlowIcon, g_locSlashIcon,
              g_locActive.load() || g_locUsage.load(), g_locDisabled.load());
    applySlot(g_micIcon, g_micGlowIcon, g_micSlashIcon,
              g_micActive.load() || g_micUsage.load(), g_micDisabled.load());
    applySlot(g_camIcon, g_camGlowIcon, g_camSlashIcon,
              g_camActive.load() || g_camUsage.load(), g_camDisabled.load());
    applySlot(g_copilotIcon, g_copilotGlowIcon, g_copilotSlashIcon,
              g_copilotActive.load(), g_copilotDisabled.load());
}

static void SetIconTooltip(FrameworkElement const& fe, PCWSTR label, bool active, bool disabled,
                           PCWSTR idleLabel     = L"Not requested",
                           PCWSTR disabledLabel = L"Hardware disabled") {
    if (!fe) return;
    const wchar_t* state = disabled ? disabledLabel
                         : active  ? L"In use"
                                   : idleLabel;
    winrt::hstring tooltip = winrt::hstring(label) + L":\n" + state;
    ToolTipService::SetToolTip(fe, winrt::box_value(tooltip));
    winrt::Windows::UI::Xaml::Automation::AutomationProperties::SetName(
        fe, winrt::hstring(label) + L": " + state);
}

static void UpdateSyntheticTooltips() {
    if (g_locIcon) SetIconTooltip(g_locIcon, L"Location",   g_locActive.load() || g_locUsage.load(), g_locDisabled.load());
    if (g_micIcon) SetIconTooltip(g_micIcon, L"Microphone", g_micActive.load() || g_micUsage.load(), g_micDisabled.load());
    if (g_camIcon) SetIconTooltip(g_camIcon, L"Camera",     g_camActive.load() || g_camUsage.load(), g_camDisabled.load());
    if (g_copilotIcon) SetIconTooltip(g_copilotIcon,
        L"Copilot", g_copilotActive.load(), g_copilotDisabled.load(),
        L"Installed (not running)", L"Not installed / disabled");
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
    if (FAILED(hr)) {
        Wh_Log(L"[Mic] => disabled/unavailable (no default capture endpoint), hr=0x%08X", hr);
        return true;  // E_NOTFOUND = no capture device at all
    }

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

class MicPrivacyMonitor final : public IMMNotificationClient,
                                public IAudioEndpointVolumeCallback {
public:
    explicit MicPrivacyMonitor(HANDLE refreshEvent) : m_refreshEvent(refreshEvent) {}

    HRESULT Init() {
        HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                      IID_PPV_ARGS(&m_enum));
        if (FAILED(hr)) {
            Wh_Log(L"[MicMon] MMDeviceEnumerator failed hr=0x%08X", hr);
            return hr;
        }

        hr = m_enum->RegisterEndpointNotificationCallback(this);
        if (FAILED(hr)) {
            Wh_Log(L"[MicMon] RegisterEndpointNotificationCallback failed hr=0x%08X", hr);
        }

        AttachDefaultEndpoint();
        SignalRefresh(L"init");
        return S_OK;
    }

    void Cleanup() {
        DetachEndpointVolume();
        if (m_enum) {
            m_enum->UnregisterEndpointNotificationCallback(this);
            m_enum->Release();
            m_enum = nullptr;
        }
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
        if (!ppvObject) return E_POINTER;
        if (riid == __uuidof(IUnknown) || riid == __uuidof(IMMNotificationClient)) {
            *ppvObject = static_cast<IMMNotificationClient*>(this);
        } else if (riid == __uuidof(IAudioEndpointVolumeCallback)) {
            *ppvObject = static_cast<IAudioEndpointVolumeCallback*>(this);
        } else {
            *ppvObject = nullptr;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

    ULONG STDMETHODCALLTYPE AddRef() override {
        return (ULONG)InterlockedIncrement(&m_refCount);
    }

    ULONG STDMETHODCALLTYPE Release() override {
        ULONG count = (ULONG)InterlockedDecrement(&m_refCount);
        if (count == 0) m_refCount = 1; // lifetime is owned by the monitor thread
        return count;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override {
        Wh_Log(L"[MicMon] DeviceStateChanged state=0x%X id=%s", dwNewState, pwstrDeviceId ? pwstrDeviceId : L"");
        AttachDefaultEndpoint();
        SignalRefresh(L"device state");
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override {
        Wh_Log(L"[MicMon] DeviceAdded id=%s", pwstrDeviceId ? pwstrDeviceId : L"");
        AttachDefaultEndpoint();
        SignalRefresh(L"device added");
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) override {
        Wh_Log(L"[MicMon] DeviceRemoved id=%s", pwstrDeviceId ? pwstrDeviceId : L"");
        AttachDefaultEndpoint();
        SignalRefresh(L"device removed");
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) override {
        if (flow == eCapture && (role == eConsole || role == eCommunications)) {
            Wh_Log(L"[MicMon] DefaultDeviceChanged role=%d id=%s", (int)role,
                   pwstrDefaultDeviceId ? pwstrDefaultDeviceId : L"");
            AttachDefaultEndpoint();
            SignalRefresh(L"default device");
        }
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY) override {
        Wh_Log(L"[MicMon] PropertyValueChanged id=%s", pwstrDeviceId ? pwstrDeviceId : L"");
        SignalRefresh(L"property");
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) override {
        if (pNotify) {
            Wh_Log(L"[MicMon] EndpointVolume muted=%d master=%.3f",
                   pNotify->bMuted ? 1 : 0, pNotify->fMasterVolume);
        } else {
            Wh_Log(L"[MicMon] EndpointVolume changed");
        }
        SignalRefresh(L"endpoint volume");
        return S_OK;
    }

private:
    void SignalRefresh(PCWSTR reason) {
        Wh_Log(L"[MicMon] Refresh requested: %s", reason);
        if (m_refreshEvent) SetEvent(m_refreshEvent);
    }

    void DetachEndpointVolume() {
        if (m_volume) {
            m_volume->UnregisterControlChangeNotify(this);
            m_volume->Release();
            m_volume = nullptr;
        }
        if (m_device) {
            m_device->Release();
            m_device = nullptr;
        }
    }

    void AttachDefaultEndpoint() {
        if (!m_enum) return;
        DetachEndpointVolume();

        HRESULT hr = m_enum->GetDefaultAudioEndpoint(eCapture, eConsole, &m_device);
        if (FAILED(hr)) {
            Wh_Log(L"[MicMon] Default capture endpoint unavailable hr=0x%08X", hr);
            return;
        }

        hr = m_device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr,
                                reinterpret_cast<void**>(&m_volume));
        if (FAILED(hr)) {
            Wh_Log(L"[MicMon] EndpointVolume activate failed hr=0x%08X", hr);
            return;
        }

        hr = m_volume->RegisterControlChangeNotify(this);
        if (FAILED(hr)) {
            Wh_Log(L"[MicMon] RegisterControlChangeNotify failed hr=0x%08X", hr);
        } else {
            Wh_Log(L"[MicMon] Watching default capture endpoint volume");
        }
    }

    volatile LONG m_refCount = 1;
    HANDLE m_refreshEvent = nullptr;
    IMMDeviceEnumerator* m_enum = nullptr;
    IMMDevice* m_device = nullptr;
    IAudioEndpointVolume* m_volume = nullptr;
};

// Returns true if a camera device exists in the Windows device database but none is
// currently accessible — i.e. the physical kill switch has cut power to the camera.
static bool CheckCameraDisabled() {
    // Check 1: WinRT DeviceAccessInformation (most reliable for Privacy Settings toggle)
    try {
        using namespace winrt::Windows::Devices::Enumeration;
        static const winrt::guid kCameraClass{0xca3e7ab9, 0xb4c3, 0x4ae6,
            {0x82, 0x51, 0x57, 0x9e, 0xf9, 0x33, 0x89, 0x0f}};
        auto info = DeviceAccessInformation::CreateFromDeviceClassId(kCameraClass);
        auto status = info.CurrentStatus();
        Wh_Log(L"[Cam] DeviceAccessStatus=%d (1=DeniedByUser,2=DeniedBySystem)", (int)status);
        if (status == DeviceAccessStatus::DeniedByUser ||
            status == DeviceAccessStatus::DeniedBySystem) {
            Wh_Log(L"[Cam] => disabled (DeviceAccess consent)"); return true;
        }
    } catch (...) {
        Wh_Log(L"[Cam] DeviceAccessInformation threw");
    }

    static const GUID GUID_DEVCLASS_CAMERA_LOCAL =
        {0xca3e7ab9, 0xb4c3, 0x4ae6, {0x82, 0x51, 0x57, 0x9e, 0xf9, 0x33, 0x89, 0x0f}};
    // Check if any camera device is registered in the system at all
    HDEVINFO allDevs = SetupDiGetClassDevs(&GUID_DEVCLASS_CAMERA_LOCAL, nullptr, nullptr, 0);
    if (allDevs == INVALID_HANDLE_VALUE) {
        Wh_Log(L"[Cam] allDevs INVALID_HANDLE_VALUE err=%u", GetLastError());
        return false;
    }
    SP_DEVINFO_DATA d{}; d.cbSize = sizeof(d);
    bool hasAny = SetupDiEnumDeviceInfo(allDevs, 0, &d) == TRUE;
    SetupDiDestroyDeviceInfoList(allDevs);
    Wh_Log(L"[Cam] hasAny=%d", hasAny);
    if (!hasAny) { Wh_Log(L"[Cam] => disabled/unavailable (no camera hardware)"); return true; }
    // Check if any non-IR camera is present (powered on)
    // Filter out IR/Hello cameras which are always-on and would mask a hardware kill switch.
    HDEVINFO presentDevs = SetupDiGetClassDevs(&GUID_DEVCLASS_CAMERA_LOCAL, nullptr, nullptr, DIGCF_PRESENT);
    bool hasPresent = false;
    if (presentDevs != INVALID_HANDLE_VALUE) {
        SP_DEVINFO_DATA pd{}; pd.cbSize = sizeof(pd);
        for (DWORD idx = 0; !hasPresent && SetupDiEnumDeviceInfo(presentDevs, idx, &pd); idx++) {
            wchar_t name[256] = {}; DWORD type = 0, sz = sizeof(name);
            SetupDiGetDeviceRegistryPropertyW(presentDevs, &pd, SPDRP_FRIENDLYNAME,
                                              &type, (BYTE*)name, sz, nullptr);
            std::wstring_view nm{name};
            bool isIR = nm.find(L"IR") != std::wstring_view::npos ||
                        (nm.find(L"Hello") != std::wstring_view::npos) ||
                        (nm.find(L"Face")  != std::wstring_view::npos);
            if (!isIR) {
                // Also check if the device is disabled in Device Manager.
                // Hardware kill switches sometimes disable the device rather than
                // removing it from DIGCF_PRESENT entirely.
                ULONG devStatus = 0, devProblem = 0;
                bool hasProblem = false;
                if (CM_Get_DevNode_Status(&devStatus, &devProblem, pd.DevInst, 0) == CR_SUCCESS) {
                    hasProblem = (devStatus & DN_HAS_PROBLEM) != 0;
                }
                Wh_Log(L"[Cam] present device: '%s' isIR=%d hasProblem=%d (status=0x%X prob=%u)",
                    name, isIR ? 1 : 0, hasProblem ? 1 : 0, devStatus, devProblem);
                if (!hasProblem) hasPresent = true;
            } else {
                Wh_Log(L"[Cam] present device: '%s' isIR=1 (filtered)", name);
            }
        }
        SetupDiDestroyDeviceInfoList(presentDevs);
    }
    Wh_Log(L"[Cam] hasPresent(non-IR)=%d", hasPresent);
    if (!hasPresent) { Wh_Log(L"[Cam] => disabled (kill switch)"); return true; }
    // Check: per-user consent (Privacy & Security → Camera toggle → HKCU)
    {
        HKEY hk = nullptr;
        LONG openR = RegOpenKeyExW(HKEY_CURRENT_USER,
                L"Software\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager"
                L"\\ConsentStore\\webcam",
                0, KEY_READ, &hk);
        Wh_Log(L"[Cam] HKCU webcam ConsentStore open=%d", openR);
        if (openR == ERROR_SUCCESS) {
            wchar_t val[64] = {};
            DWORD valLen = sizeof(val), type = 0;
            LONG r = RegQueryValueExW(hk, L"Value", nullptr, &type, (LPBYTE)val, &valLen);
            RegCloseKey(hk);
            Wh_Log(L"[Cam] HKCU Value: r=%d type=%u val='%s'", r, type, val);
            if (r == ERROR_SUCCESS && type == REG_SZ && _wcsicmp(val, L"Deny") == 0) {
                Wh_Log(L"[Cam] => disabled (HKCU consent)"); return true;
            }
        }
    }
    // Check: machine-wide policy (HKLM)
    {
        HKEY hk = nullptr;
        LONG openR = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager"
                L"\\ConsentStore\\webcam",
                0, KEY_READ, &hk);
        Wh_Log(L"[Cam] HKLM webcam ConsentStore open=%d", openR);
        if (openR == ERROR_SUCCESS) {
            wchar_t val[64] = {};
            DWORD valLen = sizeof(val), type = 0;
            LONG r = RegQueryValueExW(hk, L"Value", nullptr, &type, (LPBYTE)val, &valLen);
            RegCloseKey(hk);
            Wh_Log(L"[Cam] HKLM Value: r=%d type=%u val='%s'", r, type, val);
            if (r == ERROR_SUCCESS && type == REG_SZ && _wcsicmp(val, L"Deny") == 0) {
                Wh_Log(L"[Cam] => disabled (HKLM consent)"); return true;
            }
        }
    }
    Wh_Log(L"[Cam] => enabled");
    return false;
}

// Returns true if a Copilot app package is registered in the AppModel repository for the
// current user or machine. The package data directory in %LOCALAPPDATA%\Packages is NOT
// used — that directory survives uninstall (it holds user data) and would give a false
// positive after removal.
//
// Deliberately NOT matched: MicrosoftWindows.Client.WebExperience — that package is the
// Widgets host, present on virtually every Windows 11 install, and counting it as Copilot
// made the mod report "installed" on Copilot-free machines (live-verified 2026-07-17).
static bool CheckCopilotInstalled() {
    // Sub-keys under Repository\Packages are named <PackageFullName> e.g.
    //   Microsoft.Copilot_<ver>_x64__<pub>
    // Stale keys can survive uninstall, so a key only counts if its package path
    // still exists on disk.
    static const wchar_t* const kPrefixes[] = {
        L"Microsoft.Copilot_",
        L"Microsoft.Windows.Ai.Copilot_",
    };
    static const wchar_t* const kRoots[] = {
        L"Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion"
            L"\\AppModel\\Repository\\Packages",                     // per-user (HKCU)
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion"
            L"\\AppModel\\Repository\\Packages",                     // machine-wide (HKLM)
    };
    static const HKEY kHives[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };
    for (int h = 0; h < 2; h++) {
        HKEY hPkg = nullptr;
        LONG openR = RegOpenKeyExW(kHives[h], kRoots[h], 0,
                                   KEY_READ | KEY_ENUMERATE_SUB_KEYS, &hPkg);
        Wh_Log(L"[Copilot] AppModel %s open=%d", h == 0 ? L"HKCU" : L"HKLM", openR);
        if (openR != ERROR_SUCCESS) continue;
        wchar_t name[256]; DWORD nameLen;
        bool found = false;
        for (DWORD i = 0; !found; i++) {
            nameLen = ARRAYSIZE(name);
            LONG e = RegEnumKeyExW(hPkg, i, name, &nameLen,
                                   nullptr, nullptr, nullptr, nullptr);
            if (e == ERROR_NO_MORE_ITEMS) break;
            if (e != ERROR_SUCCESS)       continue;
            bool prefixMatch = false;
            for (const auto* prefix : kPrefixes) {
                if (wcsncmp(name, prefix, wcslen(prefix)) == 0) {
                    prefixMatch = true;
                    break;
                }
            }
            if (prefixMatch) {
                HKEY hItem = nullptr;
                if (RegOpenKeyExW(hPkg, name, 0, KEY_READ, &hItem) == ERROR_SUCCESS) {
                    wchar_t path[MAX_PATH] = {};
                    DWORD pathLen = sizeof(path), type = 0;
                    LONG pathR = RegQueryValueExW(hItem, L"PackageRootFolder", nullptr,
                                                  &type, (LPBYTE)path, &pathLen);
                    if (pathR != ERROR_SUCCESS) {
                        pathLen = sizeof(path); type = 0;
                        pathR = RegQueryValueExW(hItem, L"Path", nullptr,
                                                 &type, (LPBYTE)path, &pathLen);
                    }
                    RegCloseKey(hItem);
                    bool pathExists = pathR == ERROR_SUCCESS &&
                                      (type == REG_SZ || type == REG_EXPAND_SZ) &&
                                      GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
                    Wh_Log(L"[Copilot] package key: %s path='%s' exists=%d",
                           name, path, pathExists ? 1 : 0);
                    if (pathExists) found = true;
                }
            }
        }
        RegCloseKey(hPkg);
        if (found) { Wh_Log(L"[Copilot] => installed"); return true; }
    }
    Wh_Log(L"[Copilot] => not installed");
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

// Returns true if Copilot is disabled: not installed, turned off by group policy, or
// explicitly hidden via Settings. "Installed but not running" is idle, not disabled —
// same as mic existing but unmuted.
static bool CheckCopilotDisabled() {
    // Group policy: TurnOffWindowsCopilot=1 under either hive is a hard disable.
    static const HKEY kPolicyHives[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };
    for (HKEY hive : kPolicyHives) {
        HKEY hKey = nullptr;
        if (RegOpenKeyExW(hive,
                L"Software\\Policies\\Microsoft\\Windows\\WindowsCopilot",
                0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD off = 0, cb = sizeof(off);
            LONG r = RegQueryValueExW(hKey, L"TurnOffWindowsCopilot", nullptr, nullptr,
                                      reinterpret_cast<BYTE*>(&off), &cb);
            RegCloseKey(hKey);
            if (r == ERROR_SUCCESS && off != 0) {
                Wh_Log(L"[Copilot] => disabled (TurnOffWindowsCopilot policy)");
                return true;
            }
        }
    }
    // ShowCopilotButton=0 means the user deliberately turned Copilot off in
    // Settings > Personalization > Taskbar. Treat this as the explicit-disable signal,
    // analogous to revoking mic/camera consent.
    DWORD showButton = 1;
    DWORD cbData = sizeof(showButton);
    HKEY hKey = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExW(hKey, L"ShowCopilotButton", nullptr, nullptr,
                         reinterpret_cast<BYTE*>(&showButton), &cbData);
        RegCloseKey(hKey);
    }
    Wh_Log(L"[Copilot] ShowCopilotButton=%u  installed=%d", showButton, g_copilotInstalled.load() ? 1 : 0);
    if (showButton == 0) return true;      // explicitly disabled in Settings
    if (!g_copilotInstalled.load()) return true;  // package not found at all
    return false;
}

// Returns true if location is disabled at either the OS service level or the app consent level.
static bool CheckLocationDisabled() {
    // Check 0: Group Policy hard-disable
    {
        HKEY hk = nullptr;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Policies\\Microsoft\\Windows\\LocationAndSensors",
                0, KEY_READ, &hk) == ERROR_SUCCESS) {
            DWORD val = 0; DWORD sz = sizeof(val);
            LONG r = RegQueryValueExW(hk, L"DisableLocation", nullptr, nullptr, (LPBYTE)&val, &sz);
            RegCloseKey(hk);
            if (r == ERROR_SUCCESS && val != 0) {
                Wh_Log(L"[Loc] => disabled (Group Policy)"); return true;
            }
        }
    }
    // Check 1: Geolocation service master switch (lfsvc service configuration)
    {
        HKEY hk = nullptr;
        LONG openR = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                L"SYSTEM\\CurrentControlSet\\Services\\lfsvc\\Service\\Configuration",
                0, KEY_READ, &hk);
        Wh_Log(L"[Loc] lfsvc key open=%d", openR);
        if (openR == ERROR_SUCCESS) {
            DWORD status = 0xFFFFFFFF, valLen = sizeof(status);
            LONG qr = RegQueryValueExW(hk, L"Status", nullptr, nullptr, (LPBYTE)&status, &valLen);
            RegCloseKey(hk);
            Wh_Log(L"[Loc] lfsvc Status: qr=%d val=%u", qr, status);
            if (qr == ERROR_SUCCESS && status == 0) { Wh_Log(L"[Loc] => disabled (lfsvc)"); return true; }
        }
    }
    // Check 2: per-user consent (Privacy & Security → Location services toggle → HKCU)
    {
        HKEY hk = nullptr;
        LONG openR = RegOpenKeyExW(HKEY_CURRENT_USER,
                L"Software\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager"
                L"\\ConsentStore\\location",
                0, KEY_READ, &hk);
        Wh_Log(L"[Loc] HKCU ConsentStore open=%d", openR);
        if (openR == ERROR_SUCCESS) {
            wchar_t val[64] = {};
            DWORD valLen = sizeof(val), type = 0;
            LONG r = RegQueryValueExW(hk, L"Value", nullptr, &type, (LPBYTE)val, &valLen);
            RegCloseKey(hk);
            Wh_Log(L"[Loc] HKCU Value: r=%d type=%u val='%s'", r, type, val);
            if (r == ERROR_SUCCESS && type == REG_SZ && _wcsicmp(val, L"Deny") == 0) {
                Wh_Log(L"[Loc] => disabled (HKCU consent)"); return true;
            }
        }
    }
    // Check 3: machine-wide policy (HKLM)
    {
        HKEY hk = nullptr;
        LONG openR = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager"
                L"\\ConsentStore\\location",
                0, KEY_READ, &hk);
        Wh_Log(L"[Loc] HKLM ConsentStore open=%d", openR);
        if (openR == ERROR_SUCCESS) {
            wchar_t val[64] = {};
            DWORD valLen = sizeof(val), type = 0;
            LONG r = RegQueryValueExW(hk, L"Value", nullptr, &type, (LPBYTE)val, &valLen);
            RegCloseKey(hk);
            Wh_Log(L"[Loc] HKLM Value: r=%d type=%u val='%s'", r, type, val);
            if (r == ERROR_SUCCESS && type == REG_SZ && _wcsicmp(val, L"Deny") == 0) {
                Wh_Log(L"[Loc] => disabled (HKLM consent)"); return true;
            }
        }
    }
    Wh_Log(L"[Loc] => enabled");
    return false;
}

// In-use detection via CapabilityAccessManager usage records: each app that
// uses a capability gets a subkey under ConsentStore\<capability> (packaged
// apps directly, win32 apps under NonPackaged\) with LastUsedTimeStart /
// LastUsedTimeStop QWORDs. Stop == 0 while Start is set means the app is using
// the capability RIGHT NOW. This is how Settings > Privacy shows "currently in
// use", and it fires for hardware cameras/mics that never get a tray glyph.
static bool ScanConsentUsage(HKEY hive, const std::wstring& path, int depth = 0) {
    if (depth > 1) return false;
    HKEY hk = nullptr;
    if (RegOpenKeyExW(hive, path.c_str(), 0,
                      KEY_READ | KEY_ENUMERATE_SUB_KEYS, &hk) != ERROR_SUCCESS)
        return false;
    bool inUse = false;
    wchar_t name[256];
    for (DWORD i = 0; !inUse; i++) {
        DWORD nameLen = ARRAYSIZE(name);
        LONG e = RegEnumKeyExW(hk, i, name, &nameLen,
                               nullptr, nullptr, nullptr, nullptr);
        if (e == ERROR_NO_MORE_ITEMS) break;
        if (e != ERROR_SUCCESS) continue;
        if (_wcsicmp(name, L"NonPackaged") == 0) {
            inUse = ScanConsentUsage(hive, path + L"\\" + name, depth + 1);
            continue;
        }
        HKEY hApp = nullptr;
        if (RegOpenKeyExW(hk, name, 0, KEY_READ, &hApp) == ERROR_SUCCESS) {
            ULONGLONG start = 0, stop = 0;
            DWORD sz = sizeof(start);
            bool hasStart = RegQueryValueExW(hApp, L"LastUsedTimeStart", nullptr,
                                nullptr, (LPBYTE)&start, &sz) == ERROR_SUCCESS &&
                            start != 0;
            sz = sizeof(stop);
            bool hasStop = RegQueryValueExW(hApp, L"LastUsedTimeStop", nullptr,
                               nullptr, (LPBYTE)&stop, &sz) == ERROR_SUCCESS;
            RegCloseKey(hApp);
            if (hasStart && hasStop && stop == 0) {
                Wh_Log(L"[Usage] %s in use by %s", path.c_str(), name);
                inUse = true;
            }
        }
    }
    RegCloseKey(hk);
    return inUse;
}

static bool CheckCapabilityInUse(PCWSTR capability) {
    std::wstring base =
        L"Software\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager"
        L"\\ConsentStore\\";
    return ScanConsentUsage(HKEY_CURRENT_USER,  base + capability) ||
           ScanConsentUsage(HKEY_LOCAL_MACHINE, base + capability);
}

// Poll all privacy states and push UI update if anything changed.
// Must be called from a thread with COM initialized (COINIT_MULTITHREADED).
static void UpdateDisabledStates() {
    if (g_unloading) return;
    bool locDis  = CheckLocationDisabled();
    bool micDis  = CheckMicDisabled();
    bool camDis  = CheckCameraDisabled();
    bool locUse  = CheckCapabilityInUse(L"location");
    bool micUse  = CheckCapabilityInUse(L"microphone");
    bool camUse  = CheckCapabilityInUse(L"webcam");
    bool copInst = CheckCopilotInstalled();
    bool copAct  = CheckCopilotActive();
    // Update installed first so CheckCopilotDisabled can read it.
    bool prevCopI = g_copilotInstalled.exchange(copInst);
    bool copDis  = CheckCopilotDisabled();
    bool prevLoc  = g_locDisabled.load();
    bool prevMic  = g_micDisabled.load(),  prevCam  = g_camDisabled.load();
    bool prevCopA = g_copilotActive.load();
    bool prevCopD = g_copilotDisabled.load();
    bool changed = (g_locDisabled.exchange(locDis)           != locDis)  ||
                   (g_micDisabled.exchange(micDis)           != micDis)  ||
                   (g_camDisabled.exchange(camDis)           != camDis)  ||
                   (g_locUsage.exchange(locUse)              != locUse)  ||
                   (g_micUsage.exchange(micUse)              != micUse)  ||
                   (g_camUsage.exchange(camUse)              != camUse)  ||
                   (prevCopI                                != copInst) ||
                   (g_copilotActive.exchange(copAct)         != copAct)  ||
                   (g_copilotDisabled.exchange(copDis)       != copDis);
    Wh_Log(L"[Poll] loc %d->%d  mic %d->%d  cam %d->%d  copInst %d->%d  copAct %d->%d  copDis %d->%d  changed=%d",
           prevLoc, locDis, prevMic, micDis, prevCam, camDis,
           prevCopI, copInst, prevCopA, copAct, prevCopD, copDis, changed ? 1 : 0);
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
    bar.Margin({ (double)g_settings.groupPaddingLeft, 0.0,
                 (double)g_settings.groupPaddingRight, 0.0 });
    ApplyOffset(bar, g_settings.groupOffsetX, g_settings.groupOffsetY);

    int N = (int)activeItems.size();

    // Grid shape from the smart-grid template; mode derives from the rows /
    // columns settings (0 = auto).
    grid::Config cfg;
    cfg.fillOrder          = g_settings.fillOrder;
    cfg.shortGroupPosition = g_settings.shortGroupPosition;
    cfg.shortGroupAlign    = g_settings.shortGroupAlign;
    cfg.rows               = g_settings.gridRows;
    cfg.columns            = g_settings.gridColumns;
    if      (g_settings.gridRows > 0 && g_settings.gridColumns > 0)
        cfg.mode = grid::GridMode::FixedGrid;
    else if (g_settings.gridRows > 0)
        cfg.mode = grid::GridMode::FixedRows;
    else if (g_settings.gridColumns > 0)
        cfg.mode = grid::GridMode::FixedColumns;
    else
        cfg.mode = grid::GridMode::AutoSmart;

    // Row capacity from the taskbar height and icon pitch: a single column on
    // double-height taskbars, more columns when the stack doesn't fit.
    {
        int taskbarH = 48;
        HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
        RECT rc{};
        if (hWnd && GetWindowRect(hWnd, &rc))
            taskbarH = (int)(rc.bottom - rc.top);
        int pitch = g_settings.iconSize + std::max(0, g_settings.buttonSpacing);
        cfg.availableRows = std::max(1,
            (taskbarH + std::max(0, g_settings.buttonSpacing)) / std::max(1, pitch));
    }

    grid::Layout layout = grid::ComputeLayout(N, cfg);
    int rows = layout.rows;
    int cols = layout.columns;

    for (int r = 0; r < rows; r++) {
        RowDefinition rd;
        rd.Height({ (double)g_settings.iconSize, GridUnitType::Pixel });
        bar.RowDefinitions().Append(rd);
    }
    for (int c = 0; c < cols; c++) {
        ColumnDefinition bcd;
        bcd.Width({ (double)g_settings.iconSize, GridUnitType::Pixel });
        bar.ColumnDefinitions().Append(bcd);
    }
    if (g_settings.buttonSpacing > 0) {
        bar.ColumnSpacing((double)g_settings.buttonSpacing);
        bar.RowSpacing((double)g_settings.buttonSpacing);
    }

    g_locIcon = nullptr; g_micIcon = nullptr; g_camIcon = nullptr; g_copilotIcon = nullptr;
    g_locGlowIcon = nullptr; g_micGlowIcon = nullptr; g_camGlowIcon = nullptr; g_copilotGlowIcon = nullptr;
    g_locSlashIcon = nullptr; g_micSlashIcon = nullptr; g_camSlashIcon = nullptr; g_copilotSlashIcon = nullptr;

    for (int i = 0; i < N; i++) {
        const auto& token = activeItems[i];
        const wchar_t* glyph    = L"";
        const wchar_t* iconFont = L"Segoe MDL2 Assets";
        bool  isActive, isDisabled;
        int   offX, offY;
        const wchar_t* label;
        const wchar_t* idleLabel     = L"Not requested";
        const wchar_t* disabledLabel = L"Hardware disabled";

        if (token == L"location") {
            glyph        = L"\xE37A";
            isActive     = g_locActive.load();
            isDisabled   = g_locDisabled.load();
            offX         = g_settings.locationOffsetX;
            offY         = g_settings.locationOffsetY;
            label        = L"Location";
        } else if (token == L"mic") {
            glyph        = L"\xE720";
            isActive     = g_micActive.load();
            isDisabled   = g_micDisabled.load();
            offX         = g_settings.micOffsetX;
            offY         = g_settings.micOffsetY;
            label        = L"Microphone";
        } else if (token == L"camera") {
            glyph        = L"\xE722";
            isActive     = g_camActive.load();
            isDisabled   = g_camDisabled.load();
            offX         = g_settings.cameraOffsetX;
            offY         = g_settings.cameraOffsetY;
            label        = L"Camera";
        } else {  // copilot
            isActive      = g_copilotActive.load();
            isDisabled    = g_copilotDisabled.load();
            offX          = g_settings.copilotOffsetX;
            offY          = g_settings.copilotOffsetY;
            label         = L"Copilot";
            idleLabel     = L"Installed (not running)";
            disabledLabel = L"Not installed";
        }

        // Wrap glow + icon + slash overlay in a 1-cell Grid so they overlap (back to front)
        Grid slot;
        slot.HorizontalAlignment(HorizontalAlignment::Center);
        slot.VerticalAlignment(VerticalAlignment::Center);

        winrt::Windows::UI::Color glowColor;
        if (g_settings.activeColorSet) {
            glowColor = g_settings.activeColorValue;
        } else {
            try {
                winrt::Windows::UI::ViewManagement::UISettings ui;
                glowColor = ui.GetColorValue(
                    winrt::Windows::UI::ViewManagement::UIColorType::Accent);
            } catch (...) {
                glowColor = {255, 0, 120, 215};  // Windows blue fallback
            }
        }

        FrameworkElement iconFe = nullptr;
        FrameworkElement glowFe = nullptr;

        if (token == L"copilot") {
            bool isDark = (Application::Current().RequestedTheme() == ApplicationTheme::Dark);
            winrt::Windows::UI::Color neutralColor = isDark
                ? winrt::Windows::UI::Color{255, 255, 255, 255}
                : winrt::Windows::UI::Color{255,  30,  30,  30};

            // Try XamlReader to create the real Microsoft Copilot path icon.
            // F1 = EvenOdd fill rule (creates the inner cutout characteristic of the logo).
            // SVG viewBox is 0 0 24 24; Stretch=Uniform scales to the requested icon size.
            static constexpr wchar_t kPathData[] =
                L"F1 M9 23l.073-.001a2.53 2.53 0 01-2.347-1.838l-.697-2.433"
                L"a2.529 2.529 0 00-2.426-1.839h-.497l-.104-.002"
                L"c-4.485 0-2.935-5.278-1.75-9.225l.162-.525"
                L"C2.412 3.99 3.883 1 6.25 1h8.86"
                L"c1.12 0 2.106.745 2.422 1.829l.715 2.453"
                L"a2.53 2.53 0 002.247 1.823l.147.005.534.001"
                L"c3.557.115 3.088 3.745 2.156 7.206l-.113.413"
                L"c-.154.548-.315 1.089-.47 1.607l-.163.525"
                L"C21.588 20.01 20.116 23 17.75 23h-8.75"
                L"zm8.22-15.89l-3.856.001a2.526 2.526 0 00-2.35 1.615"
                L"L9.21 15.04a2.529 2.529 0 01-2.43 1.847"
                L"l3.853.002c1.056 0 1.992-.661 2.361-1.644"
                L"l1.796-6.287a2.529 2.529 0 012.43-1.848z";

            auto toHexColor = [](winrt::Windows::UI::Color c) -> std::wstring {
                wchar_t buf[10];
                swprintf_s(buf, L"#%02X%02X%02X%02X", c.A, c.R, c.G, c.B);
                return std::wstring(buf);
            };

            auto tryMakePath = [&](double sz, bool isGlow) -> FrameworkElement {
                try {
                    double szN = isGlow ? sz * 1.5 : sz;
                    std::wstring sizeStr = std::to_wstring((int)std::round(szN));
                    std::wstring fillHex = toHexColor(isGlow ? glowColor : neutralColor);
                    std::wstring xaml =
                        std::wstring(
                            L"<Viewbox"
                            L" xmlns=\"http://schemas.microsoft.com/winfx/2006/xaml/presentation\""
                            L" Width=\"") + sizeStr +
                        L"\" Height=\"" + sizeStr +
                        L"\">"
                        L"<Path Width=\"24\" Height=\"24\" Stretch=\"Uniform\""
                        L" HorizontalAlignment=\"Center\" VerticalAlignment=\"Center\""
                        L" IsHitTestVisible=\"False\""
                        L" Fill=\"" + fillHex +
                        L"\" Data=\"" + kPathData +
                        L"\"/>"
                        L"</Viewbox>";
                    auto elem = winrt::Windows::UI::Xaml::Markup::XamlReader::Load(xaml);
                    auto vb = elem.try_as<winrt::Windows::UI::Xaml::Controls::Viewbox>();
                    if (!vb) { Wh_Log(L"[Copilot] XamlReader returned null or non-Viewbox (isGlow=%d)", isGlow); return nullptr; }
                    if (isGlow) {
                        vb.Opacity(g_settings.glowOpacity / 100.0);
                        vb.Visibility(Visibility::Collapsed);
                    }
                    Wh_Log(L"[Copilot] XamlReader OK (isGlow=%d)", isGlow);
                    return vb.try_as<FrameworkElement>();
                } catch (...) {
                    Wh_Log(L"[Copilot] XamlReader threw (isGlow=%d)", isGlow);
                    return nullptr;
                }
            };

            // Fallback: 4-pointed sparkle Polygon when XamlReader fails
            auto makeStar = [&](double sz, bool isGlow) {
                winrt::Windows::UI::Xaml::Shapes::Polygon p;
                double cx = sz / 2.0, w = sz * 0.14;
                p.Points().Append({(float)cx,       0.0f});
                p.Points().Append({(float)(cx + w), (float)(cx - w)});
                p.Points().Append({(float)sz,        (float)cx});
                p.Points().Append({(float)(cx + w), (float)(cx + w)});
                p.Points().Append({(float)cx,        (float)sz});
                p.Points().Append({(float)(cx - w), (float)(cx + w)});
                p.Points().Append({0.0f,             (float)cx});
                p.Points().Append({(float)(cx - w), (float)(cx - w)});
                p.Width(sz); p.Height(sz);
                p.Stretch(winrt::Windows::UI::Xaml::Media::Stretch::Uniform);
                p.HorizontalAlignment(HorizontalAlignment::Center);
                p.VerticalAlignment(VerticalAlignment::Center);
                p.IsHitTestVisible(false);
                SolidColorBrush br;
                br.Color(isGlow ? glowColor : neutralColor);
                p.Fill(br);
                if (isGlow) {
                    p.Opacity(g_settings.glowOpacity / 100.0);
                    p.Visibility(Visibility::Collapsed);
                }
                return p.try_as<FrameworkElement>();
            };

            auto gp = tryMakePath((double)g_settings.iconSize, true);
            if (!gp) gp = makeStar((double)g_settings.iconSize * 1.5, true);
            glowFe = gp; slot.Children().Append(gp);

            auto ip = tryMakePath((double)g_settings.iconSize, false);
            if (!ip) ip = makeStar((double)g_settings.iconSize, false);
            SetIconTooltip(ip, label, isActive, isDisabled, idleLabel, disabledLabel);
            iconFe = ip; slot.Children().Append(ip);
        } else {
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
                glowBrush.Color(glowColor);
                glowTb.Foreground(glowBrush);
            }
            glowTb.Visibility(Visibility::Collapsed);
            glowFe = glowTb;
            slot.Children().Append(glowTb);

            auto tb = MakeIconTextBlock(glyph);
            tb.FontFamily(FontFamily(iconFont));
            SetIconTooltip(tb, label, isActive, isDisabled, idleLabel, disabledLabel);
            iconFe = tb;
            slot.Children().Append(tb);
        }

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
            if (g_settings.slashColorSet) {
                slashBrush.Color(g_settings.slashColorValue);
            } else {
                bool isDark = (Application::Current().RequestedTheme()
                               == ApplicationTheme::Dark);
                slashBrush.Color(isDark ? winrt::Windows::UI::Color{255, 255, 255, 255}
                                        : winrt::Windows::UI::Color{255,  30,  30,  30});
            }
            slashLine.Stroke(slashBrush);
        }
        slashLine.Visibility(isDisabled ? Visibility::Visible : Visibility::Collapsed);
        slot.Children().Append(slashLine);

        ApplyOffset(slot, offX, offY);

        if (token == L"location")      { g_locIcon     = iconFe; g_locGlowIcon     = glowFe; g_locSlashIcon     = slashLine; }
        else if (token == L"mic")      { g_micIcon     = iconFe; g_micGlowIcon     = glowFe; g_micSlashIcon     = slashLine; }
        else if (token == L"camera")   { g_camIcon     = iconFe; g_camGlowIcon     = glowFe; g_camSlashIcon     = slashLine; }
        else /* copilot */             { g_copilotIcon = iconFe; g_copilotGlowIcon = glowFe; g_copilotSlashIcon = slashLine; }

        grid::Cell placement = grid::GetCell(i, N, layout, cfg);
        Grid::SetRow(slot, placement.row);
        Grid::SetColumn(slot, placement.column);
        if (placement.rowSpan > 1) Grid::SetRowSpan(slot, placement.rowSpan);
        if (placement.columnSpan > 1) Grid::SetColumnSpan(slot, placement.columnSpan);
        if (placement.rowSpan > 1)
            slot.VerticalAlignment(VerticalAlignment::Top);
        if (placement.columnSpan > 1)
            slot.HorizontalAlignment(HorizontalAlignment::Left);
        if (placement.topOffsetUnits || placement.leftOffsetUnits) {
            double pitch = g_settings.iconSize + g_settings.buttonSpacing;
            auto margin = slot.Margin();
            margin.Left += placement.leftOffsetUnits * pitch;
            margin.Top += placement.topOffsetUnits * pitch;
            slot.Margin(margin);
        }

        bar.Children().Append(slot);
    }

    Grid::SetColumn(bar, insertCol);
    gridParent.Children().Append(bar);

    g_syntheticGrid   = bar;
    g_syntheticParent = gridElem;
    g_syntheticColumn = insertCol;

    UpdateSyntheticState();
    Wh_Log(L"[Inject] PrivacyAnchorBar: %d icons, %d cols, %d rows", N, cols, rows);
    return true;
}

static void RemoveSyntheticIcons() {
    // XAML defers removed-subtree teardown to a later UI tick, which can land
    // after the mod DLL unloads. The boxed tooltip and automation-name values on
    // the synthetic icons are implemented in this DLL, so release them before
    // removal (same crash class as folder-menus crash-on-disable).
    auto clearIconState = [](FrameworkElement const& fe) {
        if (!fe) return;
        try { ToolTipService::SetToolTip(fe, nullptr); } catch (...) {}
        try {
            fe.ClearValue(winrt::Windows::UI::Xaml::Automation::
                          AutomationProperties::NameProperty());
        } catch (...) {}
    };
    clearIconState(g_locIcon);
    clearIconState(g_micIcon);
    clearIconState(g_camIcon);
    clearIconState(g_copilotIcon);

    auto gridParent = g_syntheticParent ? g_syntheticParent.try_as<Grid>() : nullptr;
    if (!gridParent) {
        g_syntheticGrid    = nullptr;
        g_locIcon = nullptr; g_micIcon = nullptr; g_camIcon = nullptr; g_copilotIcon = nullptr;
        g_locGlowIcon = nullptr; g_micGlowIcon = nullptr; g_camGlowIcon = nullptr; g_copilotGlowIcon = nullptr;
        g_locSlashIcon = nullptr; g_micSlashIcon = nullptr; g_camSlashIcon = nullptr; g_copilotSlashIcon = nullptr;
        g_syntheticParent  = nullptr; g_syntheticColumn = -1;
        return;
    }

    int col = g_syntheticColumn;
    for (uint32_t i = 0; i < gridParent.Children().Size(); i++) {
        auto fe = gridParent.Children().GetAt(i).try_as<FrameworkElement>();
        if (fe && fe.Name() == L"PrivacyAnchorBar") {
            col = Grid::GetColumn(fe);
            gridParent.Children().RemoveAt(i);
            break;
        }
    }

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
                        if (s.type != detectedType)
                            SetPrivacyActive(s.type, false);
                        s.type = detectedType;
                        break;
                    }
                }
                SetPrivacyActive(detectedType, true);
            }
        });

    state.visibilityToken = iconView.RegisterPropertyChangedCallback(
        UIElement::VisibilityProperty(),
        [](DependencyObject sender, DependencyProperty) {
            if (g_unloading) return;
            if (!g_settings.suppressNativeIndicators) return;
            auto iconView = sender.try_as<FrameworkElement>();
            if (!iconView || iconView.Visibility() == Visibility::Collapsed) return;
            iconView.Visibility(Visibility::Collapsed);
            iconView.IsHitTestVisible(false);
        });

    g_privacyStates.push_back(std::move(state));
    if (g_settings.suppressNativeIndicators) {
        iconView.Visibility(Visibility::Collapsed);
        iconView.IsHitTestVisible(false);
    } else {
        iconView.IsHitTestVisible(true);
    }
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
        if (auto iv = state.iconViewRef.get()) {
            if (state.visibilityToken)
                iv.UnregisterPropertyChangedCallback(UIElement::VisibilityProperty(), state.visibilityToken);
            try {
                auto tb = state.textBlockRef.get();
                bool active = tb && !std::wstring_view(tb.Text()).empty();
                iv.IsHitTestVisible(true);
                iv.Visibility(active ? Visibility::Visible : Visibility::Collapsed);
            } catch (...) {}
        }
    }
    g_privacyStates.clear();
    g_locActive.store(false);
    g_micActive.store(false);
    g_camActive.store(false);
    g_locUsage.store(false);
    g_micUsage.store(false);
    g_camUsage.store(false);
    g_locDisabled.store(false);
    g_micDisabled.store(false);
    g_camDisabled.store(false);
    g_copilotInstalled.store(false);
    g_copilotActive.store(false);
    g_copilotDisabled.store(true);
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
    if (g_stateRefreshEvent) SetEvent(g_stateRefreshEvent);
    if (g_retryThread) {
        DWORD result;
        do {
            result = MsgWaitForMultipleObjects(
                1, &g_retryThread, FALSE, INFINITE, QS_SENDMESSAGE);
            if (result == WAIT_OBJECT_0 + 1) {
                MSG message;
                PeekMessageW(&message, nullptr, 0, 0, PM_NOREMOVE);
            }
        } while (result == WAIT_OBJECT_0 + 1);
        CloseHandle(g_retryThread);
        g_retryThread = nullptr;
    }
    if (g_retryStopEvent) {
        CloseHandle(g_retryStopEvent);
        g_retryStopEvent = nullptr;
    }
    if (g_stateRefreshEvent) {
        CloseHandle(g_stateRefreshEvent);
        g_stateRefreshEvent = nullptr;
    }
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
    if (h && path)
        HandleLoadedModuleIfSystemTray(h, path);
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

static bool HookSystemTraySymbols(HMODULE h) {
    WindhawkUtils::SYMBOL_HOOK systemTrayHooks[] = {{
        {LR"(public: __cdecl winrt::SystemTray::implementation::IconView::IconView(void))"},
        &IconView_IconView_Original,
        IconView_IconView_Hook,
        false,
    }};
    return WindhawkUtils::HookSymbols(
        h, systemTrayHooks, ARRAYSIZE(systemTrayHooks));
}

static void HandleLoadedModuleIfSystemTray(HMODULE module,
                                            LPCWSTR fileName) {
    if (!g_systemTrayModuleHooked &&
        GetSystemTrayModuleHandle() == module &&
        !g_systemTrayModuleHooked.exchange(true)) {
        Wh_Log(L"[Hooks] System tray module loaded: %s", fileName);
        if (HookSystemTraySymbols(module)) {
            Wh_ApplyHookOperations();
        } else {
            g_systemTrayModuleHooked = false;
            Wh_Log(L"[Hooks] System tray symbol hooks failed");
        }
    }
}

// ============================================================
// Windhawk lifecycle
// ============================================================

BOOL Wh_ModInit() {
    Wh_Log(L"[Init] Privacy Anchor v0.9");
    LoadSettings();

    if (!HookTaskbarDllSymbols()) {
        Wh_Log(L"[Init] taskbar.dll symbols failed");
        return FALSE;
    }

    if (HMODULE module = GetSystemTrayModuleHandle()) {
        if (!HookSystemTraySymbols(module)) {
            Wh_Log(L"[Init] System tray symbol hooks failed");
            return FALSE;
        }
        g_systemTrayModuleHooked = true;
    } else {
        HMODULE kernelbase = GetModuleHandleW(L"kernelbase.dll");
        auto loadLibraryExW = kernelbase
            ? reinterpret_cast<LoadLibraryExW_t>(
                  GetProcAddress(kernelbase, "LoadLibraryExW"))
            : nullptr;
        if (!loadLibraryExW ||
            !WindhawkUtils::SetFunctionHook(
                loadLibraryExW,
                LoadLibraryExW_Hook,
                &LoadLibraryExW_Original)) {
            Wh_Log(L"[Init] LoadLibraryExW hook unavailable");
            return FALSE;
        }
    }

    return TRUE;
}

void Wh_ModAfterInit() {
    if (!g_systemTrayModuleHooked) {
        if (HMODULE module = GetSystemTrayModuleHandle()) {
            if (!g_systemTrayModuleHooked.exchange(true)) {
                if (HookSystemTraySymbols(module))
                    Wh_ApplyHookOperations();
                else
                    g_systemTrayModuleHooked = false;
            }
        }
    }
    if (g_systemTrayModuleHooked)
        ApplyStyleOnWindowThread();

    g_retryStopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_stateRefreshEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!g_retryStopEvent || !g_stateRefreshEvent) {
        if (g_retryStopEvent) { CloseHandle(g_retryStopEvent); g_retryStopEvent = nullptr; }
        if (g_stateRefreshEvent) { CloseHandle(g_stateRefreshEvent); g_stateRefreshEvent = nullptr; }
        return;
    }
    g_retryThread = CreateThread(nullptr, 0, [](void* param) -> DWORD {
        UNREFERENCED_PARAMETER(param);
        HANDLE stop = g_retryStopEvent;
        HANDLE refresh = g_stateRefreshEvent;
        // Phase 1: retry injection up to 5×
        for (int i = 0; i < 5 && !g_unloading; i++) {
            if (WaitForSingleObject(stop, 2000) != WAIT_TIMEOUT) return 0;
            if (g_syntheticGrid) break;
            Wh_Log(L"[AfterInit] Retry %d", i + 1);
            ApplyStyleOnWindowThread();
        }
        // Phase 2: poll hardware-disabled states every 3 seconds
        if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) return 0;
        MicPrivacyMonitor micMonitor(refresh);
        micMonitor.Init();
        Wh_Log(L"[Poll] Phase 2 starting — initial hardware state check");
        UpdateDisabledStates();
        Wh_Log(L"[Poll] Baseline: loc=%d mic=%d cam=%d copInst=%d copAct=%d copDis=%d",
               g_locDisabled.load(), g_micDisabled.load(), g_camDisabled.load(),
               g_copilotInstalled.load(), g_copilotActive.load(), g_copilotDisabled.load());
        HANDLE waitEvents[] = { stop, refresh };
        while (!g_unloading) {
            DWORD wait = WaitForMultipleObjects(ARRAYSIZE(waitEvents), waitEvents, FALSE, 3000);
            if (wait == WAIT_OBJECT_0) break;
            if (wait == WAIT_OBJECT_0 + 1) Wh_Log(L"[Poll] Native refresh event");
            UpdateDisabledStates();
        }
        micMonitor.Cleanup();
        CoUninitialize();
        return 0;
    }, nullptr, 0, nullptr);
    if (!g_retryThread) {
        CloseHandle(g_retryStopEvent); g_retryStopEvent = nullptr;
        CloseHandle(g_stateRefreshEvent); g_stateRefreshEvent = nullptr;
    }
}

void Wh_ModUninit() {
    g_unloading = true;
    Wh_Log(L"[Uninit]");
    StopRetryThread();
    // Loaded revokers wrap WinRT objects that must be destroyed on the UI
    // thread — clear them inside RunFromWindowThread, not here.
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (hWnd) {
        RunFromWindowThread(hWnd, [](void*) {
            g_loadedRevokers.clear();
            ClearPrivacyStates();
            RemoveSyntheticIcons();
        }, nullptr);
    } else {
        g_loadedRevokers.clear();
        ClearPrivacyStates();
        RemoveSyntheticIcons();
    }
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Wh_Log(L"[Settings] order=%s rows=%d cols=%d suppressNative=%d",
           g_settings.itemOrder.c_str(), g_settings.gridRows,
           g_settings.gridColumns, g_settings.suppressNativeIndicators ? 1 : 0);

    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return;
    RunFromWindowThread(hWnd, [](void*) {
        ClearPrivacyStates();
        RemoveSyntheticIcons();
        ApplyStyle();
    }, nullptr);
}