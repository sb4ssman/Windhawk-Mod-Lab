# Working Notes — Windhawk Mod Lab

Living todo list — current state only, pruned every session. Completed work
(past tense) goes in [work_log.md](work_log.md). Durable rules live in
[README.md](README.md); reference material in [knowledge/](knowledge/).

## Current focus — START HERE (handoff, 2026-07-31)

**Roll the settings contract + layout template through the remaining mods.**
VD Switcher is finished and green but deliberately NOT shipped: the user wants
the other mods migrated first, because a snag in one of them may feed back into
the template, and it is cheaper to change the template once than to ship VD
Switcher twice.

**OmniButton v2.0 SHIPPED 2026-08-03** — PR #4855 updated to v2.0, CI green,
awaiting maintainer review. It is the first mod in the family to reach a
maintainer on the 2.0 contract, so its review feedback is the best available
signal for the mods still queued behind it. See its section below.

Still to migrate, one live-tested build at a time — the parity gate reports the
drift: Privacy Anchor (`nested-group-layout.h` **and `start-placement.h`**, the
latter still uninvestigated), VD Switcher and Tray Utility
(`nested-group-layout.h`), Clock Spacer (embeds none).

After the open-PR rollout, **Taskbar Folder Menus and Tray Utility Customizer
are both queued for full 2.0 family-contract upgrades**. Their merged v0.7 and
v1.1 versions are the published baselines, not the end of their family rollout.

### Privacy Anchor — TEMPLATE ROLLOUT DONE AND LIVE-CONFIRMED 2026-08-03

Went from **3 embedded templates to 8** (`TEMPLATE_PARITY_OK (8 embedded,
3 not used)`), the most templated mod in the family. All six gates green.
**The user live-tested it and confirmed everything, including camera
detection.** Commits `3000286` + `9372ca0`, both on `origin/main`.

**The live test surfaced one thing worth recording as doctrine.** The user
reported camera detection "stopped working" and believed the rollout had
removed it. It had not — `CameraPrivacyMonitor` was fully intact and the
commit touched exactly two camera lines, both `Bool(...)` -> `sio::LoadBool(...)`
with identical semantics. The real cause was older: commit `92e767e` (the 2.0
rework) **renamed `cameraHardwareDetection` to
`Behavior.CameraHardwareDetection` AND flipped its default from true to false**
in the same change. Windhawk cannot carry a value across a renamed key, so the
user's "on" was dropped and the new key read as off. A whole capability went
dark with no symptom.

DOCTRINE, now applied here and worth applying family-wide: **never rename a
settings key and change its default in the same change** — either alone is
recoverable, together they silently revert a user's choice with no trace. And
when an opt-in setting gates an entire capability rather than tuning one,
**say so at runtime**: the init log now names which switch is off and what is
lost, and the camera's idle tooltip names the setting on the taskbar itself.

**SUBMITTED 2026-08-03.** PR #4843 is now "Add Tray Privacy Indicator Anchor
v2.0", head `6f71b39f`, all five CI jobs green. Diff vs `upstream/main` is one
ADDED file, `mods/tray-privacy-indicator-anchor.wh.cpp` (named for its `@id`,
not the lab folder), byte-identical to the lab copy.

**Both READMEs were rewritten first, and they needed it.** They were still
v1.0-era: an entire "Icon order and grid layout" section documenting
`itemOrder`, `gridMode`, `smartLayout`, `gridRows`/`gridColumns` and
`shortGroupPosition`/`shortGroupAlign`, none of which exist in the mod.
`README_MATCH` passed the whole time because it compares the two copies to each
other and two identically stale copies agree — the exact trap the notes had
already recorded after OmniButton. **Check the READMEs against the settings
BLOCK, never against each other, before any submission.**

| Template | What changed |
|---|---|
| `nested-group-layout.h` | re-embedded v2.5 — adds `ContentAlong` (additive only) |
| `start-placement.h` | re-embedded v1.2 — **real fix**: centers against the taskbar ROOT height, not Start's own box |
| `color-tokens.h` | NEW — retires the third copy of the token parser |
| `visual-tree-walk.h` | NEW — `FindChildRecursive` is now a thin wrapper over `vtw::FindDescendant` |
| `settings-io.h` | NEW — every `$options` value parses to an enum AT LOAD |
| `taskbar-host.h` | NEW — discovery, dispatch, XamlRoot, symbol hooks, metrics |
| `property-lease.h` | NEW — **exact** restore replaces an inferred one |
| `injected-grid-column.h` | already matched |

**The start-placement drift the notes called "previously UNKNOWN" is
identified and fixed.** v1.2 centers the group on `lease.rootGrid.ActualHeight()`
because Start's own box is not a reliable vertical reference; the mod still had
the v1.1 math. Affects `leftOfStart` / `rightOfStart` ONLY.

**Two changes carry real behavioral risk — test these specifically:**

1. **Native-indicator restore.** `ClearPrivacyStates` used to re-derive what
   the native icon's visibility "should" be from whether its TextBlock had
   text. That is a guess, and it cannot express "there was no local value"
   — the common case for a template-bound tray icon, where the correct
   restore is `ClearValue` so the native binding resumes. Now leased:
   snapshot before first write, restore the exact prior value.
   **Test: disable the mod and confirm the native privacy icons come back
   correctly, both while idle and while the mic/camera is live.**
2. **`leftOfStart` / `rightOfStart` vertical position** moved (see above).

**Deliberately NOT adopted, and why:**

- **`tbh::RetryLoop`** — the mod's retry thread is fused with the privacy
  state worker; `g_stateRefreshEvent` shares its lifecycle in
  `StopRetryThread`. Swapping in the template's bounded apply-retry would
  mean restructuring the whole monitoring architecture. Genuine nuance.
- **`native-glyph-surface.h`** — the mod DRAWS its own icons, so it owns them
  outright; there is no template-bound native leaf to style. It reads native
  glyph text for detection, which is not styling. Same category as VD
  Switcher owning its buttons.
- `button-surface.h`, `os-setting-bridge.h` — not applicable.

**Also gained:** vertical-taskbar stand-down (was OmniButton-only), and
`AvailablePrivacyRows` now takes its DIP height from `tbh::GetMetrics`.
`g_settings` is now heap-free (fixed buffers + enums replaced every
`std::wstring` field).

**Corrections to these notes, verified in source:** the mod does NOT still
embed `smart-grid-layout.h` — zero matches, it already migrated. And all four
#4843 review items are fixed (no dangling `wstring_view`,
`CameraHardwareDetection: false`, 22 `no_destroy` sites, hook arrays named).

### Privacy Anchor — earlier live-confirmed local candidate

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

The user live-confirmed the rebuilt mod, including the follow-up location fix:
after the XAML rebuild clears cached privacy state, the existing `RefreshAll`
monitor path is requested again so an already-denied location state does not
remain "Not requested." The full rework is commit `92e767e`; that nine-line
location reconnection remains uncommitted. Nothing is pushed. READMEs,
screenshots, preflight, and PR update remain intentionally deferred.

### OmniButton — SUBMITTED 2026-08-03 (PR #4855 at v2.0), awaiting review

**DONE. The first mod in the family to ship on the 2.0 contract.** PR #4855 is
now titled "Add OmniButton Customizer v2.0", head `a6bde2de`, all five CI jobs
green (changed-file validation + Windhawk 1.6.1 / 1.7.3 / 2.0.0-alpha.2).
Nothing is owed on it until m417z responds.

What shipped, and in what order:

1. Lab `bf7f638` (on `origin/main`) — the refreshed gallery plus the real
   arrangement strings. The user pushed the gallery commit `03322aa` themselves
   and confirmed the images resolve.
2. Fork branch `add-omnibutton-customizer` amended to a single clean commit
   `a6bde2de` and force-pushed with lease. Diff vs `upstream/main` is exactly
   one ADDED file, `mods/omnibutton-customizer.wh.cpp`, byte-identical to the
   lab copy.
3. PR title + body replaced wholesale. The old body still advertised removed
   v1.0 features and embedded deleted screenshots.

**Version 2.0 is deliberate and the READMEs explain it.** v1.0 never shipped —
it only ever existed as a PR — so the number marks the settings contract and
the arrangement component, not a release history. The migration warning is
narrowed to people who installed 1.x by hand from the PR.

**Gallery doctrine learned here, worth reusing:** audit the screenshots against
the `@description` and the Features list, not against the settings block. Color
was the mod's headline claim with zero screenshots, while five shots covered
arrangement variations. Also: put the user's REAL arrangement strings under the
images. The ones I inferred from the pictures were plausible and wrong in form
(column-formed vs the row-formed string actually used), and only the user could
supply the nudge values that make an arrangement look right.

**Verified against upstream, not from memory (2026-08-03):**
`pr_validation.py:1164` requires `(added, modified, all)` in `[(1,0,1),(0,1,1)]`;
line 1172 requires the literal `## Mod authorship` heading in the body for any
added-file PR, and it reads the body AT PUSH TIME. The PR template uses nested
`- - [ ]` checkboxes and its own `## Changelog` section, with extra content
placed ABOVE the template.

**PR #4855's review, verified line by line against the current code:** both
blocking items are fixed (DPI via `taskbar_host::GetMetrics`; hook array
renamed with the three-module comment). Optional items fixed: no-destroy on
`g_settings`, the optional+reset form, the `LoadColorSetting` duplicate, the
`slotWidth` 1–15 surprise, and the unused `slotW` parameter — that last one had
survived every previous pass and was caught only by re-reading the review.
Four optional items are deliberately declined and the draft body says so:
retry start/stop mutex, `IsWindow()` revalidation, structural wifi/volume
detection, and `WM_DPICHANGED` recompute.

**Assets — GALLERY FULLY REPLACED 2026-08-03.** Every 1.x/early-2.0 screenshot
was deleted and nine fresh ones shot. Both README layers and the root catalog
are rewired onto them; all nine are referenced and exist, so the preflight's
broken/unreferenced asset gate passes.

| File | Shows |
|---|---|
| `single-height-auto.png` | lead image — `auto` picking a 2×2 |
| `single-height-arranged-2x2.png` | the same 2×2 written by hand |
| `single-height-arranged-2-over-1.png` | ragged group, percentage omitted |
| `single-height-arranged-reverse.png` | single row, non-native order |
| `single-height-compact-stack.png` | negative `Size.ItemSpacing` |
| `single-height-diamond-adjusted-with-hover.png` | nesting grammar + native tooltip intact |
| `single-height-stacked-with-percent-emphasis.png` | `Surface.PercentSize` |
| `with-colors.png` | per-item color (green percentage) |
| `double-height-arranged-vertical.png` | double-height taskbar + coexistence |

The capability audit that drove this: color was the one headline claim in the
`@description` with NO screenshot at all, and the old README opened with a
before/after nudge pair whose two files had both been deleted. Color is now
covered by `with-colors.png`. Deliberately NOT shot, per the gallery policy in
README.md ("main purpose and a few meaningful configurations, not every
setting"): opacity, `PercentFontFamily`, fixed `ItemWidth`, `Adjust` padding
and offsets, and a replacement nudge before/after pair.

Two captions are inferred from the images rather than from a recorded setting
and should be confirmed with the user if they are ever rewritten: the 2×2 shot
is captioned with the string `network, battery | volume, percent`, and the
compact-stack shot is described as two-high.

The root catalog row accurately limits size/font controls to the battery
percentage and records the current candidate as live-tested and PR-ready.

**Audit 2026-07-25 — the real cause of "wifi and volume coloring does nothing".**
Not a Codex regression; the same defect is in the committed v1.0. Glyph
TextBlocks were resolved once, on the single pass that discovered the items
host, and every retry gate stood down at that moment: `ApplyAllSettings`
returned `g_omniStackPanel != nullptr`, so `g_applied` went true, so the retry
thread broke out, the IconView `Loaded` handler no-opped, and
`OnLayoutUpdatedImpl` only re-applied when a *presenter* was missing — never
when a *glyph* was. A slot's `SystemTray.IconView` template expands after its
ContentPresenter appears, so wifi and volume were styled against a null
TextBlock forever. Battery escaped it because `g_batteryGlyphFE` is only found
after `WalkSetupBatteryInnerPanel` succeeds, which already implies an expanded
template. Opacity still worked on all four, because `ApplyItemStyle` puts
opacity on the element, not the TextBlock — that asymmetry is the confirming
signature if it ever recurs.

Fixed by making success mean *styled*, not *found*: `AllGlyphsResolved()` gates
`ApplyAllSettings`'s return, so the bounded retry thread and the IconView
`Loaded` handler stay live until every found item has its glyph. LayoutUpdated
tops the styling up in place (no teardown), bounded by `kMaxGlyphTopUps` = 60
passes so a template that never yields a glyph cannot cost a subtree walk per
frame forever. `ResolveGlyphTB` logs each glyph's resolved class/name once, and
logs the give-up.

Also in this pass: `Placement` is now a `Status` note whose `$name` reads
"Placement: not available in this mod" and whose description says the box does
nothing (the user wants the line kept in Placement's slot at the top);
`Layout.NewItems` retitled to name the battery percentage, which is the only
item that can actually arrive late; and both README layers were rewritten from
the settings block — they were still pure v1.0, documenting `itemOrder`,
`gridMode`, coupled battery mode, and eight nudge keys that no longer exist.
The stale `batt-percent-coupled.png` caption is gone from the gallery.

Note `verify-readme-sync.ps1` only compares the two README copies to each
other, so two identically-stale copies pass. It did. Consider adding a
README-vs-settings-block check when `verify-settings-order.ps1` folds into
`submission-preflight.ps1`.

**Live log 2026-07-25 — what the OmniButton actually contains.** Fresh paste,
default settings, 48px taskbar @ 96dpi. Exactly THREE native slots, no
`Unknown slot index` lines, so the item set is settled on this hardware:

| Native slot | Class | Size | Contains |
|---|---|---|---|
| 0 | ContentPresenter | 28x48 | wifi -> TextBlock `InnerTextBlock` |
| 1 | ContentPresenter | 24x48 | volume -> TextBlock `InnerTextBlock` |
| 2 | ContentPresenter | 65x48 | battery group -> inner horizontal StackPanel |

The battery group's inner panel holds TWO children: `[0]` a **Grid** 20x16 (the
battery drawing — NOT a font glyph) and `[1]` TextBlock `BatteryTextBlock`
("100%", 29.6x16). So four mod items map to three native slots.

All four glyphs resolved on the first pass with no misses, which confirms the
`AllGlyphsResolved` plumbing but does NOT prove it fixed the color bug — a
fresh paste finds the taskbar already built. The failing path is an Explorer
restart / cold boot, where the mod loads before the templates expand. Colors
were still empty in that run, so **whether coloring works is untested.**

Battery resolved to an unnamed TextBlock found by the `FindFirstTextBlock`
fallback INSIDE that Grid. We do not know what that TextBlock is. A
`LogItemSubtree` probe now dumps the battery Grid's subtree once per apply;
the next log answers whether battery color/size/font are reachable at all or
are three dead settings.

**The percentage was clipped because it is text in a glyph-sized box.** From
the log: `pctCell=(34.0,24.0) center=(0.00,5.00)`. `centerX` is computed as
`max(0, (cellWidth - desiredWidth)/2)`, so centerX = 0 proves the percentage's
desired width met or exceeded its 32px cell (`Size.ItemWidth`). One number
cannot describe both a 20x16 battery icon and a string that is 29.6px at "100%"
and narrower at "9%". Fixed by measuring: `MeasureNaturalWidth` runs before the
arrangement resolves, `OmniItemSize()` is now the single source of cell size
for the arranger, the missing-token pass, and the per-item centering, and
`PrepareSlot`/`PrepareIndependentItem` take each item's own placement size
instead of a shared `g_settings.itemWidth`. A free `ActualWidth` check in the
LayoutUpdated monitor re-applies if the value later outgrows the reserved cell
(80% -> 100%); re-applying re-measures, so it cannot loop.

`Adjust.PadX` default was **2**, which the contract says is 0 and which nothing
in the code needed — reverted to 0. If the cluster then crowds the clock, PadX
is exactly the knob for it; that is a one-line revert.

### 2026-07-26 live test — the percent toggle wrote the wrong registry value

**VERIFIED BY READING THE LIVE REGISTRY**, not inferred:

```
HKCU\...\Explorer\Advanced
  IsBatteryPercentageEnabled = 1   <- what Settings > Power & battery reflects
  TaskbarBatteryPercent      = 1   <- what the mod wrote
```

The mod wrote a value Windows 11 does not read, for its whole life, and logged
`[Battery] TaskbarBatteryPercent set to 1` every time — a success line proving
only that it wrote *something somewhere*. That is the whole of "the percent
toggle doesn't interface with the real settings."

Fixed via the new `os-setting-bridge.h`: writes both names, snapshots both
before the first write, and on restore DELETES whichever the mod invented
rather than zeroing it. Lesson recorded in `settings-profiles.md` under
"Settings that drive a WINDOWS setting": never infer a registry name, verify
against the live registry with the OS UI open and watch which value moves.

**The ActualWidth re-measure check I added on 07-25 could never fire.** A
TextBlock arranged into a slot narrower than its content reports
`ActualWidth == the slot`, so comparing it against the reserved cell compares a
number to itself. Replaced with: a sticky `g_percentWidestDesired` that
survives the element reset a re-apply performs; a correction inside the
arrangement pass (where `PrepareIndependentItem` already computes the TRUE
desired width) that widens the cell and asks for exactly one re-apply; and text
-change detection (`Text()` is a free read, unlike Measure) for 9% -> 100%.

STILL UNCONFIRMED whether this fixes the visible clipping — the 07-26 test was
run before these changes. The log now prints
`[Layout] percentage "100%" measures X, widest seen Y -> cell Z (ItemWidth 32)`
and, if the cell was too small, `percentage really needs X but was given Y`.
Those two lines settle it; get them from the next run before theorizing further.

### Template extraction 2026-07-26 — three new reusable pieces

Both came out of the OmniButton defects above, and both are family-wide.

**NEW: `_templates/native-glyph-surface.h` v1.0.** Styling a native item the
mod does not own. Every mod had grown its own "walk down to the styleable leaf
and set Foreground/FontSize/FontFamily", and they were all wrong the same way:
they assume the leaf is a TextBlock. The OmniButton's battery is a Grid of
SHAPES — that is how it draws a fill level and a charging bolt — and a blind
"first TextBlock anywhere below" search still finds *a* TextBlock under it and
binds to it, so the settings look wired and do nothing.

So it PROBES and reports. `Probe()` returns a `Surface` with a `Kind`
(TextGlyph / Shapes / Opaque / None) and `Supports()` capabilities; the mod
offers only the settings that can apply. Precedence is deliberate: host is
itself a TextBlock -> descendant named `InnerTextBlock` -> any Shape
descendants -> any TextBlock at all, flagged as a guess. **Shapes must outrank
an unnamed TextBlock**; the reverse is the bug being replaced. Colors go to
Fill/Stroke for shapes, and only where the shape already paints that one, so
recoloring cannot add an outline or a blob that was never there. The template
owns no snapshot store — it takes the mod's `TrackFn` so the existing lease
stays the single restore path.

**`nested-group-layout.h` -> v2.5**: additive `ContentAlong(measured, minimum,
cross)` plus the doctrine. Item-size settings describe a GLYPH; they cannot
describe TEXT. Measure with `MeasureNatural`, reserve through `ContentAlong`.

Both doctrines are now in `settings-profiles.md` (Size, and "Offer only the
controls the item can honor" under Surface) and both templates are registered
in `_templates/README.md`.

**Template parity — roll these out one live-tested build at a time:**

| Mod | nested-group-layout | native-glyph-surface | os-setting-bridge |
|---|---|---|---|
| OmniButton | v2.5, verified | v1.2, verified | REMOVED — see 2026-07-26 |
| Privacy Anchor | STALE (pre-v2.5) | hand-rolled walk — candidate | — |
| VD Switcher | STALE (pre-v2.5) | n/a, owns its buttons | — |
| Tray Utility | STALE (pre-v2.5) | n/a so far | candidate (the "always show these icons" idea is exactly this) |
| Clock Spacer | no embed | hand-rolled walk — candidate | — |

### 2026-07-26 — COORDINATE-SPACE MIX in the battery group (found in the log)

`[Geometry]` across two runs shows every battery-group item landing exactly
**+4px right** of where the arrangement put it, at both PadX=0 and PadX=4:

| | arrangement | rendered right edge | delta |
|---|---|---|---|
| battery (padX 0) | 26 | 30.0 | +4 |
| percent (padX 0) | 62.6 | 66.6 | +4 |
| battery (padX 4) | 30 | 34.0 | +4 |
| percent (padX 4) | 66.6 | 70.6 | +4 |

Cause: `ReadNaturalOrigin(child, g_batteryInnerPanel, …)` measured each child's
natural origin against the INNER PANEL, while `layout.placement[].x` is in the
ITEMS HOST's space. Two different origins subtracted from each other; the gap
is the inner panel's own inset, constant at 4px on this build. Fixed by
measuring against `sp` — the same space the arrangement uses.

Measuring against `sp` also folds in the battery presenter's vertical offset
(applied earlier), because `TransformToVisual` reports where the child actually
is; the per-child offset is then simply (target - current), so nothing is
double-counted.

**This is the third distinct cause behind one symptom**, and the reason three
earlier fixes each looked right and changed nothing:
1. cell too narrow for text        -> real, fixed (content sizing)
2. zero margin at the button edge  -> real, fixed (PadX default)
3. coordinate-space mix            -> real, fixed here
Only #3 was constant across every run. The lesson stands and sharpens: when a
symptom survives a correct-looking fix, the model is wrong somewhere, and only
comparing INTENDED vs RENDERED numbers side by side exposes where.

STATUS: **VERIFIED FIXED 2026-07-26.** User screenshots show the full "100%"
rendered, in both the auto 2x2 and a written row, with no clipping. The clip is
closed. It took three real causes; only #3 was present in every run.

### 2026-07-26 — "why is the OmniButton's area so enormous?" It was ItemWidth

Reported immediately after the clip was fixed: with the items in a row, the
button is huge around them, **`Adjust.PadX` has no effect on the gaps, and it
cannot be made negative.**

Both observations were correct, and neither was a bug in the code — they were a
bad default plus two wrong clamps.

- The gap between icons is `Size.ItemWidth` (was 32) minus the glyph's real
  width (~16). That is **16px of dead space per item, contributed by the
  arrangement itself.** In a 2x2 block nobody notices; in a single row it is
  half the button.
- `Adjust.PadX` is OUTER padding. It reserves space at the two ends of the
  group and can never change the distance between two items. No value of it
  ever will. The user reaching for it first is the design's fault, not theirs.
- `Size.ItemSpacing` was clamped to 0..40, so the one setting that *could*
  pull items together refused to go negative — even though `ngl` handles a
  negative gap natively.

Fix (all three, template-first):
1. `Size.ItemWidth` gains **0 = fit each item to its own content**, and 0 is
   now the default. Same content-sizing path the percentage already used, via
   `ngs::MeasureNatural`, generalized from one special-cased item to all four.
   No slack — a glyph does not grow, and padding it re-adds the dead space.
2. `Size.ItemSpacing` clamps to **-16..40**.
3. Every one of the three `$description`s now names the right knob, so the
   wrong one points at the correct one.

Mechanics worth keeping: measurements are sticky-widest-seen (a first measure
can land before the template expands and honestly report 0), fall back to 24 so
a group is never arranged at zero width, and set `g_cellsNeedRemeasure` for ONE
bounded re-arrange (`kMaxRemeasures = 3`) when a fallback was used. Styles are
now applied BEFORE measuring — otherwise cells are reserved at the native glyph
size and then painted at the user's chosen size.

Doctrine written into `_templates/settings-profiles.md` §4 Size.

STATUS: compiled, all five gates green, **not yet live-tested.**

### 2026-07-26 — WHY WIFI AND VOLUME WOULD NOT RECOLOUR (live matrix)

Live test result, the cleanest data this mod has produced:

| item | colour | font size | opacity |
|---|---|---|---|
| battery | YES | n/a | YES |
| percent | YES | YES | YES |
| wifi | **no** | **no** | YES |
| volume | **no** (even `transparent`) | **no** | YES |

Opacity working on all four PROVES the mod reaches wifi and volume — opacity
is set on the outer ContentPresenter host. So it was never a wrong-element or
a timing problem. The dividing line is exactly:

- battery / percent — plain XAML children of the battery StackPanel, no owner
- wifi / volume — `InnerTextBlock` **inside a `SystemTray.IconView`**

Cause: `InnerTextBlock` is TEMPLATE-BOUND. Its Foreground/FontSize come from
the templated parent. **The mod was styling one level too deep**; a local write
onto a template-bound child is re-asserted by the template and vanishes.

Fix, in `native-glyph-surface.h` v1.1: `Surface` gains `anchor`, the OUTERMOST
`Control` strictly between leaf and host, found by walking UP (the parent chain
is unambiguous; a downward search would have to guess which Control is the
templated parent). `ApplyColor`/`ApplyFontSize`/`ApplyFontFamily` write the
anchor first, then the leaf — when template-bound the anchor is the only write
that survives, when not the leaf's local value wins and the anchor's is
inherited past. Both leased, so restore is unaffected either way. One code
path, no per-item special case.

Also confirmed working this run: fixed `ItemWidth`, and `ItemSpacing` negative.

STATUS: compiled, gates green, **not live-tested.** If wifi/volume still
refuse after this, the remaining candidate is VisualState animation precedence
and the honest answer becomes Placement + opacity only, with recolouring
delegated to Taskbar Styler (which rewrites the Style and therefore wins).

### 2026-07-26 — OS-SETTING BRIDGE REMOVED FROM OMNIBUTTON (user's call, right call)

The battery-percentage write is gone, and so is the embedded
`os-setting-bridge.h` block. Parity now reads 7 embedded / 4 not used.

Why, in the user's words: "the mod drove the windows setting exactly once,
then i couldnt recreate it." Everything about the write was verified correct by
then — right value name, written only on a real change, broadcast sent. The
failure was downstream: Explorer only sometimes re-reads it and the Settings
page never live-refreshes, so the control worked once and then looked dead.

**A control that works once and then does not is worse than no control.** That
is now the doctrine, and it outranks "the code is correct".

`Content.Percent` now means exactly what `Content.Wifi/Volume/Battery` mean:
include this native item in the arrangement. Four toggles, one meaning, none of
them reaching outside the taskbar. `-ladvapi32` dropped from @compilerOptions
since no registry call remains. Nothing to restore on unload because nothing is
ever written.

KEEP `_templates/os-setting-bridge.h`. It is correct, its two verified findings
(the real value name; "do not re-assert at load") are worth having, and a
future mod may need it. It is simply not used here. What the template now also
needs, and does not yet say: **verify the OS ACTS on the write, not just that
the write lands.** A correct registry value the OS ignores is a dead feature.

### 2026-07-26 — SOLVED: the ghost is LAYERED GLYPHS, three of them

The deep dump ended it:

    wifi d5[0] SystemTray.AdaptiveTextBlock name=Underlay        16x16
      d6[0]      TextBlock name=InnerTextBlock                   16x16
    wifi d5[1] SystemTray.AdaptiveTextBlock name=Base            16x16
      d6[0]      TextBlock name=InnerTextBlock                   16x16
    wifi d5[2] SystemTray.AdaptiveTextBlock name=AccentOverlay    0x16
      d6[0]      TextBlock name=InnerTextBlock                    0x16

Volume is identical. **Wifi and volume are each THREE stacked glyphs**, which
is how one icon shows signal strength or a mute slash. The battery is the same
shape with two. `FindNamedTextBlock` returns the FIRST match (Underlay's) and
the Surface then claimed `fontSize=1 fontFamily=1` — a false capability the
template had been reporting for every layered item.

Resize one layer of a stack and it stops coinciding with the others. That is
the ghost, exactly: a larger glyph over the original, not a bigger icon.

Fix in `native-glyph-surface.h` v1.2: `Surface::glyphLayers`, counted by
`CountTextBlocks`, and `Supports()` gates fontSize/fontFamily on
`glyphLayers <= 1`. Colour stays available for a stack because it is written to
the ANCHOR, which every layer inherits from, so they move together.

Consequences in the mod:
- `Surface.WifiSize`, `VolumeSize`, `WifiFontFamily`, `VolumeFontFamily` are
  GONE. Only the percentage keeps size and family — it is the only item that
  is genuinely one piece of text.
- The mod's hard-coded `ApplyItemStyle(g_batterySurface, ..., 0, nullptr, ...)`
  special case is gone too. The template decides now, generically, by counting.

THE LESSON, and it is the same one three times over now: **the probe reported a
capability it had not verified.** "Found a TextBlock" was treated as "this item
is one TextBlock". Before a Surface advertises a control, it has to check the
thing that would make that control wrong — siblings, in this case.

STATUS: compiled, all five gates green, **not live-tested.**

### 2026-07-26 — my diagnostic was truncated; font size DOES work

`LogItemSubtree` had a hardcoded `depth > 4`, tuned for the battery's shallow
tree. A SystemTray.IconView nests far deeper:

    IconView > ContainerGrid > ContentGrid > TextIconContent > ContainerGrid > ...

so the dump stopped before reaching any TextBlock and answered nothing about
the ghost. maxDepth is now a parameter; wifi/volume are dumped at 12.

The same log DID settle one thing, by controlled comparison in a single run:

    [Layout] fit to content: wifi=36 volume=24 battery=20

wifi was 28 before `WifiSize` was set, volume was left unset and did not move.
**The style anchor drives FontSize correctly and the arrangement reserves the
larger cell.** The ghost is a rendering artifact downstream of a write that
demonstrably worked — not a failed write. Also confirmed: anchor is
`SystemTray.IconView` for both.

STATUS: the deeper dump is unread. Do not theorise about the ghost until it is.

### 2026-07-26 — SOLVED: battery percentage vs Windows Settings

Four live registry reads settled it completely:

| step | IsBatteryPercentageEnabled | TaskbarBatteryPercent |
|---|---|---|
| mod loaded, Percent:1 | 1 | 1 |
| mod disabled | 1 | 1 |
| **toggle turned OFF in Settings** | **0** | 1 (untouched) |
| **mod re-enabled** | **1** (stomped) | 1 |

Two independent conclusions, both now acted on:

1. **`IsBatteryPercentageEnabled` is the right value, and it is the ONLY one.**
   Settings changed exactly that and never touched `TaskbarBatteryPercent`.
   That second name has been REMOVED from the bridge. It was there on the
   strength of "some builds are reported to consult it" — hearsay, which the
   bridge's own rule forbids. Writing a registry value nobody can demonstrate a
   use for modifies a user's machine for no reason.

2. **The mod was overriding the user's OS choice at every load.** Step 4 is the
   proof: the mod's own toggle never moved, and re-enabling it put the setting
   straight back. `Wh_ModInit` called `ApplyBatteryPercent` unconditionally.

Fix: **nothing is written at load.** `Wh_ModSettingsChanged` captures
`g_settings.percent` BEFORE `LoadSettings()` and writes only when that toggle
actually moved. Restore falls out for free — `captured` only goes true once
something was written, so a session that never wrote has nothing to put back.

Why this is coherent rather than a compromise: whether the percentage is
arranged already keys off `hasPercent` — whether the native element EXISTS —
not off the mod's toggle. So the mod arranges what Windows *shows*, and the two
can never contradict each other on screen regardless of what the toggle says.

Also learned: **the Settings page does not live-refresh.** It showed OFF while
the registry read 1. That is Windows, not us; now documented in both READMEs.

Doctrine written into `os-setting-bridge.h` v1.1 as "DO NOT RE-ASSERT AT LOAD",
with the four-read evidence inline. Only OmniButton embeds this template, so
there is nothing else in the family to sync.

STATUS: compiled, all five gates green, **not live-tested.**

### 2026-07-26 — style anchor CONFIRMED for colour; font size GHOSTS

Live result after `native-glyph-surface.h` v1.1:

- **Colour now works on all four items.** The template-bound-parent theory was
  correct, and styling `InnerTextBlock` directly really was one level too deep.
- **Font size on wifi/volume produces a GHOST**: a larger glyph drawn over the
  original, both visible at once.

The ghost means the IconView draws MORE than the single `InnerTextBlock` the
probe binds to, and the anchor's `FontSize` is reaching the other thing too —
or the enlarged glyph escapes the fixed-size slot without the original being
replaced. Either way the mod is now scaling something it did not identify.

Note the asymmetry this exposes: `Foreground` on the anchor is exactly what the
glyph is template-bound to, so it lands on precisely one thing. `FontSize` on a
`Control` propagates to EVERY text descendant, which is a wider blast radius
than intended. Colour and size are not equally safe to set on an anchor, and
the template currently treats them as if they were.

NEXT: diagnostic only, no theory. Added `LogItemSubtree` dumps for wifi and
volume (battery already had one) and a log line naming the anchor's class. One
reload tells us how many TextBlocks live under an IconView and what the anchor
actually is.

Likely outcomes, in order of my confidence:
1. Two glyph layers per IconView (main + badge/overlay) -> same shape as the
   battery's layered pair, and glyph size should be DROPPED for wifi/volume
   exactly as it was for the battery, for the same documented reason.
2. Anchor is too high up the chain and should be the nearest Control, not the
   outermost.
3. The slot's fixed Width/Height stops the layout growing, so the bigger glyph
   overflows rather than replacing.

Do not "fix" this before the dump says which.

### 2026-07-26 — battery composition text corrected

The settings block and both READMEs said the battery "is drawn from *shapes*".
The live probe disproved that: it is **two unnamed TextBlocks, both 20x16 — a
layered outline+fill glyph pair.** Dropping BatterySize/BatteryFontFamily is
still right, but for a different reason: resizing one of a stacked pair pulls
the two apart. Color has the same hazard and is now documented as best-effort
that may reach only one layer. All three places corrected.

### SOLVED 2026-07-26 — the percentage clip was PadX, and I caused it

`[Geometry]` settled it in one run:

```
button=67.0  sp=67.0  battPresenter=67.0  innerPanel=67.0  percent=29.6
right edge in button space:  sp=67.0  battery=30.0  percent=66.6
```

**Content ends at 66.6 in a button 67.0 wide — 0.4px of margin.** Nothing
overflows, which is exactly why every overflow check correctly stayed quiet
through three rounds of investigation. The content is flush against the
button's edge, and the OmniButton has ROUNDED CORNERS, so the curve (plus
antialiasing) shaves the last glyph of the bottom-right item — the "%".

**Root cause is mine.** The user asked "why is there a horizontal padding of
+2 by default?"; I checked the contract, saw the canonical default is 0,
"corrected" it, and removed the very thing that was holding the content off the
edge. The honest answer to the question was: because this mod zeroes the
button's native padding (`control.Padding(0)` — "Adjust.PadX is the sole
padding owner"), so the group has to provide its own.

Fixed: `PadX` defaults to **4** (2 was never quite enough — the 07-25 log shows
it clipping at padX=2 as well). At 4 the group is 75 wide and the percentage
ends at 66.6, leaving ~8px of clearance. The setting `$description` and both
READMEs now say WHY, so it does not read as an arbitrary number to the next
person who audits it against the contract.

Doctrine added to `settings-profiles.md` under Adjust: **a mod that zeroes its
host's native padding MUST supply its own**, with this as the worked example,
and an explicit "do not 'correct' such a default back to 0 on the grounds that
the contract says 0". A deviation from a canonical default belongs in the
setting's own `$description` where the next reader meets it before changing it.

LESSON: three rounds went into the arrangement arithmetic because the symptom
looked like an overflow. It was never an overflow — it was zero margin. When
every bounds check says "fits" and the user still sees a clip, measure the
RENDERED edges against the HOST's edges, not the model against itself.

### LIVE LOG 2026-07-26 (post-crash-fix) — STABLE, and the clip is NOT the math

Enable/disable is now stable; no crash. The percentage width arithmetic is
**provably correct** in this run and the clip is still visible, so the cause is
somewhere the arrangement model does not describe:

```
percent TextBlock natural  = 29.6 wide ("100%")
early measure              = 33.0  -> cell 35   (ceil+2)
late measure (centerX=1)   = 33.0  -> agrees, correction did NOT fire (right)
arrangement total          = 67    (col A 32 + col B 35)
pctCell                    = x 32, width 35; content at 33, ends at 66 < 67
widening path              = did NOT fire, 65 <= 67 (right)
```

Every number checks out, nothing overflows the group by calculation, and the
belt-and-braces widening correctly declined to fire. **So the clip happens
above the items host**, and the user's other observation is the lead: hovering
shows EMPTY SPACE inside the highlighted button area while the percentage is
cut on the right. Empty space + right-edge clip means the button's rendered
box and the arranged content are not aligned the way the model assumes.

Added `[Geometry]` — after a final `UpdateLayout`, it logs the real
`ActualWidth` of button / items host / battery presenter / inner panel /
percentage, plus each one's RIGHT EDGE transformed into the BUTTON's own
coordinate space. That names the mismatch instead of costing a fourth theory.
Three width theories have now failed; measure, do not reason.

**Battery is NOT shape-drawn — correction to the 07-26 finding.** The probe
shows the battery Grid contains TWO unnamed TextBlocks, both 20x16 (the classic
layered outline+fill glyph pair), so `Probe` returns TextGlyph via the unnamed
fallback and reports color/fontSize/fontFamily all true. Dropping BatterySize
and BatteryFontFamily is still the right call, but for a BETTER reason than the
one recorded earlier: it is a PAIR of layered glyphs and the mod styles only
the first, so resizing or re-fonting one would misalign it against the other.
Battery COLOR has the same hazard — recolouring one layer of two. Watch for it
when the battery-color question comes back up.

### CRASH 2026-07-26 — the template refactor took Explorer down. FIXED.

**My bug, introduced in the taskbar-host extraction.** `RunFromWindowThread`'s
CALLWNDPROC hook proc did this:

```cpp
auto* dispatch = reinterpret_cast<Dispatch*>(call->lParam);   // cast FIRST
if (dispatch && call->message == RegisterWindowMessageW(dispatch->messageName))
```

A `WH_CALLWNDPROC` hook sees EVERY message sent to EVERY window on the
taskbar's UI thread. `lParam` for all of those is arbitrary — an integer, a
flag, a pointer to something else. This cast it to `Dispatch*` and then
DEREFERENCED it (`dispatch->messageName`) to read a `PCWSTR`, and handed that
to `RegisterWindowMessageW`, **before** establishing that the message was ours.
`dispatch &&` only checks non-null; it says nothing about validity. Access
violation, Explorer down.

The pre-refactor code checked `call->message == dispatchMessage` FIRST, against
a `static UINT` built from a literal, and only then touched lParam. Making the
message name a parameter is what tempted the reordering — the lambda is
captureless and could not see the local. Fixed with a namespace-scope
`g_dispatchMessage` set before the hook is installed: message compared first,
lParam touched only inside that branch. The rule is now written into the
template in capitals.

**Why the layout still applied in the user's screenshot:** the UI-thread paths
(`Wh_ModInit` when already on that thread, and the IconView `Loaded` handler)
take the `threadId == GetCurrentThreadId()` fast path and never install the
hook. Only the RETRY THREAD marshals, and that is what crashed. So the
arrangement was applied by the direct path, then Explorer died on the first
retry-thread dispatch.

LESSON, for every future hook proc: a system-wide hook is a hostile input
surface. Validate the discriminator before touching anything else the callback
hands you. Written into `taskbar-xaml-lifecycle.template.cpp` as the
SYSTEM-HOOK CONTRACT, next to the process-shutdown contract.

**Family-wide audit, 2026-07-26 — everything else is CLEAN.** Every
`SetWindowsHookEx` in the lab was checked for the same cast-before-compare
hazard:

| Mod | Verdict |
|---|---|
| Privacy Anchor | SAFE — message compared first |
| Tray Utility | SAFE |
| Clock Spacer | SAFE (`(Param*)` style) |
| Folder Menus | SAFE |
| VD Switcher | SAFE |
| `taskbar-xaml-lifecycle.template.cpp` | SAFE |
| OmniButton | was the ONLY hazard, and only because the refactor introduced it |

So this was not a latent family problem — it was one reordering in one
extraction. The rest of the guts the user described as "stable and
referenceable" are exactly that, and the diff against `HEAD` confirms it:
`Wh_ModUninit`, `ApplyOnTaskbarWindowThread`, and the rebuild path are
structurally identical to the stable version, and every function that
disappeared since then is one the templates deliberately replaced
(glyph/color/settings helpers, and the old smart-grid layout functions retired
in the 2.0 settings rework). Nothing was dropped by accident.

Also hardened: `g_dispatchMessage` is now `std::atomic<UINT>` with
acquire/release. The pre-template code got single-initialisation for free from
a function-local `static UINT` magic static, which a parameterised template
cannot use; the atomic restores the same guarantee across the retry thread and
the UI thread.

### TASKBAR POSITION — investigated 2026-07-26, VERIFIED against the catalog

The user asked what happens if the taskbar is moved. Windows 11 only supports
the bottom, but the catalog has TWO m417z mods that move it, so both are real
compatibility targets — checked in the actual upstream `mods/` tree, not assumed:

- **`taskbar-on-top.wh.cpp`** (m417z, v1.1.7) — bottom to top.
  **SUPPORTED, no work needed.** Everything in this family positions relative
  to the taskbar's own XAML tree, never to screen coordinates. Still worth a
  live test, but there is nothing to special-case.
- **`taskbar-vertical.wh.cpp`** (m417z, v1.3.13, funded by AuthLite) — left or
  right. **NOT COMPATIBLE, BY CONSTRUCTION.** Read its source: at line ~2169 it
  walks the IDENTICAL path this family walks —
  `ControlCenterButton > Grid > ContentPresenter > ItemsPresenter > StackPanel`
  — and applies `RotateTransform` to `RenderTransform` on those children.
  OmniButton positions by writing a `TranslateTransform` to `RenderTransform`
  on the same elements. One dependency property, two owners, last writer wins.
  No amount of cooperation fixes that. Its own readme already documents the
  same class of conflict for `taskbar-multirow`.

**Resolution: detect and stand down, do not attempt to support rotation.**
`taskbar_host::GetMetrics` now returns orientation, DPI, and the CONSTRAINED
extent in DIPs (the taskbar's thickness whichever way it runs);
`LayoutModelApplies` is false for a vertical taskbar. Detection is the
taskbar's own rect aspect — never sniffing for a specific mod, because the
aspect is the condition that actually matters and holds however it got that
way. OmniButton checks it before touching anything, leaves the taskbar exactly
as found, logs once, and returns "applied" so the retry loop retires. An
Explorer rebuild re-evaluates if the user disables the vertical mod.

Bonus: `AvailableOmniRows` used `rect.bottom - rect.top` as the height, which
on a vertical taskbar is the SCREEN height — it would have computed ~45 rows.
That is now `metrics.constrainedDip`, and the px/DIP conversion PR #4855
flagged lives in the template rather than in each mod.

Documented in `settings-profiles.md` ("Taskbar position, and living with the
rest of the ecosystem") and in both OmniButton READMEs, m417z-style: name the
mod, say why, say what happens.

**Only OmniButton has this so far.** Every other mod in the family still has
the old unguarded row math and will misbehave on a vertical taskbar — roll the
check out with each mod's live-tested build.

### UNIFORMITY ROLLOUT — items 1-5 DONE 2026-07-26

The user asked for all five. Four new templates, one adoption, one new gate.
OmniButton now embeds EIGHT templates, all verified verbatim.

| # | Template | What moved out of the mod |
|---|---|---|
| 1 | **`property-lease.h`** (new) | `TrackProperty` / `RestorePropertySnapshots` / snapshot struct. Documents the two details that look like style and are not: FIRST WRITE WINS (a later snapshot would capture the mod's own value) and RESTORE IN REVERSE (later mutations depend on earlier ones). Held as `optional<Lease>` + no_destroy, reset on the UI thread. |
| 2 | **`taskbar-host.h`** (new) | `FindCurrentProcessTaskbarWnd`, `RunFromWindowThread`, `GetTaskbarXamlRoot` + the runtime-disassembled offset, the five taskbar.dll symbol hooks, and a stoppable bounded `RetryLoop`. The mod keeps only its rebuild callback. |
| 3 | **`settings-io.h`** (new) | Load/clamp helpers, an RAII string holder, and TABLE-DRIVEN `$options` matching — the `_wcsicmp` chain it replaces is what silently broke Indicator symbols after an option rename. |
| 4 | **`color-tokens.h`** (new) | The one token parser. Three copies existed (button-surface, OmniButton, Privacy Anchor) agreeing by luck. |
| 5 | `visual-tree-walk.h` (existing) | `FindChildRecursive` now delegates to `vtw::FindDescendant`. |

**NEW GATE: `verify-template-parity.ps1 <mod>`.** Compares every embedded
template namespace body against its source; skips templates a mod does not use,
so only real drift fails. It found drift on its first run that was not on
anyone's list:

```
omnibutton-customizer     TEMPLATE_PARITY_OK (8 embedded, 3 not used)
privacy-indicator-anchor  DRIFT: nested-group-layout.h, start-placement.h
taskbar-vd-switcher       DRIFT: nested-group-layout.h
tray-utility-customizer   DRIFT: nested-group-layout.h
taskbar-clock-spacer      OK (embeds none)
```

`start-placement.h` drift in Privacy Anchor was previously UNKNOWN — the
nested-group-layout drift was expected (pre-v2.5), that one was not. Investigate
when Privacy Anchor comes up for its next live-tested build; do not re-embed
into a mod that is not being tested.

Fold this gate into `submission-preflight.ps1` alongside
`verify-settings-order.ps1` once the family is uniform.

### THE UNIFORMITY GAP — user, 2026-07-26

The user's standing position, in their words: the mods "all do something
different and nuanced, but they all have similar guts, and a need for similar
settings". Hand-rolled code is acceptable ONLY where it expresses a mod's
genuine nuance — OmniButton cannot be repositioned, Tray Utility commandeers an
existing space, VD Switcher creates its own buttons. Everything else should be
the same template in every mod. "I feel like you get it, but we just don't HAVE
it yet."

What is templated now: layout (`nested-group-layout.h`), native-item styling
(`native-glyph-surface.h`), OS settings (`os-setting-bridge.h`), owned button
surfaces (`button-surface.h`), tree walking (`visual-tree-walk.h`), lifecycle
(`taskbar-xaml-lifecycle.template.cpp`), placement
(`injected-grid-column.h` / `start-placement.h`).

Items 1-5 are DONE for OmniButton — see the rollout section above. What remains
is DISTRIBUTION, not extraction: every other mod still hand-rolls all five, and
each may only adopt them as part of its own live-tested build.

Remaining genuinely-unextracted pieces, for later:

- **`button-surface.h` still carries its own color parser.** It should delegate
  to `color-tokens.h`, but it is embedded in VD Switcher and others, so the
  change waits for a mod that is being tested.
- **The IconView-Loaded / LoadLibraryExW injection kick.** Shared in shape
  across the tray mods but genuinely varies in which module it hooks; may be
  nuance rather than duplication. Decide when a second mod needs it.
- **`smart-grid-layout.h`** is still embedded in Privacy Anchor and Folder
  Menus and is superseded by `nested-group-layout.h`. Delete when the last
  adopter migrates.

"STALE" is only the additive `ContentAlong` block; nothing existing changed, so
no behavior moved under those mods. Re-embed as each one comes up for its next
live-tested build — never re-embed into a mod that is not being tested. Verify
with the parity check: match `namespace windhawk_mod_templates::<ns> { ... }`
in template and mod and compare the bodies.

Per user 2026-07-26: display-scaling testing is still owed on OmniButton once
the underlying issues are resolved.

Open question for the next live test: `[Layout] Unknown slot index N: <class>`
in the log answers whether anything besides wifi/volume/battery can appear in
the items host. Wifi and volume are identified purely positionally (slot 0 and
slot 1); battery alone is found by class search. If Windows ever inserts an
item ahead of them, both are silently mislabeled. Not changed — the two slots
are indistinguishable by class, so identifying them properly needs a UWPSpy
look at what actually distinguishes them.

Current local gates: `SETTINGS_ORDER_OK`, `COMPILE_OK`,
`EXIT_TIME_DESTRUCTOR_AUDIT_OK`, `README_MATCH`,
`NESTED_TEMPLATE_BODY_MATCH`, and `git diff --check` is clean. Screenshots,
preflight, commit, and PR update are intentionally deferred until the user
live-tests this exact build.

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
Customizer (PR #4841) are now in the official catalog. Both remain queued for
their full 2.0 settings/template upgrade after the current open-PR rollout.

**Active work: cross-cutting `[[clang::no_destroy]]` resolution.** The
2026-07-23 review wave requests the same no_destroy fix on every remaining open
mod (#4843, #4844, #4855) — it was also the merge-gate on the two that merged.
Curate the definitive resolution into the lifecycle template, then distribute to
each affected mod one live-tested build at a time. VD Switcher already has the
fix (in the unified-placement rebuild, `7c1e4b8`, pending its live test).

VD Switcher rebuild on unified nested-group placement is committed at `7c1e4b8`
(compiles + audit-green, preliminary user test "looking OK"); READMEs + version
bump are deferred until the full live test. See work_log 2026-07-23.

### Fork CI — inherited upstream workflow (2026-07-25)

`sb4ssman/windhawk-mods` has been failing a nightly **Deploy static content**
run since the 07-23 fork-`main` reset. It is upstream's own `.github/deploy.yml`
(`schedule: '2 0 * * *'` + `push: main`), inherited by the fork; the reset
force-push counted as a push to `main` and woke the schedule up with it. It
fails at "Clone last deployed content" with exit 128 because that step clones
`--branch pages` from `github.repository` — the fork, which has no `pages`
branch. Harmless: fork-scoped ephemeral token, never touches upstream, and PR
checks are unaffected (`pr_validation.yml` / `mod_compatibility_check.yml` are
`pull_request`-only and run in the base repo).

Fix is `gh workflow disable "Deploy static content" --repo sb4ssman/windhawk-mods`
— a repo-side setting, **no commit**. Do NOT delete the workflow file: fork
`main` must stay hash-identical to `upstream/main`, and a deletion commit
re-plants the landmine the 07-23 reset cleared. Left for the user to run; the
agent's attempt was blocked by the permission classifier.

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
- OmniButton Customizer v1.0 — PR #4855: OPEN. All three required items are
  DONE in the local v2.0 candidate and verified 2026-07-25: (1) the BLOCKING
  DPI bug — `AvailableOmniRows` now converts px→DIP via `GetDpiForWindow` +
  `ngl::PixelsToDip` and reserves `2 × padY` before `ngl::RowsInHeight`;
  (2) hook array renamed `systemTrayModuleHooks` with all three modules named
  in the comment above it; (3) no_destroy across the XAML globals with the
  UI-thread `reset()` in `Wh_ModUninit`. Still owed before any PR update: the
  live test, fresh screenshots, preflight. Optional review items not taken:
  retry-thread race, IsWindow revalidation, dup LoadColorSetting,
  unused params/includes.
  (Root README said "PR #3859" — that PR is CLOSED, the old
  `sb4ssman-vertical-omnibutton` branch. Corrected 2026-07-25.)

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
### VERTICAL TASKBAR — SUPPORT IT, do not just stand down (user, 2026-08-04)

Every mod that has been through the 2.0 rollout now DETECTS a vertical taskbar
and stands down (`tbh::LayoutModelApplies`). That was the right first move —
it stops the family painting garbage — but standing down is not the goal. **The
goal is for the family to work on a vertical taskbar.**

What already exists to build on:

- `tbh::GetMetrics` already reports `orientation`, `constrainedDip` (the
  taskbar's thickness whichever way it runs) and `alongDip` (the extent it can
  run along). The vocabulary is deliberately axis-neutral already — nothing
  says "height".
- `ngl::RowsInHeight` is likewise named for the horizontal case but computes
  "how many items fit across the constrained axis", which is the same question
  on a vertical bar.
- `ngl::AlongAxis` already exists for items that must size themselves against
  whichever way their group runs.

What actually blocks it, and it is ONE thing:
`taskbar-vertical.wh.cpp` (m417z, funded by AuthLite) writes a
`RotateTransform` to `RenderTransform` on the very same tray children this
family writes a `TranslateTransform` to. One dependency property, two owners,
last writer wins. Verified by reading its source at ~line 2169, not assumed.

So supporting a vertical taskbar is NOT a layout-math problem — the arranger is
already axis-neutral enough. It is an OWNERSHIP problem, and there are only
three honest ways out:

1. **Compose instead of compete.** Stop writing `RenderTransform` directly and
   write a `TransformGroup`, or place with `Margin` rather than a transform,
   so the rotation and the translation can coexist. Margin-based placement is
   already what `start-placement.h` and the tray-utility host steering use, so
   part of the family is halfway there. This is the most promising route and
   should be investigated FIRST.
2. **Detect the rotation and arrange in rotated space** — read the ancestor's
   RotateTransform, swap the axes in the arrangement, and let the rotation
   carry the group. Cheaper than it sounds because the arranger takes pixel
   sizes and returns pixel placements; only the resolver and the final apply
   need to know.
3. **Coordinate with m417z.** Discussion #4542 already has his blessing for a
   coordination convention (see below). A shared "who owns RenderTransform on
   this element" answer would settle it for every mod, not just ours.

Do NOT start by writing rotation math. Start by finding out whether
`RenderTransform` can be shared at all — that single answer picks the route.

### DISCUSSION #4542 — read in full 2026-08-04, and there IS an unacted offer

https://github.com/ramensoftware/windhawk-mods/discussions/4542 — "Guidance
requested: interoperable placement for taskbar mods", opened by the user
2026-06-24. Five top-level comments; all read.

**m417z answered the governance question on day one** (2026-06-24): *"What's
the downside of such coordination? If it's just between your mods, I see no
problem with it."* So a placement-coordination convention across this family is
MAINTAINER-APPROVED and has been for six weeks. `_templates/placement-contract.md`
is not blocked on permission; it is only blocked on us.

**UNACTED: diegoalejo15 offered working code for Taskbar Folder Menus**
(2026-07-27), attached as a full `.wh.cpp`:
https://github.com/user-attachments/files/30435360/taskbar-folder-menus.wh.cpp

Three features in it, none of which Folder Menus has:

| Feature | Note |
|---|---|
| Position: **after taskbar icons** | Answers their earlier request to escape the systray and sit among the taskbar items |
| Switch to use the **folder's own default icon** instead of label text/emoji | DylanCole954 tried it and called this the standout |
| **Per-folder position** — each folder can sit somewhere different | Currently all folders share one position |

The user replied on 2026-07-24 "the next version already has more placement
options, just stay tuned" — that promise is outstanding, and Folder Menus has
had no work since it merged at v0.7.

**OPEN BUG REPORT against that build, unresolved between the two of them**
(DylanCole954, 2026-07-28/29): when the taskbar fills up, the folder buttons
run into the system-tray area and get hidden behind a painted tray.
Reproduced on a SINGLE-row taskbar too, so it is not multirow-specific.
diegoalejo15 could not reproduce it. Screenshots are in the thread. This is
worth understanding before adopting the "after taskbar icons" position, since
that position is where the overflow happens.

Also outstanding from that thread: DylanCole954 asked for a folder button that
runs an executable, and for placement BEFORE the taskbar buttons rather than
after.

NEXT STEP: review the attached file properly against the current Folder Menus
source, decide which of the three features to take, credit diegoalejo15 in the
changelog, and reproduce the overflow bug before shipping the position that
triggers it. Folder Menus is already queued for its 2.0 rework, so this folds
into that rather than being a separate pass.

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
