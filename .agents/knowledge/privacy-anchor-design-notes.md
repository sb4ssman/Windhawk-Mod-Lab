# Privacy Indicator Anchor — design notes

Moved out of working_notes 2026-07-17. Broader design doc:
`_research/privacy-indicator-anchor-design.md` (preferred implementation is a
persistent mirrored icon near `NotifyIconStack`, not moving Windows' real
privacy `IconView`).

Motivation: Windows Web Experience Pack accesses location frequently; the
location icon pops in/out causing centered taskbar icon shifts. Existing
`taskbar-tray-system-icon-tweaks` can only hide the icon entirely.

Approach:
- Target `MainStack` IconView elements (privacy indicators — mic/location)
- Register `TextProperty` callback on `InnerTextBlock` → update opacity on state change
- Register `VisibilityProperty` callback on element → override Collapsed → Visible (prevents system hiding)
- Active (text is privacy char): opacity 1.0; Idle (text empty): opacity = idleOpacity/100

XAML path: `SystemTrayIcon > ContainerGrid > ContentPresenter > ContentGrid >
SystemTray.TextIconContent > ContainerGrid > Base > InnerTextBlock`

Active detection chars (from m417z's reference mod):
- `0xE37A` — Geolocation arrow
- `0xF47F` — Mic + Geo combined
- `0xE361`, `0xE720`, `0xEC71` — Microphone variants
- `0xE722` — Camera (only appears with `NoPhysicalCameraLED` / no hardware LED)

Open questions to test:
- Does the VisibilityProperty callback flicker/conflict with m417z's system-icon-tweaks mod?
- Does the element exist in the XAML tree even when never used?
- Does `MainStack > Content > IconStack > ItemsPresenter > StackPanel` match current builds?
