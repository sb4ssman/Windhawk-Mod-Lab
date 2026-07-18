# Agent Guide — Windhawk Mod Lab

This is the canonical instruction file for any development agent (Claude, Codex,
or otherwise) working in this repo. Root `CLAUDE.md` and `AGENTS.md` are identical
pointers to this file. Durable rules live here; current state lives in the
working notes.

## Session start — getting your bearings

NOTHING in these notes is authoritative on its own. Chats get cut off
mid-update, so any single file can be stale. Getting your bearings at the
start of a new chat means cross-checking all of these, not just reading one:

1. Read this file.
2. Start at [working_notes.md](working_notes.md) — current goals, active work,
   key facts.
3. ALWAYS also check [work_log.md](work_log.md) AND git history
   (`git log --oneline`, `git status`, and the PR branches in the fork at
   `t:/Github/sb4ssman/windhawk-mods/` when PR state matters). Where they
   disagree, the newest evidence wins — and git/PR state beats notes.
4. If you need a map of the repo, run `python .agents/tools/generate_folder_map.py`
   (writes to [outputs/folder_structure.md](outputs/folder_structure.md)).
5. Check [../_research/](../_research/) for investigations, design docs, and open
   questions on specific problems.

## Standing directives (do not violate)

- READ EACH MOD CAREFULLY — understand what it does, its context, and its
  interface — BEFORE working on it.
- We are NOT working on interop/placement mechanics right now.
  `_templates/placement-contract.md` is future-design only.
- ABSOLUTELY NO SUBMISSIONS. Prime directive per mod: use `_templates` where
  possible → code works as described/intended → fresh screenshots → sync
  readmes. Only then is submission even discussable.
- The priority is THE CODE WORKING as described and as any normal user would
  expect from the interface — not fork housekeeping, not PR logistics.
- If a shared conversation/paste is truncated, SAY SO and re-read the sources —
  never fill gaps with guesses.

## Notes protocol

- `.agents/` is the home for all agent notes, tools, and generated outputs.
  Do not create per-mod agent-note folders inside mod directories.
- [working_notes.md](working_notes.md) — current state only: active work,
  per-mod status, next steps. Durable policy belongs in this file instead.
- [work_log.md](work_log.md) — completed lab-level milestones, newest context
  at the bottom.
- RECONCILE BEFORE TRUSTING: if working_notes and work_log disagree, the newer
  dated entry wins, and any claim about PR/test/push status must be verified
  against git and the actual PR branches before acting or reporting on it.
  Never tell the user something is "blocked" or "pending" based on notes alone.
- Any session that completes work MUST update working_notes.md before ending —
  stale "Next:" lines that describe already-done work have caused real damage.
- [knowledge/](knowledge/) — older per-mod notes, mod-prefixed filenames.
  Add new per-mod development notes here with a clear mod prefix.
- [tools/](tools/) — helper scripts. [outputs/](outputs/) — generated files.

## Versioning policy

- Major: architecture changes or complete rewrites.
- Minor: new features or significant behavior changes.
- Patch: bug fixes and verified internal lab calibrations.

Convention:

- `X.Y` = published / PR-ready version. Only bump the `@version` header on a PR commit.
- `X.Y.Z` = internal save after a confirmed-working local test. Tracked by git
  commit + tag (`mod/vX.Y.Z`).
- Between internal saves: just commit with a descriptive message, no version bump.
- Archives: leave as-is, stop updating them. Git is the version history.
- Workflow: test → commit internal save (`X.Y.Z`) → when stable and ready to
  PR → bump to `X.Y+1` → PR.

## README / documentation policy

When a mod's status changes, update the root [README.md](../README.md). Each mod
folder must also have its own detailed user-facing `README.md`.

Before any submission or PR update (which requires explicit user approval —
see standing directives), verify all three documentation layers:

1. Root `README.md` — version/status and concise catalog description.
2. The mod folder's `README.md` — complete user-facing documentation.
3. The source file's `// ==WindhawkModReadme==` block — the same content as the
   folder README, except that local asset paths become raw GitHub URLs.

Also verify that `// ==WindhawkModSettings==` names, defaults, accepted tokens,
and descriptions agree with both detailed README copies. Do not submit based on
source compilation alone.

Every visual mod submission needs current screenshots where applicable: at
least one clean, ordinary single-height taskbar example and one useful
double-height/elaborate configuration. The gallery should demonstrate the main
purpose and a few meaningful configurations, not every possible setting.

## Repo shape

Each mod lives in its own lowercase subdirectory. A mod folder may have its own
`archive/` folder for old code experiments, but agent notes belong in `.agents/`.
The current mod inventory and statuses are in the root [README.md](../README.md);
per-mod status detail is in [working_notes.md](working_notes.md).

Shared code templates and checklists live in [../_templates/](../_templates/).

## Windhawk mod basics

- `Wh_ModInit` - called when the mod loads into the target process
- `Wh_ModAfterInit` - called after hooks are applied
- `Wh_ModUninit` - called when the mod unloads; must restore state
- `Wh_ModSettingsChanged` - called when the user saves settings in Windhawk
- `Wh_GetIntSetting` / `Wh_GetStringSetting` / `Wh_FreeStringSetting` - settings access
- `Wh_Log(L"...")` - debug logging
- Settings are declared in the `// ==WindhawkModSettings==` YAML block

## Common patterns across taskbar mods

- `GetTaskbarXamlRoot(HWND)` - hooks `taskbar.dll` symbols to get the XAML root
- `RunFromWindowThread(HWND, proc, param)` - marshal work to the UI thread with a `WH_CALLWNDPROC` hook
- `FindCurrentProcessTaskbarWnd()` - finds `Shell_TrayWnd` for the current process
- `FindChildRecursive(element, predicate)` - recursive XAML tree search
- System tray module discovery should prefer `SystemTray.dll`, fall back to older `Taskbar.View.dll` when appropriate, then `ExplorerExtensions.dll`
- Background retry threads must be stoppable and waited during `Wh_ModUninit`
