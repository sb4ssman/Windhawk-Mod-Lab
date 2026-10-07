# Mod-Lab: Taskbar Tree Dump — live test, then publish

State (2026-10-06): v1.0 has NEVER been run. Windhawk still has the old v0.1
installed as `local@lab-taskbar-tree-dump`. The PR branch
`add-mod-lab-taskbar-tree-dump` exists ONLY in the local fork checkout
(`t:/Github/sb4ssman/windhawk-mods`, one commit on `upstream/main`, one-file
diff verified). Not pushed. No PR.

## Live test (user)

1. In Windhawk, disable and delete the old "Lab: Taskbar Tree Dump".
2. Create a new mod, paste
   `tool-mods/mod-lab-taskbar-tree-dump/mod-lab-taskbar-tree-dump.wh.cpp`,
   compile and enable.
3. About two seconds later a file appears in
   `Documents\Taskbar Tree Dumps`. Open it: the header shows the build, the
   taskbar position and DPI; task-button text shows only `textChars=N`, never
   a window title.
4. Move the taskbar to another edge. A new file with that edge and `change` in
   its name appears once the taskbar settles.
5. Set Format to JSON and Label to `test`: a new `.json` file with `test` in
   its name appears and opens as valid JSON.
6. Set Subtree to `SystemTrayFrameGrid`: the next dump contains only that
   subtree.
7. Disable the mod: no error in the log, Explorer unaffected.

## After the user confirms

In the fork: `git switch add-mod-lab-taskbar-tree-dump`, re-copy the lab file
if it changed, verify `git diff --name-only upstream/main...HEAD` prints one
path, push, then from PowerShell:

    gh pr create --repo ramensoftware/windhawk-mods --base main `
      --head sb4ssman:add-mod-lab-taskbar-tree-dump `
      --title "Add Mod-Lab: Taskbar Tree Dump v1.0" --body-file <body below>

Fill in the Testing paragraph from what the user actually ran.

## PR description (draft)

A tool for mod authors. It writes the Windows 11 taskbar's XAML element tree to a text or JSON file — on load, whenever the taskbar moves to another edge, changes thickness or is rebuilt, and on demand — one line or one JSON object per element, with the layout facts a mod needs when it has to find or move something. It only reads; nothing on the taskbar changes.

### Highlights

- **Whole-tree snapshots that diff cleanly.** Each element records its type, `#Name`, actual size and position, explicit sizes, margin, padding, alignment, panel facts (StackPanel, Grid rows/columns and cells, WrapGrid item size, Canvas position), `RenderTransform`, collapsed visibility and opacity. Window handles and object addresses are left out, so two dumps diff line by line: bottom against side taskbar, before and after a Windows update, with and without another mod.
- **Visual states included.** Every visual state group and its current state is recorded — that is where Windows states the taskbar's edge (`DockingStates` on `RootGrid`), which is how the Mod Lab's own mods learned to read it.
- **Dumps when the taskbar settles.** A move between edges re-lays out the existing tree without `TrayUI::StartTaskbar`, so the tool polls once a second — the taskbar position setting, each taskbar window's rect, its XAML root and its size, plus a count of `TrayUI::StartTaskbar` rebuilds — and writes a dump only after that has held steady for two polls, so a move is captured settled rather than mid-animation.
- **Private by default.** Taskbar text includes window titles, so text content is off by default and each text element records only its character count.
- **Configurable output.** Folder (environment variables expanded, missing folders created), text or JSON, a single named subtree, and a label added to the file name.

### Testing

(Fill in after the live test.)

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
