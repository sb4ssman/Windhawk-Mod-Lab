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

## ACTIVE — round 4/5 reviews: OmniButton done, five mods to go

All six reviews came back 2026-09-20 against the pushed heads. **The bodies and
a VERIFIED per-mod plan are saved in
[../_research/ai-reviews-2026-09-20/](../_research/ai-reviews-2026-09-20/) —
read `README.md` there first.** Every finding in it was checked against the
source; the ones that do not hold up are marked, with the evidence.

Do not re-fetch the reviews unless a NEW round has landed.

| PR | Mod | Head reviewed | Round | State |
|---|---|---|---|---|
| #4855 | OmniButton | `6a78c667` | 4 | **"No blocking issues — looks good to merge."** All items taken. DONE, awaiting live test |
| #4443 | Clock Spacer | `4ecbcc6a` | 4 | Code CLEAN per reviewer. README + pushback only |
| #4843 | Privacy Anchor | `1139b8b2` | 4 | **DONE in lab Sept 20, assembled, preflight OK. Awaiting live test.** Reply drafted in `_research/ai-reviews-2026-09-20/replies/` |
| #4844 | VD Switcher | `3cce707b` | 4 | 2 blocking, both real, both in the 1.7 bridge |
| #5568 | Folder Menus | `ed1d26f6` | 5 | **DONE in lab Sept 20, assembled, preflight OK, bridge removed. Awaiting live test.** Reply drafted |
| #5569 | Tray Utility | `b25bf87a` | 5 | 1 blocking, a real stand-down regression |

### GOAL: get all six to live-testable, then submit. Resolve EVERYTHING per mod

The user wants the reviewer HAPPY and the mods FINISHED — optional and
functionality items included, not just blocking ones. Push back only where a
finding genuinely does not hold; the README in `_research/` lists the four
places where that applies and gives the evidence for each.

### Two standing decisions (both from the user, 2026-09-20)

1. **Mods are ASSEMBLED from components, never verbatim copy-paste.**
   `python _templates/assemble.py <mod>` + `<mod>/components.list`. This
   instruction had been given before and only half-executed; do not leave it
   half-done again. Conversion recipe is in the `_research/` README.
   Converted: **OmniButton, Tray Utility, Privacy Anchor, Folder Menus**.
   Not yet: VD Switcher.
2. **The legacy-settings bridge is REMOVED** from VD Switcher, Folder Menus and
   Tray Utility. READMEs say settings were reorganised in 2.0 and must be
   re-applied once. Reason (do not re-add it): Windhawk cannot write settings,
   so any bridge means the UI shows one value while the mod uses another, and
   "prefer legacy when the 2.0 value equals its default" makes a carried value
   impossible to reset. It already caused two real mis-migrations in VD Switcher.

### EXPECTED FAILURE, do not "fix" it the wrong way

`_templates/taskbar-host.h` was changed (the `RetryLoop` orphan race), so
**TEMPLATE_PARITY now FAILS for VD Switcher** — the last mod that still embeds
it verbatim (Privacy Anchor and Folder Menus are converted and out of it). The correct resolution is to CONVERT
those three to assembly, which removes the `windhawk_mod_templates::*`
namespaces entirely and makes parity moot. Do not re-embed the template to go
green; that is throwaway work.

### What changed in the component library this session

- New: `color-tokens`, `native-glyph-surface`, `arrangement-expression-axis`
  (superset of `arrangement-expression` adding axis-relative sizing; OmniButton
  and VD Switcher need it, the other adopters must keep the reduced one or they
  regress).
- `settings-values` gained `LoadString` / `LoadChoice` built on
  `WindhawkUtils::StringSetting`, retiring the duplicate `sio::StringSetting`
  class the reviewer flagged on four mods.
- `bounded-retry` + `taskbar-host.h`: `RetryLoop::Start` publishes by
  `std::exchange` under the mutex and waits for a displaced run, closing the
  orphan race raised on OmniButton and Tray Utility. Needs `<utility>`.
- Deleted as dead in every mod: `ngl::PixelsToDip`, `ngl::AvailableRows`,
  `vtw::FindInnerStackPanel`.

### Order to work the remaining five, worst defect first

1. ~~Privacy Anchor~~ — done Sept 20. Live-test checklist: (a) default
   settings, all four icons show, natives hidden; (b) turn `Content.Camera`
   off, open the camera: Windows' own camera glyph must appear; (c) all four
   Content off: every native indicator visible, no bar; (d) toggle settings
   and disable/enable the mod: no stranded hidden natives; (e) glow on/off.
2. ~~Folder Menus~~ — done Sept 20. Live-test checklist: (a) enable the mod
   on a running Explorer with "Use native Shell icon" on: icons appear
   without an Explorer restart; (b) toggle that setting: icons follow;
   (c) right-click an item → Delete, leave the confirmation open, disable
   the mod: unload waits, answer the dialog, no crash; (d) same with a folder
   popup left open; (e) settings reset to defaults is expected (0.7 bridge
   removed) — re-enter folders; (f) Desktop menu shows no duplicates.
3. **Tray Utility** — transient states permanently retire the retry.
4. **VD Switcher** — bridge removal covers both blocking findings.
5. **Clock Spacer** — README plus the pushback; code already clean.

Nothing is committed to the PR branches. PR heads are unchanged from the table
above; the lab has local commits only.

## Superseded — AI review fixes written before the live test

The upstream two-stage review is new (`pr_flow.cjs`, enabled for all PRs
2026-07-30). Authors must comment `/ai-review`, then `/ready-for-reviewer`;
until then a PR sits in `waiting-for-author` and no human sees it. That is why
nothing merged since July — every PR predates the system and the four older
ones never even got the bot's instruction comment. `/ai-review` is now posted
on all six.

**Post the commands from PowerShell, never Git Bash** — MSYS path conversion
rewrites `/ai-review` into `C:/Program Files/Git/ai-review`. That happened, was
posted to five PRs, and was deleted and reposted.

OmniButton's review came back with five findings. All five verified as real
against the code and fixed. **None of it is live-tested, so nothing is pushed.**

- Percent-text watcher recorded from `g_batteryPercentFE` only when that was
  itself a TextBlock, but compared against the probed surface's TextBlock
  several levels deeper — never matched, so every layout pass re-applied
  forever. Now records from the same element the watcher reads.
- `Wh_ModUninit` preferred a cached HWND without `IsWindow`; a stale handle
  skipped teardown entirely, leaving Loaded delegates pointing into an image
  Windhawk frees on return. Now validated, and callbacks are revoked even when
  dispatch fails.
- `LayoutUpdated` could re-enter itself through `UpdateLayout()`. Added a
  re-entrancy guard and deleted the `[Geometry]` diagnostic that forced the
  synchronous layout pass (its arguments ran even with logging off).
- Deep arrangement expressions were exponential. See template note below.
- `Placement.Status` was a text box nothing read; removed with its group, and
  both READMEs updated.

Next: user live-tests OmniButton, then the other five (all carry template
changes). Then push, then `/ai-review` again — the bot refuses
`/ready-for-reviewer` when its recorded SHA is not the current head.

## Templates and preflight hardened from the review

- `nested-group-layout.h` v2.0 -> v2.6: **Measure is memoized.** Each group
  measured every child twice and Arrange re-measured at every level, so cost
  doubled per level; the grammar wraps each unit in its own group, so every
  "(" added two levels and ~16 nested parens reached roughly 4^16 visits — a
  frozen Explorer. The reviewer's suggested depth cap of 16 would NOT have
  fixed this: 4^16 is reachable at exactly that cap. The cap is now 24 and
  exists only to bound stack depth. Re-embedded in all five adopters, plus
  `#include <unordered_map>` where the namespace-only embed needed it.
- `taskbar-host.h` v1.0 -> v1.1: added `ResolveTaskbarWnd(cached)`, which
  validates with `IsWindow` before preferring a cached handle. The unsafe
  ternary existed in four mods; all replaced.
- New `_templates/verify-settings-used.py` — every declared setting must be
  read by the code. Catches the `Placement.Status` class AND the far worse
  case of a setting renamed in the block but not in the loader, which Windhawk
  answers with a default instead of an error. Wired into preflight.
- Preflight also rejects the unvalidated cached-handle ternary.
- `submission-checklist.md` gained a "Review items a script cannot decide"
  section for the four lessons that need the call graph to judge.
- Preflight now selects a Python 3.12+ interpreter **with pyyaml**: upstream's
  validator deps use PEP 695 `type` aliases, which are a syntax error on the
  3.10 that was on PATH. Installed pyyaml into 3.12. Without this the
  validator step fails in a way that looks like a mod defect.
- Tests: 121 existing assertions still pass, plus new deep-nesting regression
  cases (correctness at depth, timing ceiling, clean error past the cap).

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
