"""Replace the Folder Menus host boilerplate with its shared equivalents."""
from pathlib import Path

path = Path('taskbar-folder-menus/taskbar-folder-menus.wh.cpp')
source = path.read_text(encoding='utf-8')

def replace_between(first, after, replacement):
    global source
    a = source.index(first)
    b = source.index(after, a)
    source = source[:a] + replacement + '\n\n' + source[b:]

replace_between('using CTaskBand_GetTaskbarHost_t', 'static FrameworkElement FindLiveSystemTrayFrameGrid()', '''
static XamlRoot GetTaskbarXamlRoot(HWND window) {
    return tbh::GetTaskbarXamlRoot(window);
}

static bool RunFromWindowThread(HWND window, tbh::ThreadProc callback, void* parameter) {
    return tbh::RunFromWindowThread(window, callback, parameter,
        L"Windhawk_RunFromWindowThread_" WH_MOD_ID);
}

static HWND FindCurrentProcessTaskbarWnd() {
    return tbh::FindCurrentProcessTaskbarWnd();
}

static FrameworkElement FindChildRecursive(FrameworkElement const& root,
    std::function<bool(FrameworkElement)> const& predicate, int maxDepth = 20) {
    return vtw::FindDescendant(root, maxDepth, predicate);
}
''')
replace_between('using TrayUI_StartTaskbar_t =', 'static void StopRetryThread() {', '''
static bool HookTaskbarDllSymbols() {
    return tbh::HookTaskbarSymbols([] {
        if (!g_unloading && !g_updatingSettings) StartRetryThread();
    });
}
''')
path.write_text(source, encoding='utf-8', newline='\n')
print('Adopted shared host discovery, dispatch, rebuild hook and visual-tree traversal.')
