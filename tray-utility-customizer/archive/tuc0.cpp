Tray Utility Customizer
tray-utility-customizer
0.1
sb4ssman
explorer.exe
Combines the Show hidden icons and Emoji and more controls into one compact, configurable system-tray slot.
Details
Settings
Source Code
Advanced 
Collapse Readme and Settings
// ==WindhawkMod==
// @id              tray-utility-customizer
// @name            Tray Utility Customizer
// @description     Combines the Show hidden icons and Emoji and more controls into one compact, configurable system-tray slot.
// @version         0.1
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

#include <atomic>
#include <cmath>
#include <cwctype>
#include <functional>
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
    Exact,
    ForceMainStack,
};

struct Settings {
    bool enabled;
    MergeMode mergeMode;
    bool overflowFirst;
    int slotWidth;
    int buttonHeight;
    int buttonSpacing;
    int minimumTrayHeight;
    int groupOffsetX;
    int groupOffsetY;
    int overflowOffsetX;
    int overflowOffsetY;
    int emojiOffsetX;
    int emojiOffsetY;
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

static HostSnapshot g_overflowSnapshot;
static HostSnapshot g_emojiSnapshot;
static bool g_layoutApplied = false;

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
    if (mergeMode == L"exact") {
        g_settings.mergeMode = MergeMode::Exact;
    } else if (mergeMode == L"forceMainStack") {
        g_settings.mergeMode = MergeMode::ForceMainStack;
    } else {
        g_settings.mergeMode = MergeMode::Auto;
    }

    g_settings.overflowFirst =
        GetStringSetting(L"itemOrder") != L"emojiFirst";
    g_settings.slotWidth =
        ClampSetting(Wh_GetIntSetting(L"slotWidth"), 16, 96);
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
    SendMessage(hWnd, message, 0, reinterpret_cast<LPARAM>(&param));
    UnhookWindowsHookEx(hook);
    return true;
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
        !std__Ref_count_base__Decref_Original) {
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
    if (!taskbarHostSharedPtr[0] && !taskbarHostSharedPtr[1]) {
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

static bool ContainsEmojiMetadata(FrameworkElement const& element) {
    try {
        std::wstring combined =
            std::wstring(element.Name()) + L" " +
            std::wstring(AutomationProperties::GetAutomationId(element)) +
            L" " +
            std::wstring(AutomationProperties::GetName(element)) +
            L" " +
            std::wstring(AutomationProperties::GetHelpText(element));
        return ToLower(std::move(combined)).find(L"emoji") !=
               std::wstring::npos;
    } catch (...) {
        return false;
    }
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

static HostSnapshot CaptureHost(FrameworkElement const& element) {
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
    return snapshot;
}

static void RestoreHost(HostSnapshot& snapshot) {
    if (!snapshot.element) {
        return;
    }

    try {
        Grid::SetColumn(snapshot.element, snapshot.column);
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
    } catch (...) {
        Wh_Log(L"[Restore] Native host restore failed");
    }
    snapshot = {};
}

static void RestoreLayout() {
    if (!g_layoutApplied) {
        return;
    }
    RestoreHost(g_overflowSnapshot);
    RestoreHost(g_emojiSnapshot);
    g_layoutApplied = false;
    Wh_Log(L"[Restore] Native utility layout restored");
}

static void ApplyHostLayout(FrameworkElement const& host,
                            int column,
                            double verticalOffset,
                            int offsetX,
                            int offsetY) {
    Grid::SetColumn(host, column);
    Grid::SetColumnSpan(host, 1);
    host.Width(static_cast<double>(g_settings.slotWidth));
    host.Height(static_cast<double>(g_settings.buttonHeight));
    host.MinWidth(0);
    host.MinHeight(0);
    host.MaxWidth(static_cast<double>(g_settings.slotWidth));
    host.MaxHeight(static_cast<double>(g_settings.buttonHeight));
    host.HorizontalAlignment(HorizontalAlignment::Center);
    host.VerticalAlignment(VerticalAlignment::Center);
    host.Margin(Thickness{});

    TranslateTransform transform;
    transform.X(static_cast<double>(
        g_settings.groupOffsetX + offsetX));
    transform.Y(verticalOffset + static_cast<double>(
        g_settings.groupOffsetY + offsetY));
    host.RenderTransform(transform);
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

    if (g_settings.minimumTrayHeight > 0 &&
        trayGrid.ActualHeight() <
            static_cast<double>(g_settings.minimumTrayHeight)) {
        Wh_Log(L"[Apply] Tray height %.1f is below minimum %d",
               trayGrid.ActualHeight(),
               g_settings.minimumTrayHeight);
        return true;
    }

    auto overflowHost =
        FindDirectChildByName(trayGrid, L"NotifyIconStack");
    if (!overflowHost) {
        Wh_Log(L"[Apply] NotifyIconStack not found");
        return false;
    }

    auto emojiElement = FindChildRecursive(
        trayGrid,
        [](FrameworkElement element) {
            return ContainsEmojiMetadata(element);
        });
    auto exactEmojiHost =
        emojiElement
            ? FindDirectTrayHost(trayGrid, emojiElement)
            : nullptr;
    auto mainStack =
        FindDirectChildByName(trayGrid, L"MainStack");

    FrameworkElement emojiHost = exactEmojiHost;
    bool exactMatch = emojiHost != nullptr;
    int mainStackIconCount =
        mainStack ? CountVisibleIconViews(mainStack) : 0;

    if (!emojiHost &&
        g_settings.mergeMode != MergeMode::Exact) {
        emojiHost = mainStack;
    }

    LogElement(overflowHost, L"overflow host");
    LogElement(exactEmojiHost, L"exact emoji host");
    LogElement(mainStack, L"MainStack fallback");

    if (!emojiHost) {
        Wh_Log(L"[Apply] Emoji and more host not found");
        return false;
    }
    if (emojiHost == overflowHost) {
        Wh_Log(L"[Apply] Controls already share a host");
        return true;
    }

    bool safeToMerge = exactMatch;
    if (!safeToMerge &&
        g_settings.mergeMode == MergeMode::Auto) {
        safeToMerge = emojiHost == mainStack &&
                      mainStackIconCount == 1;
    }
    if (g_settings.mergeMode == MergeMode::ForceMainStack) {
        safeToMerge = emojiHost == mainStack;
    }

    if (!safeToMerge) {
        Wh_Log(
            L"[Apply] Guard refused merge: exact=%d "
            L"MainStackVisibleIconViews=%d mode=%d",
            exactMatch,
            mainStackIconCount,
            static_cast<int>(g_settings.mergeMode));
        return true;
    }

    g_overflowSnapshot = CaptureHost(overflowHost);
    g_emojiSnapshot = CaptureHost(emojiHost);

    int sharedColumn = g_overflowSnapshot.column;
    double separation =
        (g_settings.buttonHeight + g_settings.buttonSpacing) / 2.0;
    double overflowY =
        g_settings.overflowFirst ? -separation : separation;
    double emojiY =
        g_settings.overflowFirst ? separation : -separation;

    ApplyHostLayout(
        overflowHost,
        sharedColumn,
        overflowY,
        g_settings.overflowOffsetX,
        g_settings.overflowOffsetY);
    ApplyHostLayout(
        emojiHost,
        sharedColumn,
        emojiY,
        g_settings.emojiOffsetX,
        g_settings.emojiOffsetY);

    g_layoutApplied = true;
    Wh_Log(
        L"[Apply] Shared utility slot applied at column=%d "
        L"exactEmojiMatch=%d MainStackVisibleIconViews=%d",
        sharedColumn,
        exactMatch,
        mainStackIconCount);
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
        WaitForSingleObject(g_retryThread, 3000);
        CloseHandle(g_retryThread);
        g_retryThread = nullptr;
    }
    if (g_retryStopEvent) {
        CloseHandle(g_retryStopEvent);
        g_retryStopEvent = nullptr;
    }
}

BOOL Wh_ModInit() {
    Wh_Log(L"[Init] Tray Utility Customizer v0.1");
    LoadSettings();

    if (!HookTaskbarDllSymbols()) {
        Wh_Log(L"[Init] taskbar.dll symbol hooks failed");
    }

    if (HMODULE module = GetSystemTrayModuleHandle()) {
        g_systemTrayModuleHooked = true;
        if (!HookSystemTraySymbols(module)) {
            Wh_Log(L"[Init] system tray symbol hooks failed");
        }
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
