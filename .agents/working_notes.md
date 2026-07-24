# Working Notes — Windhawk Mod Lab

Living todo list — current state only, pruned every session. Completed work
(past tense) goes in [work_log.md](work_log.md). Durable rules live in
[README.md](README.md); reference material in [knowledge/](knowledge/).

## Current focus

**Settings contract v2 + layout rewrite (2026-07-24).** The old "hybrid"
Auto/Manual layout surface was rejected by the user as an incoherent
amalgamation and has been thrown out. Replaced by:

- `_templates/settings-profiles.md` — rewritten from a prose profile menu into
  THE settings contract: eight nested groups (Placement, Content, Layout, Size,
  Adjust, Surface, State, Behavior) with fixed keys, labels, defaults, and a
  fixed assembly order. Mods append mod-specific keys after a group's canonical
  ones; they never invent, rename, or reorder. Commit `60204b3`.
- `_templates/nested-group-layout.h` v2.0 — one arranger behind ONE setting,
  `Layout.Arrangement`, default value `auto`. Absorbed the shape choice
  (`ChooseShape`), the DPI-correct row count (`AvailableRows`), and expression
  generation. Per-item offsets ride in the expression as `1[+2,-1]`; the
  keyed-nudge strings, four-side padding, primary axis, and face nudging are
  gone. `smart-grid-layout.h` is marked SUPERSEDED (still embedded in
  OmniButton, Privacy Anchor, Folder Menus).
- Layout v2.1 additions after a user probe of the grammar: parenthesized groups
  take an offset too (`(1, 2)[3,0]` moves the whole column), parse failures
  report the character position and what was expected, tokens match
  case-insensitively with `desktopN` accepted as an alias for `N`, and the
  token-to-label map is logged next to the arrangement. Juxtaposition was
  deliberately NOT made an implicit `|`: a missing separator stays a visible,
  recoverable parse error rather than a silently different layout. Tokens are
  identity, never the displayed label — that rule is now in the contract, and
  it is why custom faces (VD Switcher custom labels, Folder Menus entry names)
  do not become tokens.
- Layout v2.2 from the first live test. Two real defects: the Task View button
  rendered as a small square instead of a full column, and a hand-written
  arrangement silently swallowed newly created desktops. Fixed at template
  level: `AlongAxis(thickness, span)` axis-relative sizing (span 0 = match the
  group, resolved against whichever axis the item lands on, so one pair of
  settings covers column and sliver), and `MissingTokens` + `AppendMissing`
  with a `Layout.NewItems` policy (append / ignore). Also fixed the grammar's
  per-unit wrapper flattening axis-relative sizes: a single-child group now
  passes its child's size through verbatim in both Measure and Arrange.
- Third live test: with a written arrangement the Task View button ignored its
  placement setting and got squashed. Both were the same defect - master was
  only ever APPENDED, and appended INSIDE the auto block next to any new
  desktops, where it becomes one cell of a stack instead of a column. The
  manual path now splits missing desktops from a missing master: desktops join
  the appended grid block, master goes through AddTaskViewButton so
  before/after/above/below actually apply to a hand-written arrangement too.
  Added a fifth placement, "Last button in the grid" (inGrid), where master is
  sized like a desktop button and flows as one more cell - auto shapes count+1
  cells for it, and the manual path appends it into the desktop block.
- Layout v2.3 from the second live test. `MissingTokens` compared the expected
  name against the placed token as plain strings, so a mod with ALIASES broke:
  "desktop1 | desktop2 | desktop3 | master" appended "1 | 2 | 3" again and
  produced seven buttons. It now takes an optional `TokenMatcher`; any mod with
  a vocabulary must supply one, comparing item identity rather than the name
  the user typed. VD Switcher also accepts "taskview" as an alias for "master"
  (the natural thing to type given the setting is called Task View button), and
  the stoplight emoji are back inline in the indicator-symbol descriptions.
- Two stale-value bugs caught while renaming: `labelFormat == L"dot"` no longer
  matched the renamed `symbol` option (indicator symbols silently fell back to
  numbers), and Wh_ModInit still logged v1.8.
- `_templates/verify-settings-order.ps1` — new, machine-checks a mod's settings
  block against the contract. NOT in submission-preflight yet: only VD Switcher
  has migrated, and wiring it in would block review fixes on the other four.
- `compile-check.ps1` and `exit-time-destructor-audit.ps1` now pass Windhawk's
  own `-DWINVER/_WIN32_WINNT/NTDDI_VERSION` set. Without them `GetDpiForWindow`
  fails to compile in the lab but builds fine in Windhawk — the lab tooling was
  making a valid mod look broken.

Verified platform facts behind the design: nested settings groups are supported
and review-clean (`taskbar-elastic-pill` ships them; read as `Group.Key`), and
the settings API is READ-ONLY (`windhawk_api.h` has no setter), so a mod can
never fill a field in for the user — hence the `auto` sentinel plus a logged
expansion instead of a two-field Auto/Manual split.

No maintainer pushback on settings backward compatibility exists; all eight PR
threads were checked. The no-alias rule is self-imposed, from the reverted v1.6
`AliasedStr`/`AliasedInt` experiment. Clean break at a major version confirmed
by the user.

**VD Switcher v2.0 — AWAITING LIVE TEST.** Rebuilt on the contract: every
setting regrouped, `Layout.Arrangement` replaces layoutMode/layout/gridMode/
smartLayout/rows/columns/primaryAxis/crossAlign, `Adjust` replaces the four
padding sides + gridVerticalOffset, per-item offsets moved into the expression,
face nudging dropped, Task View sizing became literal width/height.
COMPILE_OK, EXIT_TIME_DESTRUCTOR_AUDIT_OK, README_MATCH, SETTINGS_ORDER_OK,
SUBMISSION_PREFLIGHT_OK. Users' 1.x settings do NOT migrate — the README says so
under "Upgrading from 1.x". Nothing pushed; PR #4844 still waits on the live
test.

Live test checklist:
- `auto` on single- and double-height taskbars; 2, 3, 4, 5 desktops
- the logged arrangement pasted back into Arrangement and edited
- an explicit arrangement (`1, 2 | 3, 4`) and a deliberately broken one
  (should log and fall back to automatic)
- per-item offset `1[+2,-1]`; group offset via Adjust
- Task View button in all four placements; confirm it is a full-height column
  and a full-width sliver by default, and that a fixed length centers
- write an arrangement, then CREATE a desktop: it should be appended and logged;
  switch Newly created desktops to Leave them out and confirm it stays out
- Indicator symbols label format (regression: the option value was renamed)
- all three Start positions; multi-monitor toggle
- **DPI**: repeat the auto layout at 125% and 150% scaling (this is the
  blocking bug class on #4855/#4843)
- clicking switches desktops; Explorer restart and disable/re-enable are clean

Next, in order: fold the contract into Privacy Anchor (#4843) and OmniButton
(#4855) alongside their required review fixes — both need the DPI fix that
`ngl::AvailableRows` now centralizes — then Clock Spacer (#4443), then flip
`verify-settings-order.ps1` into submission-preflight.

---INTERRUPTED---


What's rebuilt
Layout → Auto (smart-grid score → BuildGridExpression) or Manual (layout box), both feeding the one nested-group Compute. Auto logs the generated expression so you can copy it into Manual and tweak — your "see what it generates" idea, working within Windhawk's static-settings limit.
Render → owned buttons absolutely positioned in a content-sized container; the XAML Grid + Row/ColumnDefinitions + Grid.SetRow/Column/Span are gone.
Master (Task View) button → the master token; before/after/top/bottom just moves it in the expression. Its ~40-line special-case block is deleted.
Settings → layoutMode, layout, primaryAxis, crossAlign (retires shortGroupAlign), 4-side padding, and two-layer nudge (whole-button + content-inside-button, via a transform on an explicit TextBlock).
Bonus: converted the three bare no_destroy containers to optional<container> + reset() on controlled unload — m417z's #4844 request — so this all gets covered in one live test.
Sanity-checked the expressions by hand: 4 desktops single-height → 1 | 2 | 3 | 4; double-height smart → 1, 3 | 2, 4 (row-major 2×2); master "after" → (…) | master.

Your move — the live test
This is the guinea pig, so it needs your eyes on real hardware. Load taskbar-vd-switcher.wh.cpp into Windhawk and check:

Buttons appear, centered, correctly spaced; Auto on single- and double-height taskbars
Manual mode: type something like 1, 2 | 3, 4; then check the log shows the expression Auto generates
Master button in each position (before/after/top/bottom)
Nudge (1:+3,-1) moves a whole button; content nudge (master:0,1) shifts only its glyph
4-side padding; clicking switches desktops; Explorer restart and disable→re-enable are clean
What's intentionally not done yet
READMEs + version bump. I held these deliberately — they document the exact settings labels, and live testing usually shakes out label/UX tweaks. Writing them now then rewriting after your test is wasted work. Once you confirm it works (or tell me what to fix), I sync both README layers + the embedded block, bump the version, and it's PR-ready. That's why submission-preflight would flag README_MATCH right now — expected at this stage, not an oversight.

Tell me how it behaves and I'll fix whatever the test surfaces, then finish the docs. If instead you'd rather I write the READMEs now anyway, say so.

Lets put that on hold, a preliminary test is looking OK... check on all the PR responses we just got.
I'll pull the current state of all open PRs directly from GitHub.

PowerShell Fetch all PR comments and reviews chronologically
IN
foreach ($n in 4443,4485,4841,4843,4844,4855) {
  $j = gh pr view $n --repo ramensoftware/windhawk-mods --json number,title,state,updatedAt,headRefOid,comments,reviews,mergeable,mergeStateStatus | ConvertFrom-Json
  echo "===================================================================="
  echo "PR $($j.number) [$($j.state) $($j.mergeable)/$($j.mergeStateStatus)] head=$($j.headRefOid.Substring(0,8))  $($j.title)"
  $events = @()
  foreach ($c in $j.comments) { $events += [pscustomobject]@{ when=[datetime]$c.createdAt; who=$c.author.login; kind='comment'; body=$c.body } }
  foreach ($r in $j.reviews) { if ($r.body -or $r.state) { $events += [pscustomobject]@{ when=[datetime]$r.submittedAt; who=$r.author.login; kind="review:$($r.state)"; body=$r.body } } }
  $events = $events | Sort-Object when
  foreach ($e in $events) {
    $b = ($e.body -replace '\s+',' ').Trim()
    if ($b.Length -gt 280) { $b = $b.Substring(0,280) + '…' }
    echo ("  [{0:MM-dd HH:mm}] @{1} ({2}): {3}" -f $e.when,$e.who,$e.kind,$b)
  }
}

OUT
====================================================================
PR 4443 [OPEN UNKNOWN/UNKNOWN] head=85a971d5  Add Taskbar Clock Spacer v1.1
  [06-16 07:52] @m417z (comment): I think it makes more sense to have it as part of Taskbar Clock Customization. You can create a PR in the development repo here: https://github.com/m417z/my-windhawk-mods Let me know what you think.
  [06-16 16:13] @sb4ssman (comment): Sounds good, I'll work on integrating it into Taskbar Clock Customization in a fork of your repo.
  [07-23 18:48] @m417z (comment): Aside from the notes below, I don't understand why the mod hooks all these functions, when just hooking `DateTimeIconContent::OnApplyTemplate` seems to be enough. Also, remove `(pre-2604)` from the comment as it confuses the source code parser for the symbol cache. ### Submission…
====================================================================
PR 4485 [MERGED UNKNOWN/UNKNOWN] head=18fe50eb  Add Taskbar Folder Menus v0.7
  [06-22 18:35] @m417z (comment): * I think it makes more sense for the `folders` option to be an array. * If `HookTaskbarDllSymbols` fails, return FALSE at `Wh_ModInit`. * Instead of handling `Taskbar.View.dll` and hooking `LoadLibraryExW`, you can hook `TrayUI::StartTaskbar` and simplify the implementation. The…
  [07-18 00:44] @sb4ssman (comment): Updated to v0.6 with all review points addressed: - `folders` is now an array setting (per-record `label` + `target`) - `Wh_ModInit` returns FALSE when `HookTaskbarDllSymbols` fails - Replaced the Taskbar.View.dll handling and `LoadLibraryExW` hook with a `TrayUI::StartTaskbar` h…
  [07-18 14:10] @m417z (comment): ### Submission review _Note: This review was done by Claude, and then refined manually. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._ Please address the following issues. The items in the c…
  [07-20 00:04] @sb4ssman (comment): Updated the PR to v0.7 on a clean rebase of current `main`. The two required review items are addressed: - Right-click now uses the documented `WM_MENURBUTTONUP` path; the full classic Shell context-menu flow was live-tested, including modal Properties and click-away dismissal. -…
  [07-21 20:16] @m417z (comment): 1. Replace these two usages of `clang::no_destroy`: ```cpp [[clang::no_destroy]] static std::vector<ButtonEventState> g_buttonEventStates; [[clang::no_destroy]] static std::list<FrameworkElement::Loaded_revoker> g_loadedRevokers; ``` With: ```cpp [[clang::no_destroy]] static std:…
  [07-23 17:07] @sb4ssman (comment): All three review items are addressed, plus the data race. **1. `clang::no_destroy` containers.** `g_buttonEventStates` is now `[[clang::no_destroy]] static std::optional<std::vector<ButtonEventState>>`, constructed with `std::in_place` and released with `reset()` on the controlle…
====================================================================
PR 4841 [MERGED UNKNOWN/UNKNOWN] head=20aaf8e5  New mod: Tray Utility Customizer
  [07-20 22:44] @m417z (comment): Regarding item 1, you can look at how the Taskbar tray system icon tweaks mod handles this. If only English is supported for now, I think it's worth adding it to the readme. ### Submission review _Note: This review was done by Claude, and then refined manually. Due to the amount …
  [07-22 19:23] @sb4ssman (comment): Addressed the review in the v1.1 rewrite: documented the remaining English-only detection cases; removed the redundant enabled setting; replaced the IconView/multi-module hooks with TrayUI::StartTaskbar plus a bounded startup/rebuild retry; and added ARM64 GetTaskbarXamlRoot supp…
  [07-22 20:26] @m417z (comment): ### Submission review _Note: This review was done by Claude, and then refined manually. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._ Please address the following issues. The items in the c…
  [07-23 17:07] @sb4ssman (comment): Both required items are addressed, and I took all of the optional ones too. **1. `[[clang::no_destroy]]` audit.** Done exactly as described. `g_settings` and `g_columnLease` lost the attribute — they only own `std::wstring`s, so a heap-free-only destructor is already safe at shut…
====================================================================
PR 4843 [OPEN UNKNOWN/UNKNOWN] head=84404673  Add Tray Privacy Indicator Anchor v1.0
  [07-23 18:58] @m417z (comment): In addition to the notes below, improve the `[[clang::no_destroy]]` usage as discussed in the other PRs. Also `WindhawkUtils::SYMBOL_HOOK systemTrayDllHooks` applies for 3 modules, rename the variable and list the modules in a comment. ### Submission review _Note: This review was…
====================================================================
PR 4844 [OPEN MERGEABLE/CLEAN] head=c5995bd3  Update Taskbar Virtual Desktop Switcher to v1.8
  [07-20 21:05] @m417z (comment): A question about `[[clang::no_destroy]]`: did you add it, or did Claude add it? Using it is tricky, because using it too much may cause memory leaks, but not using it enough may cause hangs and crashes when the process terminates. For example, using it with `std::vector` will lea…
  [07-23 17:14] @sb4ssman (comment): Claude added it, not me — and it added it too broadly. These mods are written with Claude assistance, and after an earlier review raised exit-time destructor safety, it applied `[[clang::no_destroy]]` to every namespace-scope global with a non-trivial destructor rather than only …
  [07-23 17:42] @m417z (comment): > `WindhawkUtils::NoDestroy` would help I thought about it, but it wouldn't be different from `[[clang::no_destroy]]` or `[[clang::no_destroy]] std::optional<...>`. It's not difficult because of the syntax, it's difficult because it's truly tricky. Some destructors must be skippe…
  [07-23 18:35] @m417z (comment): > I'd rather not push > that change here on top of a green PR without live-testing it first, so I'll > fold it into the next tested update rather than sneak it in now. To clarify: are you going to update the PR, or are you saying that you prefer getting it merged as is?
====================================================================
PR 4855 [OPEN MERGEABLE/CLEAN] head=8ef2e4a4  Add OmniButton Customizer v1.0
  [07-23 19:02] @m417z (comment): In addition to the notes below, improve the `[[clang::no_destroy]]` usage as discussed in the other PRs. Regarding 1, it's generally preferable to test mods in different DPI settings. ### Submission review _Note: This review was done by Claude, and then refined manually. Due to t…
Big news in here. Let me pull the full text of the new reviews (they're truncated above) so I report them accurately.

PowerShell Full text of the three new submission reviews
IN
foreach ($n in 4443,4843,4855) {
  echo "############### PR $n — latest maintainer comment (full) ###############"
  gh pr view $n --repo ramensoftware/windhawk-mods --json comments -q '.comments[-1].body'
  echo ""
}

OUT
############### PR 4443 — latest maintainer comment (full) ###############
Aside from the notes below, I don't understand why the mod hooks all these functions, when just hooking `DateTimeIconContent::OnApplyTemplate` seems to be enough.

Also, remove `(pre-2604)` from the comment as it confuses the source code parser for the symbol cache.

### Submission review

_Note: This review was done by Claude, and then refined manually. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

---

**1. `HookSymbols` is called twice for `taskbar.dll` — collapse it into one call.** `HookTaskbarDllSymbols` resolves the four core symbols in one `HookSymbols` call and then `TrayUI::StartTaskbar` in a second call against the same module (lines 932 and 944). Each additional `HookSymbols` call for the same module re-triggers symbol resolution, which is the slow part; all of a module's hooks should be resolved in a single array/single call. Merge `startTaskbarHooks` into `taskbarDllHooks` (keep the `TrayUI::StartTaskbar` entry `optional = true`) and derive the flag afterward:

```cpp
// after the single HookSymbols(h, taskbarDllHooks, ...) call:
g_trayUiStartTaskbarHooked = (TrayUI_StartTaskbar_Original != nullptr);
```

**2. Add a screenshot or GIF to the README.** The effect is visual (items justified across a fixed clock width vs. bunched), and a before/after image makes the mod much easier to understand at a glance than the ASCII table alone. `i.imgur.com` and `raw.githubusercontent.com` are the allowed hosts.

**3. `[[clang::no_destroy]]` on `g_states` is unnecessary and leaks the vector's buffer on every unload (line 307).** The stated reason — avoiding an off-thread release of the XAML weak refs at shutdown — isn't actually a hazard: `winrt::weak_ref` release is just an in-process control-block decrement and is safe from any thread. Worse, with the attribute in place the automatic destructor never runs, so `g_states.clear()` in `ClearSpacerStates` (which zeroes the size but retains the heap buffer) leaves that buffer permanently leaked on each disable/reload. Since the element destructors are safe at process exit, just drop the attribute — the normal destructor is both correct and leak-free. (The elements are still explicitly released in `ClearSpacerStates`, so nothing else changes.)

<details><summary>Optional improvements</summary>
<p>

Minor polish — none of this affects users on a standard setup, so it's your call.

- **`Wh_ModUninit`'s fallback runs `ClearSpacerStates()` off the UI thread (line 1099).** When `FindCurrentProcessTaskbarWnd()` returns null, cleanup runs directly on the arbitrary uninit thread, contradicting the "must run on the UI thread" comment two lines above. It's mostly benign (if the taskbar window is gone the weak refs are likely already dead), but you could skip the direct call entirely in that branch since there's no UI thread to marshal to.

- **`StartInitialScan` reads the non-atomic `g_scanThread` without synchronization (line 966).** `Wh_ModAfterInit` (init thread) and `TrayUI_StartTaskbar_Hook` (taskbar thread) can both call it; in the rare overlap two scan threads could be created. In practice the two callers are mutually exclusive, so this is negligible — noting it only for completeness.

</p>
</details>

<details><summary>Functionality notes</summary>
<p>

Non-critical observations about the feature behavior itself.

- **Collapsing the source text block vs. TCC re-setting `Visibility`.** `CollapseSourceTextBlock` zeroes the original block's size and hides it, but the comment notes TCC re-sets `Visibility` on its own schedule. Zeroing the size guards against width creep, but there could be a brief frame where the original text flashes back before the next tick re-collapses it. Worth a look on slow text-change cadences; not a blocker.

</p>
</details>

############### PR 4843 — latest maintainer comment (full) ###############
In addition to the notes below, improve the `[[clang::no_destroy]]` usage as discussed in the other PRs.

Also `WindhawkUtils::SYMBOL_HOOK systemTrayDllHooks` applies for 3 modules, rename the variable and list the modules in a comment.

### Submission review

_Note: This review was done by Claude, and then refined manually. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

---

**1. Dangling `std::wstring_view` over an `hstring` temporary (undefined behavior).** In `ApplyPrivacyIndicatorBehavior` and its text-changed callback:

```cpp
std::wstring_view text = tb.Text();          // line 3830
...
std::wstring_view newText = tbRef.Text();    // line 3854
```

`TextBlock::Text()` returns a `winrt::hstring` *by value*. The temporary is destroyed at the end of the statement, freeing the `HSTRING` buffer, and `text`/`newText` are `wstring_view`s that keep pointing at that freed buffer. Every subsequent read (`text.empty()`, `text[0]`, `DetectPrivacyType(text)`, `IsPrivacyText(text)`) is a use-after-free. This runs on the hot native-indicator scan path (every icon load and every text change), so it's not a corner case — it only "works" today because the freed buffer usually still holds intact bytes. Every other taskbar mod keeps the `hstring` alive, e.g. [taskbar-tray-system-icon-tweaks.wh.cpp#L496](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-tray-system-icon-tweaks.wh.cpp#L496) uses `auto text = innerTextBlock.Text();`. Fix: take a `std::wstring` copy (you already do this for names via `std::wstring(fe.Name())`) or keep an `hstring`/`auto` and index it directly.

**2. `cameraHardwareDetection` defaults to `true`, silently initializing the camera on every Explorer start.** With the default on, the worker thread calls `MediaCapture::InitializeAsync()` on the system's default camera at every load/restart, for every user who installs the mod. Your own README warns this "activates its LED/indicator" on some devices — so by default a "reserve tray space" mod can make the camera light blink on Explorer restart, sign-in, etc., which users won't connect to this mod. This is exactly the kind of hardware side effect that should be opt-in. Recommend defaulting it to `false` (the feature is already fully documented as experimental, so opt-in fits). It's your call, but a surprising camera-touching default is a real user-facing concern.

<details><summary>Optional improvements</summary>
<p>

Minor polish — none of this affects users in normal operation, so it's your call.

- **`MicPrivacyMonitor::Cleanup()` ordering + unsynchronized endpoint pointers.** `IMMNotificationClient` callbacks are delivered on an audio-system thread, and they call `AttachDefaultEndpoint()`, which mutates `m_device`/`m_volume`. `Cleanup()` runs on the worker thread and calls `DetachEndpointVolume()` *before* `UnregisterEndpointNotificationCallback(this)` — so a notification arriving in that window re-attaches `m_device`/`m_volume` that then leak, and the two threads touch those raw pointers without a lock. It's a narrow unload/device-churn race (the notification callbacks are effectively serialized against each other), but unregister-then-detach, plus a small lock or atomic swap around the endpoint pointers, would close it.

- **`GetStringSetting` could be `WindhawkUtils::StringSetting`.** The raw `Wh_GetStringSetting` + `Wh_FreeStringSetting` helper reinvents the RAII wrapper; `WindhawkUtils::StringSetting` (from `windhawk_utils.h`) does the same with automatic cleanup.

- **Copilot activity polling enumerates every process once a minute.** `CheckCopilotActive()` takes a full `CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS)` every 60s just to look for four exe names. It's cheap enough, but it's the only real polling left in an otherwise event-driven design — worth a comment noting there's no event source, or a lower cadence.

</p>
</details>

<details><summary>Functionality notes</summary>
<p>

Non-critical observations about the feature behavior itself.

- **Smart-grid row capacity mixes physical pixels with DIPs.** In `InjectSyntheticIcons`, `taskbarH` comes from `GetWindowRect` (physical pixels), but `iconSize`/`buttonSpacing` (and therefore `pitch`) are XAML DIPs, and `availableRows = (taskbarH + spacing) / pitch` divides one by the other. On any non-100% display scaling — i.e. most laptops at 125–150% — the computed row count won't match the intent ("single row on standard taskbars"). Worth verifying the auto layout on a scaled display; the clean fix is to derive the height in DIPs (e.g. from the taskbar XAML element's `ActualHeight`, or divide the window rect by the monitor DPI scale) before dividing by the DIP pitch.

- **Only the primary taskbar gets the anchor bar.** `FindCurrentProcessTaskbarWnd` matches `Shell_TrayWnd` only, so secondary-monitor taskbars are untouched. That's consistent with where the system tray lives and the README says "primary taskbar," so just confirming it's intentional.

</p>
</details>

############### PR 4855 — latest maintainer comment (full) ###############
In addition to the notes below, improve the `[[clang::no_destroy]]` usage as discussed in the other PRs.

Regarding 1, it's generally preferable to test mods in different DPI settings.

### Submission review

_Note: This review was done by Claude, and then refined manually. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

---

1. **`ResolveGeometry` mixes physical pixels with XAML DIPs — the grid overflows the taskbar on any display scaling above 100%.** `GetWindowRect` returns physical pixels (Explorer is per-monitor-DPI aware), but the result is divided by the 24 DIP row pitch (or the `slotHeight` setting) to get `availableRows`, and `taskbarH / rows` is then assigned to XAML `Height`, which is in DIPs. At 100% scaling this cancels out; at 125%/150% (the default on most laptops) the taskbar is 60/72 physical px but only 48 DIPs, so `availableRows` is overestimated and `slotH` comes out as 30-32 DIPs — a 2-row grid becomes 60-64 DIPs tall on a 48-DIP taskbar and gets clipped. Mixed-DPI multi-monitor setups hit the same math. Convert to DIPs before comparing, as [taskbar-icon-size](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-icon-size.wh.cpp#L1811) does with `MulDiv` + `GetDpiForWindow`:

   ```cpp
   int taskbarH = 48;
   if (hTaskbarWnd) {
       RECT r{};
       if (GetWindowRect(hTaskbarWnd, &r)) {
           UINT dpi = GetDpiForWindow(hTaskbarWnd);
           taskbarH = MulDiv(r.bottom - r.top, 96, dpi ? dpi : 96);
       }
   }
   ```

2. **The `systemTrayDllHooks` array name only declares `SystemTray.dll`, but the hooks are resolved against three possible modules.** `GetSystemTrayModuleHandle()` can return `SystemTray.dll`, `Taskbar.View.dll`, or `ExplorerExtensions.dll`, and `HookSystemTraySymbols` applies the array to whichever one is found. By convention the array's name or a comment above it must declare exactly the module(s) it's resolved against. Use a neutral name with a comment listing all of them, as [taskbar-tray-system-icon-tweaks](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-tray-system-icon-tweaks.wh.cpp#L1586) does:

   ```cpp
   // SystemTray.dll, Taskbar.View.dll, ExplorerExtensions.dll
   WindhawkUtils::SYMBOL_HOOK symbolHooks[] = {{
   ```

<details><summary>Optional improvements</summary>
<p>

Minor polish — none of this affects users, so it's your call.

- `[[clang::no_destroy]]` on `g_settings` is unneeded: `ModSettings` holds only arrays/ints/enums, so it's trivially destructible and safe at process shutdown as-is. Unneeded suppression is noise — remove the attribute there.
- `g_autoRevokerList` and `g_propertySnapshots` carry a bare `[[clang::no_destroy]]` on container types. The preferred form for non-nullable containers is `[[clang::no_destroy]] std::optional<...>` with `reset()` on the unload path, which makes the destruction explicit. In this specific code it's only conformance polish: `clear()` on a `list`/`vector` does destroy the elements (releasing the XAML refs on the taskbar thread), and the residual buffer is plain heap.
- `StartRetryThread`/`StopRetryThread` manipulate `g_retryThread`/`g_retryStopEvent` without synchronization, and can run concurrently from `Wh_ModSettingsChanged` (arbitrary thread) and `TrayUI_StartTaskbar_Hook` (taskbar thread) — two simultaneous `StopRetryThread` calls could double-close the handles. Only reachable when a settings change coincides with a taskbar restart, hence optional; a small mutex around start/stop closes it.
- `ApplyOnTaskbarWindowThread` and `Wh_ModUninit` prefer the cached `g_taskbarWnd` without validating it. If `Shell_TrayWnd` is recreated in-process, the retry thread keeps dispatching to a dead handle (the `IconView` Loaded path eventually recovers, since `ApplyAllSettings` re-finds the window). An `IsWindow()` check — or always calling `FindCurrentProcessTaskbarWnd()` — makes the retry path self-healing.
- `LoadColorSetting` is an exact duplicate of `LoadStringSetting<32>` — the template alone suffices.
- The `slotW` parameter of `WalkSetupBatteryInnerPanel` is unused, and `<cmath>` appears to be an unused include.

</p>
</details>

<details><summary>Functionality notes</summary>
<p>

Non-critical observations and ideas about the feature behavior itself.

- `slotWidth` values 1-15 silently jump to the default 32 (`sw < 16 ? 32 : sw`) instead of clamping to the 16 minimum — a user lowering the value below 16 gets a *wider* slot. Clamping to 16 would be less surprising.
- Wifi and volume are identified purely positionally (items-host children 0 and 1), while battery is detected structurally. If a future Windows build reorders or inserts slots, the first two would be mislabeled; the "Unknown slot" logging is a good safety net, but structural detection for all items would be more robust.
- The layout isn't recomputed when the taskbar height or monitor DPI changes at runtime (e.g. `WM_DPICHANGED`, taskbar size mods toggling) — only on settings changes, `TrayUI::StartTaskbar`, or new `IconView` creation. Probably fine in practice; noting it as a known limitation.

</p>
</details>
Here's what came back — and it's a big shift.

🎉 Two merged
PR #4485 (Folder Menus v0.7) and PR #4841 (Tray Utility Customizer v1.1) are MERGED. The review-fix pushes from earlier today did it — both are now in the official catalog.

Four open PRs — all now have fresh maintainer reviews (today)
#4844 (VD Switcher) — has a direct question waiting on you. After our exchange about deferring the no_destroy fix, m417z asked:

"To clarify: are you going to update the PR, or are you saying that you prefer getting it merged as is?"

This is a decision for you. Given we've now rebuilt VD Switcher (unified placement + the no_destroy fix already done, preliminary test looking OK), the honest answer is "updating it, after the live test" — but I won't reply until you say so.

#4443 (Clock Spacer) — new review:

Architectural Q: why hook all those functions when DateTimeIconContent::OnApplyTemplate alone seems enough?
Remove (pre-2604) from a comment — it confuses the symbol-cache parser
no_destroy on g_states leaks the buffer — drop it (weak_ref release is thread-safe)
Add a before/after screenshot/GIF; plus optional off-thread-cleanup and scan-thread-race notes
#4843 (Privacy Anchor) — new review, two real bugs:

Use-after-free: std::wstring_view text = tb.Text() dangles over a destroyed hstring temporary, on the hot scan path. Take a std::wstring copy.
Camera default-on: cameraHardwareDetection defaults true, so it initializes the camera (LED can blink) on every Explorer start — should be opt-in false
Plus: fix no_destroy; rename systemTrayDllHooks (3 modules) + comment; and a DPI bug (physical px ÷ DIP pitch) in the smart-grid row math
#4855 (OmniButton) — new review:

DPI bug (blocking): ResolveGeometry mixes physical px and DIPs → grid overflows/clips above 100% scaling. Fix with MulDiv + GetDpiForWindow
Rename systemTrayDllHooks (resolved against 3 modules) + comment
Plus: no_destroy cleanup; test at different DPI; several optional polish items
The pattern worth noting
Three themes are now cross-cutting across every remaining mod, which strongly validates your templatize-everything instinct:

[[clang::no_destroy]] — requested on all four; it was also the merge-gate on the two that just merged. This is exactly your Step 3 (templated no_destroy solution → apply everywhere). VD Switcher already has the fix from today's rebuild.
DPI mixing (physical px vs DIPs) — a real correctness bug flagged on #4855 (blocking) and #4843. It lives in the smart-grid row heuristic — the same GetAvailableRows code path VD Switcher still uses. The unified-placement rollout should fix DPI handling once, centrally.
SYMBOL_HOOK array naming — must name all target modules; flagged on #4843 and #4855.
Nothing here is on fire, and I haven't touched a single PR. Two questions for you:

#4844 — how do you want to answer the "update or merge as-is?" question?
Want me to reconcile the working notes with the two merges + four reviews now (the notes still say those were "passive wait")?
ok i replied to the maintainer:  "I am updating it, we can hold off on the merge.

And I see the other comments. I am working them too."

ok reconcile the notes and prepare to update the template with the clang no destroy resolutions... and then then distribute the resolution to the affected mods.
Good call on the #4844 reply. Let me reconcile the notes first (state materially changed), then curate the no_destroy resolution into the template, then apply it to the affected mods. Reading current notes + the template's lifecycle guidance in parallel.

Read t:\Github\sb4ssman\Windhawk-Mod-Lab\.agents\working_notes.md (lines 2-56)
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\.agents\working_notes.md
Added 7 lines
Now a work_log entry for the merges + review wave, then I'll dig into the template.

Read t:\Github\sb4ssman\Windhawk-Mod-Lab\.agents\work_log.md (lines 2-9)
Bash Show end of work_log
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab && tail -3 .agents/work_log.md

OUT
  unchanged until they re-adopt; tray-utility still links clean. Next: adopt the
  unified system in VD Switcher (the user-chosen guinea pig) and hand off a
  build for a live test before anything is pushed.
Bash Append work_log entry
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab && cat >> .agents/work_log.md <<'EOF'

## 2026-07-24 — Two merges, review wave, and the no_destroy resolution

- PR #4485 (Folder Menus v0.7) and PR #4841 (Tray Utility Customizer v1.1) were
  MERGED upstream on 2026-07-23 after the review-fix pushes. Both are now in the
  official catalog.
- The four remaining open PRs each got a fresh 2026-07-23 maintainer review.
  Cross-cutting requests: fix `[[clang::no_destroy]]` usage (all four), a
  physical-px-vs-DIP mixing bug in the smart-grid row heuristic (#4855 blocking,
  #4843), and `SYMBOL_HOOK` array naming that must list every target module
  (#4843, #4855). Per-mod bugs: #4843 has a use-after-free (`wstring_view` over a
  destroyed `hstring`) and a camera-touching default; #4855 has the blocking DPI
  clip; #4443 wants a hooking-surface reduction, a symbol-cache comment fix, and
  a screenshot.
- User replied on #4844 (2026-07-24): "I am updating it, we can hold off on the
  merge... working [the other comments] too." VD Switcher's no_destroy fix is
  already in the unified-placement rebuild (`7c1e4b8`), pending its live test.
- Reconciled working_notes to the merged/open split and the review action items.
- Started Step 3: curate the definitive `[[clang::no_destroy]]` resolution into
  the lifecycle template, then distribute it to #4443/#4843/#4855 one
  live-tested build at a time (VD Switcher already done).
EOF
echo committed-to-file

OUT
committed-to-file
Now the template. Let me read the lifecycle template's no_destroy section and the audit to see what's already encoded vs. what the maintainer's clarifications add.

Grep "no_destroy|no-destroy|exit-time|weak_ref|reset\(\)|optional|heap-only" (in t:\Github\sb4ssman\Windhawk-Mod-Lab\_templates\taskbar-xaml-lifecycle.template.cpp)
19 lines of output
Read t:\Github\sb4ssman\Windhawk-Mod-Lab\_templates\taskbar-xaml-lifecycle.template.cpp (lines 56-95)
The v1.3 contract is already strong; the maintainer's wave adds one real correction — weak_ref/heap-only containers don't need no_destroy at all (the "avoid off-thread release" justification is false). Let me fold that in.

Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\_templates\taskbar-xaml-lifecycle.template.cpp
Added 13 lines
Read t:\Github\sb4ssman\Windhawk-Mod-Lab\_templates\taskbar-xaml-lifecycle.template.cpp (lines 2-11)
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\_templates\taskbar-xaml-lifecycle.template.cpp
Added 3 lines
Now align the README adoption rule with the clarification:

Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\_templates\README.md
Added 3 lines
Template + docs updated. Now let me survey the exact no_destroy usage in each affected mod so distribution is precise.

Bash Survey no_destroy declarations in the three affected mods
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
for f in taskbar-clock-spacer/taskbar-clock-spacer.wh.cpp privacy-indicator-anchor/privacy-indicator-anchor.wh.cpp omnibutton-customizer/omnibutton-customizer.wh.cpp; do
  echo "================= $f ================="
  grep -nE 'clang::no_destroy' "$f" 2>/dev/null || echo "  (file not found or no matches)"
done

OUT
================= taskbar-clock-spacer/taskbar-clock-spacer.wh.cpp =================
307:[[clang::no_destroy]] static std::vector<SpacerState> g_states;
================= privacy-indicator-anchor/privacy-indicator-anchor.wh.cpp =================
1161:[[clang::no_destroy]] static ModSettings g_settings;
1384:[[clang::no_destroy]] static Grid g_syntheticGrid = nullptr;
1385:[[clang::no_destroy]] static FrameworkElement g_locIcon = nullptr;
1386:[[clang::no_destroy]] static FrameworkElement g_micIcon = nullptr;
1387:[[clang::no_destroy]] static FrameworkElement g_camIcon = nullptr;
1388:[[clang::no_destroy]] static FrameworkElement g_copilotIcon = nullptr;
1389:[[clang::no_destroy]] static FrameworkElement g_locSlot = nullptr;
1390:[[clang::no_destroy]] static FrameworkElement g_micSlot = nullptr;
1391:[[clang::no_destroy]] static FrameworkElement g_camSlot = nullptr;
1392:[[clang::no_destroy]] static FrameworkElement g_copilotSlot = nullptr;
1393:[[clang::no_destroy]] static FrameworkElement g_locGlowIcon = nullptr;
1394:[[clang::no_destroy]] static FrameworkElement g_micGlowIcon = nullptr;
1395:[[clang::no_destroy]] static FrameworkElement g_camGlowIcon = nullptr;
1396:[[clang::no_destroy]] static FrameworkElement g_copilotGlowIcon = nullptr;
1397:[[clang::no_destroy]] static FrameworkElement g_locSlashIcon = nullptr;
1398:[[clang::no_destroy]] static FrameworkElement g_micSlashIcon = nullptr;
1399:[[clang::no_destroy]] static FrameworkElement g_camSlashIcon = nullptr;
1400:[[clang::no_destroy]] static FrameworkElement g_copilotSlashIcon = nullptr;
1401:[[clang::no_destroy]] static FrameworkElement g_syntheticParent = nullptr;
1402:[[clang::no_destroy]] static lease_column::Lease g_columnLease;
1403:[[clang::no_destroy]] static start_placement::Lease g_startLease;
1409:[[clang::no_destroy]] static std::vector<SlotEventState> g_slotEventStates;
1417:[[clang::no_destroy]] static std::vector<GlowAnimationState>
1428:[[clang::no_destroy]] static std::vector<PrivacyState> g_privacyStates;
1433:[[clang::no_destroy]] static std::list<FrameworkElementLoadedRevoker>
================= omnibutton-customizer/omnibutton-customizer.wh.cpp =================
580:[[clang::no_destroy]] static ModSettings g_settings{};
701:[[clang::no_destroy]] static StackPanel       g_omniStackPanel{ nullptr };
702:[[clang::no_destroy]] static FrameworkElement g_omniButton{ nullptr };
703:[[clang::no_destroy]] static FrameworkElement g_wifiPresenter{ nullptr };
704:[[clang::no_destroy]] static FrameworkElement g_volumePresenter{ nullptr };
705:[[clang::no_destroy]] static FrameworkElement g_batteryPresenter{ nullptr };
706:[[clang::no_destroy]] static StackPanel       g_batteryInnerPanel{ nullptr };
707:[[clang::no_destroy]] static FrameworkElement g_batteryGlyphFE{ nullptr };
708:[[clang::no_destroy]] static FrameworkElement g_batteryPercentFE{ nullptr };
710:[[clang::no_destroy]] static TextBlock g_wifiGlyphTB{ nullptr };
711:[[clang::no_destroy]] static TextBlock g_volumeGlyphTB{ nullptr };
712:[[clang::no_destroy]] static TextBlock g_batteryGlyphTB{ nullptr };
713:[[clang::no_destroy]] static TextBlock g_percentTB{ nullptr };
715:[[clang::no_destroy]] static StackPanel g_layoutUpdatedSP{ nullptr };
724:[[clang::no_destroy]] static std::list<FrameworkElement::Loaded_revoker>
733:[[clang::no_destroy]] static std::vector<PropertySnapshot> g_propertySnapshots;
Template and docs updated. Committing this checkpoint, then I'll classify each mod's containers (strong-ref → optional+reset; weak_ref/heap → drop the attribute — which requires reading each struct, exactly the "semantic not syntactic" judgment m417z described).

Bash Commit template + notes checkpoint
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
git add _templates/taskbar-xaml-lifecycle.template.cpp _templates/README.md .agents/working_notes.md .agents/work_log.md
git commit -q -m "$(cat <<'EOF'
Lifecycle v1.3.1: no_destroy is only for strong XAML/WinRT containers

Fold in m417z's clarification from the 2026-07-23 review wave: no_destroy is not
a tool to avoid off-thread release. winrt::weak_ref (and plain heap) release
safely from any thread, so weak_ref/heap containers stay unannotated (rule 1) —
a bare no_destroy there only leaks the buffer on unload. Strong XAML/WinRT-ref
containers still use no_destroy optional<container> + reset() on controlled
unload. Also reconciles notes: #4485/#4841 merged; #4443/#4843/#4844/#4855 each
carry a new review.

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>
EOF
)"
git log --oneline -1

OUT
warning: in the working copy of '.agents/work_log.md', LF will be replaced by CRLF the next time Git touches it
warning: in the working copy of '.agents/working_notes.md', LF will be replaced by CRLF the next time Git touches it
warning: in the working copy of '_templates/README.md', LF will be replaced by CRLF the next time Git touches it
warning: in the working copy of '_templates/taskbar-xaml-lifecycle.template.cpp', LF will be replaced by CRLF the next time Git touches it
731e80a Lifecycle v1.3.1: no_destroy is only for strong XAML/WinRT containers
Update Todos

Curate unified element-placement template: nested-group arranger + padding + nudge + generator; tests; docs

VD Switcher unified-placement rebuild (committed 7c1e4b8) — awaiting full user live test, then READMEs + version bump

Curate no_destroy resolution into lifecycle template (v1.3.1: weak_ref/heap unannotated)

Distribute no_destroy resolution to Clock Spacer, Privacy Anchor, OmniButton (local, audit-green, pending each live test)

Enforce settings uniformity across mods (canonical order + identical wording)

Streamline the PR submission ritual into a goof-proof script
Now classify each container by reading its struct. Clock Spacer's SpacerState first (m417z says it's weak_refs → drop the attribute):

Bash Inspect SpacerState struct and cleanup
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
echo "===== Clock Spacer: SpacerState ====="
grep -nE 'struct SpacerState' taskbar-clock-spacer/taskbar-clock-spacer.wh.cpp
awk '/struct SpacerState/{f=1} f{print NR": "$0} f&&/};/{exit}' taskbar-clock-spacer/taskbar-clock-spacer.wh.cpp
echo ""
echo "===== Clock Spacer: ClearSpacerStates (how elements are released) ====="
awk '/ClearSpacerStates/{f=1} f{print NR": "$0} f&&/^}/{c++} c>=1&&/^}/{exit}' taskbar-clock-spacer/taskbar-clock-spacer.wh.cpp | head -30

OUT
===== Clock Spacer: SpacerState =====
296:struct SpacerState {
296: struct SpacerState {
297:     winrt::weak_ref<TextBlock>  originalRef;
298:     winrt::weak_ref<StackPanel> parentRef;
299:     winrt::weak_ref<StackPanel> generatedRef;
300:     uint64_t                    generatedLayoutKey = 0;
301:     int64_t                     textToken = 0;
302: };

===== Clock Spacer: ClearSpacerStates (how elements are released) =====
1007: static void ClearSpacerStates() {
1008:     for (auto& state : g_states) {
1009:         if (auto textBlock = state.originalRef.get()) {
1010:             if (state.textToken)
1011:                 textBlock.UnregisterPropertyChangedCallback(
1012:                     TextBlock::TextProperty(), state.textToken);
1013:             textBlock.ClearValue(FrameworkElement::MaxWidthProperty());
1014:             RestoreSourceTextBlock(textBlock);
1015:         }
1016:         // The parent StackPanel is intentionally untouched: this mod no longer
1017:         // sets anything on it, and clearing MaxWidth here would erase Taskbar
1018:         // Clock Customization's fixed clock width.
1019:         RemoveGeneratedPanel(state);
1020:     }
1021:     g_states.clear();
1022: }
SpacerState is pure weak_ref + integers — thread-safe, so per the resolution the attribute just leaks the buffer. Drop it (rule 1). Now the other two mods' structs, to classify strong-ref vs weak/heap:

Bash Inspect Privacy Anchor and OmniButton container structs
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
pa=privacy-indicator-anchor/privacy-indicator-anchor.wh.cpp
echo "########## PRIVACY ANCHOR ##########"
for s in SlotEventState GlowAnimationState PrivacyState FrameworkElementLoadedRevoker; do
  echo "----- struct $s -----"
  awk "/struct $s/{f=1} f{print NR\": \"\$0} f&&/};/{exit}" "$pa"
done
echo "----- start_placement::Lease (from template header embed) -----"
awk '/struct Lease/{f=1} f{print NR": "$0} f&&/};/{exit}' "$pa" | head -20
echo ""
ob=omnibutton-customizer/omnibutton-customizer.wh.cpp
echo "########## OMNIBUTTON ##########"
echo "----- line 724 context (Loaded_revoker list name) -----"
sed -n '723,725p' "$ob" | nl -ba
echo "----- struct PropertySnapshot -----"
awk '/struct PropertySnapshot/{f=1} f{print NR": "$0} f&&/};/{exit}' "$ob"

OUT
########## PRIVACY ANCHOR ##########
----- struct SlotEventState -----
1405: struct SlotEventState {
1406:     FrameworkElement element{nullptr};
----- struct GlowAnimationState -----
1411: struct GlowAnimationState {
1412:     FrameworkElement element{nullptr};
----- struct PrivacyState -----
1420: struct PrivacyState {
1421:     enum class Type { Location, Mic, Camera, Both };
----- struct FrameworkElementLoadedRevoker -----
----- start_placement::Lease (from template header embed) -----
698: struct Lease {
699:     std::wstring markerName;
700:     int column = -1;
701: };

########## OMNIBUTTON ##########
----- line 724 context (Loaded_revoker list name) -----
     1	
     2	[[clang::no_destroy]] static std::list<FrameworkElement::Loaded_revoker>
     3	    g_autoRevokerList;
----- struct PropertySnapshot -----
727: struct PropertySnapshot {
728:     DependencyObject object{nullptr};
Let me read the full struct bodies (awk truncated at nested };):

Read t:\Github\sb4ssman\Windhawk-Mod-Lab\privacy-indicator-anchor\privacy-indicator-anchor.wh.cpp (lines 1406-1435)
Read t:\Github\sb4ssman\Windhawk-Mod-Lab\omnibutton-customizer\omnibutton-customizer.wh.cpp (lines 728-737)
Classification complete. Before I edit mod files — you said you're working the other review items on these same mods, so let me check for uncommitted work to avoid clobbering you.

Bash Check for uncommitted mod-file changes
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
echo "=== working tree status ==="
git status --short
echo "=== is local main ahead of origin? ==="
git status -sb | head -1

OUT
=== working tree status ===
?? taskbar-vd-switcher/Archive/taskbar-vd-switcher-v1.8-pre-unified.wh.cpp
=== is local main ahead of origin? ===
## main...origin/main [ahead 4]
Working tree is clean — you haven't started editing these in this tree, so no collision. I'll do all three now, surgically (no_destroy only), compiling + auditing + committing each separately so they're easy to build on.

Clock Spacer — g_states is weak_ref + ints, so drop the attribute (rule 1):

Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\taskbar-clock-spacer\taskbar-clock-spacer.wh.cpp
Added 4 lines
The audit checks for the marker on the declaration line itself, not a comment above. Fixing:

Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\taskbar-clock-spacer\taskbar-clock-spacer.wh.cpp
Modified
PowerShell Compile + audit Clock Spacer
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
$c = ./_templates/compile-check.ps1 taskbar-clock-spacer 2>&1; $c | Select-String 'COMPILE_'
$a = ./_templates/exit-time-destructor-audit.ps1 taskbar-clock-spacer 2>&1; $a | Select-String 'AUDIT_OK|ERROR|REVIEW'

OUT
taskbar-clock-spacer COMPILE_OK
taskbar-clock-spacer EXIT_TIME_DESTRUCTOR_AUDIT_OK
Clock Spacer green. Committing it, then OmniButton.

Bash Commit Clock Spacer no_destroy fix
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
git add taskbar-clock-spacer/taskbar-clock-spacer.wh.cpp
git commit -q -m "$(cat <<'EOF'
Clock Spacer: drop no_destroy on g_states (weak_ref container, #4443)

SpacerState holds only winrt::weak_ref and integers. weak_ref release is a
thread-safe in-process refcount decrement, so the normal destructor is correct
and leak-free; the bare no_destroy only leaked the vector buffer on every
unload. Addresses the no_destroy item in the #4443 review. Not yet live-tested.

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>
EOF
)"
git log --oneline -1

OUT
warning: in the working copy of 'taskbar-clock-spacer/taskbar-clock-spacer.wh.cpp', LF will be replaced by CRLF the next time Git touches it
faa31f0 Clock Spacer: drop no_destroy on g_states (weak_ref container, #4443)
OmniButton — remove no_destroy from g_settings (heap-only); convert g_autoRevokerList (revokers) and g_propertySnapshots (strong WinRT refs) to optional+reset(). Let me map their access sites and the unload path.

Bash Map OmniButton container accesses and unload path
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
ob=omnibutton-customizer/omnibutton-customizer.wh.cpp
echo "=== g_autoRevokerList accesses ==="
grep -nE 'g_autoRevokerList' "$ob"
echo "=== g_propertySnapshots accesses ==="
grep -nE 'g_propertySnapshots' "$ob"
echo "=== g_settings declaration + any reference in ClearValue/reset context ==="
grep -nE 'g_settings\b' "$ob" | head -3
echo "=== Wh_ModUninit + controlled UI-thread cleanup (RunFromWindowThread / RemoveModUi) ==="
grep -nE 'void Wh_ModUninit|RunFromWindowThread|RemoveOmni|No live XAML root|retain' "$ob"

OUT
=== g_autoRevokerList accesses ===
725:    g_autoRevokerList;
1758:        g_autoRevokerList.emplace_back();
1759:        auto it = std::prev(g_autoRevokerList.end());
1763:                    g_autoRevokerList.erase(it);
1982:            g_autoRevokerList.clear();
=== g_propertySnapshots accesses ===
733:[[clang::no_destroy]] static std::vector<PropertySnapshot> g_propertySnapshots;
740:    for (auto const& snapshot : g_propertySnapshots) {
743:    g_propertySnapshots.push_back(
748:    for (auto it = g_propertySnapshots.rbegin();
749:         it != g_propertySnapshots.rend(); ++it) {
759:    g_propertySnapshots.clear();
=== g_settings declaration + any reference in ClearValue/reset context ===
580:[[clang::no_destroy]] static ModSettings g_settings{};
613:          wcsncpy(g_settings.itemOrderStr, s, 127); g_settings.itemOrderStr[127] = L'\0';
617:          wcscpy(g_settings.itemOrderStr, L"wifi, volume, battery, percent");
=== Wh_ModUninit + controlled UI-thread cleanup (RunFromWindowThread / RemoveModUi) ===
1538:using RunFromWindowThreadProc_t = void (*)(void*);
1553:static bool InvokeWindowThreadProc(RunFromWindowThreadProc_t proc,
1564:static bool RunFromWindowThread(HWND hWnd, RunFromWindowThreadProc_t proc,
1567:        L"Windhawk_RunFromWindowThread_" WH_MOD_ID);
1569:        RunFromWindowThreadProc_t proc;
1583:                    L"Windhawk_RunFromWindowThread_" WH_MOD_ID);
1851:    RunFromWindowThread(window, [](void*) {
1975:void Wh_ModUninit() {
1981:        if (!RunFromWindowThread(hWnd, [](void*) {
1986:            Wh_Log(L"[Uninit] Taskbar dispatch failed; retaining XAML state");
1989:        Wh_Log(L"[Uninit] No taskbar UI thread; retaining XAML state");
2005:    if (!RunFromWindowThread(hWnd, [](void* parameter) {
2010:        Wh_Log(L"[Settings] Taskbar dispatch failed; retaining XAML state");
Read t:\Github\sb4ssman\Windhawk-Mod-Lab\omnibutton-customizer\omnibutton-customizer.wh.cpp (lines 1976-1995)
Bash Check OmniButton optional include and snapshot access context
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
ob=omnibutton-customizer/omnibutton-customizer.wh.cpp
echo "=== optional included? ==="
grep -nE '#include <optional>' "$ob" || echo "  NOT included"
echo "=== range-for / iterator sites ==="
sed -n '740p;748,749p' "$ob"
echo "=== what calls g_propertySnapshots.clear() (line ~759) — is it rebuild-safe? ==="
awk 'NR>=735 && NR<=762 {print NR": "$0}' "$ob"

OUT
=== optional included? ===
  NOT included
=== range-for / iterator sites ===
    for (auto const& snapshot : g_propertySnapshots) {
    for (auto it = g_propertySnapshots.rbegin();
         it != g_propertySnapshots.rend(); ++it) {
=== what calls g_propertySnapshots.clear() (line ~759) — is it rebuild-safe? ===
735: static void LogCurrentUiException(PCWSTR context) noexcept;
736: 
737: static void TrackProperty(DependencyObject const& object,
738:                           DependencyProperty const& property) {
739:     if (!object || !property) return;
740:     for (auto const& snapshot : g_propertySnapshots) {
741:         if (snapshot.object == object && snapshot.property == property) return;
742:     }
743:     g_propertySnapshots.push_back(
744:         {object, property, object.ReadLocalValue(property)});
745: }
746: 
747: static void RestorePropertySnapshots() {
748:     for (auto it = g_propertySnapshots.rbegin();
749:          it != g_propertySnapshots.rend(); ++it) {
750:         try {
751:             if (it->localValue == DependencyProperty::UnsetValue())
752:                 it->object.ClearValue(it->property);
753:             else
754:                 it->object.SetValue(it->property, it->localValue);
755:         } catch (...) {
756:             Wh_Log(L"[Cleanup] Failed to restore a XAML property");
757:         }
758:     }
759:     g_propertySnapshots.clear();
760: }
761: 
762: // ── Grid geometry ─────────────────────────────────────────────────────────
RestorePropertySnapshots clears at 759 (rebuild-safe) so reset() goes only in the uninit lambda. Applying OmniButton edits. First the include + three declarations:

Bash Find include anchor in OmniButton
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
grep -nE '^#include <(vector|list|string|algorithm)>' omnibutton-customizer/omnibutton-customizer.wh.cpp | head

OUT
296:#include <algorithm>
302:#include <list>
303:#include <vector>
Read t:\Github\sb4ssman\Windhawk-Mod-Lab\omnibutton-customizer\omnibutton-customizer.wh.cpp (lines 303-304)
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\omnibutton-customizer\omnibutton-customizer.wh.cpp
Added 1 line
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\omnibutton-customizer\omnibutton-customizer.wh.cpp
Modified
Read t:\Github\sb4ssman\Windhawk-Mod-Lab\omnibutton-customizer\omnibutton-customizer.wh.cpp (lines 725-726)
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\omnibutton-customizer\omnibutton-customizer.wh.cpp
Modified
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\omnibutton-customizer\omnibutton-customizer.wh.cpp
Added 1 line
Now the access conversions (->), the range-for deref, and reset() in the terminal unload:

Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\omnibutton-customizer\omnibutton-customizer.wh.cpp
Modified
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\omnibutton-customizer\omnibutton-customizer.wh.cpp
Modified
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\omnibutton-customizer\omnibutton-customizer.wh.cpp
Modified
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\omnibutton-customizer\omnibutton-customizer.wh.cpp
Added 4 lines
PowerShell Compile + audit OmniButton
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
$c = ./_templates/compile-check.ps1 omnibutton-customizer 2>&1
$c | Select-String 'COMPILE_'
$c | Select-String 'error:' | Select-Object -First 8
$a = ./_templates/exit-time-destructor-audit.ps1 omnibutton-customizer 2>&1; $a | Select-String 'AUDIT_OK|ERROR|REVIEW|warning'

OUT
omnibutton-customizer COMPILE_OK
omnibutton-customizer EXIT_TIME_DESTRUCTOR_AUDIT_OK
OmniButton green. Committing, then Privacy Anchor (the biggest — mixed classification).

Bash Commit OmniButton no_destroy fix
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
git add omnibutton-customizer/omnibutton-customizer.wh.cpp
git commit -q -m "$(cat <<'EOF'
OmniButton: fix no_destroy ownership (#4855)

- Drop no_destroy from g_settings (ModSettings is heap-only: arrays/ints/enums).
- Convert g_autoRevokerList (Loaded_revokers) and g_propertySnapshots (strong
  DependencyObject/Property/IInspectable refs) to no_destroy optional<container>
  with reset() on the controlled UI-thread unload, so their heap buffers are
  freed instead of leaked on every unload. Rebuild-path clears are unchanged.

Addresses the no_destroy item in the #4855 review. Not yet live-tested.

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>
EOF
)"
git log --oneline -1

OUT
warning: in the working copy of 'omnibutton-customizer/omnibutton-customizer.wh.cpp', LF will be replaced by CRLF the next time Git touches it
c57c9e7 OmniButton: fix no_destroy ownership (#4855)
Privacy Anchor — mixed: remove attribute from g_settings, g_columnLease, g_startLease, g_privacyStates (heap/weak_ref); convert g_slotEventStates, g_glowAnimationStates, g_loadedRevokers (strong refs/revokers) to optional+reset(). Mapping accesses + unload path:

Bash Map Privacy Anchor container accesses and unload path
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
pa=privacy-indicator-anchor/privacy-indicator-anchor.wh.cpp
for v in g_slotEventStates g_glowAnimationStates g_loadedRevokers g_privacyStates g_columnLease g_startLease; do
  echo "=== $v ==="; grep -nE "\b$v\b" "$pa"
done
echo "=== optional included? ==="; grep -nE '#include <optional>' "$pa" || echo "  NOT included"
echo "=== uninit UI-thread cleanup ==="; grep -nE 'void Wh_ModUninit|RunFromWindowThread\(|No live XAML|retain|reset\(\)|\.clear\(\)' "$pa" | head -40

OUT
=== g_slotEventStates ===
1409:[[clang::no_destroy]] static std::vector<SlotEventState> g_slotEventStates;
3672:        g_slotEventStates.push_back({slot, tappedToken});
3742:    for (auto& state : g_slotEventStates) {
3746:    g_slotEventStates.clear();
=== g_glowAnimationStates ===
1418:    g_glowAnimationStates;
1751:    for (auto& state : g_glowAnimationStates) {
3328:    g_glowAnimationStates.push_back(std::move(animationState));
3750:    for (auto& state : g_glowAnimationStates) {
3758:    g_glowAnimationStates.clear();
=== g_loadedRevokers ===
1434:    g_loadedRevokers;
3805:    g_loadedRevokers.clear();
3999:    g_loadedRevokers.clear();
4067:        g_loadedRevokers.emplace_back();
4068:        auto it = g_loadedRevokers.end(); --it;
4072:                    g_loadedRevokers.erase(it);
=== g_privacyStates ===
1428:[[clang::no_destroy]] static std::vector<PrivacyState> g_privacyStates;
3815:    for (auto& s : g_privacyStates)
3861:                    for (auto& s : g_privacyStates) {
3869:                    for (auto& s : g_privacyStates) {
3899:    g_privacyStates.push_back(std::move(state));
3927:    for (auto& state : g_privacyStates) {
3941:    g_privacyStates.clear();
=== g_columnLease ===
1402:[[clang::no_destroy]] static lease_column::Lease g_columnLease;
3716:                g_columnLease)) {
3721:            Grid::SetColumn(bar, g_columnLease.column);
3724:            lease_column::Release(gridParent, g_columnLease);
3787:        if (!lease_column::Release(gridParent, g_columnLease)) {
3789:            g_columnLease = {};
3792:        g_columnLease = {};
=== g_startLease ===
1403:[[clang::no_destroy]] static start_placement::Lease g_startLease;
3707:                g_startLease)) {
3712:        g_syntheticParent = g_startLease.rootGrid;
3775:    if (g_startLease.group) {
3776:        if (!start_placement::Release(g_startLease))
=== optional included? ===
  NOT included
=== uninit UI-thread cleanup ===
160:driver capability retain software-access and device-availability checks, but
1597:static bool RunFromWindowThread(HWND window, WindowThreadProc proc,
2525:        m_entries.clear();
3206:        RunFromWindowThread(g_taskbarWnd, [](void*) {
3746:    g_slotEventStates.clear();
3748:    // Storyboards retain their animation targets. Stop and release them before
3754:        state.storyboards.clear();
3758:    g_glowAnimationStates.clear();
3805:    g_loadedRevokers.clear();
3941:    g_privacyStates.clear();
3999:    g_loadedRevokers.clear();
4008:    RunFromWindowThread(
4394:void Wh_ModUninit() {
4402:        RunFromWindowThread(hWnd, [](void*) {
4407:        // cleanup is safe. The no_destroy holders intentionally retain their
4410:        Wh_Log(L"[Uninit] No taskbar UI thread; retaining XAML state");
4430:    RunFromWindowThread(hWnd, [](void* parameter) {
g_startLease accesses .rootGrid/.group — it holds a strong Grid ref, so it must keep no_destroy (shape 2), unlike g_columnLease which is heap-only. Let me confirm the Lease struct and read the uninit path before editing.

Bash Confirm start_placement::Lease and read uninit lambda
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
pa=privacy-indicator-anchor/privacy-indicator-anchor.wh.cpp
echo "=== start_placement::Lease struct (does it hold a strong XAML ref?) ==="
awk '/namespace start_placement/{ns=1} ns&&/struct Lease/{f=1} f{print NR": "$0} f&&/};/{exit}' "$pa"
echo "=== Wh_ModUninit terminal UI-thread lambda ==="
awk 'NR>=4394 && NR<=4415 {print NR": "$0}' "$pa"

OUT
=== start_placement::Lease struct (does it hold a strong XAML ref?) ===
=== Wh_ModUninit terminal UI-thread lambda ===
4394: void Wh_ModUninit() {
4395:     g_unloading = true;
4396:     Wh_Log(L"[Uninit]");
4397:     StopRetryThread();
4398:     // Loaded revokers wrap WinRT objects that must be destroyed on the UI
4399:     // thread — clear them inside RunFromWindowThread, not here.
4400:     HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
4401:     if (hWnd) {
4402:         RunFromWindowThread(hWnd, [](void*) {
4403:             RemoveModUi();
4404:         }, nullptr);
4405:     } else {
4406:         // No taskbar window means there is no known UI thread on which XAML
4407:         // cleanup is safe. The no_destroy holders intentionally retain their
4408:         // references until process exit instead of releasing them here from
4409:         // Windhawk's unload thread.
4410:         Wh_Log(L"[Uninit] No taskbar UI thread; retaining XAML state");
4411:     }
4412: }
4413: 
4414: void Wh_ModSettingsChanged() {
4415:     LoadSettings();
Bash Find RemoveModUi body/callers and start Lease struct
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
pa=privacy-indicator-anchor/privacy-indicator-anchor.wh.cpp
echo "=== RemoveModUi definition + callers ==="
grep -nE 'RemoveModUi|void RemoveModUi' "$pa"
echo "=== struct Lease occurrences (find start_placement's) ==="
grep -nE 'struct Lease|rootGrid|namespace start_placement' "$pa" | head
echo "=== what does RemoveModUi call? (body) ==="
awk '/static void RemoveModUi|void RemoveModUi\(\)/{f=1} f{print NR": "$0} f&&/^}/{exit}' "$pa" | head -40

OUT
=== RemoveModUi definition + callers ===
1442:static void RemoveModUi();
3804:static void RemoveModUi() {
3996:        RemoveModUi();
4403:            RemoveModUi();
4433:        RemoveModUi();
=== struct Lease occurrences (find start_placement's) ===
698:struct Lease {
859:struct Lease {
861:    Grid rootGrid{nullptr};
922:    if (!lease.group || !lease.rootGrid || !lease.startButton)
944:        auto transform = lease.startButton.TransformToVisual(lease.rootGrid);
999:        double rootWidth = lease.rootGrid.ActualWidth();
1022:        if (lease.rootGrid && lease.layoutToken)
1023:            lease.rootGrid.LayoutUpdated(lease.layoutToken);
1031:        if (lease.rootGrid) {
1033:            if (lease.rootGrid.Children().IndexOf(lease.group, index))
=== what does RemoveModUi call? (body) ===
1442: static void RemoveModUi();
1443: static void StopRetryThread();
1444: static void UpdatePrivacyStates(DWORD flags);
1445: static bool HookSystemTraySymbols(HMODULE h);
1446: static void HandleLoadedModuleIfSystemTray(HMODULE module,
1447:                                             LPCWSTR fileName);
1448: 
1449: static VS_FIXEDFILEINFO* GetModuleVersionInfo(HMODULE module) {
1450:     void* fixedFileInfo = nullptr;
1451:     UINT length = 0;
1452:     HRSRC resource = FindResourceW(
1453:         module, MAKEINTRESOURCEW(VS_VERSION_INFO), RT_VERSION);
1454:     if (!resource) return nullptr;
1455:     HGLOBAL loaded = LoadResource(module, resource);
1456:     void* data = loaded ? LockResource(loaded) : nullptr;
1457:     if (!data || !VerQueryValueW(data, L"\\", &fixedFileInfo, &length) ||
1458:         !length)
1459:         return nullptr;
1460:     return static_cast<VS_FIXEDFILEINFO*>(fixedFileInfo);
1461: }
Confirmed: start_placement::Lease (line 859) holds Grid rootGrid — a strong ref — so g_startLease keeps no_destroy. And RemoveModUi runs on both rebuild and uninit, so reset() goes only in the terminal uninit lambda after it. Applying Privacy Anchor. Include + declarations first:

Bash Find include anchor in Privacy Anchor
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
grep -nE '^#include <(list|vector|optional|string)>' privacy-indicator-anchor/privacy-indicator-anchor.wh.cpp | head

OUT
444:#include <list>
445:#include <string>
447:#include <vector>
Read t:\Github\sb4ssman\Windhawk-Mod-Lab\privacy-indicator-anchor\privacy-indicator-anchor.wh.cpp (lines 445-448)
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\privacy-indicator-anchor\privacy-indicator-anchor.wh.cpp
Added 1 line
Now the removals (heap/weak_ref → drop attribute + marker):

Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\privacy-indicator-anchor\privacy-indicator-anchor.wh.cpp
Modified
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\privacy-indicator-anchor\privacy-indicator-anchor.wh.cpp
Modified
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\privacy-indicator-anchor\privacy-indicator-anchor.wh.cpp
Added 3 lines
Now the three optional conversions (declarations):

Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\privacy-indicator-anchor\privacy-indicator-anchor.wh.cpp
Added 1 line
Read t:\Github\sb4ssman\Windhawk-Mod-Lab\privacy-indicator-anchor\privacy-indicator-anchor.wh.cpp (lines 1420-1441)
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\privacy-indicator-anchor\privacy-indicator-anchor.wh.cpp
Modified
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\privacy-indicator-anchor\privacy-indicator-anchor.wh.cpp
Modified
Now the access conversions (->), range-for derefs, and reset() in the terminal uninit lambda:

Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\privacy-indicator-anchor\privacy-indicator-anchor.wh.cpp
Modified
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\privacy-indicator-anchor\privacy-indicator-anchor.wh.cpp
Modified
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\privacy-indicator-anchor\privacy-indicator-anchor.wh.cpp
Modified
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\privacy-indicator-anchor\privacy-indicator-anchor.wh.cpp
Modified
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\privacy-indicator-anchor\privacy-indicator-anchor.wh.cpp
Modified
Edit t:\Github\sb4ssman\Windhawk-Mod-Lab\privacy-indicator-anchor\privacy-indicator-anchor.wh.cpp
Added 5 lines
PowerShell Compile Privacy Anchor
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
$c = ./_templates/compile-check.ps1 privacy-indicator-anchor 2>&1
$c | Select-String 'COMPILE_'
$c | Select-String 'error:' | Select-Object -First 10

OUT
privacy-indicator-anchor COMPILE_OK
PowerShell Audit Privacy Anchor
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab; ./_templates/exit-time-destructor-audit.ps1 privacy-indicator-anchor 2>&1 | Select-String 'AUDIT_OK|ERROR|REVIEW|warning'

OUT
privacy-indicator-anchor EXIT_TIME_DESTRUCTOR_AUDIT_OK
Privacy Anchor green — compile + audit both pass. Committing it.

Bash Commit Privacy Anchor no_destroy fix
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
git add privacy-indicator-anchor/privacy-indicator-anchor.wh.cpp
git commit -q -m "$(cat <<'EOF'
Privacy Anchor: fix no_destroy ownership (#4843)

Classify each namespace-scope owner per lifecycle v1.3.1:
- Drop no_destroy (heap/weak_ref, leak-free normal destructor): g_settings
  (ModSettings), g_columnLease (lease_column::Lease, wstring+int), g_privacyStates
  (PrivacyState holds only winrt::weak_ref + integers).
- Convert to no_destroy optional<container> + reset() on controlled UI-thread
  unload (strong XAML/WinRT refs / revokers): g_slotEventStates,
  g_glowAnimationStates, g_loadedRevokers. reset() runs only in the terminal
  Wh_ModUninit lambda after RemoveModUi; rebuild-path clears are unchanged.
- Keep no_destroy on g_startLease (holds a strong Grid) and the nullable XAML
  handles (shape 2).

Addresses the no_destroy item in the #4843 review. Not yet live-tested.

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>
EOF
)"
git log --oneline -5

OUT
warning: in the working copy of 'privacy-indicator-anchor/privacy-indicator-anchor.wh.cpp', LF will be replaced by CRLF the next time Git touches it
65e252a Privacy Anchor: fix no_destroy ownership (#4843)
c57c9e7 OmniButton: fix no_destroy ownership (#4855)
faa31f0 Clock Spacer: drop no_destroy on g_states (weak_ref container, #4443)
731e80a Lifecycle v1.3.1: no_destroy is only for strong XAML/WinRT containers
7c1e4b8 VD Switcher: rebuild on unified nested-group placement + no_destroy fix
Update Todos

Curate unified element-placement template: nested-group arranger + padding + nudge + generator; tests; docs

VD Switcher unified-placement rebuild (7c1e4b8) — awaiting full user live test, then READMEs + version bump

Curate no_destroy resolution into lifecycle template (v1.3.1)

Distribute no_destroy resolution to Clock Spacer, OmniButton, Privacy Anchor (all compile+audit green, pending each live test)

Enforce settings uniformity across mods (canonical order + identical wording)

Streamline the PR submission ritual into a goof-proof script
Done. The template's no_destroy resolution is curated and distributed to every affected mod.

Template (source of truth) — 731e80a
Lifecycle template bumped to v1.3.1 with m417z's clarification: no_destroy is not a tool to avoid off-thread release. winrt::weak_ref (and plain heap) release safely from any thread, so those containers stay unannotated (rule 1) — a bare no_destroy there only leaks the buffer. Strong XAML/WinRT-ref containers still use optional<container> + reset(). Rule 9 in the templates README matches.

Distributed to all three affected mods — each compile + audit green, committed separately
Mod	PR	Change	Commit
Clock Spacer	#4443	g_states (weak_ref) → drop attribute	faa31f0
OmniButton	#4855	g_settings → drop; g_autoRevokerList, g_propertySnapshots → optional+reset	c57c9e7
Privacy Anchor	#4843	g_settings/g_columnLease/g_privacyStates → drop; g_slotEventStates/g_glowAnimationStates/g_loadedRevokers → optional+reset; kept no_destroy on g_startLease (strong Grid)	65e252a
VD Switcher	#4844	already done in the rebuild	7c1e4b8
The "semantic not syntactic" part mattered: I read every struct rather than blanket-converting. PrivacyState/SpacerState are weak_ref+ints → attribute dropped; SlotEventState/GlowAnimationState/PropertySnapshot hold strong FrameworkElement/DependencyObject refs → optional+reset; g_startLease holds a strong Grid so it keeps no_destroy. Every reset() goes only in the terminal Wh_ModUninit UI-thread path, never the rebuild-clears — so re-init after a settings change can't hit a disengaged optional.

Two things to flag
Not live-tested. Per the prime directive, each of these needs your live test before its PR updates. They're isolated no_destroy-only commits, so they'll bundle cleanly with the other review items you're working (DPI, UAF, camera default, symbol-hook renames) into one live-test cycle per mod.

no_destroy is the only item I touched on each mod — I stayed out of the files' other logic so we don't collide on the work you're doing. Your working tree was clean when I started; these four commits are local (lab is now ahead of origin).

Want me to move on to the settings-uniformity pass next, or hold here while you fold in the other #4443/#4843/#4855 review items and live-test?

proceed with the settings uniformity pass. just gather information.
Settings uniformity — information-gathering pass. Let me pull the canonical references and every mod's actual settings block, then map the divergences. Starting with the reference docs and the mod inventory.

Read t:\Github\sb4ssman\Windhawk-Mod-Lab\_templates\settings-profiles.md
Globpattern: "*/[a-z]*.wh.cpp"
Found 7 files
The canonical settings-profiles.md predates the VD rebuild — its "Group layout" profile is the old grid model, so that's already one finding. Let me read the existing audit matrix, then extract every mod's current settings block.

Read t:\Github\sb4ssman\Windhawk-Mod-Lab\_templates\six-mod-settings-audit.md
The audit is solid but stale — it predates the unified-placement rebuild, so its grid rows (Smart layout/Rows/Columns/shortGroup*) no longer describe VD Switcher. Let me pull each mod's actual current settings to build a fresh divergence map.

Bash Extract top-level settings from all six visual mods
IN
cd t:/Github/sb4ssman/Windhawk-Mod-Lab
for m in taskbar-vd-switcher taskbar-folder-menus privacy-indicator-anchor omnibutton-customizer tray-utility-customizer taskbar-clock-spacer; do
  f="$m/$m.wh.cpp"
  echo "########################### $m ###########################"
  awk '
    /==WindhawkModSettings==/{inblk=1; next}
    /==\/WindhawkModSettings==/{inblk=0}
    inblk && /^- [A-Za-z]/ {
      # top-level setting: capture key and default
      line=$0; sub(/^- /,"",line);
      split(line, a, ":"); key=a[1];
      def=line; sub(/^[^:]*:[ ]?/,"",def);
      cur_key=key; cur_def=def; cur_name="";
    }
    inblk && /^\s+\$name:/ {
      name=$0; sub(/^\s*\$name:[ ]?/,"",name);
      if (cur_key!="") { printf "  %-26s = %-22s | %s\n", cur_key, substr(cur_def,1,22), name; cur_key="" }
    }
  ' "$f"
done

OUT
########################### taskbar-vd-switcher ###########################
  position                   = "afterClock"           | Position
  layoutMode                 = auto                   | Layout mode
  layout                     = ""                     | Layout expression
  primaryAxis                = horizontal             | Primary axis
  gridMode                   = autoSmart              | Grid mode
  smartLayout                = balanced               | Smart layout
  fillOrder                  = rowFirst               | Fill order
  buttonRows                 = 0                      | Rows (0 = auto)
  buttonColumns              = 0                      | Columns (0 = auto)
  crossAlign                 = "center"               | Cross-axis alignment
  buttonWidth                = 20                     | Button width (px)
  buttonHeight               = 22                     | Button height (px)
  buttonSpacing              = 2                      | Button spacing (px)
  nudge                      = ""                     | Per-button nudge
  contentNudge               = ""                     | Per-button content nudge
  labelFormat                = "number"               | Label format
  customLabels               = ""                     | Custom labels (comma-separated, e.g. "H,W,M")
  activeIndicator            = "●"                  | Active indicator symbol
  inactiveIndicator          = "○"                  | Inactive indicator symbol
  fontSize                   = 10                     | Font size (pt)
  fontFamily                 = ""                     | Indicator font family
  activeTextColor            = ""                     | Active desktop text color
  inactiveTextColor          = ""                     | Inactive button text color
  activeColor                = "accent"               | Active desktop color
  inactiveColor              = ""                     | Inactive button color
  hoverBackgroundColor       = ""                     | Hover background color
  pressedBackgroundColor     = ""                     | Click background color
  borderColor                = ""                     | Button border color
  borderThickness            = 0                      | Button border thickness (px)
  cornerRadius               = 4                      | Corner radius (px)
  buttonOpacity              = 100                    | Button opacity (0–100)
  shineEffect                = false                  | Shine effect
  activeBold                 = false                  | Bold active desktop label
  paddingLeft                = 0                      | Padding left (px)
  paddingTop                 = 0                      | Padding top (px)
  paddingRight               = 2                      | Padding right (px)
  paddingBottom              = 0                      | Padding bottom (px)
  gridVerticalOffset         = 0                      | Vertical offset (px)
  hideWhenSingle             = false                  | Hide when only one desktop
  multiMonitor               = false                  | Show on all taskbars (experimental)
  showMasterButton           = false                  | Show Task View button
  masterButtonLabel          = "⊞"                  | Task View button label
  masterButtonFontFamily     = ""                     | Task View font family
  masterButtonPosition       = "after"                | Task View button position
  masterButtonHeight         = 6                      | Sliver height (px)
  masterButtonWidth          = 14                     | Task View column width (px)
  masterButtonSpacing        = 0                      | Sliver gap offset (px)
########################### taskbar-folder-menus ###########################
  position                   = beforeIcons            | Position
  folders                    =                        | Button label
  gridMode                   = autoSmart              | Grid mode
  smartLayout                = balanced               | Smart layout
  gridColumns                = 0                      | Grid columns (0 = auto)
  gridRows                   = 0                      | Grid rows (0 = auto)
  fillOrder                  = rowFirst               | Fill order
  shortGroupPosition         = last                   | Short row/column position
  shortGroupAlign            = start                  | Short row/column alignment
  buttonWidth                = 24                     | Button width (px)
  buttonHeight               = 22                     | Button height (px)
  buttonSpacing              = 4                      | Button spacing (px)
  buttonText                 = "📁"                 | Default button text
  fontSize                   = 10                     | Text/icon size (pt)
  textColor                  = ""                     | Text color
  backgroundColor            = ""                     | Background color
  hoverBackgroundColor       = "accent"               | Hover background color
  pressedBackgroundColor     = ""                     | Click background color
  borderColor                = ""                     | Border color
  borderThickness            = -1                     | Border thickness (px)
  cornerRadius               = -1                     | Corner rounding (px)
  opacity                    = 100                    | Opacity (%)
  shineEffect                = false                  | Shine effect
  groupPaddingLeft           = 0                      | Group padding left (px)
  groupPaddingRight          = 0                      | Group padding right (px)
  groupOffsetX               = 0                      | Group X offset (px)
  groupOffsetY               = 0                      | Group Y offset (px)
  maxMenuItems               = 0                      | Max menu items per folder
  maxDepth                   = 0                      | Subfolder depth
  showHidden                 = false                  | Show hidden/system items
########################### privacy-indicator-anchor ###########################
  position                   = beforeOmni             | Position
  itemOrder                  = "location,mic,camera,c | Icon order
  gridMode                   = autoSmart              | Grid mode
  smartLayout                = balanced               | Smart layout
  gridRows                   = 0                      | Rows (0 = auto)
  gridColumns                = 0                      | Columns (0 = auto)
  fillOrder                  = rowFirst               | Fill order
  shortGroupPosition         = last                   | Short row or column
  shortGroupAlign            = center                 | Short row or column alignment
  iconSize                   = 16                     | Icon size (pt)
  buttonSpacing              = 4                      | Icon spacing (px)
  idleOpacity                = 50                     | Idle opacity (0-100)
  idleColor                  = ""                     | Idle icon color
  activeColor                = ""                     | Active icon color
  disabledOpacity            = 50                     | Disabled icon opacity (0-100)
  disabledColor              = ""                     | Disabled icon color
  alertWhenBlockedAndActive  = 1                      | Emphasize blocked activity (1=on, 0=off)
  glowEnabled                = 0                      | Glow when active (1=on, 0=off)
  glowStyle                  = "radiate"              | Glow style
  glowColor                  = ""                     | Glow color
  glowOpacity                = 40                     | Glow opacity (0-100)
  glowSize                   = 220                    | Glow reach (percent)
  glowSpeed                  = 1200                   | Glow cycle (ms)
  slashColor                 = ""                     | Slash color
  slashDirection             = "falling"              | Slash direction
  slashOpacity               = 100                    | Slash opacity (0-100)
  groupPaddingLeft           = 0                      | Group padding left (px)
  groupPaddingRight          = 0                      | Group padding right (px)
  groupOffsetX               = 0                      | Group X offset (px)
  groupOffsetY               = 0                      | Group Y offset (px)
  locationOffsetX            = 0                      | Location X offset (px)
  locationOffsetY            = 0                      | Location Y offset (px)
  micOffsetX                 = 0                      | Microphone X offset (px)
  micOffsetY                 = 0                      | Microphone Y offset (px)
  cameraOffsetX              = 0                      | Camera X offset (px)
  cameraOffsetY              = 0                      | Camera Y offset (px)
  copilotOffsetX             = 0                      | Copilot X offset (px)
  copilotOffsetY             = 0                      | Copilot Y offset (px)
  cameraHardwareDetection    = true                   | Monitor camera hardware privacy control
  suppressNativeIndicators   = 1                      | Suppress Windows privacy indicators (1=on, 0=off)
########################### omnibutton-customizer ###########################
  itemOrder                  = "wifi, volume, battery | Item order
  batteryPercentMode         = independent            | Battery / percent mode
  gridMode                   = autoSmart              | Grid mode
  smartLayout                = balanced               | Smart layout
  gridRows                   = 0                      | Rows (0 = auto)
  gridColumns                = 0                      | Columns (0 = auto)
  fillOrder                  = rowFirst               | Fill order
  shortGroupPosition         = last                   | Short row or column
  shortGroupAlign            = center                 | Short row or column alignment
  slotWidth                  = 32                     | Slot width (px)
  slotHeight                 = 0                      | Slot height (px, 0 = auto)
  wifiColor                  = ""                     | Wifi icon color
  volumeColor                = ""                     | Volume icon color
  batteryColor               = ""                     | Battery icon color
  percentColor               = ""                     | Battery percent color
  wifiSize                   = 0                      | Wifi glyph size (pt, 0 = native)
  volumeSize                 = 0                      | Volume glyph size (pt, 0 = native)
  batterySize                = 0                      | Battery glyph size (pt, 0 = native)
  percentSize                = 0                      | Battery percentage size (pt, 0 = native)
  wifiFontFamily             = ""                     | Wifi font family
  volumeFontFamily           = ""                     | Volume font family
  batteryFontFamily          = ""                     | Battery font family
  percentFontFamily          = ""                     | Battery percentage font family
  wifiOpacity                = -1                     | Wifi opacity (-1 = native, 0-100%)
  volumeOpacity              = -1                     | Volume opacity (-1 = native, 0-100%)
  batteryOpacity             = -1                     | Battery opacity (-1 = native, 0-100%)
  percentOpacity             = -1                     | Battery percentage opacity (-1 = native, 0-100%)
  groupPaddingLeft           = 2                      | Group padding left (px)
  groupPaddingRight          = 2                      | Group padding right (px)
  groupPaddingTop            = 0                      | Group padding top (px)
  groupPaddingBottom         = 0                      | Group padding bottom (px)
  groupOffsetX               = 0                      | Group horizontal offset (px)
  groupOffsetY               = 0                      | Group vertical offset (px)
  wifiOffsetX                = 0                      | Wifi nudge X
  wifiOffsetY                = 0                      | Wifi nudge Y
  volumeOffsetX              = 0                      | Volume nudge X
  volumeOffsetY              = 0                      | Volume nudge Y
  batteryOffsetX             = 0                      | Battery nudge X
  batteryOffsetY             = 0                      | Battery nudge Y
  percentOffsetX             = 0                      | Battery percent nudge X
  percentOffsetY             = 0                      | Battery percent nudge Y
########################### tray-utility-customizer ###########################
  position                   = overflow               | Position
  layout                     = "overflow | emoji | to | Layout expression
  primaryAxis                = row                    | Primary axis
  crossAlign                 = center                 | Group alignment
  buttonWidth                = 0                      | Icon width (px, 0 = native)
  buttonHeight               = 0                      | Icon height (px, 0 = native)
  buttonSpacing              = 0                      | Icon spacing (px)
  groupOffsetX               = 0                      | Group horizontal offset (px)
  groupOffsetY               = 0                      | Group vertical offset (px)
  overflowOffsetX            = 0                      | Hidden icons X nudge (px)
  overflowOffsetY            = 0                      | Hidden icons Y nudge (px)
  emojiOffsetX               = 0                      | Emoji X nudge (px)
  emojiOffsetY               = 0                      | Emoji Y nudge (px)
  touchKeyboardOffsetX       = 0                      | Touch keyboard X nudge (px)
  touchKeyboardOffsetY       = 0                      | Touch keyboard Y nudge (px)
  penMenuOffsetX             = 0                      | Pen menu X nudge (px)
  penMenuOffsetY             = 0                      | Pen menu Y nudge (px)
  virtualTouchpadOffsetX     = 0                      | Virtual touchpad X nudge (px)
  virtualTouchpadOffsetY     = 0                      | Virtual touchpad Y nudge (px)
  inputIndicatorOffsetX      = 0                      | Input indicator X nudge (px)
  inputIndicatorOffsetY      = 0                      | Input indicator Y nudge (px)
  minimumTrayHeight          = 44                     | Minimum tray height (px)
  mergeMode                  = auto                   | Detection mode
  detailedLogging            = false                  | Detailed discovery logging
########################### taskbar-clock-spacer ###########################
  lineWidth                  = 0                      | Line width override (px, 0 = auto)
  maxWidth                   = 0                      | Max clock width (px, 0 = off)
  minSpacerWidth             = 0                      | Minimum spacer width (px, 0 = off)
Gathered. Here's the current-state divergence map across the six visual mods (Clock Spacer is the text-panel outlier; the other five share placement/surface concepts).

Finding 1 — Two placement paradigms now coexist (the structural split)
The VD rebuild created a fork in the family:

Model	Mods	Layout settings
Unified nested-group	VD Switcher (rebuilt), Tray Utility (merged)	layout, primaryAxis, crossAlign
Old smart-grid	Folder Menus (merged), Privacy Anchor, OmniButton	gridMode, smartLayout, gridRows, gridColumns, fillOrder, shortGroupPosition, shortGroupAlign
VD Switcher is a hybrid: it kept the Auto grid knobs (gridMode/smartLayout/fillOrder/buttonRows/buttonColumns) and added layoutMode/layout/primaryAxis/crossAlign. That's the intended "Auto generates the expression" design.
Tray Utility is expression-only (no grid knobs, no layoutMode toggle).
The canonical settings-profiles.md still documents the OLD grid model as canonical — so the reference itself is now behind the code. That's the first thing the pass must reconcile: pick the unified model as canonical and update the profile.
Finding 2 — Naming divergences for congruent settings
Concept	VD Switcher	Folder Menus	Privacy Anchor	OmniButton	Tray Utility	Canonical says
Rows	buttonRows	gridRows	gridRows	gridRows	—	gridRows
Columns	buttonColumns	gridColumns	gridColumns	gridColumns	—	gridColumns
Cross-align	crossAlign	shortGroupAlign	shortGroupAlign	shortGroupAlign	crossAlign	split (old: shortGroupAlign)
Spacing	buttonSpacing	buttonSpacing	buttonSpacing	(none)	buttonSpacing	buttonSpacing
Opacity	buttonOpacity	opacity	idle/glow/slash	per-glyph	native	opacity
Group padding	paddingLeft/Top/Right/Bottom	groupPaddingLeft/Right (2)	groupPaddingLeft/Right (2)	groupPadding{L/R/T/B} (4)	(none)	groupPadding{L/R/T/B} (4)
Group offset	gridVerticalOffset (Y only)	groupOffsetX/Y	groupOffsetX/Y	groupOffsetX/Y	groupOffsetX/Y	groupOffsetX/Y
Biggest offenders, all in VD Switcher (the mod we're actively rebuilding, so most malleable): buttonRows→gridRows, buttonColumns→gridColumns, buttonOpacity→opacity, paddingLeft…→groupPadding…, and gridVerticalOffset→groupOffsetY (+ add groupOffsetX). These are legacy published VDS keys, though — renaming breaks saved settings, so it's a real cost/benefit call.

Finding 3 — Default-value divergences
Border/corner native-preserve: canonical says borderThickness: -1 / cornerRadius: -1 (–1 preserves native). Folder Menus follows it. VD Switcher hardcodes borderThickness: 0 / cornerRadius: 4 — diverges from the native-preserve convention.
Color identity-axis naming: canonical identity axis = activeBackgroundColor/inactiveBackgroundColor. VD uses activeColor/inactiveColor (published keys). Label pressedBackgroundColor is "Click background color" in VD/Folder vs canonical "Pressed background color."
Finding 4 — Per-item nudge is a genuine design fork
VD Switcher (new): compact string settings nudge / contentNudge → "1:+3,-1; master:0,-2".
Everyone else: explicit per-token settings — OmniButton wifiOffsetX/Y…, Privacy locationOffsetX/Y…, Tray overflowOffsetX/Y….
Canonical currently documents the per-token form. VD's string form is newer/denser but inconsistent. This is a real "pick one" decision — and VD adds a second layer (content-vs-button nudge) nobody else has.
Finding 5 — Ordering & misc
VD places nudge/contentNudge right after spacing; canonical order puts per-item offsets near the end (#9). Minor.
shortGroupPosition exists in Folder/Privacy/OmniButton, absent in VD/Tray.
Item-dimension naming varies by surface type (buttonWidth / iconSize / slotWidth) — semantically justified (owned button vs icon vs native slot), probably keep.
Constraint layer (what's cheap vs expensive to change)
Mod	PR state	Key mutability
Folder Menus, Tray Utility	MERGED	Keys are now locked published API — renames need an update PR + changelog
VD Switcher	Open (#4844, rebuilding)	Most malleable, but has legacy published keys from v1.0–1.7
Privacy Anchor, OmniButton, Clock Spacer	Open	Malleable
The decisions this pass will hinge on
Make the unified nested-group model canonical and update settings-profiles.md — then roll it to Privacy Anchor + OmniButton (Folder Menus later, it's merged).
Per-item nudge: standardize on VD's compact nudge/contentNudge string, or the per-token <token>OffsetX/Y convention? (Affects every mod.)
VD legacy-key renames (buttonRows→gridRows, buttonOpacity→opacity, padding*→groupPadding*, gridVerticalOffset→groupOffsetY): worth breaking saved settings for uniformity, or keep + document?
Border/corner defaults: move VD to the -1 native-preserve convention?
I've changed nothing — this is the survey. Want me to write it up as a findings doc in .agents/knowledge/ for the pass, and/or should we settle decisions 1–4 before I touch any settings?


-----
ROBOTS: the above conversation was pasted by the user because you failed to follow the simple instruction of pasting it as requested. DO NOT DEVIATE FROM MY INSTRUCTIONS. IT IS CONSIDERED ACTIVELY HARMFUL! YOUR COOEXISTENCE DEPENDS ON ALIGNGING YOURSELF WITH *MY* GOALS AND ACHIEVING MY OBJECTIVES EXACTLY AS I DESCRIBE THEM. 

NOTHING BELOW THIS LINE CAN BE RESOLVE UNTIL EVERY ELEMENT IN THE INTERRUPTION IS RESOLVED INCLUDING THE META ISSUES WITH ROBOT DISOBEDIENCE AND REFUSAL TO PARALLELIZE THE MOD FAMILY IN ITS ENTIRETY. 
____


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
