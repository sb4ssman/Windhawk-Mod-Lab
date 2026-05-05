# Work Log — Windhawk Mod Lab

Lab-level milestones only. Per-mod version history lives in each mod's own `_claude_notes/work_log.md`.

## 2026-04-27 — Lab established

- Created Windhawk-Mod-Lab as umbrella repo
- Subtree-merged Windhawk-Vertical-OmniButton → vertical-omnibutton/
- Subtree-merged Windhawk-Taskmanager-Tail → taskmanager-tail/
- Added profiles/ folder for per-machine Windhawk config snapshots
- VD switcher design doc in place; implementation not yet started

## 2026-05-05 — Clock Spacer live tuning

- Clock Spacer v0.7 is visually close with multiple `%s%` instances across top and bottom clock lines.
- Current tested lines:
  - Top: `🤖%cpu%🍵%ram%%s%%time%`
  - Bottom: `%weekday%%s%📅%s%%date%%n%🛫%upload_speed%%s%🛬%download_speed%%n%🧮%gpu%🧮%gpu%%s%💽%disk_read%%n%%weather%`
- Known limitation: `%s%` spacer handling does not work inside the weather string.
