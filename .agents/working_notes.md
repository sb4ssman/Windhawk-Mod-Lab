# Working Notes — Windhawk Mod Lab

Living todo list — current state only, pruned every session. Completed work
(past tense) goes in [work_log.md](work_log.md). Durable rules live in
[README.md](README.md); reference material in [knowledge/](knowledge/).

## Current focus

**OmniButton Customizer is the active development target.** The candidate
survived the user's Explorer restart. Live diagnostics show that this Windows
build exposes the percentage as one `BatteryTextBlock` containing `81%`; the
earlier multi-child inference from a screenshot was incorrect. Smart automatic
uses the intended compact 2×2 at standard height. Current work is exact cell
centering and then optical calibration of the native icon/font shapes.

Template status (audited 2026-07-19):

- Smart Grid v1.0: adopted; implementation body is an exact template match
  (`<algorithm>` is already included earlier in the single-file mod).
- Settings profile: canonical, capability-appropriate names and ordering.
- Color tokens: canonical behavior is present through a Color-returning parser;
  the Brush-returning button-surface template is not applicable because this
  mod mutates native glyphs rather than owning a button surface.
- XAML lifecycle v1.2: adopted. All XAML/WinRT owners use process-safe lifetime,
  cleanup is UI-thread-only, callbacks contain exceptions, and startup/restart
  use the standard retry plus `TrayUI::StartTaskbar` path.
- Injected-column, Start-placement, button-surface, and placement-contract
  templates: not applicable to the current feature set.

Candidate changes awaiting live evidence:

- Missing `itemOrder` tokens now really hide items instead of being appended.
- Coupled mode treats battery/percentage as one native group; independent mode
  gives each a full grid cell inside a correctly sized footprint.
- Every item now has independent color, size, font, opacity, visibility/order,
  and X/Y nudge controls.
- Canonical `gridMode` and `smartLayout` settings expose the entire Smart Grid
  template instead of inferring a partial mode from row/column values.
- Exact original dependency-property values are snapshotted and restored on
  settings change/unload instead of guessing Windows defaults.
- Independent battery/percentage is now the default. The mod does not mutate
  outer `ControlCenterButton` height/alignment; cleanup forces a final native
  layout pass. Its width is leased only to the grid footprint so the percentage
  cannot be clipped. Native battery/percentage elements are measured at desired
  size and centered in each cell without integer truncation.
- Canonical group padding and group X/Y offsets provide safe placement detail
  without changing the native tray order. Full cross-column relocation remains
  intentionally deferred: it would invalidate other mods' `beforeOmni` /
  `beforeClock` semantics without a shared placement lease.
- `itemOrder` is now comma-separated in the setting default, description, docs,
  presets, and tests (`wifi, volume, battery, percent`); the parser continues to
  accept whitespace-only legacy input.
- Percentage visibility, color, font, size, opacity, and offsets apply to the
  single native `BatteryTextBlock`, whose text already includes the `%` sign.

Next work, in order:

- [ ] Run `.agents/knowledge/omnibutton-test-checklist.md`, starting with
      default/forced 2×2 legibility (including `%`), independent centering,
      omission of `percent`, repeated settings changes, and disable restoration.
- [ ] If battery behavior differs, capture the new `[Battery]`, `[Layout]`, and
      `[Lifecycle]` lines; the candidate now logs native slot classes, inner
      battery structure, visibility map, and resolved cell coordinates.
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
