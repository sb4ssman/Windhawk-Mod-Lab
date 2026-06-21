# Shared Taskbar Settings Rubric

Last updated: 2026-06-17

This is the repeatable framework for developing visual taskbar mods in this lab
repo before staging them to the sister fork. It is not a demand that every mod
grow the same giant settings panel. It is a way to classify a mod, choose the
controls it actually needs, name those controls consistently, and preserve
compatibility while the lab copy evolves.

## Purpose

Use this rubric when creating or revising a mod that inserts, rearranges, or
styles taskbar UI.

The repeatable loop is:

1. Assign capability profiles.
2. Map the mod against the capability matrix.
3. Add only the controls required by those profiles.
4. Keep old setting names as compatibility aliases.
5. Test in the lab repo.
6. Stage to the sister fork.
7. Sync any fork-side fixes back to the lab source.

## Visual Object Model

Most visual taskbar mods manipulate the same nested object model:

```text
Windows host slot
  group/container inserted into or found in the taskbar
    button/icon/text surface
      content: label, glyph, icon, text, slash, glow, badge
```

Settings should name the layer they affect. This prevents vague controls like
`paddingLeft` from meaning host padding in one mod, group padding in another,
and button content padding in a third.

## Capability Profiles

Profiles are reusable capability bundles. A mod may use several profiles.

### HostInjectionProfile

For mods that insert a new group into the Windows taskbar XAML tree.

Examples:

- `taskbar-vd-switcher`
- `taskbar-folder-menus`
- `privacy-indicator-anchor`
- future tray stats/grid/separator mods

Canonical controls:

- `position`
- `hostWidth`, if the host slot needs explicit reservation
- `hostHeight`, if the host slot needs explicit reservation
- `hostMinWidth`
- `hostMaxWidth`
- `hostOffsetX`
- `hostOffsetY`
- `zIndex`, only when overlap/hit testing requires it

Implementation requirements:

- marshal XAML changes to the taskbar UI thread
- make injection idempotent
- remove injected elements on unload
- undo inserted grid columns/rows/spans on unload
- retry when taskbar XAML is not ready
- keep lab and sister fork copies synchronized after PR fixes

### NativeHostMutationProfile

For mods that reshape an existing Windows-owned host rather than inserting an
independent group.

Examples:

- `omnibutton-customizer`
- `taskbar-clock-customization-spacer`, partly

Canonical controls:

- `hostWidth`, only if the mod can safely reserve native space
- `hostHeight`, only if the mod can safely reserve native space
- `hostOffsetX`
- `hostOffsetY`

Implementation requirements:

- preserve native placement by default
- avoid styling or sizing native surfaces unless the mod explicitly owns them
- restore original values on unload/settings change

### GroupLayoutProfile

For mods that arrange multiple units in rows, columns, or grids.

Examples:

- `taskbar-vd-switcher`
- `omnibutton-customizer`
- `privacy-indicator-anchor`
- `taskbar-folder-menus`

Canonical controls:

- `layoutMode`: row, column, grid, or auto
- `gridMode`: advanced fixed/smart behavior, only when needed
- `gridRows`: fixed rows, 0 for auto
- `gridColumns`: fixed columns, 0 for auto
- `fillOrder`: row-first or column-first
- `smartLayout`: balanced, vertical-pack, horizontal-pack, only when needed
- `shortGroupAlign`: start, center, end
- `buttonSpacing`: gap between item surfaces, even when the surfaces contain icons
- `groupPaddingLeft`
- `groupPaddingRight`
- `groupPaddingTop`
- `groupPaddingBottom`
- `groupOffsetX`
- `groupOffsetY`

Compatibility aliases seen today:

- `buttonRows` -> `gridRows`
- `buttonColumns` -> `gridColumns`
- `gridFillOrder` -> `fillOrder`
- `iconSpacing` -> `buttonSpacing`
- `barOffsetX` / `gridVerticalOffset` -> `groupOffsetX` / `groupOffsetY`
- vague `paddingLeft` / `paddingRight` -> usually `groupPaddingLeft` /
  `groupPaddingRight`

### ButtonSurfaceProfile

For clickable button-like surfaces.

Examples:

- `taskbar-vd-switcher`
- `taskbar-folder-menus`
- future stats or launcher buttons

Canonical controls:

- `buttonWidth`
- `buttonHeight`
- `backgroundColor`
- `hoverBackgroundColor`
- `pressedBackgroundColor`
- `activeBackgroundColor`, only when active/current state exists
- `inactiveBackgroundColor`, only when inactive state exists
- `disabledBackgroundColor`, only when disabled state exists
- `opacity`
- `cornerRadius`
- `borderThickness`
- `borderColor`
- `activeBorderColor`, only when useful
- `shineEffect`

Compatibility aliases seen today:

- `buttonOpacity` -> `opacity`
- `activeColor` -> usually `activeBackgroundColor`
- `inactiveColor` -> usually `inactiveBackgroundColor`

Important rule:

If a mod exposes custom button backgrounds and the surface is visually similar
to another button mod, `shineEffect` should be considered part of the reusable
button surface profile. If a mod skips it, the reason should be explicit.

### IconSurfaceProfile

For visible icon slots that are not ordinary clickable command buttons.

Examples:

- `privacy-indicator-anchor`
- `omnibutton-customizer`, where Windows-owned icon hosts can be adjusted safely

Canonical controls:

- `buttonWidth` / `buttonHeight`, when the icon slot is owned by the mod
- `iconSize`, when the content is an icon rather than text
- `opacity`
- `activeOpacity`, if active/idle state differs
- `inactiveOpacity` or `idleOpacity`, if idle visibility is the main concept
- `activeColor`, if the color affects the icon glyph itself
- `inactiveColor`, if the color affects the icon glyph itself

Special icon effects:

- `glowEnabled`
- `glowOpacity`
- `slashEnabled`
- `slashColor`
- `slashDirection`
- `slashOpacity`

### ContentFormatProfile

For text, glyph, emoji, icon, or generated labels inside a surface.

Examples:

- VDS desktop numbers/roman numerals/dots/custom labels
- Folder Menu labels and default folder glyph
- OmniButton item glyphs/percentage text
- Privacy indicator glyphs

Canonical controls:

- `itemOrder`
- `labelFormat`
- `customLabels`
- `buttonText`, if there is a fallback glyph/text for all items
- `fontSize`
- `textColor`
- `activeTextColor`, only when active/current state exists
- `inactiveTextColor`, only when inactive state exists
- `disabledTextColor`, only when disabled state exists
- `contentOffsetX`
- `contentOffsetY`
- `<item>OffsetX`
- `<item>OffsetY`

Compatibility aliases seen today:

- `wifiX` / `wifiY` -> `wifiOffsetX` / `wifiOffsetY`
- `volumeX` / `volumeY` -> `volumeOffsetX` / `volumeOffsetY`
- `batteryX` / `batteryY` -> `batteryOffsetX` / `batteryOffsetY`
- `percentX` / `percentY` -> `percentOffsetX` / `percentOffsetY`

### ActiveStateProfile

For mods with a current, active, idle, unavailable, hover, or pressed state.

Examples:

- VDS active desktop
- Privacy Anchor active privacy state
- Folder Menus hover/pressed button state

Canonical state naming:

- `activeTextColor`
- `inactiveTextColor`
- `activeBackgroundColor`
- `inactiveBackgroundColor`
- `hoverBackgroundColor`
- `pressedBackgroundColor`
- `disabledTextColor`
- `disabledBackgroundColor`
- `activeBorderColor`
- `activeOpacity`
- `inactiveOpacity`

Rule:

Name the setting by the layer it affects. Avoid bare `activeColor` unless the
mod has exactly one colorable thing and the README says what it means.

### TextPanelProfile

For text panels rather than button/icon groups.

Examples:

- `taskbar-clock-customization-spacer`
- future tray stats text panels

Canonical controls:

- `panelWidth`
- `panelHeight`
- `panelMaxWidth`
- `panelMaxHeight`
- `lineSpacing`
- `textAlignment`
- `fontSize`
- `fontFamily`
- `fontWeight`
- `textColor`
- `contentOffsetX`
- `contentOffsetY`

Clock Spacer note:

Taskbar Clock Customization already has its own settings language. The lab
should not force button styling onto it. Only the shared concepts of width,
alignment, spacing, and text formatting apply.

## Capability Matrix Template

Make or update a matrix before changing a mod family. Use:

- `yes`: implemented under canonical name
- `alias`: implemented under older/different name
- `missing`: belongs to this mod's profiles but not implemented
- `n/a`: not part of this mod's profiles
- `defer`: useful but intentionally postponed

```text
Control / behavior          VDS       FolderMenus   OmniButton   PrivacyAnchor
HostInjectionProfile        yes       yes           n/a          yes
NativeHostMutationProfile   n/a       n/a           yes          n/a
GroupLayoutProfile          yes       partial       yes          partial
ButtonSurfaceProfile        yes       partial       n/a          n/a
IconSurfaceProfile          n/a       n/a           partial      yes
ContentFormatProfile        yes       partial       alias        partial
ActiveStateProfile          yes       hover only    n/a          yes
TextPanelProfile            n/a       n/a           n/a          n/a
position                    yes       yes           native       yes
gridRows                    alias     missing       yes          missing
gridColumns                 alias     yes           yes          yes
fillOrder                   yes       missing       yes          alias
shortGroupAlign             yes       missing       yes          yes
buttonSpacing               yes       yes           missing      alias
groupPaddingLeft/Right      alias     missing       alias        alias
groupOffsetX/Y              alias     missing       missing      alias
buttonWidth/Height          yes       yes           slot alias   n/a
opacity                     alias     yes           n/a          alias
shineEffect                 yes       missing       n/a          n/a
```

This matrix is a planning artifact, not a promise that every `missing` item must
be added immediately.

## Implementation Method

For each mod:

1. Identify profiles.
2. Fill the matrix row for that mod.
3. Sort findings into:
   - required for current behavior
   - good normalization target
   - intentionally not applicable
   - defer
4. Add canonical settings only for required/current-profile controls.
5. Read old names as fallbacks.
6. Update README/settings docs to show canonical names.
7. Keep code comments minimal and layer-specific.
8. Test enable, settings change, Explorer restart, unload, and interaction.
9. Stage to the sister fork only after lab behavior is verified.
10. Copy any accepted fork-side fixes back to the lab repo.

## Compatibility Rule

Do not casually break existing user settings.

When normalizing names:

1. Prefer the canonical setting name.
2. If the canonical setting is empty/default, read the old name as a fallback.
3. Keep old behavior unless the user explicitly chose the new setting.
4. Document canonical names in README and Windhawk settings descriptions.
5. Remove old names only in a deliberate cleanup pass, if ever.

## Settings Order

When a mod has the relevant controls, order settings like this:

1. host placement
2. item/content list
3. layout/grid
4. button/icon dimensions and spacing
5. content formatting
6. button/icon surface styling
7. state styling
8. group padding and offsets
9. host sizing and offsets
10. advanced behavior

## First Practical Target: Folder Menus

Profiles:

- `HostInjectionProfile`
- `GroupLayoutProfile`
- `ButtonSurfaceProfile`
- `ContentFormatProfile`
- limited `ActiveStateProfile` for hover/pressed state

Already present:

- `position`
- `folders`
- `layoutMode`
- `gridColumns`
- `buttonWidth`
- `buttonHeight`
- `buttonSpacing`
- `buttonText`
- `fontSize`
- `textColor`
- `backgroundColor`
- `hoverBackgroundColor`
- `pressedBackgroundColor`
- `borderColor`
- `borderThickness`
- `cornerRadius`
- `opacity`

Likely missing but profile-relevant:

- `gridRows`
- `fillOrder`
- `shortGroupAlign`
- `groupPaddingLeft`
- `groupPaddingRight`
- `groupOffsetX`
- `groupOffsetY`
- `contentOffsetX`
- `contentOffsetY`
- `shineEffect`

Not currently applicable:

- active desktop/current-item state
- icon slash/glow state
- Start overlay placement

This is the kind of scoped answer the rubric should produce for each mod.

