# Mod identity — the authority

The `@id` and `@name` of every mod, as the user set them. **Agents do not
change these.** If a source file, note, branch or PR title disagrees with this
table, that other thing is wrong and is what gets corrected.

`@id` is also the filename the library expects: `mods/<id>.wh.cpp`.

| Lab folder | `@id` | `@name` |
|---|---|---|
| privacy-indicator-anchor | `privacy-indicator-anchor` | Privacy Indicator Anchor |
| omnibutton-customizer | `omnibutton-customizer` | OmniButton Customizer |
| taskbar-clock-spacer | `taskbar-clock-spacer` | Taskbar Clock Spacer |
| taskbar-folder-menus | `taskbar-folder-menus` | Taskbar Folder Menus |
| taskbar-vd-switcher | `taskbar-vd-switcher` | Taskbar Virtual Desktop Switcher |
| tray-utility-customizer | `tray-utility-customizer` | Tray Utility Customizer |
| taskmanager-tail | `task-manager-tail` | Task Manager Tail |
| tool-mods/ (flat file) | `mod-tool-taskbar-tree-dump` | Windhawk-Mod-Lab Tool: Taskbar Tree Dump |

Read from each mod's source on 2026-09-24. The tree-dump tool's identity was
set by the user on 2026-10-06; tools for mod authors live under `tool-mods/`
and take the `mod-tool-` `@id` prefix and the `Windhawk-Mod-Lab Tool:` name prefix (settled by the user Oct 6, after trying `mod-lab-` and `mod-lab-tool-`). One folder name does not match its
`@id`: `taskmanager-tail/` holds `task-manager-tail`. A folder name is
internal and carries no authority; the `@id` does. Never "fix" an `@id` to
match a folder — that is the mistake this file exists to prevent.

## Why this file exists

An agent renamed Privacy Indicator Anchor to "Tray Privacy Indicator Anchor"
(`tray-privacy-indicator-anchor`) without being asked. It reached PR #4843 on
2026-08-03 and stood for seven weeks, because every note written afterwards
recorded the wrong name as established fact, and each session read those notes
as truth. The user caught it on 2026-09-24.

The name was redundant on its face: the mod anchors privacy indicators, which
already live in the tray. That is the tell — a rename that "clarifies" is an
agent editing the user's product.

Two rules follow:

1. **Never edit identity.** Propose it in chat, with reasons, and wait.
2. **A note is not the authority.** When a note and this table disagree, this
   table wins and the note gets corrected.
