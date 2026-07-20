// Copy-source template v1.1: Windows 11 taskbar XAML lifecycle.
//
// Adapter contract: implement ApplyModUi() and RemoveModUi() below. Both are
// always called synchronously on the taskbar window thread. ApplyModUi returns
// true only when the UI exists or the mod intentionally has nothing to show.

#undef GetCurrentTime

#include <winrt/Windows.UI.Xaml.h>

#include <atomic>
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

// PROCESS-SHUTDOWN CONTRACT (crash class found 2026-07-19 in Privacy
// Indicator Anchor): Explorer shutdown doesn't guarantee Wh_ModUninit. Never
// let CRT global destruction release namespace-scope state after the XAML
// framework has torn down or from the shutdown thread. Prefer eliminating
// non-trivial namespace-scope objects. When they are necessary, mark them
// no_destroy--especially strong XAML/WinRT references and containers that own
// WinRT references, event revokers, weak_ref objects, Storyboards, or delegates:
//
// [[clang::no_destroy]] static FrameworkElement g_ownedRoot = nullptr;
// [[clang::no_destroy]] static std::vector<OwnedEventState> g_eventStates;
// [[clang::no_destroy]] static ModSettings g_settings;
//
// This is intentional process-lifetime retention, not a replacement for
// cleanup. Controlled settings changes and mod unload must still revoke and
// release everything synchronously on the taskbar UI thread. If no taskbar
// window/UI thread can be reached during Wh_ModUninit, retain the no_destroy
// state; do not add an off-thread fallback that clears XAML objects.
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

static bool RunFromWindowThread(HWND window, WindowThreadProc proc,
                                void* parameter) {
    static UINT message = RegisterWindowMessageW(
        L"Windhawk_RunFromWindowThread_" WH_MOD_ID);
    struct Dispatch { WindowThreadProc proc; void* parameter; };
    DWORD threadId = GetWindowThreadProcessId(window, nullptr);
    if (!threadId) return false;
    if (threadId == GetCurrentThreadId()) {
        proc(parameter);
        return true;
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
                    dispatch->proc(dispatch->parameter);
                }
            }
            return CallNextHookEx(nullptr, code, wParam, lParam);
        },
        nullptr, threadId);
    if (!hook) return false;

    Dispatch dispatch{proc, parameter};
    SendMessageW(window, message, 0, reinterpret_cast<LPARAM>(&dispatch));
    UnhookWindowsHookEx(hook);
    return true;
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
    if (g_templateRetryStopEvent)
        SetEvent(g_templateRetryStopEvent);
    if (g_templateRetryThread) {
        DWORD result;
        do {
            result = MsgWaitForMultipleObjects(
                1, &g_templateRetryThread, FALSE, INFINITE, QS_SENDMESSAGE);
            if (result == WAIT_OBJECT_0 + 1) {
                MSG message;
                PeekMessageW(&message, nullptr, 0, 0, PM_NOREMOVE);
            }
        } while (result == WAIT_OBJECT_0 + 1);
        CloseHandle(g_templateRetryThread);
        g_templateRetryThread = nullptr;
    }
    if (g_templateRetryStopEvent) {
        CloseHandle(g_templateRetryStopEvent);
        g_templateRetryStopEvent = nullptr;
    }
}

static void StartRetryThread() {
    StopRetryThread();
    if (g_templateUnloading) return;
    g_templateRetryStopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!g_templateRetryStopEvent) return;
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
    StartRetryThread();
}

void Wh_ModSettingsChanged() {
    StopRetryThread();
    HWND window = g_templateTaskbarWnd ? g_templateTaskbarWnd
                                       : FindCurrentProcessTaskbarWnd();
    if (window) {
        RunFromWindowThread(window, [](void*) {
            RemoveModUi();
            g_templateApplied = false;
            ApplyOnTaskbarThread();
        }, nullptr);
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
            g_templateApplied = false;
        }, nullptr);
    } else {
        // Intentionally retain all no_destroy XAML/WinRT holders. There is no
        // known UI thread on which releasing them would be safe.
        Wh_Log(L"[Uninit] No taskbar UI thread; retaining XAML state");
    }
}
