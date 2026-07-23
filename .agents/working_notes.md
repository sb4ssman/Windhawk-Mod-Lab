# Working Notes — Windhawk Mod Lab

Living todo list — current state only, pruned every session. Completed work
(past tense) goes in [work_log.md](work_log.md). Durable rules live in
[README.md](README.md); reference material in [knowledge/](knowledge/).

## Current focus

**Taskbar Folder Menus v0.7** (PR #4485, commit `18fe50eb`) and **Tray Utility
Customizer v1.1** (PR #4841, commit `20aaf8e5`) were updated upstream on
2026-07-23 with explicit user approval after a live test. Both address every
required and optional maintainer review item; all four CI jobs are green on
each, and a review reply is posted on both. Versions were intentionally not
bumped — neither mod is published upstream yet, so 0.7 and 1.1 remain the
proposed initial versions. Await maintainer review; do not change either PR
without fresh explicit approval.

OmniButton Customizer v1.0 is submitted as PR #4855 on branch
`add-omnibutton-customizer` (base upstream/main, one file:
`mods/omnibutton-customizer.wh.cpp`). This replaces the mislabeled reuse of the
legacy Vertical OmniButton PR #3859, which is now CLOSED with a redirect to
#4855; the stale `sb4ssman-vertical-omnibutton` branch is deleted. Await
maintainer review; do not change the PR without fresh explicit approval.

## Open upstream PRs — re-audited 2026-07-23

All six are CI green and were submitted/updated with explicit user approval.
All six are currently open, mergeable, and `CLEAN`, and every PR branch is a
clean one-file diff against `upstream/main`. Do not change a PR without fresh
explicit approval, and never push a review fix before a fresh live test.

- OmniButton Customizer v1.0 — PR #4855: no comments/reviews; passive wait.
- Privacy Indicator Anchor v1.0 — PR #4843: no comments/reviews; passive wait.
- Taskbar Clock Spacer v1.1 — PR #4443: maintainer explicitly said it can be
  merged as the standalone mod; integration PR m417z/my-windhawk-mods #68 is
  already closed. Passive wait.
- Taskbar Folder Menus v0.7 — PR #4485: UPDATED 2026-07-23, commit `18fe50eb`.
  The 2026-07-21 review is fully answered: optional-backed no-destroy state with
  controlled reset (`g_loadedRevokers` removed outright), no IconView/
  multi-module startup path, a single apply in `Wh_ModAfterInit`, and
  SRW-guarded retry-handle handoff. CI green; reply posted. Passive wait.
- Tray Utility Customizer v1.1 — PR #4841: UPDATED 2026-07-23, commit
  `20aaf8e5`. Both required items plus every optional item are addressed —
  no-destroy ownership, the MainStack host-leaf phantom-straggler gap, retry
  synchronization, settings recovery via the retry path, token revocation,
  parser rejection, and metadata cleanup — while retaining language-neutral
  detection. CI green; reply posted. Passive wait.
- Taskbar VD Switcher v1.8 — PR #4844: required `g_settings` no-destroy removal
  is already present in pushed commit `c5995bd3`, CI is green, and the
  maintainer reply was posted 2026-07-23 (confirming Claude introduced the
  annotation). Committed publicly to converting `g_autoRevokerList`,
  `g_buttonEventStates`, and `g_secondaryBars` to optional-backed no-destroy
  with `reset()` in the NEXT LIVE-TESTED update — not before a live test.
  Passive wait.

Follow-ups:

- [ ] Clock Spacer: optional fresh gallery and residual restart/disable/token
      checks. m417z/my-windhawk-mods PR #68 is already closed.
- [ ] VD Switcher: verify issue #4784 hover/hit-test and test `multiMonitor`
      when Explorer restarts will not disrupt the user's desktop order.
- [ ] Privacy Anchor: optional final-build restart/hardware-monitor checks and
      idle screenshot refresh; keep frame-signature inference opt-in only.
- [ ] Tray Utility v1.1 (per-icon rewrite) — submitted in PR #4841.
      Full history in work_log 2026-07-21 "Tray Utility Customizer v2
      rewrite". State: `COMPILE_OK`, exit-time-destructor audit OK,
      `README_MATCH`, and upstream validator `SUBMISSION_PREFLIGHT_OK`.
      Settings/template audit and gallery work are complete. Maintainer item
      1 was initially addressed by documenting the English limitation. A new
      local candidate now removes that limitation entirely: it matches
      Windows' `EmojiAndMore`, `TouchKeyboard`, `InkWorkspace`,
      `VirtualTouchpad`, `Language`, and `Ime` system-tray data-model classes,
      plus exact XAML/Automation identities and established glyphs. No
      localized accessibility words participate. Automated preflight passes;
      live-test all available utilities before another PR update. Item 3 is addressed
      by removing the IconView constructor hook, the SystemTray/Taskbar.View/
      ExplorerExtensions dependency, and the LoadLibraryExW hook; lifecycle
      now relies on taskbar.dll `TrayUI::StartTaskbar`, the bounded initial/
      rebuild retry, visibility watchers, and the layout-intactness check.
      User live-confirmed icon appearance/churn, Explorer/taskbar rebuild, and
      disable→native restore→re-enable. PR commit `43d1ac77`; changed-file
      validation and Windhawk 1.6.1/1.7.3/2.0.0-alpha.1 compile jobs all pass.
      Await maintainer review. Two documented limitations remain (see below).
- [ ] Tray Utility edge flyout clipping — ACCEPTED LIMITATION (both READMEs
      document it). The SetWindowPos work-area clamp was tried TWICE
      (2026-07-21) and REMOVED both times — it never fixed the overflow
      flyout, so `Xaml_WindowedPopupClass` + a window-move hook is the wrong
      lever. Working theory: the overflow flyout is an in-process XAML Popup
      positioned in DIP space off the taskbar XamlRoot (no separate HWND that
      SetWindowPos would touch), so a real fix would need a XAML-layer
      intervention (find the transient Popup, adjust placement/offset) — high
      risk for a cosmetic edge-only issue, deferred. The Emoji panel is drawn
      by `TextInputHost.exe` (different process) — genuinely out of reach from
      explorer injection; would need `@include TextInputHost.exe`. Do NOT
      re-attempt the SetWindowPos clamp. If ever revisited, FIRST capture the
      real overflow-flyout window (Spy++) to confirm whether it's even a
      separate HWND.
- [ ] PRIORITY — Tray Utility Right/Left of Start settle: the centered
      taskbar re-flows through an animation after the hosts leave the tray;
      current mitigation is `UpdateLayout()` + immediate re-Position + a
      600 ms settle-timer re-Position. Needs live confirmation it fully
      fixes the "settles only after interaction" behavior. Always-on [Start]
      log (inRepeater flag, Start rect, group margin, root size) is present
      for diagnosis.
- [ ] Tray Utility FEATURE IDEA (user, 2026-07-21): "Always show these
      icons" switch — drive the Windows taskbar-settings visibility toggles
      for the selected utilities (touch keyboard TipbandDesiredVisibility,
      pen ShowPenWorkspaceButton, touchpad) instead of an in-mod enable
      toggle. Deferred; needs its own investigation.
## Lab-level todos
- [ ] FORK `main` RESET — AWAITING APPROVAL. `sb4ssman/windhawk-mods` `main` is
      ahead of `upstream/main` by ten commits that self-merge
      `mods/vertical-omnibutton.wh.cpp` (fork PR #1) plus merge noise. Any
      branch cut from it inherits the stray file — this already forced two
      "remove stray vertical-omnibutton.wh.cpp" cleanup commits. Fix is
      `git branch -f main upstream/main` + force-push to origin. No open PR
      descends from fork `main`, so nothing breaks. Old tip: `de5feb0f`.
- [ ] STALE FORK BRANCHES — AWAITING APPROVAL. `sb4ssman-taskbar-vd-switcher`
      (PR #3932 merged), `update-taskbar-vd-switcher` (#4516 merged), and
      `sb4ssman-virtual-desktop-switcher` (#4484 closed) exist locally and on
      the fork with no live PR. Safe to delete once approved.
- [ ] LIFECYCLE v1.3 ROLLOUT — the corrected template distinguishes heap-only
      state, direct XAML handles, and optional-backed XAML containers with
      controlled UI-thread `reset()`. Folder Menus and Tray Utility now have
      local preflight-green, live-tested adoptions. The strengthened audit
      still intentionally fails VD Switcher, Privacy Anchor, OmniButton, and
      Clock Spacer; roll those out one live-tested mod at a time before their
      next PR updates.
- [ ] FUTURE TEMPLATE EXTRACTION — the tray-utility v2 "positioning system"
      is only PARTLY templated. Done: `nested-group-layout.h` (expression →
      pixel placements, tested). NOT done: the orchestration in
      `ApplyLayout` — reparent native hosts into one owned group + steer each
      IconView to its target via flow-compensating margins + anchor selection
      (borrow column / injected-column lease / start-placement lease). That
      per-icon reparent+margin-steer pattern is the novel reusable piece;
      extract it into e.g. `tray-group-placement.h` (with the OmniButton
      "independent mode" as a second data point) when another mod needs it.
- [ ] TOOLING: UWPSpy is available (XAML-tree inspection) — see memory
      `reference_uwpspy`. Use it (not Spy++) to investigate whether the
      tray-utility overflow flyout is an in-process XAML Popup before any
      further edge-clip attempt.
- [ ] TEMPLATE UNIFORMITY IS A STANDING PRIORITY (user, 2026-07-19): mods keep
      shipping without the curated `_templates/` profiles (settings names,
      order, smart grid). Every mod touched must be checked against
      `_templates/settings-profiles.md`, the applicable copy-source templates,
      and `six-mod-settings-audit.md` before screenshots/PR. OmniButton's
      applicable gaps are resolved in the current test candidate; Tray Utility
      v2 adopted lifecycle v1.2 (now superseded by v1.3), start-placement v1.2,
      injected-column v1.2,
      and the two new layout/tree-walk templates, but its new `layout`/
      `primaryAxis`/`crossAlign` settings still need a `settings-profiles.md`
      audit pass before any PR.
- [ ] windhawk-mods PR update script (pull upstream → copy .wh.cpp →
      create/update PR); fork at `t:/Github/sb4ssman/windhawk-mods/`

## Future ideas (unscheduled)

- Densification layout (see work log 2026-05-08; `_research/densification-analysis.md`)
- Privacy anchor filler/status mode (mic dB meter, globe affordance)
- Folder-menus chevron-area sharing experiment
- Tray Stats Panel mod (free-standing stats panel)
- OmniButton glyph color animations — archived by user direction, needs a
  design conversation first (knowledge/omnibutton-color-animation-archived.md)
