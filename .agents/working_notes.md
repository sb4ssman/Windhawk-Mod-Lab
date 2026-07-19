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

3. **Taskbar Folder Menus** — READY FOR SCREENSHOTS. The v0.6 reviewer
   revision is compile-checked and live-confirmed: native Shell context menus,
   modal verbs, nested-cascade retention, click-away dismissal, and button
   visual reset are working. Defaults are now Smart automatic, automatic
   column cap (`gridColumns: 0`), and 24 px button width.
   - [ ] Capture current screenshots: one ordinary single-height taskbar and
         one useful double-height/elaborate configuration
   - [ ] After screenshots, perform the final three-layer docs check before
         discussing any PR #4485 sync (no submission yet)

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

5. **Privacy Indicator Anchor** — ACTIVE TOOLTIP/ACTION TEST. Mostly
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
   - [ ] If user's saved `slashDirection` is still "rising" from the old
     default, one dropdown click to Falling fixes it
   - [x] Embedded↔folder readme unification
   - [ ] Capture submission screenshots, including an intentionally striking
         radiating-active example and a blocked/active slash combination
   - Design/reference: knowledge/privacy-anchor-design-notes.md,
     `_research/privacy-indicator-anchor-design.md`

## Lab-level todos

- [ ] Taskbar Clock Customization Spacer: investigate the repeating
      `PDH_INVALID_DATA` counter failures captured during privacy testing; see
      `knowledge/taskbar-clock-spacer-pdh-invalid-data.md`.
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
