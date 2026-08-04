# Tray Privacy Indicator Anchor

A Windhawk mod for Windows 11 that reserves stable tray space for privacy and
status indicators. Location, microphone, camera, and Copilot placeholders stay
visible in the system tray: dim when idle, bright when active.

The goal is to stop taskbar tray sections from shifting when Windows briefly
shows or hides privacy indicators, especially when Windows Web Experience Pack
or Widgets frequently access location.

## Gallery

Idle location and microphone placeholders reserve their tray space without
demanding attention:

![Idle location and microphone placeholders](assets/location-mic-availble-not-in-use.png)

All four unavailable indicators in a single row:

![All four privacy indicators unavailable in one row](assets/all-4-disabled.png)

The same four indicators in a compact block, which is what `auto` picks when
the taskbar is tall enough for two rows:

![All four unavailable indicators in a compact grid](assets/all-4-disabled-grid.png)

Activity highlighted in red — the microphone in use, and the slashed camera
reporting attempted use while its hardware switch blocks it:

![Active microphone in red beside a red slashed camera blocked by its hardware switch](assets/camera-mic-in-use-highlighted.png)

The active glow treatment provides a more emphatic alternative:

![Active microphone and camera with glow](assets/cam-mic-in-use-highlight-glow.png)

Active or requested pathways can remain conspicuous even while blocked:

![Requested or active privacy pathways shown while blocked](assets/requested-or-active-pathways-disabled.png)

The location tooltip explains the access-denied reason and opens the matching
Windows privacy page when clicked:

![Location evidence tooltip and Windows Location settings](assets/location-tooltip-and-win-settings.png)

Microphone evidence distinguishes an endpoint mute from privacy denial:

![Microphone endpoint-mute evidence tooltip](assets/mic-tooltip.png)

Supported camera drivers report their hardware privacy-control evidence:

![Camera hardware privacy-control evidence tooltip](assets/camera-tooltip.png)

Copilot reports its installation state and links to the relevant settings:

![Copilot installation-state tooltip](assets/copilot-tooltip.png)

## Features

- Persistent placeholder icons for location, microphone, camera, and Copilot
- Idle opacity setting so inactive icons can be subtle but still reserve space
- One nestable **Arrangement** expression places the icons in any shape —
  fitted to your taskbar height automatically, or written out by hand
- Turn any of the four icons off individually
- Tray placement before icons, before OmniButton, before clock, after clock, or after Show Desktop
- Experimental placement immediately left or right of Start
- Per-icon and per-group pixel nudges inside the arrangement expression, plus a
  whole-group offset
- Independent idle, active, disabled, glow, and slash colors
- Steady, breathing, or radiating active emphasis with reach/speed controls
- Disabled slash overlays for blocked or unavailable privacy devices
- Hardware camera shutter/kill-switch detection on supported Windows 11 camera drivers
- Evidence-specific tooltips instead of a generic "hardware disabled" label
- Click-through to the relevant Windows privacy, input, camera, taskbar, or app settings
- Optional testing toggle to let Windows' native privacy indicators appear

## Why this starts at 2.0

Version 1.0 was never published — it existed only as a pull request. The 2.0 in
the version field marks the settings contract, not a history of releases: every
mod in this family moved to the same grouped layout — Placement, Content,
Layout, Size, Adjust, Surface, Behavior — and to the shared **Arrangement**
expression that replaced each mod's homegrown grid settings.

**If you installed 1.x by hand from the pull request**, Windhawk cannot carry
values across renamed keys, so your previous customizations are not migrated —
re-apply them once. In particular, **check `Behavior` → `Monitor camera
hardware privacy control`**: it is now opt-in and defaults to off, and it is
the only thing that detects a physical camera shutter or kill switch.

`itemOrder` and the whole grid-mode family are gone, replaced by a single
**Arrangement** field. Grid mode, smart layout, fixed rows and columns, short
row/column position and alignment, and the per-icon nudge settings no longer
exist; what replaced each of them is below.

## The Arrangement field

`Layout` → `Arrangement` decides how the icons are placed, and it is the only
field that does. Its default value is the word `auto`:

- **`auto`** fits the enabled icons to the available taskbar height. `Fill
  order` chooses whether they fill across rows or down columns; `Short row or
  column` aligns a ragged last group. The shape is worked out for you: the mod
  takes the narrowest grid that fits the height, preferring the one that wastes
  the fewest slots — four icons on a double-height taskbar become a 2×2 block,
  not a lopsided 3+1.
- **Anything else** is an arrangement you write. Names sit side by side with
  `|` and stack with `,`, and parentheses group them:

  ```text
  location, camera | mic, copilot     a 2x2 block
  location | mic | camera | copilot   a single row
  location, mic, camera, copilot      a single column
  location | (mic, camera) | copilot  a diamond
  ```

  The tokens are `location`, `mic` (or `microphone`), `camera`, and `copilot`,
  and they are case-insensitive. A separator is always required —
  `location (mic | camera)` is an error, not a shorthand.

**Omitting a token hides that icon**, exactly like turning it off in `Content`.

Every time the layout is applied, the arrangement `auto` produced is written to
the Windhawk log. Copy that line into the Arrangement field and you have the
automatic layout as a starting point to edit — the automatic and manual paths
are the same field and the same syntax. If what you write doesn't parse, the
log says what was expected and where, and the automatic arrangement is used
until you fix it.

**Nudging.** Append a pixel offset to any name to move just that icon:

```text
location[+2,-1] | mic | camera   location moves 2px right and 1px up
(mic, camera)[3,0] | location    the stacked pair moves 3px right
```

Offsets are cosmetic. Nothing else shifts, and the group's overall size does
not change. To move the whole cluster instead, use `Adjust` → horizontal and
vertical offset.

**Enabling an icon later.** An arrangement you write names the icons that
existed when you wrote it. Turn another one on afterwards and it is in no
group, so by default it is appended after your arrangement rather than
vanishing — the log says when that happened. Set `Layout` → `Newly enabled
icons` to *Leave them out* if you would rather your arrangement be the whole
truth. `auto` always includes every enabled icon.

## Placement

The five tray positions reserve a dedicated system-tray column. The
experimental `leftOfStart` and `rightOfStart` positions instead place the
owned indicator group beside Start and reserve matching room in the centered
taskbar items area. These Start-adjacent modes may need adjustment on future
Windows builds or with other mods that also reposition Start.

## States and colors

Each icon has four visual states: idle/available, active, disabled/unavailable,
and active while disabled. The last state keeps the active treatment underneath
the slash by default, so a muted or shuttered device still demands attention
when Windows reports attempted use. `Surface` → `Emphasize blocked activity`
can turn that combined treatment off.

Every color setting — idle, active, disabled, glow, and slash — accepts
`#RRGGBB` or `#AARRGGBB` hex (the alpha byte is honored, and the `#` is
optional), the generics `accent`, `accentLight`, and `accentDark` for the
Windows accent shades, or `transparent`. An empty color uses the system
foreground, except an empty glow color, which follows the active color and then
the Windows accent color.

The glow can be a steady halo, a breathing pulse, or animated radiation rings.
Its opacity, reach, and speed are independent controls. The effect is drawn
inside the existing icon slot and never changes the taskbar width.

For a deliberately striking treatment, start with `Glow style: Radiating
rings`, `Glow opacity: 85`, `Glow reach: 260`, and `Glow cycle: 850`, then
choose an active/glow color that fits the rest of the taskbar theme.

## Notes

Camera hardware-switch detection and the Copilot indicator are experimental
because Windows exposes those states differently across devices and builds.

Camera activity is detected from Windows webcam-usage records and any mirrored
native privacy state. Hardware camera blocking uses Windows 11
`CameraOcclusionInfo` when the camera driver supports it. Cameras without that
driver capability retain software-access and device-availability checks, but
their physical shutter or kill-switch state might not be detectable.

Windows documents idle-camera occlusion reports as advisory rather than an
absolute privacy guarantee. The tooltip therefore says "likely blocked" and
names the camera-driver evidence.

**This monitor is opt-in and defaults to OFF**, under `Behavior` -> `Monitor
camera hardware privacy control`. It is the only thing that detects a physical
shutter or kill switch, so with it off the camera icon still reports software
access, device availability and in-use state, but a shutter will never
register. The mod says so in its log and in the camera's own tooltip rather
than leaving you to wonder. When enabled, it initializes the default camera
controller in `SharedReadOnly` mode but never starts preview or frame capture.
Turn it off if a particular camera activates its LED/indicator or behaves
poorly while monitored. State changes use the driver's native event; a
five-minute watchdog only checks that the subscription remains responsive. The
controller is also released when the camera icon is turned off in `Content`.

Privacy access, usage records, policies, packages, and device topology are
monitored with Windows registry/device events rather than a three-second global
sweep. Copilot process activity is checked separately once per minute, and a
five-minute health reconciliation repairs any missed notification. Failed
camera and registry-monitor setup backs off from seconds to thirty minutes;
access-denied registry monitors remain disabled for the current mod session.

Each icon is clickable. Location opens Location privacy settings. Microphone
opens either microphone privacy or default-input settings according to the
reported reason. Camera opens either camera privacy or camera-device settings.
Copilot opens taskbar or installed-app settings.

`Behavior` -> `Suppress Windows privacy indicators` is on by default, so the
mod hides Windows' own pop-in indicators and mirrors their state into the
stable placeholders. Turn it off temporarily when comparing against Windows'
native tray glyphs during testing. Everything the mod changes on a native icon
is restored to its exact prior value when the mod unloads.
