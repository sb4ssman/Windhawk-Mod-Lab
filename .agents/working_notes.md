# Working Notes — Windhawk Mod Lab

Reconciled 2026-09-09 against git, live GitHub, installed sources/settings and
Windhawk process status. Historical details and all deferred work remain in
[the previous handoff](knowledge/lab-handoff-before-2026-09-09.md); its status
claims are historical and often superseded. Durable rules: [README](README.md).

## RESOLVED — SystemTrayFrameGrid StackPanel break, live-confirmed Sept 18

KB5129195 changed the tray panel's type while keeping its name, so every
`.try_as<Grid>()` returned null and four mods silently injected nothing.
Diagnosis and fix shape:
[_research/systemtrayframegrid-stackpanel-2026-09.md](../_research/systemtrayframegrid-stackpanel-2026-09.md).
Reported upstream as issue #5530.

`injected-grid-column.h` v1.3 leases a slot on the `Panel` base — a column on a
Grid, a child index on a StackPanel — and all four mods classify the tray on
every injection. Template embedded in Folder Menus, Privacy Anchor and Tray
Utility; VD Switcher carries the same fork inline.

**User live-tested all four on 26200.9457 (Sept 18) and all four work.** Folder
Menus, Privacy Anchor, VD Switcher and Tray Utility all inject correctly. The
anchor-failure diagnostics never fired, so MainStack / NotifyIconStack /
ControlCenterButton / NotificationCenterButton / ShowDesktopStack are still
direct children of the renamed panel. Folder Menus' settings did not carry over,
which is the documented 0.7 break, not a defect.

Still pending: no @version bumps, no commits, nothing pushed, no reply posted to
issue #5530. Replying there would help other affected mod authors.

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
| VD Switcher | Lab 2.0 plus DWM hover previews; tray injection confirmed Sept 18 | Catalog 1.7; PR #4844 still 1.8 | Re-test themed preview chrome |
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

## PUBLISHED — all six PRs live and green, Sept 18

Fork main re-pointed to upstream/main (was 240 behind) and pushed. Every PR
diff verified as exactly one file against upstream/main before pushing.

| PR | Mod | Ver | State |
|---|---|---|---|
| #4443 | Clock Spacer | 1.1 | pushed eba63e7a, 5/5 green |
| #4843 | Privacy Anchor | 2.0 | pushed 6e44777d, 5/5 green |
| #4844 | VD Switcher | 2.0 | pushed 9681854d, title updated 1.8 -> 2.0, 5/5 green |
| #4855 | OmniButton | 2.0 | ALREADY CURRENT — not touched, 5/5 green |
| #5568 | Folder Menus | 2.0 | NEW update PR, fixes #5530, 5/5 green |
| #5569 | Tray Utility | 2.0 | NEW update PR, 5/5 green |

OmniButton needed no push: its branch was already byte-identical to the lab.
An earlier "8,392 changed lines" reading was CRLF noise from a raw byte diff —
always normalize line endings before quoting a diff size for these files.

Awaiting maintainer review. Nothing else to push.

## Open issues on the user's mods (verified against GitHub Sept 18)

- **#5530** Folder Menus / KB5129195 — OPEN, `mod-bug`. EvEric99 reported with
  a debug log; Vc-86 and Jax765 confirmed. User replied "I'm working on it."
  **The fix is now in PR #5568.** No reply posted yet — user parked this
  deliberately; return to it.
- **#4830** VD Switcher custom indicator (Deen-0x) — **all three asks shipped
  in 2.0**: custom Active/Inactive symbols, fonts for labels and Task View,
  and native Checked/CheckedPointerOver/CheckedPressed states for Styler.
  Closeable once #4844 merges.
- **#4831** VD Switcher multi-monitor (Deen-0x) — only PARTIALLY addressed.
  `Placement.AllTaskbars` exists, but Start positions remain primary-only,
  which is exactly their complaint. Do not imply it is fixed.
- **#5049 / #5050** — same StackPanel root cause on 26H2 build 26300,
  reported 2026-08-08, attributed to "the movable taskbar work". Confirms the
  StackPanel is the forward shape, not a revertible regression.
- **#2063** insecure LoadLibrary — none of the user's mods are listed; they
  use `LOAD_LIBRARY_SEARCH_SYSTEM32`. No action.

## Superseded — all six live-tested, batch push (done)

User live-tested the whole family on 26200.9457 (Sept 18): Folder Menus,
Privacy Anchor, VD Switcher, Tray Utility, then OmniButton, Clock Spacer and
the themed VD preview. All confirmed good.

Local state: Folder Menus bumped 0.7 -> 2.0 (source header and init log; the
"Upgrading from 0.7" section correctly still names the published version).
Root README status lines updated from "awaiting live test" to live-tested.
All six COMPILE_OK, EXIT_TIME_DESTRUCTOR_AUDIT_OK, README_MATCH and
SUBMISSION_PREFLIGHT_OK — Folder Menus' reused-version warning cleared with
the bump.

Not done, awaiting explicit go-ahead: nothing pushed, no PR edits, no reply to
issue #5530. Fork main is 240 behind upstream/main and must be re-pointed
before update branches are cut for the two merged mods.

Push plan per mod:
- Privacy Anchor #4843, OmniButton #4855, Clock Spacer #4443 — unmerged add
  PRs; push the branch to update the PR in place, no version bump needed.
- VD Switcher #4844 — open update PR at 1.8, lab is 2.0; push updates it.
- Tray Utility (#4841 merged at 1.1) and Folder Menus (#4485 merged at 0.7) —
  both need update branches cut fresh FROM upstream/main, one file each.

## Active — VD Switcher hover preview chrome

User's Sept 18 verdict: previews work, but the chrome was ugly. It painted with
`COLOR_INFOBK` / `COLOR_INFOTEXT`, the legacy tooltip palette, which renders
pale yellow and does not follow the light/dark setting at all — the Win32 system
colors never did.

Done Sept 18, NOT yet live-tested: the preview derives its palette from
`Themes\Personalize\SystemUsesLightTheme` (taskbar key, `AppsUseLightTheme`
fallback, light when absent) and re-reads it per hover so a theme switch during
an Explorer session is picked up. Added DWM rounded corners, immersive dark
mode and a themed border. Default `Behavior.PreviewWidth` 360 -> 320 per user,
synchronized across source header, settings block and both READMEs. COMPILE_OK.

Next: user re-tests preview appearance in both light and dark themes.

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
