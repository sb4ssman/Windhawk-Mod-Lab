Updates Taskbar Virtual Desktop Switcher to 2.1: clickable desktop buttons with grouped settings, a nestable Arrangement expression, custom labels/indicator symbols and fonts, native checked states, and hover previews. Renamed 1.x settings reset once; the README explains how to copy settings before updating.

Windows 11 supports native horizontal and side taskbars, literal written layouts and screen-axis offsets. The experimental Windows 10 backend uses native tray windows and clock-layout reservation, three tray positions, and compact columns-first automatic grids with an equally sized Task View cell. Windows 10 appearance and exhaustive lifecycle/edge coverage remain experimental limitations.

Latest reviewer findings are addressed: deferred nonblocking taskbar rebuild recovery, UI-thread settings loading, helper/destructor cleanup, clamped typography defaults, DPI-aware preview fonts, and bounded same-process caption reads. The shared edge watcher ignores length-only changes.

### Screenshots

![Default Windows 11 tray placement](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-vd-switcher/assets/simple3.png)

![Windows 11 native side-taskbar grid](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-vd-switcher/assets/win11-side-greek-grid.png)

![Experimental Windows 10 compact grid](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-vd-switcher/assets/win10-experimental-grid.png)

### Tested live

The author live-tested the exact Windows 11 candidate and gave it the seal of approval. The Windows 10 compact-grid checkpoint was separately live-tested and accepted for experimental use. Full local submission preflight, real-font native-layout regression and edge-watch regression passed.

## Changelog

<!-- changelog:start -->
* Added grouped settings and one nestable Arrangement expression; old renamed keys reset once
* Added native side-taskbar layouts and literal written arrangements
* Added experimental Windows 10 tray-window placement and compact automatic grids
* Hardened rebuild, settings, teardown and retry lifecycle handling
* Added DPI-aware preview fonts and bounded same-process caption reads
* Retained configurable labels/symbols/fonts, native checked states and hover previews
<!-- changelog:end -->

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
