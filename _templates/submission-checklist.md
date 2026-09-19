# Windhawk Mod Submission Checklist

Use this before opening or updating a submission PR.

## Three documentation layers

- [ ] Root `README.md` has the correct version/status and concise description.
- [ ] The mod folder `README.md` describes the current, tested behavior.
- [ ] `// ==WindhawkModReadme==` matches the folder README. The only expected
      differences are local asset paths versus raw GitHub asset URLs.
- [ ] `// ==WindhawkModSettings==` names, defaults, accepted tokens, and
      descriptions agree with both detailed README copies.
- [ ] Run `_templates/verify-readme-sync.ps1 <mod-folder>` and get
      `README_MATCH`.

## Screenshots

- [ ] Inventory every current image in the mod's `assets/` folder. Include all
      useful screenshots that give users distinct ideas, even when layouts are
      similar; archive or deliberately remove obsolete/unreferenced images.
- [ ] Screenshots show the current version rather than an archived UI.
- [ ] Include a clean single-height taskbar example where the mod is visible.
- [ ] Include a useful double-height or elaborate configuration where applicable.
- [ ] Show the main function and a few meaningful configurations; exhaustive
      setting coverage is unnecessary.
- [ ] Captions say what each screenshot demonstrates.
- [ ] Image links work in both the folder README and embedded Windhawk README.

## Verification

- [ ] Default configuration works.
- [ ] Each newly shared component or setting family has one focused proof case.
- [ ] Settings save/reload works, including returning custom values to native
      system behavior.
- [ ] Main interaction and hit-testing work repeatedly.
- [ ] Explorer restart, mod disable/unload, and re-enable are clean.
- [ ] The source passes the installed Windhawk compile-and-link check, including
      its declared `@compilerOptions` libraries.
- [ ] Run `_templates/exit-time-destructor-audit.ps1 <mod-folder>` and resolve
      every diagnostic. Explorer shutdown may skip `Wh_ModUninit`; necessary
      direct XAML globals use intentional `[[clang::no_destroy]]` lifetime;
      XAML-owning containers use `no_destroy optional<container>` and are reset
      after explicit UI-thread cleanup on controlled unload; heap-only settings
      and leases are not annotated and carry an explicit same-line
      `exit-time-safe: heap-only` audit marker.
- [ ] Run `_templates/submission-preflight.ps1 <mod-folder>` and get
      `SUBMISSION_PREFLIGHT_OK`. This includes the upstream Windhawk PR
      validator; local Clang compilation alone does not cover repository policy.
- [ ] Every `SYMBOL_HOOK` array identifies its target module in the variable
      name (for example, `taskbarDllHooks`) or in the immediately preceding
      module comment, as required by upstream validation.
- [ ] Version header, init log, root catalog, and development notes agree.
- [ ] PR contents are reviewed only after the live checks above pass.

## Review items a script cannot decide

These reached real pull requests. Each needs the call graph or intent to judge,
so they are read by a human rather than enforced by preflight — a check that
cries wolf on correct code teaches everyone to ignore the whole preflight.

- [ ] **No layout handler can re-enter itself.** `UpdateLayout()` runs a
      SYNCHRONOUS layout pass, which raises `LayoutUpdated` again. If a
      `LayoutUpdated` handler can reach `UpdateLayout()` — directly or through
      an apply function — it re-enters while the outer frame is mid-flight,
      and a nested reset can null the very elements the outer frame is still
      writing to. Either deflect through a `DispatcherTimer` (as Tray Utility
      does) or guard the handler with a re-entrancy bool (as OmniButton does).
      Grep alone cannot tell these apart from a safe call in a click handler.
- [ ] **Keep ownership, staleness, and retry demand separate.** An `applied`
      flag means the mod owns a live XAML tree that must be restored. Never
      clear it merely to force a retry after settings change; use a separate
      stale-tree flag for `TrayUI::StartTaskbar` and `RetryLoop`'s forced first
      attempt for a requested reapply.
- [ ] **Every explicit callback has an unconditional unload path.** Revoke
      `LayoutUpdated`, routed-event, property-changed, Loaded, timer, subclass,
      and hook tokens before any `applied`/root-exists early return. Check the
      UI-thread dispatch result and retry/log a failure; a delegate retained by
      Explorer after `FreeLibrary` is a shell crash, not cosmetic state.
- [ ] **Background work is waited by handle, not just counted.** A counter
      decremented inside a worker proc reaches zero before its final return out
      of mod code. Retain handles, wait with a sent-message pump during unload,
      then close them. COM callback objects must be heap-owned with normal
      reference counting, never borrowed from a worker stack frame.
- [ ] **A watcher compares values read from the SAME element it recorded.**
      OmniButton recorded the percentage text from the presenter only when that
      presenter was itself a `TextBlock`, but compared against the probed
      surface's `TextBlock`, which can be several levels deeper. The two never
      matched, so every layout pass triggered a full re-apply — an endless loop
      on the Explorer UI thread. Whenever a cached "last seen" value guards a
      re-apply, confirm the record site and the compare site read one element.
- [ ] **Diagnostics do not cost anything when logging is off.** `Wh_Log` is
      free, but its ARGUMENTS are evaluated regardless. A logging block that
      calls `UpdateLayout()`, `TransformToVisual` or `get_class_name` runs in
      full for every user who never opens the log.
- [ ] **Unbounded recursion over user input has a cost ceiling.** A layout
      expression is user-typed and can nest arbitrarily. Confirm the work is
      linear in node count (nested-group-layout.h memoizes Measure for exactly
      this reason) and that the parser caps nesting depth so deep input is a
      clean error rather than a stack overflow.

## PR construction and CI

- [ ] Before copying a source into the fork, obtain the destination filename
      from the PR branch's existing `git diff --name-only upstream/main...HEAD`
      (or from `@id` for a fresh PR). Do not infer `mods/<id>.wh.cpp` from a
      local lab-folder name: Privacy Anchor's folder and its upstream filename
      intentionally differ.
- [ ] Rebase the submission branch on current `upstream/main` and verify that
      the PR contains exactly one added or modified `.wh.cpp` file.
- [ ] Start from the repository's current pull-request template; keep the
      complete `## Mod authorship` section and accurately select submitter and
      AI-assistance entries. Do not recreate the PR body from memory.
- [ ] After push, verify PR validation and every supported Windhawk compile job
      are green. A PR-description-only edit does not retrigger GitHub Actions;
      if it fixes validation metadata, explicitly rerun the failed workflow or
      push an intentional empty commit.
