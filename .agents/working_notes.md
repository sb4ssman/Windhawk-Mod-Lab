# Working Notes — Windhawk Mod Lab

Living todo list — current state only, pruned every session. Completed work
(past tense) goes in [work_log.md](work_log.md). Durable rules live in
[README.md](README.md); reference material in [knowledge/](knowledge/).

## Current focus

**No active development target.** OmniButton Customizer v1.0 is submitted as
PR #4855 on branch `add-omnibutton-customizer` (base upstream/main, one file:
`mods/omnibutton-customizer.wh.cpp`). This replaces the mislabeled reuse of the
legacy Vertical OmniButton PR #3859, which is now CLOSED with a redirect to
#4855; the stale `sb4ssman-vertical-omnibutton` branch is deleted. Await
maintainer review; do not change the PR without fresh explicit approval.

## Submitted — awaiting maintainer review

All six are CI green and were submitted/updated with explicit user approval.
Do not change a PR without fresh explicit approval.

- OmniButton Customizer v1.0 — PR #4855 (fresh clean PR 2026-07-21; #3859 closed/redirected)
- Privacy Indicator Anchor v1.0 — PR #4843
- Taskbar Clock Spacer v1.1 — PR #4443
- Taskbar Folder Menus v0.7 — PR #4485
- Tray Utility Customizer v1.0 — PR #4841
- Taskbar VD Switcher v1.8 — PR #4844

Follow-ups:

- [ ] Clock Spacer: close m417z/my-windhawk-mods PR #68; optional fresh gallery
      and residual restart/disable/token checks.
- [ ] VD Switcher: verify issue #4784 hover/hit-test and test `multiMonitor`
      when Explorer restarts will not disrupt the user's desktop order.
- [ ] Privacy Anchor: optional final-build restart/hardware-monitor checks and
      idle screenshot refresh; keep frame-signature inference opt-in only.
- [ ] Tray Utility: before any approved PR update, adopt lifecycle v1.2 and
      Start-placement v1.1; archive/delete unreferenced `part-of-a-mess.png` at
      user discretion.
- [ ] Tray Utility ACTIVE OVERHAUL (2026-07-21, working copy, not pushed):
      round 3 after user's full-once-over demand. State now:
      (a) smart-grid v1.1 (`minColumns` + `PackUnits`), tests pass, block
      diff-verified; (b) start-placement.h v1.1 ADOPTED verbatim, replacing
      the pre-template inline overlay whose v1.0 pinned-anchor geometry was
      the left/right-of-Start regression; (c) lifecycle v1.2 ADOPTED —
      exception-contained RunFromWindowThread dispatcher, TrayUI::StartTaskbar
      rehook with stale-tree drop, StartRetryThread/StopRetryThread,
      no_destroy on all XAML-owning globals, uninit retains state without a
      UI thread; exit-time-destructor audit OK; (d) keyboard-clipping root
      causes addressed: snapshots baseline the effective visible IconView
      COUNT and the intactness check reapplies on any change (touch keyboard
      arriving after boot previously left units undercounted → clipped), and
      bundle units are also floored by measured icon widths; (e) [Apply] log
      now always records mode/availableRows/trayHeight/groupSize. All three
      template blocks diff-verified, syntax check clean. Round 4 (user:
      chevron off-center, row-first violated): smart-grid template bumped to
      v1.2 — PackUnits now honors shortGroupPosition (underfull rows
      partition to front/back; default Last puts the full bundle row first,
      chevron row last), tests updated and pass; NEW mod setting
      `overflowPlacement` (auto/topRow/bottomRow/leftColumn/rightColumn)
      gives the chevron its own full row or column on any side, aligned by
      shortGroupAlign; embedded readme + settings updated (folder README
      still needs syncing before any PR). Round 5 verdict (2026-07-21, user
      live test "looks like shit"): host-moving architecture CANNOT deliver
      the mod's purpose on build 26200 — emoji + touch keyboard share ONE
      host (NonActivatableStack) so they can never stack vertically; forcing
      24px cells on wider native icons breaks hover highlights and spacing.
      `enabled` setting DELETED per user order. Round 6 (2026-07-21, user
      approved): FULL v2 REWRITE shipped to working copy. New templates:
      `_templates/nested-group-layout.h` v1.0 (nestable layout expression —
      `|` primary axis, `,` cross, parens alternate; pixel-space, native
      sizes, absent tokens collapse; tests pass) and
      `_templates/visual-tree-walk.h` v1.0 (walk/find/collect + OmniButton
      inner-StackPanel walk). Mod v2 architecture: per-icon control WITHOUT
      splitting Windows hosts — hosts reparent into one owned group
      (`TrayUtilityCustomizerGroup`) placed by the position options (borrow
      column = plain append, dedicated = column lease, Start = template
      lease); each IconView is individually placed via flow-compensating
      margins inside its host; chevron/MainStack are host-leaf items.
      BREAKING settings: `layout` expression replaces itemOrder/gridMode/
      smartLayout/gridRows/gridColumns/fillOrder/shortGroup*/
      overflowPlacement; buttonWidth/Height default 0 = native size;
      `primaryAxis` + `crossAlign` added. smart-grid template block removed
      from this mod (template file stays for other adopters). Unplaced
      visible icons append after the group. All four template blocks
      diff-verified; compile + exit-time-destructor audit clean. NEEDS USER
      LIVE TEST: defaults, diamond expression, single column
      (primaryAxis=column), per-icon stacking of emoji over keyboard,
      chevron hover highlight, flyouts, positions, unload restore. Folder
      README not yet synced to the v2 embedded readme. Round 7: v2 default
      row live-confirmed OK by screenshots; user's "overflow, emoji,
      keyboard" column failed because `keyboard` wasn't a token — added
      token aliases (chevron/keyboard/pen/touchpad/input, canonicalized
      pre-resolve via parsed-tree leaf collection) and an unknown-token log
      warning. Column still needs live re-test; note a 3-icon native column
      (~72-90px) overhangs a 48px single-height bar — suggest icon sizes
      ~16 to fit. Round 8: user reports Start positions misaligned
      vertically and Right-of-Start not opening a column between Start and
      the task list. start-placement.h bumped to v1.2 — group now centers
      vertically against the taskbar RootGrid, not Start's padded box; mod
      re-synced verbatim. Added an always-on [Start] log (inRepeater flag,
      Start rect, group margin, root size) — if inRepeater is misdetected,
      the counter-shift direction is wrong and the gap opens on the wrong
      side of Start; needs one capture to confirm. privacy-indicator-anchor
      remains on start-placement v1.1 (submitted PR #4843 — do not touch
      without approval).
- [ ] Tray Utility flyout clipping — round 9 MITIGATION shipped: explorer's
      tray flyouts are windowed XAML popups (Xaml_WindowedPopupClass);
      added a SetWindowPos hook that clamps them into the monitor work
      area when a screen-edge position would push them off (gated on
      g_layoutApplied, logs [Flyout] on clamp). REMAINING limitation: the
      emoji panel is drawn by TextInputHost.exe — unreachable from this
      mod's explorer injection; reaching it would need @include
      TextInputHost.exe (separate opt-in decision). Also round 9: Right-of-
      Start "snaps in only after screenshot/touch" fixed — after a start
      Acquire the mod now forces rootGrid.UpdateLayout() + re-Position,
      because removing the hosts re-centers the taskbar on the next layout
      pass and nothing triggered one.
- [ ] Tray Utility FEATURE IDEA (user, 2026-07-21): "Always show these
      icons" switch — drive the Windows taskbar-settings visibility toggles
      for the selected utilities (touch keyboard TipbandDesiredVisibility,
      pen ShowPenWorkspaceButton, touchpad) instead of an in-mod enable
      toggle. Deferred; needs its own investigation.
- [ ] Tray Utility FOLLOW-UP: flyout positioning — emoji flyout placement is
      inconsistent when the group moves, and overflow (hidden-icons) flyout
      opens cut off / off-screen at edge positions (left of Start, after
      Show Desktop). Windows computes flyout position from the host's screen
      location; needs its own investigation (which window, who positions it).

## Lab-level todos
- [ ] TEMPLATE UNIFORMITY IS A STANDING PRIORITY (user, 2026-07-19): mods keep
      shipping without the curated `_templates/` profiles (settings names,
      order, smart grid). Every mod touched must be checked against
      `_templates/settings-profiles.md`, the applicable copy-source templates,
      and `six-mod-settings-audit.md` before screenshots/PR. OmniButton's
      applicable gaps are resolved in the current test candidate; Tray Utility
      still has lifecycle and Start-placement follow-ups before its next
      approved PR update.
- [ ] windhawk-mods PR update script (pull upstream → copy .wh.cpp →
      create/update PR); fork at `t:/Github/sb4ssman/windhawk-mods/`

## Future ideas (unscheduled)

- Densification layout (see work log 2026-05-08; `_research/densification-analysis.md`)
- Privacy anchor filler/status mode (mic dB meter, globe affordance)
- Folder-menus chevron-area sharing experiment
- Tray Stats Panel mod (free-standing stats panel)
- OmniButton glyph color animations — archived by user direction, needs a
  design conversation first (knowledge/omnibutton-color-animation-archived.md)
