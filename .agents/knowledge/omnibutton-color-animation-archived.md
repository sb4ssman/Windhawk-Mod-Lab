# OmniButton — Animated glyph colors (ARCHIVED FEATURE)

Status: **removed from the mod 2026-07-17 by user direction** ("no animations,
leave it in notes as a possible future addition"). The user wanted something
different from what was built; do not re-add without an explicit design
conversation first.

## Where the working code lives

Commit `2f6ec2c` (and the introducing commit `4e5e347`,
"add independent battery/percent mode, per-glyph colors, and animated colors")
contain the last full implementation in
`omnibutton-customizer/omnibutton-customizer.wh.cpp`. Recover with:

```
git show 2f6ec2c:omnibutton-customizer/omnibutton-customizer.wh.cpp
```

## What it did

- Settings: `wifiColorTo` / `volumeColorTo` / `batteryColorTo` /
  `percentColorTo` (target color per glyph) + `colorAnimateDuration`
  (half-cycle ms, auto-reversed, default 2000).
- Trigger rule: base color (`wifiColor` etc.) AND the matching `*ColorTo` both
  set → the glyph pulses between the two forever. `*ColorTo` alone did nothing.

## Core mechanism (for a future rebuild)

WinRT `Storyboard` + `ColorAnimation` targeting the `Color` property of the
`SolidColorBrush` assigned to the glyph TextBlock's `Foreground`:

```cpp
ColorAnimation anim;
anim.From(fromColor);  anim.To(toColor);
anim.Duration(DurationHelper::FromTimeSpan(TimeSpan{int64_t(durMs) * 10000LL}));
anim.AutoReverse(true);
anim.RepeatBehavior(RepeatBehaviorHelper::Forever());
Storyboard sb;
sb.Children().Append(anim);
Storyboard::SetTarget(anim, brush);          // the brush, not the TextBlock
Storyboard::SetTargetProperty(anim, L"Color");
sb.Begin();
g_activeStoryboards.push_back(sb);           // must Stop() before any cleanup
```

Lifecycle lessons learned (keep for any future animation work):

- Storyboards MUST be stopped before element cleanup (`StopAndClearStoryboards`
  first in every teardown path) or they race with disable/re-template.
- Requires `<winrt/Windows.UI.Xaml.Media.Animation.h>` and the
  `Media::Animation` namespace.
- The brush is the animation target; clearing the TextBlock Foreground while a
  storyboard is running on its brush is the crash/leak hazard.

## Open design question (why it was pulled)

The user wanted a different animation concept than a two-color forever pulse —
what exactly is undecided. Capture requirements before rebuilding.
