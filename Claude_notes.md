# Claude Session Notes — vertical-omnibutton.wh.cpp

## Current Status: v1.21.0 — compile errors in FlipBatteryLayout, NOT YET FIXED

## Goal
Standalone Windhawk mod (`vertical-omnibutton.wh.cpp`) that rearranges the system tray
OmniButton (wifi/volume/battery) from horizontal to vertical. Effect previously achieved
with Windows 11 Taskbar Styler + style.yaml. **Only work on this file — not the second file.**

## What the mod does (v1.21.0)
Uses **XAML Diagnostics API** (`InitializeXamlDiagnosticsEx`) — the same mechanism as
the Windows 11 Taskbar Styler. When `Windows.UI.Xaml.dll` loads, inject a TAP COM class
that calls `AdviseVisualTreeChange`. In `OnVisualTreeChange`:
- Find OmniButton's inner `StackPanel (IsItemsHost=true)`, set `Orientation=Vertical`,
  `VerticalAlignment=Center`, `Spacing=iconSpacing`
- Set `HorizontalContentAlignment=Center` + `VerticalContentAlignment=Center` on OmniButton (as Control)
- For battery %: find battery ContentPresenter via `HasBatteryDescendant`, set `Height=50`,
  then attempt `FlipBatteryLayout` to flip inner horizontal StackPanel to Vertical
- For clock: find clock's outer StackPanel, set `Spacing`, wrap date TextBlock with
  `TextWrapping=WrapWholeWords` + `MaxWidth=80` to create 3 rows

## XAML tree structure
```
SystemTray.OmniButton (Name="ControlCenterButton")
  Grid
    ContentPresenter
      ItemsPresenter
        StackPanel (IsItemsHost=true)   ← set Orientation=Vertical here
          ContentPresenter              ← set Width/Height here (×3)
            SystemTray.IconView
```

## Properties set
- `StackPanel.Orientation = Vertical` (IsItemsHost StackPanel inside OmniButton)
- `StackPanel.VerticalAlignment = Center` (centers icon stack within button height)
- `StackPanel.Spacing = iconSpacing` (uniform gap between icons, default 4px)
- `Control.HorizontalContentAlignment = Center` on OmniButton
- `Control.VerticalContentAlignment = Center` on OmniButton
- `FrameworkElement.Height = 50` on battery ContentPresenter (when showBatteryPercent=true)
- `StackPanel.Orientation = Vertical` on battery IconView's inner StackPanel (4th row attempt)
- Registry: `HKCU\...\Explorer\Advanced\TaskbarBatteryPercent` DWORD (battery % toggle)
- Clock: `StackPanel.Spacing`, TextBlock `TextWrapping=WrapWholeWords`, `MaxWidth=80`,
  `HorizontalAlignment`, `TextAlignment`, `LineHeight`

## Why XAML Diagnostics (not other approaches)
- `Taskbar.View.dll` loads via NT loader internals, NOT through `LoadLibraryExW` or
  `LoadPackagedLibrary` — cannot be intercepted via those hooks
- `LoadPackagedLibrary` fails with `APPMODEL_ERROR_NO_PACKAGE` (15700) from Windhawk context
- Hooking `StackPanel::MeasureOverride` via PDB symbols: `pThis` is an adjusted pointer
  to a vtable subobject (multiple inheritance), NOT the IUnknown start → QI crash
- XAML Diagnostics avoids all of these: it's an official Windows API, works cross-process,
  and `GetIInspectableFromHandle` gives a proper WinRT interface pointer

## Architecture
- `DllGetClassObject` export → `OmniButtonTAPFactory` → `OmniButtonTAP` (IObjectWithSite)
- `OmniButtonTAP::SetSite` creates `VisualTreeWatcher` (IVisualTreeServiceCallback2)
- `VisualTreeWatcher` ctor spawns a thread to call `AdviseVisualTreeChange` (must be
  async — calling it synchronously from SetSite deadlocks)
- `FreeLibrary(self)` in SetSite balances the LoadLibrary that IXDE does on our DLL
- `LoadLibraryExW` hook on `kernelbase!LoadLibraryExW` catches `Windows.UI.Xaml.dll` load
- Also tries injection immediately at init and in `Wh_ModAfterInit` if DLL already loaded

## Compile errors fixed
- **v1.8.0**: `DllGetClassObject` declared `noexcept` but `combaseapi.h` declares it
  without `noexcept` → removed `noexcept` from definition

## Compile errors NOT YET FIXED (v1.21.0)
`FlipBatteryLayout` uses `std::function<bool(DependencyObject, int)>` for a recursive
lambda — two problems:
1. `#include <functional>` is missing → `std::function` undeclared
2. `DependencyObject` in the template argument is ambiguous — clang resolves it to the
   forward-declared struct in `Windows.UI.Xaml.Documents.0.h` (not a value type usable
   in a function signature at that point)

**Fix**: Replace the `std::function` recursive lambda with a static free function:
```cpp
static bool WalkBatteryTree(DependencyObject const& node, int depth) {
    if (depth > 5) return false;
    int n = VisualTreeHelper::GetChildrenCount(node);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(node, i);
        if (!child) continue;
        if (g_settings.debugLogging) { /* log */ }
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
static void FlipBatteryLayout(FrameworkElement const& batteryCP) {
    if (!WalkBatteryTree(batteryCP, 0))
        Wh_Log(L"[Battery4] No horizontal StackPanel found — enable debug logging to see tree");
}
```
The logging inside `WalkBatteryTree` can't reference the lambda capture anymore — it must
use `g_settings.debugLogging` as a global directly (fine since it's a static free function).

## Settings (v1.21.0)
```yaml
- enableVertical: true
- iconSpacing: 4         (extra px between icons, applied as StackPanel::Spacing)
- showBatteryPercent: false  (registry toggle + XAML 4th-row attempt)
- verticalClock: true
- clockAlignment: 1      (0=Left 1=Center 2=Right, rendered as $options dropdown)
- clockLineSpacing: 8
- debugLogging: false
```

## Globals (v1.21.0)
```cpp
static StackPanel       g_omniStackPanel
static FrameworkElement g_omniButton
static FrameworkElement g_batteryPresenter   // battery ContentPresenter (for Height)
static StackPanel       g_batteryInnerPanel  // battery's inner horiz StackPanel (flipped)
static StackPanel       g_clockDayDatePanel
static FrameworkElement g_clockButton
static TextBlock        g_clockTimeTextBlock
static TextBlock        g_clockDateTextBlock
static DWORD            g_originalBatteryPercent  // registry value saved before we write
```

## Battery % approach (v1.21.0)
Two-part:
1. **Registry**: `HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced\TaskbarBatteryPercent`
   DWORD=1 to show %. Saved before first write, restored on uninit. Requires explorer restart.
2. **XAML 4th row**: Find battery ContentPresenter via `HasBatteryDescendant`, set `Height=50`,
   then walk subtree with `WalkBatteryTree` looking for a horizontal (non-ItemsHost) StackPanel
   to flip Vertical. If the internal layout is a Grid rather than StackPanel, this won't work
   and we'll need the debug log output to identify what's actually there.

## Battery % internal structure (unknown)
`LogBatterySubtree` / `WalkBatteryTree` debug output has NOT yet been captured.
We do not know whether the battery IconView contains:
- a horizontal StackPanel (our flip will work), or
- a Grid (flip won't work — need different approach)
Must run with `showBatteryPercent=true` + `debugLogging=true` and capture the log to know.

## Clock alignment $options
v1.21.0 uses Windhawk `$options` YAML syntax to render clockAlignment as a dropdown:
```yaml
- clockAlignment: 1
  $options:
    - 0: Left
    - 1: Center
    - 2: Right
```
True "disabled when verticalClock=off" is not a Windhawk feature — documented in $description only.

## Previous approaches and why they failed
- **v1.4.0**: `LoadLibraryExW` with full path to `Taskbar.View.dll` → error 126 (packaged DLL)
- **v1.5.0**: `LoadPackagedLibrary` → error 15700 (no package identity in Windhawk context)
- **v1.6.0**: Hook `LoadPackagedLibrary` → hook never fires (DLL loads via NT loader directly)
- **v1.7.0**: Hook `StackPanel::MeasureOverride` via PDB → crash (pThis adjusted pointer problem)

## Expected success log sequence
```
[Init] Vertical OmniButton v1.8.0
[Init] Windows.UI.Xaml.dll at init: 0x0      <- 0x0 on fresh restart
[LoadLib] Windows.UI.Xaml.dll loaded — injecting TAP
[TAP] Injecting from: C:\...\vertical-omnibutton.dll
[TAP] InitializeXamlDiagnosticsEx result: 00000000
[TAP] DllGetClassObject called
[TAP] VisualTreeWatcher created
[TAP] AdviseVisualTreeChange: 00000000
[Layout] Applied vertical layout (children=3)
```

## If DllGetClassObject is never called
IXDE isn't loading our DLL. Possible causes:
1. The CLSID `{7E6D9C3A-2F1B-4A58-B3E7-1D5C8F9A2B6E}` conflicts with something
2. The `DllGetClassObject` export isn't visible (check with `dumpbin /exports`)
3. IXDE is finding and rejecting our DLL for some reason

## CLSID
`{7E6D9C3A-2F1B-4A58-B3E7-1D5C8F9A2B6E}` — arbitrarily chosen, should be unique

## Compiler options
`-lole32 -loleaut32 -lruntimeobject`

## Key references
- Windows 11 Taskbar Styler source: uses identical XAML Diagnostics pattern
- `style.yaml` in this repo: the reference for which properties/values to set
- `vertical-system-tray-icons.wh.cpp`: second file in repo — DO NOT WORK ON THIS
