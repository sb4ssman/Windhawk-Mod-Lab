# Working Notes — Windhawk Mod Lab

Living todo list — current state only, pruned every session. Completed work
(past tense) goes in [work_log.md](work_log.md). Durable rules live in
[README.md](README.md); reference material in [knowledge/](knowledge/).

## Current focus

**No active development target.** OmniButton Customizer v1.0 replaced the
legacy Vertical OmniButton proposal in PR #3859. The one-file submission is
based on current upstream/main and all four CI checks are green. Await maintainer
review; do not change the PR without fresh explicit approval.

## Submitted — awaiting maintainer review

All six are CI green and were submitted/updated with explicit user approval.
Do not change a PR without fresh explicit approval.

- OmniButton Customizer v1.0 — PR #3859 (updated 2026-07-21)
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
