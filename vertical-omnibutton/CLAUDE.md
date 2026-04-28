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
from horizontal to vertical stacking using Taskbar.View.dll symbol hooks and GetTaskbarXamlRoot.

**Do not touch** `vertical-system-tray-icons.wh.cpp` — that is a different, separate mod.

## Current version: 1.2

## Windhawk mod basics

- `Wh_ModInit` — called when mod loads into explorer.exe
- `Wh_ModAfterInit` — called after hooks are applied
- `Wh_ModUninit` — called when mod unloads; must restore XAML to native state
- `Wh_ModSettingsChanged` — called when user saves settings in Windhawk UI
- `Wh_GetIntSetting` / `Wh_GetStringSetting` / `Wh_FreeStringSetting` — settings access
- `Wh_Log(L"...")` — debug logging (shown in Windhawk log panel)
- Settings declared in `// ==WindhawkModSettings==` YAML block at top of file

## How the mod works

Hooks `winrt::SystemTray::implementation::IconView::IconView(void)` from `Taskbar.View.dll`.
When any system tray icon (IconView) loads, the mod calls `ApplyAllSettings()` if the
OmniButton layout hasn't been applied yet. `ApplyAllSettings` uses `GetTaskbarXamlRoot`
(via taskbar.dll symbol hooks) to get the XAML root, then traverses down to find the
OmniButton (by Name="ControlCenterButton").

Key functions:
- `ApplyAllSettings` — entry point; calls GetTaskbarXamlRoot, traverses to OmniButton
- `ApplyLayout(sp)` — sets Vertical orientation on the IsItemsHost StackPanel
- `OnLayoutUpdated` — handles deferred battery elements via LayoutUpdated event on SP
- `CleanupXamlElements` — restores all modified XAML properties; called by uninit and settings-changed

## Two-path problem (now handled via LayoutUpdated)

The IsItemsHost StackPanel sometimes arrives before battery CP is populated.
`OnLayoutUpdated` is registered on the StackPanel and retries finding the battery slot
and inner panel each layout pass, unregistering itself once complete.

## Testing

Load the compiled mod in Windhawk. After any code change, Windhawk recompiles automatically.
Test sequence for each battery mode:
1. Set mode, save → verify live (no restart needed)
2. Toggle mod off → OmniButton returns to horizontal, no residual transforms
3. Change an offset setting → icon moves, no "draw over" artifact
