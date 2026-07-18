# Working Notes — Windhawk Mod Lab

Living todo list — current state only, pruned every session. Completed work
(past tense) goes in [work_log.md](work_log.md). Durable rules live in
[README.md](README.md); reference material in [knowledge/](knowledge/).

## Current focus

Five mods to prepare. No submissions until each works as described with
current docs/screenshots (standing directives in README.md).

1. **Clock Spacer** — STALEMATE (PR #4443 / integration PR #68, see work log
   2026-06-21). No action planned.

2. **Taskbar VD Switcher** — v1.7 on PR #4516, CI green, awaiting merge.
   - [ ] Verify the full-rebuild change also fixes issue #4784 (hover/hit-test)
   - [ ] Test the experimental `multiMonitor` toggle (deferred: Explorer
         restarts disrupt the user's desktop order)
   - [ ] Refresh screenshots (accent default changed visuals)
   - [ ] Sync folder README settings table

3. **Taskbar Folder Menus** — v0.6 on PR #4485, CI green, awaiting m417z
   review. Retest rule: Windhawk log window CLOSED (open log masks the
   teardown race).

4. **OmniButton Customizer** — BROKEN. Test round 3 failed 2026-07-17 (work
   log): battery percent still wrong; independent mode hides the battery glyph
   entirely. Grid template not implicated per user.
   - [ ] FIRST: get Windhawk logs from the user's machine (`[Battery]`,
         `[Layout]` lines) — no more static guessing
   - [ ] Fix coupled percent + independent mode from that evidence
   - [ ] Then grid retest (dual + single height), shortGroupPosition, colors
   - [ ] Fresh screenshots (current ones are vertical-era), then PR
   - PR #3859 (vertical-omnibutton): awaiting m417z response on retiring it in
     favor of the customizer; if refused, the 5 review comments need a response
     commit (archive: knowledge/pr-review-comments.md)

5. **Privacy Indicator Anchor** — mostly working as of 2026-07-18 user tests:
   copilot slash now sticks (WebExperience false-positive fixed), slash default
   flipped to falling (`\`), native active-mic suppression CONFIRMED working
   with correct mirrored state, in-use lighting confirmed (fires on actual
   streaming only — ConsentStore records are written on stream, not open).
   **Camera kill-switch verdict (full probe history in work log 2026-07-18):
   invisible to software at every level on this Legion — hardware-disabled
   detection is IMPOSSIBLE on this machine.** Camera icon contract: bright =
   actively streaming; no disabled-slash here.
   - [ ] Remove or gate the Lenovo `LENOVO_GAMEZONE_DATA` WMI check (this
     machine's firmware has no camera methods — it permanently no-ops); update
     both readmes to document the camera limitation honestly
   - [ ] Optional last stone: elevated `LENOVO_OTHER_METHOD` method list (low odds)
   - [ ] Replace 3s polling with event-driven (USER DIRECTIVE — dislikes
     polling): RegNotifyChangeKeyValue on ConsentStore keys + WMI event
     subscription for Lenovo Fn keys (`LENOVO_UTILITY_EVENT`); mic monitor
     already event-driven
   - [ ] If user's saved `slashDirection` is still "rising" from the old
     default, one dropdown click to Falling fixes it
   - [ ] Embedded↔folder readme unification + screenshots before submission
   - Design/reference: knowledge/privacy-anchor-design-notes.md,
     `_research/privacy-indicator-anchor-design.md`

## Lab-level todos

- [ ] taskmanager-tail README: user decision needed on unification direction
      (published mod; folder README is standalone-repo style)
- [ ] tray-utility-customizer: dedicated settings/readme pass (folder README
      richer than embedded); optionally add `shortGroupPosition`
- [ ] windhawk-mods PR update script (pull upstream → copy .wh.cpp →
      create/update PR); fork at `t:/Github/sb4ssman/windhawk-mods/`

## Future ideas (unscheduled)

- Densification layout (see work log 2026-05-08; `_research/densification-analysis.md`)
- Privacy anchor filler/status mode (mic dB meter, globe affordance)
- Folder-menus chevron-area sharing experiment
- Tray Stats Panel mod (free-standing stats panel)
- OmniButton glyph color animations — archived by user direction, needs a
  design conversation first (knowledge/omnibutton-color-animation-archived.md)
