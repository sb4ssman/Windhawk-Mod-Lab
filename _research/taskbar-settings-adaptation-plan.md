# Taskbar Settings Adaptation Plan

Last updated: 2026-06-18

This is the staged plan for adapting the visual taskbar mods to the shared
settings rubric without changing default behavior. The goal is uniform
capabilities, naming, code structure, and UI settings where those concepts
apply. The goal is not to make every mod expose every control.

## Normalization Status — 2026-06-24

- `taskbar-folder-menus`: settings follow host, content, layout, dimensions,
  content styling, surface/state styling, group geometry, then Shell behavior.
- `privacy-indicator-anchor`: visible settings use canonical `fillOrder`,
  `buttonSpacing`, `groupPaddingLeft/Right`, and `groupOffsetX/Y`; legacy names
  remain accepted.
- `omnibutton-customizer`: keeps truthful `slotWidth` / `slotHeight`, while
  visible per-item nudges use canonical `<item>OffsetX/Y` names with legacy
  aliases.
- `taskbar-vd-switcher`: settings are ordered by placement, grid, dimensions,
  content, surface/state, group geometry, then special behavior.
- `tray-utility-customizer`: follows the same hierarchy, with detection and
  diagnostics last.
- Clock Spacer and Taskmanager Tail remain outside this visual-control pass.

## Non-Negotiables

- Preserve current defaults.
- Preserve current behavior when users do not touch new settings.
- Read old setting names as compatibility aliases when canonical names are
  introduced.
- Approach each mod individually; use profiles to decide what belongs.
- Keep lab repo and sister fork synchronized after staging or PR fixes.
- Do not normalize by broad refactor. Make narrow, testable passes.

## Shared Implementation Pattern

Each mod adaptation should follow this sequence:

1. Record profiles for the mod.
2. Add canonical settings with defaults that reproduce current behavior.
3. Add alias reads for old names.
4. Internally group settings by profile: host, group, surface, content, state.
5. Apply settings at the right layer in code.
6. Update the embedded Windhawk settings block.
7. Update the mod README only after behavior is verified.
8. Test enable, settings change, Explorer restart, unload, and interaction.
9. Stage to the sister fork.
10. Copy any fork-side fix back into this lab repo.

## Cross-Mod Canonical Names

Use these names when a mod has the relevant capability:

- Host: `position`, `hostWidth`, `hostHeight`, `hostOffsetX`,
  `hostOffsetY`, `zIndex`.
- Group: `layoutMode`, `gridMode`, `gridRows`, `gridColumns`,
  `fillOrder`, `smartLayout`, `shortGroupAlign`, `buttonSpacing`,
  `groupPaddingLeft`, `groupPaddingRight`, `groupPaddingTop`,
  `groupPaddingBottom`, `groupOffsetX`, `groupOffsetY`.
- Button surface: `buttonWidth`, `buttonHeight`, `backgroundColor`,
  `hoverBackgroundColor`, `pressedBackgroundColor`,
  `activeBackgroundColor`, `inactiveBackgroundColor`, `opacity`,
  `cornerRadius`, `borderThickness`, `borderColor`, `shineEffect`.
- Icon surface: `iconSize`, `idleOpacity`, `activeOpacity`,
  `glowEnabled`, `glowOpacity`, `slashColor`, `slashDirection`,
  `slashOpacity`.
- Content: `itemOrder`, `labelFormat`, `customLabels`, `buttonText`,
  `fontSize`, `textColor`, `activeTextColor`, `inactiveTextColor`,
  `contentOffsetX`, `contentOffsetY`, `<item>OffsetX`, `<item>OffsetY`.

## Family Audit

```text
Mod                         Profiles
taskbar-vd-switcher    HostInjection, GroupLayout, ButtonSurface,
                            ContentFormat, ActiveState, StartPlacement
taskbar-folder-menus        HostInjection, GroupLayout, ButtonSurface,
                            ContentFormat, hover/pressed ActiveState,
                            ShellMenu
privacy-indicator-anchor    HostInjection, GroupLayout, IconSurface,
                            ContentFormat, ActiveState
omnibutton-customizer       NativeHostMutation, GroupLayout, IconSurface,
                            ContentFormat
taskbar-clock-spacer        TextPanel, NativeHostMutation subset
system-tray-grid-lines      HostInjection, GroupLayout, SeparatorSurface
taskmanager-tail            Out of visual settings family; lifecycle only
```

## Virtual Desktop Switcher

Current shape:

- Most complete button mod.
- Has placement, smart/fixed grid modes, button dimensions, spacing, active and
  inactive colors, opacity, shine, labels, text colors, radius, border, padding,
  vertical offset, master button, hide-when-single, and Start placement modes.
- Uses older names for several canonical concepts.

Do not change defaults:

- Keep `position: afterClock`.
- Keep current button size, spacing, active color, label format, master button
  defaults, and hide-when-single behavior.

Normalize in phases:

1. Add canonical aliases:
   - `gridRows` reads/falls back to `buttonRows`.
   - `gridColumns` reads/falls back to `buttonColumns`.
   - `opacity` reads/falls back to `buttonOpacity`.
   - `activeBackgroundColor` reads/falls back to `activeColor`.
   - `inactiveBackgroundColor` reads/falls back to `inactiveColor`.
   - `groupPaddingLeft` / `groupPaddingRight` read/fall back to
     `paddingLeft` / `paddingRight`.
   - `groupOffsetY` reads/falls back to `gridVerticalOffset`.
2. Rename internal setting fields only after aliases exist.
3. Keep old Windhawk settings visible until the alias behavior has been tested.
4. Later, update README to teach canonical names while noting old-name
   compatibility.

Implementation notes:

- `shineEffect` implementation already exists; it should become the shared
  reference pattern for button-surface shine.
- Start placement is special to this mod and should remain in a
  StartPlacement-specific section, not pushed into every mod.

## Taskbar Folder Menus

Current shape:

- Host-injected clickable button group.
- Has position, folder list, row/column/grid layout, grid columns, button size,
  button spacing, menu limits, default button text, font size, text/background
  colors, hover/pressed backgrounds, border, radius, and opacity.
- Missing some GroupLayout controls and `shineEffect`.

Do not change defaults:

- Preserve current default folder entries.
- Preserve current `position`, `layoutMode`, `gridColumns`, button dimensions,
  spacing, colors, border/radius sentinel behavior, and opacity.
- Preserve current menu behavior and Shell PIDL behavior.

Normalize in phases:

1. Add group layout controls with inert defaults:
   - `gridRows: 0`
   - `fillOrder: rowFirst`
   - `shortGroupAlign: start` or current-equivalent behavior
   - `groupPaddingLeft: 0`
   - `groupPaddingRight: 0`
   - `groupOffsetX: 0`
   - `groupOffsetY: 0`
   - `contentOffsetX: 0`
   - `contentOffsetY: 0`
2. Add `shineEffect: false` and reuse the VDS gradient pattern for custom
   background colors.
3. Internally split settings into `HostSettings`, `GroupLayoutSettings`,
   `ButtonSurfaceSettings`, `ContentSettings`, and `ShellMenuSettings` or
   equivalent sub-structs.
4. Keep current setting names; no aliases are needed yet except if new canonical
   names replace ambiguous future names.
5. Update README only after visual testing.

Implementation notes:

- `buttonSpacing` already maps cleanly to group row/column spacing.
- `opacity` is already canonical.
- `buttonText` belongs to ContentFormatProfile, not ButtonSurfaceProfile.
- Hover and pressed state are state styling, but active/current state is not
  applicable unless folder buttons gain selected/current semantics later.

## Privacy Indicator Anchor

Current shape:

- Host-injected icon group with active/idle/disabled states.
- Has item order, grid columns, fill order under `gridFillOrder`, short-group
  position/alignment, icon size, placement, padding, icon spacing, whole-bar
  offset, per-item offsets, glow, custom active color, disabled slash color,
  slash direction, and slash opacity.
- Uses icon-specific state styling, not clickable button styling.

Do not change defaults:

- Preserve `idleOpacity`, default item order, grid layout, position,
  icon spacing, offsets, glow defaults, active color default, and slash defaults.

Normalize in phases:

1. Add canonical aliases:
   - `fillOrder` reads/falls back to `gridFillOrder`.
   - `buttonSpacing` reads/falls back to `iconSpacing` only internally; README
     can continue to call it icon spacing if that is clearer for users.
   - `groupPaddingLeft` / `groupPaddingRight` read/fall back to `paddingLeft`
     / `paddingRight`.
   - `groupOffsetX` / `groupOffsetY` read/fall back to `barOffsetX` /
     `barOffsetY`.
2. Consider `gridRows: 0` only if current layout code can support it without
   changing defaults.
3. Keep `idleOpacity` rather than forcing `inactiveOpacity`; it is clearer for
   the privacy state model.
4. Keep RGB active color fields until a hex `activeColor` replacement is tested
   and can read existing values.
5. Split implementation settings into host/group/icon/content/state sections.

Implementation notes:

- This mod should not get `shineEffect`.
- Glow and slash are content/state effects, not button chrome.
- `shortGroupPosition` appears genuinely useful here; do not remove it just for
  symmetry.

## OmniButton Customizer

Current shape:

- Mutates the existing OmniButton rather than injecting an independent button
  group.
- Has slot width/height, grid columns/rows, fill order, short-group alignment,
  horizontal padding, item order, and per-item nudges.
- Keeps outer OmniButton placement native and mostly avoids button styling.

Do not change defaults:

- Preserve native host placement.
- Preserve current slot geometry, item order, grid defaults, and per-item nudge
  behavior.

Normalize in phases:

1. Add canonical aliases for per-item offsets:
   - `wifiOffsetX` / `wifiOffsetY` read/fall back to `wifiX` / `wifiY`.
   - `volumeOffsetX` / `volumeOffsetY` read/fall back to `volumeX` /
     `volumeY`.
   - `batteryOffsetX` / `batteryOffsetY` read/fall back to `batteryX` /
     `batteryY`.
   - `percentOffsetX` / `percentOffsetY` read/fall back to `percentX` /
     `percentY`.
2. Decide whether `slotWidth` / `slotHeight` should stay as-is. They are more
   accurate than `buttonWidth` / `buttonHeight` because the mod does not own
   button surfaces.
3. Consider aliasing `buttonHorizontalPadding` to `groupPaddingLeft/Right` only
   if code proves it affects group padding rather than native button chrome.
4. Keep style controls out unless there is a tested, reversible way to style
   the native OmniButton safely.

Implementation notes:

- This is not a ButtonSurfaceProfile mod by default.
- Avoid adding hover/pressed/background/radius/shine controls until the mod owns
  those surfaces rather than nudging Windows-owned children.

## Taskbar Clock Customization Spacer

Current shape:

- Submitted integration scratch for Taskbar Clock Customization.
- Works through clock text formatting and XAML text layout, not taskbar button
  injection.
- Has width/height/max width, text spacing, top/bottom line styles, text color,
  alignment, font settings, character spacing, and spacer tokens.

Do not change defaults:

- Preserve upstream Taskbar Clock Customization defaults.
- Preserve `%s%` and `{spacer}` behavior.
- Do not force button/icon group settings onto this mod.

Normalize in phases:

1. Treat as `TextPanelProfile` only.
2. Document mapping without renaming upstream-style settings:
   - `MaxWidth` corresponds to `panelMaxWidth`.
   - `TextSpacing` corresponds to `lineSpacing`.
   - `TimeStyle` / `DateStyle` correspond to content text styling.
3. Avoid changing setting names while this is tracking upstream.
4. If a standalone stats/text panel emerges later, use the canonical
   `TextPanelProfile` names there instead of back-porting them into the
   upstream clock mod.

Implementation notes:

- Because this is already submitted upstream, changes should be extra narrow.
- Live Windhawk visual validation remains required for layout claims.

## System Tray Grid Lines

Current shape:

- Concept folder only.

Planned profiles:

- `HostInjectionProfile`
- `GroupLayoutProfile`, if it supports section/minor line grids
- future `SeparatorSurfaceProfile`

Planned canonical controls:

- `position`
- `lineColor`
- `lineOpacity`
- `lineThickness`
- `lineSpacing` or section-specific spacing
- `hostOffsetX`
- `hostOffsetY`
- `zIndex`, if overlay hit-testing becomes relevant

Implementation notes:

- Should default to off/minimally visible if implemented.
- Must not intercept clicks unless explicitly designed as an interactive tool.

## Taskmanager Tail

Current shape:

- Event-driven taskbar ordering utility, not a visual layout/style mod.

Plan:

- Do not adapt to the visual settings rubric.
- Keep only general Windhawk lifecycle, settings clarity, and maintainer-safety
  guidance in common.

## Suggested Order Of Work

1. Folder Menus
   - Smallest active button-surface mod.
   - Add missing profile controls and shine without changing defaults.
2. Privacy Anchor
   - Normalize aliases around group layout and offsets.
   - Keep icon-state semantics intact.
3. OmniButton
   - Normalize per-item offset aliases.
   - Avoid button-style creep.
4. VDS
   - Already feature-rich; add canonical aliases and internal cleanup after the
     smaller mods prove the pattern.
5. Clock Spacer
   - Documentation-only mapping unless upstream feedback requires code changes.
6. Grid Lines
   - Use the profiles before implementation starts.

## Verification Checklist Per Mod

- Default settings produce the same visual result as before.
- Existing old setting names still work.
- New canonical settings work when explicitly changed.
- Settings change reloads without duplicate injected elements.
- Explorer restart reapplies the layout.
- Unload removes injected elements and restores native UI.
- Click/hit-test behavior is unchanged.
- README and embedded Windhawk settings describe only controls that are real.
