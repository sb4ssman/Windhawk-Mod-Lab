// ==WindhawkMod==
// @id              vertical-omnibutton
// @name            Vertical OmniButton
// @description     Stacks Windows 11 wifi/volume/battery OmniButton vertically
// @version         1.29.0
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

- **Enable vertical arrangement** — master toggle for wifi/volume/battery icons
- **Icon spacing** — extra vertical pixels between each icon (default 8)
- **Battery percentage** — Off / Inline (% in battery slot, 3rd row) / Stacked (% as separate 4th row). Inline and Stacked enable Windows native battery % — requires restarting explorer.exe.
- **Vertical clock** — splits clock into three rows: time / day / date
- **Clock row spacing** — extra vertical space between clock rows
- **Clock alignment** — Left / Center / Right (only applies when vertical clock is enabled)
- **Debug logging** — log XAML element types as they are added

*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- enableVertical: true
  $name: Enable vertical arrangement
  $description: Enable/disable vertical stacking of wifi, volume, and battery icons
- iconSpacing: 0
  $name: Icon spacing
  $description: Extra vertical space between icons in pixels (minimum 0, maximum 20)
- batteryMode: "off"
  $name: Battery percentage
  $description: "Off: battery icon only. Inline: percentage shown in the battery icon slot (3rd row). Stacked: percentage as a separate 4th row below the battery icon. Inline and Stacked enable the Windows native battery % display — requires restarting explorer.exe to take effect."
  $options:
    - "off": "Off — battery icon only"
    - "inline": "Inline — percentage in battery slot (3rd row)"
    - "stacked": "Stacked — percentage as 4th row below battery"
- verticalClock: true
  $name: Vertical clock (three rows)
  $description: Split the clock into three rows — time, day of week, date
- clockLineSpacing: 8
  $name: Clock row spacing
  $description: Extra vertical space between clock rows in pixels (minimum 0, maximum 20)
- clockAlignment: "center"
  $name: Clock alignment
  $description: "Aligns time, day, and date text. Only applies when Vertical clock is enabled."
  $options:
    - "left": "Left"
    - "center": "Center"
    - "right": "Right"
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
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

// ── Settings ───────────────────────────────────────────────────────────────

struct {
    bool enableVertical;
    bool verticalClock;
    int batteryMode;      // 0=off, 1=inline (3rd row), 2=stacked (4th row)
    int iconSpacing;      // extra px between icons in the vertical stack
    int clockAlignment;   // 0=Left 1=Center 2=Right
    int clockLineSpacing; // uniform spacing for all three clock rows
    bool debugLogging;
} g_settings;

bool g_unloading  = false;
bool g_ixdeStarted = false; // prevent double-injection from Wh_ModAfterInit

void LoadSettings() {
    g_settings.enableVertical = Wh_GetIntSetting(L"enableVertical") != 0;
    g_settings.verticalClock  = Wh_GetIntSetting(L"verticalClock") != 0;
    {
        auto* bm = Wh_GetStringSetting(L"batteryMode");
        if (bm) {
            if (wcscmp(bm, L"inline") == 0)       g_settings.batteryMode = 1;
            else if (wcscmp(bm, L"stacked") == 0) g_settings.batteryMode = 2;
            else                                   g_settings.batteryMode = 0;
            Wh_FreeStringSetting(bm);
        } else {
            g_settings.batteryMode = 0;
        }
    }
    g_settings.iconSpacing    = Wh_GetIntSetting(L"iconSpacing");
    if (g_settings.iconSpacing < 0)  g_settings.iconSpacing = 0;
    if (g_settings.iconSpacing > 20) g_settings.iconSpacing = 20;
    {
        auto* ca = Wh_GetStringSetting(L"clockAlignment");
        if (ca) {
            if (wcscmp(ca, L"left") == 0)        g_settings.clockAlignment = 0;
            else if (wcscmp(ca, L"right") == 0)  g_settings.clockAlignment = 2;
            else                                  g_settings.clockAlignment = 1;
            Wh_FreeStringSetting(ca);
        } else {
            g_settings.clockAlignment = 1;
        }
    }
    g_settings.clockLineSpacing   = Wh_GetIntSetting(L"clockLineSpacing");
    if (g_settings.clockLineSpacing < 0)  g_settings.clockLineSpacing = 0;
    if (g_settings.clockLineSpacing > 20) g_settings.clockLineSpacing = 20;
    g_settings.debugLogging       = Wh_GetIntSetting(L"debugLogging") != 0;
}

// ── CLSID for our TAP ─────────────────────────────────────────────────────
// {7E6D9C3A-2F1B-4A58-B3E7-1D5C8F9A2B6E}

static const CLSID CLSID_OmniButtonTAP =
    { 0x7E6D9C3A, 0x2F1B, 0x4A58, { 0xB3, 0xE7, 0x1D, 0x5C, 0x8F, 0x9A, 0x2B, 0x6E } };

// ── Cached element references for cleanup on unload ───────────────────────

static StackPanel       g_omniStackPanel{ nullptr };
static FrameworkElement g_omniButton{ nullptr };
static FrameworkElement g_batteryPresenter{ nullptr };
static StackPanel       g_batteryInnerPanel{ nullptr }; // inner panel flipped to Vertical for 4th row

static StackPanel       g_clockDayDatePanel{ nullptr };
static FrameworkElement g_clockButton{ nullptr };
static TextBlock        g_clockTimeTextBlock{ nullptr };
static TextBlock        g_clockDateTextBlock{ nullptr };

// ── Battery XAML helpers ──────────────────────────────────────────────────

static bool HasBatteryDescendant(DependencyObject const& node, int depth = 0) {
    if (depth > 3) return false;
    int n = VisualTreeHelper::GetChildrenCount(node);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(node, i);
        if (!child) continue;
        std::wstring cls(winrt::get_class_name(child).c_str());
        if (cls.find(L"Battery") != std::wstring::npos) return true;
        if (HasBatteryDescendant(child, depth + 1)) return true;
    }
    return false;
}

static bool WalkBatteryTree(DependencyObject const& node, int depth) {
    if (depth > 5) return false;
    int n = VisualTreeHelper::GetChildrenCount(node);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(node, i);
        if (!child) continue;
        if (g_settings.debugLogging) {
            auto fe = child.try_as<FrameworkElement>();
            auto tb = child.try_as<TextBlock>();
            if (fe) Wh_Log(L"[Battery4] [%d/%d] %s \"%s\"%s",
                depth, i, winrt::get_class_name(fe).c_str(), fe.Name().c_str(),
                tb ? (std::wstring(L" text=") + tb.Text().c_str()).c_str() : L"");
        }
        auto sp = child.try_as<StackPanel>();
        if (sp && !sp.IsItemsHost() && sp.Orientation() == Orientation::Horizontal) {
            sp.Orientation(Orientation::Vertical);
            g_batteryInnerPanel = sp;
            Wh_Log(L"[Battery4] Flipped inner StackPanel at depth %d", depth);
            return true;
        }
        if (WalkBatteryTree(child, depth + 1)) return true;
    }
    return false;
}

// Walk the battery ContentPresenter's subtree. When showBatteryPercent is on,
// try to find a horizontal StackPanel (the icon+% layout) and flip it Vertical
// so the % appears below the battery icon glyph.
static void FlipBatteryLayout(FrameworkElement const& batteryCP) {
    if (!WalkBatteryTree(batteryCP, 0))
        Wh_Log(L"[Battery4] No horizontal StackPanel found — enable debug logging to see tree");
}

// ── Battery percentage (Windows registry toggle) ──────────────────────────

static DWORD g_originalBatteryPercent = MAXDWORD; // MAXDWORD = not yet saved

static constexpr LPCWSTR kAdvancedKey =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced";

static void ApplyBatteryPercent(int mode) {
    bool show = (mode > 0);
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kAdvancedKey, 0,
                      KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        Wh_Log(L"[Battery] Failed to open registry key");
        return;
    }
    // Save original value the first time we touch the key.
    if (g_originalBatteryPercent == MAXDWORD) {
        DWORD val = 0, sz = sizeof(val), type = 0;
        if (RegQueryValueExW(hKey, L"TaskbarBatteryPercent", nullptr, &type,
                             reinterpret_cast<BYTE*>(&val), &sz) == ERROR_SUCCESS)
            g_originalBatteryPercent = val;
        else
            g_originalBatteryPercent = 0;
    }
    DWORD val = show ? 1 : 0;
    RegSetValueExW(hKey, L"TaskbarBatteryPercent", 0, REG_DWORD,
                   reinterpret_cast<const BYTE*>(&val), sizeof(val));
    RegCloseKey(hKey);
    // Broadcast so explorer picks up the change on next restart.
    SendNotifyMessageW(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                       reinterpret_cast<LPARAM>(kAdvancedKey));
    Wh_Log(L"[Battery] TaskbarBatteryPercent set to %d (requires explorer restart)", val);
}

static void RestoreBatteryPercent() {
    if (g_originalBatteryPercent == MAXDWORD) return;
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kAdvancedKey, 0,
                      KEY_WRITE, &hKey) != ERROR_SUCCESS) return;
    RegSetValueExW(hKey, L"TaskbarBatteryPercent", 0, REG_DWORD,
                   reinterpret_cast<const BYTE*>(&g_originalBatteryPercent),
                   sizeof(g_originalBatteryPercent));
    RegCloseKey(hKey);
    SendNotifyMessageW(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                       reinterpret_cast<LPARAM>(kAdvancedKey));
    Wh_Log(L"[Battery] Restored TaskbarBatteryPercent to %lu", g_originalBatteryPercent);
    g_originalBatteryPercent = MAXDWORD;
}

// ── Layout application ────────────────────────────────────────────────────

static TextAlignment ClockTextAlignment() {
    switch (g_settings.clockAlignment) {
        case 0:  return TextAlignment::Left;
        case 2:  return TextAlignment::Right;
        default: return TextAlignment::Center;
    }
}

static HorizontalAlignment ClockHorizontalAlignment() {
    switch (g_settings.clockAlignment) {
        case 0:  return HorizontalAlignment::Left;
        case 2:  return HorizontalAlignment::Right;
        default: return HorizontalAlignment::Center;
    }
}

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
    sp.VerticalAlignment(VerticalAlignment::Center);
    sp.Spacing(g_settings.iconSpacing);

    if (g_omniButton) {
        auto ctrl = g_omniButton.try_as<Control>();
        if (ctrl) {
            ctrl.HorizontalContentAlignment(HorizontalAlignment::Center);
            ctrl.VerticalContentAlignment(VerticalAlignment::Center);
        }
    }

    // Size each icon slot explicitly so they render correctly in vertical layout.
    {
        int n = VisualTreeHelper::GetChildrenCount(sp);
        for (int i = 0; i < n; i++) {
            auto child = VisualTreeHelper::GetChild(sp, i).try_as<FrameworkElement>();
            if (child) {
                child.Width(32.0);
                child.Height(28.0);
                child.HorizontalAlignment(HorizontalAlignment::Center);
                auto cp = child.try_as<ContentPresenter>();
                if (cp) cp.HorizontalContentAlignment(HorizontalAlignment::Center);
            }
        }
    }

    // Always find battery presenter so settings changes can update its height later.
    {
        int n = VisualTreeHelper::GetChildrenCount(sp);
        for (int i = 0; i < n; i++) {
            auto child = VisualTreeHelper::GetChild(sp, i).try_as<FrameworkElement>();
            if (!child) continue;
            if (HasBatteryDescendant(child)) {
                g_batteryPresenter = child;
                Wh_Log(L"[Battery] Battery slot at index %d (mode=%d)", i, g_settings.batteryMode);
                if (g_settings.batteryMode == 1) {
                    // Inline: clear forced width so "79%" text isn't clipped.
                    child.ClearValue(FrameworkElement::WidthProperty());
                } else if (g_settings.batteryMode == 2) {
                    child.Height(120.0);
                    FlipBatteryLayout(child);
                    if (g_batteryInnerPanel) g_batteryInnerPanel.Spacing(0.0);
                }
                break;
            }
        }
        if (!g_batteryPresenter)
            Wh_Log(L"[Battery] No battery slot found (desktop without battery?)");
    }

    Wh_Log(L"[Layout] Applied vertical layout (children=%d, spacing=%d)",
           VisualTreeHelper::GetChildrenCount(sp), g_settings.iconSpacing);
}

// ── Clock three-row layout ────────────────────────────────────────────────

static bool IsClockParent(DependencyObject const& start) {
    DependencyObject cur = start;
    for (int depth = 0; depth < 16; depth++) {
        auto p = VisualTreeHelper::GetParent(cur);
        if (!p) break;
        auto pfe = p.try_as<FrameworkElement>();
        if (!pfe) { cur = p; continue; }
        auto cls = winrt::get_class_name(pfe);
        if (g_settings.debugLogging) {
            Wh_Log(L"[Clock] Parent[%d] class=%s", depth, cls.c_str());
        }
        std::wstring clsStr(cls.c_str());
        if (clsStr.find(L"Clock")    != std::wstring::npos ||
            clsStr.find(L"DateTime") != std::wstring::npos) {
            g_clockButton = pfe;
            return true;
        }
        cur = p;
    }
    return false;
}

// The outer clock StackPanel is already Vertical (time on row 1, "Day Date" on row 2).
// We make it three rows by forcing the date TextBlock (last child) to wrap at word
// boundaries with a narrow MaxWidth, splitting "Sunday 04/19/2026" onto two lines.
static void ApplyClockLayout(StackPanel const& sp) {
    if (g_clockDayDatePanel) return;
    if (sp.IsItemsHost()) return;
    int n = VisualTreeHelper::GetChildrenCount(sp);
    if (n < 2 || n > 4) return;
    if (!IsClockParent(sp)) return;

    g_clockDayDatePanel = sp;

    sp.Spacing(g_settings.clockLineSpacing);

    // First child is the time TextBlock ("3:39 PM").
    auto firstChild = VisualTreeHelper::GetChild(sp, 0).try_as<TextBlock>();
    if (firstChild) {
        firstChild.HorizontalAlignment(ClockHorizontalAlignment());
        firstChild.TextAlignment(ClockTextAlignment());
        g_clockTimeTextBlock = firstChild;
    }

    // Last child is the "DayOfWeek Date" TextBlock.
    auto lastChild = VisualTreeHelper::GetChild(sp, n - 1);
    auto dateTB = lastChild.try_as<TextBlock>();
    if (dateTB) {
        dateTB.TextWrapping(TextWrapping::WrapWholeWords);
        dateTB.MaxWidth(80.0);
        dateTB.HorizontalAlignment(ClockHorizontalAlignment());
        dateTB.TextAlignment(ClockTextAlignment());
        // NOTE: StackPanel::Spacing (time↔day) and TextBlock::LineHeight (day↔date)
        // use different measurement models. Spacing is pixel-exact; LineHeight includes
        // the font's internal leading/line-gap, so the same numeric value will look
        // visually tighter between day↔date than between time↔day. This is fundamental
        // and cannot be made perfectly uniform with these two properties.
        if (g_settings.clockLineSpacing > 0)
            dateTB.LineHeight(dateTB.FontSize() + g_settings.clockLineSpacing);
        else
            dateTB.ClearValue(TextBlock::LineHeightProperty());
        g_clockDateTextBlock = dateTB;
        Wh_Log(L"[Clock] Wrapping date TextBlock for three-row layout");
    } else {
        Wh_Log(L"[Clock] Last clock child is not a TextBlock (class=%s)",
               winrt::get_class_name(lastChild).c_str());
    }
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
        if (g_unloading) return S_OK;
        if (!g_settings.enableVertical && !g_settings.verticalClock) return S_OK;

        if (g_settings.debugLogging && element.Type) {
            Wh_Log(L"[TAP] Add: %s", element.Type);
        }

        auto insp = FromHandle(element.Handle);
        if (!insp) return S_OK;

        try {
            auto sp = insp.try_as<StackPanel>();
            if (sp) {
                if (!g_omniStackPanel) ApplyLayout(sp);
                if (!g_clockDayDatePanel && g_settings.verticalClock) ApplyClockLayout(sp);
                return S_OK;
            }

            // Size any new FrameworkElement added directly to the OmniButton StackPanel.
            // This fires when Windows adds/removes battery % slots live via WM_SETTINGCHANGE.
            if (g_omniStackPanel && g_settings.enableVertical) {
                auto fe = insp.try_as<FrameworkElement>();
                if (fe) {
                    auto parent = VisualTreeHelper::GetParent(fe);
                    if (parent) {
                        auto parentSP = parent.try_as<StackPanel>();
                        if (parentSP && parentSP == g_omniStackPanel) {
                            fe.Width(32.0);
                            fe.Height(28.0);
                            fe.HorizontalAlignment(HorizontalAlignment::Center);
                            auto cp = fe.try_as<ContentPresenter>();
                            if (cp) cp.HorizontalContentAlignment(HorizontalAlignment::Center);
                            Wh_Log(L"[Layout] Sized new OmniButton child: %s",
                                   winrt::get_class_name(fe).c_str());
                        }
                    }
                }
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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdll-attribute-on-redeclaration"
extern "C" __declspec(dllexport)
HRESULT WINAPI DllGetClassObject(GUID const& clsid, GUID const& iid, void** ppv) {
    Wh_Log(L"[TAP] DllGetClassObject called");
    if (!IsEqualGUID(clsid, CLSID_OmniButtonTAP))
        return CLASS_E_CLASSNOTAVAILABLE;
    try {
        return winrt::make_self<OmniButtonTAPFactory>()->QueryInterface(iid, ppv);
    } catch (...) { return winrt::to_hresult(); }
}
#pragma clang diagnostic pop

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
    if (SUCCEEDED(hr)) g_ixdeStarted = true;
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
    Wh_Log(L"[Init] Vertical OmniButton v1.29.0");
    LoadSettings();
    ApplyBatteryPercent(g_settings.batteryMode);

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
    // Only retry if IXDE was never successfully initiated (e.g., DLL wasn't loaded yet at init).
    if (!g_ixdeStarted) {
        HMODULE hXaml = GetModuleHandleW(L"Windows.UI.Xaml.dll");
        if (hXaml) {
            Wh_Log(L"[AfterInit] Retrying inject...");
            InjectOmniButtonTAP();
        }
    }
    Wh_Log(L"[AfterInit] ixdeStarted=%d watcher=%p",
           (int)g_ixdeStarted, (void*)g_visualTreeWatcher.get());
}

void Wh_ModUninit() {
    g_unloading = true;
    Wh_Log(L"[Uninit]");

    if (g_visualTreeWatcher) {
        g_visualTreeWatcher->Unadvise();
        g_visualTreeWatcher = nullptr;
    }

    RestoreBatteryPercent();

    auto sp     = g_omniStackPanel;
    auto btn    = g_omniButton;
    auto bp     = g_batteryPresenter;
    auto bip    = g_batteryInnerPanel;
    auto cdp    = g_clockDayDatePanel;
    auto timeTB = g_clockTimeTextBlock;
    auto dateTB = g_clockDateTextBlock;
    g_omniStackPanel     = nullptr;
    g_omniButton         = nullptr;
    g_batteryPresenter   = nullptr;
    g_batteryInnerPanel  = nullptr;
    g_clockDayDatePanel  = nullptr;
    g_clockButton        = nullptr;
    g_clockTimeTextBlock = nullptr;
    g_clockDateTextBlock = nullptr;

    // Helper lambda that does the actual XAML restoration.
    auto doCleanup = [sp, btn, bp, bip, cdp, timeTB, dateTB]() {
        try {
            if (sp) {
                sp.ClearValue(StackPanel::OrientationProperty());
                sp.ClearValue(StackPanel::SpacingProperty());
                sp.ClearValue(FrameworkElement::VerticalAlignmentProperty());
                int n = VisualTreeHelper::GetChildrenCount(sp);
                for (int i = 0; i < n; i++) {
                    auto child = VisualTreeHelper::GetChild(sp, i).try_as<FrameworkElement>();
                    if (child) {
                        child.ClearValue(FrameworkElement::WidthProperty());
                        child.ClearValue(FrameworkElement::HeightProperty());
                        child.ClearValue(FrameworkElement::HorizontalAlignmentProperty());
                        auto cp = child.try_as<ContentPresenter>();
                        if (cp) cp.ClearValue(ContentPresenter::HorizontalContentAlignmentProperty());
                    }
                }
            }
        } catch (...) {}
        try {
            if (btn) {
                btn.ClearValue(FrameworkElement::WidthProperty());
                auto ctrl = btn.try_as<Control>();
                if (ctrl) {
                    ctrl.ClearValue(Control::HorizontalContentAlignmentProperty());
                    ctrl.ClearValue(Control::VerticalContentAlignmentProperty());
                }
            }
        } catch (...) {}
        try { if (bp)  bp.ClearValue(FrameworkElement::HeightProperty()); } catch (...) {}
        try { if (bip) { bip.ClearValue(StackPanel::OrientationProperty()); bip.ClearValue(StackPanel::SpacingProperty()); } } catch (...) {}
        try { if (cdp) cdp.ClearValue(StackPanel::SpacingProperty()); } catch (...) {}
        try {
            if (timeTB) {
                timeTB.ClearValue(FrameworkElement::HorizontalAlignmentProperty());
                timeTB.ClearValue(TextBlock::TextAlignmentProperty());
            }
        } catch (...) {}
        try {
            if (dateTB) {
                dateTB.ClearValue(TextBlock::TextWrappingProperty());
                dateTB.ClearValue(FrameworkElement::MaxWidthProperty());
                dateTB.ClearValue(FrameworkElement::HorizontalAlignmentProperty());
                dateTB.ClearValue(TextBlock::TextAlignmentProperty());
                dateTB.ClearValue(TextBlock::LineHeightProperty());
            }
        } catch (...) {}
    };

    // Try synchronous first (works if uninit is already on the UI thread).
    doCleanup();

    // Also schedule async dispatch as belt-and-suspenders for wrong-thread cases.
    // Bump DLL refcount so the lambda is safe after Windhawk's FreeLibrary.
    // Need any XAML element to get the dispatcher.
    auto dispSrc = sp  ? sp.try_as<FrameworkElement>()
                      : (dateTB ? dateTB.try_as<FrameworkElement>() : nullptr);
    if (!dispSrc) return;
    auto disp = dispSrc.Dispatcher();
    if (!disp) return;

    HMODULE hSelf = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        reinterpret_cast<LPCWSTR>(&Wh_ModUninit), &hSelf);

    try {
        auto _ = disp.RunAsync(
            winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
            [doCleanup, hSelf]() {
                Wh_Log(L"[Uninit] Async cleanup running");
                doCleanup();
                Wh_Log(L"[Uninit] Async cleanup done");
                if (hSelf) FreeLibrary(hSelf);
            });
    } catch (...) {
        if (hSelf) FreeLibrary(hSelf);
    }
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Wh_Log(L"[Settings] Updated");

    // Battery % is a registry toggle — apply immediately on any thread.
    ApplyBatteryPercent(g_settings.batteryMode);

    // XAML properties must be set on the UI thread. Get a dispatcher from any live element.
    FrameworkElement dispSrc{ nullptr };
    if (g_omniStackPanel)     dispSrc = g_omniStackPanel.try_as<FrameworkElement>();
    if (!dispSrc && g_clockDateTextBlock) dispSrc = g_clockDateTextBlock.try_as<FrameworkElement>();
    if (!dispSrc) { Wh_Log(L"[Settings] No XAML elements yet — restart explorer to apply"); return; }
    auto disp = dispSrc.Dispatcher();
    if (!disp) return;

    // Capture all state needed on the UI thread.
    auto sp     = g_omniStackPanel;
    auto btn    = g_omniButton;
    auto bp     = g_batteryPresenter;
    auto bip    = g_batteryInnerPanel;
    auto cdp    = g_clockDayDatePanel;
    auto timeTB = g_clockTimeTextBlock;
    auto dateTB = g_clockDateTextBlock;
    bool enableV      = g_settings.enableVertical;
    bool enableC      = g_settings.verticalClock;
    int  batteryMode  = g_settings.batteryMode;
    int  iconSpc      = g_settings.iconSpacing;
    int  clockAlign   = g_settings.clockAlignment;
    int  clockSpacing = g_settings.clockLineSpacing;

    try {
        disp.RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
        [sp, btn, bp, bip, cdp, timeTB, dateTB,
         enableV, enableC, batteryMode, iconSpc, clockAlign, clockSpacing]() mutable
        {
            // OmniButton layout
            try {
                if (sp) {
                    if (enableV) {
                        sp.Orientation(Orientation::Vertical);
                        sp.VerticalAlignment(VerticalAlignment::Center);
                        sp.Spacing(iconSpc);
                        if (btn) {
                            auto ctrl = btn.try_as<Control>();
                            if (ctrl) {
                                ctrl.HorizontalContentAlignment(HorizontalAlignment::Center);
                                ctrl.VerticalContentAlignment(VerticalAlignment::Center);
                            }
                        }
                        {
                            int n = VisualTreeHelper::GetChildrenCount(sp);
                            for (int i = 0; i < n; i++) {
                                auto child = VisualTreeHelper::GetChild(sp, i).try_as<FrameworkElement>();
                                if (child) {
                                    child.Width(32.0);
                                    child.Height(28.0);
                                    child.HorizontalAlignment(HorizontalAlignment::Center);
                                    auto cp = child.try_as<ContentPresenter>();
                                    if (cp) cp.HorizontalContentAlignment(HorizontalAlignment::Center);
                                }
                            }
                        }
                        // Find battery slot if needed.
                        if (!bp && sp) {
                            int nc = VisualTreeHelper::GetChildrenCount(sp);
                            for (int i = 0; i < nc; i++) {
                                auto child = VisualTreeHelper::GetChild(sp, i).try_as<FrameworkElement>();
                                if (child && HasBatteryDescendant(child)) {
                                    bp = child;
                                    g_batteryPresenter = child;
                                    break;
                                }
                            }
                        }
                        if (bp) {
                            if (batteryMode == 1) {
                                // Inline: let battery slot auto-size so % text isn't clipped.
                                bp.ClearValue(FrameworkElement::WidthProperty());
                                bp.ClearValue(FrameworkElement::HeightProperty());
                            } else if (batteryMode == 2) {
                                bp.Height(120.0);
                                // Do NOT call FlipBatteryLayout here — the XAML tree only has
                                // the % structure after an explorer restart with registry=1.
                                // Flipping on a stale tree corrupts g_batteryInnerPanel.
                                // The flip already ran in ApplyLayout if the tree was correct.
                                if (bip) bip.Spacing(0.0);
                            } else {
                                bp.ClearValue(FrameworkElement::WidthProperty());
                                bp.ClearValue(FrameworkElement::HeightProperty());
                            }
                        }
                        if (bip && batteryMode != 2) {
                            bip.ClearValue(StackPanel::OrientationProperty());
                            bip.ClearValue(StackPanel::SpacingProperty());
                        }
                    } else {
                        sp.ClearValue(StackPanel::OrientationProperty());
                        sp.ClearValue(StackPanel::SpacingProperty());
                        sp.ClearValue(FrameworkElement::VerticalAlignmentProperty());
                        {
                            int n = VisualTreeHelper::GetChildrenCount(sp);
                            for (int i = 0; i < n; i++) {
                                auto child = VisualTreeHelper::GetChild(sp, i).try_as<FrameworkElement>();
                                if (child) {
                                    child.ClearValue(FrameworkElement::WidthProperty());
                                    child.ClearValue(FrameworkElement::HeightProperty());
                                    child.ClearValue(FrameworkElement::HorizontalAlignmentProperty());
                                    auto cp = child.try_as<ContentPresenter>();
                                    if (cp) cp.ClearValue(ContentPresenter::HorizontalContentAlignmentProperty());
                                }
                            }
                        }
                        if (btn) {
                            auto ctrl = btn.try_as<Control>();
                            if (ctrl) {
                                ctrl.ClearValue(Control::HorizontalContentAlignmentProperty());
                                ctrl.ClearValue(Control::VerticalContentAlignmentProperty());
                            }
                            btn.ClearValue(FrameworkElement::WidthProperty());
                        }
                        if (bp)  bp.ClearValue(FrameworkElement::HeightProperty());
                        if (bip) bip.ClearValue(StackPanel::OrientationProperty());
                    }
                }
            } catch (...) {}

            // Clock layout
            auto clockTA = clockAlign == 0 ? TextAlignment::Left
                         : clockAlign == 2 ? TextAlignment::Right
                         : TextAlignment::Center;
            auto clockHA = clockAlign == 0 ? HorizontalAlignment::Left
                         : clockAlign == 2 ? HorizontalAlignment::Right
                         : HorizontalAlignment::Center;
            try {
                if (enableC) {
                    if (cdp)    cdp.Spacing(clockSpacing);
                    if (timeTB) { timeTB.HorizontalAlignment(clockHA); timeTB.TextAlignment(clockTA); }
                    if (dateTB) {
                        dateTB.TextWrapping(TextWrapping::WrapWholeWords);
                        dateTB.MaxWidth(80.0);
                        dateTB.HorizontalAlignment(clockHA);
                        dateTB.TextAlignment(clockTA);
                        if (clockSpacing > 0)
                            dateTB.LineHeight(dateTB.FontSize() + clockSpacing);
                        else
                            dateTB.ClearValue(TextBlock::LineHeightProperty());
                    }
                } else {
                    if (cdp)    cdp.ClearValue(StackPanel::SpacingProperty());
                    if (timeTB) { timeTB.ClearValue(FrameworkElement::HorizontalAlignmentProperty()); timeTB.ClearValue(TextBlock::TextAlignmentProperty()); }
                    if (dateTB) {
                        dateTB.ClearValue(TextBlock::TextWrappingProperty());
                        dateTB.ClearValue(FrameworkElement::MaxWidthProperty());
                        dateTB.ClearValue(FrameworkElement::HorizontalAlignmentProperty());
                        dateTB.ClearValue(TextBlock::TextAlignmentProperty());
                        dateTB.ClearValue(TextBlock::LineHeightProperty());
                    }
                }
            } catch (...) {}
        });
    } catch (...) {}
}
