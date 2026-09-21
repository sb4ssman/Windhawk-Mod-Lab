<!-- ai-review sha=4ecbcc6a62988aa4d93d5654d7ee8af11306047c -->

### Submission review

_Note: This review was done by Claude. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Remember: The AI reviewer can be wrong - it may misread code, flag correct code as broken, or suggest changes that make things worse. Treat its findings as suggestions to verify, not instructions to follow blindly. You're responsible for the code you submit, so if a finding doesn't hold up, say so instead of changing working code to satisfy it.

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

---

This round landed the big one: the taskbar.dll hook set, the `XamlRoot` walk and the scan thread are gone, and the mod is now the same two system-tray symbols that [taskbar-clock-to-left](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-clock-to-left.wh.cpp#L1617-L1634) uses — `OnApplyTemplate` mandatory, `get_ViewModel` optional — with the same `pThis` layout, `IsLoaded()` gating and module selection. The vendored template code is out, the `no_destroy` on the mutex went with the mutex, the style is re-copied on the fast path, expired states are pruned, the width is capped, and the includes are explicit. I found no correctness or stability defect in the remaining ~1200 lines; the one item left is about how the mod is positioned against the canonical mod.

**1. The README should tell users that Taskbar Clock Customization itself now has a `Justified` alignment, and say concretely what this mod adds on top of it.**

The "Relationship to Taskbar Clock Customization" section (lines 133-139) says the maintainer "preferred an approach that does not add generated layout elements", but stops there. The outcome of that thread ([m417z/my-windhawk-mods PR 68](https://github.com/m417z/my-windhawk-mods/pull/68)) was the maintainer's counter-proposal — `TextAlignment = Justify`, regular spaces where you want elastic gaps, a non-justifying space (U+00A0 / U+2005 / U+2060) inside items — which they demonstrated working and then shipped: TCC v1.8, merged into this repo on 2026-07-01, has `Justify: Justified` in both the time and date `Text alignment` dropdowns ([taskbar-clock-customization.wh.cpp#L378-L385](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-clock-customization.wh.cpp#L378-L385)). So for many users the zero-install answer to "spread my clock items across a fixed width" is already in the mod they have installed, and this README doesn't mention it.

Two things to do, and one thing that isn't for this review to decide:

* Add a short paragraph to the README (and ideally the first lines of `@description`) that names the built-in option — "Taskbar Clock Customization 1.8+ can justify a line on its own: set *Text alignment* to *Justified*, use ordinary spaces for the gaps and a non-breaking space inside items" — so a user can try that first.
* Then state, in the same place, what `%s%` gives that `Justified` does not, in user terms. From the code that's roughly: gaps that are weighted (`%s%%s%`), gaps mixed freely with ordinary spaces without editing in special Unicode spaces, a per-gap `Minimum spacer width`, `{spacer}` inside the weather format, and rows pinned to exactly the configured width. If some of those turn out to be achievable with `Justified` as well, drop them — the list should be the honest residual difference, because it's what justifies a second catalog entry.
* The maintainer has said twice on this PR and on the upstream one that they'd rather extend TCC than merge a companion. Whether the residual difference above is enough for a separate mod is their call, not this review's; the ask here is only that the README gives users (and the maintainer) the facts to make it.

<details><summary>Optional improvements</summary>
<p>

Minor polish — none of this affects users, so it's your call.

* **A few comments still describe code that was removed this round.** Lines 1061-1066 present `TrayUI::StartTaskbar` as the "preferred wait-for-module path" with `LoadLibraryExW` as "a late-load fallback" — there is no `StartTaskbar` hook any more, so the paragraph now describes the mod's *only* late-load path as a fallback to something that doesn't exist. Lines 187-191 and 460-461 still talk about `_templates/visual-tree-walk.h` and "the template's" walk, which aren't in this repo, and lines 1028-1029 are a note to yourself about the CI validator. Each mod here is read as a standalone file, so trim these to what the code actually does.

* **The template scaffolding can be flattened now that the templates are gone.** Three single-use namespaces with aliases (`clock_spacer_tree_walk`/`vtw`, `clock_spacer_settings`/`sio`, `clock_spacer_taskbar`/`tbh`), three forwarding wrappers that exist only to strip the namespace (`FindChildRecursive` at 462-467, `RunFromWindowThread` and `FindCurrentProcessTaskbarWnd` at 912-923), and a `SetExceptionLogger`/`g_logException` indirection (302-318, 1110-1117) whose only ever target is `Wh_Log`. Plain static functions at file scope, with `Wh_Log` called directly in `Invoke`, would be shorter and match how the other taskbar mods read. `sio::LoadInt` is a one-liner used twice.

* **Make `ClearSpacerStates` tolerant of one entry failing (lines 1088-1102).** It runs unregister → restore → remove-panel per entry, and `g_states.clear()` only at the end. If any of those throws for entry *k*, the exception propagates out of the dispatched callback, `Invoke` reports failure, and the retry in `Wh_ModUninit` re-runs the loop from the top — where it hits the same throw again, five times, and then the mod unloads with the property-changed callbacks of entries *k+1…n* still registered on live XAML objects and pointing into the unmapped image. Unlikely (`ClearValue`/`RemoveAt` on live elements don't normally throw), but the cost is a crash on the next clock tick, and the fix is small: wrap each entry's body in its own `try { … } catch (...) {}`, and zero `state.textToken` right after the unregister so a retry never double-unregisters. That also makes the "retry the whole thing" loop genuinely idempotent.

* `Wh_Log(L"Clock Spacer v1.1")` (line 1115) hard-codes the version; `WH_MOD_VERSION` is available and won't drift when you bump `@version`.

</p>
</details>

<details><summary>Functionality notes</summary>
<p>

Non-critical observations and ideas about the feature behavior itself.

* **`CopyTextStyle` doesn't carry TCC's line-height or text-spacing settings, so a multi-line spaced block can have a different vertical rhythm from an unspaced one.** TCC applies `LineHeight` + `LineStackingStrategy::BlockLineHeight` to the source block when *Line height* is set ([TCC#L4361-L4372](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-clock-customization.wh.cpp#L4361-L4372)), and `StackPanel.Spacing` to the shared panel for *Text spacing* ([TCC#L4230-L4235](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-clock-customization.wh.cpp#L4230-L4235)). `CopyTextStyle` (523-533) copies neither `LineHeight` nor `LineStackingStrategy`, and the generated vertical `StackPanel` has no `Spacing` between its rows, so `%n%` lines inside a spaced block stack at the font's natural height while the neighbouring unspaced block honours the user's line height. Copying those two properties and setting `generated.Spacing(parent.Spacing())` would close it. While you're there, it's worth confirming with UWPSpy that the parent's `Spacing` isn't also being applied around the zero-sized, collapsed source block (older XAML builds counted collapsed children) — if it is, a user with *Text spacing* set would see one extra gap.

* **`SetupSpacerForTextBlock` still builds before it records** (868-873, repeat from last round): `UpdateSpacerLine(state)` runs on the local `state` before `push_back`, so if it throws after `InsertAt` the generated panel is in the tree with no `g_states` entry to remove it on unload. Pushing first and updating `g_states.back()` (or wrapping the call) closes it. Very unlikely to fire.

* Worth calling out as correct: the mandatory/optional split on the two system-tray symbols is right; the `get_ViewModel` body matches the reference mods exactly; `%s%` is not a TCC format token, so it survives `FormatLineNoLock` intact (including `%s%%s%` and `%s%%date%`); reading TCC's *Max width* off the shared `StackPanel` as a setting rather than a measurement avoids the width ratchet; `g_states` as a plain vector of `weak_ref`s is the right shape for process shutdown; and every `g_states` access is on the taskbar UI thread (hooks, XAML callbacks, and the two dispatched lambdas), so no lock is needed.

</p>
</details>

---

**Next steps:**

* `/ai-review` - after pushing fixes, to get a review of the updated code. You can repeat this as many times as you need, but each review is thorough and usually there's no need for more than 2-3 iterations.
* `/ready-for-reviewer` - once you're satisfied with the state of the pull request, to hand it over to a human reviewer. If some findings above are left unaddressed, add a short note explaining why.

See the [review process](https://github.com/ramensoftware/windhawk/wiki/Mod-submission-review-process) for details.

Important: Each reply that was generated by AI must be disclosed by including the following text in the reply: "Generated by AI. Model: (ai-model-name)".
