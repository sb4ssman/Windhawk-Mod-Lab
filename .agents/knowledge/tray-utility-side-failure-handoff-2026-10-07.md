# Tray Utility side-taskbar failure — checkpoint handoff

## User direction and boundaries

User requested a checkpoint and handoff after repeated failed live tests.
Stop implementation here. This is a failed local candidate, not an approved
release. No push, publication, PR changes, or installed-mod changes authorized
by this checkpoint request. Do not modify any other mod. User workflow is:
fix, exact-build human live approval, publish, then stop touching that mod.
Read the repo agent guide, working notes and work log before continuing.

## Reproduction and latest observed result

Top/bottom taskbars work. Left/right taskbars do not. User toggled off/on;
latest result is only emoji visible. Manual arrangements and per-item nudges
did not recover touch keyboard or overflow. No side candidate has passed.

Settings: Placement.Position=overflow; all six Content toggles=1;
Layout.Arrangement=`touchKeyboard | emoji | overflow`; FillOrder=rows;
Justify=center; NewItems=append; ItemWidth/ItemHeight/ItemSpacing=0;
PadX/PadY/OffsetX/OffsetY=0; MinimumTrayHeight=44; Detection=auto.
User also tried `auto` without success.

User reported an unidentified Windows publisher/signature block during one
compilation attempt. Exact executable and message are unknown. Do not assume
this is caused by XAML layout changes or bypass any Windows protection.

## Candidate history

1. Corrected margin compensation to follow each native StackPanel or
   ItemsStackPanel orientation with independent flow cursors. Live failed.
2. Corrected default item sizing to use along-taskbar dimensions rather than
   stretched side-host width. Live failed.
3. Resized native WrapGrid cells and translated enclosing ContentPresenters.
   Rejected; OmniButton source already documented native-cell resizing hiding
   slots. This scheme has been removed from the current source.
4. Current checkpoint adds PlaceNativeSideItems: preserves native cells,
   sizes the outer host, measures first visible nonempty TextBlock (control
   fallback), and translates item controls to requested target centers.
   Side path bypasses old icon size/margin writes. Live failed: only emoji.

Version header remains 2.1; no version bump or approval tag.

## Evidence and confirmed code defects

- Earlier targeted debug log discovered all three controls and applied both
  manual and auto expressions. Native reported host/icon sizes were zero.
  This log predates the newest candidate; it does not prove its failure stage.
- Earlier read-only check verified installed source matched the then-current
  candidate, Explorer DLL matched mod registry, mod enabled, and saved settings
  matched reproduction. It does not establish newest installed-source equality.
- Existing ignored native tree captures show side NonActivatableStack contains
  StackListView/ItemsPresenter/WrapGrid with fixed80x38 cells; top has a
  horizontal StackPanel and32x48 presenters. Treat captures as historical.
- DEFINITE READINESS BUG: ApplyLayout sets g_layoutApplied=true before host
  reparenting and side positioning. LayoutIsApplied returns g_layoutApplied
  or g_stoodDown. PlaceNativeSideItems can return false for zero measurements
  after mutations, without rollback. Retry can therefore stop on incomplete
  work. Prior statements that false reliably retries were incorrect.
- DEFINITE STRUCTURAL DIFFERENCE: OmniButton measures inside its existing
  native host. Tray moves several hosts into a fixed-size owned Grid before
  measuring. Copying center/translation math did not preserve that context.
- Current side helper iterates `items`, while straggler targets are appended
  separately; unidentified carried controls do not get positioned there.
- Whether clipping, stale measurements, host parenting, transforms, or another
  mechanism causes the latest missing icons has NOT been established live.
  Do not present these as proven causes without fresh geometry/log evidence.

## Relevant code and next investigation

- tray-utility-customizer/tray-utility-customizer.wh.cpp:
  ResolveLayoutItems, NativeItemSize, PlaceNativeSideItems, ApplyLayout,
  RestoreLayout, LayoutIsApplied, ApplyLayoutOnWindowThread.
- omnibutton-customizer/omnibutton-customizer.wh.cpp:
  ApplyItemsHostFootprint, VisualCenter, CenterOn, ApplySideTaskbarLayout.
  Its comments document the prior live failure from reshaping WrapGrid.

First separate tree ownership/restoration from successful completion so failure
does not leave a partial layout marked complete. Examine measurement timing
before/after reparenting and the full clipping/size chain. Obtain focused
per-control host/glyph bounds and applied transform evidence for the installed
candidate. Do not offer another speculative geometry patch as a verified fix.

## Validation limits

Compile/link passes via `_templates/compile-check.ps1 tray-utility-customizer`.
`.agents/tools/test-tray-native-flow.py` extracts actual native-size and stack
margin helpers; passes both axes and order/nudge cases. It DOES NOT test WinRT
rendering, clipping, reparenting, or current side helper. No full submission
preflight or successful live side test. Raw logs and tree dumps are not committed.

## Repository state at checkpoint

Starting lab HEAD: c64ba48. Checkpoint commits only Tray source, its regression
script, root catalog status and agent handoff/notes. No remote operations.
Unrelated preexisting edits deliberately remain outside this checkpoint:

- _templates/components/start-lane-placement/body.h
- privacy-indicator-anchor/privacy-indicator-anchor.wh.cpp
- taskbar-folder-menus/taskbar-folder-menus.wh.cpp
- untracked .agents/outputs/pr-audit-2026-10-07/fix-settings-review.py

These were previous unapproved edits; do not assemble them into Tray or publish
them. Preserve them until the user directs their disposition.

Latest prior PR audit (historical, not refreshed for this handoff): Tray #5569
has an incorrectly retained waiting-for-reviewer label, with a visible withdrawal
comment explaining no human approval. Submitted source differs from local.
Other PRs/branches were not touched during the Tray fixes. See work log for
the wider publication audit; verify GitHub before making current-status claims.

User requested the original top-taskbar screenshot at the top of both READMEs.
It was not available as a local image file; still outstanding. Do not substitute
the latest failure screenshot or manufacture a replacement.
