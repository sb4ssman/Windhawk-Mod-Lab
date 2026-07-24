# Working Notes — Windhawk Mod Lab

Living todo list — current state only, pruned every session. Completed work
(past tense) goes in [work_log.md](work_log.md). Durable rules live in
[README.md](README.md); reference material in [knowledge/](knowledge/).

## Current focus — START HERE (handoff, 2026-07-24)

**Roll the settings contract + layout template through the remaining mods.**
VD Switcher is finished and green but deliberately NOT shipped: the user wants
the other mods migrated first, because a snag in one of them may feed back into
the template, and it is cheaper to change the template once than to ship VD
Switcher twice.

### Privacy Anchor — local candidate awaiting live test

The first rollout build is now in the working tree, uncommitted and unpushed.
It adopts the grouped settings contract and the exact nested-group layout v2.4
body: semantic `location` / `mic` (`microphone`) / `camera` / `copilot` tokens,
one `Layout.Arrangement`, per-icon and group offsets in the expression,
Content enable switches, and `Layout.NewItems` for newly enabled icons. The old
item-order string, smart-grid modes, fixed rows/columns, and keyed icon offsets
are gone as one clean settings break.

PR #4843's required items are in the candidate: owned strings replace both
dangling `wstring_view`s, camera hardware monitoring defaults false, the three
system-tray modules are named above a neutral hook-array name, no-destroy
ownership follows lifecycle v1.3.1, and taskbar pixels are converted to DIPs
before `RowsInHeight` after reserving `Adjust.PadY`. All optional review ideas
and unrelated implementation changes are deliberately excluded: the original
working detection, monitoring, cleanup, hook, and placement guts stay intact.

Current local gates: `SETTINGS_ORDER_OK`, `COMPILE_OK`,
`EXIT_TIME_DESTRUCTOR_AUDIT_OK`, nested-layout tests pass, and copied template
bodies match. READMEs, screenshots, preflight, commit, and PR update are
intentionally deferred until the user live-tests this exact build.

### What exists now

Nine commits, all local, `main` ahead of `origin/main`. Nothing pushed, no PR
touched.

| Commit | What |
|---|---|
| `60204b3` | Settings contract v2 — `_templates/settings-profiles.md` |
| `9fa5382` | Layout v2.0 + VD Switcher rebuilt on it |
| `7e2b02c` | v2.1 group offsets, located parse errors, token vocabulary |
| `ab28339` | v2.2 axis-relative sizing, MissingTokens/AppendMissing |
| `36084b4` | v2.3 TokenMatcher (alias duplication fix) |
| `b31a5a9` | Task View placement in written arrangements + in-grid |
| `3307b2b` | Task View gap |
| `46132e0` | v2.4 RowsInHeight — reserve before you divide |
| `75ab5b1` | settings-UI wording |

The two files that matter, and they are the contract, not suggestions:

- **`_templates/settings-profiles.md`** — eight nested settings groups
  (Placement, Content, Layout, Size, Adjust, Surface, State, Behavior), fixed
  keys, fixed order. A mod assembles from it; it never invents, renames, or
  reorders. Read this BEFORE touching any mod's settings block.
- **`_templates/nested-group-layout.h` v2.4** — the one arranger, behind the one
  `Layout.Arrangement` field.

Enforcement: `_templates/verify-settings-order.ps1 <mod>` checks a settings
block against the contract. It is deliberately NOT in `submission-preflight.ps1`
yet — wiring it in now would block the review fixes owed on the unmigrated
mods. Fold it into preflight once the family is uniform; that is the last step.

### Rollout order and what each mod owes

Each mod is ONE live-tested build. Never push before the user live-tests — see
memory `feedback_never_push_without_live_test`.

1. **Privacy Anchor (PR #4843)** — required review fixes: the `wstring_view`
   use-after-free over a destroyed `hstring` (take a `std::wstring` copy);
   `cameraHardwareDetection` default to false; rename `systemTrayDllHooks` +
   list all three modules. Its smart-grid row math has the physical-px/DIP bug
   that `ngl::RowsInHeight` now fixes centrally. Icon surface variant of the
   Surface group; dynamic item set, so it needs `Layout.NewItems`.
2. **OmniButton (PR #4855)** — the DPI bug is BLOCKING there and is the same
   fix; also the `systemTrayDllHooks` rename. Fixed item set (wifi, volume,
   battery, percent), so semantic tokens and NO `NewItems`. Its `itemOrder`
   string is REPLACED by `Layout.Arrangement` — an expression already encodes
   order and grouping, and keeping both would be two strings describing one
   thing.
3. **Clock Spacer (PR #4443)** — hooking-surface reduction to
   `DateTimeIconContent::OnApplyTemplate`, remove `(pre-2604)` from a comment
   (it confuses the symbol-cache parser), add a before/after screenshot. It
   tracks an upstream settings language, so it is the one mod that does NOT
   adopt the grouped keys; see the Text panel section of the contract.
4. **Then**: flip `verify-settings-order.ps1` into `submission-preflight.ps1`,
   and only then ship VD Switcher + the rest.

### Lessons the live test bought — expect these again

Four defects, all found by running it rather than reading it. Check each one in
every mod you migrate:

- **Aliases and identity.** `MissingTokens` compared names as strings, so
  `desktop1` looked missing next to expected `1` and got appended AGAIN —
  seven buttons instead of four. Any mod with a token vocabulary MUST pass a
  `TokenMatcher` that compares item identity, not spelling.
- **Axis-relative sizing.** An item that must match its neighbours (a Task View
  button, a separator) cannot take a fixed width and height, because a
  hand-written arrangement decides which way it lands. Use
  `AlongAxis(thickness, span)`; span 0 fills.
- **Reserve before you divide.** `maxRows` is the height the ITEM GRID gets,
  not the taskbar. Subtract outer padY and any row-shaped extra item first, or
  the group overflows. A cosmetic offset is NOT reserved.
- **Stale string comparisons after a rename.** Renaming an option value broke
  `labelFormat == L"dot"` silently (Indicator symbols fell back to numbers).
  After any option rename, grep the mod for the old literal.

Also: `compile-check.ps1` and `exit-time-destructor-audit.ps1` now pass
Windhawk's own `WINVER/_WIN32_WINNT/NTDDI_VERSION` defines. Without them
`GetDpiForWindow` fails locally while building fine in Windhawk — if a lab
script disagrees with Windhawk, suspect the script's flags first.

### VD Switcher v2.0 — done, held

All five gates green: COMPILE_OK, EXIT_TIME_DESTRUCTOR_AUDIT_OK, README_MATCH,
SETTINGS_ORDER_OK, SUBMISSION_PREFLIGHT_OK. Live-tested through four rounds;
the user's last word was "this is fantastic, lets ship it" before choosing to
hold. When it does ship: it is a MAJOR version (settings keys all changed, no
migration — the README's "Upgrading from 1.x" says so), and PR #4844 gets
updated, not merged as-is (the user already told m417z "I am updating it").

Still unverified on real hardware, worth a pass if VD Switcher is revisited:
the in-grid Task View placement at several desktop counts, and the auto layout
at 125%/150% scaling now that the reservation changed the shapes it picks.

**MERGED 2026-07-23:** Taskbar Folder Menus (PR #4485) and Tray Utility
Customizer (PR #4841) are now in the official catalog. No further action.

**Active work: cross-cutting `[[clang::no_destroy]]` resolution.** The
2026-07-23 review wave requests the same no_destroy fix on every remaining open
mod (#4843, #4844, #4855) — it was also the merge-gate on the two that merged.
Curate the definitive resolution into the lifecycle template, then distribute to
each affected mod one live-tested build at a time. VD Switcher already has the
fix (in the unified-placement rebuild, `7c1e4b8`, pending its live test).

VD Switcher rebuild on unified nested-group placement is committed at `7c1e4b8`
(compiles + audit-green, preliminary user test "looking OK"); READMEs + version
bump are deferred until the full live test. See work_log 2026-07-23.

## Open upstream PRs — reconciled 2026-07-24

Two merged; four open, each now carrying a 2026-07-23 maintainer review. Never
push a review fix before a fresh live test.

- Taskbar Folder Menus v0.7 — PR #4485: **MERGED** (`18fe50eb`).
- Tray Utility Customizer v1.1 — PR #4841: **MERGED** (`20aaf8e5`).
- Taskbar VD Switcher v1.8 — PR #4844: OPEN. User replied 2026-07-24 "I am
  updating it, we can hold off on the merge... working [the other comments]
  too." The unified-placement rebuild (`7c1e4b8`) already includes the
  no_destroy conversion; awaits full live test, READMEs, version bump before the
  PR is updated.
- Taskbar Clock Spacer v1.1 — PR #4443: OPEN, ACTION REQUIRED. Review asks: (1)
  why hook everything vs just `DateTimeIconContent::OnApplyTemplate`; (2) remove
  `(pre-2604)` from a comment (breaks symbol-cache parser); (3) drop no_destroy
  on `g_states` (weak_ref release is thread-safe; bare container leaks buffer);
  (4) add before/after screenshot; optional off-thread-cleanup + scan-thread
  race notes.
- Privacy Indicator Anchor v1.0 — PR #4843: OPEN, ACTION REQUIRED. Two real
  bugs: (1) use-after-free — `wstring_view` over a destroyed `hstring` temporary
  on the hot scan path (take a `std::wstring` copy); (2) `cameraHardwareDetection`
  defaults true → touches camera on every Explorer start, make it opt-in false.
  Plus: no_destroy fix; rename `systemTrayDllHooks` (3 modules) + comment; DPI
  mixing (physical px ÷ DIP pitch) in row math. Optional: mic-monitor cleanup
  race, StringSetting RAII, Copilot poll cadence.
- OmniButton Customizer v1.0 — PR #4855: OPEN, ACTION REQUIRED. (1) DPI bug
  (BLOCKING) — `ResolveGeometry` mixes physical px and DIPs → grid clips above
  100% scaling; fix with `MulDiv` + `GetDpiForWindow`. (2) rename
  `systemTrayDllHooks` (3 modules) + comment. Plus: no_destroy fix; test at
  varied DPI. Optional: retry-thread race, IsWindow revalidation, dup
  LoadColorSetting, slotWidth<16 clamp, unused params/includes.

### Cross-cutting themes from the 2026-07-23 review wave
- **`[[clang::no_destroy]]`** — requested on all four open mods; merge-gate on
  the two merged. Being resolved via the lifecycle template (this session).
- **DPI mixing (physical px vs DIPs)** — real bug on #4855 (blocking) and #4843;
  lives in the smart-grid `GetAvailableRows` heuristic shared across mods. The
  unified-placement rollout should centralize a DIP-correct height.
- **`SYMBOL_HOOK` array naming** — must name every target module; #4843, #4855.

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
- [x] FORK `main` RESET — DONE 2026-07-23. `sb4ssman/windhawk-mods` `main` was
      reset onto `upstream/main` and force-pushed; `origin/main` is now
      hash-identical to `upstream/main` (`9f9f096a`) with no
      `vertical-omnibutton.wh.cpp`. Old tip kept as tag
      `backup/fork-main-pre-reset` (`de5feb0f`). All six PR branches verified
      unchanged and still one-file diffs. Landmine defused; keep it that way by
      never branching from fork `main` (see README directive).
- [x] STALE FORK BRANCHES — DONE 2026-07-23. Deleted
      `sb4ssman-taskbar-vd-switcher`, `update-taskbar-vd-switcher`, and
      `sb4ssman-virtual-desktop-switcher` local + remote; `backup/deleted-*`
      tags retained. Fork now carries exactly the six live PR branches + `main`.
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
