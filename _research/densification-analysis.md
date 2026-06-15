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
- `%s%` also works at the beginning and/or end of a line, which makes it an
  alignment primitive, not only an inter-item spacer:
  - `%s%content` pushes the content right
  - `content%s%` keeps the content left
  - `%s%content%s%` centers the content
  This is important for mixed clock/stat layouts where each row may need its
  own alignment while still sharing the same fixed clock width.

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
- Near-term standalone UX:
  - Keep fixed width manual for now (`Max clock width` here or `Clock max width` in
    Taskbar Clock Customization).
  - Add/readme-document outside spacers as the recommended way to left/right/center
    individual rows.
  - Consider optional `minGap` / `maxGap` settings so short rows do not look absurdly
    stretched when the width is chosen to survive a long day name.
- Best sizing algorithm:
  1. Parse each row into segments split by `%s%`.
  2. Generate candidate strings for variable tokens, especially all seven weekdays.
  3. Measure each segment using real XAML `TextBlock.Measure()` with the active font,
     weight, stretch, size, character spacing, and DPI.
  4. Compute `requiredWidth = sum(segmentWidths) + minGap * spacerCount`.
  5. Pick the max required width across candidate rows, then optionally cap visual
     spacer width with `maxGap`.
- Important constraint: fixed width + fill means shorter strings necessarily get larger
  gaps than longer strings. Measurement can prevent clipping and pick a better width,
  but it cannot make every weekday look equally spaced without allowing dynamic width,
  capped gaps, or abbreviated/custom weekdays.
- The slickest implementation probably belongs inside Taskbar Clock Customization,
  because that mod owns the raw format strings and token expansion logic. As a
  standalone companion, Clock Spacer mostly sees only the already-rendered final text.

**Maintainer pitch:**
- Approach with respect for m417z's ownership:
  - First, polish the standalone companion mod enough to demonstrate the behavior,
    limitations, and unload safety.
  - Open a discussion/issue or PR comment before proposing a direct patch to
    Taskbar Clock Customization: frame it as "I built a companion experiment; would
    you be open to this as a native token?"
  - Keep the patch minimal if upstreamed: add an elastic spacer token to the existing
    formatter/layout pipeline, with no broad refactor and no duplicate copy of the
    whole mod.
  - Be explicit that m417z may prefer this to remain a separate companion mod.
    The goal is to offer a clean contribution, not to impose a design direction.
- Pitch angle if upstream is welcome:
  "Adds an elastic spacer token `%s%` for dense multi-line clock layouts. Internal and
  edge spacers allow per-row left/right/center alignment while preserving a fixed clock
  width."

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
