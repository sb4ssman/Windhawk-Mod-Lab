# Working Notes — Windhawk Mod Lab

Living todo list — current state only, pruned every session. Completed work
(past tense) goes in [work_log.md](work_log.md). Durable rules live in
[README.md](README.md); reference material in [knowledge/](knowledge/).

## Current focus

**OmniButton Customizer is the active development target.** Its v1.0 source
compiles and its README layers match, but the last live test is broken: coupled
battery percentage is wrong and independent mode hides the battery glyph. The
user confirmed that the Grid template is not implicated.

Template status (audited 2026-07-19):

- Smart Grid v1.0: adopted; implementation body is an exact template match
  (`<algorithm>` is already included earlier in the single-file mod).
- Settings profile: canonical, capability-appropriate names and ordering.
- Color tokens: canonical behavior is present through a Color-returning parser;
  the Brush-returning button-surface template is not applicable because this
  mod mutates native glyphs rather than owning a button surface.
- XAML lifecycle v1.2: **not adopted**. The exit-time audit reports 14
  destructible namespace-scope XAML/WinRT owners. Dispatch lacks the template's
  exception boundary/live-root contract, and unload/settings fallbacks can
  release XAML state off the taskbar UI thread.
- Injected-column, Start-placement, button-surface, and placement-contract
  templates: not applicable to the current feature set.

Next work, in order:

- [ ] FIRST collect Windhawk `[Battery]` and `[Layout]` logs from the failing
      coupled and independent modes; do not resume static guessing.
- [ ] Fix coupled percentage and independent glyph behavior from that evidence.
- [ ] Adopt lifecycle v1.2: `[[clang::no_destroy]]` owners, UI-thread-only
      cleanup, intentional retention when no live taskbar exists, live-root
      guards, callback exception containment, and startup/restart retry path.
- [ ] Rename or annotate both generic symbol-hook arrays for upstream validator
      compliance.
- [ ] Re-run compile, destructor audit, README parity, template comparison, and
      submission preflight.
- [ ] Live-test dual/single height, short-group position/alignment, colors,
      settings reload, disable, and Explorer restart.
- [ ] Replace the vertical-era screenshots, then prepare a PR only after explicit
      user approval.

PR #3859 (`vertical-omnibutton`) is awaiting m417z's response on retiring it in
favor of this full customizer. If retirement is refused, its five review comments
still need a response commit; see `knowledge/pr-review-comments.md`.

## Submitted — awaiting maintainer review

All five are CI green and were submitted/updated with explicit user approval on
2026-07-19. Do not change a PR without fresh explicit approval.

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
      and `six-mod-settings-audit.md` before screenshots/PR. OmniButton's open
      lifecycle gap is documented above; Tray Utility also has lifecycle and
      Start-placement follow-ups before its next approved PR update.
- [ ] windhawk-mods PR update script (pull upstream → copy .wh.cpp →
      create/update PR); fork at `t:/Github/sb4ssman/windhawk-mods/`

## Future ideas (unscheduled)

- Densification layout (see work log 2026-05-08; `_research/densification-analysis.md`)
- Privacy anchor filler/status mode (mic dB meter, globe affordance)
- Folder-menus chevron-area sharing experiment
- Tray Stats Panel mod (free-standing stats panel)
- OmniButton glyph color animations — archived by user direction, needs a
  design conversation first (knowledge/omnibutton-color-animation-archived.md)
