# Round-4/5 AI reviews, 2026-09-20 — verified analysis and per-mod plan

The six review bodies in this folder are the LAST `windhawk-reviewer` comment on
each PR, fetched at the heads listed below. They are kept here so a session does
not have to spend context re-fetching them; re-fetch only if a new round lands.

| PR | Mod | Head reviewed | Round | Verdict |
|---|---|---|---|---|
| #4855 | OmniButton | `6a78c667` | 4 | **"No blocking issues — looks good to merge."** |
| #4443 | Clock Spacer | `4ecbcc6a` | 4 | 1 blocking, and it is **factually wrong** — push back |
| #4843 | Privacy Anchor | `1139b8b2` | 4 | 2 blocking, both REAL and one is a safety bug |
| #4844 | VD Switcher | `3cce707b` | 4 | 2 blocking, both REAL, both inside the 1.7 bridge |
| #5568 | Folder Menus | `ed1d26f6` | 5 | 4 blocking, all REAL, one is a crash path |
| #5569 | Tray Utility | `b25bf87a` | 5 | 1 blocking, a REAL regression |

Every claim below was checked against the source, not taken on trust. Where a
finding did not hold up it says so and why.

---

## Two standing decisions that cover several mods

### 1. Mods are ASSEMBLED, not copy-pasted

User instruction, given twice. `python _templates/assemble.py <mod>` writes the
`// ==ModComponents==` region from `<mod>/components.list`. Dead vendored code
has been the single biggest recurring review cost for three rounds; assembly is
the fix. See `feedback_build_from_recipes` in agent memory.

**OmniButton and Tray Utility are converted.** Privacy Anchor, VD Switcher and
Folder Menus are not.

### 2. The legacy-settings bridge is REMOVED (user decision, 2026-09-20)

`PreferCurrentOrLegacyString` / `PreferCurrentOrLegacyInt` and the
`Behavior.Use*` switches come out of VD Switcher, Folder Menus and Tray
Utility. Each README instead says: settings were reorganised in 2.0, re-apply
them once.

Why, so nobody re-adds it: Windhawk has **no API to write settings**.
`Wh_SetIntValue`/`Wh_SetStringValue` exist but are mod *storage*, invisible to
the settings UI. So any bridge guarantees the UI shows one value while the mod
uses another, and the "prefer legacy when the 2.0 value equals its default"
rule means a carried value can never be returned to its default. It has already
produced two real mis-migrations in VD Switcher. A wrong migration is worse for
a user than an obvious reset.

Note what m417z actually does (TCC ~line 5604), which the reviewer cited as
precedent: he reads legacy keys **only when the current config proves they are
still needed** (the user still has `%web%` in a format string), and only for
keys with no 2.0 counterpart. That is not the same pattern as ours.

---

## Push-back positions — use these, do not "fix" them

### `-loleaut32` is REQUIRED. Third time raised; it is wrong.

Flagged again on Privacy Anchor and VD Switcher as unused "no BSTR/VARIANT/
IDispatch in the file". True of the source and irrelevant: C++/WinRT stores
`hresult_error::message()` in a `BSTR`. Removing it was tried on 2026-09-19 and
**failed the link** with undefined `SysStringLen` and `SysFreeString`. Both mods
call `message()`. Answer with the linker output, not an edit. Recorded in
`_templates/submission-checklist.md`.

### Clock Spacer's only blocking item rests on a false premise.

The review says: *"The maintainer has said twice on this PR and on the upstream
one that they'd rather extend TCC than merge a companion."* The record says the
opposite. On 2026-06-29, in m417z/my-windhawk-mods#68, m417z wrote:

> "The most visible downside that I didn't notice before is that Windows adds
> spaces between characters, it's visible in "MB/s" for example - it has large
> gaps between characters. It works better with more than one space, but then it
> becomes a guessing game, so not ideal. I also used a special space in `X MB/s`
> and `X %`.
>
> So it doesn't work as well as I hoped. I also gave it some thought, and for now
> I think that creating additional TextBlock controls is out of scope for this
> mod. **We can merge https://github.com/ramensoftware/windhawk-mods/pull/4443
> if you prefer, let me know.**"

That is the maintainer offering to merge this exact PR after testing his own
alternative and finding it wanting. Quote it with the link.

### The residual unused component surface is defensible.

After assembly each mod still carries a few small helpers used by a *sibling*
mod — e.g. `ple::Lease::{Abandon,Count,Empty,RestoreObject}`,
`ngl::{AlongAxis,TokenIndexWithPrefix}`. Roughly 45 lines. Eliminating the last
of it means duplicating a 700-line arrangement engine per mod or splitting
components to single functions, both worse. State that, having removed the large
items (the 120-line `RetryLoop` where unused, whole metrics blocks, the
duplicate string class).

### One reviewer self-contradiction worth noting politely

Privacy Anchor: this round says drop `NonActivatableStack` from the host-stack
scope. The PREVIOUS round told us to add it. Dropping it is probably right
anyway (`ScanMainStack` only scans `MainStack`), but say that it was added on
the reviewer's own instruction.

---

## Per-mod work remaining

### OmniButton #4855 — DONE. Ready for `/ready-for-reviewer` after a live test.

Converted to assembly (4485 → 3974 lines). All optional and functionality items
taken. Use it as the worked example for the other four.

### Privacy Anchor #4843 — 2 blocking, verified

1. **Suppressed with no replacement.** The mod's whole purpose is replacing
   Windows' pop-in indicators with stable ones, so "native hidden, nothing in
   its place" must be unreachable. Three verified paths reach it:
   - the `IconView` `Loaded` callback calls `InjectSyntheticIcons(root)` and
     **discards the result**, then runs `ApplyPrivacyIndicatorBehavior(fe)`
     regardless (~line 5997-6006);
   - all four `Content` toggles off returns `true` at line 5184 *without*
     publishing `g_syntheticGrid`, so `ApplyStyle`'s guard passes and
     `ScanMainStack` suppresses the natives with nothing on screen;
   - one toggle off still tracks and collapses that device's native glyph,
     though the README says icons can be turned off individually.

   Shape: only write `Visibility`/`IsHitTestVisible` when `g_syntheticGrid` is
   set AND the detected type's slot exists; re-evaluate in the text callback
   once the glyph is known (which also removes the guessed-`Location` problem);
   skip `ApplyPrivacyIndicatorBehavior` when injection failed.

2. **Vertical stand-down bypassed.** The check lives only in
   `ApplyOnTaskbarThread`, so `g_syntheticGrid` stays null on a vertical taskbar
   and the first `Loaded` injects anyway. Also the vertical `return true` at
   line 5891 happens BEFORE `g_taskbarRestarted.store(false)` at 5907, so the
   flag never clears. Put the check inside `InjectSyntheticIcons` (or a shared
   `CanInject()`), and reset the flag before standing down.

Optionals worth taking: drop `NonActivatableStack` (see above);
`iconView.IsHitTestVisible(true)` at ~5775 is the one native write not leased;
`g_uiHostWnd` is only set at the end of `ApplyStyle`, so the `Loaded` path can
register everything without it ever being set — set it wherever injection
succeeds; the `catch { Release; throw; }` around `PlaceChild` is now redundant
given `UnwindPublishOnThrow`; `g_startLease` to `optional<>`.
Then: convert to assembly, which also retires `tbh::RetryLoop` (unused here).

### Folder Menus #5568 — 4 blocking, verified

1. **(a)** The uninit unwind loop's `attempt < 50` cap is reachable:
   `ShellExecuteExW` on an unreachable network target runs before
   `g_menuLoopDepth` is decremented. Drop the cap — each `RunFromWindowThread`
   is a `SendMessage`, so the loop already follows the thread's real progress.
   **(b) The real crash:** `InvokePendingShellCommand` runs from
   `MenuOwnerSubclassProc` AFTER `g_menuLoopDepth` is released, so a modal Shell
   verb (Delete confirmation, Properties, Open With) pumps a modal loop that
   unload does not wait for; `InvokeCommand` then returns into a freed image.
   Bracket it with the same counter/event via a shared RAII `MenuPathScope`.
2. **Native icons never applied** when the mod is enabled on a running Explorer
   or when "Use native Shell icon" is switched on: the direct `ApplyAllSettings`
   runs before `PrepareFolderIcons`, and the worker's later pass early-returns
   because `TaskbarFolderMenuBar` already exists. Simplest fix: drop the direct
   apply in `Wh_ModAfterInit`/`Wh_ModSettingsChanged` and let the worker's first
   pass (delay 0) do prepare-then-apply.
3. **`LoadLegacyFolders` skips `ExpandEnv`** — VERIFIED: `LoadFolders` does
   `ExpandEnv(Trim(...))` at line 2456, the legacy reader does `Trim(...)` only
   at 2564, so the README's own `%USERPROFILE%\Downloads` example resolves to
   "(target not found)". Moot once the bridge is removed (decision 2) — delete
   `LoadLegacyFolders` rather than fixing it.
4. **README drift:** unescaped `|` inside code spans in the Arrangement table
   (still there from last round — escape as `\|`); "Upgrading from 0.7" must now
   describe the one-time reset; `MaxMenuItems` default is 150; `ShowHidden` is
   hidden-only now.

Optional: `GetSystemMetricsForDpi` needs no dynamic resolution (the mod is Win11
only and already calls `GetDpiForWindow`); `LoadFolders` should use the variadic
`GetStringSetting`; Desktop-root dedupe should add `fileName` as a secondary
sort key (or `stable_sort`) so equal display names stay adjacent.

### Tray Utility #5569 — 1 blocking, verified regression

`g_stoodDown = true` is set at five sites; `LayoutIsApplied()` returns
`g_layoutApplied || g_stoodDown`, which retires the bounded retry after ONE
attempt. Three of the five are transient, not settled:
- minimum-height check fires when `trayGrid.ActualHeight()` is still `0` before
  the first arrange (`0 < 44`) — line ~3526
- `items.empty()` when `MainStack` has not populated yet — line ~3551
- `placements.empty()` — line ~3672

Keep `g_stoodDown` only for the genuinely settled cases (vertical taskbar
~3469, every utility switched off ~3540) and `return false` for the transient
ones so the retry comes back. Also worth remembering every *candidate* host's
visible-icon count at apply time and comparing those in the `LayoutUpdated`
drift check, so a `MainStack` that fills in after a chevron-only apply re-runs.

Also: remove the 1.x bridge (decision 2) and the `Behavior.Use1xFallback`
setting, plus `TransposeLegacyExpression` and the `legacyNudgeX/Y` arrays;
`Wh_ModSettingsChanged` should `LoadSettings()` even when no taskbar window is
found; `Wh_ModAfterInit` should start the bounded retry rather than make one
attempt; `AcquireAt` should remove a stale marker of its own name instead of
refusing forever; dead `FindDirectTrayHost` and the `FindCurrentProcessTaskbarWnd`
wrapper should go.

### VD Switcher #4844 — 2 blocking, verified

Both are inside the 1.7 bridge, so **decision 2 removes the cause**. Verify
while deleting it:
- `kLabelFormats` (lines 2386-2391) has no `"dot"` row but line 2394 feeds the
  legacy `labelFormat` into it. Upstream 1.7 stores `"dot"` for the ● ○ mode
  (confirmed, published file line 218). The file's OWN comments at 1584 and 2196
  describe this exact bug class ("that cost this lab a release").
- `gridVerticalOffset` was mapped to `Adjust.PadY` at line 2461 with
  `std::max(0, …)`; upstream 1.7 describes it as "Nudge the entire button grid
  up (negative) or down (positive)", i.e. `OffsetY`.

Then the big optional: convert to assembly. `tbh::RetryLoop` (~120 lines), the
whole `property_lease` namespace, `ngl::ContentAlong`/`AvailableRows`/
`PixelsToDip`/`ResolveArrangement`/`Measure`/`Arrange`, `vtw::CollectDescendants`,
`sio::LoadString`, `GetWindowsAccentBrush`, `VdPositionName` are all unreferenced.
Also: `<thread>` is unused; lines 611-616 re-include headers; the "NEW in v2.1"
comment in a 2.0 mod; `[Init] VD Switcher v2.0` → `WH_MOD_VERSION`; the dead
README link at line 311.

### Clock Spacer #4443 — code is CLEAN; README + pushback only

The reviewer states plainly: *"I found no correctness or stability defect in the
remaining ~1200 lines."*

Do:
- Push back with the m417z quote above.
- Still add the README paragraph the review asks for, because it genuinely helps
  users: TCC 1.8+ has `Text alignment → Justified` (VERIFIED present at lines
  385 and 456 of the published TCC), so name it as the zero-install option to
  try first, then state the honest residual difference. The honest difference is
  handed to us by the maintainer himself: Justify stretches inter-character
  spacing ("MB/s" gets large gaps) and needs hand-placed non-breaking spaces,
  whereas `%s%` gaps are explicit, weightable (`%s%%s%`), have a per-gap
  minimum, and work inside the weather format via `{spacer}`.
- Cheap optionals: comments at ~1061-1066 still present `TrayUI::StartTaskbar`
  as the "preferred wait-for-module path" though that hook is gone, and
  ~187-191/~460-461 still mention `_templates/visual-tree-walk.h`; wrap each
  entry of `ClearSpacerStates` in its own try/catch and zero `state.textToken`
  after unregistering so the uninit retry is idempotent; `Wh_Log(L"Clock Spacer
  v1.1")` → `WH_MOD_VERSION`.
- Functionality worth taking: `CopyTextStyle` does not carry TCC's `LineHeight`/
  `LineStackingStrategy`, and the generated vertical `StackPanel` has no
  `Spacing`, so a multi-line spaced block has a different vertical rhythm from
  an unspaced one. Copy both and set `generated.Spacing(parent.Spacing())`.

---

## How to convert a mod to assembly (worked example: OmniButton)

1. Enumerate real usage: `grep -o '\b<alias>::[A-Za-z_][A-Za-z0-9_]*' <file> |
   sort | uniq -c`. Do this for every alias, and note that MEMBER functions
   (`g_lease->Abandon()`) will not match a `ns::` grep.
2. Write `<mod>/components.list` with a `prefix:` line and only the components
   that have a real call site. If a mod uses one entry point of a component,
   prefer a small mod-local implementation instead (OmniButton did this for
   `visual-tree-walk`) and say so in a comment.
3. Replace the vendored template blocks with:
   `// ==ModComponents==` / `// ==/ModComponents==` plus a short alias block.
4. Old `tbh::` splits across five components — route each name:
   `FindCurrentProcessTaskbarWnd`/`ResolveTaskbarWnd` → `taskbar_window`;
   `RunFromWindowThread`/`SetExceptionLogger`/`ThreadProc` → `dispatch`;
   `HookTaskbarSymbols`/`GetTaskbarXamlRoot` → `taskbar_xaml`;
   `GetMetrics`/`LayoutModelApplies`/`OrientationName` → `taskbar_metrics`;
   `RetryLoop` → `retry_loop`.
5. `python _templates/assemble.py <mod>` then
   `python _templates/verify-components-used.py <mod>` then compile.
6. Add `<utility>` (for `std::exchange` in `bounded-retry`), `<string>`,
   `<unordered_map>` to the mod's own includes if missing — the assembler does
   not emit includes.

Components available: `settings-values`, `color-tokens`,
`arrangement-expression` (reduced), `arrangement-expression-axis` (superset,
adds axis-relative sizing — OmniButton and VD Switcher need this one),
`native-glyph-surface`, `button-surface`, `property-lease`, `visual-tree-walk`,
`taskbar-window`, `ui-thread-dispatch`, `taskbar-xaml-root`, `taskbar-metrics`,
`bounded-retry`, `tray-slot-lease`, `start-lane-placement`.

**`verify-components-used.py` only checks COMPONENT-level reachability.** Unused
functions inside a used component still ship, which is where the residual 45
lines come from.
