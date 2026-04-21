# CLAUDE.md — Vertical OmniButton Windhawk Mod

## First: read the notes

Go to [_claude_notes/](_claude_notes/) and read:
1. [working_notes.md](_claude_notes/working_notes.md) — current goals, active problems, key facts
2. [work_log.md](_claude_notes/work_log.md) — completed work by version (orient yourself here)
3. [Claude_notes.md](_claude_notes/Claude_notes.md) — architecture, XAML tree, why certain approaches were chosen
4. [root_causes.md](_claude_notes/root_causes.md) — confirmed bug root causes and how they were fixed

Keep notes up to date: move completed items from working_notes to work_log. Keep working_notes short.

## The project

Single file: [vertical-omnibutton.wh.cpp](vertical-omnibutton.wh.cpp)

A Windhawk mod for Windows 11 that rearranges the system tray OmniButton (wifi/volume/battery)
from horizontal to vertical stacking using the Windows XAML Diagnostics API.

**Do not touch** `vertical-system-tray-icons.wh.cpp` — that is a different, separate mod.

## Current version: 1.43.0

## Windhawk mod basics

- `Wh_ModInit` — called when mod loads into explorer.exe
- `Wh_ModAfterInit` — called after hooks are applied
- `Wh_ModUninit` — called when mod unloads; must restore XAML to native state
- `Wh_ModSettingsChanged` — called when user saves settings in Windhawk UI
- `Wh_GetIntSetting` / `Wh_GetStringSetting` / `Wh_FreeStringSetting` — settings access
- `Wh_Log(L"...")` — debug logging (shown in Windhawk log panel)
- Settings declared in `// ==WindhawkModSettings==` YAML block at top of file

## How the mod works

Uses `InitializeXamlDiagnosticsEx` to inject a TAP COM class (`OmniButtonTAP`) into
explorer's XAML diagnostics layer. The TAP registers `VisualTreeWatcher` via
`AdviseVisualTreeChange`, which fires `OnVisualTreeChange` for every element added/removed.

Key callbacks:
- `ApplyLayout` — fires when `IsItemsHost` StackPanel is found; sets Vertical orientation
- `ApplyClockLayout` — fires on clock's outer StackPanel
- Deferred handlers in `OnVisualTreeChange` — handle late-arriving battery elements

## Critical: two-path problem

The IsItemsHost StackPanel often arrives with 0 children. `ApplyLayout` runs but finds nothing
to configure for battery. The battery ContentPresenter arrives later as a NEW Add event.
The "Size new FE" handler at the bottom of `OnVisualTreeChange` must handle the battery CP
correctly — it must NOT apply Width=32/Height=28 to the battery slot when battery % is enabled.
This was the root cause of failures in v1.36–v1.41.

## Testing

Load the compiled mod in Windhawk. After any code change, Windhawk recompiles automatically.
Test sequence for each battery mode:
1. Set mode, enable "Restart explorer.exe", save → wait for restart
2. Verify display after restart (not live-switch — that almost always works)
3. Change a non-battery setting (e.g. iconSpacing) and verify mode doesn't change
