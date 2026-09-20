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

## ACTIVE — acting on the six AI reviews, one mod at a time

All six reviews came back 2026-09-19. Saved copies are gone with the session;
re-fetch with `gh pr view <n> --repo ramensoftware/windhawk-mods --json comments`
and take the LAST `windhawk-reviewer` comment.

**DONE AND PUBLISHED — all six live-tested, pushed, and re-reviewed.**
User live-tested all six on 2026-09-19 and confirmed every one works as
expected. All six pushed, 5/5 CI green, `/ai-review` posted and accepted
(labels moved `waiting-for-author` -> `waiting-for-ai-review`).

| PR | Mod | New head | CI |
|---|---|---|---|
| #4443 | Clock Spacer | `4ecbcc6a` | 5/5 |
| #4843 | Privacy Anchor | `1139b8b2` | 5/5 |
| #4844 | VD Switcher | `3cce707b` | 5/5 |
| #4855 | OmniButton | `6a78c667` | 5/5 |
| #5568 | Folder Menus | `ed1d26f6` | 5/5 |
| #5569 | Tray Utility | `b25bf87a` | 5/5 |

Each branch was verified as exactly one `mods/*.wh.cpp` diff against
`upstream/main` before and after committing, and each pushed file was
confirmed byte-identical to the live-tested lab source.

**Next: wait for the six AI reviews, then act on them.** Do not post
`/ready-for-reviewer` until a review comes back and its findings are
answered — and remember the bot refuses it when its recorded SHA is not the
current head.

Every mod is COMPILE + preflight + template-parity green.

| Mod | Review fixes | State |
|---|---|---|
| Tray Utility #5569 | DONE (all 3 review rounds) | preflight green; also the one ASSEMBLED mod |
| Folder Menus #5568 | DONE (5 blocking + 7 optional) | preflight green |
| OmniButton #4855 | DONE (1 blocking + carried-over) | preflight green |
| VD Switcher #4844 | DONE (5 blocking + 7 optional) | preflight green |
| Privacy Anchor #4843 | DONE (all 5 blocking + 5 of 7 optional) | preflight + parity green |
| Clock Spacer #4443 | DONE (all 3 blocking + most optional) | preflight green; 1709 -> 1017 lines |
| Task Manager Tail | n/a — fixed 2 pre-existing preflight failures | green but for the expected reused-version warning |

### Deliberately NOT done, and why

- **Privacy Anchor optional: `g_startLease` to `optional<>` + `reset()`.** The
  bare `[[clang::no_destroy]]` aggregate works because `Release` assigns
  `lease = {}`. Cosmetic consistency only; left for a quieter pass.
- **VD Switcher has the same `ClearValue` bug class the start-placement
  template just had.** `SetStartButtonVisualOffset` (~line 4267) clears
  `RenderTransformProperty` on the Start button instead of restoring the prior
  local value, so releasing it destroys another mod's transform on Start. Its
  own reviewer did NOT raise this, and that mod's review work is complete and
  verified, so it was recorded rather than changed. Fix it deliberately, with a
  live test, not as a drive-by.
- **Clock Spacer's standalone-vs-fold-into-TCC question** is the maintainer's
  call; the review says so explicitly. No action.

### The reviewer was WRONG about one thing — do not "fix" it again

`-loleaut32` has no `BSTR`/`VARIANT`/`IDispatch` user *in the source*, and the
reviewer suggested dropping it on both Privacy Anchor and (previously) other
mods. It is still REQUIRED: C++/WinRT stores `hresult_error::message()` in a
`BSTR`, so removing it produced three undefined symbols (`SysStringLen`,
`SysFreeString`) at LINK time. Verified 2026-09-19 by actually removing it.
This is why `compile-check.ps1` links a real DLL instead of `-fsyntax-only`.
Recorded in the submission checklist; answer the suggestion with the link error.

### Pattern established across the family — reuse it

Three mods now carry the same **legacy-settings fallback** for the 2.0 key
rename, because the reviewer raised it on every one: a `Behavior.Use*` switch,
a `HasLegacySettings()` probe (Windhawk cannot tell "unset" from "zero", so
prove the old config exists before trusting any single key), and
`PreferCurrentOrLegacy{String,Int}` helpers that let a 2.0 key win as soon as
it differs from its declared default. Folder Menus and VD Switcher use the
`legacyDefault` parameter form, which is the better one — Tray Utility's older
form uses `legacy == 0` instead. **Clock Spacer will need the same treatment if
its review raises it.**

Do NOT add a fallback for a boolean whose off state is the default: the legacy
"on" then wins forever and the 2.0 switch can never turn it off. That is why
Tray Utility's logging key was excluded before the setting was removed.

Nothing is committed, pushed, or live-tested. PR heads are unchanged from the
table below.

| PR | Mod | Head |
|---|---|---|
| #4443 | Clock Spacer | `ae7869a2` |
| #4843 | Privacy Anchor | `06f01b57` |
| #4844 | VD Switcher | `934df900` |
| #4855 | OmniButton | `3b03963c` |
| #5568 | Folder Menus | `f7e905b3` |
| #5569 | Tray Utility | `0e68eb34` |

Every prior published branch was rechecked as exactly one `mods/*.wh.cpp` diff
from `upstream/main`; each had all five CI checks green. Tray Utility #5569 is
now `waiting-for-author` on its prior head; do not post `/ai-review` again
until this new candidate is live-tested and pushed. The other five remain in
their AI-review flow.

Reusable review lessons are now in the taskbar host/lifecycle templates and
submission checklist: distinguish ownership from stale-tree state; force an
initial settings reapply without losing ownership; revoke explicit XAML
callbacks even after partial injection or a failed UI dispatch; retain and wait
worker handles; and give COM-owned monitors heap/reference-counted lifetime.

Retain the #4855 note: memoization fixes exponential Measure; the depth-24
limit only bounds stack depth.

### Housekeeping not done

Fork `main` is at `7d26c6c1` while `upstream/main` is at `68ea8919`. That did
NOT matter for this push, because every branch already existed and was only
added to — nothing was cut from `main`. Re-point it (`git branch -f main
upstream/main` + `--force-with-lease`) BEFORE cutting any new update branch,
or the new branch inherits whatever `main` is carrying.

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
