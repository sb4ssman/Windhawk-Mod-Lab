# Working Notes — Windhawk Mod Lab

Living todo list — current state only, pruned every session. Completed work
(past tense) goes in [work_log.md](work_log.md). Durable rules live in
[README.md](README.md); reference material in [knowledge/](knowledge/).

## Current focus

Privacy Anchor was submitted 2026-07-19 with explicit user approval (PR
#4843). Remaining active work: Clock Spacer (width fix awaiting live test) and
OmniButton (broken, needs logs). Folder Menus
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
   Superseded details (kept for this session only):
   DIAGNOSED AND FIXED 2026-07-19 (static analysis vs the current TCC source;
   compiles clean with the local Windhawk clang — see
   knowledge/lab-local-compile-check.md): the "multiplying spaces" the user saw
   were a measurement-feedback ratchet — `EffectiveLineWidth` fell back to
   `parent.ActualWidth()` and hard-set `Width` on the generated rows from that
   snapshot every tick, so the clock could only ever grow. A second defect
   compounded it: with the mod's own maxWidth at 0, `ApplyWidthConstraint` on
   the parent CLEARED the StackPanel MaxWidth that Taskbar Clock Customization
   itself sets — erasing the user's fixed clock width (TCC only reapplies on
   style invalidation). Fix: never touch the parent panel at all, collapse the
   source block to zero width so the panel follows the generated rows, and take
   the width from our settings or TCC's constant parent MaxWidth — never from
   ActualWidth. maxWidth setting docs updated in all three layers. Target
   behavior reference: `taskbar-clock-spacer/target.png` (5-line justified
   clock from the retired TCC-integration build).
   ROUND 2 (2026-07-19, after user screenshot comparison vs target.png): two
   more changes, UNCOMMITTED AND UNPUSHED pending user test (explicit user
   directive: NO PUSH before their testing).
   - Weather spacer restored from the archived integration copy's design
     (`_archive/taskbar-clock-customization-spacer`): `{spacer}` is now a
     second split token. wttr.in consumes `%s` (sunset), so `%s%` can't ride
     through the Weather format, but a literal `{spacer}` is echoed verbatim
     by wttr.in and lands in the clock text where the mod splits on it — no
     TCC modification needed (the integration used a  marker because it
     rewrote the format pre-request; standalone doesn't need to).
   - Edge-inset defect fixed: generated rows had explicit Width, so when a
     sibling line (the unspaced weather line) was wider, the fixed-width rows
     floated centered inside the wider panel — the insets in the user's
     screenshot. Now the panel gets MinWidth (+ MaxWidth cap) and rows
     stretch, so lines always justify to the panel's true width; explicit
     Width only with the Line width override setting. Unspaced rows carry no
     width constraint (no clipping).
   - Docs updated in both layers (weather {spacer} section + limitations).
     Compiles clean (-Wall) with the local Windhawk clang.
   ROUND 3 (2026-07-19, user screenshot: lines justify but exceed TCC's
   150px Max width): a StackPanel arranges a child at max(slot, desired), so
   the unspaced weather row's natural width dragged the uncapped generated
   panel past TCC's cap and every spaced row stretched with it. Fix: the
   generated panel is now pinned to exactly the effective width (MinWidth AND
   MaxWidth), and every row gets MaxWidth = width, so an over-long unspaced
   line clips at the fixed width like the native block. Row caps reapply on
   the fast path (TCC Max width can change without touching this mod's layout
   key). Also corrected the readme table's double-spacer example per user.
   Compiles clean (-Wall).
   - [ ] USER LIVE TEST: (a) clock width exactly matches TCC's Max width
         (150), never wider; (b) gaps constant across ticks, no creep;
         (c) all lines hug both edges matching target.png; (d) `{spacer}` in
         TCC's Weather format produces gaps inside the weather line (takes
         effect on the next weather refresh); (e) verify with TCC Max width,
         with this mod's Max clock width, and with neither (spacer inert)
   - [ ] After user confirmation: commit, then screenshots, PR #4443 rebase +
         description reframe
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
