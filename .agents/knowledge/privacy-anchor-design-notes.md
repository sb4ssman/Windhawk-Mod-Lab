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

## Hardware kill-switch trust model (design, 2026-07-18)

Grew out of the 2026-07-18 Legion probe chain (work_log same date): a
physical camera kill switch cuts the sensor stream below every software
layer this mod checked — PnP stays `Status=OK`, `DeviceAccessInformation`
stays `Allowed`, `MediaCapture::InitializeAsync` succeeds, and no
ConsentStore usage record is written (records are written on stream start,
not on open, so a switch-blocked "successful" open never produces one).
`LENOVO_GAMEZONE_DATA.GetCameraStatus` is the one vendor channel that could
have exposed switch position, and this Legion's firmware simply doesn't
implement it (elevated probe: no camera/privacy methods at all, only
thermal/fan/GPU/keyboard/OC). There is no Windows-generic API for "physical
kill switch position" — vendors that expose it (if any) do so through
private, undocumented, model-specific channels.

**Reframe**: stop trying to prove the switch's on/off position — that's
unknowable in the general case, not just unproven on this machine — and
design around a hardware/software trust split instead:

- **Software's honest job**: presence (is there camera/mic hardware at
  all, per SetupDi/CM_Get_DevNode_Status) and real usage (is a stream
  actually flowing right now, per the ConsentStore in-use scan). Both are
  independently verifiable and already implemented.
- **Hardware's job**: the switch itself, plus its own LED/shutter if it has
  one, is the *only* trustworthy indicator of the switch's position. The
  mod should not simulate or infer that position.
- **Why this isn't a gap, it's a non-goal**: "no usage record" is the
  correct, honest software state whether the camera is idle-and-untouched
  *or* an app is trying to use it while the switch silently blocks the
  feed — both look identical to every API this mod can call. Idle is the
  truthful answer in both cases; inventing a distinct "switch engaged"
  glyph would mean fabricating a signal the mod cannot actually observe.
- **Icon contract stays**: bright = confirmed actively streaming; idle =
  everything else, including hardware-blocked attempts. No disabled-slash
  for camera/mic based on switch position, ever, on any machine.
- **Generalizes past this Legion**: the same reasoning applies to any
  laptop with a physical camera shutter/kill switch, and to mic hardware
  mute switches whose mute state isn't surfaced to the OS audio stack
  (contrast: mic hardware mute that *does* reach the endpoint volume mute
  flag is already legitimately detectable and used for native active-mic
  suppression — that's a real software signal, keep using it where it
  exists; the trust model only applies where no such signal exists).

**Proposed follow-up (not yet built)**: a per-capability tri-state setting,
e.g. `cameraHardwareSwitch` / `micHardwareSwitch` = `auto | present | none`,
purely to drive tooltip/readme wording — not detection. `present` swaps in
copy like "this device may be hardware-gated; trust the physical
switch/LED for its on/off state, this icon only confirms active streaming."
`auto` (default) keeps current behavior unchanged. This turns the
unprovable detection problem into an honest documented contract instead of
a silent limitation.

**Implication for the open Lenovo-WMI todo**: `LENOVO_GAMEZONE_DATA` should
be kept only as a best-effort, self-disabling bonus signal (it already
short-circuits via `s_unavailable` once it fails once) — not something to
keep chasing with more vendor-specific WMI classes (`LENOVO_OTHER_METHOD`
etc.). The trust model means the mod no longer *needs* switch-position
detection to be correct.
