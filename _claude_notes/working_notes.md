# Working Notes — Windhawk Mod Lab

## Current focus

1. **Privacy Indicator Anchor** — v0.1, just created. Keeps location/mic privacy indicator always visible (dim when idle) to prevent taskbar layout shifts. Needs testing.
2. **Virtual Desktop Switcher** — v1.0, PR submitted (#3932). Awaiting review.
3. **Vertical OmniButton** — v1.2, PR submitted. Awaiting review.

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

## Completed

- taskmanager-tail v1.0 published (PR #3045), updated to v1.1 with Windows 10 support (PR #3143)
- virtual-desktop-switcher v1.0 PR submitted (#3932)
- vertical-omnibutton v1.2 PR submitted
- repo consolidation done (2026-04-27): vertical-omnibutton/ and taskmanager-tail/ subtree-merged
