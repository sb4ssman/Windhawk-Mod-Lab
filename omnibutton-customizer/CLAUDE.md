# CLAUDE.md — OmniButton Customizer

## First: read the notes

Go to [_claude_notes/](_claude_notes/) and read:
1. [working_notes.md](_claude_notes/working_notes.md)
2. [work_log.md](_claude_notes/work_log.md)
3. [root_causes.md](_claude_notes/root_causes.md)

The old Vertical OmniButton history is useful background, but the active file is now
[omnibutton-customizer.wh.cpp](omnibutton-customizer.wh.cpp).

## Current shape

OmniButton Customizer is a generalized successor to Vertical OmniButton. It arranges
the Windows 11 system tray OmniButton items into a configurable grid:

- wifi
- volume
- battery
- battery percentage, when Windows exposes it

It uses `GetTaskbarXamlRoot` plus symbol hooks, not XAML Diagnostics.

## Key implementation points

- `IconView::IconView` hook triggers layout once system tray icons load.
- `GetSystemTrayModuleHandle()` supports `SystemTray.dll`, older `Taskbar.View.dll`,
  and `ExplorerExtensions.dll`.
- `ApplyLayout()` computes grid geometry and uses `RenderTransform` nudges.
- `LayoutUpdated` handles delayed battery/percent elements.
- `CleanupAndResetCurrentElements()` must restore transforms, sizing, padding, and
  layout event handlers on unload or settings change.

## Test checklist

After code changes, load in Windhawk and test:

1. Default 2x2 layout.
2. `gridColumns: 1`, `itemOrder: "wifi volume battery"`.
3. `gridColumns: 1`, `itemOrder: "wifi volume battery percent"`.
4. `fillOrder: columnFirst`.
5. A swapped order, e.g. `itemOrder: "volume wifi battery percent"`.
6. Per-item nudge settings.
7. Toggle mod off and confirm native OmniButton returns.
8. Restart Explorer and confirm layout reapplies.

## Submission posture

Do not treat this as a drop-in update to PR #3859 without discussion. PR #3859 is
for Vertical OmniButton. This file is a broader Customizer and should either get
its own PR or be intentionally scoped back down before updating the existing PR.
