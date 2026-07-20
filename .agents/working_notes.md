# Working Notes — Windhawk Mod Lab

Living todo list — current state only, pruned every session. Completed work
(past tense) goes in [work_log.md](work_log.md). Durable rules live in
[README.md](README.md); reference material in [knowledge/](knowledge/).

## Current focus

Five mods to prepare. Immediate execution order from the user (2026-07-19):
OmniButton → Privacy Anchor. Folder Menus is ready for an explicitly authorized
PR sync; Clock Spacer is deferred behind the active fixes; VD Switcher is
follow-up feature work. No submissions until each works as described with
current docs/screenshots (standing directives in README.md).

1. **Taskbar Clock Spacer** — DEFERRED behind Folder Menus/OmniButton/Privacy.
   v1.1 FAILED first live round (2026-07-19,
   user report): the spacer effect does not work as expected ("something is
   funny" — behavior unspecified, needs diagnosis with logs/UWPSpy), and the
   WEATHER-COMPONENT SPACER feature is MISSING entirely (a spacer slot for the
   taskbar weather widget/component was part of the intended feature set).
   - [ ] Diagnose the wrong/absent spacer effect on live taskbar
   - [ ] Add the weather-component spacer feature
   DIRECTION SETTLED 2026-07-19: the deliverable is the STANDALONE companion mod
   (`@id taskbar-clock-spacer`, PR #4443, still open). The integration attempt
   (PR #68 in m417z/my-windhawk-mods) is finished — the maintainer preferred a
   Justify-based approach and implemented it himself; the user closed it out in
   comment on 2026-07-19. PR #68 is still technically OPEN and should be closed
   explicitly.
   - PDH `0xC0000BC6` is OUT OF SCOPE: it comes from m417z's performance-metrics
     code in the integration copy. The standalone mod has zero PDH references.
     `knowledge/taskbar-clock-spacer-pdh-invalid-data.md` applies only to the
     archived integration copy.
   - [ ] Live-test: spacers expand with Max width set; confirm the per-second
         rebuild is gone (visual tree stable under UWPSpy)
   - [ ] Live-test the `TrayUI::StartTaskbar` path via an Explorer restart, and
         mod-disable teardown (clock returns to normal)
   - [ ] Re-test multiple `%s%` per line, multiline clock content, settings
         reload, and `minSpacerWidth`
   - [ ] Fresh screenshots
   - [ ] Rebase branch `add-taskbar-clock-spacer` — it is ~248k deletions out of
         date against upstream/main and cannot merge as-is
   - [ ] Reframe the PR #4443 description as "companion mod, since the token
         didn't fit the canonical customizer"
   - Folders: `taskbar-clock-spacer/` is active. `clock-spacer/` (v1.0) and
     `taskbar-clock-customization-spacer/` (integration copy) are being archived
     by the user.

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

5. **Privacy Indicator Anchor** — ACTIVE FINAL LIVE TEST.
   User isolated frequent post-restart Explorer/Windhawk recovery to this mod
   on 2026-07-19. Windows Error Reporting confirms repeated `explorer.exe`
   access violations in `Windows.UI.Xaml.dll` (`0xc0000005`, identical offset
   `0x24c62f`) while `local@tray-privacy-indicator-anchor_0.9_621759.dll` is
   loaded. Source audit found the matching lifecycle defect: the namespace-
   scope synthetic `Grid`/`FrameworkElement` owners, `g_privacyStates`, and
   `g_loadedRevokers` are destructible globals. During process shutdown,
   Windhawk does not guarantee `Wh_ModUninit`, so their destructors can release
   XAML/WinRT objects after XAML teardown and off the UI thread. This is the
   same proven crash class previously fixed in Folder Menus.
   - [x] Fix all namespace-scope XAML/WinRT owners with intentional
         `[[clang::no_destroy]]` lifetime, retaining explicit UI-thread cleanup
         for controlled mod unload/settings changes
   - [x] Remove the unsafe no-taskbar fallback in `Wh_ModUninit` that directly
         clears revokers and XAML state off the UI thread; intentionally retain
         the no-destroy holders if no taskbar UI thread can be reached
   - [x] First restart regression test FAILED 2026-07-19. The old XAML AV did
         not recur; the fresh failure is an uncaught MinGW/C++ exception
         (`0x20474343`) surfaced through `KERNELBASE.dll`.
   - [x] Fix the dump-proven startup chain: removed `Application::Current`,
         stopped clearing from repainting absent synthetic UI, requires a live
         XAML root before cleanup/reapply, and contains exceptions at all native
         and XAML callback boundaries.
   - [x] Full system restart survived with the crash fix on 2026-07-19.
   - [ ] Re-test one Explorer or system restart with the final Start-placement
         build and confirm Event Viewer has neither the old XAML AV nor
         `0x20474343`.
   The camera worker is not implicated by the fresh dump. Source compilation,
   the exit-time-destructor gate, and upstream source validation still pass.
   Full applicable template adoption is complete: Smart Grid v1.0,
   injected-grid-column v1.2, lifecycle dispatcher v1.2, canonical settings,
   and StartPlacement v1.0 are verbatim. Complete submission preflight,
   including the current upstream validator, passes.
   Do not treat the visual-state implementation or the 2026-07-19 metadata fix
   as completion. Mostly
   working as of 2026-07-18 user tests:
   copilot slash now sticks (WebExperience false-positive fixed), slash default
   flipped to falling (`\`), native active-mic suppression CONFIRMED working
   with correct mirrored state, in-use lighting confirmed (fires on actual
   streaming only — ConsentStore records are written on stream, not open).
   The camera slash is now LIVE-CONFIRMED: it responds to the physical switch.
   Microsoft treats idle-camera occlusion as advisory, so the refined tooltip
   says "likely blocked" until Windows also reports active camera use.
   - [ ] Live-test the evidence-specific tooltip text for all four icons,
         especially Copilot's new transparent slot hit target
   - [ ] Click each icon and confirm the reason-aware Settings destination:
         location privacy; mic privacy/default input; camera privacy/device;
         Copilot taskbar/installed apps
   - [ ] Toggle `cameraHardwareDetection` off/on and remove/re-add `camera` in
         `itemOrder`; confirm the controller releases/reopens without an
         Explorer restart and only one monitor remains active
   - [ ] Confirm the persistent `SharedReadOnly` monitor does not turn on the
         webcam LED or create a permanent webcam-usage record by itself.
   - [ ] Confirm the Fn-key mic state is labeled "Endpoint muted" rather than
         "Hardware disabled" and that Windows privacy denial gets a distinct
         access-denied reason
   - [ ] Keep frame-signature inference only as an opt-in fallback experiment
         for cameras whose drivers do not support `CameraHardware` occlusion.
   - [x] Live-test the new event-driven state worker: location, mic, camera,
         permission, and hardware-shutter states all produced the expected UI.
         Some camera/location/mic responses remain naturally delayed, but the
         removed three-second global sweep has not been needed for correctness.
   - [ ] Live-test the four-state visual polish: idle, active, unavailable, and
         active-while-unavailable. Compare steady/pulse/radiate at low and high
         intensity, and confirm radiation never changes tray width.
   - [x] Smart Grid uses the full canonical six-mode settings and verbatim
         template algorithm
   - [ ] Live-test `leftOfStart` and `rightOfStart`, then switch back to a tray
         position and confirm Start/task-item placement restores exactly
   - [ ] If user's saved `slashDirection` is still "rising" from the old
     default, one dropdown click to Falling fixes it
   - [x] Embedded↔folder readme unification
   - [x] All ten PNG screenshots examined and included in both README layers,
         including glow, blocked-active, Smart Grid, and all four tooltips
   - [ ] COMMIT + PUSH the four new tooltip PNGs (camera/copilot/mic/
         location-tooltip-and-win-settings) — they are untracked, so every
         raw.githubusercontent URL for them 404s (screenshot-audit 2026-07-19).
         The other six assets are already on origin/main.
   - [ ] Consider retaking `location-mic-availble-not-in-use.png` — only idle
         screenshot, dated May 9 (old build, filename typo "availble")
   - [ ] Pre-PR: decide what to do with the embedded readme's `## Files` and
         `## Status` sections — relative links/lab language don't belong on
         windhawk.net (Folder Menus precedent has neither section)
   - [ ] After final live confirmation, bump v0.9 to PR-ready v1.0 and prepare
         the current-upstream one-file PR branch/body
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
- [ ] Lab repo has post-push edits to commit: @version 0.5→1.0 bump (with
      matching init-log line) + root README status row
- [ ] `assets/part-of-a-mess.png` (June leftover) is unreferenced — archive
      or delete at user's discretion

## Lab-level todos
- [ ] TEMPLATE UNIFORMITY IS A STANDING PRIORITY (user, 2026-07-19): mods keep
      shipping without the curated `_templates/` profiles (settings names,
      order, smart grid). Every mod touched must be checked against
      `_templates/settings-profiles.md` + `six-mod-settings-audit.md` before
      screenshots/PR. Remaining known gaps: privacy-indicator-anchor (smart
      grid settings not exposed/working per user), folder-menus (smart grid
      broken). tray-utility-customizer RESOLVED 2026-07-19 (submitted v1.0).
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
