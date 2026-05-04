# Windhawk Mod Development Notes

Last refreshed: 2026-05-02

## Core Model

- A Windhawk mod is a single C++ source file compiled into a dynamic library.
- The compiled mod is loaded into the context of the processes targeted by its metadata.
- The source file carries its own metadata, readme, settings YAML, and implementation.
- For Explorer/taskbar mods, assume code executes inside `explorer.exe` unless using the tool-mod pattern.

Source: https://github.com/ramensoftware/windhawk/wiki/Creating-a-new-mod

## Metadata Checklist

- `@id`: unique, only lowercase letters, digits, and hyphens.
- `@name`: user-visible name.
- `@description`: short catalog description.
- `@version`: must increase for published updates.
- `@author`: author name/nickname.
- `@github`: author profile; must match PR author for official submissions.
- `@include` / `@exclude`: target process list.
- `@architecture`: include if the mod is intentionally architecture-limited.
- `@compilerOptions`: libraries and compiler flags needed by the mod.

Submission note: new and update PRs to `ramensoftware/windhawk-mods` must be one file:
`mods/<mod-id>.wh.cpp`.

Source: https://github.com/ramensoftware/windhawk-mods

## Readme And Settings

- Put user docs in `// ==WindhawkModReadme==`.
- The readme supports Markdown.
- Embedded images are limited to approved domains, notably `raw.githubusercontent.com`.
- Put settings in `// ==WindhawkModSettings==` as YAML.
- Settings support booleans, numbers, strings, arrays, nested options, `$name`, `$description`, and `$options`.

## Lifecycle

Order for a normally-started target process:

1. `Wh_ModInit`
2. implicit hook application
3. `Wh_ModAfterInit`
4. waiting for events
5. `Wh_ModSettingsChanged` as needed
6. `Wh_ModBeforeUninit`
7. hooks removed
8. `Wh_ModUninit`

If the mod is loaded into an already-running process, lifecycle callbacks execute on the Windhawk engine thread. Treat unload and settings changes as cross-thread cleanup points.

Source: https://github.com/ramensoftware/windhawk/wiki/Mod-lifetime

## Hooking Rules

- Set normal hooks in `Wh_ModInit`.
- `Wh_SetFunctionHook` cannot be called after `Wh_ModBeforeUninit` returns.
- `Wh_ApplyHookOperations` is automatic after init and is slow; avoid relying on it repeatedly.
- If a hook is delayed until a DLL loads, hook `LoadLibraryExW` carefully and record whether the target DLL has been handled.

## Process Targeting

- Windhawk injects broadly by design so mods can load early enough for target processes.
- A mod still controls which processes it targets via `@include` / `@exclude`.
- Some system-critical or incompatible processes are excluded by default.
- Pattern-based targets can be ignored for predefined system processes unless explicitly targeted or advanced settings are changed.

Source: https://github.com/ramensoftware/windhawk/wiki/Injection-targets-and-critical-system-processes

## Tool-Mod Pattern

For mods that do not need to hook or run inside another process:

- Prefer a standalone tool-mod shape to avoid risking Explorer stability.
- Rename callbacks to `WhTool_ModInit`, `WhTool_ModSettingsChanged`, and `WhTool_ModUninit`.
- Target `windhawk.exe`, not no target.
- Use the official tool-mod launcher snippet from the wiki.

Source: https://github.com/ramensoftware/windhawk/wiki/Mods-as-tools:-Running-mods-in-a-dedicated-process

## Troubleshooting Workflow

When a mod does not work:

- Disable the mod.
- Open the mod's Advanced tab.
- Set Debug logging to detailed logs.
- Open log output.
- Enable the mod and reproduce.
- Save the log and restore logging to none.

An empty log can mean Windhawk failed to inject. Symbol download errors can mean Microsoft symbols are unavailable or blocked.

Source: https://github.com/ramensoftware/windhawk/wiki/Troubleshooting

## House Rules For Our Mods

- Prefer `Wh_Log` directly; it is already disabled by default.
- Avoid persistent OS changes such as registry writes unless the mod is explicitly a tool for that and clearly reverts state.
- Every thread, timer, event hook, registered callback, notification cookie, and XAML event revoker needs an unload path.
- Background retry loops must be stoppable before unload; no detached threads in Explorer mods.
- XAML changes should be reversible on `Wh_ModUninit` and settings changes.
- If touching taskbar XAML, marshal to the taskbar UI thread before reading or mutating live XAML elements.
- When injecting into a `Grid`, account for both `Grid.Column` and `Grid.ColumnSpan` during insert/remove, or hit testing and layout can break.

