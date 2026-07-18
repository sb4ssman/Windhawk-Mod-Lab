# Six-Mod Settings Audit

Audited against the active sources on 2026-07-16.

Legend: `yes` means canonical/current; `legacy` means a published or truthful
different name; `partial` means the capability exists with fewer modes; `n/a`
means the profile does not belong to the mod.

| Capability | VD Switcher | Folder Menus | Privacy Anchor | OmniButton | Tray Utility | Clock Spacer |
|---|---|---|---|---|---|---|
| Owner model | injected group + Start overlay | injected group | injected icon group | native mutation | native relocation | native text mutation |
| `position` | yes, extended | yes | yes | native | yes | native |
| Smart layout | full balanced/vertical/horizontal | balanced/vertical/horizontal | missing | existing automatic geometry | partial auto row/column | n/a |
| Rows | `buttonRows` legacy | `gridRows` | missing | `gridRows` | `gridRows` | n/a |
| Columns | `buttonColumns` legacy | `gridColumns` | `gridColumns` | `gridColumns` | `gridColumns` | n/a |
| `fillOrder` | yes | yes | yes | yes | yes | n/a |
| `shortGroupPosition` | trailing only | yes | yes | trailing only | trailing only | n/a |
| `shortGroupAlign` | yes | yes | yes | yes | yes | n/a |
| Item order | desktop order fixed | folder array order | `itemOrder` string | `itemOrder` string | `itemOrder` string | format lines/arrays |
| Owned item dimensions | button width/height | button width/height | icon size | slot width/height | button width/height | width/height/max width |
| Shared spacing | `buttonSpacing` | `buttonSpacing` | `buttonSpacing` | horizontal padding only | `buttonSpacing` | `TextSpacing` legacy |
| Group padding | left/right legacy keys | left/right | left/right | horizontal padding, different meaning | missing | n/a |
| Group offsets | vertical legacy key | X/Y | X/Y | n/a | X/Y | content/text layout |
| Per-item offsets | n/a | n/a | yes | yes | yes | n/a |
| Base background | active/inactive | yes | n/a | native | native | n/a |
| Hover/pressed | yes | yes | n/a | native | native | n/a |
| Active/inactive state | yes | n/a | active/idle/disabled | per-glyph color | n/a | n/a |
| Text color/font | active/inactive + font | yes | glyph model | native glyph | native | `TimeStyle`/`DateStyle` |
| Border/radius | yes | yes | n/a | native | native | n/a |
| Opacity | `buttonOpacity` legacy | `opacity` | idle/glow/slash | per-color animation | native | text styles |
| Shine | yes | yes | n/a | gradient endpoints | n/a | n/a |
| Accent keyword | yes | yes | RGB fields | missing | n/a | text-style parser |
| Reversible insertion marker | named root + live column | named root + live column | named root + live column | n/a | explicit marker + snapshots | n/a |
| Startup trigger | IconView + retry | TrayUI + retry | three-DLL IconView + retry | IconView + retry | Loaded hooks + retry | upstream-specific hooks |

## Repeated settings evidence

- `fillOrder` and `shortGroupAlign`: 5 of 6
- `position`: 4 of 6
- column count: 4 of 6 under one canonical name, plus VDS legacy key
- row count: 3 canonical, plus VDS legacy key
- button/item spacing: 4 of 6
- item order: 3 of 6
- group X/Y offsets: 3 of 6
- per-item X/Y offsets: 3 of 6
- owned button width/height: 3 of 6
- shine, radius, and border: the two mods that fully own button surfaces

## Highest-value convergence

1. Adopt one grid result/cell algorithm in the five grid-capable mods.
2. Adopt one button-surface block in VD Switcher and Folder Menus.
3. Adopt marker-based inserted-column cleanup in the three injected tray groups.
4. Adopt one guarded XAML-root and retry lifecycle block in taskbar XAML mods.
5. Keep icon, native mutation, and text-panel profiles distinct.
6. Design universal placement around ownership leases, not a larger `position`
   dropdown copied into every mod.

## Applied in the first non-placement refactor

- Folder Menus gained smart balanced/vertical/horizontal selection, first/last
  short groups, and half-cell-accurate short-group alignment.
- Privacy Anchor now uses the same first/last and half-cell placement math with
  fixed icon cells, while retaining icon-specific state behavior.
- VD Switcher and Folder Menus now share native-default Button state handling,
  explicit hover/pressed resources, shine behavior, and the `accent` keyword.
- Dead integer-sentinel aliases and pointer-only string fallbacks were removed
  from the in-development mods; VDS published storage keys were preserved.
- Privacy Anchor gained the current three-DLL SystemTray discovery path and
  guarded XAML-root access. Retry-thread shutdown was hardened where needed.
- No universal placement host, placement lease broker, or shared-root protocol
  was implemented.

## Remaining differences to resolve gradually

- VDS has the best smart-grid selector but published row/column/opacity/color
  keys must remain stable.
- OmniButton and Tray Utility use enums; VDS, Folder Menus, and Privacy Anchor
  compare strings directly.
- OmniButton and Tray Utility already implement half-cell centering through
  their own native-host offset math, so replacing it would be churn rather
  than a straightforward refactor.
- Clock Spacer is upstream-derived and should be mapped, not mechanically
  renamed or forced into grid/button profiles.
