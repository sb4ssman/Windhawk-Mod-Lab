Adds **Taskbar Clock Spacer** (v1.1): a small companion mod for [Taskbar Clock Customization](https://windhawk.net/mods/taskbar-clock-customization) that introduces an elastic `%s%` spacer token for the top/bottom clock line formats. Each `%s%` becomes a gap, and the clock's leftover fixed width is shared out evenly between the gaps — so dense multi-line clocks can justify their items edge-to-edge instead of bunching.

This is deliberately a standalone companion rather than a change to the canonical customizer: the generated-layout approach didn't fit that mod's direction (m417z/my-windhawk-mods#68), so it lives separately, does nothing without Taskbar Clock Customization installed, and leaves that mod completely untouched.

Highlights:

- `%s%` between items in the Top/Bottom line formats: first item hugs the left edge, last hugs the right, gaps share the leftover width evenly; stacked `%s%%s%` weights a gap double.
- `{spacer}` inside the **Weather format** does the same for weather items — it rides through the weather service verbatim (which would consume `%s` as its sunset token) and becomes an identical elastic gap.
- The fixed clock width is taken from Taskbar Clock Customization's own **Max width** setting automatically (read as a setting, never measured — no layout feedback), or from this mod's Max clock width (`maxWidth`); `minSpacerWidth` controls the minimum gap.
- The generated rows are rewritten in place on clock ticks, not rebuilt, so the visual tree stays stable; lines without a spacer token are left completely alone.
- Namespace-scope state stores weak XAML references and integers, with no `no_destroy` attribute. Cleanup unregisters callbacks and restores native blocks on the taskbar UI thread.

Tested live on Windows 11: multi-line top/bottom formats with single and stacked spacers, width pinned exactly to the clock mod's Max width across text changes (no width creep), weather `{spacer}` justification, settings reload, and disable/unload restore.

## Changelog

If this pull request updates an existing mod, describe the changes below:

* v1.0 → v1.1: the branch is rebuilt on current main (the previous branch base was long stale). Fixed a width-feedback defect where the generated rows took their width from a live measurement and the clock could ratchet permanently wider; the width now comes only from settings (including reading the clock mod's own Max width), and the generated panel is pinned to it so lines can never exceed the configured width. Stopped clearing the clock mod's MaxWidth on the shared panel. Added the `{spacer}` weather-format token. In-place per-tick updates replace the old rebuild-per-second behavior.

## Mod authorship

If this pull request introduces a new mod, please complete the section below.

This mod was created by:

- - [ ] The submitter, without AI assistance
- - [x] The submitter, with AI assistance
- - [x] Claude
- - [x] ChatGPT
- - [ ] Gemini
- - [ ] Another AI (please specify):
- - [ ] Other (please specify):
