# Confirmation lights / DCG stoplight experiment

Run `pwsh -NoProfile -STA -File experiments/confirmation-lights/start.ps1`.
Add `-DcgOnly` for a single DCG stoplight. Requires Python 3.10+ and Windows;
TOML expectation comparisons require Python 3.11+. PowerShell 7 is recommended;
the app does not change execution policy if your shell refuses local scripts.
No installation, startup registration, automatic repairs, or guard exceptions.

The tray circle is green when checks pass, yellow while checking or when a
provider reports work in progress, red on failed expectations (including stale
reports or a DCG version below your configured minimum), and gray when evidence
cannot be read. The context menu selects and saves colors. Close hides the panel;
Exit stops the app. Edit `panel.json` to define any number of detectors.

Three initial slots are GUARD, BACKUP, LINK. The latter two are deliberately
unknown until you supply real reports; no example is silently represented as
healthy. Write reports atomically to `evidence/backup.json` or `evidence/link.json`:
`{"state":"complete","checkedAt":"2026-10-09T18:00:00Z"}`.
Link uses connected/connecting/disconnected. Backup uses complete/running/failed.

JSON checks use dotted `field` paths, ordered equality rules, and optional
timezone-bearing `timestampField` / `maxAgeSeconds`. `file` checks test existence
and modification age. Relative paths resolve beside the config; environment
variables are expanded. Missing/invalid/unmatched reports are unknown.

DCG runs read-only `doctor --format json --strict`. Optional `executable`,
`minimumVersion`, `expectedConfig`, `actualConfig`, and `sections` compare your
selected TOML sections. Section comparison ignores formatting/comments but is
otherwise exact (including list order); it is not a full DCG effective-config
resolver. Diagnostics passing does not attest a running agent's interception.
The app never updates DCG, edits its config, installs hooks, or grants exceptions.

CLI: `python experiments/confirmation-lights/checker.py --config panel.json`.
The Windows host executes checks asynchronously to keep menus responsive.
