# Icon inside OmniButton / Quick Settings experiment

This experiment investigates the actual question: can an outside icon occupy
OmniButton's area, or the footer/gutter of the flyout it opens?

First executable probe: `pwsh -NoProfile -File anchor-probe.ps1 -Mode Omni`.
For the gutter, open Quick Settings yourself and run it with `-Mode QuickSettings`.
It enumerates visible controls in candidate shell windows, returning automation
identities and physical screen rectangles. Window classes are candidates, not a
claim that every Windows build exposes this tree. An empty result is evidence
that this approach needs another shell-host discovery method. Output may contain
tray labels; keep captures local rather than committing them.

`overlay.ps1` puts a supplied ICO over one exact detected automation identity:

```powershell
pwsh -NoProfile -STA -File overlay.ps1 -IconPath C:\path\circle.ico -Mode Omni -AutomationId VERIFIED_ID -OffsetX 4 -OffsetY 4
```

Use a verified ID from the probe, not the literal placeholder above. Size is
8..64 pixels; offsets are physical pixels. It hides when the target is absent
or ambiguous. Right-click the icon to exit. Optional `-OnClickScript` invokes
your local action script on a deliberate left-click. Without one it is display
only. The icon file reloads when modified; it does not yet follow Power Circle's
cached icon selection automatically.

This is an external overlay feasibility prototype, **not native XAML injection**
and not relocation of a tray icon. It reserves no native layout space, so it can
cover a native icon/click target. It does not add a `powerCircle` arrangement
token. It neither sends hotkeys nor changes power modes automatically.

Next native stage: use the verified host identity to create a leased sibling
inside the button's container, measuring its footprint and restoring it on
unload. Quick Settings needs its own host/lifecycle investigation; Lenovo's
button does not establish a public extension API. Keep this outside the published
OmniButton mod until it survives the user's live tests on horizontal/side bars,
flyout open/close, DPI changes, and Explorer restart.
