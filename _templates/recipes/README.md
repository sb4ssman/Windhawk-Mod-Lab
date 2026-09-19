# Recipes

A **component** is working code: a namespace with one contract, assembled into
a mod's single `.wh.cpp` by `_templates/assemble.py`.

A **recipe** is the conceptual half. It says which components a given *shape*
of mod needs, in what order they run, and — the part no component can carry —
what the mod's own body has to do to hold up its end. Components are the nouns;
a recipe is the verb.

Recipes exist because the components cannot tell you the things that actually
break mods. `property-lease` cannot tell you that you must announce a write
*before* making it. `bounded-retry` cannot tell you that "applied" has to mean
the work is finished rather than that the tray was found. Those rules live here.

## The recipes

| Recipe | Shape | Mods on it |
|---|---|---|
| [taskbar-xaml-arranger.md](taskbar-xaml-arranger.md) | Borrow live taskbar XAML elements, position them as an owned group, restore exactly on unload | Tray Utility; OmniButton, Privacy Anchor, VD Switcher, Clock Spacer are the same shape |

## Using one

1. Read the recipe end to end before writing anything. The failure modes it
   lists are ones this lab has actually shipped.
2. Copy its `components.list` into the mod folder and edit the `prefix:`.
3. Put `// ==ModComponents==` / `// ==/ModComponents==` in the source where the
   shared code belongs — after the settings block, before the mod's own state.
4. `python _templates/assemble.py <mod-dir>`.
5. Write the mod body against the recipe's checklist.
6. `_templates/submission-preflight.ps1 <mod-dir>` gates the rest: the region
   must match the library, and every component must have a real call site.

## Adding one

A new recipe earns its place when a second mod needs the same shape. Until
then it is just that one mod's body. Do not write a recipe speculatively —
that is how the old template set grew modes nobody used.
