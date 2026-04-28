# Windhawk Mod Lab

Development home for all of sb4ssman's [Windhawk](https://windhawk.net) mods.
Mods are submitted to the community via a fork of [ramensoftware/windhawk-mods](https://github.com/ramensoftware/windhawk-mods).

---

## Mods

### [Vertical OmniButton](vertical-omnibutton/)
**Status:** v1.2 — published  
Rearranges the system tray OmniButton (wifi / volume / battery) from horizontal to vertical stacking. Supports three battery % display modes (off / inline / stacked) with per-mode pixel offsets.  
→ [README](vertical-omnibutton/README.md) · [Source in windhawk-mods](../windhawk-mods/mods/vertical-omnibutton.wh.cpp)

---

### [Task Manager Tail](taskmanager-tail/)
**Status:** v1.0 — published (Windows 11); v1.1 Win10 support in progress  
Displays live CPU and RAM usage on the taskbar, tailing Task Manager data.  
→ [README](taskmanager-tail/README.md) · [Source in windhawk-mods](../windhawk-mods/mods/task-manager-tail.wh.cpp)

---

### [VD Switcher](vd-switcher/) *(in development)*
Injects numbered buttons into the taskbar/system tray — one per virtual desktop — for direct switching by click. Supports grid layout and flexible placement anywhere on the taskbar.  
→ [Design doc](_claude_notes/vd-switcher-design.md)

---

## Structure

```
Windhawk-Mod-Lab/
  vertical-omnibutton/   — mod source + notes
  taskmanager-tail/      — mod source + notes
  vd-switcher/           — mod source + notes (in development)
  profiles/              — per-machine Windhawk config snapshots
  _claude_notes/         — lab-wide notes and design docs
```
