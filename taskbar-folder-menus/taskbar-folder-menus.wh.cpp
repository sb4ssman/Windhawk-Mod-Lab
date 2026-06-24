// ==WindhawkMod==
// @id              taskbar-folder-menus
// @name            Taskbar Folder Menus
// @description     Adds compact taskbar buttons that open configured Shell targets as popup menus, similar to classic Windows taskbar toolbars.
// @version         0.5
// @author          sb4ssman
// @github          https://github.com/sb4ssman
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lversion -lshell32 -luuid -lgdi32 -lcomctl32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Taskbar Folder Menus

Adds compact taskbar buttons that open Shell targets as native popup menus,
recreating the most useful part of the classic Windows taskbar toolbar workflow.

![Two folder buttons in the system tray](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-folder-menus/desktop-controlpanel.png)
*Two folder buttons — Desktop and Control Panel — injected into the system tray.*

![Control Panel open as a native Shell popup menu](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-folder-menus/controlpanel-menu-open.png)
*Control Panel open as a native Shell popup menu with full icons.*

![Four buttons on a denser taskbar](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-folder-menus/c-github-desktop-controlpanel-v.png)
*Four-button layout on a taller taskbar alongside a stats panel.*

Click a small button to browse a folder, drive, Desktop, or Control Panel
directly from the taskbar — no minimizing required. Subfolders expand on
hover. Right-click any item for the full Windows Shell context menu. Folders
include an "Open in Explorer" shortcut at the top of their submenu.

## Example folder entries

```text
🖥=shell:Desktop
⚙=shell:ControlPanelFolder
📥=%USERPROFILE%\Downloads
C:=C:\
```

Use `Label=Target` form for each entry, separated by newlines, `|`, or commas.
Targets can be normal paths or Shell namespace roots like `shell:Desktop` and
`shell:ControlPanelFolder`. Environment variables such as `%USERPROFILE%` are
expanded automatically. The full label and target appear in the tooltip.

Emoji labels are a natural fit for narrow buttons. Label ideas: 📁 folder,
🖥 desktop, 💻 laptop, 🪟 windows, 📥 downloads, 🌐 network, 🗄 drive,
📄 documents, 🔧 tools, ⚙ settings, ⭐ favorites.

## Settings

| Setting | Default | Description |
|---------|---------|-------------|
| Position | Before notification icons | Where to inject the button group in the tray |
| Folders | 🖥=shell:Desktop, ⚙=shell:ControlPanelFolder | Folder buttons: `Label=Target` per entry |
| Layout mode | Single row | Single row, single column, or grid |
| Grid columns | 2 | Columns used in grid mode |
| Grid rows | 0 (auto) | Rows in grid mode; 0 = derived from column count |
| Fill order | Row-first | Row-first or column-first |
| Short row/column alignment | Start | Align an incomplete final row or column |
| Button width | 32 px | Width of each folder button |
| Button height | 22 px | Height of each folder button |
| Button spacing | 4 px | Gap between buttons |
| Default button text | 📁 | Fallback icon for entries with long labels |
| Text/icon size | 10 pt | Button label font size |
| Text color | *(system)* | Optional hex color for button labels |
| Background color | *(system)* | Optional hex color for button backgrounds |
| Hover background color | `#4488FF` | Highlight color when hovering a button |
| Click background color | *(system)* | Color while a button is pressed |
| Border color | *(system)* | Optional hex border color |
| Border thickness | -1 (system) | -1 = system default, 0 = no border |
| Corner rounding | -1 (system) | -1 = system default, 0 = square |
| Opacity | 100% | Button transparency |
| Shine effect | Off | Gradient highlight; activates when a custom background color is set |
| Group padding left | 0 px | Extra space to the left of the button group |
| Group padding right | 0 px | Extra space to the right of the button group |
| Group X offset | 0 px | Shift the entire group left or right |
| Group Y offset | 0 px | Shift the entire group up or down |
| Max menu items | 0 (unlimited) | Limit items shown per folder |
| Subfolder depth | 0 (unlimited) | How many subfolder levels to include |
| Show hidden/system items | Off | Include hidden and system Shell items |

## Note on shell:Desktop

`shell:Desktop` shows the full Desktop Shell namespace — user shortcuts, public
shortcuts, and virtual items like Recycle Bin — not just the physical Desktop
folder. Duplicates from the user+public Desktop merge are suppressed automatically.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- position: beforeIcons
  $name: Position
  $description: Where to place the folder menu buttons in the tray.
  $options:
  - "beforeIcons": "Before notification icons"
  - "beforeOmni": "Before OmniButton (wifi/vol/bat)"
  - "beforeClock": "Before clock"
  - "afterClock": "After clock"
  - "afterShowDesktop": "After Show Desktop strip"

- folders: "🖥=shell:Desktop,⚙=shell:ControlPanelFolder"
  $name: Folders
  $description: >-
    Folder buttons to show. Use entries in Label=Target form, separated by
    newlines, |, or commas. Targets can be normal paths or Shell namespace
    roots such as shell:Desktop and shell:ControlPanelFolder. Environment
    variables such as %USERPROFILE% are expanded. Emoji labels work well on
    narrow buttons.

- layoutMode: row
  $name: Layout mode
  $description: How to arrange multiple folder buttons.
  $options:
  - "row": "Single row"
  - "column": "Single column"
  - "grid": "Grid"

- gridColumns: 2
  $name: Grid columns
  $description: Number of columns in grid mode.

- gridRows: 0
  $name: Grid rows (0 = auto)
  $description: Number of rows in grid mode. 0 = calculate from the column count.

- fillOrder: rowFirst
  $name: Fill order
  $description: Whether folder buttons fill across rows first or down columns first.
  $options:
  - rowFirst: Row first
  - columnFirst: Column first

- shortGroupAlign: start
  $name: Short row/column alignment
  $description: How to align an incomplete final row or column.
  $options:
  - start: Start
  - center: Center
  - end: End

- buttonWidth: 32
  $name: Button width (px)

- buttonHeight: 22
  $name: Button height (px)

- buttonSpacing: 4
  $name: Button spacing (px)

- buttonText: "📁"
  $name: Default button text
  $description: >-
    Icon shown for folder entries with long labels. One- or two-character
    labels are shown directly. Supports Unicode and emoji.

- fontSize: 10
  $name: Text/icon size (pt)
  $description: Size of the button label, including emoji labels.

- textColor: ""
  $name: Text color
  $description: "Optional #RRGGBB or #AARRGGBB. Empty = system default."

- backgroundColor: ""
  $name: Background color
  $description: "Optional #RRGGBB or #AARRGGBB. Empty = system default."

- hoverBackgroundColor: ""
  $name: Hover background color
  $description: "Optional #RRGGBB or #AARRGGBB. Empty = system default."

- pressedBackgroundColor: ""
  $name: Click background color
  $description: "Optional #RRGGBB or #AARRGGBB. Empty = system default."

- borderColor: ""
  $name: Border color
  $description: "Optional #RRGGBB or #AARRGGBB. Empty = system default."

- borderThickness: -1
  $name: Border thickness (px)
  $description: "-1 = system default. 0 = no border. Positive values set a custom border thickness."

- cornerRadius: -1
  $name: Corner rounding (px)
  $description: "-1 = system default. 0 = square corners. Positive values round the button corners."

- opacity: 100
  $name: Opacity (%)
  $description: "Button opacity. 100 = fully opaque, 0 = invisible."

- shineEffect: false
  $name: Shine effect
  $description: Adds a subtle gradient highlight when a custom background color is set.

- groupPaddingLeft: 0
  $name: Group padding left (px)
  $description: Extra space to the left of the button group.

- groupPaddingRight: 0
  $name: Group padding right (px)
  $description: Extra space to the right of the button group.

- groupOffsetX: 0
  $name: Group X offset (px)
  $description: Move the entire button group left (negative) or right (positive).

- groupOffsetY: 0
  $name: Group Y offset (px)
  $description: Move the entire button group up (negative) or down (positive).

- maxMenuItems: 0
  $name: Max menu items per folder
  $description: Limit menu size for very large folders. 0 = unlimited.

- maxDepth: 0
  $name: Subfolder depth
  $description: How many subfolder levels to include as nested menus. 0 = unlimited.

- showHidden: false
  $name: Show hidden/system items

*/
// ==/WindhawkModSettings==

#undef GetCurrentTime

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Media.h>

#include <algorithm>
#include <atomic>
#include <cwctype>
#include <functional>
#include <string>
#include <vector>

#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <windhawk_utils.h>
#include <winver.h>

#ifndef WM_MENURBUTTONDOWN
#define WM_MENURBUTTONDOWN 0x012E
#endif

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Input;
using namespace winrt::Windows::UI::Xaml::Media;

// ============================================================
// Settings
// ============================================================

struct FolderEntry {
    std::wstring label;
    std::wstring target;
};

struct ModSettings {
    std::wstring position = L"beforeIcons";
    std::wstring layoutMode = L"row";
    std::wstring buttonText = L"📁";
    std::vector<FolderEntry> folders;
    int gridColumns = 2;
    int gridRows = 0;
    std::wstring fillOrder = L"rowFirst";
    std::wstring shortGroupAlign = L"start";
    int buttonWidth = 32;
    int buttonHeight = 22;
    int buttonSpacing = 4;
    int maxMenuItems = 0;
    int maxDepth = 0;
    bool showHidden = false;
    int fontSize = 10;
    std::wstring textColor;
    std::wstring backgroundColor;
    std::wstring hoverBackgroundColor;
    std::wstring pressedBackgroundColor;
    std::wstring borderColor;
    int borderThickness = -1;
    int cornerRadius = -1;
    int opacityPct = 100;
    bool shineEffect = false;
    int groupPaddingLeft = 0;
    int groupPaddingRight = 0;
    int groupOffsetX = 0;
    int groupOffsetY = 0;
};
static ModSettings g_settings;

static std::wstring Trim(std::wstring s) {
    auto isWs = [](wchar_t c) { return std::iswspace(c) != 0; };
    while (!s.empty() && isWs(s.front())) s.erase(s.begin());
    while (!s.empty() && isWs(s.back())) s.pop_back();
    return s;
}

// Expand a KNOWNFOLDERID token (e.g. %DESKTOP%) in text.
static std::wstring ExpandKnownFolder(std::wstring text,
                                       const wchar_t* token,
                                       REFKNOWNFOLDERID id) {
    size_t pos = 0;
    while ((pos = text.find(token, pos)) != std::wstring::npos) {
        PWSTR path = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(id, 0, nullptr, &path)) && path) {
            text.replace(pos, wcslen(token), path);
            CoTaskMemFree(path);
        } else {
            pos += wcslen(token);
        }
    }
    return text;
}

static std::wstring ExpandEnv(std::wstring const& s) {
    // Expand known-folder tokens before the standard ExpandEnvironmentStrings call.
    std::wstring result = ExpandKnownFolder(s, L"%DESKTOP%",   FOLDERID_Desktop);
    result = ExpandKnownFolder(result, L"%DOWNLOADS%", FOLDERID_Downloads);
    result = ExpandKnownFolder(result, L"%DOCUMENTS%", FOLDERID_Documents);

    DWORD needed = ExpandEnvironmentStringsW(result.c_str(), nullptr, 0);
    if (!needed) return result;
    std::wstring out(needed, L'\0');
    DWORD written = ExpandEnvironmentStringsW(result.c_str(), out.data(), needed);
    if (!written || written > needed) return result;
    if (!out.empty() && out.back() == L'\0') out.pop_back();
    return out;
}

static std::wstring FileNameFromPath(std::wstring path) {
    while (!path.empty() && (path.back() == L'\\' || path.back() == L'/'))
        path.pop_back();
    size_t pos = path.find_last_of(L"\\/");
    return pos == std::wstring::npos ? path : path.substr(pos + 1);
}

static std::vector<std::wstring> SplitFolderLines(std::wstring text) {
    std::replace(text.begin(), text.end(), L'|', L'\n');
    std::replace(text.begin(), text.end(), L',', L'\n');
    std::vector<std::wstring> lines;
    size_t start = 0;
    while (start <= text.size()) {
        size_t end = text.find_first_of(L"\r\n", start);
        if (end == std::wstring::npos) end = text.size();
        auto line = Trim(text.substr(start, end - start));
        if (!line.empty())
            lines.push_back(line);
        start = end + 1;
        while (start < text.size() && (text[start] == L'\r' || text[start] == L'\n'))
            start++;
        if (end == text.size()) break;
    }
    return lines;
}

static std::vector<FolderEntry> ParseFolders(std::wstring text) {
    std::vector<FolderEntry> folders;
    for (auto line : SplitFolderLines(text)) {
        size_t eq = line.find(L'=');
        FolderEntry entry;
        if (eq == std::wstring::npos) {
            entry.target = ExpandEnv(Trim(line));
            entry.label = FileNameFromPath(entry.target);
        } else {
            entry.label = Trim(line.substr(0, eq));
            entry.target = ExpandEnv(Trim(line.substr(eq + 1)));
            if (entry.label.empty())
                entry.label = FileNameFromPath(entry.target);
        }
        if (!entry.target.empty())
            folders.push_back(entry);
    }
    if (folders.empty()) {
        folders.push_back({ L"🖥", L"shell:Desktop" });
        folders.push_back({ L"⚙", L"shell:ControlPanelFolder" });
    }
    return folders;
}

static std::wstring GetStringSetting(PCWSTR name, PCWSTR fallback) {
    PCWSTR raw = Wh_GetStringSetting(name);
    std::wstring value = raw ? raw : fallback;
    Wh_FreeStringSetting(raw);
    return value;
}

static void LoadSettings() {
    auto clamp = [](int value, int lo, int hi) {
        return std::max(lo, std::min(hi, value));
    };

    g_settings.position = GetStringSetting(L"position", L"beforeIcons");
    g_settings.layoutMode = GetStringSetting(L"layoutMode", L"row");
    g_settings.buttonText = GetStringSetting(L"buttonText", L"📁");
    g_settings.folders = ParseFolders(GetStringSetting(L"folders", L"🖥=shell:Desktop,⚙=shell:ControlPanelFolder"));
    g_settings.gridColumns = std::max(1, Wh_GetIntSetting(L"gridColumns", 2));
    g_settings.gridRows = std::max(0, Wh_GetIntSetting(L"gridRows", 0));
    g_settings.fillOrder = GetStringSetting(L"fillOrder", L"rowFirst");
    g_settings.shortGroupAlign = GetStringSetting(L"shortGroupAlign", L"start");
    g_settings.buttonWidth = std::max(10, Wh_GetIntSetting(L"buttonWidth", 32));
    g_settings.buttonHeight = std::max(10, Wh_GetIntSetting(L"buttonHeight", 22));
    g_settings.buttonSpacing = std::max(0, Wh_GetIntSetting(L"buttonSpacing", 4));
    g_settings.maxMenuItems = std::max(0, Wh_GetIntSetting(L"maxMenuItems", 0));
    g_settings.maxDepth = std::max(0, Wh_GetIntSetting(L"maxDepth", 0));
    g_settings.showHidden = Wh_GetIntSetting(L"showHidden", 0) != 0;
    g_settings.fontSize = std::max(1, Wh_GetIntSetting(L"fontSize", 10));
    g_settings.textColor = GetStringSetting(L"textColor", L"");
    g_settings.backgroundColor = GetStringSetting(L"backgroundColor", L"");
    g_settings.hoverBackgroundColor = GetStringSetting(L"hoverBackgroundColor", L"");
    g_settings.pressedBackgroundColor = GetStringSetting(L"pressedBackgroundColor", L"");
    g_settings.borderColor = GetStringSetting(L"borderColor", L"");
    g_settings.borderThickness = std::max(-1, Wh_GetIntSetting(L"borderThickness", -1));
    g_settings.cornerRadius = std::max(-1, Wh_GetIntSetting(L"cornerRadius", -1));
    g_settings.opacityPct = std::clamp(Wh_GetIntSetting(L"opacity", 100), 0, 100);
    g_settings.shineEffect = Wh_GetIntSetting(L"shineEffect", 0) != 0;
    g_settings.groupPaddingLeft = clamp(Wh_GetIntSetting(L"groupPaddingLeft", 0), -80, 80);
    g_settings.groupPaddingRight = clamp(Wh_GetIntSetting(L"groupPaddingRight", 0), -80, 80);
    g_settings.groupOffsetX = clamp(Wh_GetIntSetting(L"groupOffsetX", 0), -80, 80);
    g_settings.groupOffsetY = clamp(Wh_GetIntSetting(L"groupOffsetY", 0), -80, 80);
}

// ============================================================
// Globals
// ============================================================

static std::atomic<bool> g_unloading{false};
static HWND              g_taskbarWnd = nullptr;
static Grid              g_buttonGrid = nullptr;
static FrameworkElement  g_injectionParent = nullptr;
static int               g_injectedColumn = -1;

static HANDLE g_retryThread = nullptr;
static HANDLE g_retryStopEvent = nullptr;
static bool   g_taskbarViewDllLoaded = false;

// Lazy Shell menu loading state (per-ShowFolderMenu call, single-threaded UI).
static UINT g_menuNextId = 1000;
static std::vector<PIDLIST_ABSOLUTE> g_menuIdToPidl;
static std::vector<HBITMAP> g_menuBitmaps;

struct PendingSubmenu {
    HMENU hmenu;
    PIDLIST_ABSOLUTE pidl;
    int depth;
};
static std::vector<PendingSubmenu> g_pendingSubmenus;

// Forward declarations
static void ApplyAllSettings();
static void ApplyAllSettingsOnWindowThread();
static void RemoveButtonGrid();
static void StopRetryThread();

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
        // 48:83EC 28 | sub rsp,28
        // 48:83C1 48 | add rcx,48
        const BYTE* b = (const BYTE*)TaskbarHost_FrameHeight_Original;
        if (b[0] == 0x48 && b[1] == 0x83 && b[2] == 0xEC && b[4] == 0x48 &&
            b[5] == 0x83 && b[6] == 0xC1 && b[7] <= 0x7F)
            offset = b[7];
        else
            Wh_Log(L"Unsupported TaskbarHost::FrameHeight");
    }
#elif defined(_M_ARM64)
    {
        // 7f2303d5 pacibsp
        // fd7bbfa9 stp     fp, lr, [sp, #-0x10]!
        // fd030091 mov     fp, sp
        // 080c41f8 ldr     x8, [x0, #0x10]!
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
// XAML helpers
// ============================================================

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

static FrameworkElement FindLiveSystemTrayFrameGrid() {
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return nullptr;
    auto xamlRoot = GetTaskbarXamlRoot(hWnd);
    if (!xamlRoot) return nullptr;
    auto root = xamlRoot.Content().try_as<FrameworkElement>();
    if (!root) return nullptr;
    return FindChildRecursive(root, [](FrameworkElement fe) {
        return fe.Name() == L"SystemTrayFrameGrid";
    });
}

// ============================================================
// Folder menu
// ============================================================

struct ShellMenuItem {
    std::wstring displayName;
    PIDLIST_ABSOLUTE pidl = nullptr;
    bool canExpand = false;
};

static void FreeMenuState() {
    for (auto& ps : g_pendingSubmenus)
        if (ps.pidl) CoTaskMemFree(ps.pidl);
    g_pendingSubmenus.clear();

    for (auto pidl : g_menuIdToPidl)
        if (pidl) CoTaskMemFree(pidl);
    g_menuIdToPidl.clear();

    for (auto bmp : g_menuBitmaps)
        if (bmp) DeleteObject(bmp);
    g_menuBitmaps.clear();
}

static bool IsTarget(std::wstring const& value, const wchar_t* token) {
    return _wcsicmp(value.c_str(), token) == 0;
}

static PIDLIST_ABSOLUTE ParseDisplayNamePidl(const wchar_t* name) {
    PIDLIST_ABSOLUTE pidl = nullptr;
    SFGAOF attrs = 0;
    if (SUCCEEDED(SHParseDisplayName(name, nullptr, &pidl, 0, &attrs)))
        return pidl;
    return nullptr;
}

static PIDLIST_ABSOLUTE ParseShellTarget(std::wstring const& target) {
    PIDLIST_ABSOLUTE pidl = nullptr;

    if (IsTarget(target, L"shell:Desktop") || IsTarget(target, L"desktop:")) {
        if (SUCCEEDED(SHGetSpecialFolderLocation(nullptr, CSIDL_DESKTOP, &pidl)))
            return pidl;
        return nullptr;
    }

    if (IsTarget(target, L"shell:ControlPanelFolder") ||
        IsTarget(target, L"control:") ||
        IsTarget(target, L"control")) {
        if ((pidl = ParseDisplayNamePidl(target.c_str())) != nullptr)
            return pidl;
        if ((pidl = ParseDisplayNamePidl(L"shell:::{26EE0668-A00A-44D7-9371-BEB064C98683}")) != nullptr)
            return pidl;
        if ((pidl = ParseDisplayNamePidl(L"shell:::{21EC2020-3AEA-1069-A2DD-08002B30309D}")) != nullptr)
            return pidl;
        return nullptr;
    }

    return ParseDisplayNamePidl(target.c_str());
}

static bool BindFolderFromPidl(PCIDLIST_ABSOLUTE pidl, IShellFolder** folder) {
    *folder = nullptr;
    if (!pidl)
        return false;

    if (ILIsEmpty(pidl))
        return SUCCEEDED(SHGetDesktopFolder(folder));

    IShellFolder* parent = nullptr;
    PCUITEMID_CHILD child = nullptr;
    HRESULT hr = SHBindToParent(pidl, IID_IShellFolder, (void**)&parent, &child);
    if (FAILED(hr) || !parent)
        return false;

    hr = parent->BindToObject(child, nullptr, IID_IShellFolder, (void**)folder);
    parent->Release();
    return SUCCEEDED(hr) && *folder;
}

static std::wstring StrRetToString(STRRET const& str, PCUITEMID_CHILD pidl) {
    if (str.uType == STRRET_WSTR) {
        std::wstring result = str.pOleStr ? str.pOleStr : L"";
        CoTaskMemFree(str.pOleStr);
        return result;
    }

    if (str.uType == STRRET_OFFSET)
        return (LPCWSTR)(((const BYTE*)pidl) + str.uOffset);

    if (str.uType == STRRET_CSTR) {
        int needed = MultiByteToWideChar(CP_ACP, 0, str.cStr, -1, nullptr, 0);
        if (needed > 0) {
            std::wstring result(needed, L'\0');
            MultiByteToWideChar(CP_ACP, 0, str.cStr, -1, result.data(), needed);
            if (!result.empty() && result.back() == L'\0')
                result.pop_back();
            return result;
        }
    }

    return L"";
}

static HBITMAP BitmapFromIcon(HICON icon) {
    if (!icon)
        return nullptr;

    int iconW = GetSystemMetrics(SM_CXSMICON);
    int iconH = GetSystemMetrics(SM_CYSMICON);
    int bmpW = std::max(16, iconW);
    int bmpH = std::max(16, iconH);

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = bmpW;
    bmi.bmiHeader.biHeight = -bmpH;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HDC screenDc = GetDC(nullptr);
    HBITMAP bmp = CreateDIBSection(screenDc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bmp) {
        if (screenDc)
            ReleaseDC(nullptr, screenDc);
        return nullptr;
    }
    HDC memDc = CreateCompatibleDC(screenDc);
    if (!memDc) {
        DeleteObject(bmp);
        if (screenDc)
            ReleaseDC(nullptr, screenDc);
        return nullptr;
    }
    HGDIOBJ oldBmp = SelectObject(memDc, bmp);
    if (bits)
        ZeroMemory(bits, bmpW * bmpH * 4);
    DrawIconEx(memDc, (bmpW - iconW) / 2, (bmpH - iconH) / 2,
               icon, iconW, iconH, 0, nullptr, DI_NORMAL);
    SelectObject(memDc, oldBmp);
    DeleteDC(memDc);
    if (screenDc)
        ReleaseDC(nullptr, screenDc);
    return bmp;
}

static HBITMAP CreateMenuBitmapForPidl(PCIDLIST_ABSOLUTE pidl) {
    SHFILEINFOW sfi{};
    if (!SHGetFileInfoW((LPCWSTR)pidl, 0, &sfi, sizeof(sfi),
                        SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON))
        return nullptr;

    HBITMAP bmp = BitmapFromIcon(sfi.hIcon);
    DestroyIcon(sfi.hIcon);
    if (bmp)
        g_menuBitmaps.push_back(bmp);
    return bmp;
}

static std::vector<ShellMenuItem> EnumerateShellFolder(PCIDLIST_ABSOLUTE folderPidl) {
    std::vector<ShellMenuItem> items;

    IShellFolder* folder = nullptr;
    if (!BindFolderFromPidl(folderPidl, &folder))
        return items;

    DWORD flags = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS;
    if (g_settings.showHidden)
        flags |= SHCONTF_INCLUDEHIDDEN;

    IEnumIDList* enumList = nullptr;
    if (SUCCEEDED(folder->EnumObjects(g_taskbarWnd, flags, &enumList)) && enumList) {
        PITEMID_CHILD child = nullptr;
        ULONG fetched = 0;
        while (enumList->Next(1, &child, &fetched) == S_OK && child) {
            STRRET str{};
            std::wstring displayName;
            if (SUCCEEDED(folder->GetDisplayNameOf(child, SHGDN_NORMAL, &str)))
                displayName = StrRetToString(str, child);

            SFGAOF attrs = SFGAO_FOLDER | SFGAO_HASSUBFOLDER | SFGAO_FILESYSTEM;
            PCUITEMID_CHILD childConst = child;
            folder->GetAttributesOf(1, &childConst, &attrs);

            PIDLIST_ABSOLUTE abs = ILCombine(folderPidl, child);
            CoTaskMemFree(child);
            child = nullptr;

            if (!abs || displayName.empty()) {
                if (abs) CoTaskMemFree(abs);
                continue;
            }

            ShellMenuItem item;
            item.displayName = std::move(displayName);
            item.pidl = abs;
            item.canExpand = (attrs & SFGAO_FOLDER) && (attrs & SFGAO_FILESYSTEM);
            items.push_back(std::move(item));
        }
        enumList->Release();
    }

    folder->Release();

    std::sort(items.begin(), items.end(), [](auto const& a, auto const& b) {
        if (a.canExpand != b.canExpand) return a.canExpand > b.canExpand;
        return _wcsicmp(a.displayName.c_str(), b.displayName.c_str()) < 0;
    });

    // The Desktop namespace merges user+public Desktop folders and virtual items,
    // causing the same shortcut to appear multiple times. Remove consecutive
    // duplicates by display name after sorting.
    for (auto it = items.begin(); it != items.end(); ) {
        auto next = it + 1;
        if (next != items.end() &&
                _wcsicmp(it->displayName.c_str(), next->displayName.c_str()) == 0) {
            if (next->pidl) CoTaskMemFree(next->pidl);
            items.erase(next);
        } else {
            ++it;
        }
    }

    if (g_settings.maxMenuItems > 0 && (int)items.size() > g_settings.maxMenuItems) {
        for (size_t i = g_settings.maxMenuItems; i < items.size(); i++)
            if (items[i].pidl) CoTaskMemFree(items[i].pidl);
        items.resize(g_settings.maxMenuItems);
    }

    return items;
}

static void AddShellFolderItemsToMenu(HMENU menu, PCIDLIST_ABSOLUTE folderPidl, int depth, UINT startPosition = 0);

static void InsertShellMenuItem(HMENU menu, UINT position, ShellMenuItem& item, int depth) {
    bool canExpand = item.canExpand && (g_settings.maxDepth == 0 || depth < g_settings.maxDepth);

    MENUITEMINFOW mii{};
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STRING | MIIM_BITMAP;
    mii.dwTypeData = const_cast<LPWSTR>(item.displayName.c_str());
    mii.cch = (UINT)item.displayName.size();
    mii.hbmpItem = CreateMenuBitmapForPidl(item.pidl);

    if (canExpand) {
        HMENU sub = CreatePopupMenu();
        AppendMenuW(sub, MF_STRING | MF_GRAYED, 0, L"(Loading…)");
        UINT id = g_menuNextId++;
        g_menuIdToPidl.push_back(ILCloneFull(item.pidl)); // clone for right-click context menu
        g_pendingSubmenus.push_back({sub, item.pidl, depth + 1});
        item.pidl = nullptr;
        mii.fMask |= MIIM_SUBMENU | MIIM_ID;
        mii.hSubMenu = sub;
        mii.wID = id;
    } else {
        UINT id = g_menuNextId++;
        g_menuIdToPidl.push_back(item.pidl);
        item.pidl = nullptr;
        mii.fMask |= MIIM_ID;
        mii.wID = id;
    }

    InsertMenuItemW(menu, position, TRUE, &mii);
}

static void AddShellFolderItemsToMenu(HMENU menu, PCIDLIST_ABSOLUTE folderPidl, int depth, UINT startPosition) {
    auto items = EnumerateShellFolder(folderPidl);
    if (items.empty()) {
        AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, L"(empty)");
        return;
    }

    UINT position = startPosition;
    for (auto& item : items)
        InsertShellMenuItem(menu, position++, item, depth);

    for (auto& item : items)
        if (item.pidl) CoTaskMemFree(item.pidl);
}

static void InvokePidl(HWND owner, PCIDLIST_ABSOLUTE pidl) {
    SHELLEXECUTEINFOW sei{};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_IDLIST | SEE_MASK_INVOKEIDLIST | SEE_MASK_ASYNCOK;
    sei.hwnd = owner;
    sei.lpVerb = L"open";
    sei.lpIDList = (void*)pidl;
    sei.nShow = SW_SHOWNORMAL;
    ShellExecuteExW(&sei);
}

static void ShowShellContextMenu(HWND owner, PCIDLIST_ABSOLUTE pidl) {
    IShellFolder* parent = nullptr;
    PCUITEMID_CHILD child = nullptr;
    if (FAILED(SHBindToParent(pidl, IID_IShellFolder, (void**)&parent, &child)) || !parent)
        return;

    IContextMenu* ctxMenu = nullptr;
    HRESULT hr = parent->GetUIObjectOf(owner, 1, &child, IID_IContextMenu, nullptr, (void**)&ctxMenu);
    parent->Release();
    if (FAILED(hr) || !ctxMenu)
        return;

    HMENU hPopup = CreatePopupMenu();
    ctxMenu->QueryContextMenu(hPopup, 0, 1, 0x7FFF, CMF_NORMAL);

    POINT pt;
    GetCursorPos(&pt);
    int cmd = (int)TrackPopupMenu(hPopup, TPM_RETURNCMD | TPM_RIGHTBUTTON,
                                   pt.x, pt.y, 0, owner, nullptr);
    if (cmd > 0) {
        CMINVOKECOMMANDINFO ici{};
        ici.cbSize = sizeof(ici);
        ici.hwnd = owner;
        ici.lpVerb = MAKEINTRESOURCEA(cmd - 1);
        ici.nShow = SW_SHOWNORMAL;
        ctxMenu->InvokeCommand(&ici);
    }

    DestroyMenu(hPopup);
    ctxMenu->Release();
}

static LRESULT CALLBACK MenuOwnerSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
    UINT_PTR /*subclassId*/, DWORD_PTR /*data*/) {
    if (msg == WM_INITMENUPOPUP) {
        HMENU hMenu = (HMENU)wParam;
        for (size_t i = 0; i < g_pendingSubmenus.size(); ++i) {
            if (g_pendingSubmenus[i].hmenu == hMenu) {
                int d = g_pendingSubmenus[i].depth;
                PIDLIST_ABSOLUTE pidl = g_pendingSubmenus[i].pidl;
                g_pendingSubmenus[i].pidl = nullptr;
                g_pendingSubmenus.erase(g_pendingSubmenus.begin() + i);
                while (GetMenuItemCount(hMenu) > 0)
                    RemoveMenu(hMenu, 0, MF_BYPOSITION);

                // "Open in Explorer" header with its own ID for direct open and right-click.
                UINT openId = g_menuNextId++;
                g_menuIdToPidl.push_back(ILCloneFull(pidl));
                MENUITEMINFOW openMii{};
                openMii.cbSize = sizeof(openMii);
                openMii.fMask = MIIM_STRING | MIIM_ID;
                openMii.wID = openId;
                wchar_t openLabel[] = L"Open in Explorer";
                openMii.dwTypeData = openLabel;
                openMii.cch = (UINT)wcslen(openLabel);
                InsertMenuItemW(hMenu, 0, TRUE, &openMii);
                AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);

                AddShellFolderItemsToMenu(hMenu, pidl, d, 2);
                CoTaskMemFree(pidl);
                break;
            }
        }
    }

    if (msg == WM_MENURBUTTONDOWN) {
        UINT index = (UINT)wParam;
        HMENU hMenu = (HMENU)lParam;
        MENUITEMINFOW mii{};
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_ID;
        if (GetMenuItemInfo(hMenu, index, TRUE, &mii) && mii.wID >= 1000) {
            size_t idx = mii.wID - 1000;
            if (idx < g_menuIdToPidl.size() && g_menuIdToPidl[idx])
                ShowShellContextMenu(hwnd, g_menuIdToPidl[idx]);
        }
        return 0;
    }

    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

static void ShowFolderMenu(FolderEntry folder) {
    HWND owner = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!owner)
        owner = GetForegroundWindow();

    g_menuNextId = 1000;
    FreeMenuState();

    HMENU menu = CreatePopupMenu();
    PIDLIST_ABSOLUTE rootPidl = ParseShellTarget(folder.target);
    if (!rootPidl) {
        AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, L"(target not found)");
        Wh_Log(L"[Menu] Failed to parse target: %s", folder.target.c_str());
    } else {
        AddShellFolderItemsToMenu(menu, rootPidl, 0);
    }

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(owner);

    // Align the menu so it opens away from the taskbar edge.
    UINT tpmAlign = TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_BOTTOMALIGN;
    if (owner) {
        APPBARDATA abd{};
        abd.cbSize = sizeof(abd);
        abd.hWnd = owner;
        SHAppBarMessage(ABM_GETTASKBARPOS, &abd);
        switch (abd.uEdge) {
            case ABE_TOP:   tpmAlign = TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN;   break;
            case ABE_LEFT:  tpmAlign = TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN;   break;
            case ABE_RIGHT: tpmAlign = TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_RIGHTALIGN | TPM_TOPALIGN;  break;
            default:        break; // ABE_BOTTOM: default flags are correct
        }
    }

    SetWindowSubclass(owner, MenuOwnerSubclassProc, 1, 0);
    UINT cmd = TrackPopupMenu(menu, tpmAlign, pt.x, pt.y, 0, owner, nullptr);
    RemoveWindowSubclass(owner, MenuOwnerSubclassProc, 1);

    if (cmd >= 1000) {
        size_t idx = cmd - 1000;
        if (idx < g_menuIdToPidl.size() && g_menuIdToPidl[idx])
            InvokePidl(owner, g_menuIdToPidl[idx]);
    }

    DestroyMenu(menu);
    if (rootPidl)
        CoTaskMemFree(rootPidl);
    FreeMenuState();

}

// ============================================================
// Button grid
// ============================================================

static Brush ParseColorBrush(std::wstring const& hex) {
    if (hex.empty() || hex[0] != L'#')
        return nullptr;

    std::wstring h = hex.substr(1);
    if (h.size() == 6)
        h = L"FF" + h;
    if (h.size() != 8)
        return nullptr;

    UINT32 val = 0;
    for (wchar_t c : h) {
        val <<= 4;
        if (c >= L'0' && c <= L'9')
            val |= (UINT32)(c - L'0');
        else if (c >= L'A' && c <= L'F')
            val |= (UINT32)(10 + c - L'A');
        else if (c >= L'a' && c <= L'f')
            val |= (UINT32)(10 + c - L'a');
        else
            return nullptr;
    }

    winrt::Windows::UI::Color color;
    color.A = (BYTE)(val >> 24);
    color.R = (BYTE)(val >> 16);
    color.G = (BYTE)(val >> 8);
    color.B = (BYTE)val;
    SolidColorBrush brush;
    brush.Color(color);
    return brush;
}

static SolidColorBrush MakeBrush(BYTE a, BYTE r, BYTE g, BYTE b) {
    winrt::Windows::UI::Color c;
    c.A = a; c.R = r; c.G = g; c.B = b;
    SolidColorBrush brush;
    brush.Color(c);
    return brush;
}

static Brush MakeShineBrush(Brush const& base) {
    if (!g_settings.shineEffect)
        return base;

    auto solid = base ? base.try_as<SolidColorBrush>() : nullptr;
    if (!solid)
        return base;

    auto c = solid.Color();
    auto adjust = [](BYTE value, int delta) {
        return (BYTE)std::clamp((int)value + delta, 0, 255);
    };

    LinearGradientBrush brush;
    brush.StartPoint({0.0, 0.0});
    brush.EndPoint({0.0, 1.0});

    GradientStop shine;
    winrt::Windows::UI::Color shineColor{180, 255, 255, 255};
    shine.Color(shineColor);
    shine.Offset(0.0);
    brush.GradientStops().Append(shine);

    GradientStop light;
    winrt::Windows::UI::Color lightColor{
        c.A, adjust(c.R, 34), adjust(c.G, 34), adjust(c.B, 34)};
    light.Color(lightColor);
    light.Offset(0.42);
    brush.GradientStops().Append(light);

    GradientStop baseStop;
    baseStop.Color(c);
    baseStop.Offset(0.52);
    brush.GradientStops().Append(baseStop);

    GradientStop dark;
    winrt::Windows::UI::Color darkColor{
        c.A, adjust(c.R, -28), adjust(c.G, -28), adjust(c.B, -28)};
    dark.Color(darkColor);
    dark.Offset(1.0);
    brush.GradientStops().Append(dark);

    return brush;
}

static void ApplyButtonStyle(Button btn,
                             Brush const& textBrush,
                             Brush const& bgBrush,
                             Brush const& hoverBgBrush,
                             Brush const& pressedBgBrush,
                             Brush const& borderBrush) {
    auto res = btn.Resources();

    // Foreground: pin to custom color across all visual states so the
    // VisualStateManager can't override it on hover/press.
    if (textBrush) {
        btn.Foreground(textBrush);
        res.Insert(winrt::box_value(L"ButtonForeground"), textBrush);
        res.Insert(winrt::box_value(L"ButtonForegroundPointerOver"), textBrush);
        res.Insert(winrt::box_value(L"ButtonForegroundPressed"), textBrush);
    }

    // Background: set both the live property and the theme resource.
    if (bgBrush) {
        auto buttonBgBrush = MakeShineBrush(bgBrush);
        btn.Background(buttonBgBrush);
        res.Insert(winrt::box_value(L"ButtonBackground"), buttonBgBrush);
    }

    // Hover: explicit > same-as-bg > default subtle white brightening.
    // Using Resources() here is essential — PointerEntered events fire
    // before the VisualStateManager animation, which would override any
    // Background() call we make there.
    Brush hoverBrush = hoverBgBrush;
    bool hoverUsesCustomSurface = true;
    if (!hoverBrush) {
        if (bgBrush) {
            hoverBrush = bgBrush;
        } else {
            hoverBrush = MakeBrush(0xFF, 0x44, 0x88, 0xFF);
            hoverUsesCustomSurface = false;
        }
    }
    if (hoverUsesCustomSurface)
        hoverBrush = MakeShineBrush(hoverBrush);
    res.Insert(winrt::box_value(L"ButtonBackgroundPointerOver"), hoverBrush);

    // Pressed: explicit > hover > slightly stronger brightening.
    Brush pressBrush = pressedBgBrush;
    bool pressUsesCustomSurface = true;
    if (!pressBrush) {
        if (hoverBgBrush || bgBrush) {
            pressBrush = hoverBrush;
        } else {
            pressBrush = MakeBrush(70, 255, 255, 255);
            pressUsesCustomSurface = false;
        }
    }
    if (pressUsesCustomSurface)
        pressBrush = MakeShineBrush(pressBrush);
    res.Insert(winrt::box_value(L"ButtonBackgroundPressed"), pressBrush);

    // Border.
    if (borderBrush) {
        btn.BorderBrush(borderBrush);
        res.Insert(winrt::box_value(L"ButtonBorderBrush"), borderBrush);
        res.Insert(winrt::box_value(L"ButtonBorderBrushPointerOver"), borderBrush);
        res.Insert(winrt::box_value(L"ButtonBorderBrushPressed"), borderBrush);
    }

    if (g_settings.borderThickness >= 0) {
        double t = (double)g_settings.borderThickness;
        btn.BorderThickness({ t, t, t, t });
    }

    if (g_settings.cornerRadius >= 0) {
        double r = (double)g_settings.cornerRadius;
        btn.CornerRadius({ r, r, r, r });
    }
}

struct FolderGridLayout {
    int rows = 1;
    int cols = 1;
    bool columnFirst = false;
};

static FolderGridLayout ComputeFolderGridLayout(int count) {
    FolderGridLayout layout;
    layout.columnFirst = g_settings.fillOrder == L"columnFirst";

    if (g_settings.layoutMode == L"column") {
        layout.rows = count;
        layout.cols = 1;
        return layout;
    }

    if (g_settings.layoutMode != L"grid") {
        layout.rows = 1;
        layout.cols = count;
        return layout;
    }

    layout.cols = std::max(1, g_settings.gridColumns);
    layout.rows = g_settings.gridRows > 0 ? g_settings.gridRows
                                          : (count + layout.cols - 1) / layout.cols;

    if (layout.rows * layout.cols < count) {
        if (layout.columnFirst) {
            layout.cols = (count + layout.rows - 1) / layout.rows;
        } else {
            layout.rows = (count + layout.cols - 1) / layout.cols;
        }
    }

    return layout;
}

static int GetShortGroupOffset(int capacity, int countInGroup) {
    int leftover = capacity - countInGroup;
    if (leftover <= 0)
        return 0;

    if (g_settings.shortGroupAlign == L"center")
        return leftover / 2;

    if (g_settings.shortGroupAlign == L"end")
        return leftover;

    return 0;
}

static Grid BuildFolderButtonGrid() {
    int count = (int)g_settings.folders.size();
    auto layout = ComputeFolderGridLayout(count);

    Grid grid;
    grid.Name(L"TaskbarFolderMenuBar");
    grid.VerticalAlignment(VerticalAlignment::Center);
    grid.Margin({(double)g_settings.groupPaddingLeft, 0.0,
                 (double)g_settings.groupPaddingRight, 0.0});
    if (g_settings.groupOffsetX || g_settings.groupOffsetY) {
        TranslateTransform transform;
        transform.X((double)g_settings.groupOffsetX);
        transform.Y((double)g_settings.groupOffsetY);
        grid.RenderTransform(transform);
    }
    if (g_settings.buttonSpacing > 0) {
        grid.ColumnSpacing((double)g_settings.buttonSpacing);
        grid.RowSpacing((double)g_settings.buttonSpacing);
    }

    for (int r = 0; r < layout.rows; r++) {
        RowDefinition rd;
        rd.Height({ (double)g_settings.buttonHeight, GridUnitType::Pixel });
        grid.RowDefinitions().Append(rd);
    }
    for (int c = 0; c < layout.cols; c++) {
        ColumnDefinition cd;
        cd.Width({ (double)g_settings.buttonWidth, GridUnitType::Pixel });
        grid.ColumnDefinitions().Append(cd);
    }

    auto textBrush = ParseColorBrush(g_settings.textColor);
    auto bgBrush = ParseColorBrush(g_settings.backgroundColor);
    auto hoverBgBrush = ParseColorBrush(g_settings.hoverBackgroundColor);
    auto pressedBgBrush = ParseColorBrush(g_settings.pressedBackgroundColor);
    auto borderBrush = ParseColorBrush(g_settings.borderColor);

    for (int i = 0; i < count; i++) {
        auto entry = g_settings.folders[i];
        std::wstring caption = entry.label;
        if (caption.empty())
            caption = g_settings.buttonText.empty() ? L"📁" : g_settings.buttonText;

        Button btn;
        btn.Name(L"FolderMenuButton_" + std::to_wstring(i));
        btn.Content(winrt::box_value(winrt::hstring(caption)));
        btn.Width((double)g_settings.buttonWidth);
        btn.Height((double)g_settings.buttonHeight);
        btn.Padding({ 0.0, 0.0, 0.0, 1.0 });
        btn.FontSize((double)g_settings.fontSize);
        btn.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
        btn.HorizontalAlignment(HorizontalAlignment::Stretch);
        btn.VerticalAlignment(VerticalAlignment::Stretch);
        ApplyButtonStyle(btn, textBrush, bgBrush, hoverBgBrush, pressedBgBrush, borderBrush);
        if (g_settings.opacityPct != 100)
            btn.Opacity((double)g_settings.opacityPct / 100.0);
        ToolTipService::SetToolTip(btn,
            winrt::box_value(winrt::hstring(entry.label + L"\n" + entry.target)));

        btn.Click([entry](auto const&, auto const&) {
            if (!g_unloading)
                ShowFolderMenu(entry);
        });

        int row;
        int col;
        if (layout.columnFirst) {
            col = i / layout.rows;
            row = i % layout.rows;

            int lastCol = (count - 1) / layout.rows;
            if (col == lastCol) {
                int countInCol = count - lastCol * layout.rows;
                row += GetShortGroupOffset(layout.rows, countInCol);
            }
        } else {
            row = i / layout.cols;
            col = i % layout.cols;

            int lastRow = (count - 1) / layout.cols;
            if (row == lastRow) {
                int countInRow = count - lastRow * layout.cols;
                col += GetShortGroupOffset(layout.cols, countInRow);
            }
        }
        Grid::SetRow(btn, row);
        Grid::SetColumn(btn, col);
        grid.Children().Append(btn);
    }

    return grid;
}

// ============================================================
// Injection / cleanup
// ============================================================

static bool RemoveButtonGridFrom(Grid gridParent, int col) {
    if (!gridParent) return false;

    bool removed = false;
    int liveCol = col;
    for (uint32_t i = 0; i < gridParent.Children().Size(); i++) {
        auto fe = gridParent.Children().GetAt(i).try_as<FrameworkElement>();
        if (fe && fe.Name() == L"TaskbarFolderMenuBar") {
            liveCol = Grid::GetColumn(fe);
            gridParent.Children().RemoveAt(i);
            removed = true;
            break;
        }
    }

    if (removed && liveCol >= 0) {
        uint32_t colU = (uint32_t)liveCol;
        if (colU < gridParent.ColumnDefinitions().Size())
            gridParent.ColumnDefinitions().RemoveAt(colU);
        for (auto child : gridParent.Children()) {
            auto fe = child.try_as<FrameworkElement>();
            if (!fe) continue;
            int c = Grid::GetColumn(fe);
            int span = Grid::GetColumnSpan(fe);
            if (c > liveCol)
                Grid::SetColumn(fe, c - 1);
            else if (c < liveCol && c + span > liveCol)
                Grid::SetColumnSpan(fe, span - 1);
        }
    }

    return removed;
}

static void RemoveButtonGrid() {
    auto gridParent = FindLiveSystemTrayFrameGrid().try_as<Grid>();
    if (!RemoveButtonGridFrom(gridParent, g_injectedColumn))
        Wh_Log(L"[Remove] TaskbarFolderMenuBar not found");

    g_buttonGrid = nullptr;
    g_injectionParent = nullptr;
    g_injectedColumn = -1;
}

static bool InjectButtonGrid(FrameworkElement root) {
    auto parent = FindChildRecursive(root, [](FrameworkElement fe) {
        return fe.Name() == L"SystemTrayFrameGrid";
    });
    if (!parent) {
        Wh_Log(L"[Inject] SystemTrayFrameGrid not found");
        return false;
    }

    auto gridParent = parent.try_as<Grid>();
    if (!gridParent) {
        Wh_Log(L"[Inject] SystemTrayFrameGrid is not a Grid");
        return false;
    }

    for (auto child : gridParent.Children()) {
        if (auto fe = child.try_as<FrameworkElement>();
            fe && fe.Name() == L"TaskbarFolderMenuBar")
            return true;
    }

    auto findNamedDirect = [&](PCWSTR name) -> FrameworkElement {
        for (auto child : gridParent.Children()) {
            if (auto fe = child.try_as<FrameworkElement>(); fe && fe.Name() == name)
                return fe;
        }
        return nullptr;
    };

    const auto& pos = g_settings.position;
    FrameworkElement refElem = nullptr;
    bool insertAfterRef = false;

    if (pos == L"beforeOmni")
        refElem = findNamedDirect(L"ControlCenterButton");
    else if (pos == L"beforeClock")
        refElem = findNamedDirect(L"NotificationCenterButton");
    else if (pos == L"afterClock")
        refElem = findNamedDirect(L"ShowDesktopStack");
    else if (pos == L"afterShowDesktop") {
        refElem = findNamedDirect(L"ShowDesktopStack");
        insertAfterRef = true;
    }

    int insertCol;
    if (insertAfterRef && refElem)
        insertCol = Grid::GetColumn(refElem) + 1;
    else if (refElem)
        insertCol = Grid::GetColumn(refElem);
    else
        insertCol = 0;

    ColumnDefinition cd;
    cd.Width({ 1.0, GridUnitType::Auto });
    if ((uint32_t)insertCol < gridParent.ColumnDefinitions().Size())
        gridParent.ColumnDefinitions().InsertAt((uint32_t)insertCol, cd);
    else
        gridParent.ColumnDefinitions().Append(cd);

    for (auto child : gridParent.Children()) {
        auto fe = child.try_as<FrameworkElement>();
        if (!fe) continue;
        int col = Grid::GetColumn(fe);
        int span = Grid::GetColumnSpan(fe);
        if (col >= insertCol)
            Grid::SetColumn(fe, col + 1);
        else if (col + span > insertCol)
            Grid::SetColumnSpan(fe, span + 1);
    }

    auto grid = BuildFolderButtonGrid();
    Grid::SetColumn(grid, insertCol);
    gridParent.Children().Append(grid);

    g_buttonGrid = grid;
    g_injectionParent = parent;
    g_injectedColumn = insertCol;

    Wh_Log(L"[Inject] TaskbarFolderMenuBar at column=%d, folders=%d",
           insertCol, (int)g_settings.folders.size());
    return true;
}

static void ApplyAllSettings() {
    HWND hWnd = FindCurrentProcessTaskbarWnd();
    if (!hWnd) {
        Wh_Log(L"[Apply] No taskbar window");
        return;
    }
    g_taskbarWnd = hWnd;

    auto xamlRoot = GetTaskbarXamlRoot(hWnd);
    if (!xamlRoot) {
        Wh_Log(L"[Apply] GetTaskbarXamlRoot failed");
        return;
    }
    auto root = xamlRoot.Content().try_as<FrameworkElement>();
    if (!root) {
        Wh_Log(L"[Apply] No XAML root content");
        return;
    }

    if (!InjectButtonGrid(root))
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

using LoadLibraryExW_t = HMODULE (WINAPI*)(LPCWSTR, HANDLE, DWORD);
LoadLibraryExW_t LoadLibraryExW_Original;

HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR path, HANDLE file, DWORD flags) {
    HMODULE h = LoadLibraryExW_Original(path, file, flags);
    if (h && path && !g_taskbarViewDllLoaded) {
        const wchar_t* base = wcsrchr(path, L'\\');
        base = base ? base + 1 : path;
        if (_wcsicmp(base, L"Taskbar.View.dll") == 0) {
            g_taskbarViewDllLoaded = true;
            Wh_Log(L"[LLEW] Taskbar.View.dll loaded");
            ApplyAllSettingsOnWindowThread();
        }
    }
    return h;
}

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

static HMODULE GetTaskbarViewModule() {
    for (auto* name : { L"Taskbar.View.dll", L"taskbar.view.dll" })
        if (HMODULE h = GetModuleHandleW(name)) return h;
    return nullptr;
}

static void StopRetryThread() {
    HANDLE thread = g_retryThread;
    HANDLE event = g_retryStopEvent;

    if (event)
        SetEvent(event);

    if (thread) {
        WaitForSingleObject(thread, 3000);
        CloseHandle(thread);
    }

    if (event)
        CloseHandle(event);

    g_retryThread = nullptr;
    g_retryStopEvent = nullptr;
}

// ============================================================
// Windhawk lifecycle
// ============================================================

BOOL Wh_ModInit() {
    Wh_Log(L"[Init] Taskbar Folder Menus v0.5");
    LoadSettings();

    if (!HookTaskbarDllSymbols())
        Wh_Log(L"[Init] taskbar.dll hooks failed - XamlRoot unavailable");

    if (GetTaskbarViewModule()) {
        g_taskbarViewDllLoaded = true;
    } else {
        HMODULE kb = GetModuleHandleW(L"kernelbase.dll");
        auto pLLEW = kb ? (LoadLibraryExW_t)GetProcAddress(kb, "LoadLibraryExW") : nullptr;
        if (pLLEW)
            Wh_SetFunctionHook((void*)pLLEW, (void*)LoadLibraryExW_Hook, (void**)&LoadLibraryExW_Original);
    }

    return TRUE;
}

void Wh_ModAfterInit() {
    if (!g_taskbarViewDllLoaded && GetTaskbarViewModule())
        g_taskbarViewDllLoaded = true;

    if (g_taskbarViewDllLoaded)
        ApplyAllSettingsOnWindowThread();

    g_retryStopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!g_retryStopEvent)
        return;

    HANDLE stopEvent = g_retryStopEvent;
    g_retryThread = CreateThread(nullptr, 0, [](void* param) -> DWORD {
        HANDLE stopEvent = static_cast<HANDLE>(param);
        for (int i = 0; i < 5 && !g_unloading; i++) {
            if (WaitForSingleObject(stopEvent, 2000) != WAIT_TIMEOUT) break;
            if (g_buttonGrid) break;
            Wh_Log(L"[AfterInit] Retry %d", i + 1);
            ApplyAllSettingsOnWindowThread();
        }
        return 0;
    }, stopEvent, 0, nullptr);

    if (!g_retryThread) {
        CloseHandle(g_retryStopEvent);
        g_retryStopEvent = nullptr;
    }
}

void Wh_ModUninit() {
    g_unloading = true;
    Wh_Log(L"[Uninit]");

    StopRetryThread();

    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (hWnd)
        RunFromWindowThread(hWnd, [](void*) { RemoveButtonGrid(); }, nullptr);
    else
        RemoveButtonGrid();
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Wh_Log(L"[Settings] Changed");

    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return;

    RunFromWindowThread(hWnd, [](void*) {
        RemoveButtonGrid();
        ApplyAllSettings();
    }, nullptr);
}
