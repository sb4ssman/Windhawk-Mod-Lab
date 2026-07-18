# Design Doc: Windhawk Virtual Desktop Switcher Buttons Mod

## Goal

A new standalone Windhawk mod (`taskbar-vd-switcher` or similar) that injects numbered
clickable buttons into the Windows 11 system tray taskbar — one per virtual desktop —
that switch directly to that desktop on click. The user wants placement options, with
the primary target being **between the clock and the Show Desktop button**.

---

## Local Environment

All reference mods are in: `t:\Github\sb4ssman\windhawk-mods\mods\`

Key source files to read before writing code:
- `taskbar-desktop-indicator.wh.cpp` — desktop detection, COM notification, registry reading
- `taskbar-empty-space-clicks.wh.cpp` — `SwitchVirtualDesktop()` at lines 2873–3073
- `taskbar-content-presenter-injector.wh.cpp` — XAML element injection pattern
- `windows-11-taskbar-styler.wh.cpp` — system tray XAML tree structure reference

The mod being developed lives in a new repo. The sibling mod `vertical-omnibutton.wh.cpp`
in `t:\Github\sb4ssman\Windhawk-Vertical-OmniButton\` is a good reference for
Windhawk mod boilerplate (lifecycle hooks, settings, GetTaskbarXamlRoot, etc.).

---

## System Tray XAML Structure

From `windows-11-taskbar-styler.wh.cpp`, the system tray elements left→right:

```
Grid#SystemTrayFrameGrid
  ├── SystemTray.StackListView#IconStack        (notification tray icons)
  ├── SystemTray.OmniButton#ControlCenterButton (wifi / volume / battery)
  ├── SystemTray.OmniButton#NotificationCenterButton  (clock + notification center)
  └── SystemTray.Stack#ShowDesktopStack         (thin show-desktop strip, far right)
```

`Grid#SystemTrayFrameGrid` is the direct parent of all tray elements. Injection = insert
a new child into this Grid (or its inner StackPanel/panel) at a chosen position index.

The clock's inner XAML (`DateTimeIconContent`):
```
SystemTray.OmniButton#NotificationCenterButton
  └── Grid > ContentPresenter > ItemsPresenter > StackPanel > ContentPresenter
        └── SystemTray.DateTimeIconContent
              └── Grid#ContainerGrid
                    └── StackPanel
                          ├── TextBlock#DateInnerTextBlock
                          └── TextBlock#TimeInnerTextBlock
```

---

## Injection Position Options (Settings)

Expose a `position` setting with these options:

| Value | Placement |
|---|---|
| `beforeIcons` | Left of notification icons (leftmost in tray) |
| `beforeOmni` | Left of OmniButton (wifi/vol/bat) |
| `beforeClock` | Between OmniButton and Clock |
| `afterClock` | **Between Clock and Show Desktop** ← user's preferred |
| `afterShowDesktop` | Right of Show Desktop (outside tray, edge of screen) |

Implementation: after finding `Grid#SystemTrayFrameGrid`, enumerate Children(),
find the target sibling by class name, and `Children().InsertAt(index, buttonPanel)`.

---

## Virtual Desktop Infrastructure

### Desktop Count + Current Desktop (from `taskbar-desktop-indicator.wh.cpp`)

**Registry paths** (read both; prefer session-specific):
```
HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\VirtualDesktops
HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\SessionInfo\{SessionId}\VirtualDesktops
  Value: VirtualDesktopIDs   (REG_BINARY — array of GUIDs, each 16 bytes)
  Value: CurrentVirtualDesktop  (REG_BINARY or REG_SZ — GUID of active desktop)
```

Desktop index = position of `CurrentVirtualDesktop` GUID in `VirtualDesktopIDs` array (0-based).
Desktop count = `VirtualDesktopIDs` byte length / 16.

### Live Change Notifications

```cpp
// CLSIDs / SIDs (from taskbar-desktop-indicator.wh.cpp lines 284–318)
const CLSID CLSID_ImmersiveShell = {
    0xc2f03a33, 0x21f5, 0x47fa,
    {0xb4, 0xbb, 0x15, 0x63, 0x62, 0xa2, 0xf2, 0x39}
};
const GUID SID_VirtualDesktopNotificationService = {
    0xa501fdec, 0x4a09, 0x464c,
    {0xae, 0x4e, 0x1b, 0x9c, 0x21, 0xb8, 0x49, 0x18}
};

MIDL_INTERFACE("0CD45E71-D927-4F15-8B0A-8FEF525337BF")
IVirtualDesktopNotificationService : public IUnknown {
    virtual HRESULT Register(IUnknown* pNotification, DWORD* pdwCookie) = 0;
    virtual HRESULT Unregister(DWORD dwCookie) = 0;
};
```

Registration flow:
1. `CoCreateInstance(CLSID_ImmersiveShell, ...) → IServiceProvider`
2. `pServiceProvider->QueryService(SID_VirtualDesktopNotificationService, ...) → IVirtualDesktopNotificationService`
3. Build a notification object with a dynamic vtable that handles `CurrentChanged` / `CurrentChangedWithMonitors`
4. `notificationService->Register(notificationObject, &cookie)`

On `CurrentChanged`: re-read registry for new current desktop → update button highlight.

### Switching (from `taskbar-empty-space-clicks.wh.cpp` lines 2873–3073)

The implementation uses **raw vtable indexing** (not keyboard simulation).
Vtable indices: `GetCurrentDesktop=6`, `GetDesktops=7`, `SwitchDesktop=9`.

**Build detection**: read `twinui.pcshell.dll` file version to get build number.

**Version-specific IIDs** (IVirtualDesktopManagerInternal / IVirtualDesktop):

| Windows build | IVirtualDesktopManagerInternal IID | IVirtualDesktop IID | usesHMonitor |
|---|---|---|---|
| 26100+ (24H2) | `53F5CA0B-158F-4124-900C-057158060B27` | `3F07F4BE-B107-441A-AF0F-39D82529072C` | false |
| 22621–26099 (23H2) | `A3175F2D-239C-4BD2-8AA0-EEBA8B0B138E` | `3F07F4BE-B107-441A-AF0F-39D82529072C` | false |
| 22000–22482 (Win11 early) | `B2F925B9-5A0F-4D2E-9F4D-2B1507593C10` | `536D3495-B208-4CC9-AE26-DE8111275BF8` | true |
| 20348–21999 (Server 2022) | `094AFE11-44F2-4BA0-976F-29A97E263EE0` | `62FDF88B-11CA-4AFB-8BD8-2296DFAE49E2` | true |
| <20348 (Win10) | `F31574D6-B682-4CDC-BD56-1827860ABEC6` | `FF72FFDD-BE7E-43FC-9C03-AD81681E88E4` | false |

**Also needed**: `CLSID_VirtualDesktopManagerInternal = {0xC5E0CDCA, 0x7B6E, 0x41B2, {0x9F, 0xC4, 0xD9, 0x39, 0x75, 0xCC, 0x46, 0x7B}}`

**SwitchToDesktop(int targetIndex) pseudocode** (adapt from empty-space-clicks):
```cpp
void SwitchToDesktop(int targetIndex) {
    // 1. Load twinui.pcshell.dll version → pick IIDs + usesHMonitor
    // 2. CoCreateInstance(CLSID_ImmersiveShell) → IServiceProvider
    // 3. QueryService(CLSID_VirtualDesktopManagerInternal, IID_IVirtualDesktopManagerInternal)
    // 4. Vtable[7]: GetDesktops(nullptr_or_hmonitor, &pDesktopArray)
    // 5. pDesktopArray->GetAt(targetIndex, IID_IVirtualDesktop, &pTarget)
    // 6. Vtable[9]: SwitchDesktop(nullptr_or_hmonitor, pTarget)
}
```
Key difference from the source: source takes `bool reverse` and steps ±1.
New version takes `int targetIndex` directly — skip steps finding current index.

---

## XAML Injection Pattern (from `taskbar-content-presenter-injector.wh.cpp`)

```cpp
// Find injection point in tray grid, insert a StackPanel of Buttons
auto trayGrid = /* find Grid#SystemTrayFrameGrid via GetTaskbarXamlRoot + traversal */;
auto panel = trayGrid.try_as<Controls::Panel>();

// Create a horizontal StackPanel to hold the buttons
Controls::StackPanel buttonBar;
buttonBar.Orientation(Controls::Orientation::Horizontal);
buttonBar.VerticalAlignment(VerticalAlignment::Center);
buttonBar.Name(L"VdSwitcherBar");

// Defer if not yet laid out:
if (panel.ActualWidth() == 0) {
    panel.SizeChanged([weakPanel, ...](auto, auto) {
        // inject once sized
    });
} else {
    // Find sibling index, InsertAt
    int insertIndex = FindSiblingIndex(panel, L"SystemTray.Stack") + 0; // before ShowDesktop
    panel.Children().InsertAt(insertIndex, buttonBar);
}
```

Hooks that trigger injection (same pattern as content-presenter-injector):
- Hook `DateTimeIconContent::OnApplyTemplate` (already done in desktop-indicator — reuse)
- Or hook `LoadLibraryExW` to catch `Taskbar.View.dll` load, then traverse

**Finding `Grid#SystemTrayFrameGrid`**: use `GetTaskbarXamlRoot` (same approach as
`vertical-omnibutton.wh.cpp` — hook `CTaskBand::GetTaskbarHost` + `TaskbarHost::FrameHeight`
from `taskbar.dll` symbols), then `FindChildRecursive` looking for Name == `"SystemTrayFrameGrid"`.

---

## Button Creation

```cpp
void RebuildButtons(Controls::StackPanel& bar, int desktopCount, int currentIndex) {
    bar.Children().Clear();
    for (int i = 0; i < desktopCount; i++) {
        Controls::Button btn;
        btn.Content(winrt::box_value(std::to_wstring(i + 1)));
        btn.Width(g_settings.buttonWidth);   // setting: default 20
        btn.Height(g_settings.buttonHeight); // setting: default 28
        btn.Padding({2, 0, 2, 0});
        btn.Margin({1, 0, 1, 0});

        // Highlight current
        if (i == currentIndex) {
            Media::SolidColorBrush highlight;
            highlight.Color(/* from setting */);
            btn.Background(highlight);
        }

        int capturedIndex = i;
        btn.Click([capturedIndex](auto, auto) {
            SwitchToDesktop(capturedIndex);
        });

        bar.Children().Append(btn);
    }
}
```

On `CurrentChanged` notification → call `RebuildButtons` (or just update backgrounds)
on the UI thread via `bar.Dispatcher().RunAsync(...)`.

On desktop added/removed → full `RebuildButtons`.

---

## Settings Schema (YAML)

```yaml
- position: "afterClock"
  $name: Button position in system tray
  $options:
    - "beforeIcons": "Left of notification icons"
    - "beforeOmni": "Left of OmniButton (wifi/vol/bat)"
    - "beforeClock": "Between OmniButton and Clock"
    - "afterClock": "Between Clock and Show Desktop"
    - "afterShowDesktop": "Right of Show Desktop"

- buttonWidth: 20
  $name: Button width (px)

- buttonHeight: 28
  $name: Button height (px)

- activeColor: "#4488FF"
  $name: Active desktop button color (hex)

- inactiveColor: ""
  $name: Inactive button color (hex, empty = system default)

- labelFormat: "number"
  $name: Button label
  $options:
    - "number": "1  2  3"
    - "dot": "●  ○  ○"
    - "custom": "Custom (set labels below)"

- customLabels: ""
  $name: Custom labels (comma-separated, e.g. "Home,Work,Media")
```

---

## Cleanup (Wh_ModUninit)

```cpp
// Remove injected panel from tray grid children
if (g_buttonBar && g_trayGrid) {
    auto panel = g_trayGrid.try_as<Controls::Panel>();
    uint32_t idx;
    if (panel.Children().IndexOf(g_buttonBar, idx))
        panel.Children().RemoveAt(idx);
}
// Unregister desktop change notifications
if (g_notificationService && g_notificationCookie)
    g_notificationService->Unregister(g_notificationCookie);
```

---

## Windhawk Boilerplate Reference

From `vertical-omnibutton.wh.cpp` in `t:\Github\sb4ssman\Windhawk-Vertical-OmniButton\`:

- `GetTaskbarXamlRoot(HWND)` — full implementation, hooks `taskbar.dll` symbols
  (`CTaskBand::GetTaskbarHost`, `TaskbarHost::FrameHeight`, `std::_Ref_count_base::_Decref`)
- `RunFromWindowThread(HWND, proc, param)` — marshal calls to the UI thread via `WH_CALLWNDPROC` hook
- `FindCurrentProcessTaskbarWnd()` — finds `Shell_TrayWnd` for current process
- `FindChildRecursive(element, predicate)` — recursive XAML tree search
- Background retry loop in `Wh_ModAfterInit` — retries `ApplyAllSettings` 5× at 2s intervals
  if XAML tree not ready (handles early-startup timing)
- `std::atomic<bool> g_unloading` — guards background thread against use-after-uninit

The `@compilerOptions` needed: `-lole32 -loleaut32 -lruntimeobject`

WinRT includes needed:
```cpp
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Input.h>   // for RoutedEventHandler / Click
```

---

## Known Risks

1. **vtable indices may shift** in future Windows builds. The twinui version-check pattern
   from empty-space-clicks handles this gracefully — add a new `else if` branch when a new
   build changes the ABI.

2. **Injection timing** — `Grid#SystemTrayFrameGrid` may not exist when the mod loads.
   Use the same LayoutUpdated / SizeChanged deferred injection pattern as the content-presenter
   injector, combined with the background retry loop.

3. **Desktop count can change at runtime** (user adds/removes desktops). The notification
   callbacks must handle `DesktopCreated` and `DesktopDestroyed` events, not just `CurrentChanged`,
   to keep the button count correct.

4. **Multi-monitor** — each monitor has its own taskbar. `Grid#SystemTrayFrameGrid` appears
   on each. The injection loop should handle multiple instances. The `usesHMonitor` flag in the
   COM vtable calls matters here for correct per-monitor current desktop state.

---

## Implementation Order

1. Boilerplate: mod header, settings struct, `GetTaskbarXamlRoot` (copy from vertical-omnibutton)
2. Desktop state: registry read for count + current; notification registration
3. `SwitchToDesktop(int index)`: adapt from empty-space-clicks `SwitchVirtualDesktop()`
4. Button bar creation: `RebuildButtons()` with WinRT Button elements
5. Injection: find `SystemTrayFrameGrid`, insert at position based on setting
6. Live update: hook notifications → dispatch `RebuildButtons` on UI thread
7. Cleanup: `Wh_ModUninit` removes bar and unregisters notifications
8. Settings: position, size, colors, label format
