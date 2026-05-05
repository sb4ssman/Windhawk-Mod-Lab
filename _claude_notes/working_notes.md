# Working Notes — Windhawk Mod Lab

## Current focus

1. **Privacy Indicator Anchor** — v0.3 (Codex). Added layoutMode (row/col), position setting, hideNativeIndicator. Root-cause fix: InjectSyntheticIcons now uses FindChildRecursive for SystemTrayFrameGrid. IsPrivacyGlyph guard prevents false positives. Needs real-device test of: vertical stacking, position options, settings-reinject state tracking, hideNativeIndicator.
2. **Virtual Desktop Switcher** — nextToStart/aboveStart: reservedStartW = startHidden ? gridW : max(startW, gridW). Needs visual test; user saw encroachment with hidden start button.
3. **Clock Spacer** — v0.4. ApplySpacerToDateTimeContent now uses FindChildRecursive to find TimeInnerTextBlock/DateInnerTextBlock directly (fixes root cause: shallow ContainerGrid search failed when intermediate elements exist). User needs MaxWidth set in clock mod (or lineWidth in spacer settings) for * columns to have real width. Needs test.
4. **Taskbar Folder Menu** — v0.3. Default label changed from "Desktop" to "📁" everywhere (YAML, C++ fallback, empty-parse fallback). Comma-split already in SplitFolderLines. Needs test with multi-folder entry.
5. **Vertical OmniButton** — v1.2, PR submitted. Maintainer asked to apply VD switcher review comments; local lab copy uses stoppable CreateThread/event cleanup.

---

## Repo consolidation — DONE (2026-04-27)

Both existing mod repos subtree-merged with full history:
- `vertical-omnibutton/` ← Windhawk-Vertical-OmniButton
- `taskmanager-tail/` ← Windhawk-Taskmanager-Tail

Notes: `CLAUDE.md` and `_claude_notes/` were gitignored in the OmniButton source repo — copied in manually.
Original repos left as-is on GitHub (no need to touch them).

---

---

## Privacy Indicator Anchor — design notes

Motivation: Windows Web Experience Pack accesses location frequently; location icon pops in/out causing centered taskbar icon shifts. Existing `taskbar-tray-system-icon-tweaks` can only hide the icon entirely.

Approach:
- Target `MainStack` IconView elements (privacy indicators — mic/location)
- Register `TextProperty` callback on `InnerTextBlock` → update opacity on state change
- Register `VisibilityProperty` callback on element → override Collapsed → Visible (prevents system hiding)
- Active (text is privacy char): opacity 1.0; Idle (text empty): opacity = idleOpacity/100

XAML path: `SystemTrayIcon > ContainerGrid > ContentPresenter > ContentGrid > SystemTray.TextIconContent > ContainerGrid > Base > InnerTextBlock`

Active detection chars (from m417z's reference mod):
- `0xE37A` — Geolocation arrow
- `0xF47F` — Mic + Geo combined
- `0xE361`, `0xE720`, `0xEC71` — Microphone variants

Potential issues to test:
- Does the VisibilityProperty callback cause any flicker/conflict with m417z's system-icon-tweaks mod?
- Does the element exist in the XAML tree even when never used (privacy indicator never shown)?
- Does `MainStack > Content > IconStack > ItemsPresenter > StackPanel` path match current Windows builds?

## TODO (future)

- **windhawk-mods PR update script** — a script that:
  1. Pulls latest from `ramensoftware/windhawk-mods` upstream into the local fork
  2. Copies the updated `.wh.cpp` from the lab mod folder into `mods/`
  3. Creates or updates a PR on the fork with appropriate commit message
  - Should handle both new mod submissions and version bumps to existing ones
  - The windhawk-mods fork lives at `t:/Github/sb4ssman/windhawk-mods/`

## Current experiments

- Virtual Desktop Switcher has experimental `nextToStart` and `aboveStart` positions for PR #3932 feedback. It injects into `TaskbarFrame > RootGrid`, spans the root grid with a high z-index, and tracks Start button layout instead of joining the taskbar item panel. Needs visual testing in Windhawk.
- Privacy Indicator Anchor tray-grid direction is documented in `_research/privacy-indicator-anchor-design.md`. Preferred implementation is a persistent mirrored icon near `NotifyIconStack`, not moving Windows' real privacy `IconView`.
- Taskbar Folder Menu prototype added in `taskbar-folder-menu/`. It injects compact folder buttons into `SystemTrayFrameGrid` and opens native popup menus with `TrackPopupMenu`. Future placement experiment: line buttons up directly with the tray overflow button if the XAML tree has a stable parent.

## Completed

- taskmanager-tail v1.0 published (PR #3045), updated to v1.1 with Windows 10 support (PR #3143)
- virtual-desktop-switcher v1.0 PR submitted (#3932)
- vertical-omnibutton v1.2 PR submitted
- repo consolidation done (2026-04-27): vertical-omnibutton/ and taskmanager-tail/ subtree-merged
