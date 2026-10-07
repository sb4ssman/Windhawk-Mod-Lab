# Windhawk-Mod-Lab Tool: Taskbar Tree Dump — publish

State (2026-10-06): live-tested by the user (final identity set Oct 6:
`@id` `mod-tool-taskbar-tree-dump`, `@name` "Windhawk-Mod-Lab Tool: Taskbar
Tree Dump"). Fork branch `add-mod-tool-taskbar-tree-dump` (local, one commit
`76153d54` on `upstream/main`, one-file diff verified). NOT pushed; no PR.
The old local branch `add-mod-lab-taskbar-tree-dump` is stale — delete it.

Remaining before pushing: the user reinstalls the final lab file (it differs
from the tested build only by three display strings: dump header, JSON
`"tool"` field, init log line) and confirms. Then:

    cd t:/Github/sb4ssman/windhawk-mods
    git push -u origin add-mod-tool-taskbar-tree-dump

and from PowerShell (never Git Bash):

    gh pr create --repo ramensoftware/windhawk-mods --base main `
      --head sb4ssman:add-mod-tool-taskbar-tree-dump `
      --title "Add Windhawk-Mod-Lab Tool: Taskbar Tree Dump v1.0" --body-file <body below>

Then `/ai-review` once CI is green.

## PR description (draft)

A tool for mod authors. It writes the Windows 11 taskbar's XAML element tree to a text or JSON file — on load, whenever the taskbar moves to another edge, changes thickness or is rebuilt, and on demand — one line or one JSON object per element, with the layout facts a mod needs when it has to find or move something. It only reads; nothing on the taskbar changes.

### Highlights

- **Whole-tree snapshots that diff cleanly.** Each element records its type, `#Name`, actual size and position, explicit sizes, margin, padding, alignment, panel facts (StackPanel, Grid rows/columns and cells, WrapGrid item size, Canvas position), `RenderTransform`, collapsed visibility and opacity. Window handles and object addresses are left out, so two dumps diff line by line: bottom against side taskbar, before and after a Windows update, with and without another mod.
- **Visual states included.** Every visual state group and its current state is recorded — that is where Windows states the taskbar's edge (`DockingStates` on `RootGrid`), which is how the Mod Lab's own mods learned to read it.
- **Dumps when the taskbar settles.** A move between edges re-lays out the existing tree without `TrayUI::StartTaskbar`, so the tool polls once a second — the taskbar position setting, each taskbar window's rect, its XAML root and its size, plus a count of `TrayUI::StartTaskbar` rebuilds — and writes a dump only after that has held steady for two polls, so a move is captured settled rather than mid-animation.
- **Private by default.** Taskbar text includes window titles, so text content is off by default and each text element records only its character count.
- **Configurable output.** Folder (environment variables expanded, missing folders created), text or JSON, a single named subtree, and a label added to the file name.

### Testing

Live-tested on Windows 11 build 26300.9550 (26H2), 2560x1600 at 100%:

- Dump on load wrote one file about two seconds after enabling; text content hidden by default (character counts only).
- Moved the taskbar bottom → top → left → right → bottom: each move produced one settled dump (about 2 s apart), all reporting the same root element, so a move re-lays out the existing tree rather than rebuilding it.
- JSON output parses, with no duplicate keys; unnamed visual state groups are numbered `(unnamed N)`.
- Subtree (`ControlCenterButton`) restricted the dump to that element; Label added to the file name; Include text content showed the real text.

<!-- ⚠️ Please keep the template below intact and fill in the relevant sections. Any additional content can be placed above the template. -->

## Changelog

If this pull request updates an existing mod, describe the changes below:

* 

## Mod authorship

If this pull request introduces a new mod, please complete the section below.

This mod was created by:

- - [ ] The submitter, without AI assistance
- - [x] The submitter, with AI assistance
- - [x] Claude
- - [ ] ChatGPT
- - [ ] Gemini
- - [ ] Another AI (please specify): 
- - [ ] Other (please specify): 

Please select the options that best apply. Your selection does not affect the acceptance criteria, but it helps reviewers understand the context of the code and provide relevant feedback.
