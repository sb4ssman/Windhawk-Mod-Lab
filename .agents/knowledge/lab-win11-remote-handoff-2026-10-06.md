# Windows 11 remote handoff preserved during Win10 push

Preserved from origin/main d4a3265 while merging the accepted Win10 work.
These are the remote agent's recorded results; reconcile current PR/fork
state before acting on them.

## PUBLISHED Oct 6 — OmniButton 2.1 and Clock Spacer 1.1 (round-6 fixes)

Both live-tested by the user (installed sources == lab, verified by diff).
- #4443 Clock Spacer: pushed `85d9f69d` (round-6 fixes from the Sept 22
  checkpoint), reply posted, `/ai-review` requested.
- #4855 OmniButton: title now v2.1, PR description rewritten for 2.1,
  pushed `f2516de9`, reply (round-6 + 2.1 summary) posted, `/ai-review`
  requested. Posted texts:
  [replies/posted-2026-10-06/](../_research/ai-reviews-2026-09-21/replies/posted-2026-10-06/).
- Next: when the bot reviews land, save them and verify each finding before
  acting. `/ready-for-reviewer` only on a clean round.
- Tree dump: NOT submitted. Test steps + PR draft:
  [outputs/tree-dump-publish.md](outputs/tree-dump-publish.md).
  - Live-tested Oct 6 (user):
    - load dump OK, text hidden;
    - bottom → top → left → bottom each dumped settled, about 3 s apart, same root;
    - JSON valid;
    - unnamed visual state groups now numbered (0 duplicate keys, verified).
  - Fixed after the test, not yet run live: WrapGrid `itemW`/`itemH` NaN
    printed as `-1.#IND`; it now prints `auto` (`ItemSizeText`). Preflight OK.
  - Still untested: Label (expect `…-<label>` in the file name), Subtree
    (`ControlCenterButton`), disable. Unexplained: switching Format wrote a
    `load` dump, not `settings`. Windhawk recorded the settings change at
    17:03:51, and the next dump was at 17:03:57, so the mod reloaded rather
    than calling `Wh_ModSettingsChanged`. Ask the user how they changed it.
  - Fork branch `add-mod-lab-taskbar-tree-dump` (local, unpushed) does NOT
    have the NaN fix yet, and will be renamed anyway (next item).
  - IDENTITY PENDING — user is redesigning; do not publish until settled:
    - The user tried `@id` `mod-lab-tool-taskbar-tree-dump` / "Mod-Lab Tool:
      Taskbar Tree Dump"; that is the build installed in Windhawk now.
    - Then (Oct 6) they described the end structure: `tool-mods/` holding
      flat files `mod-tool-taskbar-tree-dump.wh.cpp`,
      `mod-tool-<next>.wh.cpp` (e.g. a Start-menu probe), one file per
      tool, with no subfolder each.
    - Confirm the exact `@id`/`@name` prefix (`mod-tool-` vs
      `mod-lab-tool-`) with the user, then update
      [mod-identity.md](mod-identity.md).
    - Flat layout breaks `sync-readme.py` (expects `<mod>/README.md` at the
      lab root), `assemble.py`'s per-mod `components.list`, and preflight's
      folder argument. Decide where each tool's README and component list
      live.
    - Then rename everywhere: source, both READMEs, the dump header and JSON
      `"tool"` field, root README tools table, publish doc, fork branch and
      PR title.
- Privacy Anchor, VD Switcher, Folder Menus, Tray Utility 2.1: still need the
  user's live test; their round-6 replies are drafted in
  `_research/ai-reviews-2026-09-21/replies/` (OmniButton's needed rewriting
  for 2.1 — check the others the same way before posting).
- `_research/tree-dumps/` is deliberately uncommitted: the Oct 4 dumps
  contain the user's window titles (Gmail, CalFresh, LinkedIn...). Redact or
  gitignore before ever committing.

