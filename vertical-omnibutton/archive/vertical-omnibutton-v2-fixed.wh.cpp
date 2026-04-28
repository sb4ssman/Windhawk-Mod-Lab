// ==WindhawkMod==
// @id              vertical-omnibutton-v2
// @name            Vertical System Tray OmniButton Icons (Test Version)
// @description     Transforms the horizontal system tray icon group (wifi, volume, battery) into a vertical arrangement using hooks
// @version         2.1.0
// @author          t.miller85@gmail.com
// @github          https://github.com/sb4ssman/Windhawk-Vertical-Omnibutton
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

1. Hooks into StackViewModel::UpdateIconIndexes (called when system tray layout updates)
2. Traverses XAML tree to find the OmniButton element
3. Locates IconView children inside the OmniButton
4. Applies vertical TranslateTransform to stack icons
5. Maintains icon functionality

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

#include <unknwn.h>               // IUnknown
#include <winrt/base.h>          // winrt::com_ptr
#include <windhawk_api.h>
#include <windhawk_utils.h>

#undef GetCurrentTime  // Fix for WinRT conflict

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
                // Convert hstring to wstring for comparison
                std::wstring classNameStr(className);
                if (classNameStr.find(L"OmniButton") != std::wstring::npos ||
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

// --- Replacement: ApplyVerticalTransform (matches your signature) ---
void ApplyVerticalTransform(FrameworkElement iconView, int /*iconIndex_unused*/)
{
    try {
        if (!g_settings.enableVertical || g_unloading) {
            // Properly clear RenderTransform
            iconView.ClearValue(winrt::Windows::UI::Xaml::FrameworkElement::RenderTransformProperty());
            return;
        }

        // Determine index at runtime (ignore passed iconIndex which was unreliable)
        int iconIndex = GetIndexInParent(iconView);
        if (iconIndex < 0) iconIndex = 0;

        // Compute sizing/offset
        double itemHeight = static_cast<double>(g_settings.iconSize) + static_cast<double>(g_settings.iconSpacing);

        // Attempt to get sibling count
        int siblingCount = 1;
        auto parent = winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetParent(iconView);
        if (parent) {
            auto parentFE = parent.try_as<FrameworkElement>();
            if (parentFE) {
                siblingCount = winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetChildrenCount(parentFE);
                if (siblingCount <= 0) siblingCount = 1;
            }
        }

        double totalHeight = itemHeight * static_cast<double>(siblingCount - 1);
        double yOffset = itemHeight * static_cast<double>(iconIndex) - (totalHeight / 2.0);

        Wh_Log(L"[Transform] index=%d siblings=%d yOffset=%.2f itemH=%.2f", iconIndex, siblingCount, yOffset, itemHeight);

        // Create and apply a real WinRT TranslateTransform
        TranslateTransform transform;
        transform.Y(yOffset);
        transform.X(0);

        // Stabilize layout: set explicit icon size
        iconView.Width(static_cast<double>(g_settings.iconSize));
        iconView.Height(static_cast<double>(g_settings.iconSize));

        // Apply transform (Loaded handler will be on UI thread)
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

// Find OmniButton in the XAML tree starting from a given element
FrameworkElement FindOmniButtonFromRoot(FrameworkElement root) {
    try {
        if (!root) return nullptr;

        // Check if this element is the OmniButton
        auto className = winrt::get_class_name(root);
        std::wstring classNameStr(className);

        if (classNameStr.find(L"OmniButton") != std::wstring::npos ||
            classNameStr.find(L"ControlCenterButton") != std::wstring::npos) {
            Wh_Log(L"[FindOmniButton] Found: %s", className.c_str());
            return root;
        }

        // Check children recursively
        int childCount = VisualTreeHelper::GetChildrenCount(root);
        for (int i = 0; i < childCount; i++) {
            auto child = VisualTreeHelper::GetChild(root, i).try_as<FrameworkElement>();
            if (child) {
                auto found = FindOmniButtonFromRoot(child);
                if (found) return found;
            }
        }

    } catch (...) {
        // Silently ignore
    }

    return nullptr;
}

// Apply vertical styling to all OmniButton icons
void ApplyVerticalStylingToOmniButton(FrameworkElement stackViewModel) {
    Wh_Log(L"[ApplyVertical] Starting to search for OmniButton from StackViewModel");

    try {
        if (!stackViewModel) {
            Wh_Log(L"[ApplyVertical] StackViewModel is null");
            return;
        }

        // Try to find OmniButton by traversing up and down the tree
        // First try going up to find a common parent
        auto current = stackViewModel;
        FrameworkElement systemTrayRoot = nullptr;

        for (int i = 0; i < 10; i++) {
            auto parent = VisualTreeHelper::GetParent(current);
            if (!parent) break;

            auto parentElement = parent.try_as<FrameworkElement>();
            if (parentElement) {
                auto className = winrt::get_class_name(parentElement);
                std::wstring classNameStr(className);

                Wh_Log(L"[ApplyVertical] Parent %d: %s", i, className.c_str());

                if (classNameStr.find(L"SystemTray") != std::wstring::npos &&
                    classNameStr.find(L"implementation") != std::wstring::npos) {
                    systemTrayRoot = parentElement;
                    Wh_Log(L"[ApplyVertical] Found SystemTray root: %s", className.c_str());
                    break;
                }

                current = parentElement;
            } else {
                break;
            }
        }

        // Now search down from the root for OmniButton
        if (systemTrayRoot) {
            auto omniButton = FindOmniButtonFromRoot(systemTrayRoot);

            if (omniButton) {
                Wh_Log(L"[ApplyVertical] Found OmniButton! Styling children...");

                // Find the StackPanel inside OmniButton
                int childCount = VisualTreeHelper::GetChildrenCount(omniButton);
                Wh_Log(L"[ApplyVertical] OmniButton has %d children", childCount);

                for (int i = 0; i < childCount; i++) {
                    auto child = VisualTreeHelper::GetChild(omniButton, i).try_as<FrameworkElement>();
                    if (child) {
                        auto childClassName = winrt::get_class_name(child);
                        std::wstring childClassNameStr(childClassName);

                        Wh_Log(L"[ApplyVertical] Child %d: %s", i, childClassName.c_str());

                        // Look for IconView children
                        if (childClassNameStr.find(L"IconView") != std::wstring::npos) {
                            Wh_Log(L"[ApplyVertical] Styling IconView at index %d", i);
                            ApplyVerticalTransform(child, i);
                        } else if (childClassNameStr.find(L"StackPanel") != std::wstring::npos ||
                                   childClassNameStr.find(L"Grid") != std::wstring::npos) {
                            // Look inside containers
                            int containerChildCount = VisualTreeHelper::GetChildrenCount(child);
                            Wh_Log(L"[ApplyVertical] Container has %d children", containerChildCount);

                            for (int j = 0; j < containerChildCount; j++) {
                                auto iconView = VisualTreeHelper::GetChild(child, j).try_as<FrameworkElement>();
                                if (iconView) {
                                    auto iconClassName = winrt::get_class_name(iconView);
                                    std::wstring iconClassNameStr(iconClassName);

                                    if (iconClassNameStr.find(L"IconView") != std::wstring::npos) {
                                        Wh_Log(L"[ApplyVertical] Styling IconView at container index %d", j);
                                        ApplyVerticalTransform(iconView, j);
                                    }
                                }
                            }
                        }
                    }
                }
            } else {
                Wh_Log(L"[ApplyVertical] Could not find OmniButton");
            }
        } else {
            Wh_Log(L"[ApplyVertical] Could not find SystemTray root");
        }

    } catch (...) {
        Wh_Log(L"[ApplyVertical] Exception");
    }
}

// --- Helper: obtain FrameworkElement from pThis safely ---
static winrt::com_ptr<winrt::Windows::UI::Xaml::FrameworkElement> GetFrameworkElementFromThis(void* pThis)
{
    if (!pThis) return nullptr;

    // pThis typically implements IUnknown; treat it as IUnknown*
    IUnknown* rawUnknown = reinterpret_cast<IUnknown*>(pThis);
    if (!rawUnknown) return nullptr;

    // add ref while we QI so object doesn't disappear mid-query
    rawUnknown->AddRef();

    winrt::com_ptr<IUnknown> unk;
    unk.attach(rawUnknown); // attach takes ownership (we already AddRef'ed)

    // Try to QI to IInspectable first (common for WinRT objects)
    winrt::com_ptr<IInspectable> insp;
    HRESULT hr = unk->QueryInterface(winrt::guid_of<winrt::Windows::Foundation::IInspectable>().data(),
                                     reinterpret_cast<void**>(winrt::put_abi(insp)));
    if (FAILED(hr) || !insp) {
        // Fallback: try QI directly to FrameworkElement
        winrt::com_ptr<::IUnknown> feunk;
        hr = unk->QueryInterface(winrt::guid_of<winrt::Windows::UI::Xaml::FrameworkElement>().data(),
                                 reinterpret_cast<void**>(winrt::put_abi(feunk)));
        if (FAILED(hr) || !feunk) {
            rawUnknown->Release();
            return nullptr;
        }
        // Convert feunk to FrameworkElement com_ptr
        winrt::com_ptr<winrt::Windows::UI::Xaml::FrameworkElement> fe;
        fe.copy_from(reinterpret_cast<winrt::Windows::UI::Xaml::FrameworkElement*>(feunk.get()));
        rawUnknown->Release();
        return fe;
    }

    // Convert inspectable to FrameworkElement
    winrt::Windows::UI::Xaml::FrameworkElement fe = insp.try_as<winrt::Windows::UI::Xaml::FrameworkElement>();
    rawUnknown->Release();
    if (!fe) return nullptr;

    winrt::com_ptr<winrt::Windows::UI::Xaml::FrameworkElement> fe_ptr;
    fe_ptr.copy_from(winrt::get_abi(fe));
    return fe_ptr;
}

// --- Helper: get child's index in its parent (returns -1 if unknown) ---
static int GetIndexInParent(winrt::Windows::UI::Xaml::FrameworkElement const& child)
{
    try {
        auto parent = winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetParent(child);
        if (!parent) return -1;
        auto parentFE = parent.try_as<winrt::Windows::UI::Xaml::FrameworkElement>();
        if (!parentFE) return -1;

        int count = winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetChildrenCount(parentFE);
        for (int i = 0; i < count; ++i) {
            auto c = winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetChild(parentFE, i).try_as<winrt::Windows::UI::Xaml::FrameworkElement>();
            if (c && c == child) return i;
        }
    } catch (...) {}
    return -1;
}

// --- Replacement: IconView constructor hook (matches your file) ---
void IconView_IconView_Hook(void* pThis) {
    Wh_Log(L"=== IconView::IconView constructor called (HOOK) ===");

    // Call original constructor
    IconView_IconView_Original(pThis);

    if (g_unloading || !g_settings.enableVertical) return;

    // Safely obtain FrameworkElement
    auto fe_ptr = GetFrameworkElementFromThis(pThis);
    if (!fe_ptr) {
        Wh_Log(L"[IconView Hook] Failed to obtain FrameworkElement from pThis");
        return;
    }

    // Convert com_ptr to FrameworkElement object
    FrameworkElement iconView = *reinterpret_cast<FrameworkElement*>(fe_ptr.get());

    Wh_Log(L"[IconView Hook] created class=%s name=%s",
           winrt::to_hstring(winrt::get_class_name(iconView)).c_str(),
           iconView.Name().c_str());

    // Register Loaded handler (runs on UI thread)
    iconView.Loaded([iconView](auto&&, auto&&) {
        try {
            Wh_Log(L"[IconView Loaded] Loaded fired - checking parents");

            // Walk up parents to detect OmniButton/ControlCenterButton
            auto current = iconView;
            bool isOmni = false;
            for (int depth = 0; depth < 10; ++depth) {
                auto parent = winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetParent(current);
                if (!parent) break;
                auto parentFE = parent.try_as<FrameworkElement>();
                if (!parentFE) break;
                auto cls = winrt::get_class_name(parentFE);
                auto nm = parentFE.Name();
                std::wstring clsStr(cls);
                Wh_Log(L"[IconView Loaded] Parent %d class=%s name=%s", depth, cls.c_str(), nm.c_str());
                if (clsStr.find(L"OmniButton") != std::wstring::npos || nm == L"ControlCenterButton") {
                    isOmni = true;
                    break;
                }
                current = parentFE;
            }

            if (!isOmni) {
                Wh_Log(L"[IconView Loaded] Not an OmniButton icon - skipping");
                return;
            }

            Wh_Log(L"[IconView Loaded] OmniButton icon detected - applying vertical transform");
            // Use runtime index detection; pass 0 for the second param (ignored now)
            ApplyVerticalTransform(iconView, 0);

        } catch (...) {
            Wh_Log(L"[IconView Loaded] Exception in Loaded handler");
        }
    });
}

// Apply styling to existing OmniButton icons in the XAML tree
void ApplyStyleToExistingIcons(XamlRoot xamlRoot) {
    Wh_Log(L"[ApplyStyle] Searching existing XAML tree for OmniButton icons");

    if (!xamlRoot) {
        Wh_Log(L"[ApplyStyle] XamlRoot is null");
        return;
    }

    try {
        auto rootContent = xamlRoot.Content();
        if (!rootContent) {
            Wh_Log(L"[ApplyStyle] No content in XamlRoot");
            return;
        }

        auto rootElement = rootContent.try_as<FrameworkElement>();
        if (!rootElement) {
            Wh_Log(L"[ApplyStyle] Root content is not a FrameworkElement");
            return;
        }

        Wh_Log(L"[ApplyStyle] Starting tree traversal from root");
        TraverseAndStyleXamlTree(rootElement, 0);

    } catch (...) {
        Wh_Log(L"[ApplyStyle] Exception");
    }
}

// Run ApplyStyleToExistingIcons on the window thread
void ApplySettings() {
    Wh_Log(L"[ApplySettings] Running on window thread to find existing icons");

    // This needs to run on the UI thread where the XAML elements live
    // We'll use a similar approach to the reference mod
    HWND hTaskbarWnd = FindWindow(L"Shell_TrayWnd", nullptr);
    if (!hTaskbarWnd) {
        Wh_Log(L"[ApplySettings] Could not find Shell_TrayWnd");
        return;
    }

    Wh_Log(L"[ApplySettings] Found taskbar window: %p", hTaskbarWnd);

    // Try to enumerate all XamlRoot instances
    // This is tricky - we need access to the XamlRoot, which the reference mod gets differently
    // For now, just log that we tried
    Wh_Log(L"[ApplySettings] TODO: Need to get XamlRoot to traverse existing tree");
    Wh_Log(L"[ApplySettings] For now, the hook will catch icons as they're created");
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

    // Hook IconView constructor - called when each icon is created
    // Use the correct pattern: ((IUnknown**)pThis)[1] to get the FrameworkElement
    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {
            {LR"(public: __cdecl winrt::SystemTray::implementation::IconView::IconView(void))"},
            &IconView_IconView_Original,
            IconView_IconView_Hook,
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

// Recursively traverse XAML tree to find OmniButton
void TraverseAndStyleXamlTree(FrameworkElement element, int depth = 0) {
    if (!element || depth > 20) return;  // Prevent infinite recursion

    try {
        auto className = winrt::get_class_name(element);
        std::wstring classNameStr(className);

        // Log if debugging
        if (g_settings.debugLogging && depth < 5) {
            std::wstring indent(depth * 2, L' ');
            Wh_Log(L"%s[Traverse] %s", indent.c_str(), className.c_str());
        }

        // Check if this is OmniButton or ControlCenterButton
        if (classNameStr.find(L"OmniButton") != std::wstring::npos ||
            classNameStr.find(L"ControlCenterButton") != std::wstring::npos) {
            Wh_Log(L"[Traverse] FOUND OmniButton at depth %d: %s", depth, className.c_str());

            // Find IconView children and style them
            int childCount = VisualTreeHelper::GetChildrenCount(element);
            Wh_Log(L"[Traverse] OmniButton has %d children", childCount);

            for (int i = 0; i < childCount; i++) {
                auto child = VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();
                if (child) {
                    auto childClassName = winrt::get_class_name(child);
                    std::wstring childClassNameStr(childClassName);

                    if (childClassNameStr.find(L"IconView") != std::wstring::npos) {
                        Wh_Log(L"[Traverse] Found IconView child at index %d", i);
                        StyleOmniButtonIcon(child);
                    }
                }
            }

            return;  // Found it, no need to traverse deeper
        }

        // Traverse children
        int childCount = VisualTreeHelper::GetChildrenCount(element);
        for (int i = 0; i < childCount; i++) {
            auto child = VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();
            if (child) {
                TraverseAndStyleXamlTree(child, depth + 1);
            }
        }

    } catch (...) {
        // Silently ignore - some elements might not be accessible
    }
}

// Find the SystemTray XAML root and traverse it
void FindAndStyleOmniButton() {
    Wh_Log(L"[FindAndStyle] Searching for OmniButton in XAML tree...");

    try {
        // Get the Taskbar.View.dll module
        HMODULE taskbarViewModule = GetModuleHandle(L"Taskbar.View.dll");
        if (!taskbarViewModule) {
            Wh_Log(L"[FindAndStyle] Taskbar.View.dll not loaded");
            return;
        }

        // Try to get the SystemTray window
        // We need to enumerate windows and find the right one
        // For now, just log that we're trying
        Wh_Log(L"[FindAndStyle] Taskbar.View.dll is loaded, but we need a XAML root element");
        Wh_Log(L"[FindAndStyle] This requires finding the XamlIslandRoot or using Windows.UI.Xaml APIs");
        Wh_Log(L"[FindAndStyle] Will wait for user to interact with system tray to trigger hook");

    } catch (...) {
        Wh_Log(L"[FindAndStyle] Exception");
    }
}

void Wh_ModAfterInit() {
    Wh_Log(L"=== AfterInit called ===");

    // Try hooking again in case Taskbar.View.dll loaded after init
    if (!IconView_IconView_Original) {
        Wh_Log(L"Symbols not hooked yet, trying again");
        HookTaskbarViewSymbols();
    }

    // Try to find and style existing OmniButton
    FindAndStyleOmniButton();

    Wh_Log(L"=== Waiting for system tray interaction to trigger hook ===");
    Wh_Log(L"=== Try clicking wifi/sound/battery icons ===");
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
