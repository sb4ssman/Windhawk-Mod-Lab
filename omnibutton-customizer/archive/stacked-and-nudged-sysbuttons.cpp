// ==WindhawkMod==
// @id              system-tray-aligner
// @name            System Tray Pixel Aligner
// @description     Individually nudges the Wifi and Battery icons for perfect alignment.
// @version         1.1
// @author          PixelPerfectionist
// @include         explorer.exe
// @compilerOptions -lOLE32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
This mod hooks into the Windows XAML layout system to provide per-icon positioning.
It targets the system tray vertical stack (Wifi, Sound, Battery) and applies 
manual margins to correct optical alignment issues caused by icon shapes.
*/
// ==/WindhawkModReadme==

#include <windows.h>
#include <string>

// =============================================================
//  CONFIGURATION
//  Change these values to tweak your alignment!
// =============================================================

// Index 0: Wi-Fi (Top)
// To move LEFT: Increase RIGHT margin.
// To move RIGHT: Increase LEFT margin.
const double WIFI_MARGIN_LEFT   = 0;
const double WIFI_MARGIN_TOP    = 0;
const double WIFI_MARGIN_RIGHT  = 4.0; // Pushes Wifi Left
const double WIFI_MARGIN_BOTTOM = 0;

// Index 2: Battery (Bottom)
// To move RIGHT: Increase LEFT margin.
const double BATT_MARGIN_LEFT   = 2.0; // Pushes Battery Right
const double BATT_MARGIN_TOP    = 0;
const double BATT_MARGIN_RIGHT  = 0;
const double BATT_MARGIN_BOTTOM = 0;

// =============================================================
//  Manual Interface Definitions (Fixes Compilation Errors)
// =============================================================

// GUIDs
const IID IID_IPanel            = { 0x65a8994c, 0xf312, 0x47b3, { 0x9e, 0x5b, 0x65, 0x14, 0x95, 0x6c, 0x86, 0x7e } };
const IID IID_IVector           = { 0x913337e9, 0x11a1, 0x4345, { 0xa3, 0xa2, 0x4e, 0x7f, 0x95, 0x6e, 0x22, 0x2d } };
const IID IID_IFrameworkElement = { 0xa391d09b, 0x4a99, 0x4b7c, { 0x9d, 0x8d, 0x6f, 0xa5, 0xd0, 0x1f, 0x6f, 0xbf } };

// Structures
struct XamlThickness {
    double Left, Top, Right, Bottom;
};

struct XamlSize {
    float width;
    float height;
};

// Base Interface: IUnknown
struct IUnknown_Manual {
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) = 0;
    virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
    virtual ULONG STDMETHODCALLTYPE Release() = 0;
};

// Base Interface: IInspectable
struct IInspectable_Manual : public IUnknown_Manual {
    virtual HRESULT STDMETHODCALLTYPE GetIids(ULONG* iidCount, IID** iids) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetRuntimeClassName(void** className) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetTrustLevel(int* trustLevel) = 0;
};

// Interface: IFrameworkElement
struct IFrameworkElement_Manual : public IInspectable_Manual {
    virtual HRESULT STDMETHODCALLTYPE get_Triggers(void** value) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Resources(void** value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_Resources(void* value) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Tag(void** value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_Tag(void* value) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Language(void** value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_Language(void* value) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_ActualWidth(double* value) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_ActualHeight(double* value) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Width(double* value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_Width(double value) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Height(double* value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_Height(double value) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_MinWidth(double* value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_MinWidth(double value) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_MaxWidth(double* value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_MaxWidth(double value) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_MinHeight(double* value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_MinHeight(double value) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_MaxHeight(double* value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_MaxHeight(double value) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_HorizontalAlignment(int* value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_HorizontalAlignment(int value) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_VerticalAlignment(int* value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_VerticalAlignment(int value) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Margin(XamlThickness* value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_Margin(XamlThickness value) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Name(void** value) = 0;
};

// Interface: IPanel
struct IPanel_Manual : public IFrameworkElement_Manual {
    virtual HRESULT STDMETHODCALLTYPE get_Children(void** value) = 0;
};

// Interface: IVector
struct IVector_Manual : public IInspectable_Manual {
    virtual HRESULT STDMETHODCALLTYPE get_At(unsigned int index, void** item) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Size(unsigned int* size) = 0;
    // Other methods omitted for brevity, we only need GetAt and Size
};

// =============================================================
//  Helpers & Globals
// =============================================================

typedef PCWSTR (WINAPI *WindowsGetStringRawBuffer_t)(void* string, UINT32* length);
typedef HRESULT (WINAPI *WindowsDeleteString_t)(void* string);

WindowsGetStringRawBuffer_t pWindowsGetStringRawBuffer = nullptr;
WindowsDeleteString_t pWindowsDeleteString = nullptr;

typedef HRESULT (WINAPI *Measure_t)(void* pThis, XamlSize availableSize);
Measure_t pOriginalMeasure = nullptr;

std::wstring GetRuntimeClassName(void* pInspectable) {
    if (!pInspectable || !pWindowsGetStringRawBuffer) return L"";
    IInspectable_Manual* pInsp = (IInspectable_Manual*)pInspectable;
    void* hClassName = nullptr;
    
    if (SUCCEEDED(pInsp->GetRuntimeClassName(&hClassName))) {
        UINT32 length = 0;
        PCWSTR buffer = pWindowsGetStringRawBuffer(hClassName, &length);
        std::wstring name(buffer, length);
        pWindowsDeleteString(hClassName);
        return name;
    }
    return L"";
}

bool IsTargetStackPanel(void* pElement) {
    // Check if this is the StackPanel inside the tray
    // We check class name + number of children (simple heuristic)
    if (GetRuntimeClassName(pElement) != L"Windows.UI.Xaml.Controls.StackPanel") return false;

    IPanel_Manual* pPanel = nullptr;
    if (SUCCEEDED(((IUnknown_Manual*)pElement)->QueryInterface(IID_IPanel, (void**)&pPanel))) {
        void* pChildrenRaw = nullptr;
        if (SUCCEEDED(pPanel->get_Children(&pChildrenRaw))) {
            IVector_Manual* pChildren = nullptr;
            ((IUnknown_Manual*)pChildrenRaw)->QueryInterface(IID_IVector, (void**)&pChildren);
            
            unsigned int size = 0;
            if (pChildren) {
                pChildren->get_Size(&size);
                pChildren->Release();
            }
            
            if (pChildrenRaw) ((IUnknown_Manual*)pChildrenRaw)->Release();
            pPanel->Release();

            // The tray usually has exactly 3 items in that stack: Net, Sound, Batt
            // Or 4 if Mic is active. We only act if there are 3 or 4.
            return (size >= 3 && size <= 5);
        }
        pPanel->Release();
    }
    return false;
}

// =============================================================
//  The Hook
// =============================================================

HRESULT WINAPI MeasureHook(void* pThis, XamlSize availableSize) {
    // Run logic before measurement to set properties
    if (IsTargetStackPanel(pThis)) {
        IPanel_Manual* pPanel = nullptr;
        ((IUnknown_Manual*)pThis)->QueryInterface(IID_IPanel, (void**)&pPanel);
        
        void* pChildrenRaw = nullptr;
        pPanel->get_Children(&pChildrenRaw);
        
        IVector_Manual* pChildren = nullptr;
        ((IUnknown_Manual*)pChildrenRaw)->QueryInterface(IID_IVector, (void**)&pChildren);
        
        unsigned int count = 0;
        pChildren->get_Size(&count);
        
        for (unsigned int i = 0; i < count; i++) {
            void* pItemRaw = nullptr;
            pChildren->get_At(i, &pItemRaw);
            
            if (pItemRaw) {
                IFrameworkElement_Manual* pFe = nullptr;
                ((IUnknown_Manual*)pItemRaw)->QueryInterface(IID_IFrameworkElement, (void**)&pFe);
                
                if (pFe) {
                    // Reset Margin
                    XamlThickness m = {0,0,0,0};

                    // INDEX 0 = TOP (Wi-Fi)
                    if (i == 0) {
                        m = { WIFI_MARGIN_LEFT, WIFI_MARGIN_TOP, WIFI_MARGIN_RIGHT, WIFI_MARGIN_BOTTOM };
                    }
                    // INDEX 2 = BOTTOM (Battery)
                    // Note: If you have a Microphone icon, Battery might be Index 3. 
                    // It is safer to check the last index.
                    else if (i == count - 1) { 
                        m = { BATT_MARGIN_LEFT, BATT_MARGIN_TOP, BATT_MARGIN_RIGHT, BATT_MARGIN_BOTTOM };
                    }

                    pFe->put_Margin(m);
                    
                    // Force Center Alignment on the container
                    // 2 = Center
                    pFe->put_HorizontalAlignment(2); 

                    pFe->Release();
                }
                ((IUnknown_Manual*)pItemRaw)->Release();
            }
        }

        if (pChildren) pChildren->Release();
        if (pChildrenRaw) ((IUnknown_Manual*)pChildrenRaw)->Release();
        pPanel->Release();
    }

    return pOriginalMeasure(pThis, availableSize);
}

// =============================================================
//  Init
// =============================================================

BOOL Wh_ModInit() {
    Wh_Log(L"Init Pixel Aligner");

    HMODULE hComBase = LoadLibrary(L"combase.dll");
    if (hComBase) {
        pWindowsGetStringRawBuffer = (WindowsGetStringRawBuffer_t)GetProcAddress(hComBase, "WindowsGetStringRawBuffer");
        pWindowsDeleteString = (WindowsDeleteString_t)GetProcAddress(hComBase, "WindowsDeleteString");
    }

    HMODULE hXaml = LoadLibrary(L"Windows.UI.Xaml.dll");
    if (!hXaml) return FALSE;

    void* pMeasureAddr = (void*)GetProcAddress(hXaml, "?Measure@UIElement@Xaml@UI@Windows@@QEAAXUSize@Foundation@4@@Z");
    if (!pMeasureAddr) {
        Wh_Log(L"Failed to find Measure symbol");
        return FALSE;
    }

    Wh_SetFunctionHook(pMeasureAddr, (void*)MeasureHook, (void**)&pOriginalMeasure);
    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L"Uninit");
}