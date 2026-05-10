# Maintainer PR Feedback — m417z review notes

Last updated: 2026-05-09

## PR #3932 — Virtual Desktop Switcher

### Comment: IconView::IconView moved to SystemTray.dll (newer builds)
> "In newer builds, this function was moved to `SystemTray.dll`. Some mods were updated to
> support both older and newer builds. Please consider updating your mod as well."
> — refs issue #3926

**Status: DONE** — `GetSystemTrayModuleHandle` already handles the 3-DLL chain
(SystemTray.dll → Taskbar.View.dll w/ version check → ExplorerExtensions.dll).

---

### Comment: GetTaskbarXamlRoot — use 0x10 default + ARM64 disasm probe
> "Use this updated code with 0x10 default offset and ARM64 support."
> — refs https://github.com/m417z/my-windhawk-mods/blob/e1261d85c2f42006b0dc355fbbc3a8d71a078585/mods/taskbar-multirow.wh.cpp#L261-L291

**Status: DONE (2026-05-09)** — Applied to both VDS and OmniButton.

The correct pattern (replace the old `0x48` default + `// Use default offset.` stub):
```cpp
size_t offset = 0x10;
#if defined(_M_X64)
    {
        // 48:83EC 28 | sub rsp,28
        // 48:83C1 48 | add rcx,48
        const BYTE* b = (const BYTE*)TaskbarHost_FrameHeight_Original;
        if (b[0]==0x48 && b[1]==0x83 && b[2]==0xEC && b[4]==0x48 &&
            b[5]==0x83 && b[6]==0xC1 && b[7]<=0x7F)
            offset = b[7];
        else
            Wh_Log(L"Unsupported TaskbarHost::FrameHeight");
    }
#elif defined(_M_ARM64)
    {
        // 7f2303d5 pacibsp
        // fd7bbfa9 stp     fp, lr, [sp, #-0x10]!
        // fd030091 mov     fp, sp
        // 080c41f8 ldr     x8, [x0, #0x10]!
        const DWORD* p = (const DWORD*)TaskbarHost_FrameHeight_Original;
        if (p[0] == 0xD503237F && (p[1] & 0xFFC07FFF) == 0xA9807BFD &&
            p[2] == 0x910003FD && (p[3] & 0xFFF00FE0) == 0xF8400C00)
            offset = (p[3] >> 12) & 0xFF;
        else
            Wh_Log(L"Unsupported TaskbarHost::FrameHeight");
    }
#else
#error "Unsupported architecture"
#endif
```

The ARM64 probe reads the pre-index load offset from the 4th instruction of
`TaskbarHost::FrameHeight`. The mask `0xFFF00FE0` on `p[3]` captures the ldr
opcode+size while leaving the immediate field free; `(p[3] >> 12) & 0xFF` extracts it.

**Must apply to any new mod that uses GetTaskbarXamlRoot.**

---

### Comment: FreeLibrary inside Dispatcher.RunAsync lambda
> "That might be related to the crashes. Why call FreeLibrary here?"

**Status: NOT PRESENT** — Current VDS code has no `FreeLibrary` in any lambda.
This comment was on an older PR version. No action needed.

---

### Comment: std::thread().detach() retry loop
> "Is this really necessary? It will cause a crash if the mod is unloaded at this time."

**Status: ADDRESSED** — VDS uses `CreateThread` with `g_retryThread`/`g_retryStopEvent`
and `WaitForSingleObject(INFINITE)` in `Wh_ModUninit`. The detach pattern is gone.

The maintainer's deeper question: is the retry thread needed at all given the
`IconView::IconView` hook? For VDS: yes, because the hook only fires for new
IconViews; on mod load when the tray is already populated, the hook won't fire.
The retry thread catches the initial load. Consider adding a comment explaining this.

---

### General: Crashes reported during review
> "Sometimes only half of the button is clickable. Disabling the mod crashes explorer."

**Click issue status**: H1 (ZIndex conflict) ruled out by diagnostic log. H2 (ShowDesktop
hover area) and H4 (WH_CALLWNDPROC delivery window) remain as candidates.

**Crash-on-disable status**: FIXED — `MsgWaitForMultipleObjects(QS_SENDMESSAGE)` in
`StopNotificationThread`, `g_unloading` guard removed from `RunFromWindowThread`
(was blocking uninit's own `RemoveButtonGrid` call).

---

## PR #3859 — Vertical OmniButton (v1.4 review, 2026-05-09)

### Comment: GetTaskbarXamlRoot ARM64 probe
> "Please use the same code as can be found here [taskbar-multirow.wh.cpp#L261-L291].
> In this mod and your other mod."

**Status: DONE (2026-05-09)** — Applied. Same fix as VDS above.

---

### Comment: Retry thread in Wh_ModAfterInit unnecessary?
> "The mod already hooks `IconView::IconView`, so why is this thread needed?"

**Status: OPEN** — The retry thread exists because on initial mod load, existing
IconViews have already been constructed (the hook only fires for new constructions).
Without the retry, the mod doesn't apply when the tray is already populated at load time.

However, the maintainer may not be aware of this constraint or may have a better
approach in mind. **Consider adding a clear comment, OR investigate whether there's
a way to enumerate existing IconViews at hook time to eliminate the retry entirely.**

Reference: other mods that avoid the retry thread use a different initial trigger
(e.g. hooking `IconView::OnLoaded` or a frame-level hook that fires even for
already-loaded elements).

---

### Comment: %% escape in Wh_Log
> "Use `%%` to escape the `%` sign."

**Status: DONE (2026-05-09)** — Fixed line: 
`Wh_Log(L"[Battery4] No inner StackPanel found (%% may not be in tree yet)");`

---

### Comment: README text
> Suggested diff: `into Windows 11 Taskbar Styler → Settings → Textual mode.`

**Status: OPEN (LOW PRIORITY)** — Minor doc tweak, apply before next PR update.

---

### Comment: iconView.Loaded without unsubscribing → crash risk
> "Unsubscribe after handling, otherwise the callback might be called again when
> the mod is unloaded, causing a crash."
> — refs taskbar-tray-system-icon-tweaks.wh.cpp#L1321-L1331

**Status: DONE (2026-05-09)** — Replaced with auto-revoke pattern using `g_autoRevokerList`.

The pattern (self-cleaning on first call):
```cpp
g_autoRevokerList.emplace_back();
auto autoRevokerIt = std::prev(g_autoRevokerList.end());
*autoRevokerIt = iconView.Loaded(
    winrt::auto_revoke_t{},
    [autoRevokerIt](IInspectable const&, RoutedEventArgs const&) {
        g_autoRevokerList.erase(autoRevokerIt);
        if (!g_unloading && !g_omniStackPanel)
            ApplyAllSettings();
    });
```

`g_autoRevokerList` is `std::list<FrameworkElement::Loaded_revoker>`.
The list must be a global (not local) so the revokers stay alive until the
callback fires. `std::list` is used (not `std::vector`) because iterator
stability is required — the lambda captures `autoRevokerIt`.

**Must apply to any mod that subscribes `Loaded` inside a constructor hook.**

---

## Checklist — items to apply to new mods

When writing a new system-tray mod that uses `GetTaskbarXamlRoot`:
- [ ] Use `0x10` default + full ARM64 disasm probe (copy from above)
- [ ] `iconView.Loaded` → always use auto-revoke + `g_autoRevokerList` pattern
- [ ] `std::thread().detach()` → never; use `CreateThread` + stop event + `WaitForSingleObject`
- [ ] `%%` in any `Wh_Log` format string containing a literal `%`
- [ ] Add comment on retry thread explaining why it's needed alongside the hook
