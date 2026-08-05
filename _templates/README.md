# Windhawk Mod Templates

This folder is the copy-source library for sb4ssman's taskbar mods. Windhawk
submissions are single-file mods, so these files are not a shared runtime
dependency. Copy a complete block into a mod, keep the template attribution
comment, and adapt only through the documented settings or callback contract.

## Template set

| Template | Use it for |
|---|---|
| `settings-profiles.md` | **THE SETTINGS CONTRACT.** Component library of nested settings groups — fixed keys, labels, defaults — plus the fixed assembly order every mod follows. Assemble from it; never invent, rename, or reorder a key |
| `nested-group-layout.h` | **THE element-placement primitive.** One expression (`\|` horizontal, `,` vertical, parens nest), inline offsets on items and groups (`1[+2,-1]`, `(1, 2)[3,0]`), located parse errors, axis-relative sizing (`AlongAxis`) for items that must match their neighbours, content sizing (`ContentAlong`) for items that are text rather than glyphs, `MissingTokens`/`AppendMissing` so a newly created item is never silently unreachable, symmetric padding, the deterministic `auto` shape, DPI-correct `AvailableRows`, and `ResolveArrangement` behind the single `Layout.Arrangement` setting |
| `smart-grid-layout.h` | **SUPERSEDED** by the above. Kept only while OmniButton, Privacy Anchor, and Folder Menus still embed it; do not copy into anything new |
| `property-lease.h` | **THE most safety-critical component.** Snapshot the exact prior LOCAL value of every dependency property the mod mutates, and put it back — first-write-wins, restored in reverse. This is what makes disabling a mod give the taskbar back. Never guess a native default |
| `taskbar-host.h` | The price of admission: `Shell_TrayWnd` discovery, UI-thread marshalling, the `CTaskBand` → `XamlRoot` walk with its runtime-disassembled offset, the `taskbar.dll` symbol hooks, and the bounded stoppable `RetryLoop` |
| `settings-io.h` | Settings-block readers: RAII string holder, clamped ints, and table-driven `$options` matching (a `_wcsicmp` chain fails silently after any option rename). Encodes the three Windhawk API gotchas |
| `color-tokens.h` | **THE** color token parser — hex, `accent*` shades, `transparent`, and empty-means-native — in one place, with `Parse` (Color) and `ParseBrush` (Brush) shapes |
| `visual-tree-walk.h` | Descendant walk/find/collect helpers plus the OmniButton inner-StackPanel walk |
| `button-surface.h` | Surfaces **the mod owns**: hex/accent colors, native-default clearing, hover/pressed resources, border, opacity, and shine |
| `native-glyph-surface.h` | Native items **the mod borrows**. Probes what an item is actually made of — text glyph, shapes, or neither — and reports `Supports()` so a mod offers only the settings that can apply. Shapes outrank an unnamed `TextBlock`, which is what stopped the OmniButton's shape-drawn battery from binding to a mystery text element. Also `MeasureNatural`, for content-sized items |
| `os-setting-bridge.h` | A mod setting that drives a **Windows** setting. Snapshot / write-all-aliases / restore-exactly (deleting a value the mod invented). Carries the VERIFIED registry names — the taskbar battery percentage is `IsBatteryPercentageEnabled`, **not** the same-key `TaskbarBatteryPercent` the OmniButton wrote for months while its log reported success |
| `injected-grid-column.h` | Reversible `SystemTrayFrameGrid` column insertion with marker-based cleanup |
| `start-placement.h` | v1.3 — experimental owned-group placement left of, right of, or **over** Start. Left/Right reserve a lane (task-item reservation + Start counter-shift, both reversible); Over reserves nothing and is nudged clear by the adopter's vertical offset, which needs a double-height taskbar to be useful |
| `taskbar-xaml-lifecycle.template.cpp` | Taskbar-thread dispatch, guarded XAML-root access, explicit no-destroy ownership, and race-safe `TrayUI::StartTaskbar` retry lifecycle |
| `placement-contract.md` | Future cross-mod placement vocabulary, ownership rules, and placement lease design |
| `six-mod-settings-audit.md` | Evidence matrix for the six active visual mods |
| `submission-checklist.md` | Documentation parity, screenshot coverage, version, compile, and live-test gate |
| `submission-preflight.ps1` | Automated compile, README/gallery, diff, symbol-hook, and upstream-validator gate |
| `exit-time-destructor-audit.ps1` | Clang gate for unsafe namespace-scope destructors in injected mods |
| `verify-readme-sync.ps1` | Normalized folder README versus embedded Windhawk README parity check |
| `verify-settings-order.ps1` | Machine-checks a mod's settings block against `settings-profiles.md`: the eight groups, their order, canonical key names and order, mod-specific keys only after canonical ones, and retired keys staying dead |
| `verify-template-parity.ps1` | Mods are single-file, so every shared template is embedded as a verbatim copy — and copies drift. Compares each embedded namespace body against its source and fails on any difference. A template a mod does not use is skipped, so only real drift fails |

`verify-settings-order.ps1` is deliberately **not** in `submission-preflight.ps1`
yet — only VD Switcher has migrated, so wiring it in would block review fixes on
the four mods that have not. Run it by hand as each mod migrates, and fold it
into preflight once the family is uniform.

`tests/nested-group-layout-tests.cpp` covers the diamond arrangement, spacing,
absent-token collapse, nesting, all three justifications, symmetric padding,
per-item offsets parsed from the expression, five parse failures, DPI-correct
`AvailableRows`, the `ChooseShape` rule, both fill orders, ragged grids, a
custom namer, a generated expression round-tripping through the arranger, and
`ResolveArrangement`'s auto detection. Build and run with Windhawk's clang:

```powershell
& "C:\Program Files\Windhawk\Compiler\bin\clang++.exe" -std=c++23 `
  -target x86_64-w64-mingw32 -static -Wall -o "$env:TEMP\ngl-tests.exe" `
  _templates\tests\nested-group-layout-tests.cpp; & "$env:TEMP\ngl-tests.exe"
```

`-static` is required — without it the test exe looks for the runtime under
Windhawk's `.whl` names and cannot start. `tests/smart-grid-layout-tests.cpp`
belongs to the superseded header and stays until the last mod migrates off it.

### Unified element placement (v2.0)

There is one arranger and, in the settings, **one field**: `Layout.Arrangement`,
whose default value is the word `auto`.

* `auto` — `ChooseShape` picks rows × columns from the item count and the
  DPI-correct height budget, and `BuildGridExpression` turns that into the
  expression. The mod logs it.
* anything else — that string IS the layout.

`ResolveArrangement` implements exactly that, so both paths end at the same
`Compute` call and there is no mode toggle to reason about. The Windhawk
settings API is read-only, so a mod can never write the generated expression
into the field for the user; logging it is the supported way to hand it over
for editing.

Per-item offsets ride **in the expression** as `1[+2,-1]` — one string, nothing
to keep in sync. They are cosmetic: a leaf moves inside its slot, never a
neighbor, never the group size. Outer padding is symmetric (`padX`, `padY`) and
participates in layout; group offset is a visual translation that must not
reserve space, so the mod applies it to the container and the arranger never
sees it. Do NOT reintroduce a second placement engine, a second offset syntax,
or a four-sided anything.

## The six audited mods

1. Taskbar Virtual Desktop Switcher
2. Taskbar Folder Menus
3. Privacy Indicator Anchor
4. OmniButton Customizer
5. Tray Utility Customizer
6. Taskbar Clock Customization Spacer

System Tray Grid Lines is still a concept, so it should consume these templates
when implemented. Taskmanager Tail is lifecycle-only and does not belong to the
visual settings family.

## Adoption rules

1. Pick profiles by capability. Do not add button settings to a mod that does
   not own a button surface.
2. Published Windhawk setting keys are persistent API. Keep their keys stable;
   align `$name`, descriptions, internal structs, and future unpublished mods.
3. Copy whole algorithm blocks. Partial copies are how grid and cleanup logic
   drifted across the current mods.
4. Keep mod-specific behavior outside the template namespace or adapter layer.
5. Preserve native XAML values when a custom setting is empty or disabled.
6. Every insertion must have a marker or named root and a symmetric removal
   path that restores shifted columns and spans.
7. Test defaults, settings reload, Explorer restart, unload, and hit testing
   before staging a template adoption to a PR.
8. Complete `submission-checklist.md`; compilation is not a substitute for the
   three documentation layers, current screenshots, or focused live testing.
9. Namespace-scope ownership follows lifecycle v1.3.1's rule: heap-only
   settings/leases/containers destruct normally (never annotated — that only
   leaks the buffer on unload); direct nullable XAML/WinRT handles use
   intentional `no_destroy`; containers of STRONG XAML/WinRT refs use
   `no_destroy optional<container>`, with event revocation followed by `reset()`
   on controlled UI-thread unload, retaining engaged state when no UI thread is
   reachable at teardown. `no_destroy` is NOT a tool to dodge off-thread release:
   `winrt::weak_ref` (and plain heap) release safely from any thread, so those
   containers stay unannotated (rule 1).
10. Treat every taskbar UI callback as an exception boundary. Keep the v1.2
    `RunFromWindowThread` dispatcher verbatim, catch WinRT/C++ exceptions in
    native hooks and XAML event/property callbacks, and verify a live XAML root
    before removing or reapplying UI during startup and settings changes.
11. Start-adjacent placement is a separate extension from tray-column
    injection. Copy `start-placement.h` as a complete block, use it only for an
    owned group, and release its lease before destroying the group.

## Update workflow

When a mod exposes a reusable fix:

1. Prove it in the mod that discovered the issue.
2. Update the relevant template and its version note.
3. Compare every adopter against the changed contract.
4. Roll it out one mod at a time; never bulk-replace published setting keys.

The templates describe the desired common implementation. They do not claim
that all six mods already conform.
