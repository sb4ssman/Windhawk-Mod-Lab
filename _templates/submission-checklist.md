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
- [ ] Version header, init log, root catalog, and development notes agree.
- [ ] PR contents are reviewed only after the live checks above pass.
