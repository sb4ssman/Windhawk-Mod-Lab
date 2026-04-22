// ==WindhawkMod==
// @id              vertical-system-tray-icons
// @name            Vertical OmniButton
// @description     Stacks Windows 11 wifi/volume/battery OmniButton vertically
// @version         1.1.0
// @author          sb4ssman
// @github          https://github.com/sb4ssman/Windhawk-Vertical-wifi-sound-battery-button
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Vertical OmniButton

Rearranges the Windows 11 system tray OmniButton (wifi, volume, battery) from
horizontal layout to clean vertical stacking.

## How it works

Hooks the `SystemTray.IconView` constructor in `Taskbar.View.dll`. When an
OmniButton icon element loads, the mod walks up the XAML tree to the containing
`StackPanel` and sets its `Orientation` to `Vertical`, then sizes the wrapping
`ContentPresenter` elements according to your settings.

This is the same approach used by the Windows 11 Taskbar Styler — it modifies
the layout rather than applying a visual transform, so hit-testing works
correctly.

## Usage

After enabling the mod, **restart explorer.exe** (Task Manager → Restart) for
the hook to catch the freshly created elements.

Enable **debug logging** and watch Windhawk's log output to confirm the hook
is firing. If nothing appears, the symbol may not match your Windows build.

## Settings

- **Enable vertical arrangement** — master toggle
- **Icon size** — width/height of each icon slot in pixels (16–48)
- **Icon spacing** — extra height added below each icon for gap (0–24)
- **Debug logging** — log parent class names while walking the XAML tree

*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- enableVertical: true
  $name: Enable vertical arrangement
  $description: Enable/disable vertical stacking of system tray icons
- iconSize: 28
  $name: Icon size
  $description: Width and height of each icon slot in pixels (16-48)
- iconSpacing: 6
  $name: Icon spacing
  $description: Extra height added below each icon as a gap (0-24)
- debugLogging: false
  $name: Enable debug logging
  $description: Log XAML parent names while finding OmniButton — useful for troubleshooting
*/
// ==/WindhawkModSettings==

#include <unknwn.h>
#include <winrt/base.h>
#include <windhawk_api.h>
#include <windhawk_utils.h>

#undef GetCurrentTime

#include <windows.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

struct {
    bool enableVertical;
    int iconSize;
    int iconSpacing;
    bool debugLogging;
} g_settings;

bool g_unloading = false;

void IconView_IconView_Hook(void* pThis);
decltype(&IconView_IconView_Hook) IconView_IconView_Original;

// ── Core layout fix ────────────────────────────────────────────────────────

// Called from the Loaded handler once we know this icon is inside OmniButton.
// Tree (from UWPSpy):
//   OmniButton > Grid > ContentPresenter > ItemsPresenter >
//   StackPanel (ItemsHost) > ContentPresenter (item) > IconView
//
// Strategy (matches the working style.yaml approach):
//   1. Parent of iconView  → item ContentPresenter — set Width/Height
//   2. Grandparent         → StackPanel            — set Orientation=Vertical
//   3. Walk up to OmniButton and set MinWidth so the button doesn't shrink
static void ApplyVerticalLayout(FrameworkElement const& iconView) {
    try {
        // --- item ContentPresenter (direct parent) ---
        auto itemCPObj = VisualTreeHelper::GetParent(iconView);
        if (!itemCPObj) { Wh_Log(L"[Layout] no parent"); return; }
        auto itemCP = itemCPObj.try_as<FrameworkElement>();

        // --- StackPanel (grandparent) ---
        DependencyObject spObj = itemCPObj;
        StackPanel stackPanel = nullptr;
        // Walk up a few levels to find the StackPanel in case the tree
        // has an extra layer between ContentPresenter and StackPanel.
        for (int i = 0; i < 4 && !stackPanel; i++) {
            spObj = VisualTreeHelper::GetParent(spObj);
            if (!spObj) break;
            stackPanel = spObj.try_as<StackPanel>();
        }

        if (stackPanel) {
            stackPanel.Orientation(g_unloading ? Orientation::Horizontal
                                               : Orientation::Vertical);
            Wh_Log(L"[Layout] StackPanel orientation -> %s",
                   g_unloading ? L"Horizontal" : L"Vertical");
        } else {
            Wh_Log(L"[Layout] StackPanel not found within 4 levels of iconView");
        }

        // Size the item ContentPresenter to match settings
        if (itemCP) {
            if (!g_unloading) {
                itemCP.Width(g_settings.iconSize);
                itemCP.Height(g_settings.iconSize + g_settings.iconSpacing);
            } else {
                itemCP.ClearValue(FrameworkElement::WidthProperty());
                itemCP.ClearValue(FrameworkElement::HeightProperty());
            }
        }

        // --- OmniButton (ancestor) ---
        DependencyObject cur = iconView;
        for (int depth = 0; depth < 14; depth++) {
            auto p = VisualTreeHelper::GetParent(cur);
            if (!p) break;
            auto pfe = p.try_as<FrameworkElement>();
            if (!pfe) break;
            auto cls = winrt::get_class_name(pfe);
            auto nm  = pfe.Name();
            std::wstring clsStr(cls);
            if (clsStr.find(L"OmniButton") != std::wstring::npos ||
                nm == L"ControlCenterButton") {
                if (!g_unloading) {
                    pfe.MinWidth(g_settings.iconSize + 16.0);
                } else {
                    pfe.ClearValue(FrameworkElement::MinWidthProperty());
                }
                Wh_Log(L"[Layout] OmniButton found (%s), MinWidth updated", cls.c_str());
                break;
            }
            cur = p;
        }

    } catch (...) {
        Wh_Log(L"[Layout] Exception");
    }
}

// ── Symbol hook ────────────────────────────────────────────────────────────

void IconView_IconView_Hook(void* pThis) {
    IconView_IconView_Original(pThis);

    // Always log — this is the first signal that the hook is working at all.
    Wh_Log(L"[Hook] IconView::IconView (pThis=%p)", pThis);

    if (g_unloading || !g_settings.enableVertical) return;

    try {
        // pThis is the C++/WinRT implementation object. Its vtable begins with
        // the IUnknown interface, so we can cast and QI through to FrameworkElement.
        IUnknown* rawUnknown = reinterpret_cast<IUnknown*>(pThis);
        if (!rawUnknown) return;

        rawUnknown->AddRef();
        winrt::com_ptr<IUnknown> unk;
        unk.attach(rawUnknown);

        auto insp = unk.try_as<winrt::Windows::Foundation::IInspectable>();
        if (!insp) { Wh_Log(L"[Hook] QI to IInspectable failed"); return; }

        FrameworkElement iconView = insp.try_as<FrameworkElement>();
        if (!iconView) { Wh_Log(L"[Hook] QI to FrameworkElement failed"); return; }

        Wh_Log(L"[Hook] Got element: %s (name=%s)",
               winrt::get_class_name(iconView).c_str(), iconView.Name().c_str());

        // Wait until the element is in the live tree before traversing parents.
        iconView.Loaded([iconView](auto&&, auto&&) {
            Wh_Log(L"[Loaded] Fired");
            try {
                // Walk up to check whether this IconView lives inside an OmniButton.
                bool isOmni = false;
                DependencyObject cur = iconView;
                for (int depth = 0; depth < 12; depth++) {
                    auto p = VisualTreeHelper::GetParent(cur);
                    if (!p) break;
                    auto pfe = p.try_as<FrameworkElement>();
                    if (!pfe) break;
                    auto cls = winrt::get_class_name(pfe);
                    auto nm  = pfe.Name();
                    if (g_settings.debugLogging) {
                        Wh_Log(L"[Loaded] Parent[%d] class=%s name=%s",
                               depth, cls.c_str(), nm.c_str());
                    }
                    std::wstring clsStr(cls);
                    if (clsStr.find(L"OmniButton") != std::wstring::npos ||
                        nm == L"ControlCenterButton") {
                        isOmni = true;
                        break;
                    }
                    cur = p;
                }

                if (!isOmni) { Wh_Log(L"[Loaded] Not OmniButton — skip"); return; }

                Wh_Log(L"[Loaded] OmniButton confirmed — applying layout");
                ApplyVerticalLayout(iconView);

            } catch (...) {
                Wh_Log(L"[Loaded] Exception");
            }
        });

    } catch (...) {
        Wh_Log(L"[Hook] Exception");
    }
}

// ── Windhawk lifecycle ─────────────────────────────────────────────────────

void LoadSettings() {
    g_settings.enableVertical = Wh_GetIntSetting(L"enableVertical") != 0;
    g_settings.iconSize       = Wh_GetIntSetting(L"iconSize");
    g_settings.iconSpacing    = Wh_GetIntSetting(L"iconSpacing");
    g_settings.debugLogging   = Wh_GetIntSetting(L"debugLogging") != 0;

    if (g_settings.iconSize < 16) g_settings.iconSize = 16;
    if (g_settings.iconSize > 48) g_settings.iconSize = 48;
    if (g_settings.iconSpacing < 0)  g_settings.iconSpacing = 0;
    if (g_settings.iconSpacing > 24) g_settings.iconSpacing = 24;
}

static bool HookSymbolsForModule(HMODULE hModule) {
    WindhawkUtils::SYMBOL_HOOK iconViewHook = {
        // Multiple variants — Windhawk will match whichever appears in the PDB.
        // The winrt:: prefix is part of the C++ namespace in the compiled binary;
        // some Windows builds omit it in the undecorated symbol string.
        {
            LR"(public: __cdecl winrt::SystemTray::implementation::IconView::IconView(void))",
            LR"(public: __cdecl SystemTray::implementation::IconView::IconView(void))",
        },
        &IconView_IconView_Original,
        IconView_IconView_Hook,
    };

    bool ok = WindhawkUtils::HookSymbols(hModule, &iconViewHook, 1);
    Wh_Log(L"[Hook] HookSymbols: %s  original=%p",
           ok ? L"OK" : L"FAILED", (void*)IconView_IconView_Original);
    return ok;
}

static bool TryHookTaskbarView() {
    HMODULE hModule = GetModuleHandle(L"Taskbar.View.dll");
    if (!hModule) return false;
    Wh_Log(L"[Hook] Taskbar.View.dll at %p", (void*)hModule);
    return HookSymbolsForModule(hModule);
}

// ── LoadLibraryExW hook — catches Taskbar.View.dll loading after explorer start

using LoadLibraryExW_t = decltype(&LoadLibraryExW);
LoadLibraryExW_t LoadLibraryExW_Original;

HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
    HMODULE hModule = LoadLibraryExW_Original(lpLibFileName, hFile, dwFlags);

    if (hModule && !IconView_IconView_Original && lpLibFileName) {
        // Compare just the filename portion, case-insensitively.
        const wchar_t* slash = wcsrchr(lpLibFileName, L'\\');
        const wchar_t* name  = slash ? (slash + 1) : lpLibFileName;
        if (_wcsicmp(name, L"Taskbar.View.dll") == 0) {
            Wh_Log(L"[LoadLib] Taskbar.View.dll just loaded — installing hook");
            HookSymbolsForModule(hModule);
        }
    }

    return hModule;
}

BOOL Wh_ModInit() {
    Wh_Log(L"[Init] Vertical OmniButton v1.1.0");
    LoadSettings();

    // Watch for Taskbar.View.dll in case it loads after our init callbacks fire.
    // On a fresh explorer start the DLL often isn't present yet at Wh_ModInit time.
    Wh_SetFunctionHook((void*)LoadLibraryExW,
                        (void*)LoadLibraryExW_Hook,
                        (void**)&LoadLibraryExW_Original);

    // Also try immediately — works when the mod is installed on a running explorer.
    if (!TryHookTaskbarView()) {
        Wh_Log(L"[Init] Taskbar.View.dll not loaded yet — will hook via LoadLibraryExW");
    }

    return TRUE;
}

void Wh_ModAfterInit() {
    // One more attempt — sometimes the DLL appears just after Wh_ModInit.
    if (!IconView_IconView_Original) {
        Wh_Log(L"[AfterInit] Retrying...");
        TryHookTaskbarView();
    }
    Wh_Log(L"[AfterInit] Hook ptr=%p", (void*)IconView_IconView_Original);
}

void Wh_ModUninit() {
    g_unloading = true;
    Wh_Log(L"[Uninit]");
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Wh_Log(L"[Settings] Updated");
}
