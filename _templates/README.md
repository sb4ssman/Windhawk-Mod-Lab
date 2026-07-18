# Windhawk Mod Templates

This folder is the copy-source library for sb4ssman's taskbar mods. Windhawk
submissions are single-file mods, so these files are not a shared runtime
dependency. Copy a complete block into a mod, keep the template attribution
comment, and adapt only through the documented settings or callback contract.

## Template set

| Template | Use it for |
|---|---|
| `settings-profiles.md` | Canonical setting names, order, defaults, and profile boundaries |
| `smart-grid-layout.h` | Row, column, fixed-grid, and smart-grid calculation plus short-group placement |
| `button-surface.h` | Hex/accent colors, native-default clearing, hover/pressed resources, border, opacity, and shine |
| `injected-grid-column.h` | Reversible `SystemTrayFrameGrid` column insertion with marker-based cleanup |
| `taskbar-xaml-lifecycle.template.cpp` | Taskbar-thread dispatch, guarded XAML-root access, `TrayUI::StartTaskbar`, and bounded retry lifecycle |
| `placement-contract.md` | Future cross-mod placement vocabulary, ownership rules, and placement lease design |
| `six-mod-settings-audit.md` | Evidence matrix for the six active visual mods |
| `submission-checklist.md` | Documentation parity, screenshot coverage, version, compile, and live-test gate |
| `verify-readme-sync.ps1` | Normalized folder README versus embedded Windhawk README parity check |

`tests/smart-grid-layout-tests.cpp` covers balanced selection, half-cell
centering, both fill orders, and first/last short-group placement. The pure
layout test and all three WinRT templates are syntax-checked independently.

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

## Update workflow

When a mod exposes a reusable fix:

1. Prove it in the mod that discovered the issue.
2. Update the relevant template and its version note.
3. Compare every adopter against the changed contract.
4. Roll it out one mod at a time; never bulk-replace published setting keys.

The templates describe the desired common implementation. They do not claim
that all six mods already conform.
