# Round 5/6 AI reviews — verified analysis (2026-09-21)

The bot reviewed the heads we pushed from lab commit `6cf1320`. Its SHAs match:
#4443 a78f6636, #4843 b77afb23, #4844 b5a24403, #4855 c48a4f1e, #5568 20abef99,
#5569 2f2a7da7. Raw threads are in `prN-comments.md`; no inline comments and no
review objects this round.

## Verdicts

| PR | Mod | Verdict | Blocking item (verified in source) |
|---|---|---|---|
| #4443 | Clock Spacer | Clean, "looks good to merge" | — |
| #4843 | Privacy Anchor | Clean, "looks good to merge" | — |
| #5569 | Tray Utility | Clean, "looks good to merge" | — |
| #4844 | VD Switcher | 1 item | `OnTaskbarRebuilt` keeps stale `g_buttonGrid`; IconView hook and retry both skip on non-null grid, so a failed immediate apply leaves the bar missing, and a successful one leaks the old ButtonEventState records. CONFIRMED. |
| #4855 | OmniButton | 1 item | `OnLayoutUpdatedImpl` (3850-3853) flags change whenever presenter is null with childCount >= 1/2, but `ApplyLayout` (3491-3492) deliberately leaves them null when battery is slot 0/1: teardown + reapply every layout pass. Hypothetical trigger today. CONFIRMED. |
| #5568 | Folder Menus | 1 item | StartTaskbar hook -> `StartRetryThread` -> `RetryLoop::Start` -> `Stop()` waits INFINITE on the UI thread while `PrepareFolderIcons` does Shell icon extraction (only checks `g_unloading`). An unreachable UNC target freezes the taskbar for the SMB timeout, on every rebuild. CONFIRMED. |

## Our replies overstated what we removed

In three PRs the bot found code our reply said was gone:
- #4844: `sio::LoadString`, `ContentAlong`, uncached `Measure`/`Arrange`,
  `ResolveArrangement` + `Arrangement` all still present.
- #4843: `ple::Lease::Abandon/Count/Empty`, `clr::ParseBrush`, the `Anchor`
  overload of `Acquire`, `Metrics::alongDip`.
- #5568: `igc::Acquire` (reply explicitly named it as removed).

Cause: the assembler embeds whole components, and the replies described intent
rather than the diff. The next replies must correct this explicitly. Before any
future reply claims a removal, grep the pushed file for it.

## Cross-mod items (fix once, in the component)

- `bounded-retry`: comment "both of this mod's callers run on the taskbar's UI
  thread" is wrong everywhere (flagged on #4844, #4855, #5568, #5569).
- Dead component API in every adopter. Structural fix: let `assemble.py` drop
  component functions the mod never references, or split optional API into
  separate blocks; add a preflight check for unreferenced component symbols.
- `[[clang::no_destroy]]` on `g_retry`: flagged on #4844 and #5568 (and defended
  on #5569 last round). The audit already accepts `// exit-time-safe:` markers,
  so drop the attribute and mark it instead.

## Optional items worth taking (user-visible behavior)

- #4855: geometry-change rebuild bypasses the vertical stand-down.
- #4844: clamp `FontSize` >= 1 and `CornerRadius` >= 0 (bar vanishes silently);
  `GetWindowTextW` on same-process Explorer windows can freeze the taskbar ->
  `SendMessageTimeoutW`; preview text uses `DEFAULT_GUI_FONT` (tiny on high DPI);
  run `LoadSettings` on the UI thread.
- #5568: remember failed icon targets per settings load (part of item 1).
- #5569: "no placements" exit is settled, and nothing watches for the utility
  appearing later; lone-icon MainStack fallback can double-claim a host.

## Can decline, with a note

- #4443: TCC-hidden line with `%s%` reappears as rows (unlikely combination);
  first-tick width warning (log only).
- #4843: stale Visibility snapshot (only if Windows toggles IconView Visibility;
  not observed); text-callback pointer comment; `g_syntheticGrid` null-check race.
- #5569: mic/location indicator churn (identical result).

## Outcome (2026-09-22)

Every item above, blocking and optional, was taken, plus one found on the way: Folder Menus held its icon mutex across Shell extraction, which the UI thread also takes (a second freeze on the same path). Drafted replies are in replies/. Nothing pushed; awaiting the live test in .agents/outputs/live-test-2026-09-22.md.

