// Copy-source template v1.3.1: Windows 11 taskbar XAML lifecycle.
// v1.3.1: clarify that no_destroy is ONLY for strong XAML/WinRT-owning
// containers. weak_ref and plain-heap containers are thread-safe to destroy and
// must stay unannotated (m417z clarification across the 2026-07-23 PR reviews).
// v1.3: make no_destroy ownership explicit, free container storage on
// controlled unload, and serialize retry-handle ownership. Wh_ModAfterInit
// performs one immediate apply; bounded retries are reserved for taskbar
// startup/rebuild notifications.
// v1.2: contain every UI-dispatch exception at the WH_CALLWNDPROC boundary
// and verify a live XAML root before settings-change removal/reapply.
//
// Adapter contract: implement ApplyModUi() and RemoveModUi() below. Both are
// always called synchronously on the taskbar window thread. ApplyModUi returns
// true only when the UI exists or the mod intentionally has nothing to show.

#undef GetCurrentTime

#include <winrt/Windows.UI.Xaml.h>

#include <atomic>
#include <exception>
#include <optional>
#include <vector>

#include <windhawk_utils.h>
#include <winver.h>

using namespace winrt::Windows::UI::Xaml;

static bool ApplyModUi() {
    // TODO: Build/reacquire the mod UI. Return true when no retry is needed.
    return false;
}

static void RemoveModUi() {
    // TODO: Revoke events, remove the owned root/lease, and restore snapshots.
    //
    // TEARDOWN CONTRACT (crash class found 2026-07-17 in folder-menus): XAML
    // tears removed subtrees down on a LATER UI tick, which can land after the
    // mod DLL has been unloaded. Any object implemented in this DLL that the
    // subtree still references is then freed memory. Before Children().RemoveAt
    // on anything you injected, release every mod-implemented reference:
    //   1. revoke event tokens (Click/Tapped/Pointer*/property-changed);
    //   2. ToolTipService::SetToolTip(element, IInspectable{nullptr});
    //   3. null boxed Content / Tag set via winrt::box_value (the boxed
    //      wrapper's vtable lives in this DLL — system brushes/transforms are
    //      safe, boxed values and delegates are not);
    //   4. stop and clear Storyboards the mod started.
    // Reference: taskbar-ai-quota ClearQuotaEventState ("Routed event
    // delegates point into this DLL, so revoke before XAML tears down the
    // subtree"), folder-menus ClearButtonEventState.
}

static std::atomic<bool> g_templateUnloading{false};
static std::atomic<bool> g_templateApplied{false};
static HWND g_templateTaskbarWnd = nullptr;
static HANDLE g_templateRetryThread = nullptr;
static HANDLE g_templateRetryStopEvent = nullptr;
static SRWLOCK g_templateRetryLock = SRWLOCK_INIT;

// SYSTEM-HOOK CONTRACT (crash class found 2026-07-26 in OmniButton, and it
// took Explorer down): a WH_CALLWNDPROC / WH_GETMESSAGE / CBT hook proc is a
// HOSTILE INPUT SURFACE. It is invoked for EVERY message sent to EVERY window
// on the hooked thread — for the taskbar UI thread that is thousands of
// messages a second, from Windows and from every other mod.
//
//   VALIDATE THE DISCRIMINATOR BEFORE TOUCHING ANYTHING ELSE.
//
// Marshalling work onto the UI thread means sending a private registered
// message whose lParam is a pointer to your own dispatch struct. For every
// OTHER message, lParam is something else entirely — a flag, an integer, a
// pointer into a structure you do not own. So:
//
//   if (call->message == ourRegisteredMessage) {        // FIRST
//       auto* dispatch = (Dispatch*)call->lParam;       // only now
//       ...
//   }
//
// Reversing those two lines — casting lParam and reading a field out of it in
// order to decide whether the message is yours — dereferences arbitrary memory
// on the first unrelated message that arrives, which is immediately. A null
// check on the cast pointer proves nothing; the value is not null, it is
// simply not yours. This is a crash, not a glitch, and it kills explorer.exe.
//
// The same rule covers any callback Windows hands you: establish that the
// event is the one you asked for before dereferencing anything it carries.
//
// PROCESS-SHUTDOWN CONTRACT (crash class found 2026-07-19 in Privacy
// Indicator Anchor): Explorer shutdown doesn't guarantee Wh_ModUninit. Never
// let CRT global destruction release namespace-scope state after the XAML
// framework has torn down or from the shutdown thread. Classify every
// namespace-scope owner instead of applying no_destroy broadly:
//
// 1. Heap-only C++ state destructs normally — do NOT annotate it. This covers
//    settings, strings, numeric leases, AND containers whose elements are
//    trivially destructible or only own heap (e.g. std::wstring). Its
//    destructor is safe at process shutdown, and suppressing it LEAKS the
//    storage on every normal unload (disable/reload). A settings struct of
//    strings/ints/bools/arrays is trivially safe — never no_destroy it.
//
// 2. Direct nullable XAML/WinRT handles (Grid/FrameworkElement = nullptr,
//    DispatcherTimer, etc.) use no_destroy and are explicitly set to nullptr by
//    RemoveModUi on the taskbar UI thread.
//
// 3. Containers that own STRONG XAML/WinRT references, event revokers,
//    Storyboards, or delegates use a no_destroy optional<container>. These are
//    the objects that must not be released off the UI thread after framework
//    teardown. Revoke/clear their elements on settings changes. On controlled
//    Wh_ModUninit, call reset() on the UI thread after revocation so the
//    container's heap buffer is also freed. If the UI thread is unavailable,
//    intentionally keep the engaged optional and its XAML state alive.
//
//    Do NOT reach for no_destroy just to avoid an off-thread release: a
//    container of winrt::weak_ref (or plain heap) is safe to destroy from ANY
//    thread — weak_ref release is only an in-process control-block decrement.
//    Such containers fall under rule 1: leave them unannotated and release their
//    elements on unload. (m417z, PR #4443: bare no_destroy there leaked the
//    buffer for no benefit.)
//
// static ModSettings g_settings;  // exit-time-safe: heap-only
// [[clang::no_destroy]] static FrameworkElement g_ownedRoot = nullptr;
// [[clang::no_destroy]] static std::optional<std::vector<OwnedEventState>>
//     g_eventStates{std::in_place};
//
// This is intentional process-lifetime retention, not a replacement for
// cleanup. Controlled settings changes must still revoke and clear everything
// synchronously on the taskbar UI thread while leaving optional containers
// engaged for reuse. Controlled mod unload must revoke first, then reset the
// optional containers on that same UI thread. If no taskbar window/UI thread
// can be reached during Wh_ModUninit, retain all no_destroy state; do not add
// an off-thread fallback that clears XAML objects.
// Run exit-time-destructor-audit.ps1 before submission; every diagnostic must
// be eliminated or intentionally converted to a no_destroy lifetime.

using CTaskBand_GetTaskbarHost_t = void* (WINAPI*)(void*, void*);
static CTaskBand_GetTaskbarHost_t CTaskBand_GetTaskbarHost_Original;
using TaskbarHost_FrameHeight_t = int (WINAPI*)(void*);
static TaskbarHost_FrameHeight_t TaskbarHost_FrameHeight_Original;
using std__Ref_count_base__Decref_t = void (WINAPI*)(void*);
static std__Ref_count_base__Decref_t std__Ref_count_base__Decref_Original;
static void* CTaskBand_ITaskListWndSite_vftable;

static HWND FindCurrentProcessTaskbarWnd() {
    HWND result = nullptr;
    EnumWindows([](HWND window, LPARAM parameter) -> BOOL {
        DWORD processId = 0;
        wchar_t className[32];
        if (GetWindowThreadProcessId(window, &processId) &&
            processId == GetCurrentProcessId() &&
            GetClassNameW(window, className, ARRAYSIZE(className)) &&
            _wcsicmp(className, L"Shell_TrayWnd") == 0) {
            *reinterpret_cast<HWND*>(parameter) = window;
            return FALSE;
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&result));
    return result;
}

using WindowThreadProc = void (*)(void*);

static void LogCurrentUiException(PCWSTR context) noexcept {
    try {
        throw;
    } catch (winrt::hresult_error const& error) {
        Wh_Log(L"[Lifecycle] %s failed hr=0x%08X: %s", context,
               static_cast<unsigned>(error.code().value),
               error.message().c_str());
    } catch (std::exception const&) {
        Wh_Log(L"[Lifecycle] %s failed with a C++ exception", context);
    } catch (...) {
        Wh_Log(L"[Lifecycle] %s failed with an unknown exception", context);
    }
}

static bool InvokeWindowThreadProc(WindowThreadProc proc, void* parameter) {
    try {
        proc(parameter);
        return true;
    } catch (...) {
        LogCurrentUiException(L"UI callback");
    }
    return false;
}

static bool RunFromWindowThread(HWND window, WindowThreadProc proc,
                                void* parameter) {
    static UINT message = RegisterWindowMessageW(
        L"Windhawk_RunFromWindowThread_" WH_MOD_ID);
    struct Dispatch {
        WindowThreadProc proc;
        void* parameter;
        bool succeeded = false;
    };
    DWORD threadId = GetWindowThreadProcessId(window, nullptr);
    if (!threadId) return false;
    if (threadId == GetCurrentThreadId()) {
        return InvokeWindowThreadProc(proc, parameter);
    }

    HHOOK hook = SetWindowsHookExW(
        WH_CALLWNDPROC,
        [](int code, WPARAM wParam, LPARAM lParam) -> LRESULT {
            if (code == HC_ACTION) {
                auto call = reinterpret_cast<CWPSTRUCT const*>(lParam);
                static UINT dispatchMessage = RegisterWindowMessageW(
                    L"Windhawk_RunFromWindowThread_" WH_MOD_ID);
                if (call->message == dispatchMessage) {
                    auto dispatch = reinterpret_cast<Dispatch*>(call->lParam);
                    dispatch->succeeded = InvokeWindowThreadProc(
                        dispatch->proc, dispatch->parameter);
                }
            }
            return CallNextHookEx(nullptr, code, wParam, lParam);
        },
        nullptr, threadId);
    if (!hook) return false;

    Dispatch dispatch{proc, parameter};
    SendMessageW(window, message, 0, reinterpret_cast<LPARAM>(&dispatch));
    UnhookWindowsHookEx(hook);
    return dispatch.succeeded;
}

static XamlRoot GetTaskbarXamlRoot(HWND taskbarWindow) {
    if (!CTaskBand_GetTaskbarHost_Original ||
        !TaskbarHost_FrameHeight_Original ||
        !std__Ref_count_base__Decref_Original ||
        !CTaskBand_ITaskListWndSite_vftable)
        return nullptr;

    HWND taskSwitchWindow = reinterpret_cast<HWND>(
        GetPropW(taskbarWindow, L"TaskbandHWND"));
    if (!taskSwitchWindow) return nullptr;
    void* taskBand = reinterpret_cast<void*>(
        GetWindowLongPtrW(taskSwitchWindow, 0));
    if (!taskBand) return nullptr;

    void* taskBandForSite = taskBand;
    for (int i = 0;
         *reinterpret_cast<void**>(taskBandForSite) !=
             CTaskBand_ITaskListWndSite_vftable;
         ++i) {
        if (i == 20) return nullptr;
        taskBandForSite = reinterpret_cast<void**>(taskBandForSite) + 1;
    }

    void* taskbarHostSharedPtr[2]{};
    CTaskBand_GetTaskbarHost_Original(taskBandForSite,
                                     taskbarHostSharedPtr);
    if (!taskbarHostSharedPtr[0] || !taskbarHostSharedPtr[1]) {
        if (taskbarHostSharedPtr[1])
            std__Ref_count_base__Decref_Original(taskbarHostSharedPtr[1]);
        return nullptr;
    }

    size_t elementOffset = 0x10;
#if defined(_M_X64)
    auto bytes = reinterpret_cast<BYTE const*>(TaskbarHost_FrameHeight_Original);
    if (bytes[0] == 0x48 && bytes[1] == 0x83 && bytes[2] == 0xEC &&
        bytes[4] == 0x48 && bytes[5] == 0x83 && bytes[6] == 0xC1 &&
        bytes[7] <= 0x7F) {
        elementOffset = bytes[7];
    } else {
        Wh_Log(L"[Template] Unsupported TaskbarHost::FrameHeight");
    }
#elif defined(_M_ARM64)
    auto instructions = reinterpret_cast<DWORD const*>(
        TaskbarHost_FrameHeight_Original);
    if (instructions[0] == 0xD503237F &&
        (instructions[1] & 0xFFC07FFF) == 0xA9807BFD &&
        instructions[2] == 0x910003FD &&
        (instructions[3] & 0xFFF00FE0) == 0xF8400C00) {
        elementOffset = (instructions[3] >> 12) & 0xFF;
    } else {
        Wh_Log(L"[Template] Unsupported TaskbarHost::FrameHeight");
    }
#else
#error Unsupported architecture
#endif

    auto unknown = *reinterpret_cast<IUnknown**>(
        reinterpret_cast<BYTE*>(taskbarHostSharedPtr[0]) + elementOffset);
    if (!unknown) {
        std__Ref_count_base__Decref_Original(taskbarHostSharedPtr[1]);
        return nullptr;
    }

    FrameworkElement taskbarElement = nullptr;
    unknown->QueryInterface(winrt::guid_of<FrameworkElement>(),
                            winrt::put_abi(taskbarElement));
    auto result = taskbarElement ? taskbarElement.XamlRoot() : nullptr;
    std__Ref_count_base__Decref_Original(taskbarHostSharedPtr[1]);
    return result;
}

static bool ApplyOnTaskbarThread() {
    HWND window = FindCurrentProcessTaskbarWnd();
    if (!window) return false;
    g_templateTaskbarWnd = window;
    if (!GetTaskbarXamlRoot(window)) return false;
    bool applied = ApplyModUi();
    g_templateApplied.store(applied);
    return applied;
}

static void ApplyOnTaskbarWindowThread() {
    HWND window = g_templateTaskbarWnd ? g_templateTaskbarWnd
                                       : FindCurrentProcessTaskbarWnd();
    if (!window) return;
    RunFromWindowThread(window, [](void*) { ApplyOnTaskbarThread(); }, nullptr);
}

static void StopRetryThread() {
    // Detach ownership while locked, then wait and close outside the lock. A
    // retry worker can be blocked in SendMessage to the taskbar UI thread; the
    // UI thread must never wait on this lock while another caller waits for
    // that worker.
    AcquireSRWLockExclusive(&g_templateRetryLock);
    HANDLE retryThread = g_templateRetryThread;
    HANDLE retryStopEvent = g_templateRetryStopEvent;
    g_templateRetryThread = nullptr;
    g_templateRetryStopEvent = nullptr;
    if (retryStopEvent)
        SetEvent(retryStopEvent);
    ReleaseSRWLockExclusive(&g_templateRetryLock);

    if (retryThread) {
        DWORD result;
        do {
            result = MsgWaitForMultipleObjects(
                1, &retryThread, FALSE, INFINITE, QS_SENDMESSAGE);
            if (result == WAIT_OBJECT_0 + 1) {
                MSG message;
                PeekMessageW(&message, nullptr, 0, 0, PM_NOREMOVE);
            }
        } while (result == WAIT_OBJECT_0 + 1);
        CloseHandle(retryThread);
    }
    if (retryStopEvent) {
        CloseHandle(retryStopEvent);
    }
}

static void StartRetryThread() {
    StopRetryThread();
    AcquireSRWLockExclusive(&g_templateRetryLock);
    // Another concurrent StartRetryThread may have won after our stop. Keep
    // its single retry loop instead of creating a second one.
    if (g_templateUnloading || g_templateRetryThread ||
        g_templateRetryStopEvent) {
        ReleaseSRWLockExclusive(&g_templateRetryLock);
        return;
    }
    g_templateRetryStopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!g_templateRetryStopEvent) {
        ReleaseSRWLockExclusive(&g_templateRetryLock);
        return;
    }
    HANDLE stopEvent = g_templateRetryStopEvent;
    g_templateRetryThread = CreateThread(
        nullptr, 0,
        [](void* parameter) -> DWORD {
            HANDLE stopEvent = reinterpret_cast<HANDLE>(parameter);
            for (int attempt = 0; attempt < 5 && !g_templateUnloading;
                 ++attempt) {
                if (g_templateApplied)
                    break;
                if (attempt &&
                    WaitForSingleObject(stopEvent, 2000) != WAIT_TIMEOUT)
                    break;
                ApplyOnTaskbarWindowThread();
            }
            return 0;
        },
        stopEvent, 0, nullptr);
    if (!g_templateRetryThread) {
        CloseHandle(g_templateRetryStopEvent);
        g_templateRetryStopEvent = nullptr;
    }
    ReleaseSRWLockExclusive(&g_templateRetryLock);
}

using TrayUI_StartTaskbar_t = void (WINAPI*)(void*);
static TrayUI_StartTaskbar_t TrayUI_StartTaskbar_Original;

static void WINAPI TrayUI_StartTaskbar_Hook(void* self) {
    TrayUI_StartTaskbar_Original(self);
    if (!g_templateUnloading) {
        g_templateApplied = false;
        StartRetryThread();
    }
}

static bool HookTaskbarSymbols() {
    HMODULE module = LoadLibraryExW(L"taskbar.dll", nullptr,
                                    LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!module) return false;
    // PR validation requires the variable name (or an immediately preceding
    // module comment) to identify the symbol target module.
    WindhawkUtils::SYMBOL_HOOK taskbarDllHooks[] = {
        {{LR"(const CTaskBand::`vftable'{for `ITaskListWndSite'})"},
         &CTaskBand_ITaskListWndSite_vftable},
        {{LR"(public: virtual class std::shared_ptr<class TaskbarHost> __cdecl CTaskBand::GetTaskbarHost(void)const )"},
         &CTaskBand_GetTaskbarHost_Original},
        {{LR"(public: int __cdecl TaskbarHost::FrameHeight(void)const )"},
         &TaskbarHost_FrameHeight_Original},
        {{LR"(public: void __cdecl std::_Ref_count_base::_Decref(void))"},
         &std__Ref_count_base__Decref_Original},
        {{LR"(public: virtual void __cdecl TrayUI::StartTaskbar(void))"},
         &TrayUI_StartTaskbar_Original, TrayUI_StartTaskbar_Hook},
    };
    return WindhawkUtils::HookSymbols(
        module, taskbarDllHooks, ARRAYSIZE(taskbarDllHooks));
}

BOOL Wh_ModInit() {
    if (!HookTaskbarSymbols()) {
        Wh_Log(L"[Init] taskbar.dll hooks failed");
        return FALSE;
    }
    return TRUE;
}

void Wh_ModAfterInit() {
    // The already-running-taskbar case needs one immediate attempt. A taskbar
    // that is still starting (or later rebuilding) is handled by the
    // TrayUI::StartTaskbar hook, which owns the bounded retry path.
    ApplyOnTaskbarWindowThread();
}

void Wh_ModSettingsChanged() {
    StopRetryThread();
    HWND window = g_templateTaskbarWnd ? g_templateTaskbarWnd
                                       : FindCurrentProcessTaskbarWnd();
    if (window) {
        RunFromWindowThread(window, [](void* parameter) {
            HWND window = static_cast<HWND>(parameter);
            if (!GetTaskbarXamlRoot(window)) return;
            RemoveModUi();
            g_templateApplied = false;
            ApplyOnTaskbarThread();
        }, window);
    }
}

void Wh_ModUninit() {
    g_templateUnloading = true;
    StopRetryThread();
    HWND window = g_templateTaskbarWnd ? g_templateTaskbarWnd
                                       : FindCurrentProcessTaskbarWnd();
    if (window) {
        RunFromWindowThread(window, [](void*) {
            RemoveModUi();
            // After RemoveModUi revokes every event/delegate, reset each
            // no_destroy optional<container> here to free its heap storage:
            // g_eventStates.reset();
            g_templateApplied = false;
        }, nullptr);
    } else {
        // Intentionally retain all no_destroy XAML/WinRT holders. There is no
        // known UI thread on which releasing them would be safe.
        Wh_Log(L"[Uninit] No taskbar UI thread; retaining XAML state");
    }
}
