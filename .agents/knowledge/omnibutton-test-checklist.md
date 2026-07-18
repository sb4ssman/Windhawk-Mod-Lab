# OmniButton Customizer — test checklist

Moved out of the folder README (2026-07-17) when it was regenerated from the
embedded readme block; this is dev-facing content, not user documentation.

- Enable mod with default 2x2 layout
- Toggle off and verify the native horizontal OmniButton returns
- Test `gridColumns: 1` with `itemOrder: "wifi volume battery"`
- Test `gridColumns: 1` with `itemOrder: "wifi volume battery percent"`
- Test `fillOrder: columnFirst`
- Test `itemOrder: "volume wifi battery percent"`
- Change each nudge setting and verify only that item moves
- Restart Explorer and verify the layout reapplies
- Change settings after restart and verify there are no stale transforms
- Verify the OmniButton stays between the expected tray elements and does not
  overlap the customized clock

New since 4e5e347 (add to test pass):

- `batteryPercentMode: independent` — battery and percent at non-adjacent cells;
  verify absolute placement math
- Per-glyph colors (`wifiColor` etc.) — hex, and the new generics
  `accent` / `accentLight` / `accentDark` / `transparent`
- Animated colors (`wifiColorTo` + `colorAnimateDuration`) — looping pulse,
  storyboards stop cleanly on disable
