// ==WindhawkMod==
// @id              vertical-system-tray-icons
// @name            Vertical System Tray Icons
// @description     Forces the system tray icons (Network, Sound, Battery) into a vertical column or grid. Useful for double-height taskbars.
// @version         1.0
// @author          Modder
// @github          https://github.com/
// @include         explorer.exe
// @compilerOptions -lwindows.ui.xaml
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
This mod hooks into the Windows XAML layout system to force the system tray icons 
(contained in the "OmniButton") to align vertically.

This is specifically designed for users with a double-height taskbar (e.g., via other Windhawk mods)
who want the system icons to stack neatly instead of forming a long horizontal row.
*/
// ==/WindhawkModReadme==

#include <windows.h>
#include <string>
#include <vector>
#include <wrl.h>
#include <windows.ui.xaml.h>
#include <windows.ui.xaml.controls.h>
#include <windows.ui.xaml.media.h>
#include <windows.foundation.h>

using namespace Microsoft::WRL;
using namespace ABI::Windows::UI::Xaml;
using namespace ABI::Windows::UI::Xaml::Controls;
using namespace ABI::Windows::UI::Xaml::Media;
using namespace ABI::Windows::Foundation;

// Global settings
// Set to true to try and force a Grid (WrapGrid) layout. 
// Set to false for a simple Vertical Column (StackPanel).
bool g_ForceGrid = false; 

// Typedef for the Measure method we are hooking
typedef HRESULT (WINAPI *Measure_t)(IUIElement* pThis, Size availableSize);
Measure_t pOriginalMeasure = nullptr;

// Helper to get the Runtime Class Name of an object
std::wstring GetRuntimeClassName(IInspectable* pInspectable) {
    HSTRING hClassName;
    if (SUCCEEDED(pInspectable->GetRuntimeClassName(&hClassName))) {
        UINT32 length;
        PCWSTR buffer = WindowsGetStringRawBuffer(hClassName, &length);
        std::wstring name(buffer, length);
        WindowsDeleteString(hClassName);
        return name;
    }
    return L"";
}

// Helper to check ancestry
bool IsSystemTrayStackPanel(IUIElement* pElement) {
    // We are looking for: SystemTray.OmniButton -> ... -> ItemsPresenter -> StackPanel
    
    // 1. Verify this is a StackPanel
    ComPtr<IInspectable> spInsp;
    pElement->QueryInterface(IID_PPV_ARGS(&spInsp));
    if (GetRuntimeClassName(spInsp.Get()) != L"Windows.UI.Xaml.Controls.StackPanel") {
        return false;
    }

    // 2. Walk up the visual tree
    ComPtr<IDependencyObject> current;
    pElement->QueryInterface(IID_PPV_ARGS(&current));

    // We need IVisualTreeHelperStatics to walk up
    // However, since we are inside a layout pass, the parent property usually works on FrameworkElement
    ComPtr<IFrameworkElement> spFe;
    if (FAILED(pElement->QueryInterface(IID_PPV_ARGS(&spFe)))) return false;

    ComPtr<IDependencyObject> parent;
    spFe->GetParent(&parent);
    
    // Robust check: Walk up up to 5 levels to find "ControlCenterButton"
    for (int i = 0; i < 6 && parent; i++) {
        ComPtr<IFrameworkElement> parentFe;
        if (SUCCEEDED(parent.As(&parentFe))) {
            HSTRING hName;
            if (SUCCEEDED(parentFe->get_Name(&hName))) {
                UINT32 len;
                PCWSTR buf = WindowsGetStringRawBuffer(hName, &len);
                std::wstring nameStr(buf, len);
                WindowsDeleteString(hName);

                // The OmniButton has x:Name="ControlCenterButton"
                if (nameStr == L"ControlCenterButton") {
                    // Double check class name to be sure
                    ComPtr<IInspectable> parentInsp;
                    parent.As(&parentInsp);
                    std::wstring className = GetRuntimeClassName(parentInsp.Get());
                    // Class is usually SystemTray.OmniButton
                    if (className.find(L"OmniButton") != std::wstring::npos) {
                        return true;
                    }
                }
            }
        }

        // Move up
        ComPtr<IDependencyObject> nextParent;
        // Try getting visual parent if logical parent fails, but for this specific tree structure, logical parent (GetParent) is usually fine.
        // If we needed VisualTreeHelper: 
        // GetVisualTreeHelperStatics()->GetParent(current, &nextParent);
        // Stick to logical parent for simplicity as it links ItemsPresenter to ItemsControl.
        parentFe->GetParent(&nextParent);
        parent = nextParent;
    }

    return false;
}

// The Hook
HRESULT WINAPI MeasureHook(IUIElement* pThis, Size availableSize) {
    if (IsSystemTrayStackPanel(pThis)) {
        ComPtr<IStackPanel> spStackPanel;
        if (SUCCEEDED(pThis->QueryInterface(IID_PPV_ARGS(&spStackPanel)))) {
            
            // Force Vertical Orientation
            Orientation currentOrient;
            spStackPanel->get_Orientation(&currentOrient);
            if (currentOrient != Orientation_Vertical) {
                spStackPanel->put_Orientation(Orientation_Vertical);
            }

            // OPTIONAL: Adjust margins or spacing if needed
            // ComPtr<IFrameworkElement> spFe;
            // pThis->QueryInterface(IID_PPV_ARGS(&spFe));
            // spFe->put_Margin({0, 0, 0, 0});
        }
    }

    return pOriginalMeasure(pThis, availableSize);
}

BOOL Wh_ModInit() {
    Wh_Log(L"Init Vertical System Tray Icons");

    // We hook UIElement::Measure because it's a reliable choke point 
    // that runs whenever layout updates.
    
    // Symbol: Windows.UI.Xaml.UIElement::Measure(Windows.Foundation.Size)
    // The symbol is usually exported by Windows.UI.Xaml.dll.
    // We use the mangled name for x64.
    // ?Measure@UIElement@Xaml@UI@Windows@@QEAAXUSize@Foundation@4@@Z
    
    HMODULE hXaml = LoadLibrary(L"Windows.UI.Xaml.dll");
    if (!hXaml) {
        Wh_Log(L"Failed to load Xaml dll");
        return FALSE;
    }

    // Try finding the export
    void* pMeasureAddr = (void*)GetProcAddress(hXaml, "?Measure@UIElement@Xaml@UI@Windows@@QEAAXUSize@Foundation@4@@Z");
    
    if (!pMeasureAddr) {
        Wh_Log(L"Failed to find Measure symbol. Trying alternate...");
        // Sometimes symbols vary. If this fails, we might need a pattern scan or different symbol.
        return FALSE;
    }

    // Install Hook
    Wh_SetFunctionHook(pMeasureAddr, (void*)MeasureHook, (void**)&pOriginalMeasure);

    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L"Uninit");
}