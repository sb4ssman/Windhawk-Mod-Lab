# Maintainer review comment archive (PR #3932, PR #3859)

Reference copies of m417z's inline review comments, moved out of working_notes
2026-07-17. The #3932 items are all resolved (v1.4+ in-tree, mod published).
The #3859 items are pending only if m417z declines the proposal to retire
vertical-omnibutton in favor of omnibutton-customizer.

## PR #3932 — Taskbar Virtual Desktop Switcher (MERGED 2026-06-17, v1.5)

**Comment 3175358025** — on `LoadLibraryExW_Hook`, hooks for `Taskbar.View.dll` / `IconView::IconView`:
> In newer builds, this function was moved to `SystemTray.dll`, see: https://github.com/ramensoftware/windhawk-mods/issues/3926
> Some mods were updated to support both older and newer builds. Please consider updating your mod as well.
→ ACTION: `GetSystemTrayModuleHandle` 3-DLL pattern. Resolved in v1.4.

**Comment 3175381264** — on `GetTaskbarXamlRoot`, offset detection (x64-only, default 0x48):
> Use this updated code with 0x10 default offset and ARM64 support:
> https://github.com/m417z/my-windhawk-mods/blob/e1261d85c2f42006b0dc355fbbc3a8d71a078585/mods/taskbar-multirow.wh.cpp#L261-L291
→ ACTION: default `0x10` + full ARM64 disasm probe. Resolved in v1.4.

**Comment 3175383064** — on `Wh_ModUninit`, `FreeLibrary(hSelf)` inside a `Dispatcher.RunAsync` lambda:
> That might be related to the crashes. Why call `FreeLibrary` here?
→ ACTION: Removed — the DLL must not free itself from a pending async callback.

**Comment 3175387557** — on `Wh_ModAfterInit` retry `std::thread(...).detach()`:
> Is this really necessary? It will cause a crash if the mod is unloaded at this time.
→ ACTION: Stoppable retry thread (`g_retryStopEvent` + wait/join in uninit). Resolved in v1.4.

## PR #3859 — Vertical OmniButton (5 inline comments, review of 2026-05-09)

**Comment 3213784030** — on `GetTaskbarXamlRoot`, `#elif defined(_M_ARM64) // Use default offset.`:
> Please use the same code as can be found here:
> https://github.com/m417z/my-windhawk-mods/blob/e1261d85c2f42006b0dc355fbbc3a8d71a078585/mods/taskbar-multirow.wh.cpp#L261-L291
> In this mod and your other mod.
→ ACTION: full ARM64 disasm probe (4-instruction pattern), not "use default".

**Comment 3213836339** — on `g_retryThread` / `CreateThread` in `Wh_ModAfterInit`:
> The mod already hooks `IconView::IconView`, so why is this thread needed?
→ ACTION: Explain or remove the retry thread (redundant with the IconView hook).

**Comment 3213837729** — on `Wh_Log(L"... (% may not be in tree yet)")`:
> Use `%%` to escape the `%` sign.

**Comment 3213839585** — on README, `→ Settings → Advanced`:
> ```suggestion
> into Windows 11 Taskbar Styler → Settings → Textual mode.
> ```

**Comment 3213844970** — on `iconView.Loaded([](...)` without unsubscribing:
> Unsubscribe after handling, otherwise the callback might be called again when the mod is unloaded, causing a crash.
> https://github.com/ramensoftware/windhawk-mods/blob/a5fb564ecbe5c4cca655a63a0562113123fd5b21/mods/taskbar-tray-system-icon-tweaks.wh.cpp#L1321-L1331
→ ACTION: `winrt::auto_revoke_t{}` pattern, token stored in `g_autoRevokerList`,
erased from within the callback:
```cpp
g_autoRevokerList.emplace_back();
auto autoRevokerIt = g_autoRevokerList.end();
--autoRevokerIt;
*autoRevokerIt = iconView.Loaded(
    winrt::auto_revoke_t{},
    [autoRevokerIt](IInspectable const& sender, RoutedEventArgs const& e) {
        g_autoRevokerList.erase(autoRevokerIt);
        // ... do work ...
    });
```

### Earlier #3859 comments (resolved in v1.4)

- **3129967175**: XAML Diagnostics exclusive-consumer conflict with Taskbar Styler → switched to `GetTaskbarXamlRoot`.
- **3129973528**: persistent registry write (`TaskbarBatteryPercent`) → removed.
- **3130831004** (sb4ssman reply): acknowledged, confirmed both fixed.
