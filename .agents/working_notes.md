# Working Notes — Windhawk Mod Lab

Living todo list — current state only, pruned every session. Completed work
(past tense) goes in [work_log.md](work_log.md). Durable rules live in
[README.md](README.md); reference material in [knowledge/](knowledge/).

## Current focus

Privacy Anchor was submitted 2026-07-19 with explicit user approval (PR
#4843). Remaining active work: OmniButton (broken, needs logs); Clock Spacer v1.1 submitted (PR #4443, CI green). Folder Menus
is submitted and awaiting review; Clock Spacer is deferred; VD Switcher is
follow-up feature work. No further submissions until each works as described
with current docs/screenshots (standing directives in README.md).

1. **Taskbar Clock Spacer** — v1.1 SUBMITTED 2026-07-19 (PR #4443 updated),
   CI GREEN. User live-tested and approved all three fix rounds. The branch
   `add-taskbar-clock-spacer` was rebuilt as a single fresh commit on current
   upstream/main (old base was ~248k deletions stale); PR title/body updated
   with the v1.0→v1.1 changelog, companion-mod framing, and authorship
   section. Validator required module-named symbol-hook variables
   (systemTrayDllHooks + a "// taskbar.dll" comment form). PR validation and
   the 1.6.1/1.7.3/2.0.0-alpha.1 compile matrix all pass. Watch for
   maintainer review; no PR updates without explicit user approval.
   - [ ] Optional: fresh screenshots for the PR/readme gallery (target.png is
         a reference shot, not published)
   - [ ] Close m417z/my-windhawk-mods PR #68 explicitly (still open; user
         decided 2026-07-19 the integration attempt is finished)
   - [ ] Optional residual live checks: Explorer-restart path and
         mod-disable teardown; multiple `%s%` per line; `minSpacerWidth`
   - Fix history and design rationale: work log 2026-07-19 entries.
     Old experiments live in `_archive/` (clock-spacer v1.0 and the
     TCC-integration copy; its PDH note is
     knowledge/taskbar-clock-spacer-pdh-invalid-data.md).

2. **Taskbar VD Switcher** — v1.7 MERGED 2026-07-18 (PR #4516). Remaining work
   is follow-up, not release-blocking.
   - [ ] Issue #4830 feature set: custom user-defined indicator symbols/letters
         (example active/inactive dot strings), separate master-button and
         indicator fonts, and Styler-friendly Checked/CheckedPointerOver/
         CheckedPressed visual states
   - [ ] Verify the full-rebuild change also fixes issue #4784 (hover/hit-test)
   - [ ] Test the experimental `multiMonitor` toggle (deferred: Explorer
         restarts disrupt the user's desktop order)
   - [ ] Refresh screenshots (accent default changed visuals)
   - [ ] Sync folder README settings table

3. **Taskbar Folder Menus** — v0.7 SUBMITTED 2026-07-19 in PR #4485, CI GREEN.
   The PR branch is cleanly based on current upstream/main at commit `46f3301`.
   PR validation and the Windhawk 1.6.1/1.7.3/2.0.0-alpha.1 compile matrix all
   pass. User live-confirmed the replacement Smart Grid and restart persistence;
   both README layers use all six current screenshots. Watch for maintainer
   follow-up; do not update the PR again without explicit approval.

4. **OmniButton Customizer** — ACTIVE. BROKEN. Test round 3 failed 2026-07-17 (work
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

5. **Privacy Indicator Anchor** — v1.0 SUBMITTED 2026-07-19 (PR #4843), CI
   GREEN. Branch add-tray-privacy-indicator-anchor on the fork, based on
   current upstream/main; user approved the submission. PR validation and the
   1.6.1/1.7.3/2.0.0-alpha.1 compile matrix all pass. Development history is
   in the work log (2026-07-19 entries).
   - [ ] Watch PR #4843 for maintainer review; no PR updates without explicit
         user approval
   - User confirmed 2026-07-19 they live-tested the Start-adjacent positions
         before approving submission — treat the submitted build's
         leftOfStart/rightOfStart as user-verified; no open test item. (If the
         maintainer questions it, deal with it then.)
   - [ ] Remaining live checks (non-blocking follow-ups): restart regression on
         the final build (no XAML AV, no `0x20474343`); cameraHardwareDetection
         toggle + camera itemOrder removal releases/reopens the controller; the
         SharedReadOnly monitor never lights the webcam LED or writes a usage
         record; Fn-key mic reads "Muted - endpoint", privacy denial reads
         access-denied; four-state visual polish and glow styles
   - [ ] Consider retaking `location-mic-availble-not-in-use.png` — only idle
         screenshot, dated May 9 (old build, filename typo "availble")
   - [ ] Frame-signature inference stays an opt-in fallback experiment only,
         for cameras without `CameraHardware` occlusion support
   - Design/reference: knowledge/privacy-anchor-design-notes.md,
     `_research/privacy-indicator-anchor-design.md`

## Tray Utility Customizer — v1.0 SUBMITTED 2026-07-19 (PR #4841), CI GREEN

PR #4841 on ramensoftware/windhawk-mods (branch add-tray-utility-customizer).
All CI checks pass: PR validation (after adding the required "## Mod
authorship" template section: submitter-with-AI-assistance + Claude) and the
Windhawk 1.6.1 / 1.7.3 / 2.0.0-alpha.1 compile matrix. Full development
history is in the work log (2026-07-19 entries).
- [ ] Watch PR #4841 for maintainer review
- [ ] Before any PR update, adopt lifecycle template v1.1: the destructor audit
      found unprotected XAML owners, revokers, snapshots/watchers, and timer
      state. Do not update the submitted PR without explicit user approval.
- [ ] Also adopt start-placement template v1.1 at that time: TUC still carries
      the pre-template inline copy (`PositionStartGroup`) with the same v1.0
      geometry defects fixed in privacy-anchor on 2026-07-19 (stale absolute
      anchor, leftOfStart pinned to taskbar edge).
- [ ] `assets/part-of-a-mess.png` (June leftover) is unreferenced — archive
      or delete at user's discretion

## Lab-level todos
- [ ] TEMPLATE UNIFORMITY IS A STANDING PRIORITY (user, 2026-07-19): mods keep
      shipping without the curated `_templates/` profiles (settings names,
      order, smart grid). Every mod touched must be checked against
      `_templates/settings-profiles.md` + `six-mod-settings-audit.md` before
      screenshots/PR. folder-menus RESOLVED (replacement Smart Grid
      live-confirmed, v0.7 in PR #4485); tray-utility-customizer and
      privacy-indicator-anchor RESOLVED 2026-07-19 (submitted at v1.0 with
      full canonical profiles). No known open gaps; keep checking every mod
      touched.
- [ ] taskmanager-tail README: user decision needed on unification direction
      (published mod; folder README is standalone-repo style)
- [ ] windhawk-mods PR update script (pull upstream → copy .wh.cpp →
      create/update PR); fork at `t:/Github/sb4ssman/windhawk-mods/`

## Future ideas (unscheduled)

- Densification layout (see work log 2026-05-08; `_research/densification-analysis.md`)
- Privacy anchor filler/status mode (mic dB meter, globe affordance)
- Folder-menus chevron-area sharing experiment
- Tray Stats Panel mod (free-standing stats panel)
- OmniButton glyph color animations — archived by user direction, needs a
  design conversation first (knowledge/omnibutton-color-animation-archived.md)
