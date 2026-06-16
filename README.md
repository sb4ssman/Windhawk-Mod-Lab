# Windhawk Mod Lab

Development home for all of sb4ssman's [Windhawk](https://windhawk.net) mods.
Mods are submitted to the community via a fork of [ramensoftware/windhawk-mods](https://github.com/ramensoftware/windhawk-mods).

These mods are generally intended to take advantage of a double-height, two row task bar and the extra space in the system tray it affords. 

---

## Mods

| Folder | Status | Description |
|--------|--------|-------------|
| [omnibutton-customizer/](omnibutton-customizer/) | v1.0, in development | Configurable grid layout for the system tray OmniButton (wifi/volume/battery/percent) |
| [taskmanager-tail/](taskmanager-tail/) | v1.1, published | Keeps Task Manager pinned to the end of the taskbar (Windows 10 & 11) |
| [virtual-desktop-switcher/](virtual-desktop-switcher/) | v1.1, PR update ready | Virtual desktop switcher buttons injected into system tray |
| [privacy-indicator-anchor/](privacy-indicator-anchor/) | v0.1, in development | Keeps location/mic privacy indicator always visible; dim when idle to prevent taskbar icon shifts |
| [clock-spacer/](clock-spacer/) | v1.0, PR submitted | Adds %s% elastic spacer to clock format strings (companion to Taskbar Clock Customization) |
| [taskbar-folder-menu/](taskbar-folder-menu/) | v0.5, prototype | Compact taskbar buttons that open configured folders as popup menus |

---

### [OmniButton Customizer](omnibutton-customizer/)
**Status:** v1.0 — in development
Rearranges the system tray OmniButton (wifi / volume / battery / percent) into a configurable grid with per-item nudges.
→ [README](omnibutton-customizer/README.md)

---

### [Task Manager Tail](taskmanager-tail/)
**Status:** v1.1 — published  
Automatically keeps Task Manager (or any configured app) pinned to the end of the taskbar. Event-driven, zero polling. Supports Windows 10 & 11.  
→ [README](taskmanager-tail/README.md)

---

### [Virtual Desktop Switcher](virtual-desktop-switcher/)
**Status:** v1.1 — PR update ready  
Injects numbered buttons into the system tray — one per virtual desktop — for direct switching by click. Configurable grid layout (smart auto, fixed rows/columns, fill order), master Task View button, and experimental Start-adjacent placement. Fully customizable colors, labels, fonts, borders, and placement.  
→ [README](virtual-desktop-switcher/README.md)

---

### [Privacy Indicator Anchor](privacy-indicator-anchor/)
**Status:** v0.1 — in development  
Keeps the system tray privacy indicator (location / microphone) permanently visible as a dim icon when idle rather than popping in and out, preventing taskbar layout shifts caused by frequent location access (e.g. Windows Web Experience Pack / Widgets).  
→ [Source](privacy-indicator-anchor/privacy-indicator-anchor.wh.cpp)

---

### [Clock Spacer](clock-spacer/)
**Status:** v1.0 — PR submitted
Companion to Taskbar Clock Customization. Adds a `%s%` elastic spacer token to clock line format strings — items on either side of `%s%` are separated by flexible gaps that fill the available width.
Known limitation: `%s%` is not intercepted inside generated composite segments such as the weather string.
→ [README](clock-spacer/README.md)

---

### [Taskbar Folder Menus](taskbar-folder-menu/)
**Status:** v0.5 — prototype
Adds compact taskbar buttons that open configured folders as native popup menus, recreating the useful part of classic taskbar folder toolbars such as Desktop.
→ [README](taskbar-folder-menu/README.md)

---

## Structure

```
Windhawk-Mod-Lab/
  omnibutton-customizer/    — mod source + notes
  taskmanager-tail/         — mod source + notes
  virtual-desktop-switcher/ — mod source + notes
  privacy-indicator-anchor/ — mod source
  clock-spacer/             — mod source
  taskbar-folder-menu/      — mod source + notes
  _claude_notes/            — lab-wide notes and design docs
```
