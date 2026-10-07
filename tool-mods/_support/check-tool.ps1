param([Parameter(Mandatory = $true)][string]$Tool)

# tool-mods/ is flat: tool-mods/<id>.wh.cpp, with its README.md and
# components.list under tool-mods/_support/<id>/. The lab's scripts expect all
# three in one folder, so this stages them in a fresh scratch folder and runs
# them there: assemble (must change nothing), then the submission preflight.
#   pwsh tool-mods/_support/check-tool.ps1 mod-tool-taskbar-tree-dump

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$stage = Join-Path ([IO.Path]::GetTempPath()) ("tool-check-" + [guid]::NewGuid().ToString('N') + "\$Tool")
New-Item -ItemType Directory -Force $stage | Out-Null
Copy-Item (Join-Path $repo "tool-mods\$Tool.wh.cpp") $stage
Copy-Item (Join-Path $PSScriptRoot "$Tool\*") $stage

python (Join-Path $repo '_templates\assemble.py') $stage
if ($LASTEXITCODE -ne 0) { throw "$Tool assembly failed" }
$staged = (Get-Content -Raw (Join-Path $stage "$Tool.wh.cpp")) -replace "`r`n", "`n"
$lab = (Get-Content -Raw (Join-Path $repo "tool-mods\$Tool.wh.cpp")) -replace "`r`n", "`n"
if ($staged -ne $lab) { throw "$Tool is out of date with its components; assemble it" }

& (Join-Path $repo '_templates\submission-preflight.ps1') $stage
