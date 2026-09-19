# Recipe — taskbar XAML arranger

The shape shared by every visual mod in this family: reach into the live
taskbar's XAML tree, borrow elements Windows owns, position them as one owned
group, and put everything back exactly as it was when the mod unloads.

Windows keeps owning those elements the whole time. That single fact produces
every rule below.

## Components

```
prefix: <mod_prefix>

settings-values
arrangement-expression     # only if the mod takes a layout expression
visual-tree-walk
property-lease
taskbar-window
ui-thread-dispatch
taskbar-xaml-root
taskbar-metrics
bounded-retry
tray-slot-lease            # only if the mod injects into the tray panel
start-lane-placement       # only if the mod offers a Start-adjacent position
```

Take the smallest set. A component with no call site fails preflight, which is
the point — the previous rule was "paste the whole template," and that shipped
hundreds of lines nobody could reach in five separate mods.

## The order things happen

```
Wh_ModInit          LoadSettings, set the exception logger,
                    taskbar_xaml::HookTaskbarSymbols(OnTaskbarRebuilt)
Wh_ModAfterInit     one apply attempt - covers loading into a running taskbar
TrayUI::StartTaskbar  the tree is gone: mark it stale, restart the retry
Wh_ModSettingsChanged dispatch to the UI thread, reload, restart the retry
Wh_ModUninit        stop the retry, then tear down ON the UI thread
```

Every one of those converges on one idempotent `ApplyLayout()`. Do not write a
second path that mutates the tree.

## What the mod body must do

### 1. Stand down before you touch anything

`taskbar_metrics::LayoutModelApplies` first. A vertical taskbar is another
mod's rotated coordinate space; arranging into it paints garbage the user
cannot diagnose. Stand down, say so in the log, and retire the retry — a
settled decision not to act is not the same as "not applied yet", and if you
conflate them the stand-down repeats once per retry attempt.

### 2. Announce every write before you make it

`property_lease::Lease::Track(element, property)` **before** the write, for
every dependency property you set on an element Windows owns. The lease
restores the exact prior *local* value, or clears the property when there was
none.

Clearing is the part people get wrong. A tray element's size and alignment
usually come from its template. Writing a "restored" concrete value where no
local value existed overrides that binding permanently — the user sees it as
the mod having broken their taskbar even after uninstalling.

Track is first-write-wins, so calling it again on a re-apply cannot capture
your own values.

### 3. Own a slot; never share one

`slot_lease::Acquire` / `AcquireAt` put a named zero-size marker in the panel
and hand you the slot. Read the marker back at release time, never the index
you captured at acquire time: other mods inject and remove siblings around you.

If you borrow an element's slot because you are moving that element into your
group, you inherit a free slot. If the element is going to stay where it is,
you must lease a *neighbouring* slot instead, or you put two children in one
column on top of each other.

### 4. "Applied" means finished

`bounded-retry` stops on the first `true` from your applied predicate. If that
predicate means "I found the tray" rather than "the work is done", the retry
retires while the layout is still unresolved and nothing ever fixes it.

Return `true` from a settled stand-down too, and track it in a flag distinct
from "we own live state" — you need both answers and they are not the same
question.

### 5. Keep ownership and staleness apart

Two different questions, two different flags:

- **we own live tree state that must be restored** — set when you apply,
  cleared only by the restore.
- **the tree we captured is gone** — set *only* by the rebuild hook.

Overloading one flag for both is a shipped bug from this family's own history:
a settings save took the abandon path against a live tree, so nothing was
restored, the re-apply could not find the hosts it had already moved, and
disabling the mod afterwards restored nothing at all. That last part breaks
reversibility, which is the one thing a Windhawk mod may not do.

If the retry needs to run an attempt while you still legitimately own live
state — which is exactly what a settings save needs — ask for it with the
force-first-attempt parameter. Do not lie about ownership to wake the loop.

### 6. Teardown is unconditional

Revoke every `LayoutUpdated` token, every property-changed callback and every
`DispatcherTimer` whether or not a restore is possible. They hold pointers into
an image Windhawk is about to free; the next layout pass jumps into unmapped
memory and takes Explorer with it.

So: revoke *before* any "did we apply?" early return, and check what
`dispatch::RunFromWindowThread` returned. It reports whether the callback
actually ran. If it did not, rediscover the taskbar and try once more, then say
so loudly in the log.

### 7. Settings are read on the UI thread

Dispatch the reload into the UI thread, so a layout pass cannot observe a
half-written settings struct. Keep the struct heap-free (fixed buffers, not
`std::wstring`) so it needs no exit-time destructor.

### 8. No logging verbosity setting

`Wh_Log` already expands to `if (InternalWh_IsLogEnabled(...)) { ... }`, so
Windhawk's own per-mod logging switch gates it *and* the arguments are only
evaluated when logging is on. A per-mod verbosity setting therefore saves
nothing, duplicates a control the user already has, and is a thing the
maintainer consistently asks submissions to drop. `submission-preflight.ps1`
rejects one.

If a diagnostic needs real work to *build* its message — a tree walk, a string
join — and that work sits in a helper rather than in the `Wh_Log` argument
list, keep the helper but make sure it runs once per apply, not per frame.
Nothing in this family needs a per-frame diagnostic.

## Failure modes to test before claiming it works

- Change a setting, save repeatedly. The layout must re-apply, and disabling
  the mod afterwards must restore the native taskbar completely.
- Restart Explorer with the mod loaded.
- Disable and re-enable the mod.
- Let Windows hide and show a borrowed element while the mod is applied.
- A second mod injecting into the same panel.
