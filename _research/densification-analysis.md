# Tray Densification — Analysis and Ideas

Last updated: 2026-05-08

## Screenshot analysis (2026-05-08)

Double-height taskbar, mods active:
- Left: pinned app icons, `^` overflow chevron, bluetooth, wifi-off, volume, mic (privacy indicator active), 100% battery
- Clock area: 6 "rows" of stats stacked: cpu/ram + time / date / net speeds / GPU+disk / weather+wind
- Far right: VD switcher (3 stacked desktop buttons)

**Visible density issues:**
1. Tray icon row has normal spacing — not wasted but not packed
2. Privacy indicator (mic) is a full-width column slot that's usually empty or idle
3. The overflow chevron `^` is a full-width column, used infrequently
4. OmniButton (wifi/vol/bat) — Vertical OmniButton stacks these in 1 column, good
5. VD switcher — 1 column of 3 buttons, correct
6. Gap between clock and desktop switcher column — depends on flex columns in the grid

**Observation: the biggest wasted real estate is the overhead of separate columns for**:
- Overflow chevron
- Privacy indicators (mic/location) — nearly always idle

These two things could plausibly share a single column on a double-height taskbar.

---

## Chevron + Privacy Icon column sharing

**The idea:** On a double-height taskbar, the `^` overflow chevron and the privacy indicators
(mic, location) could share a single column — the chevron in one row, the privacy indicator
mirrored below (or above) it.

**Why this is interesting:**
- Both are small, use-infrequent elements that always occupy their own column
- On double height, each column has 2× the vertical space — perfect for stacking
- Would free up a column for other content or reduce total tray width

**XAML location of each:**
- Overflow chevron: `SystemTrayFrameGrid` → likely `ChevronSystemTrayFrameController` or similar.
  Need to inspect live tree to confirm exact element name. The Taskbar Folder Menu experiment
  targeted this area too — check that code for any discovered names.
- Privacy indicators: `SystemTray.Stack#MainStack` → `SystemTray.IconView` elements.
  These are system-owned and not safe to move directly. Privacy Indicator Anchor mod
  mirrors them instead.

**Architecture sketch:**
1. Inject a shared column into `SystemTrayFrameGrid` at the chevron position.
2. Inside: a `Grid` with 2 row definitions (each ~half the taskbar height).
3. Row 0: mirrored privacy indicator (from Privacy Indicator Anchor logic).
4. Row 1: the native chevron button (keep it functional — it must remain interactive).
5. The native chevron column is removed and replaced by this shared column.
   OR: native chevron is kept but visually moved inside the injected overlay.

**Risks:**
- Moving or re-parenting the native chevron button is fragile (same problem as privacy IconView).
  Better to inject an overlay Grid that covers both positions and forward click events.
- The chevron click handler may be tied to its native parent — needs testing.
- On single-height taskbars this doesn't apply (no room to stack).

**Prerequisite:** Confirm the chevron XAML element name by inspecting the live tree.
The VD switcher's `FindChildRecursive` approach scanning for known names is the template.

**This feature could live in Privacy Indicator Anchor** (since it already mirrors the privacy
icon) and add optional chevron-sharing as a placement mode.

---

## Clock Spacer — current limitations and path forward

**What works:**
- `%s%` token distributes leftover space as gap columns between text segments
- Multi-line format strings with `\n` produce separate spacer rows per line
- Style (font, color, size) copied from source TextBlock

**What doesn't work well:**
- Width detection is unreliable without a fixed `Max Width` set in the Clock mod
- The width varies with content — the widest element (usually full day name like "Wednesday")
  determines the natural width; `%s%` gaps shrink when that day appears
- The `%s%` token inside the weather string (which the clock mod generates as a single chunk)
  is never seen by our hook, so it can't be intercepted

**What the user wants:**
- The clock area to stay a fixed predictable width always
- Content to align consistently regardless of which day/value is currently shown
- This is fundamentally the `Max Width` problem: both the clock mod and the spacer mod
  need to agree on the fixed width, and the user must set it

**Path to improvement:**
- Add a `Measure widest` button or auto-detection: scan the TextBlock's font metrics and
  the format string to estimate the widest plausible content, then apply that as MaxWidth.
  Hard to do correctly for arbitrary emoji/unicode content.
- Better UX: expose a single `Fixed width (px)` setting that our mod forwards to both the
  source TextBlock and the generated spacer grid, so the user only has to set it once (here).
  The user already has `Max clock width` setting — verify it's being applied correctly.
- Consider: should the mod snap to the widest currently-observed width and lock it in?
  A `Calibrate` approach: after a few minutes, measure peak ActualWidth and store it.

**Maintainer pitch:**
- The spacer logic is small enough (~200 lines of active code) to propose as a direct
  addition to m417z's Taskbar Clock Customization mod.
- Pitch angle: "adds elastic spacer token `%s%` — drops in alongside existing token
  processing in OnApplyTemplate, no new hooks needed if inside the same mod."
- Separately, the maintainer may prefer to keep mods small and composable.
  Either way, the Clock Spacer should be PR-ready as a standalone mod first.

---

## Tray Stats mod — future idea

**User's instinct:** Windows 11 removed visual density from the taskbar. Windhawk gives
some of it back, but the system tray could be a richer info panel. A dedicated "Tray Stats
Panel" mod could:
- Inject a stats panel anywhere in the tray (not tied to the clock area)
- Show CPU, RAM, GPU, network, disk, battery in configurable layout
- Use the same column-injection pattern as VD switcher
- Allow icon+value rows (emoji glyph + number, like the clock mod does now but decoupled from the clock)

**Relationship to clock mod:**
- The clock mod already does this via format tokens (`%cpu%`, `%ram%`, etc.)
- A Tray Stats Panel would be independent: not a clock, not requiring a fixed-width clock area
- It would make more sense as a free-standing panel with its own width/height settings

**Why this might be better than the clock approach:**
- Stats don't need to be near the clock — they could be left of the OmniButton, right of
  the notification icons, etc.
- Doesn't require Clock Customization to be installed
- Can be configured for any subset of stats without worrying about max-width and spacer tokens

**Note to self:** Before building this, check if any existing Windhawk mod does it —
`taskbar-tray-system-icon-tweaks` or something similar may cover some of this ground.

---

## Mod priority and sequencing implications

Densification goal requires these mods to cooperate:
1. **VD Switcher** — column for desktop buttons (done, modulo layout bug)
2. **Vertical OmniButton** — column for wifi/vol/bat stacked (done)
3. **Privacy Indicator Anchor** — column for privacy icons (in development)
4. **Clock Spacer** — fixed-width clock area (in development)
5. **Chevron sharing** — potential future enhancement of Privacy Indicator Anchor

When all five are working, the tray should have:
- [VD buttons] | [notification icons + chevron (shared)] | [privacy mirror] | [OmniButton] | [clock stats] | [show desktop]

The goal: each column earns its pixels.
