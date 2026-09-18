# Release candidate live tests — September 11, 2026

Test **VD Switcher and Folder Menus first**. Those have new features. Clock
Spacer and Tray Utility need a brief check of newer lab code against your
working installed builds; Privacy Anchor has a newer embedded Start-placement
helper. OmniButton is unchanged and recovered. Task Manager Tail is closed by
your Windows 10/11 confirmation and needs no further test.

The package's `candidates` directory contains complete Windhawk source files.
`installed-backups` contains the installed source and settings read before
packaging. `manifest.json` identifies the exact candidate hashes. Nothing in
the package installs itself or modifies your running mods.

## Load a candidate

For an existing local mod, use its Windhawk source editor, replace its source
with the matching candidate, then save/compile. For Folder Menus, make a local
mod from the candidate and disable the catalog copy while testing. Keep only
one copy of each mod enabled. Your backups preserve the installed originals.

Folder Menus and the older installed Tray Utility use different settings keys
from their new grouped settings. Save your textual settings before replacing
code; keep the exported settings JSON as an additional reference. These
backups include registry value types and are **reference files, not Windhawk
import files**. To roll back, restore the backed-up source and its previous
settings, or disable the Folder Menus test copy and re-enable its catalog copy.

## 1. VD Switcher — hover previews

File: [taskbar-vd-switcher.wh.cpp](candidates/taskbar-vd-switcher.wh.cpp)

- Put different visible windows on two desktops. Hover each desktop button
  for about 400 ms: the preview should identify that desktop and show its
  windows without switching desktops or stealing keyboard focus.
- Move off the button, move quickly between buttons, then click one. The
  preview should disappear appropriately and clicking should still switch.
- Try a desktop with no windows and one with a minimized window. Empty and
  minimized states should be understandable. Off-desktop, suspended or protected
  windows can have blank or last-rendered DWM images; this is an overview of
  available window thumbnails, not a screenshot of the desktop wallpaper.
- Change Behavior → Preview delay and Preview width, then turn Hover preview
  off/on. Off should restore ordinary tooltips. Check your usual tray/Start
  placement and secondary taskbar if you use one.
- Disable/re-enable once, including disabling while a preview is showing.
  No preview window should remain behind.

Existing VD screenshots are accepted; no replacement gallery is requested.

## 2. Folder Menus — grouped layout, native icons, after-app placement

File: [taskbar-folder-menus.wh.cpp](candidates/taskbar-folder-menus.wh.cpp)

The source keeps the published `0.7` version header until the PR commit, as
required by the lab's versioning rule. Its new settings format is the upcoming
2.0 format: old flat keys do not automatically migrate.

Your current six entries, in order, belong under **Content → Folders**:

| Label | Target |
|---|---|
| 🖥 | `shell:Desktop` |
| ⚙ | `shell:ControlPanelFolder` |
| GH | `T:\github\` |
| 📁 | `T:\storage` |
| C:\ | `C:\` |
| T:\ | `T:\` |

Keep Use native Shell icon off initially. To reproduce your current appearance:
Placement.Position = `beforeIcons`; Layout.Arrangement = `auto`; Size =
24×22, spacing 4; Surface.FontSize = 12; BackgroundColor and BorderColor =
`transparent`; HoverBackgroundColor = `accent`; PressedBackgroundColor =
`accentLight`; BorderThickness and CornerRadius = −1; Opacity = 100;
ShineEffect off. Menu limits stay 0 (unlimited), ShowHidden off.
Your old padding is left 0/right 2: new PadX = 1 and OffsetX = −1 approximate
the same reserved width and content position. PadY/OffsetY remain 0.

- Open Desktop, Control Panel and a drive. Check cascading subfolders,
  right-click Shell context menus, Open in Explorer, and hover recovery after
  dismissing a menu.
- Enable native icons for Desktop and Control Panel. They should replace the
  labels, remain crisp, and preserve tooltips/menu behavior. Turn one off again.
- Try `1, 2 | 3, 4 | 5, 6`, then `1 | 2 | 3 | 4 | 5 | 6`, then `auto`.
  Unlisted folders append by default; selecting Hide should hide unlisted ones.
- Select **After pinned/running app icons**. Open/close apps and check both
  center/left taskbar alignment if practical. The whole toolbar should follow
  the rendered app buttons and stay clear of the system tray. With insufficient
  room it should hide, then return when room is available.
- Return to Before notification icons, then disable/re-enable. No empty
  columns, duplicate buttons, lingering app spacing or blocked clicks should remain.

All placement is for one grouped toolbar. Per-folder taskbar positions are
deliberately absent. Existing screenshots remain in the README.

## 3. Brief checks for the other candidates

| Candidate | What needs confirming |
|---|---|
| [Clock Spacer](candidates/taskbar-clock-spacer.wh.cpp) | Your existing clock format, width and spacer tokens still distribute correctly; toggling the mod restores/reapplies normally. Your supplied screenshot is included. |
| [Tray Utility](candidates/tray-utility-customizer.wh.cpp) | The lab 2.0 build uses grouped settings. Set your preferred icons/layout and placement, confirm the icons open their native controls, and check restoration on disable. |
| [Privacy Anchor](candidates/privacy-indicator-anchor.wh.cpp) | Your current settings still behave correctly; if placed beside Start, opening/closing apps keeps placement stable. Preserve your chosen camera-detection setting. |

After the individual checks, one Explorer restart with the candidates enabled
checks taskbar reconstruction across the family. Save any Explorer-dependent
work first. Report the mod and exact symptom if anything fails; a screenshot
or log is useful for a failure, but another general screenshot collection is
not required.

## Validation and remaining release steps

All six relevant lab mods compile/link and pass the exit-time lifetime and
README checks. Embedded templates match their originals. Grouped settings
checks pass for VD, Folder Menus, Tray Utility, OmniButton and Privacy Anchor.
Clock Spacer deliberately retains its companion-specific settings format;
the existing settings audit says not to force it into the grid/button profile.

Upstream source preflight passes for VD, Clock, Tray Utility, OmniButton and
Privacy Anchor. Folder Menus' upstream validator reports the retained `0.7`
version as already published; bump to 2.0 on its PR commit after the live test.
No source has been pushed and no PR has been changed. A live confirmation must
refer to these exact candidate hashes before publication work resumes.
