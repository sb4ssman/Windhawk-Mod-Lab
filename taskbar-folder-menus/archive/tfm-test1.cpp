// ==WindhawkMod==
// @id              taskbar-folder-menu
// @name            Taskbar Folder Menu
// @description     Adds compact taskbar buttons that open configured folders as popup menus, similar to classic Windows taskbar toolbars.
// @version         0.3
// @author          sb4ssman
// @github          https://github.com/sb4ssman
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lversion -lshell32 -luuid
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
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Media.h>

#include <algorithm>
#include <atomic>
#include <cwctype>
#include <functional>
#include <string>
#include <vector>

#include <shellapi.h>
#include <shlobj.h>
#include <windhawk_utils.h>
#include <winver.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

// ============================================================
// Settings
// ============================================================

struct FolderEntry {
    std::wstring label;
    std::wstring path;
};

struct ModSettings {
    std::wstring position = L"beforeIcons";
    std::wstring layoutMode = L"row";
    std::wstring buttonText = L"📁";
    std::vector<FolderEntry> folders;
    int gridColumns = 2;
    int buttonWidth = 20;
    int buttonHeight = 22;
    int buttonSpacing = 2;
    int maxMenuItems = 80;
    int maxDepth = 2;
    bool showHidden = false;
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
            entry.path = ExpandEnv(Trim(line));
            entry.label = FileNameFromPath(entry.path);
        } else {
            entry.label = Trim(line.substr(0, eq));
            entry.path = ExpandEnv(Trim(line.substr(eq + 1)));
            if (entry.label.empty())
                entry.label = FileNameFromPath(entry.path);
        }
        if (!entry.path.empty())
            folders.push_back(entry);
    }
    if (folders.empty()) {
        PWSTR knownPath = nullptr;
        std::wstring desktopPath;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &knownPath))) {
            desktopPath = knownPath;
            CoTaskMemFree(knownPath);
        } else {
            desktopPath = ExpandEnv(L"%USERPROFILE%\\Desktop");
        }
        folders.push_back({ L"📁", desktopPath });
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
    g_settings.position = GetStringSetting(L"position", L"beforeIcons");
    g_settings.layoutMode = GetStringSetting(L"layoutMode", L"row");
    g_settings.buttonText = GetStringSetting(L"buttonText", L"📁");
    g_settings.folders = ParseFolders(GetStringSetting(L"folders", L"📁=%DESKTOP%"));
    g_settings.gridColumns = std::max(1, Wh_GetIntSetting(L"gridColumns", 2));
    g_settings.buttonWidth = std::max(10, Wh_GetIntSetting(L"buttonWidth", 20));
    g_settings.buttonHeight = std::max(10, Wh_GetIntSetting(L"buttonHeight", 22));
    g_settings.buttonSpacing = std::max(0, Wh_GetIntSetting(L"buttonSpacing", 2));
    g_settings.maxMenuItems = std::max(0, Wh_GetIntSetting(L"maxMenuItems", 80));
    g_settings.maxDepth = std::max(0, Wh_GetIntSetting(L"maxDepth", 2));
    g_settings.showHidden = Wh_GetIntSetting(L"showHidden", 0) != 0;
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

struct MenuItemInfo {
    std::wstring name;
    std::wstring path;
    bool isDir = false;
};

static bool ShouldSkip(WIN32_FIND_DATAW const& fd) {
    if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
        return true;
    if (!g_settings.showHidden &&
        (fd.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)))
        return true;
    return false;
}

static std::vector<MenuItemInfo> EnumerateFolder(std::wstring const& folder) {
    std::vector<MenuItemInfo> items;
    std::wstring pattern = folder;
    if (!pattern.empty() && pattern.back() != L'\\' && pattern.back() != L'/')
        pattern += L"\\";
    pattern += L"*";

    WIN32_FIND_DATAW fd{};
    HANDLE find = FindFirstFileW(pattern.c_str(), &fd);
    if (find == INVALID_HANDLE_VALUE)
        return items;

    do {
        if (ShouldSkip(fd))
            continue;
        MenuItemInfo item;
        item.name = fd.cFileName;
        item.path = folder;
        if (!item.path.empty() && item.path.back() != L'\\' && item.path.back() != L'/')
            item.path += L"\\";
        item.path += fd.cFileName;
        item.isDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        items.push_back(std::move(item));
    } while (FindNextFileW(find, &fd));
    FindClose(find);

    std::sort(items.begin(), items.end(), [](auto const& a, auto const& b) {
        if (a.isDir != b.isDir) return a.isDir > b.isDir;
        return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0;
    });
    if (g_settings.maxMenuItems > 0 && (int)items.size() > g_settings.maxMenuItems)
        items.resize(g_settings.maxMenuItems);
    return items;
}

static void AddFolderItemsToMenu(HMENU menu, std::wstring const& folder,
                                 int depth, UINT& nextId,
                                 std::vector<std::wstring>& idToPath) {
    auto items = EnumerateFolder(folder);
    if (items.empty()) {
        AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, L"(empty)");
        return;
    }

    for (auto const& item : items) {
        if (item.isDir && (g_settings.maxDepth == 0 || depth < g_settings.maxDepth)) {
            HMENU sub = CreatePopupMenu();
            AddFolderItemsToMenu(sub, item.path, depth + 1, nextId, idToPath);
            AppendMenuW(menu, MF_POPUP | MF_STRING, (UINT_PTR)sub, item.name.c_str());
        } else {
            UINT id = nextId++;
            idToPath.push_back(item.path);
            AppendMenuW(menu, MF_STRING, id, item.name.c_str());
        }
    }
}

static void ShowFolderMenu(FolderEntry folder) {
    HWND owner = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!owner)
        owner = GetForegroundWindow();

    HMENU menu = CreatePopupMenu();
    UINT nextId = 1000;
    std::vector<std::wstring> idToPath;

    DWORD attrs = GetFileAttributesW(folder.path.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, L"(folder not found)");
    } else {
        AddFolderItemsToMenu(menu, folder.path, 0, nextId, idToPath);
    }

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(owner);
    UINT cmd = TrackPopupMenu(menu,
        TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY | TPM_BOTTOMALIGN,
        pt.x, pt.y, 0, owner, nullptr);

    if (cmd >= 1000) {
        size_t idx = cmd - 1000;
        if (idx < idToPath.size()) {
            ShellExecuteW(owner, L"open", idToPath[idx].c_str(),
                          nullptr, nullptr, SW_SHOWNORMAL);
        }
    }

    DestroyMenu(menu);
}

// ============================================================
// Button grid
// ============================================================

static Grid BuildFolderButtonGrid() {
    int count = (int)g_settings.folders.size();
    int cols = count;
    int rows = 1;
    if (g_settings.layoutMode == L"column") {
        cols = 1;
        rows = count;
    } else if (g_settings.layoutMode == L"grid") {
        cols = std::max(1, g_settings.gridColumns);
        rows = (count + cols - 1) / cols;
    }

    Grid grid;
    grid.Name(L"TaskbarFolderMenuBar");
    grid.VerticalAlignment(VerticalAlignment::Center);
    if (g_settings.buttonSpacing > 0) {
        grid.ColumnSpacing((double)g_settings.buttonSpacing);
        grid.RowSpacing((double)g_settings.buttonSpacing);
    }

    for (int r = 0; r < rows; r++) {
        RowDefinition rd;
        rd.Height({ (double)g_settings.buttonHeight, GridUnitType::Pixel });
        grid.RowDefinitions().Append(rd);
    }
    for (int c = 0; c < cols; c++) {
        ColumnDefinition cd;
        cd.Width({ (double)g_settings.buttonWidth, GridUnitType::Pixel });
        grid.ColumnDefinitions().Append(cd);
    }

    for (int i = 0; i < count; i++) {
        auto entry = g_settings.folders[i];
        std::wstring caption = entry.label;
        if (caption.empty() || caption.size() > 2)
            caption = g_settings.buttonText.empty() ? L"📁" : g_settings.buttonText;

        Button btn;
        btn.Name(L"FolderMenuButton_" + std::to_wstring(i));
        btn.Content(winrt::box_value(winrt::hstring(caption)));
        btn.Width((double)g_settings.buttonWidth);
        btn.Height((double)g_settings.buttonHeight);
        btn.Padding({ 0.0, 0.0, 0.0, 1.0 });
        btn.FontSize(10.0);
        btn.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
        btn.HorizontalAlignment(HorizontalAlignment::Stretch);
        btn.VerticalAlignment(VerticalAlignment::Stretch);
        ToolTipService::SetToolTip(btn,
            winrt::box_value(winrt::hstring(entry.label + L"\n" + entry.path)));

        btn.Click([entry](auto const&, auto const&) {
            if (!g_unloading)
                ShowFolderMenu(entry);
        });

        int row = i / cols;
        int col = i % cols;
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
    for (uint32_t i = 0; i < gridParent.Children().Size(); i++) {
        auto fe = gridParent.Children().GetAt(i).try_as<FrameworkElement>();
        if (fe && fe.Name() == L"TaskbarFolderMenuBar") {
            gridParent.Children().RemoveAt(i);
            removed = true;
            break;
        }
    }

    if (removed && col >= 0) {
        uint32_t colU = (uint32_t)col;
        if (colU < gridParent.ColumnDefinitions().Size())
            gridParent.ColumnDefinitions().RemoveAt(colU);
        for (auto child : gridParent.Children()) {
            auto fe = child.try_as<FrameworkElement>();
            if (!fe) continue;
            int c = Grid::GetColumn(fe);
            int span = Grid::GetColumnSpan(fe);
            if (c > col)
                Grid::SetColumn(fe, c - 1);
            else if (c + span > col)
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
    Wh_Log(L"[Init] Taskbar Folder Menu v0.1");
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