# Windhawk Mod Templates

This folder is the copy-source library for sb4ssman's taskbar mods. Windhawk
submissions are single-file mods, so these files are not a shared runtime
dependency. Copy a complete block into a mod, keep the template attribution
comment, and adapt only through the documented settings or callback contract.

## Template set

| Template | Use it for |
|---|---|
| `settings-profiles.md` | **THE SETTINGS CONTRACT.** Component library of nested settings groups — fixed keys, labels, defaults — plus the fixed assembly order every mod follows. Assemble from it; never invent, rename, or reorder a key |
| `nested-group-layout.h` | **PRIMARY element-placement primitive.** Pixel-space placement from one nestable layout expression (`\|` along the primary axis, `,` across, parentheses alternate axes) with native-size items, four-side outer padding, first-class per-element nudge, and the `BuildGridExpression` bridge |
| `smart-grid-layout.h` | Shape heuristic only: picks rows × columns (and short-group packing) for a homogeneous, dynamic-count collection. Its output feeds `BuildGridExpression`; it is no longer an arranger |
| `visual-tree-walk.h` | Descendant walk/find/collect helpers plus the OmniButton inner-StackPanel walk |
| `button-surface.h` | Hex/accent colors, native-default clearing, hover/pressed resources, border, opacity, and shine |
| `injected-grid-column.h` | Reversible `SystemTrayFrameGrid` column insertion with marker-based cleanup |
| `start-placement.h` | Experimental owned-group placement immediately left or right of Start with reversible task-item reservation |
| `taskbar-xaml-lifecycle.template.cpp` | Taskbar-thread dispatch, guarded XAML-root access, explicit no-destroy ownership, and race-safe `TrayUI::StartTaskbar` retry lifecycle |
| `placement-contract.md` | Future cross-mod placement vocabulary, ownership rules, and placement lease design |
| `six-mod-settings-audit.md` | Evidence matrix for the six active visual mods |
| `submission-checklist.md` | Documentation parity, screenshot coverage, version, compile, and live-test gate |
| `submission-preflight.ps1` | Automated compile, README/gallery, diff, symbol-hook, and upstream-validator gate |
| `exit-time-destructor-audit.ps1` | Clang gate for unsafe namespace-scope destructors in injected mods |
| `verify-readme-sync.ps1` | Normalized folder README versus embedded Windhawk README parity check |

`tests/smart-grid-layout-tests.cpp` covers balanced selection, half-cell
centering, both fill orders, and first/last short-group placement.
`tests/nested-group-layout-tests.cpp` covers the diamond arrangement, both
primary axes, spacing, absent-token collapse, nesting, cross alignment, parse
failure, four-side outer padding, per-element nudge, and the
`BuildGridExpression` bridge (single row, both fill orders, ragged grids, axis
transpose, and a generated expression round-tripping through the arranger). The
pure layout tests and the WinRT templates are syntax-checked independently.

### Unified element placement (v1.2)

There is one arranger. A mod chooses element positions in exactly one of two
ways, and both end at the same `Compute` call:

* **Manual layout** — the user authors the expression string directly.
* **Auto layout** — `smart_grid::ComputeLayout` picks rows × columns from the
  item count and height budget, then `nested_group_layout::BuildGridExpression`
  turns that grid into the equivalent expression string.

Because both paths produce a string that the same engine parses, measures, and
arranges, centering, per-element nudge, four-side outer padding, and
absent-item collapse behave identically regardless of how the shape was chosen.
This is what lets a mod expose a single "Layout: Auto / Manual" toggle that only
swaps which settings group is active. Per-element nudge is a cosmetic offset
applied to a leaf inside its slot (it never moves a neighbor or resizes the
group); outer padding is applied once around the whole arranged group and each
of its four sides is independently addressable. Do NOT reintroduce a second
placement engine — a shape heuristic emits an expression, it does not place.

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
