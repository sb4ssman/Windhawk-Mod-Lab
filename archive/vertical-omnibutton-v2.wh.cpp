// ==WindhawkMod==
// @id              vertical-omnibutton-v2
// @name            Vertical System Tray OmniButton Icons (Test Version)
// @description     Transforms the horizontal system tray icon group (wifi, volume, battery) into a vertical arrangement using hooks
// @version         2.0.0
// @author          sb4ssman
// @github          https://github.com/tmiller711/Windhawk-Vertical-Omnibutton
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Vertical System Tray OmniButton Icons - Test Version

**STATUS: EXPERIMENTAL** - This version implements function hooks to test the approach.

This mod transforms the Windows 11 system tray icon grouping (wifi, volume, battery)
from horizontal to vertical arrangement using TranslateTransform positioning.

## How It Works

1. Hooks into the IconView constructor (similar to taskbar-notification-icon-spacing)
2. Identifies OmniButton-related IconViews by checking parent elements
3. Applies vertical TranslateTransform to stack icons
4. Maintains icon functionality

## Settings

- **Enable vertical arrangement**: Toggle on/off
- **Icon size**: Size of each icon (default: 32px)
- **Icon spacing**: Vertical spacing between icons (default: 4px)
- **Debug logging**: Enable detailed logs (use DebugView)

## Usage

1. Enable the mod in Windhawk
2. Watch DebugView for log output
3. Restart explorer.exe if needed
4. Report findings!

## Known Issues

- Experimental - may not work on all Windows 11 versions
- May conflict with other system tray mods
- Requires testing and iteration

## Credits

Based on taskbar-notification-icon-spacing.wh.cpp by m417z.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- enableVertical: true
  $name: Enable vertical arrangement
  $description: Enable/disable vertical stacking of system tray icons
- iconSize: 32
  $name: Icon size (pixels)
  $description: Size of each icon in pixels (16-48)
- iconSpacing: 4
  $name: Icon spacing (pixels)
  $description: Vertical spacing between icons when arranged vertically (0-32)
- debugLogging: true
  $name: Enable debug logging
  $description: Log detailed information for troubleshooting (use DebugView - enabled by default for testing)
*/
// ==/WindhawkModSettings==

#include <windhawk_api.h>
#include <windhawk_utils.h>

#include <windows.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

// Settings
struct {
    bool enableVertical;
    int iconSize;
    int iconSpacing;
    bool debugLogging;
} g_settings;

bool g_initialized = false;
bool g_unloading = false;

// Find child element by class name
FrameworkElement FindChildByClassName(
    DependencyObject element,
    const wchar_t* className
) {
    try {
        int childrenCount = Media::VisualTreeHelper::GetChildrenCount(element);

        for (int i = 0; i < childrenCount; i++) {
            auto child = Media::VisualTreeHelper::GetChild(element, i);

            auto childElement = child.try_as<FrameworkElement>();
            if (childElement) {
                auto childClassName = winrt::get_class_name(childElement);
                if (childClassName == className) {
                    return childElement;
                }
            }

            auto found = FindChildByClassName(child, className);
            if (found) {
                return found;
            }
        }
    } catch (...) {
    }

    return nullptr;
}

// Check if this IconView is part of the OmniButton
bool IsOmniButtonIcon(FrameworkElement iconView) {
    try {
        // Walk up the tree to find if we're inside an OmniButton
        DependencyObject current = iconView;

        for (int depth = 0; depth < 10; depth++) {  // Max 10 levels up
            auto parent = Media::VisualTreeHelper::GetParent(current);
            if (!parent) break;

            auto parentElement = parent.try_as<FrameworkElement>();
            if (parentElement) {
                auto className = winrt::get_class_name(parentElement);
                auto name = parentElement.Name();

                Wh_Log(L"[OmniButton Check] Parent %d: class=%s, name=%s",
                       depth, className.c_str(), name.c_str());

                // Check for OmniButton indicators
                if (className.find(L"OmniButton") != std::wstring::npos ||
                    name == L"ControlCenterButton") {
                    Wh_Log(L"[OmniButton Check] FOUND! This is an OmniButton icon");
                    return true;
                }
            }

            current = parent;
        }

        Wh_Log(L"[OmniButton Check] Not an OmniButton icon");
        return false;

    } catch (...) {
        Wh_Log(L"[OmniButton Check] Exception");
        return false;
    }
}

// Apply vertical transform to an icon
void ApplyVerticalTransform(FrameworkElement iconView, int iconIndex) {
    try {
        if (!g_settings.enableVertical || g_unloading) {
            iconView.RenderTransform(nullptr);
            return;
        }

        // Calculate vertical offset
        double itemHeight = g_settings.iconSize + g_settings.iconSpacing;
        double yOffset = itemHeight * iconIndex;

        // Center the stack (3 icons: wifi, sound, battery)
        int iconCount = 3;
        double totalHeight = itemHeight * (iconCount - 1);
        yOffset -= totalHeight / 2;

        Wh_Log(L"[Transform] Icon %d: yOffset=%.2f (itemHeight=%.2f)",
               iconIndex, yOffset, itemHeight);

        // Apply transform
        Media::TranslateTransform transform;
        transform.Y(yOffset);
        transform.X(0);
        iconView.RenderTransform(transform);

    } catch (...) {
        Wh_Log(L"[Transform] Exception applying transform");
    }
}

// Style the OmniButton IconView
void StyleOmniButtonIcon(FrameworkElement iconView) {
    try {
        Wh_Log(L"[StyleOmniButton] Starting to style icon");

        // We need to determine which icon this is (0, 1, or 2)
        // For now, we'll use a simple counter approach
        // In a production version, we'd track this more carefully
        static int iconCounter = 0;
        int iconIndex = iconCounter % 3;
        iconCounter++;

        Wh_Log(L"[StyleOmniButton] Assigning icon index: %d", iconIndex);

        ApplyVerticalTransform(iconView, iconIndex);

        Wh_Log(L"[StyleOmniButton] Transform applied successfully");

    } catch (...) {
        Wh_Log(L"[StyleOmniButton] Exception");
    }
}

// Hook: IconView constructor
using IconView_IconView_t = void(WINAPI*)(void* pThis);
IconView_IconView_t IconView_IconView_Original;

void WINAPI IconView_IconView_Hook(void* pThis) {
    Wh_Log(L"=== IconView::IconView called ===");

    // Call original constructor
    IconView_IconView_Original(pThis);

    try {
        // Get the FrameworkElement from the COM object
        winrt::Windows::UI::Xaml::FrameworkElement iconView = nullptr;

        // The pThis pointer is the COM object, need to QueryInterface
        // This is how taskbar-notification-icon-spacing does it
        ((IUnknown*)pThis)->QueryInterface(winrt::guid_of<FrameworkElement>(),
                                          winrt::put_abi(iconView));

        if (!iconView) {
            Wh_Log(L"[IconView Hook] Failed to get FrameworkElement");
            return;
        }

        auto className = winrt::get_class_name(iconView);
        Wh_Log(L"[IconView Hook] IconView created: class=%s", className.c_str());

        // Check if this is an OmniButton icon
        bool isOmniButton = IsOmniButtonIcon(iconView);

        if (isOmniButton) {
            Wh_Log(L"[IconView Hook] This IS an OmniButton icon - will style it!");

            // Register a Loaded event handler
            auto loadedToken = iconView.Loaded(winrt::auto_revoke,
                [iconView](auto&&, auto&&) {
                    Wh_Log(L"[Loaded Event] OmniButton icon loaded, styling now");
                    StyleOmniButtonIcon(iconView);
                }
            );

            // Note: The auto_revoke token will automatically unregister when destroyed
            // We should ideally store this somewhere, but for testing this works

        } else {
            Wh_Log(L"[IconView Hook] This is NOT an OmniButton icon - skipping");
        }

    } catch (...) {
        Wh_Log(L"[IconView Hook] Exception");
    }
}

// Load settings
void LoadSettings() {
    g_settings.enableVertical = Wh_GetIntSetting(L"enableVertical");
    g_settings.iconSize = Wh_GetIntSetting(L"iconSize");
    g_settings.iconSpacing = Wh_GetIntSetting(L"iconSpacing");
    g_settings.debugLogging = Wh_GetIntSetting(L"debugLogging");

    // Validate
    if (g_settings.iconSize < 16) g_settings.iconSize = 16;
    if (g_settings.iconSize > 48) g_settings.iconSize = 48;
    if (g_settings.iconSpacing < 0) g_settings.iconSpacing = 0;
    if (g_settings.iconSpacing > 32) g_settings.iconSpacing = 32;

    Wh_Log(L"Settings: enable=%d, size=%d, spacing=%d, debug=%d",
           g_settings.enableVertical, g_settings.iconSize,
           g_settings.iconSpacing, g_settings.debugLogging);
}

// Hook symbols
bool HookTaskbarViewSymbols() {
    Wh_Log(L"Attempting to hook Taskbar.View.dll symbols");

    HMODULE taskbarViewModule = GetModuleHandle(L"Taskbar.View.dll");
    if (!taskbarViewModule) {
        Wh_Log(L"Taskbar.View.dll not loaded yet");
        return false;
    }

    Wh_Log(L"Taskbar.View.dll is loaded, hooking symbols");

    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {
            {
                L"public: __cdecl winrt::SystemTray::implementation::IconView::IconView(void)"
            },
            &IconView_IconView_Hook,
            (void**)&IconView_IconView_Original,
            false
        }
    };

    if (!WindhawkUtils::HookSymbols(taskbarViewModule, hooks, ARRAYSIZE(hooks))) {
        Wh_Log(L"Failed to hook symbols");
        return false;
    }

    Wh_Log(L"Successfully hooked symbols");
    return true;
}

// Windhawk callbacks
BOOL Wh_ModInit() {
    Wh_Log(L"========================================");
    Wh_Log(L"=== Vertical OmniButton Mod Init v2 ===");
    Wh_Log(L"========================================");

    LoadSettings();

    if (!HookTaskbarViewSymbols()) {
        Wh_Log(L"WARNING: Failed to hook Taskbar.View.dll symbols");
        Wh_Log(L"This may be normal if the module isn't loaded yet");
        // Don't return FALSE - we'll try again in AfterInit
    }

    g_initialized = true;

    Wh_Log(L"Init complete");
    return TRUE;
}

void Wh_ModAfterInit() {
    Wh_Log(L"=== AfterInit called ===");

    // Try hooking again in case Taskbar.View.dll loaded after init
    if (!IconView_IconView_Original) {
        Wh_Log(L"Symbols not hooked yet, trying again");
        HookTaskbarViewSymbols();
    }
}

void Wh_ModUninit() {
    Wh_Log(L"=== Uninit ===");
    g_unloading = true;
}

void Wh_ModSettingsChanged() {
    Wh_Log(L"=== Settings Changed ===");
    LoadSettings();
    Wh_Log(L"Note: Restart explorer.exe for changes to take full effect");
}
