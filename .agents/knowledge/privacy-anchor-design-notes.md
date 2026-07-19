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

## 2026-07-18 camera kill-switch reorientation

The earlier probe proved only that this Legion exposes no useful PnP,
ConsentStore, Lenovo registry, or Lenovo WMI switch-state change. It did not
prove that the camera stream has no observable privacy state.

Microsoft's current camera privacy-control contract describes the user's exact
behavior: a kill switch can leave the ISP and camera device available while it
disconnects/stops the sensor and substitutes synthesized black frames. The
preferred detector order is therefore:

1. While the camera is active, query
   `VideoDeviceController.CameraOcclusionInfo` and subscribe to its
   `StateChanged` event. Test support for
   `CameraOcclusionKind::CameraHardware`; this is the direct Windows 11 API
   backed by `CT_PRIVACY_CONTROL` / `KSPROPERTY_CAMERACONTROL_PRIVACY` when the
   driver implements the contract.
2. If unsupported, characterize actual frames from a temporary
   `MediaFrameReader` opened `SharedReadOnly`: frame arrival continuity,
   average luminance, spatial variance/entropy, temporal variance, and whether
   the switch produces a stable all-black or fixed replacement image.
3. Treat frame classification as calibrated inference, not universal truth:
   a dark room or covered lens can resemble a privacy frame. Require several
   consecutive frames and a device-specific baseline before showing a slash.

Live paired probe result on this Legion:

- Hardware switch blocked: `CameraHardware` supported, `IsOccluded=True`, and
  `IsOcclusionKind(CameraHardware)=True`.
- Hardware switch enabled: support remained true, while both state values were
  false.

That is a repeatable driver signal, so frame classification is not needed for
this camera. Microsoft nevertheless documents shutter state as advisory unless
the camera is actively streaming; the UI must say "likely blocked" at idle and
must never present the report as an absolute privacy guarantee. The mod owns
one `SharedReadOnly` `MediaCapture` on its state thread, subscribes to
`StateChanged`, and retains the existing software policy/device checks. The
event path is now live-confirmed on this Legion.

Do not add permanent frame capture inside Explorer merely to detect state. If
frame sampling is required for a different unsupported camera, keep it
conditional on an already-active external webcam usage record and explicitly
handle the probe's own ConsentStore footprint/feedback loop.
