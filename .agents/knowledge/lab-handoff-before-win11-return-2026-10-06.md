# Historical handoff snapshot - 2026-10-06

Preserved before pruning the working notes for the return to Windows 11.
Status and NEXT claims below are historical; reconcile against current git,
fresh human tests, and live PR state before publishing.

# Working Notes — Windhawk Mod Lab

Reconciled 2026-09-09 against git, live GitHub, installed sources/settings and
Windhawk process status. Historical details and all deferred work remain in
[the previous handoff](knowledge/lab-handoff-before-2026-09-09.md); its status
claims are historical and often superseded. Durable rules: [README](README.md).

## ACTIVE - Windows 10 VD Switcher (Oct 6)

Sole current priority: taskbar-vd-switcher on Windows 10. Other mod work is
deferred; mention only easy/tangential compatibility reuse. Machine: Windows
10 Home 22H2, 19045.7725; started clean at 2870100.

Next: replace the classic placement approach to support only three Windows 10
positions: beside the hidden-icons chevron, between clock and notifications,
and after notifications. Remove the gap introduced by attempted Start placement.
User Oct 6 live result: partial success; switcher stays left of chevron for all
positions, some choices leave a large blank gap. This is NOT full live approval.
Installed Windhawk is 1.7.3. Official 2.0 alpha 6 adds dynamicSelect and showIf/
hideIf; 1.7.3 cannot hide OS-inapplicable dropdown options automatically.
Header stays 2.1; no new templates. No push/PR update before full confirmation.

## Deferred - 2.1 pass: every mod works on every taskbar edge (Oct 4)

User direction (Oct 4): fix the family for Windows' native taskbar positions
(Bottom/Top/Left/Right, Sept 2026 update) before any more testing or pushing.
Every changed mod goes to 2.1; one live test per mod at the end. Clock Spacer
is unaffected (it only rewrites `%s%` inside TCC's clock text) and stays 1.1.
The untested Sept 22 round (`e37222d`) and the Privacy Anchor name fix ride
along in the same pass. Machine is now 26300.9550 (26H2); native positioning
is active on it. Clock height on a side taskbar: Styler's job, out of scope.

Decided:
- READ Windows' own edge per taskbar (XAML visual states `DockingStates` /
  `OrientationStates`), like m417z — not the aspect-ratio test.
- Arrangement strings stay one string for all edges. Precedence: `()`, then
  `,`, then `|`. **Everything is LITERAL on every edge (user, Oct 6, after
  live-testing both transposition schemes):** a written arrangement is laid
  out exactly as written, nudges/padding/offsets are screen pixels, and
  screen-named places mean what they say. Only GENERATED layout adapts:
  `auto` and the appended block fill across a side taskbar's width (component
  `across` parameter). No mirroring. Transposition code is deleted.
  Facts: [_research/taskbar-orientation-2026-10.md](../_research/taskbar-orientation-2026-10.md).
- If a layout reads badly on another edge, the user re-nudges. No per-edge
  override setting.

Progress (Oct 4):
- Tree dumps DONE (findings in the research note). Styler clock rule recorded
  in [knowledge/styler-recipes.md](knowledge/styler-recipes.md).
- Shared code DONE: `taskbar-metrics` gained `ReadDockedEdge` (RootGrid
  DockingStates), `CanArrange` (stand down only when ROTATED: window runs down
  a side while Windows reports a horizontal edge, or edge unknown),
  `RunsDownSide`, `EdgeName`, `StartEdgeWatch`/`StopEdgeWatch` (TaskbarFrame
  SizeChanged; a move is a re-layout, not a rebuild). `LayoutModelApplies`
  kept as a pruned legacy part until every mod migrates. Both arrangement
  components gained `Config::transposed` (padX along / padY across, nudges
  swap). Tests: `_templates/tests/arrangement-transpose-tests.cpp` pass.
- OmniButton 2.1 DONE, preflight OK, NOT live-tested: items host is a Panel
  (StackPanel bottom/top, WrapGrid sides — Windows swaps it in place);
  WrapGrid set to one footprint cell per row; slot origins measured on the
  WrapGrid; button MinHeight on sides (Windows pins Height 38); offsets swap;
  edge watch + "host detached" check schedule a full re-apply. README updated.
- Tray Utility, Privacy Anchor, Folder Menus, VD Switcher 2.1 DONE, all six
  `SUBMISSION_PREFLIGHT_OK` (Clock Spacer unchanged at 1.1). NOT live-tested.
  `start-lane-placement` is axis-aware (Left/Right of Start = above/below on
  a side taskbar); Folder Menus now assembles `taskbar-metrics`; VD's own
  Start placement and hover preview are side-aware. `LayoutModelApplies`
  removed from the component (no users). Recipe + settings-profiles docs
  updated to the edge model.
- Oct 6 live test, round 1 (Windows updated again; clock height now right
  natively): Clock Spacer OK. OmniButton OK at bottom, side "a disaster" —
  screenshots showed only wifi drawn in a tall empty button. Cause: reshaping
  Windows' side WrapGrid to one footprint cell per row pushed volume and the
  battery slot into rows outside the grid's height, which are never drawn.
  FIXED (preflight OK, untested): the side path now leaves the WrapGrid as
  Windows built it (only MinHeight raised), keeps every slot in its native
  cell, and centers each drawn glyph on its arranged cell by RenderTransform
  (`ApplySideTaskbarLayout`). Bottom/top StackPanel path untouched. VD OK bottom/top; on sides "Fill order did
  nothing" = all desktops fit one line across 160 px (not a bug), plus a real
  bug fixed: transposition inverted "rows first" and Task View above/below
  on screen. Nudges made screen-literal family-wide (all five preflight OK).
- Oct 6 round 2: OmniButton side path draws all items now. User's
  `percent | battery | volume | wifi` showed `wifi` is not a token (network
  was appended) — `wifi` added as an alias and unknown names are logged.
  Bottom->top left the OmniButton low until a re-apply: the edge watch now
  also follows RootGrid's DockingStates (fires on same-size moves). Written
  shapes made literal (above). Tests renamed
  `_templates/tests/arrangement-side-taskbar-tests.cpp` (pass). All five
  preflight OK, untested.
- **Tree-dump tool to be PUBLISHED** (user, Oct 6) as "Mod-Lab: Taskbar
  Tree Dump" (name set by user). Polished to v1.0, moved Oct 6 to
  [tool-mods/mod-lab-taskbar-tree-dump/](../tool-mods/mod-lab-taskbar-tree-dump/),
  `SUBMISSION_PREFLIGHT_OK`, untested since the
  rewrite: configurable output folder (env vars, default
  `%USERPROFILE%\Documents\Taskbar Tree Dumps`), text/JSON, subtree filter,
  text content off by default (privacy), dump on load/change toggles,
  deterministic output (no HWNDs/pointers). Identity CONFIRMED by the user
  Oct 6: `@id` `mod-lab-taskbar-tree-dump`, `@name` "Mod-Lab: Taskbar Tree
  Dump", folder `tool-mods/` (recorded in mod-identity.md). Publishes with
  the 2.1 batch, after the live test.
- **OmniButton side-taskbar spacing** (Oct 6): user wants defaults right
  with NO nudges ("get it right once ourselves"). Tree dumps showed the
  centre was already right (WrapGrid 160@0, highlight 152@4, both centred on
  80); the uneven gaps came from slot-measured cells (network slot gets the
  WrapGrid's first-cell 4px pad -> 28 vs 24; battery 20 and percent text+2
  carry no inset). Side cells are now drawn width + 4px each side (even 8px
  gaps); bottom/top sizing unchanged. Measured widths are forgotten on every
  edge change. Preflight OK, untested: retest left with zero nudges.
- **User wants VD Switcher on Windows 10** (two Win10 machines). Now the active priority above; classic toolbar candidate awaits live testing.
- **Next: ONE live test of all six:**
  [outputs/live-test-2026-10-04.md](outputs/live-test-2026-10-04.md)
  (supersedes the Sept 22 guide; its items are folded in). Then: decide the
  Privacy Anchor PR rename, push, reply on #5568/#5530, `/ai-review`.

Next (superseded): user runs the tree-dump diagnostic
(now [tool-mods/mod-lab-taskbar-tree-dump/](../tool-mods/mod-lab-taskbar-tree-dump/), writes to
`_research/tree-dumps/`) at all four edges. Then shared components (edge
detection, edge transform, edge/size watcher, narrowed stand-down, side-taskbar
anchors), then mods in order: OmniButton, Tray Utility, Privacy Anchor,
Folder Menus (answers PeTomczyk on #5568), VD Switcher.

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

## BLOCKING — Privacy Indicator Anchor was renamed by an agent (Sept 24)

The mod's real name is **Privacy Indicator Anchor** / `privacy-indicator-anchor`.
An agent added a `tray-` prefix; it went out with PR #4843 on 2026-08-03 and
every note since recorded the wrong name as fact. The user caught it Sept 24.
Canonical values now live in [mod-identity.md](mod-identity.md) — read it every
session, and never edit a mod's `@id`, `@name` or `@description`.

Corrected in the lab (preflight OK): source header, folder README, embedded
README, `tools/package-test-candidates.ps1`. Archives and the frozen
`outputs/test-candidates-2026-09-11/` snapshots keep the old name as history.

STILL WRONG, and not yet acted on:
- PR #4843's title and its file path, `mods/tray-privacy-indicator-anchor.wh.cpp`.
- The fork branch `add-tray-privacy-indicator-anchor` (a branch cannot be
  renamed on an open PR).
- The user's installed copy: Windhawk keys settings by `@id`, so this reinstalls
  as a new mod and the old `local@tray-privacy-indicator-anchor` entry remains.

How to resolve each of these has NOT been researched or decided. Do not guess,
and do not push anything, until the user settles it.

## ACTIVE — round 5/6 fixes done Sept 22; awaiting ONE live test of all six

User directed: fix the three blocking items AND take every optional item.
All done in the lab, uncommitted, nothing pushed. Every mod
`SUBMISSION_PREFLIGHT_OK`. Test guide:
[outputs/live-test-2026-09-22.md](outputs/live-test-2026-09-22.md) — the key
test is a taskbar REBUILD (display-scale change), not an Explorer restart.
Reviews, analysis and the six drafted replies:
[_research/ai-reviews-2026-09-21/](../_research/ai-reviews-2026-09-21/).

After the user confirms: push each lab file to its PR branch (one-file diff vs
upstream/main), post each reply from PowerShell, then `/ai-review` on all six.
Replies correct round 5's false "removed" claims (#4844, #4843, #5568).

Shared changes this round (all adopters re-assembled):
- `assemble.py` prunes `//@part Name` ... `//@end` blocks nothing references
  (comments/strings ignored). Documented in `_templates/README.md`. Renamed
  collision-prone API: `LoadString`→`LoadStringSetting`, lease
  `Count/Empty`→`SnapshotCount/HasSnapshots`, slot-lease anchor `Acquire`→
  `AcquireAtAnchor`.
- `bounded-retry`: `forceFirstAttempt` removed; new `StartOrWake` (never waits,
  first attempt forced) and `StopRequested`. Tests:
  `_templates/tests/bounded-retry-tests.cpp` (pass, build `-static`).
- arrangement components: `ComputeTree`; uncached `Measure`/`Arrange` removed;
  OmniButton moved to the plain (non-axis) variant. Legacy layout suite passes
  against the axis component through a test-only `AvailableRows` shim.
- property-lease `Refresh` (Privacy Anchor's stale Visibility snapshot).
- Exit-time audit now rejects `no_destroy` on a `RetryLoop`; removed in 4 mods.

## Superseded — round 5/6 AI reviews requested Sept 21; wait for them

User live-tested all six (Sept 21, all "looking good"). Pushed, CI 5/5 green on
all six, replies posted on five, `/ai-review` posted on all six; every PR is
`waiting-for-ai-review`. New heads: #4443 `a78f6636`, #4843 `b77afb23`,
#4844 `b5a24403`, #4855 `c48a4f1e`, #5568 `20abef99`, #5569 `2f2a7da7`.
Lab commit that was pushed: `6cf1320`.

Next: when the reviews land, save the bodies into a new
`_research/ai-reviews-<date>/` folder and verify each finding against the
source before acting. `/ready-for-reviewer` only on a clean round.

## Superseded — all six round-4/5 review fixes done; awaiting ONE live test

Every finding in the round-4/5 reviews is resolved in the lab (Sept 20-21),
optional and functionality items included. Reviews, the verified per-mod
analysis and the drafted replies are in
[../_research/ai-reviews-2026-09-20/](../_research/ai-reviews-2026-09-20/)
(`replies/` holds one draft per PR). Do not re-fetch unless a NEW round lands.

**The user is live-testing all six at once**, following
[outputs/live-test-2026-09-21.md](outputs/live-test-2026-09-21.md). Nothing
pushed; PR heads unchanged:

| PR | Mod | Head reviewed | Round |
|---|---|---|---|
| #4855 | OmniButton | `6a78c667` | 4 ("looks good to merge") |
| #4443 | Clock Spacer | `4ecbcc6a` | 4 |
| #4843 | Privacy Anchor | `1139b8b2` | 4 |
| #4844 | VD Switcher | `3cce707b` | 4 |
| #5568 | Folder Menus | `ed1d26f6` | 5 |
| #5569 | Tray Utility | `b25bf87a` | 5 |

After the user confirms: push each lab file to its PR branch (one-file diff
against upstream/main), post each drafted reply from PowerShell, then
`/ai-review`. `/ready-for-reviewer` only once a round comes back clean — the bot
refuses it when its recorded SHA is not the current head. OmniButton has no
reply to post; push, then `/ai-review`.

### Standing decisions (user, 2026-09-20) — still in force

1. **Mods are ASSEMBLED from `_templates/components`**, never copy-pasted.
   All five taskbar mods are now assembled; Clock Spacer uses only plain
   file-scope helpers. Every mod reports `TEMPLATE_PARITY_OK (0 embedded)`.
2. **The legacy-settings bridge is REMOVED** (VD Switcher, Folder Menus, Tray
   Utility). Windhawk cannot write settings, so any bridge leaves the UI showing
   one value while the mod uses another. Do not re-add it.

### Shared-component changes this round (all adopters re-assembled)

- `ui-thread-dispatch`: `Dispatch::ran` — concurrent dispatches no longer run a
  callback once per installed hook.
- `tray-slot-lease`: `AcquireAt` releases a stale same-named marker unless the
  caller's own lease holds it (then it still refuses).
- `sync-readme.py`: `](../` lab links become repository URLs in the embedded
  readme (verify-readme-sync already mapped them back).

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
