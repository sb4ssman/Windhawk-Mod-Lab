// ==WindhawkMod==
// @id              vertical-omnibutton
// @name            Vertical OmniButton
// @description     Stacks Windows 11 wifi/volume/battery OmniButton vertically
// @version         1.9.0
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

Uses the Windows XAML Diagnostics API to watch the live XAML visual tree
(the same mechanism used by the Windows 11 Taskbar Styler). When OmniButton
elements appear, the mod forces `Orientation=Vertical` on the inner StackPanel
and sizes each icon slot according to your settings.

## Usage

After enabling the mod, **restart explorer.exe** (Task Manager → Restart).

Enable **debug logging** to trace which XAML elements are being checked.

## Settings

- **Enable vertical arrangement** — master toggle
- **Icon width** — width of each icon slot in pixels (16–64, default 32)
- **Icon height** — height of each icon slot in pixels (16–64, default 28)
- **Debug logging** — log XAML element types as they are added

*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- enableVertical: true
  $name: Enable vertical arrangement
  $description: Enable/disable vertical stacking of system tray icons
- iconWidth: 32
  $name: Icon width
  $description: Width of each icon slot in pixels (16-64)
- iconHeight: 28
  $name: Icon height
  $description: Height of each icon slot in pixels (16-64)
- debugLogging: false
  $name: Enable debug logging
  $description: Log XAML element types as they are added to the visual tree
*/
// ==/WindhawkModSettings==

#include <unknwn.h>
#include <winrt/base.h>
#include <windhawk_api.h>
#include <windhawk_utils.h>

#undef GetCurrentTime

#include <windows.h>
#include <objidl.h>   // IObjectWithSite
#include <xamlom.h>   // IXamlDiagnostics, IVisualTreeService3, XAML diagnostics types

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

// ── Settings ───────────────────────────────────────────────────────────────

struct {
    bool enableVertical;
    int iconWidth;
    int iconHeight;
    bool debugLogging;
} g_settings;

bool g_unloading = false;

void LoadSettings() {
    g_settings.enableVertical = Wh_GetIntSetting(L"enableVertical") != 0;
    g_settings.iconWidth      = Wh_GetIntSetting(L"iconWidth");
    g_settings.iconHeight     = Wh_GetIntSetting(L"iconHeight");
    g_settings.debugLogging   = Wh_GetIntSetting(L"debugLogging") != 0;

    if (g_settings.iconWidth  < 16) g_settings.iconWidth  = 16;
    if (g_settings.iconWidth  > 64) g_settings.iconWidth  = 64;
    if (g_settings.iconHeight < 16) g_settings.iconHeight = 16;
    if (g_settings.iconHeight > 64) g_settings.iconHeight = 64;
}

// ── CLSID for our TAP ─────────────────────────────────────────────────────
// {7E6D9C3A-2F1B-4A58-B3E7-1D5C8F9A2B6E}

static const CLSID CLSID_OmniButtonTAP =
    { 0x7E6D9C3A, 0x2F1B, 0x4A58, { 0xB3, 0xE7, 0x1D, 0x5C, 0x8F, 0x9A, 0x2B, 0x6E } };

// ── Cached element references for cleanup on unload ───────────────────────

static StackPanel       g_omniStackPanel{ nullptr };
static FrameworkElement g_omniButton{ nullptr };

// ── Layout application ────────────────────────────────────────────────────

static bool IsOmniButtonParent(DependencyObject const& start) {
    DependencyObject cur = start;
    for (int depth = 0; depth < 12; depth++) {
        auto p = VisualTreeHelper::GetParent(cur);
        if (!p) break;
        auto pfe = p.try_as<FrameworkElement>();
        if (!pfe) break;
        auto cls = winrt::get_class_name(pfe);
        auto nm  = pfe.Name();
        if (g_settings.debugLogging) {
            Wh_Log(L"[Layout] Parent[%d] class=%s name=%s", depth, cls.c_str(), nm.c_str());
        }
        std::wstring clsStr(cls.c_str());
        if (clsStr.find(L"OmniButton") != std::wstring::npos ||
            nm == L"ControlCenterButton") {
            g_omniButton = pfe;
            return true;
        }
        cur = p;
    }
    return false;
}

static void ApplyLayout(StackPanel const& sp) {
    if (g_omniStackPanel) return;  // already found the right one, ignore others
    if (!sp.IsItemsHost()) return;
    if (!IsOmniButtonParent(sp)) return;

    g_omniStackPanel = sp;

    sp.Orientation(Orientation::Vertical);

    int n = VisualTreeHelper::GetChildrenCount(sp);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(sp, i).try_as<FrameworkElement>();
        if (!child) continue;
        child.Width(g_settings.iconWidth);
        child.Height(g_settings.iconHeight);
    }

    if (g_omniButton) {
        g_omniButton.MinWidth(g_settings.iconWidth + 16.0);
    }

    Wh_Log(L"[Layout] Applied vertical layout (children=%d)", n);
}

// Size a ContentPresenter that's a direct child of the OmniButton StackPanel.
static void ApplyContentPresenterSize(FrameworkElement const& fe) {
    auto parent = VisualTreeHelper::GetParent(fe).try_as<StackPanel>();
    if (!parent || parent != g_omniStackPanel) return;
    fe.Width(g_settings.iconWidth);
    fe.Height(g_settings.iconHeight);
}

// ── XAML diagnostics — VisualTreeWatcher ──────────────────────────────────

class VisualTreeWatcher
    : public winrt::implements<VisualTreeWatcher, IVisualTreeServiceCallback2, winrt::non_agile>
{
public:
    explicit VisualTreeWatcher(winrt::com_ptr<IUnknown> const& site)
        : m_diagnostics(site.as<IXamlDiagnostics>())
    {
        Wh_Log(L"[TAP] VisualTreeWatcher created");
        AddRef();
        HANDLE t = CreateThread(nullptr, 0, [](LPVOID p) -> DWORD {
            auto self = reinterpret_cast<VisualTreeWatcher*>(p);
            HRESULT hr = self->m_diagnostics.as<IVisualTreeService3>()
                             ->AdviseVisualTreeChange(self);
            Wh_Log(L"[TAP] AdviseVisualTreeChange: %08X", hr);
            self->Release();
            return 0;
        }, this, 0, nullptr);
        if (t) CloseHandle(t);
        else    Release();
    }

    void Unadvise() {
        m_diagnostics.as<IVisualTreeService3>()->UnadviseVisualTreeChange(this);
    }

private:
    winrt::com_ptr<IXamlDiagnostics> m_diagnostics;

    winrt::Windows::Foundation::IInspectable FromHandle(InstanceHandle handle) {
        winrt::Windows::Foundation::IInspectable obj;
        m_diagnostics->GetIInspectableFromHandle(
            handle, reinterpret_cast<::IInspectable**>(winrt::put_abi(obj)));
        return obj;
    }

    HRESULT STDMETHODCALLTYPE OnVisualTreeChange(
        ParentChildRelation,
        VisualElement element,
        VisualMutationType mutationType) override try
    {
        if (mutationType != VisualMutationType::Add) return S_OK;
        if (!g_settings.enableVertical || g_unloading) return S_OK;

        if (g_settings.debugLogging && element.Type) {
            Wh_Log(L"[TAP] Add: %s", element.Type);
        }

        auto insp = FromHandle(element.Handle);
        if (!insp) return S_OK;

        try {
            auto sp = insp.try_as<StackPanel>();
            if (sp) { ApplyLayout(sp); return S_OK; }

            // If we already found the StackPanel, size newly added ContentPresenters.
            if (g_omniStackPanel) {
                auto fe = insp.try_as<FrameworkElement>();
                if (fe) ApplyContentPresenterSize(fe);
            }
        } catch (...) {}

        return S_OK;
    }
    catch (...) { return S_OK; }

    HRESULT STDMETHODCALLTYPE OnElementStateChanged(
        InstanceHandle, VisualElementState, LPCWSTR) noexcept override
    { return S_OK; }
};

// ── XAML diagnostics — OmniButtonTAP (IObjectWithSite) ────────────────────

static winrt::com_ptr<VisualTreeWatcher> g_visualTreeWatcher;

class OmniButtonTAP
    : public winrt::implements<OmniButtonTAP, IObjectWithSite, winrt::non_agile>
{
public:
    HRESULT STDMETHODCALLTYPE SetSite(IUnknown* pUnkSite) override try {
        if (g_visualTreeWatcher) {
            g_visualTreeWatcher->Unadvise();
            g_visualTreeWatcher = nullptr;
        }
        if (pUnkSite) {
            winrt::com_ptr<IUnknown> site;
            site.copy_from(pUnkSite);
            g_visualTreeWatcher = winrt::make_self<VisualTreeWatcher>(site);
        }
        // Balance the LoadLibrary that InitializeXamlDiagnosticsEx does on our DLL.
        FreeLibrary(GetCurrentModuleHandleHelper());
        return S_OK;
    }
    catch (...) { return winrt::to_hresult(); }

    HRESULT STDMETHODCALLTYPE GetSite(REFIID riid, void** ppvSite) noexcept override
    { return E_NOTIMPL; }

private:
    static HMODULE GetCurrentModuleHandleHelper() {
        HMODULE h = nullptr;
        GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(&GetCurrentModuleHandleHelper), &h);
        return h;
    }
};

// ── IClassFactory for OmniButtonTAP ───────────────────────────────────────

struct OmniButtonTAPFactory
    : winrt::implements<OmniButtonTAPFactory, IClassFactory>
{
    HRESULT STDMETHODCALLTYPE CreateInstance(
        IUnknown* pOuter, REFIID riid, void** ppv) noexcept override
    {
        if (pOuter) return CLASS_E_NOAGGREGATION;
        try {
            auto tap = winrt::make_self<OmniButtonTAP>();
            return tap->QueryInterface(riid, ppv);
        } catch (...) { return winrt::to_hresult(); }
    }
    HRESULT STDMETHODCALLTYPE LockServer(BOOL) noexcept override { return S_OK; }
};

// InitializeXamlDiagnosticsEx loads our DLL and calls DllGetClassObject to
// get the factory for CLSID_OmniButtonTAP.
extern "C" __declspec(dllexport)
HRESULT WINAPI DllGetClassObject(GUID const& clsid, GUID const& iid, void** ppv) {
    Wh_Log(L"[TAP] DllGetClassObject called");
    if (!IsEqualGUID(clsid, CLSID_OmniButtonTAP))
        return CLASS_E_CLASSNOTAVAILABLE;
    try {
        return winrt::make_self<OmniButtonTAPFactory>()->QueryInterface(iid, ppv);
    } catch (...) { return winrt::to_hresult(); }
}

// ── TAP injection ─────────────────────────────────────────────────────────

static HMODULE GetCurrentModuleHandle() {
    HMODULE h = nullptr;
    GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&GetCurrentModuleHandle), &h);
    return h;
}

using PFN_IXDE = HRESULT(WINAPI*)(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, CLSID, LPCWSTR);

static void InjectOmniButtonTAP() {
    HMODULE hXaml = GetModuleHandleW(L"Windows.UI.Xaml.dll");
    if (!hXaml) { Wh_Log(L"[TAP] Windows.UI.Xaml.dll not loaded"); return; }

    auto ixde = reinterpret_cast<PFN_IXDE>(
        GetProcAddress(hXaml, "InitializeXamlDiagnosticsEx"));
    if (!ixde) { Wh_Log(L"[TAP] InitializeXamlDiagnosticsEx not found"); return; }

    HMODULE hSelf = GetCurrentModuleHandle();
    WCHAR selfPath[MAX_PATH];
    if (!GetModuleFileNameW(hSelf, selfPath, MAX_PATH)) {
        Wh_Log(L"[TAP] GetModuleFileName failed (%lu)", GetLastError());
        return;
    }

    Wh_Log(L"[TAP] Injecting from: %s", selfPath);

    HRESULT hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    for (int i = 1; i <= 10000 && hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND); i++) {
        WCHAR conn[64];
        wsprintf(conn, L"VisualDiagConnection%d", i);
        hr = ixde(conn, GetCurrentProcessId(), L"", selfPath,
                  CLSID_OmniButtonTAP, nullptr);
    }
    Wh_Log(L"[TAP] InitializeXamlDiagnosticsEx result: %08X", hr);
}

// ── LoadLibraryExW hook ────────────────────────────────────────────────────

using LoadLibraryExW_t = decltype(&LoadLibraryExW);
LoadLibraryExW_t LoadLibraryExW_Original;

HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
    HMODULE hModule = LoadLibraryExW_Original(lpLibFileName, hFile, dwFlags);

    if (hModule && !g_visualTreeWatcher && lpLibFileName) {
        const wchar_t* name = wcsrchr(lpLibFileName, L'\\');
        name = name ? (name + 1) : lpLibFileName;
        if (_wcsicmp(name, L"Windows.UI.Xaml.dll") == 0) {
            Wh_Log(L"[LoadLib] Windows.UI.Xaml.dll loaded — injecting TAP");
            InjectOmniButtonTAP();
        }
    }

    return hModule;
}

// ── Windhawk lifecycle ─────────────────────────────────────────────────────

BOOL Wh_ModInit() {
    Wh_Log(L"[Init] Vertical OmniButton v1.9.0");
    LoadSettings();

    HMODULE kernelbase = GetModuleHandleW(L"kernelbase.dll");
    auto pLoadLibraryExW = kernelbase
        ? reinterpret_cast<LoadLibraryExW_t>(GetProcAddress(kernelbase, "LoadLibraryExW"))
        : nullptr;

    if (pLoadLibraryExW) {
        Wh_SetFunctionHook((void*)pLoadLibraryExW,
                            (void*)LoadLibraryExW_Hook,
                            (void**)&LoadLibraryExW_Original);
        Wh_Log(L"[Init] LoadLibraryExW hook queued");
    }

    HMODULE hXaml = GetModuleHandleW(L"Windows.UI.Xaml.dll");
    Wh_Log(L"[Init] Windows.UI.Xaml.dll at init: %p", (void*)hXaml);
    if (hXaml) {
        InjectOmniButtonTAP();
    }

    return TRUE;
}

void Wh_ModAfterInit() {
    if (!g_visualTreeWatcher) {
        HMODULE hXaml = GetModuleHandleW(L"Windows.UI.Xaml.dll");
        if (hXaml) {
            Wh_Log(L"[AfterInit] Retrying inject...");
            InjectOmniButtonTAP();
        }
    }
    Wh_Log(L"[AfterInit] watcher=%p", (void*)g_visualTreeWatcher.get());
}

void Wh_ModUninit() {
    g_unloading = true;
    Wh_Log(L"[Uninit]");

    if (g_omniStackPanel) {
        try {
            g_omniStackPanel.ClearValue(StackPanel::OrientationProperty());
            int n = VisualTreeHelper::GetChildrenCount(g_omniStackPanel);
            for (int i = 0; i < n; i++) {
                auto child = VisualTreeHelper::GetChild(g_omniStackPanel, i)
                                 .try_as<FrameworkElement>();
                if (child) {
                    child.ClearValue(FrameworkElement::WidthProperty());
                    child.ClearValue(FrameworkElement::HeightProperty());
                }
            }
        } catch (...) {}
        g_omniStackPanel = nullptr;
    }

    if (g_omniButton) {
        try { g_omniButton.ClearValue(FrameworkElement::MinWidthProperty()); }
        catch (...) {}
        g_omniButton = nullptr;
    }

    if (g_visualTreeWatcher) {
        g_visualTreeWatcher->Unadvise();
        g_visualTreeWatcher = nullptr;
    }
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Wh_Log(L"[Settings] Updated");
    if (g_omniStackPanel && g_settings.enableVertical) {
        try {
            // Re-apply sizing directly (ApplyLayout guards against re-entry via g_omniStackPanel)
            g_omniStackPanel.Orientation(Orientation::Vertical);
            int n = VisualTreeHelper::GetChildrenCount(g_omniStackPanel);
            for (int i = 0; i < n; i++) {
                auto child = VisualTreeHelper::GetChild(g_omniStackPanel, i).try_as<FrameworkElement>();
                if (child) { child.Width(g_settings.iconWidth); child.Height(g_settings.iconHeight); }
            }
            if (g_omniButton) g_omniButton.MinWidth(g_settings.iconWidth + 16.0);
        } catch (...) {}
    }
}
