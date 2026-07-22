# Working Notes — Windhawk Mod Lab

Living todo list — current state only, pruned every session. Completed work
(past tense) goes in [work_log.md](work_log.md). Durable rules live in
[README.md](README.md); reference material in [knowledge/](knowledge/).

## Current focus

**Tray Utility Customizer v2** (per-icon rewrite) in the local working copy —
NOT submitted, diverged from what's in PR #4841. Live-confirmed across most
layouts; two active limitation priorities (edge flyouts, Start settle) and a
pre-PR audit remain. Full history in work_log 2026-07-21; active items below.

OmniButton Customizer v1.0 is submitted as PR #4855 on branch
`add-omnibutton-customizer` (base upstream/main, one file:
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
- Tray Utility Customizer v1.0 — PR #4841 (⚠ local working copy is now the v2
  per-icon rewrite, wholly diverged from this PR — do not conflate)
- Taskbar VD Switcher v1.8 — PR #4844

Follow-ups:

- [ ] Clock Spacer: close m417z/my-windhawk-mods PR #68; optional fresh gallery
      and residual restart/disable/token checks.
- [ ] VD Switcher: verify issue #4784 hover/hit-test and test `multiMonitor`
      when Explorer restarts will not disrupt the user's desktop order.
- [ ] Privacy Anchor: optional final-build restart/hardware-monitor checks and
      idle screenshot refresh; keep frame-signature inference opt-in only.
- [ ] Tray Utility v2 (per-icon rewrite) — working copy, NOT submitted.
      Full history in work_log 2026-07-21 "Tray Utility Customizer v2
      rewrite". State: compiles clean, exit-time-destructor audit OK, all
      four template blocks diff-verified verbatim, both README layers match
      (README_MATCH) with the new gallery. Live-confirmed by user across
      row/stack/column/leased-column/busy-tray/Right-of-Start. Before any
      PR: run `six-mod-settings-audit.md` against the new `layout`/
      `primaryAxis`/`crossAlign` settings, do a final consolidated
      disable→restore→re-enable live pass, then get explicit approval.
      Two ACTIVE PRIORITY limitations remain (see below).
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
- [ ] Tray Utility housekeeping: old gallery assets `with-carot-stacked.png`,
      `with-extra-icons-carot.png`, and `part-of-a-mess.png` are no longer
      referenced by either README; archive/delete at user discretion.

## Lab-level todos
- [ ] TEMPLATE UNIFORMITY IS A STANDING PRIORITY (user, 2026-07-19): mods keep
      shipping without the curated `_templates/` profiles (settings names,
      order, smart grid). Every mod touched must be checked against
      `_templates/settings-profiles.md`, the applicable copy-source templates,
      and `six-mod-settings-audit.md` before screenshots/PR. OmniButton's
      applicable gaps are resolved in the current test candidate; Tray Utility
      v2 adopted lifecycle v1.2, start-placement v1.2, injected-column v1.2,
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
