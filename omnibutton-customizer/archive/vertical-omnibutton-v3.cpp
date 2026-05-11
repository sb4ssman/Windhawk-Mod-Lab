// ==WindhawkMod==
// @id              tray-icon-relayout
// @name            Tray Icon Re-Layout (Skeleton)
// @description     Framework for intercepting and overriding system tray icon layout
// @version         0.1
// @author          Thomas
// @include         explorer.exe
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
This mod is the skeleton used for discovering and hooking the internal tray
icon layout logic inside explorer.exe (Taskbar / Shell components).

It does NOT modify anything yet. It only logs when candidate functions
are hit, so we can determine which internal layout routine to intercept.

Next steps:
1. Use UWPSpy to locate the XAML class names of the tray icon container(s)
2. Enable symbol logging here, rebuild, and observe Windhawk log output
3. Identify which internal explorer.exe function controls tray icon layout
4. Replace that function's computed RECT positions with custom ones
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
{
    "enableLogging": {
        "type": "bool",
        "default": true,
        "description": "Log candidate layout calls"
    }
}
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <shellapi.h>

struct {
    bool enableLogging;
} g_settings;

// Example placeholder for a layout function we will hook.
// We DON'T know the signature yet — part of the discovery process.

using CandidateLayoutFunc_t = void(*)(void* a1, void* a2);
CandidateLayoutFunc_t g_CandidateLayoutFunc_Original = nullptr;

// This is only a logging shim. We do NOT override anything yet.
void __stdcall CandidateLayoutFunc_Hook(void* a1, void* a2) {
    if (g_settings.enableLogging) {
        Wh_Log(L"[LayoutProbe] CandidateLayoutFunc_Hook hit: a1=%p a2=%p", a1, a2);
    }

    // Call original (we do not break anything yet)
    g_CandidateLayoutFunc_Original(a1, a2);
}

void LoadSettings() {
    g_settings.enableLogging = Wh_GetIntSetting(L"enableLogging") != 0;
}

BOOL Wh_ModInit() {
    Wh_Log(L"Init");

    LoadSettings();

    //
    // STEP 1: Locate likely explorer.exe / taskbar modules
    //
    HMODULE hTaskbar = GetModuleHandleW(L"taskbar.dll");
    HMODULE hTaskbarView = GetModuleHandleW(L"Taskbar.View.dll");
    HMODULE hShell32 = GetModuleHandleW(L"shell32.dll");
    HMODULE hExplorer = GetModuleHandleW(NULL); // explorer.exe itself

    Wh_Log(L"Loaded modules:");
    Wh_Log(L" - explorer.exe       : %p", hExplorer);
    Wh_Log(L" - taskbar.dll        : %p", hTaskbar);
    Wh_Log(L" - Taskbar.View.dll   : %p", hTaskbarView);
    Wh_Log(L" - shell32.dll        : %p", hShell32);

    //
    // STEP 2: Identify a function to hook.
    //
    // We cannot know the real signature yet until we observe logs.
    // So we temporarily hook an exported function known to fire during layout,
    // such as Shell_NotifyIconGetRect (documented API).
    //
    FARPROC pCandidate = GetProcAddress(hShell32, "Shell_NotifyIconGetRect");
    if (pCandidate) {
        Wh_Log(L"Hooking Shell_NotifyIconGetRect at %p", pCandidate);

        Wh_SetFunctionHook(
            (void*)pCandidate,
            (void*)CandidateLayoutFunc_Hook,
            (void**)&g_CandidateLayoutFunc_Original
        );
    } else {
        Wh_Log(L"Shell_NotifyIconGetRect not found.");
    }

    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L"Uninit");
}

void Wh_ModSettingsChanged() {
    Wh_Log(L"SettingsChanged");
    LoadSettings();
}
