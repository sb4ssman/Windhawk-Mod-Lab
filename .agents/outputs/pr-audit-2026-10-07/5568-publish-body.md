Adds compact Windows 11 taskbar buttons that open configured Shell targets as native popup menus, recreating the most useful part of the classic taskbar toolbar workflow. Subfolders expand on hover, right-click gives the full classic Shell context menu, and every folder popup carries an "Open in Explorer" shortcut.

### Fixes #5530 — the mod stopped loading on 26200.9457

Windows 11 26200.9457 (KB5129195) kept the element name `SystemTrayFrameGrid` but **changed it from a `Grid` to a `StackPanel`**. Every `try_as<Grid>()` on it returned `nullptr`, so the mod loaded, resolved its symbols, applied its hook, reached the XamlRoot — and then silently injected nothing. The reporter's debug log dies on exactly that line.

The fix does not switch types; it supports both, because older builds still ship a `Grid` and the rollout is staged:

- The injection point is resolved to the `Panel` base and **classified on every attempt, never cached** — `StartTaskbar` rebuilds the tree.
- A `Grid` leases a **column**, as before. A `StackPanel` leases a **child index**, because there child order *is* layout and there is no column to own.
- Anything that is neither is refused and logged with its real class name rather than guessed at.
- When an anchor cannot be resolved, the log now prints the tray panel's class and its named children, so a future restructure is diagnosable from one run.
- Column collision detection and repair stand down on an ordered panel, where there are no columns to collide.

The same change was independently reported against two other mods on 26H2 build 26300 (#5049, #5050) and fixed the same way in `taskbar-ai-quota` v1.6.5, so this is the forward shape of the tray rather than a one-off regression.

### Also in 2.1

- **Grouped settings contract** — `Placement` / `Content` / `Layout` / `Size` / `Adjust` / `Surface` / `Behavior`, matching the rest of this family. Windhawk cannot carry values across renamed keys, so old flat keys are **not migrated**; the README documents the upgrade explicitly.
- **One `Layout.Arrangement` expression** replaces the row/column and padding settings. `1 | 2` is a row, `1, 2` a column, parentheses nest, and `name[dx,dy]` nudges one item. The default `auto` fits the taskbar height and logs the expression it generated, so it can be pasted back into the same field and edited.
- **Native Shell icons** per entry, cached as pixels at the display size, with the label as fallback when an icon cannot be obtained.
- **Placement after the pinned/running app icons**, following them as apps open and close. It reserves space through the repeater margin and stops at the tray's left edge; if the taskbar cannot fit the toolbar it hides rather than overlapping the tray.
- Menu items are limited by optional per-folder item caps and submenu depth, and duplicate entries from the Desktop namespace's user+public merge are suppressed.

Native-icon and after-app-icon feature ideas were contributed by [diegoalejo15](https://github.com/diegoalejo15).

### Screenshots

![Two folder buttons in the system tray](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-folder-menus/assets/desktop-controlpanel.png)
*A minimal two-button setup for Desktop and Control Panel.*

![Four folder buttons on a standard taskbar](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-folder-menus/assets/c-github-desktop-controlpanel.png)
*Drive, GitHub, Desktop and Control Panel arranged in a row.*

![Four folder buttons on a taller taskbar](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-folder-menus/assets/c-github-desktop-controlpanel-v.png)
*The same four on a double-height taskbar, arranged by `auto`.*

![Control Panel opened as a native Shell menu](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-folder-menus/assets/controlpanel-menu-open.png)
*The Control Panel namespace opens as a native Shell menu with full icons.*

![Whole drive opened from a taskbar folder button](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-folder-menus/assets/whole-drive-on-taskbar.png)
*A drive-root button opens the whole drive as a native cascading menu.*

![Folder button destination tooltip](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-folder-menus/assets/tooltip-shows-destination.png)
*Hovering a compact button shows its configured Shell target.*

### Tested live

Tested on Windows 11 26200.9457 — the build this fixes — covering injection at each tray position and after the app icons, automatic and hand-written arrangements, native icons and label fallback, nested submenu expansion, the right-click Shell context menu, "Open in Explorer", settings changes, Explorer restart and taskbar rebuild, and disable → native restore → re-enable.


The author also live-tested the exact updated 2.1 candidate and gave it the seal of approval. All latest review findings are addressed; full local preflight and edge-watch regression pass. Existing screenshots retained.

## Changelog

* Added native side-taskbar layouts with literal manual arrangements
* Made taskbar construction wake the icon worker without joining it
* Added cancellation checks between targets and cached every failed icon target until settings reload
* Published settings on the taskbar thread and filtered length-only edge events
* Cleaned up unused helpers, initialization handles and comments

## Mod authorship

If this pull request introduces a new mod, please complete the section below.

This mod was created by:

- - [ ] The submitter, without AI assistance
- - [x] The submitter, with AI assistance
- - [x] Claude
- - [x] ChatGPT
- - [ ] Gemini
- - [ ] Another AI (please specify):
- - [ ] Other (please specify):

Please select the options that best apply. Your selection does not affect the acceptance criteria, but it helps reviewers understand the context of the code and provide relevant feedback.
