# Taskbar Placement Contract

Status: design reference only. No universal placement host, shared root, lease
broker, or cross-mod placement implementation is being introduced in the
current refactor pass.

This is the design boundary for a future universal placement implementation.
It is deliberately a contract first: today the mods use three different kinds
of ownership, and pretending they are interchangeable would make cleanup and
hit testing less reliable.

## Placement owners

| Owner kind | Current mods | Meaning |
|---|---|---|
| Injected group | VD Switcher, Folder Menus, Privacy Anchor | The mod owns a named root and usually a dedicated tray-grid column |
| Native host mutation | OmniButton, Clock Spacer | The mod changes Windows-owned elements in place and must restore snapshots |
| Native host relocation | Tray Utility Customizer | The mod moves several Windows-owned hosts and owns a reversible column lease |
| Start overlay extension | VD Switcher | The mod overlays or reserves space around Start; this is not a tray-column placement |

## Canonical request

The eventual placement adapter should consume a request shaped like this:

```cpp
enum class TaskbarAnchor {
    BeforeIcons,
    BeforeOmni,
    BeforeClock,
    AfterClock,
    AfterShowDesktop,
    LeftOfStart,
    OverStart,
    RightOfStart,
};

enum class PlacementOwnership {
    InjectedRoot,
    NativeMutation,
    NativeRelocation,
};

struct TaskbarPlacementRequest {
    TaskbarAnchor anchor;
    PlacementOwnership ownership;
    double desiredWidth;
    double desiredHeight;
    double offsetX;
    double offsetY;
    int zIndex;
    const wchar_t* ownerName;  // Globally distinctive XAML marker/root name.
};
```

Not every owner supports every anchor. Unsupported combinations must fail and
log; they must not silently fall back to column zero.

## Placement lease

Every successful mutation returns or records a lease containing enough state
to reverse itself:

- live parent/root identity
- marker or named injected root
- inserted column index discovered from the live marker
- original columns and spans for relocated native hosts
- original margin, size, alignment, transform, visibility, and z-index values
- event/revoker tokens installed by the placement
- owner kind and anchor

Cleanup uses live discovery first and cached object references second. A cached
column number alone is not a lease because Windows can rebuild or renumber the
tray grid.

## Inter-mod rules

1. Each mod owns only its distinct named root or marker.
2. A mod must never remove another mod's column based only on a stale index.
3. Insertions shift existing starts and widen spans crossing the insertion.
   Removal performs the exact inverse.
4. Existing named roots are re-acquired instead of duplicated.
5. Overlay roots use explicit z-index and hit-test policy.
6. Native mutations snapshot before the first write and restore once.
7. Placement changes run on the taskbar window thread.
8. Explorer rebuild means reacquire and reapply; unload means restore and revoke.

## Shared-root protocol candidate

A later generation can reduce column contention by allowing multiple mods to
reuse one named host such as `Sb4ssmanTaskbarExtensionsHost`, with child roots
named by mod id. That requires:

- deterministic child ordering
- a shared layout vocabulary
- reference-free discovery (separate DLLs cannot safely share C++ globals)
- last-child cleanup that removes the shared column only when empty
- per-child hit-test and z-index isolation

Until that protocol is implemented and tested, use one reversible marked column
per independently injected mod.
