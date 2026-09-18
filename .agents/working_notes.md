# Working Notes — Windhawk Mod Lab

Reconciled 2026-09-09 against git, live GitHub, installed sources/settings and
Windhawk process status. Historical details and all deferred work remain in
[the previous handoff](knowledge/lab-handoff-before-2026-09-09.md); its status
claims are historical and often superseded. Durable rules: [README](README.md).

## BLOCKER — SystemTrayFrameGrid is a StackPanel on 26200.9457

KB5129195 changed the tray panel's type while keeping its name. Every
`.try_as<Grid>()` on `SystemTrayFrameGrid` now returns null, so Folder Menus,
Privacy Anchor, VD Switcher and Tray Utility all silently inject nothing.
Diagnosis, evidence and fix shape:
[_research/systemtrayframegrid-stackpanel-2026-09.md](../_research/systemtrayframegrid-stackpanel-2026-09.md).
Reported upstream as issue #5530; user has replied that a fix is in progress.

This lands ahead of the test batch: the Sept 9 confirmations were made on
26200.9278, before this update. Re-validate on the current build.

**Fix written Sept 18, NOT live-tested.** `injected-grid-column.h` v1.3 leases a
slot on the `Panel` base — a column on a Grid, a child index on a StackPanel —
and all four mods classify the tray on every injection. Refreshed template
embedded in Folder Menus, Privacy Anchor and Tray Utility (parity verified); VD
Switcher carries the same fork inline. All four COMPILE_OK and pass preflight
(Folder Menus' only warning is the reserved 0.7 version bump). No @version
bumps, no commits, nothing pushed, no reply posted to issue #5530.

Next: user live-tests all four on 26200.9457. The new logs print the tray
panel's class and its named children when an anchor fails, which settles the
open question of whether MainStack / NotifyIconStack / ControlCenterButton /
NotificationCenterButton / ShowDesktopStack are still direct children there.

## Priority: polish and publish the mod family

OmniButton recovered and the user confirms the controlled ordinary-logging
reload also works. Investigation parked by user: transient failure, cause
unproven. Do not request the same test again or speculate a code fix.

### User confirmations and scope — September 9

- Clock Spacer works; supplied screenshot is included in both synchronized
  READMEs. The newer lab rollout still needs its exact-build live test.
- VD Switcher works. Existing screenshots are sufficient by user direction.
  Hover previews are implemented; live-test the packaged candidate next.
- Tray Utility works. Folder Menus works.
- Folder Menus: **reject independent per-folder placement around the taskbar**.
  One grouped toolbar preserves the intended classic-toolbar scope. This
  supersedes the old open contract question; no external reply sent.
- Task Manager Tail works beautifully on Windows 10 and 11. **Done; leave it
  alone**, including the previously noted local lifecycle delta. No more tests.
- Confirmations apply to the user's working installed builds. Installed vs
  newer lab differences measured earlier remain facts; review those deltas
  before shipping. Do not silently label different binaries live-tested.

## Family status — verified Sept 9

| Mod | Lab / installed | Upstream | Next |
|---|---|---|---|
| OmniButton | 2.0; installed source identical; recovered after reload | PR #4855 open, a6bde2de, five green checks | Recovered; parked by user |
| Privacy Anchor | 2.0 live-confirmed; lab additionally embeds start-placement v1.3 | PR #4843 open, 6f71b39f, v2.0 | Await review |
| VD Switcher | Lab 2.0 plus new DWM hover previews; installed is earlier | Catalog 1.7; PR #4844 still 1.8 | Live-test packaged candidate |
| Clock Spacer | 1.1; newer rollout, docs/screenshot and hook comment reconciled | PR #4443 open, 85a971d5 | Live-test packaged candidate |
| Tray Utility | Lab 2.0 rework d600242; installed local 1.1 | Catalog 1.1; #4841 merged | Live-test 2.0 |
| Folder Menus | New grouped-settings/native-icon/after-app candidate; header retained 0.7 | Catalog 0.7; #4485 merged | Live-test candidate; PR commit will bump to 2.0 |
| Task Manager Tail | 1.1; lab adds child-process termination on unload | Catalog 1.1 | Done; no further work per user |
| System Tray Grid Lines | Concept, no mod source | No submission in user's PR inventory | Unscheduled |
| Positive Confirmation Indicator | Idea only, no mod source | — | Unscheduled; see idea note below |

Old notes wrongly said Tray Utility still needed the rework: git shows it
complete Aug 5, six checks green then, **not live-tested**. Working installed
builds are not evidence of live tests of newer lab code. OmniButton/Privacy
have no maintainer review newer than July 23 despite August PR updates.

## Queue after OmniButton

- Live-test the exact files in outputs/test-candidates-2026-09-11, following
  TESTING.md. It includes installed source/settings backups and a hash manifest.
  VD/Folder have new features; Clock/Tray/Privacy have newer lab deltas.
- VD: label options, Task View placement, Explorer rebuild, vertical stand-down;
  desktop hover previews required; existing screenshots accepted.
- Clock: required StartTaskbar hook/fallback, token behavior, July review items.
- Tray Utility: grouped settings migration, Start settling, restore. Edge flyout
  clipping deferred; do not repeat ineffective SetWindowPos workaround.
- Folder Menus: test after-app placement under crowded taskbars, native icons,
  settings migration, menu behavior and unload/rebuild. Independent per-folder
  placement remains rejected. Contributor credit is in the README.
- Privacy: future Recall/OneDrive and filler/status indicators. Preserve opt-in
  camera monitoring and setting-rename lessons.
- Real vertical-taskbar support remains a user goal, distinct from today's
  vertical OmniButton icon arrangement. Recheck new native Windows behavior.
- **Idea — Positive Confirmation Indicator** (user, Sept 18). A tray indicator
  that mirrors Privacy Anchor, but inverted: Privacy Anchor reports that
  something unwanted is ACTIVE, this one reports that something wanted is
  HEALTHY. Only candidate signal so far: "destructive-command guard active and
  up to date". Unscheduled, no design decisions taken, no source. Open: what
  other signals earn a slot, and how a green-when-fine indicator avoids becoming
  wallpaper the user stops seeing.
- Placement coordination / Discussion #4542 remains TABLED by user direction.
- Remaining extraction, tooling and unscheduled ideas are preserved in previous
  handoff; OmniButton glyph animation remains archived.

## Repository state

Started clean at a42cb6d (Aug 31), main matching cached origin/main. Fork clean
on add-tray-privacy-indicator-anchor; PR branches match cached origin refs.
Fork main is behind cached upstream/main. No fetch/branch changes, commits,
pushes, PR edits, installed-setting edits or process changes this session.

## Test handoff — September 11

- Guide: outputs/release-test-guide.md; packaged copy is TESTING.md.
- Source edits remain uncommitted pending live tests. No @version bumps yet.
- Folder Menus retains its existing bounded retry worker; six other applicable
  templates are embedded verbatim. After-app placement is mod-specific, not a
  restart of the tabled inter-mod placement contract.
- Folder upstream validation has one expected warning: published version 0.7
  reused. Other five source preflights pass. Clock's settings-order checker is
  inapplicable per six-mod-settings-audit.md (companion-specific settings).
- The oversized edit command was rejected, then the user explicitly directed
  reasonable patches. Patches and short named helper scripts completed the work.
  Do not report Folder Menus as permission-blocked.
