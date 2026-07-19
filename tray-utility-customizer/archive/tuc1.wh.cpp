// ==WindhawkMod==
// @id              tray-utility-customizer
// @name            Tray Utility Customizer
// @description     Arranges Windows tray utility controls such as Show hidden icons, Emoji, touch keyboard, pen menu, and virtual touchpad into a configurable row, column, or grid.
// @version         0.3
// @author          sb4ssman
// @github          https://github.com/sb4ssman
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lversion
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Tray Utility Customizer

Arranges low-frequency Windows 11 system-tray utility controls into one compact
row, column, or grid:

- **Show hidden icons** (the overflow chevron)
- **Emoji and more** (emoji, GIF, kaomoji, symbols, and clipboard history)
- **Touch keyboard**
- **Pen menu**
- **Virtual touchpad**
- **Input/language indicator**

Only controls detected on the current Windows build are included. The mod keeps
Windows-owned controls intact and moves their native tray hosts instead of drawing
replacement buttons or forwarding clicks.

## Detection

- **Automatic** uses Windows accessibility metadata and a guarded Emoji fallback.
- **Force MainStack** allows the complete native `MainStack` to participate when
  Windows doesn't expose useful metadata. It can include unrelated indicators.

## Layout and position

Use `itemOrder` to select and order utilities. Choose a row, column, grid, or
automatic layout. The group can borrow the original hidden-icons or Emoji column,
or reserve a dedicated column at several tray positions.

If multiple selected utilities belong to one indivisible Windows host, they stay
bundled together and the log identifies the shared host.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- position: overflow
  $name: Group position
  $description: >-
    Borrow a native utility column or insert a dedicated column elsewhere in
    the system tray.
  $options:
  - overflow: Hidden-icons column
  - emoji: Emoji column
  - beforeIcons: Before notification icons
  - beforeOmni: Before Wi-Fi/volume/battery
  - beforeClock: Before clock
  - afterClock: After clock
  - afterShowDesktop: After Show Desktop strip

- enabled: true
  $name: Enable utility layout

- itemOrder: "overflow,emoji"
  $name: Utility items and order
  $description: >-
    Comma-separated utility tokens. Remove a token to exclude it. Only controls
    detected on this Windows build are included. Valid tokens: overflow, emoji,
    touchKeyboard, penMenu, virtualTouchpad, inputIndicator.

- layoutMode: column
  $name: Layout
  $options:
  - auto: Automatic
  - row: Row
  - column: Column
  - grid: Grid

- gridColumns: 0
  $name: Grid columns (0 = auto)

- gridRows: 0
  $name: Grid rows (0 = auto)

- fillOrder: rowFirst
  $name: Grid fill order
  $options:
  - rowFirst: Row-first
  - columnFirst: Column-first

- shortGroupAlign: center
  $name: Short row/column alignment
  $description: Alignment when the final grid row or column is not full.
  $options:
  - start: Start
  - center: Center
  - end: End

- buttonWidth: 24
  $name: Button width (px)

- buttonHeight: 24
  $name: Button height (px)

- buttonSpacing: 0
  $name: Button spacing (px)
  $description: Horizontal and vertical gap between utility cells.

- overflowOffsetX: 0
  $name: Hidden icons X offset (px)

- overflowOffsetY: 0
  $name: Hidden icons Y offset (px)

- emojiOffsetX: 0
  $name: Emoji X offset (px)

- emojiOffsetY: 0
  $name: Emoji Y offset (px)

- touchKeyboardOffsetX: 0
  $name: Touch keyboard X offset (px)

- touchKeyboardOffsetY: 0
  $name: Touch keyboard Y offset (px)

- penMenuOffsetX: 0
  $name: Pen menu X offset (px)

- penMenuOffsetY: 0
  $name: Pen menu Y offset (px)

- virtualTouchpadOffsetX: 0
  $name: Virtual touchpad X offset (px)

- virtualTouchpadOffsetY: 0
  $name: Virtual touchpad Y offset (px)

- inputIndicatorOffsetX: 0
  $name: Input indicator X offset (px)

- inputIndicatorOffsetY: 0
  $name: Input indicator Y offset (px)

- groupOffsetX: 0
  $name: Group X offset (px)

- groupOffsetY: 0
  $name: Group Y offset (px)

- minimumTrayHeight: 44
  $name: Minimum tray height (px)
  $description: >-
    Below this height the mod leaves the native layout unchanged. Use 0 to allow
    stacking on any taskbar height.

- mergeMode: auto
  $name: Detection mode
  $description: >-
    Automatic is guarded and recommended. Force allows the complete native
    MainStack to be moved when Windows doesn't identify its controls.
  $options:
  - auto: Automatic
  - forceMainStack: Force MainStack (experimental)

- detailedLogging: true
  $name: Detailed discovery logging
  $description: >-
    Logs tray host names, classes, columns, visible IconView counts, and the
    accessibility metadata used to locate Emoji and more.
*/
// ==/WindhawkModSettings==

#include <atomic>
#include <algorithm>
#include <cmath>
#include <cwctype>
#include <functional>
#include <initializer_list>
#include <list>
#include <string>
#include <vector>

#include <windows.h>
#include <windhawk_api.h>
#include <windhawk_utils.h>

#undef GetCurrentTime

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Automation.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Automation;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

enum class MergeMode {
    Auto,
    ForceMainStack,
};

enum class LayoutMode {
    Auto,
    Row,
    Column,
    Grid,
};

enum class FillOrder {
    RowFirst,
    ColumnFirst,
};

enum class ShortAlign {
    Start,
    Center,
    End,
};

enum class Position {
    Overflow,
    Emoji,
    BeforeIcons,
    BeforeOmni,
    BeforeClock,
    AfterClock,
    AfterShowDesktop,
};

struct Settings {
    bool enabled;
    MergeMode mergeMode;
    std::wstring itemOrder;
    LayoutMode layoutMode;
    int gridColumns;
    int gridRows;
    FillOrder fillOrder;
    ShortAlign shortGroupAlign;
    Position position;
    int buttonWidth;
    int buttonHeight;
    int buttonSpacing;
    int minimumTrayHeight;
    int groupOffsetX;
    int groupOffsetY;
    int overflowOffsetX;
    int overflowOffsetY;
    int emojiOffsetX;
    int emojiOffsetY;
    int touchKeyboardOffsetX;
    int touchKeyboardOffsetY;
    int penMenuOffsetX;
    int penMenuOffsetY;
    int virtualTouchpadOffsetX;
    int virtualTouchpadOffsetY;
    int inputIndicatorOffsetX;
    int inputIndicatorOffsetY;
    bool detailedLogging;
};

static Settings g_settings{};
static std::atomic<bool> g_unloading = false;
static std::atomic<bool> g_systemTrayModuleHooked = false;
static HWND g_taskbarWnd = nullptr;
static HANDLE g_retryStopEvent = nullptr;
static HANDLE g_retryThread = nullptr;
static std::list<FrameworkElement::Loaded_revoker> g_loadedRevokers;

struct HostSnapshot {
    FrameworkElement element{nullptr};
    FrameworkElement columnMarker{nullptr};
    int column = 0;
    int columnSpan = 1;
    int row = 0;
    int rowSpan = 1;
    double width = NAN;
    double height = NAN;
    double minWidth = 0;
    double minHeight = 0;
    double maxWidth = INFINITY;
    double maxHeight = INFINITY;
    Thickness margin{};
    HorizontalAlignment horizontalAlignment = HorizontalAlignment::Stretch;
    VerticalAlignment verticalAlignment = VerticalAlignment::Stretch;
    Transform renderTransform{nullptr};
};

struct UtilityHost {
    std::wstring token;
    FrameworkElement element{nullptr};
    int offsetX = 0;
    int offsetY = 0;
};

static std::vector<HostSnapshot> g_hostSnapshots;
static bool g_layoutApplied = false;
static bool g_insertedColumn = false;
static int g_layoutColumn = -1;
static Grid g_layoutGrid{nullptr};
static FrameworkElement g_layoutColumnMarker{nullptr};

static constexpr PCWSTR kLayoutColumnMarkerName =
    L"TrayUtilityCustomizerColumnMarker";

static int ClampSetting(int value, int low, int high) {
    return value < low ? low : value > high ? high : value;
}

static std::wstring GetStringSetting(PCWSTR key) {
    PCWSTR value = Wh_GetStringSetting(key);
    std::wstring result = value ? value : L"";
    if (value) {
        Wh_FreeStringSetting(value);
    }
    return result;
}

static void LoadSettings() {
    g_settings.enabled = Wh_GetIntSetting(L"enabled") != 0;

    auto mergeMode = GetStringSetting(L"mergeMode");
    if (mergeMode == L"forceMainStack") {
        g_settings.mergeMode = MergeMode::ForceMainStack;
    } else {
        g_settings.mergeMode = MergeMode::Auto;
    }

    g_settings.itemOrder = GetStringSetting(L"itemOrder");
    if (g_settings.itemOrder.empty()) {
        g_settings.itemOrder = L"overflow,emoji";
    } else if (g_settings.itemOrder == L"overflowFirst") {
        g_settings.itemOrder = L"overflow,emoji";
    } else if (g_settings.itemOrder == L"emojiFirst") {
        g_settings.itemOrder = L"emoji,overflow";
    }

    auto layoutMode = GetStringSetting(L"layoutMode");
    if (layoutMode == L"row") {
        g_settings.layoutMode = LayoutMode::Row;
    } else if (layoutMode == L"grid") {
        g_settings.layoutMode = LayoutMode::Grid;
    } else if (layoutMode == L"auto") {
        g_settings.layoutMode = LayoutMode::Auto;
    } else {
        g_settings.layoutMode = LayoutMode::Column;
    }

    g_settings.gridColumns =
        ClampSetting(Wh_GetIntSetting(L"gridColumns"), 0, 8);
    g_settings.gridRows =
        ClampSetting(Wh_GetIntSetting(L"gridRows"), 0, 8);
    g_settings.fillOrder =
        GetStringSetting(L"fillOrder") == L"columnFirst"
            ? FillOrder::ColumnFirst
            : FillOrder::RowFirst;
    auto shortAlign = GetStringSetting(L"shortGroupAlign");
    if (shortAlign == L"start") {
        g_settings.shortGroupAlign = ShortAlign::Start;
    } else if (shortAlign == L"end") {
        g_settings.shortGroupAlign = ShortAlign::End;
    } else {
        g_settings.shortGroupAlign = ShortAlign::Center;
    }

    auto position = GetStringSetting(L"position");
    if (position == L"emoji") {
        g_settings.position = Position::Emoji;
    } else if (position == L"beforeIcons") {
        g_settings.position = Position::BeforeIcons;
    } else if (position == L"beforeOmni") {
        g_settings.position = Position::BeforeOmni;
    } else if (position == L"beforeClock") {
        g_settings.position = Position::BeforeClock;
    } else if (position == L"afterClock") {
        g_settings.position = Position::AfterClock;
    } else if (position == L"afterShowDesktop") {
        g_settings.position = Position::AfterShowDesktop;
    } else {
        g_settings.position = Position::Overflow;
    }
    g_settings.buttonWidth =
        ClampSetting(Wh_GetIntSetting(L"buttonWidth"), 16, 96);
    g_settings.buttonHeight =
        ClampSetting(Wh_GetIntSetting(L"buttonHeight"), 12, 64);
    g_settings.buttonSpacing =
        ClampSetting(Wh_GetIntSetting(L"buttonSpacing"), -16, 32);
    g_settings.minimumTrayHeight =
        ClampSetting(Wh_GetIntSetting(L"minimumTrayHeight"), 0, 160);
    g_settings.groupOffsetX =
        ClampSetting(Wh_GetIntSetting(L"groupOffsetX"), -100, 100);
    g_settings.groupOffsetY =
        ClampSetting(Wh_GetIntSetting(L"groupOffsetY"), -100, 100);
    g_settings.overflowOffsetX =
        ClampSetting(Wh_GetIntSetting(L"overflowOffsetX"), -100, 100);
    g_settings.overflowOffsetY =
        ClampSetting(Wh_GetIntSetting(L"overflowOffsetY"), -100, 100);
    g_settings.emojiOffsetX =
        ClampSetting(Wh_GetIntSetting(L"emojiOffsetX"), -100, 100);
    g_settings.emojiOffsetY =
        ClampSetting(Wh_GetIntSetting(L"emojiOffsetY"), -100, 100);
    g_settings.touchKeyboardOffsetX =
        ClampSetting(Wh_GetIntSetting(L"touchKeyboardOffsetX"), -100, 100);
    g_settings.touchKeyboardOffsetY =
        ClampSetting(Wh_GetIntSetting(L"touchKeyboardOffsetY"), -100, 100);
    g_settings.penMenuOffsetX =
        ClampSetting(Wh_GetIntSetting(L"penMenuOffsetX"), -100, 100);
    g_settings.penMenuOffsetY =
        ClampSetting(Wh_GetIntSetting(L"penMenuOffsetY"), -100, 100);
    g_settings.virtualTouchpadOffsetX =
        ClampSetting(Wh_GetIntSetting(L"virtualTouchpadOffsetX"), -100, 100);
    g_settings.virtualTouchpadOffsetY =
        ClampSetting(Wh_GetIntSetting(L"virtualTouchpadOffsetY"), -100, 100);
    g_settings.inputIndicatorOffsetX =
        ClampSetting(Wh_GetIntSetting(L"inputIndicatorOffsetX"), -100, 100);
    g_settings.inputIndicatorOffsetY =
        ClampSetting(Wh_GetIntSetting(L"inputIndicatorOffsetY"), -100, 100);
    g_settings.detailedLogging =
        Wh_GetIntSetting(L"detailedLogging") != 0;
}

using RunFromWindowThreadProc_t = void (*)(void*);

static bool RunFromWindowThread(HWND hWnd,
                                RunFromWindowThreadProc_t proc,
                                void* procParam) {
    static const UINT message =
        RegisterWindowMessage(L"Windhawk_RunFromWindowThread_" WH_MOD_ID);
    struct Param {
        RunFromWindowThreadProc_t proc;
        void* procParam;
    };

    DWORD threadId = GetWindowThreadProcessId(hWnd, nullptr);
    if (!threadId) {
        return false;
    }
    if (threadId == GetCurrentThreadId()) {
        proc(procParam);
        return true;
    }

    HHOOK hook = SetWindowsHookEx(
        WH_CALLWNDPROC,
        [](int code, WPARAM wParam, LPARAM lParam) -> LRESULT {
            if (code == HC_ACTION) {
                const CWPSTRUCT* cwp =
                    reinterpret_cast<const CWPSTRUCT*>(lParam);
                if (cwp->message == RegisterWindowMessageW(
                                        L"Windhawk_RunFromWindowThread_" WH_MOD_ID)) {
                    auto* param = reinterpret_cast<Param*>(cwp->lParam);
                    param->proc(param->procParam);
                }
            }
            return CallNextHookEx(nullptr, code, wParam, lParam);
        },
        nullptr,
        threadId);
    if (!hook) {
        return false;
    }

    Param param{proc, procParam};
    DWORD_PTR messageResult = 0;
    bool sent =
        SendMessageTimeoutW(
            hWnd,
            message,
            0,
            reinterpret_cast<LPARAM>(&param),
            SMTO_ABORTIFHUNG | SMTO_BLOCK,
            3000,
            &messageResult) != 0;
    UnhookWindowsHookEx(hook);
    return sent;
}

static HWND FindCurrentProcessTaskbarWnd() {
    HWND result = nullptr;
    EnumWindows(
        [](HWND hWnd, LPARAM lParam) -> BOOL {
            DWORD processId = 0;
            WCHAR className[32]{};
            if (GetWindowThreadProcessId(hWnd, &processId) &&
                processId == GetCurrentProcessId() &&
                GetClassName(hWnd, className, ARRAYSIZE(className)) &&
                _wcsicmp(className, L"Shell_TrayWnd") == 0) {
                *reinterpret_cast<HWND*>(lParam) = hWnd;
                return FALSE;
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&result));
    return result;
}

using CTaskBand_GetTaskbarHost_t =
    void* (WINAPI*)(void* pThis, void* taskbarHostSharedPtr);
static CTaskBand_GetTaskbarHost_t CTaskBand_GetTaskbarHost_Original;

using TaskbarHost_FrameHeight_t = int (WINAPI*)(void* pThis);
static TaskbarHost_FrameHeight_t TaskbarHost_FrameHeight_Original;

using std__Ref_count_base__Decref_t = void (WINAPI*)(void* pThis);
static std__Ref_count_base__Decref_t std__Ref_count_base__Decref_Original;

static void* CTaskBand_ITaskListWndSite_vftable = nullptr;

static XamlRoot GetTaskbarXamlRoot(HWND hTaskbarWnd) {
    if (!CTaskBand_GetTaskbarHost_Original ||
        !TaskbarHost_FrameHeight_Original ||
        !std__Ref_count_base__Decref_Original ||
        !CTaskBand_ITaskListWndSite_vftable) {
        return nullptr;
    }

    HWND taskSwWnd =
        reinterpret_cast<HWND>(GetProp(hTaskbarWnd, L"TaskbandHWND"));
    if (!taskSwWnd) {
        return nullptr;
    }

    void* taskBand =
        reinterpret_cast<void*>(GetWindowLongPtr(taskSwWnd, 0));
    if (!taskBand) {
        return nullptr;
    }

    void* taskBandForSite = taskBand;
    for (int i = 0;
         *reinterpret_cast<void**>(taskBandForSite) !=
             CTaskBand_ITaskListWndSite_vftable;
         i++) {
        if (i == 20) {
            return nullptr;
        }
        taskBandForSite =
            reinterpret_cast<void**>(taskBandForSite) + 1;
    }

    void* taskbarHostSharedPtr[2]{};
    CTaskBand_GetTaskbarHost_Original(taskBandForSite,
                                     taskbarHostSharedPtr);
    if (!taskbarHostSharedPtr[0] || !taskbarHostSharedPtr[1]) {
        if (taskbarHostSharedPtr[1]) {
            std__Ref_count_base__Decref_Original(
                taskbarHostSharedPtr[1]);
        }
        return nullptr;
    }

    size_t offset = 0x10;
    const BYTE* bytes =
        reinterpret_cast<const BYTE*>(TaskbarHost_FrameHeight_Original);
    if (bytes[0] == 0x48 && bytes[1] == 0x83 &&
        bytes[2] == 0xEC && bytes[4] == 0x48 &&
        bytes[5] == 0x83 && bytes[6] == 0xC1 &&
        bytes[7] <= 0x7F) {
        offset = bytes[7];
    } else {
        Wh_Log(L"[XamlRoot] Unsupported TaskbarHost::FrameHeight");
    }

    auto* unknown = *reinterpret_cast<IUnknown**>(
        reinterpret_cast<BYTE*>(taskbarHostSharedPtr[0]) + offset);
    if (!unknown) {
        std__Ref_count_base__Decref_Original(taskbarHostSharedPtr[1]);
        return nullptr;
    }

    FrameworkElement taskbarElement = nullptr;
    unknown->QueryInterface(winrt::guid_of<FrameworkElement>(),
                            winrt::put_abi(taskbarElement));
    auto result =
        taskbarElement ? taskbarElement.XamlRoot() : nullptr;
    std__Ref_count_base__Decref_Original(taskbarHostSharedPtr[1]);
    return result;
}

static FrameworkElement FindChildRecursive(
    FrameworkElement const& element,
    std::function<bool(FrameworkElement)> const& predicate,
    int maxDepth = 20) {
    if (!element || maxDepth <= 0) {
        return nullptr;
    }

    int count = VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(element, i)
                         .try_as<FrameworkElement>();
        if (!child) {
            continue;
        }
        if (predicate(child)) {
            return child;
        }
        auto found =
            FindChildRecursive(child, predicate, maxDepth - 1);
        if (found) {
            return found;
        }
    }
    return nullptr;
}

static std::wstring ToLower(std::wstring value) {
    for (auto& ch : value) {
        ch = static_cast<wchar_t>(towlower(ch));
    }
    return value;
}

static std::wstring GetElementMetadata(FrameworkElement const& element) {
    try {
        return ToLower(
            std::wstring(element.Name()) + L" " +
            std::wstring(AutomationProperties::GetAutomationId(element)) +
            L" " +
            std::wstring(AutomationProperties::GetName(element)) +
            L" " +
            std::wstring(AutomationProperties::GetHelpText(element)));
    } catch (...) {
        return L"";
    }
}

static bool MetadataContainsAny(
    FrameworkElement const& element,
    std::initializer_list<PCWSTR> terms) {
    auto metadata = GetElementMetadata(element);
    for (auto term : terms) {
        if (metadata.find(term) != std::wstring::npos) {
            return true;
        }
    }
    return false;
}

static FrameworkElement FindDirectTrayHost(
    Grid const& trayGrid,
    FrameworkElement element) {
    DependencyObject current = element;
    while (current) {
        auto parent = VisualTreeHelper::GetParent(current);
        if (!parent) {
            return nullptr;
        }
        if (parent == trayGrid) {
            return current.try_as<FrameworkElement>();
        }
        current = parent;
    }
    return nullptr;
}

static FrameworkElement FindDirectChildByName(
    Grid const& trayGrid,
    PCWSTR name) {
    auto children = trayGrid.Children();
    for (uint32_t i = 0; i < children.Size(); i++) {
        auto element = children.GetAt(i).try_as<FrameworkElement>();
        if (element && element.Name() == name) {
            return element;
        }
    }
    return nullptr;
}

static std::vector<std::wstring> ParseItemOrder(
    std::wstring value) {
    for (auto& ch : value) {
        if (ch == L',' || ch == L';' || iswspace(ch)) {
            ch = L' ';
        }
    }

    std::vector<std::wstring> result;
    size_t position = 0;
    while (position < value.size()) {
        while (position < value.size() &&
               iswspace(value[position])) {
            position++;
        }
        size_t end = position;
        while (end < value.size() && !iswspace(value[end])) {
            end++;
        }
        if (end > position) {
            auto token = value.substr(position, end - position);
            if (std::find(result.begin(), result.end(), token) ==
                result.end()) {
                result.push_back(std::move(token));
            }
        }
        position = end;
    }
    return result;
}

static int CountVisibleIconViews(FrameworkElement const& host) {
    int result = 0;
    std::function<void(FrameworkElement const&, int)> visit =
        [&](FrameworkElement const& element, int depth) {
            if (!element || depth > 12) {
                return;
            }
            int count = VisualTreeHelper::GetChildrenCount(element);
            for (int i = 0; i < count; i++) {
                auto child = VisualTreeHelper::GetChild(element, i)
                                 .try_as<FrameworkElement>();
                if (!child) {
                    continue;
                }
                if (winrt::get_class_name(child) ==
                        L"SystemTray.IconView" &&
                    child.Visibility() == Visibility::Visible) {
                    result++;
                }
                visit(child, depth + 1);
            }
        };
    visit(host, 0);
    return result;
}

static FrameworkElement FindUtilityElement(
    Grid const& trayGrid,
    std::wstring const& token) {
    return FindChildRecursive(
        trayGrid,
        [&](FrameworkElement element) {
            if (token == L"emoji") {
                return MetadataContainsAny(
                    element, {L"emoji", L"kaomoji"});
            }
            if (token == L"touchKeyboard") {
                return MetadataContainsAny(
                    element, {L"touch keyboard"});
            }
            if (token == L"penMenu") {
                return MetadataContainsAny(
                    element, {L"pen menu"});
            }
            if (token == L"virtualTouchpad") {
                return MetadataContainsAny(
                    element, {L"virtual touchpad"});
            }
            if (token == L"inputIndicator") {
                return MetadataContainsAny(
                    element,
                    {L"input indicator", L"language bar",
                     L"input language"});
            }
            return false;
        });
}

static std::vector<UtilityHost> DiscoverUtilityHosts(
    Grid const& trayGrid,
    FrameworkElement overflowHost,
    FrameworkElement mainStack) {
    std::vector<UtilityHost> result;
    auto tokens = ParseItemOrder(g_settings.itemOrder);

    for (auto const& token : tokens) {
        FrameworkElement host = nullptr;
        if (token == L"overflow") {
            host = overflowHost;
        } else {
            auto element = FindUtilityElement(trayGrid, token);
            if (element) {
                host = FindDirectTrayHost(trayGrid, element);
            }

            if (!host && token == L"emoji" && mainStack) {
                int visibleIcons = CountVisibleIconViews(mainStack);
                if (g_settings.mergeMode == MergeMode::ForceMainStack ||
                    visibleIcons == 1) {
                    host = mainStack;
                    Wh_Log(
                        L"[Discover] Emoji using MainStack fallback "
                        L"(visibleIcons=%d force=%d)",
                        visibleIcons,
                        g_settings.mergeMode ==
                            MergeMode::ForceMainStack);
                }
            }
        }

        if (!host) {
            Wh_Log(L"[Discover] Utility not found: %s",
                   token.c_str());
            continue;
        }

        auto existing = std::find_if(
            result.begin(),
            result.end(),
            [&](UtilityHost const& utility) {
                return utility.element == host;
            });
        if (existing != result.end()) {
            Wh_Log(
                L"[Discover] %s shares native host with %s; "
                L"keeping the host bundled",
                token.c_str(),
                existing->token.c_str());
            continue;
        }

        UtilityHost utility;
        utility.token = token;
        utility.element = host;
        if (token == L"overflow") {
            utility.offsetX = g_settings.overflowOffsetX;
            utility.offsetY = g_settings.overflowOffsetY;
        } else if (token == L"emoji") {
            utility.offsetX = g_settings.emojiOffsetX;
            utility.offsetY = g_settings.emojiOffsetY;
        } else if (token == L"touchKeyboard") {
            utility.offsetX = g_settings.touchKeyboardOffsetX;
            utility.offsetY = g_settings.touchKeyboardOffsetY;
        } else if (token == L"penMenu") {
            utility.offsetX = g_settings.penMenuOffsetX;
            utility.offsetY = g_settings.penMenuOffsetY;
        } else if (token == L"virtualTouchpad") {
            utility.offsetX = g_settings.virtualTouchpadOffsetX;
            utility.offsetY = g_settings.virtualTouchpadOffsetY;
        } else if (token == L"inputIndicator") {
            utility.offsetX = g_settings.inputIndicatorOffsetX;
            utility.offsetY = g_settings.inputIndicatorOffsetY;
        }
        result.push_back(std::move(utility));
    }

    return result;
}

static void LogElement(FrameworkElement const& element,
                       PCWSTR prefix) {
    if (!g_settings.detailedLogging || !element) {
        return;
    }

    try {
        auto className = winrt::get_class_name(element);
        auto name = element.Name();
        auto automationId =
            AutomationProperties::GetAutomationId(element);
        auto automationName =
            AutomationProperties::GetName(element);
        Wh_Log(
            L"[Discover] %s class=%s name=%s automationId=%s "
            L"automationName=%s col=%d size=%.1fx%.1f visibleIcons=%d",
            prefix,
            className.c_str(),
            name.c_str(),
            automationId.c_str(),
            automationName.c_str(),
            Grid::GetColumn(element),
            element.ActualWidth(),
            element.ActualHeight(),
            CountVisibleIconViews(element));
    } catch (...) {
        Wh_Log(L"[Discover] Failed to log %s", prefix);
    }
}

static HostSnapshot CaptureHost(FrameworkElement const& element,
                                Grid const& trayGrid,
                                int markerIndex) {
    HostSnapshot snapshot;
    snapshot.element = element;
    snapshot.column = Grid::GetColumn(element);
    snapshot.columnSpan = Grid::GetColumnSpan(element);
    snapshot.row = Grid::GetRow(element);
    snapshot.rowSpan = Grid::GetRowSpan(element);
    snapshot.width = element.Width();
    snapshot.height = element.Height();
    snapshot.minWidth = element.MinWidth();
    snapshot.minHeight = element.MinHeight();
    snapshot.maxWidth = element.MaxWidth();
    snapshot.maxHeight = element.MaxHeight();
    snapshot.margin = element.Margin();
    snapshot.horizontalAlignment = element.HorizontalAlignment();
    snapshot.verticalAlignment = element.VerticalAlignment();
    snapshot.renderTransform = element.RenderTransform();

    Grid marker;
    marker.Name(
        L"TrayUtilityCustomizerHostMarker_" +
        std::to_wstring(markerIndex));
    marker.Width(0);
    marker.Height(0);
    marker.MinWidth(0);
    marker.MinHeight(0);
    marker.MaxWidth(0);
    marker.MaxHeight(0);
    marker.IsHitTestVisible(false);
    Grid::SetColumn(marker, snapshot.column);
    Grid::SetColumnSpan(marker, 1);
    Grid::SetRow(marker, snapshot.row);
    Grid::SetRowSpan(marker, snapshot.rowSpan);
    trayGrid.Children().Append(marker);
    snapshot.columnMarker = marker;
    return snapshot;
}

static void RestoreHost(HostSnapshot& snapshot) {
    try {
        if (snapshot.element) {
            int restoreColumn = snapshot.column;
            if (snapshot.columnMarker) {
                restoreColumn = Grid::GetColumn(snapshot.columnMarker);
            }

            Grid::SetColumn(snapshot.element, restoreColumn);
            Grid::SetColumnSpan(snapshot.element, snapshot.columnSpan);
            Grid::SetRow(snapshot.element, snapshot.row);
            Grid::SetRowSpan(snapshot.element, snapshot.rowSpan);
            snapshot.element.Width(snapshot.width);
            snapshot.element.Height(snapshot.height);
            snapshot.element.MinWidth(snapshot.minWidth);
            snapshot.element.MinHeight(snapshot.minHeight);
            snapshot.element.MaxWidth(snapshot.maxWidth);
            snapshot.element.MaxHeight(snapshot.maxHeight);
            snapshot.element.Margin(snapshot.margin);
            snapshot.element.HorizontalAlignment(
                snapshot.horizontalAlignment);
            snapshot.element.VerticalAlignment(snapshot.verticalAlignment);
            snapshot.element.RenderTransform(snapshot.renderTransform);
        }

        if (snapshot.columnMarker && g_layoutGrid) {
            uint32_t markerIndex = 0;
            if (g_layoutGrid.Children().IndexOf(
                    snapshot.columnMarker, markerIndex)) {
                g_layoutGrid.Children().RemoveAt(markerIndex);
            }
        }
    } catch (...) {
        Wh_Log(L"[Restore] Native host restore failed");
    }
    snapshot = {};
}

static void RemoveInsertedColumn() {
    if (!g_insertedColumn || !g_layoutGrid ||
        !g_layoutColumnMarker) {
        return;
    }

    uint32_t markerIndex = 0;
    if (!g_layoutGrid.Children().IndexOf(
            g_layoutColumnMarker, markerIndex)) {
        Wh_Log(
            L"[Restore] Dedicated-column marker missing; "
            L"leaving columns untouched");
        return;
    }

    int liveColumn = Grid::GetColumn(g_layoutColumnMarker);
    g_layoutGrid.Children().RemoveAt(markerIndex);

    auto columns = g_layoutGrid.ColumnDefinitions();
    if (liveColumn < 0 ||
        static_cast<uint32_t>(liveColumn) >= columns.Size()) {
        Wh_Log(
            L"[Restore] Dedicated-column marker has invalid column %d",
            liveColumn);
        return;
    }
    columns.RemoveAt(static_cast<uint32_t>(liveColumn));

    auto children = g_layoutGrid.Children();
    for (uint32_t i = 0; i < children.Size(); i++) {
        auto element =
            children.GetAt(i).try_as<FrameworkElement>();
        if (!element) {
            continue;
        }
        int column = Grid::GetColumn(element);
        int span = Grid::GetColumnSpan(element);
        if (column > liveColumn) {
            Grid::SetColumn(element, column - 1);
        } else if (column < liveColumn &&
                   column + span > liveColumn) {
            Grid::SetColumnSpan(element, span - 1);
        }
    }
}

static void RestoreLayout() {
    if (!g_layoutApplied) {
        return;
    }

    RemoveInsertedColumn();
    for (auto& snapshot : g_hostSnapshots) {
        RestoreHost(snapshot);
    }
    g_hostSnapshots.clear();
    g_insertedColumn = false;
    g_layoutColumn = -1;
    g_layoutColumnMarker = nullptr;
    g_layoutGrid = nullptr;
    g_layoutApplied = false;
    Wh_Log(L"[Restore] Native utility layout restored");
}

static void ApplyHostLayout(FrameworkElement const& host,
                            int column,
                            double horizontalOffset,
                            double verticalOffset,
                            int offsetX,
                            int offsetY) {
    Grid::SetColumn(host, column);
    Grid::SetColumnSpan(host, 1);
    host.Width(static_cast<double>(g_settings.buttonWidth));
    host.Height(static_cast<double>(g_settings.buttonHeight));
    host.MinWidth(0);
    host.MinHeight(0);
    host.MaxWidth(static_cast<double>(g_settings.buttonWidth));
    host.MaxHeight(static_cast<double>(g_settings.buttonHeight));
    host.HorizontalAlignment(HorizontalAlignment::Center);
    host.VerticalAlignment(VerticalAlignment::Center);
    host.Margin(Thickness{});

    TranslateTransform transform;
    transform.X(horizontalOffset + static_cast<double>(
        g_settings.groupOffsetX + offsetX));
    transform.Y(verticalOffset + static_cast<double>(
        g_settings.groupOffsetY + offsetY));
    host.RenderTransform(transform);
}

struct LayoutMetrics {
    int columns = 1;
    int rows = 1;
    double groupWidth = 0;
    double groupHeight = 0;
};

static LayoutMetrics CalculateLayoutMetrics(
    int itemCount,
    double trayHeight) {
    LayoutMetrics metrics;
    LayoutMode mode = g_settings.layoutMode;
    if (mode == LayoutMode::Auto) {
        double neededHeight =
            itemCount * g_settings.buttonHeight +
            std::max(0, itemCount - 1) *
                g_settings.buttonSpacing;
        mode = trayHeight >= neededHeight
                   ? LayoutMode::Column
                   : LayoutMode::Row;
    }

    if (mode == LayoutMode::Row) {
        metrics.columns = std::max(1, itemCount);
        metrics.rows = 1;
    } else if (mode == LayoutMode::Column) {
        metrics.columns = 1;
        metrics.rows = std::max(1, itemCount);
    } else {
        int columns = g_settings.gridColumns;
        int rows = g_settings.gridRows;
        if (columns <= 0 && rows <= 0) {
            columns = static_cast<int>(
                std::ceil(std::sqrt(
                    static_cast<double>(itemCount))));
        }
        if (columns <= 0) {
            columns = static_cast<int>(
                std::ceil(static_cast<double>(itemCount) /
                          std::max(1, rows)));
        }
        if (rows <= 0) {
            rows = static_cast<int>(
                std::ceil(static_cast<double>(itemCount) /
                          std::max(1, columns)));
        }
        while (columns * rows < itemCount) {
            if (g_settings.fillOrder == FillOrder::RowFirst) {
                rows++;
            } else {
                columns++;
            }
        }
        metrics.columns = std::max(1, columns);
        metrics.rows = std::max(1, rows);
    }

    metrics.groupWidth =
        metrics.columns * g_settings.buttonWidth +
        std::max(0, metrics.columns - 1) *
            g_settings.buttonSpacing;
    metrics.groupHeight =
        metrics.rows * g_settings.buttonHeight +
        std::max(0, metrics.rows - 1) *
            g_settings.buttonSpacing;

    if (metrics.rows > 1 &&
        metrics.groupHeight > trayHeight) {
        Wh_Log(
            L"[Layout] Requested vertical layout needs %.1fpx "
            L"but tray height is %.1fpx; honoring the requested layout",
            metrics.groupHeight,
            trayHeight);
    }
    return metrics;
}

static FrameworkElement FindPositionReference(
    Grid const& trayGrid,
    Position position) {
    if (position == Position::BeforeOmni) {
        return FindDirectChildByName(
            trayGrid, L"ControlCenterButton");
    }
    if (position == Position::BeforeClock) {
        return FindDirectChildByName(
            trayGrid, L"NotificationCenterButton");
    }
    if (position == Position::AfterClock ||
        position == Position::AfterShowDesktop) {
        return FindDirectChildByName(
            trayGrid, L"ShowDesktopStack");
    }
    return nullptr;
}

static int InsertDedicatedColumn(
    Grid const& trayGrid,
    int insertColumn,
    double width) {
    ColumnDefinition definition;
    definition.Width(
        GridLength{width, GridUnitType::Pixel});
    auto columns = trayGrid.ColumnDefinitions();
    if (static_cast<uint32_t>(insertColumn) <
        columns.Size()) {
        columns.InsertAt(
            static_cast<uint32_t>(insertColumn),
            definition);
    } else {
        insertColumn = static_cast<int>(columns.Size());
        columns.Append(definition);
    }

    auto children = trayGrid.Children();
    for (uint32_t i = 0; i < children.Size(); i++) {
        auto element =
            children.GetAt(i).try_as<FrameworkElement>();
        if (!element) {
            continue;
        }
        int column = Grid::GetColumn(element);
        int span = Grid::GetColumnSpan(element);
        if (column >= insertColumn) {
            Grid::SetColumn(element, column + 1);
        } else if (column + span > insertColumn) {
            Grid::SetColumnSpan(element, span + 1);
        }
    }

    Grid marker;
    marker.Name(kLayoutColumnMarkerName);
    marker.Width(0);
    marker.Height(0);
    marker.MinWidth(0);
    marker.MinHeight(0);
    marker.MaxWidth(0);
    marker.MaxHeight(0);
    marker.IsHitTestVisible(false);
    Grid::SetColumn(marker, insertColumn);
    trayGrid.Children().Append(marker);
    g_layoutColumnMarker = marker;

    return insertColumn;
}

static bool LooksLikeStaleV02Host(
    FrameworkElement const& host) {
    if (!host || std::isnan(host.Width()) ||
        std::isnan(host.Height())) {
        return false;
    }
    return host.RenderTransform()
        .try_as<TranslateTransform>() != nullptr;
}

static void ResetHostToNativeDefaults(
    FrameworkElement const& host,
    int column) {
    Grid::SetColumn(host, column);
    Grid::SetColumnSpan(host, 1);
    host.Width(NAN);
    host.Height(NAN);
    host.MinWidth(0);
    host.MinHeight(0);
    host.MaxWidth(INFINITY);
    host.MaxHeight(INFINITY);
    host.Margin(Thickness{});
    host.HorizontalAlignment(HorizontalAlignment::Stretch);
    host.VerticalAlignment(VerticalAlignment::Stretch);
    host.RenderTransform(Transform{nullptr});
}

static void RecoverStaleV02Layout(
    Grid const& trayGrid,
    FrameworkElement const& overflowHost,
    FrameworkElement const& emojiHost) {
    if (!overflowHost || !emojiHost ||
        Grid::GetColumn(overflowHost) !=
            Grid::GetColumn(emojiHost) ||
        !LooksLikeStaleV02Host(overflowHost) ||
        !LooksLikeStaleV02Host(emojiHost)) {
        return;
    }

    auto notificationArea =
        FindDirectChildByName(
            trayGrid, L"NotificationAreaIcons");
    auto mainStack =
        FindDirectChildByName(trayGrid, L"MainStack");
    auto secondaryClock =
        FindDirectChildByName(
            trayGrid, L"SecondaryClockStack");

    int overflowColumn =
        notificationArea
            ? std::max(
                  0,
                  Grid::GetColumn(notificationArea) - 1)
            : Grid::GetColumn(overflowHost);
    int emojiColumn =
        secondaryClock
            ? std::max(
                  0,
                  Grid::GetColumn(secondaryClock) - 1)
            : mainStack
                  ? Grid::GetColumn(mainStack) + 1
                  : Grid::GetColumn(emojiHost);

    ResetHostToNativeDefaults(
        overflowHost, overflowColumn);
    ResetHostToNativeDefaults(
        emojiHost, emojiColumn);

    auto columns = trayGrid.ColumnDefinitions();
    if (overflowColumn >= 0 &&
        static_cast<uint32_t>(overflowColumn) <
            columns.Size()) {
        columns.GetAt(
            static_cast<uint32_t>(overflowColumn))
            .Width(GridLength{
                1.0, GridUnitType::Auto});
    }

    Wh_Log(
        L"[Recovery] Repaired stale v0.2 layout "
        L"(overflow col=%d, emoji col=%d)",
        overflowColumn,
        emojiColumn);
}

static bool ApplyLayout() {
    RestoreLayout();

    HWND hWnd =
        g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) {
        return false;
    }
    g_taskbarWnd = hWnd;

    auto xamlRoot = GetTaskbarXamlRoot(hWnd);
    if (!xamlRoot) {
        Wh_Log(L"[Apply] Taskbar XAML root unavailable");
        return false;
    }

    auto root = xamlRoot.Content().try_as<FrameworkElement>();
    if (!root) {
        return false;
    }

    auto trayGridElement = FindChildRecursive(
        root,
        [](FrameworkElement element) {
            return element.Name() == L"SystemTrayFrameGrid";
        });
    auto trayGrid = trayGridElement.try_as<Grid>();
    if (!trayGrid) {
        Wh_Log(L"[Apply] SystemTrayFrameGrid not found");
        return false;
    }

    if (g_settings.detailedLogging) {
        auto children = trayGrid.Children();
        for (uint32_t i = 0; i < children.Size(); i++) {
            LogElement(children.GetAt(i).try_as<FrameworkElement>(),
                       L"tray child");
        }
    }

    if (!g_settings.enabled) {
        Wh_Log(L"[Apply] Disabled by setting");
        return true;
    }

    auto overflowHost =
        FindDirectChildByName(trayGrid, L"NotifyIconStack");
    if (!overflowHost) {
        Wh_Log(L"[Apply] NotifyIconStack not found");
        return false;
    }

    auto mainStack =
        FindDirectChildByName(trayGrid, L"MainStack");

    auto emojiElement =
        FindUtilityElement(trayGrid, L"emoji");
    auto exactEmojiHost =
        emojiElement
            ? FindDirectTrayHost(
                  trayGrid, emojiElement)
            : nullptr;
    RecoverStaleV02Layout(
        trayGrid, overflowHost, exactEmojiHost);

    if (g_settings.minimumTrayHeight > 0 &&
        trayGrid.ActualHeight() <
            static_cast<double>(
                g_settings.minimumTrayHeight)) {
        Wh_Log(
            L"[Apply] Tray height %.1f is below minimum %d",
            trayGrid.ActualHeight(),
            g_settings.minimumTrayHeight);
        return true;
    }

    LogElement(overflowHost, L"overflow host");
    LogElement(mainStack, L"MainStack fallback");

    auto utilities = DiscoverUtilityHosts(
        trayGrid, overflowHost, mainStack);
    if (utilities.empty()) {
        Wh_Log(L"[Apply] No selected utility hosts found");
        return true;
    }

    g_layoutGrid = trayGrid;
    g_layoutApplied = true;
    for (int index = 0;
         index < static_cast<int>(utilities.size());
         index++) {
        auto const& utility = utilities[index];
        LogElement(utility.element, utility.token.c_str());
        g_hostSnapshots.push_back(
            CaptureHost(utility.element, trayGrid, index));
    }

    auto metrics = CalculateLayoutMetrics(
        static_cast<int>(utilities.size()),
        trayGrid.ActualHeight());

    int sharedColumn = -1;
    FrameworkElement emojiHost = nullptr;
    auto emojiIt = std::find_if(
        utilities.begin(),
        utilities.end(),
        [](UtilityHost const& utility) {
            return utility.token == L"emoji";
        });
    if (emojiIt != utilities.end()) {
        emojiHost = emojiIt->element;
    }
    if (!emojiHost) {
        auto emojiElement =
            FindUtilityElement(trayGrid, L"emoji");
        if (emojiElement) {
            emojiHost =
                FindDirectTrayHost(trayGrid, emojiElement);
        } else if (mainStack &&
                   (g_settings.mergeMode ==
                        MergeMode::ForceMainStack ||
                    CountVisibleIconViews(mainStack) == 1)) {
            emojiHost = mainStack;
        }
    }

    bool customPosition =
        g_settings.position != Position::Overflow &&
        g_settings.position != Position::Emoji;
    bool dedicated =
        customPosition ||
        metrics.columns > 1;
    if (g_settings.position == Position::Emoji &&
        emojiHost && !dedicated) {
        sharedColumn = Grid::GetColumn(emojiHost);
    } else if (!customPosition && !dedicated) {
        if (g_settings.position == Position::Emoji) {
            Wh_Log(
                L"[Apply] Emoji anchor unavailable; "
                L"using hidden-icons column");
        }
        sharedColumn = Grid::GetColumn(overflowHost);
    } else {
        if (!customPosition) {
            sharedColumn =
                g_settings.position == Position::Emoji &&
                        emojiHost
                    ? Grid::GetColumn(emojiHost)
                    : Grid::GetColumn(overflowHost);
        } else {
            auto reference =
                FindPositionReference(
                    trayGrid, g_settings.position);
            bool insertAfter =
                g_settings.position ==
                Position::AfterShowDesktop;
            if (g_settings.position ==
                Position::BeforeIcons) {
                sharedColumn = 0;
            } else if (reference) {
                sharedColumn =
                    Grid::GetColumn(reference) +
                    (insertAfter ? 1 : 0);
            } else {
                Wh_Log(
                    L"[Apply] Requested position anchor unavailable; "
                    L"leaving the native layout unchanged");
                RestoreLayout();
                return false;
            }
        }
        sharedColumn = InsertDedicatedColumn(
            trayGrid, sharedColumn, metrics.groupWidth);
        g_insertedColumn = true;
    }

    g_layoutColumn = sharedColumn;

    double pitchX =
        g_settings.buttonWidth + g_settings.buttonSpacing;
    double pitchY =
        g_settings.buttonHeight + g_settings.buttonSpacing;
    for (int index = 0;
         index < static_cast<int>(utilities.size());
         index++) {
        double row = 0;
        double column = 0;
        if (g_settings.fillOrder == FillOrder::RowFirst) {
            row = index / metrics.columns;
            column = index % metrics.columns;

            int shortCount =
                static_cast<int>(utilities.size()) %
                metrics.columns;
            bool inShortRow =
                shortCount > 0 &&
                static_cast<int>(row) == metrics.rows - 1;
            if (inShortRow) {
                double spare = metrics.columns - shortCount;
                if (g_settings.shortGroupAlign ==
                    ShortAlign::Center) {
                    column += spare / 2.0;
                } else if (g_settings.shortGroupAlign ==
                           ShortAlign::End) {
                    column += spare;
                }
            }
        } else {
            row = index % metrics.rows;
            column = index / metrics.rows;

            int shortCount =
                static_cast<int>(utilities.size()) %
                metrics.rows;
            bool inShortColumn =
                shortCount > 0 &&
                static_cast<int>(column) ==
                    metrics.columns - 1;
            if (inShortColumn) {
                double spare = metrics.rows - shortCount;
                if (g_settings.shortGroupAlign ==
                    ShortAlign::Center) {
                    row += spare / 2.0;
                } else if (g_settings.shortGroupAlign ==
                           ShortAlign::End) {
                    row += spare;
                }
            }
        }

        double x =
            (column - (metrics.columns - 1) / 2.0) *
            pitchX;
        double y =
            (row - (metrics.rows - 1) / 2.0) *
            pitchY;
        ApplyHostLayout(
            utilities[index].element,
            sharedColumn,
            x,
            y,
            utilities[index].offsetX,
            utilities[index].offsetY);
    }

    Wh_Log(
        L"[Apply] Utility layout applied: items=%d "
        L"columns=%d rows=%d trayColumn=%d dedicated=%d",
        static_cast<int>(utilities.size()),
        metrics.columns,
        metrics.rows,
        sharedColumn,
        g_insertedColumn);
    return true;
}

static void ApplyLayoutOnWindowThread() {
    HWND hWnd =
        g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd || g_unloading) {
        return;
    }
    RunFromWindowThread(
        hWnd,
        [](void*) {
            try {
                ApplyLayout();
            } catch (...) {
                Wh_Log(L"[Apply] Exception while applying layout");
            }
        },
        nullptr);
}

static VS_FIXEDFILEINFO* GetModuleVersionInfo(HMODULE module) {
    void* info = nullptr;
    UINT length = 0;
    HRSRC resource =
        FindResource(module, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
    if (resource) {
        HGLOBAL loaded = LoadResource(module, resource);
        if (loaded) {
            void* data = LockResource(loaded);
            if (data &&
                (!VerQueryValue(data, L"\\", &info, &length) ||
                 length == 0)) {
                info = nullptr;
            }
        }
    }
    return static_cast<VS_FIXEDFILEINFO*>(info);
}

static HMODULE GetSystemTrayModuleHandle() {
    HMODULE module = GetModuleHandleW(L"SystemTray.dll");
    if (!module) {
        module = GetModuleHandleW(L"Taskbar.View.dll");
        if (module) {
            auto* version = GetModuleVersionInfo(module);
            WORD major =
                version ? HIWORD(version->dwFileVersionMS) : 0;
            if (!major || major >= 2604) {
                module = nullptr;
            }
        }
    }
    if (!module) {
        module = GetModuleHandleW(L"ExplorerExtensions.dll");
    }
    return module;
}

using IconView_IconView_t = void* (WINAPI*)(void*);
static IconView_IconView_t IconView_IconView_Original;

static void* WINAPI IconView_IconView_Hook(void* pThis) {
    void* result = IconView_IconView_Original(pThis);
    if (g_unloading) {
        return result;
    }

    FrameworkElement iconView = nullptr;
    reinterpret_cast<IUnknown**>(pThis)[1]->QueryInterface(
        winrt::guid_of<FrameworkElement>(),
        winrt::put_abi(iconView));
    if (!iconView) {
        return result;
    }

    g_loadedRevokers.emplace_back();
    auto revoker = std::prev(g_loadedRevokers.end());
    *revoker = iconView.Loaded(
        winrt::auto_revoke_t{},
        [revoker](auto const&, auto const&) {
            g_loadedRevokers.erase(revoker);
            if (!g_unloading) {
                ApplyLayoutOnWindowThread();
            }
        });
    return result;
}

using LoadLibraryExW_t =
    HMODULE (WINAPI*)(LPCWSTR, HANDLE, DWORD);
static LoadLibraryExW_t LoadLibraryExW_Original;

static bool HookSystemTraySymbols(HMODULE module) {
    // SystemTray.dll, Taskbar.View.dll, ExplorerExtensions.dll
    WindhawkUtils::SYMBOL_HOOK systemTrayHooks[] = {{
        {LR"(public: __cdecl winrt::SystemTray::implementation::IconView::IconView(void))"},
        &IconView_IconView_Original,
        IconView_IconView_Hook,
    }};
    return WindhawkUtils::HookSymbols(
        module, systemTrayHooks, ARRAYSIZE(systemTrayHooks));
}

static HMODULE WINAPI LoadLibraryExW_Hook(
    LPCWSTR fileName,
    HANDLE file,
    DWORD flags) {
    HMODULE module =
        LoadLibraryExW_Original(fileName, file, flags);
    if (module && fileName &&
        !g_systemTrayModuleHooked &&
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
    return module;
}

static bool HookTaskbarDllSymbols() {
    HMODULE module = LoadLibraryExW(
        L"taskbar.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!module) {
        return false;
    }

    WindhawkUtils::SYMBOL_HOOK taskbarDllHooks[] = {
        {{LR"(const CTaskBand::`vftable'{for `ITaskListWndSite'})"},
         &CTaskBand_ITaskListWndSite_vftable},
        {{LR"(public: virtual class std::shared_ptr<class TaskbarHost> __cdecl CTaskBand::GetTaskbarHost(void)const )"},
         &CTaskBand_GetTaskbarHost_Original},
        {{LR"(public: int __cdecl TaskbarHost::FrameHeight(void)const )"},
         &TaskbarHost_FrameHeight_Original},
        {{LR"(public: void __cdecl std::_Ref_count_base::_Decref(void))"},
         &std__Ref_count_base__Decref_Original},
    };
    return WindhawkUtils::HookSymbols(
        module, taskbarDllHooks, ARRAYSIZE(taskbarDllHooks));
}

static void StopRetryThread() {
    if (g_retryStopEvent) {
        SetEvent(g_retryStopEvent);
    }
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
}

BOOL Wh_ModInit() {
    Wh_Log(L"[Init] Tray Utility Customizer v0.3");
    LoadSettings();

    if (!HookTaskbarDllSymbols()) {
        Wh_Log(L"[Init] taskbar.dll symbol hooks failed");
        return FALSE;
    }

    if (HMODULE module = GetSystemTrayModuleHandle()) {
        if (!HookSystemTraySymbols(module)) {
            Wh_Log(L"[Init] system tray symbol hooks failed");
            return FALSE;
        }
        g_systemTrayModuleHooked = true;
    } else {
        HMODULE kernelbase = GetModuleHandleW(L"kernelbase.dll");
        auto loadLibraryExW = kernelbase
            ? reinterpret_cast<LoadLibraryExW_t>(
                  GetProcAddress(kernelbase, "LoadLibraryExW"))
            : nullptr;
        if (loadLibraryExW) {
            WindhawkUtils::SetFunctionHook(
                loadLibraryExW,
                LoadLibraryExW_Hook,
                &LoadLibraryExW_Original);
        } else {
            Wh_Log(L"[Init] LoadLibraryExW hook unavailable");
            return FALSE;
        }
    }
    return TRUE;
}

void Wh_ModAfterInit() {
    ApplyLayoutOnWindowThread();

    g_retryStopEvent =
        CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_retryThread = CreateThread(
        nullptr,
        0,
        [](void*) -> DWORD {
            for (int attempt = 1;
                 attempt <= 6 && !g_unloading;
                 attempt++) {
                if (WaitForSingleObject(
                        g_retryStopEvent, 1500) != WAIT_TIMEOUT) {
                    break;
                }
                if (g_layoutApplied) {
                    break;
                }
                Wh_Log(L"[Retry] Layout attempt %d", attempt);
                ApplyLayoutOnWindowThread();
            }
            return 0;
        },
        nullptr,
        0,
        nullptr);
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Wh_Log(L"[Settings] Reapplying");
    ApplyLayoutOnWindowThread();
}

void Wh_ModUninit() {
    g_unloading = true;
    Wh_Log(L"[Uninit]");
    StopRetryThread();

    HWND hWnd =
        g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (hWnd) {
        RunFromWindowThread(
            hWnd,
            [](void*) {
                g_loadedRevokers.clear();
                RestoreLayout();
            },
            nullptr);
    } else {
        g_loadedRevokers.clear();
        RestoreLayout();
    }
}