// ---- Window discovery -------------------------------------------------------

//! LAB NOTE - stripped on assembly, do not turn these into shipped comments.
//!
//! Every mod in this family once wrote the cache as an unvalidated ternary:
//!
//!     HWND w = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
//!
//! After Shell_TrayWnd is recreated inside the same Explorer process that
//! hands back a dead handle forever, because the live window is only ever
//! looked up when the cache is null. GetWindowThreadProcessId then returns 0,
//! the UI dispatch fails, and the caller silently does nothing - survivable on
//! a retry path, NOT on the unload path, where it means the mod's callbacks
//! are never revoked before its image is freed. Flagged by the AI review on
//! PR #4855; submission-preflight.ps1 now rejects the bare ternary.

inline HWND FindCurrentProcessTaskbarWnd() {
    HWND result = nullptr;
    EnumWindows(
        [](HWND window, LPARAM parameter) -> BOOL {
            DWORD processId = 0;
            WCHAR className[32];
            if (GetWindowThreadProcessId(window, &processId) &&
                processId == GetCurrentProcessId() &&
                GetClassName(window, className, ARRAYSIZE(className)) &&
                _wcsicmp(className, L"Shell_TrayWnd") == 0) {
                *reinterpret_cast<HWND*>(parameter) = window;
                return FALSE;
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&result));
    return result;
}

// Shell_TrayWnd can be recreated inside Explorer. A cache is useful only while
// it names a live window; otherwise rediscover before dispatch or teardown.
inline HWND ResolveTaskbarWnd(HWND cached) {
    if (cached && IsWindow(cached))
        return cached;
    return FindCurrentProcessTaskbarWnd();
}
