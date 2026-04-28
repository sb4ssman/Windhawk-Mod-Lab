# Working Notes — Windhawk Mod Lab

## Current focus

1. **Virtual Desktop Switcher** — v0.5, tested, ready to push and submit to windhawk-mods.
2. **PR submission workflow** — script to copy .wh.cpp into windhawk-mods fork and open PR (see TODO below)

---

## Repo consolidation — DONE (2026-04-27)

Both existing mod repos subtree-merged with full history:
- `vertical-omnibutton/` ← Windhawk-Vertical-OmniButton
- `taskmanager-tail/` ← Windhawk-Taskmanager-Tail

Notes: `CLAUDE.md` and `_claude_notes/` were gitignored in the OmniButton source repo — copied in manually.
Original repos left as-is on GitHub (no need to touch them).

---

## Virtual Desktop Switcher status — DONE (2026-04-28)

v0.5 tested and working. Key iterations:
- v0.1: first draft
- v0.2: fixed Grid column overlap bug (SystemTrayFrameGrid uses column layout, not z-order)
- v0.3: added opacity, shine effect, spacing, roman numerals, padding, README
- v0.4: added text color, font size, corner radius, active bold, border, hideWhenSingle, tooltips (desktop names from registry)
- v0.5: fixed disappearing numeral on switch (btn.Foreground(nullptr) bug), fixed corner radius default

**Next: push to GitHub and submit to windhawk-mods.**

---

## TODO (future)

- **windhawk-mods PR update script** — a script that:
  1. Pulls latest from `ramensoftware/windhawk-mods` upstream into the local fork
  2. Copies the updated `.wh.cpp` from the lab mod folder into `mods/`
  3. Creates or updates a PR on the fork with appropriate commit message
  - Should handle both new mod submissions and version bumps to existing ones
  - The windhawk-mods fork lives at `t:/Github/sb4ssman/windhawk-mods/`

## Completed

- vertical-omnibutton v1.51.0 published (final calibration)
- taskmanager-tail v1.0 published
- virtual-desktop-switcher v0.5 ready for submission
