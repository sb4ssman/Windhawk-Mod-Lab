# Privacy Indicator Anchor 2.1 — preflight candidate

Source: privacy-indicator-anchor/privacy-indicator-anchor.wh.cpp.
No installed source or settings were changed by the agent.

1. Confirm idle placeholders in auto and a written arrangement; check opacity,
   colors, glow and tooltips. Click each icon to verify its settings link.
2. Start/stop microphone use in your usual app. Compare with native indicators
   by turning Suppress Windows privacy indicators off/on.
3. Turn the microphone placeholder off while it is in use: Windows' native
   indicator must remain visible. Repeat for location if an app is using it.
   Also try a written layout omitting mic with newly enabled icons set to ignore.
4. Disable/re-enable during microphone use; confirm the native indicator and
   taskbar spacing restore, then the placeholders return. Check idle restoration.
5. Check camera software-access state with hardware monitoring OFF (default).
   If you use the opt-in hardware monitor, separately check shutter evidence and
   turning it off. Camera/Copilot hardware and build behavior remain experimental.
6. Move bottom -> top -> left -> right -> bottom; confirm literal arrangements
   and nudges, auto fitting, settings saves and spacing restoration.
7. If using a content-sized Styler theme, open/close windows and confirm the
   indicator group stays stable. Restart Explorer if convenient and confirm it
   returns. Try all content toggles off and confirm native indicators remain.

Report the exact candidate as working before any PR push. Screenshots of useful
side-taskbar configurations are welcome; avoid private window text.
