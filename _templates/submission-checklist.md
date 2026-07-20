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
- [ ] The source passes the installed Windhawk compiler syntax check.
- [ ] Run `_templates/exit-time-destructor-audit.ps1 <mod-folder>` and resolve
      every diagnostic. Explorer shutdown may skip `Wh_ModUninit`; necessary
      non-trivial globals use intentional `[[clang::no_destroy]]` lifetime,
      while controlled unload still performs explicit UI-thread cleanup.
- [ ] Run `_templates/submission-preflight.ps1 <mod-folder>` and get
      `SUBMISSION_PREFLIGHT_OK`. This includes the upstream Windhawk PR
      validator; local Clang compilation alone does not cover repository policy.
- [ ] Every `SYMBOL_HOOK` array identifies its target module in the variable
      name (for example, `taskbarDllHooks`) or in the immediately preceding
      module comment, as required by upstream validation.
- [ ] Version header, init log, root catalog, and development notes agree.
- [ ] PR contents are reviewed only after the live checks above pass.

## PR construction and CI

- [ ] Rebase the submission branch on current `upstream/main` and verify that
      the PR contains exactly one added or modified `.wh.cpp` file.
- [ ] Start from the repository's current pull-request template; keep the
      complete `## Mod authorship` section and accurately select submitter and
      AI-assistance entries. Do not recreate the PR body from memory.
- [ ] After push, verify PR validation and every supported Windhawk compile job
      are green. A PR-description-only edit does not retrigger GitHub Actions;
      if it fixes validation metadata, explicitly rerun the failed workflow or
      push an intentional empty commit.
