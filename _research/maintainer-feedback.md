# Maintainer Feedback On sb4ssman's Mods

Last refreshed: 2026-05-02

This is the practical "do not repeat these mistakes" file.

## Cross-Mod Rules

- Do not use XAML Diagnostics for taskbar mods unless the mod truly needs it. There can be only one XAML Diagnostics consumer, so it conflicts with Windows 11 Taskbar Styler and similar mods.
- Prefer `GetTaskbarXamlRoot` / symbol-hook based taskbar access for Explorer taskbar XAML mods.
- Do not make persistent system changes for ordinary Windhawk mods. Windhawk mods should be safe to try and disabling them should revert changes.
- Do not leave detached threads or async callbacks running after unload. Stoppable thread handles/events are acceptable; detached sleeping lambdas are not.
- Reversible XAML cleanup matters. If the mod changes layout, width, height, transforms, orientation, column definitions, event callbacks, or visibility, it needs a reliable cleanup route.
- Use `Wh_Log` directly instead of adding a redundant logging setting.
- For mods that do not need injection, use the tool-mod pattern and target `windhawk.exe`.

## PR #3859 - Vertical OmniButton

PR: https://github.com/ramensoftware/windhawk-mods/pull/3859

Status as checked on 2026-05-02:

- State: open.
- Latest maintainer ask: apply the comments from PR #3932 too.
- Checks seen: PR validation and mod compatibility checks were passing on the latest visible run.

Maintainer notes:

- XAML Diagnostics conflict: m417z pointed out that there can only be one XAML Diagnostics consumer. If both this mod and Windows 11 Taskbar Styler try to consume it, one will miss notifications. Suggested direction: use `GetTaskbarXamlRoot` based mods as references, or consider whether Taskbar Styler configuration is enough.
  - Link: https://github.com/ramensoftware/windhawk-mods/pull/3859#discussion_r3129967175
- Persistent registry change: m417z objected to changing the battery percentage registry value because Windhawk mods are expected to make in-memory changes that disabling the mod reverts.
  - Link: https://github.com/ramensoftware/windhawk-mods/pull/3859#discussion_r3129973528
- Apply VD switcher review comments: m417z later noted the code is similar to PR #3932 and asked to apply those comments here too.
  - Link: https://github.com/ramensoftware/windhawk-mods/pull/3859#issuecomment-4363738738

Local follow-up done:

- Current lab copy uses `GetTaskbarXamlRoot` style access instead of XAML Diagnostics.
- Registry modification was removed from current lab copy.
- Detached retry thread pattern was replaced locally with a stoppable `CreateThread` plus stop event, and unload cleanup now runs synchronously on the taskbar UI thread.

## PR #3932 - Taskbar Virtual Desktop Switcher

PR: https://github.com/ramensoftware/windhawk-mods/pull/3932

Status as checked on 2026-05-02:

- State: open.
- Maintainer feedback visible: quick test found partial click hit area and Explorer crash on disable.
- Checks seen: PR validation and compatibility checks were passing on the latest visible run.

Maintainer notes:

- Quick test found two issues: sometimes only half of a button was clickable, and disabling the mod crashed Explorer.
  - Link: https://github.com/ramensoftware/windhawk-mods/pull/3932#issuecomment-4361837429
- Inline review on detached retry thread: m417z asked whether it was really necessary and warned it can crash if the mod is unloaded at that time.
  - Link: https://github.com/ramensoftware/windhawk-mods/pull/3932#discussion_r3175387557
- User feature request: Salyts suggested adding "Next to the Start button" and "Above the Start button" position options, especially for use with Hide Start Button.
  - Link: https://github.com/ramensoftware/windhawk-mods/pull/3932

Local follow-up done:

- Current lab copy fixes the click-area issue by preserving/widening `Grid.ColumnSpan` when inserting/removing the injected column.
- Current lab copy fixes unload crash risk by using a stoppable retry thread and synchronous cleanup.
- Current lab copy re-walks live XAML by name during cleanup to avoid stale WinRT proxy identity issues.
- Current lab copy has an experimental Start overlay implementation for `nextToStart` and `aboveStart`.

## PR #3045 - Task Manager Tail v1.0

PR: https://github.com/ramensoftware/windhawk-mods/pull/3045

Status:

- Merged on 2026-01-16.

Maintainer notes:

- Logging: m417z asked why not just use `Wh_Log` everywhere, since it is already disabled by default.
  - Link: https://github.com/ramensoftware/windhawk-mods/pull/3045#discussion_r2696287995
- Process model: m417z pointed to the tool-mod guide for mods that do not need injection.
  - Link: https://github.com/ramensoftware/windhawk-mods/pull/3045#discussion_r2696288581
- Tool-mod target detail: m417z noted that the guide says to change target from `explorer.exe` to `windhawk.exe`; having no target is not the same thing.
  - Link: https://github.com/ramensoftware/windhawk-mods/pull/3045#discussion_r2696425559

Rule extracted:

- Start with `Wh_Log`.
- Before injecting into Explorer, ask whether the mod can be a tool mod.
- If using the tool-mod pattern, follow the wiki exactly and use `@include windhawk.exe`.

## PR #3143 - Task Manager Tail Windows 10/11 Update

PR: https://github.com/ramensoftware/windhawk-mods/pull/3143

Status:

- Merged on 2026-01-26.
- No maintainer comments were visible in the PR data checked on 2026-05-02.
