# Windhawk Mod Lab

Development home for all of sb4ssman's [Windhawk](https://windhawk.net) mods.
Mods are submitted to the community via a fork of [ramensoftware/windhawk-mods](https://github.com/ramensoftware/windhawk-mods).

---

## Mods

| Folder | Status | Description |
|--------|--------|-------------|
| [vertical-omnibutton/](vertical-omnibutton/) | v1.2, PR submitted | Vertical stacking of system tray OmniButton (wifi/volume/battery) |
| [taskmanager-tail/](taskmanager-tail/) | v1.1, published | Keeps Task Manager pinned to the end of the taskbar (Windows 10 & 11) |
| [virtual-desktop-switcher/](virtual-desktop-switcher/) | v1.0, PR submitted | Virtual desktop switcher buttons injected into system tray |
| [privacy-indicator-anchor/](privacy-indicator-anchor/) | v0.1, in development | Keeps location/mic privacy indicator always visible; dim when idle to prevent taskbar icon shifts |

---

### [Vertical OmniButton](vertical-omnibutton/)
**Status:** v1.2 — PR submitted  
Rearranges the system tray OmniButton (wifi / volume / battery) from horizontal to vertical stacking. Supports three battery % display modes with per-mode pixel offsets.  
→ [README](vertical-omnibutton/README.md)

---

### [Task Manager Tail](taskmanager-tail/)
**Status:** v1.1 — published  
Automatically keeps Task Manager (or any configured app) pinned to the end of the taskbar. Event-driven, zero polling. Supports Windows 10 & 11.  
→ [README](taskmanager-tail/README.md)

---

### [Virtual Desktop Switcher](virtual-desktop-switcher/)
**Status:** v1.0 — PR submitted  
Injects numbered buttons into the system tray — one per virtual desktop — for direct switching by click. Grid layout auto-sizes rows from taskbar height. Fully customizable colors, labels, fonts, borders, and placement.  
→ [README](virtual-desktop-switcher/README.md)

---

### [Privacy Indicator Anchor](privacy-indicator-anchor/)
**Status:** v0.1 — in development  
Keeps the system tray privacy indicator (location / microphone) permanently visible as a dim icon when idle rather than popping in and out, preventing taskbar layout shifts caused by frequent location access (e.g. Windows Web Experience Pack / Widgets).  
→ [Source](privacy-indicator-anchor/privacy-indicator-anchor.wh.cpp)

---

## Structure

```
Windhawk-Mod-Lab/
  vertical-omnibutton/      — mod source + notes
  taskmanager-tail/         — mod source + notes
  virtual-desktop-switcher/ — mod source + notes
  _claude_notes/            — lab-wide notes and design docs
```
