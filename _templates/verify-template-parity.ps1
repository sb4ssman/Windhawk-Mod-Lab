param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$ModDirectory
)

# Windhawk mods are single-file, so every shared template is embedded as a
# verbatim copy of its namespace body. Copies drift. This compares each
# embedded block against its source and fails on any difference, so a mod can
# never quietly carry a stale or locally-patched template.
#
# A mod that does not embed a given template is simply skipped — not every mod
# needs every component. Only a MISMATCH fails.

$ErrorActionPreference = 'Stop'
$modPath = (Resolve-Path -LiteralPath $ModDirectory).Path
$modName = Split-Path -Leaf $modPath
$templatesPath = Split-Path -Parent $PSCommandPath

$sourceFile = Get-ChildItem -LiteralPath $modPath -Filter '*.wh.cpp' |
    Select-Object -First 1
if (-not $sourceFile) { throw "No .wh.cpp source found in $modPath" }

$templates = @(
    @{ File = 'nested-group-layout.h';  Ns = 'nested_group_layout' }
    @{ File = 'native-glyph-surface.h'; Ns = 'native_glyph_surface' }
    @{ File = 'os-setting-bridge.h';    Ns = 'os_setting_bridge' }
    @{ File = 'property-lease.h';       Ns = 'property_lease' }
    @{ File = 'settings-io.h';          Ns = 'settings_io' }
    @{ File = 'color-tokens.h';         Ns = 'color_tokens' }
    @{ File = 'taskbar-host.h';         Ns = 'taskbar_host' }
    @{ File = 'visual-tree-walk.h';     Ns = 'visual_tree_walk' }
    @{ File = 'button-surface.h';       Ns = 'button_surface' }
    @{ File = 'injected-grid-column.h'; Ns = 'injected_grid_column' }
    @{ File = 'start-placement.h';      Ns = 'start_placement' }
)

$modSource = (Get-Content -Raw -LiteralPath $sourceFile.FullName) -replace "`r`n", "`n"
$problems = [System.Collections.Generic.List[string]]::new()
$matched = 0
$skipped = 0

foreach ($template in $templates) {
    $templateFile = Join-Path $templatesPath $template.File
    if (-not (Test-Path -LiteralPath $templateFile)) { continue }

    $fullNs = 'windhawk_mod_templates::' + $template.Ns
    $escaped = [regex]::Escape($fullNs)
    $pattern = '(?s)namespace ' + $escaped + ' \{\n(.*)\n\}\s*// namespace ' + $escaped

    $inMod = [regex]::Match($modSource, $pattern)
    if (-not $inMod.Success) { $skipped++; continue }

    $templateSource = (Get-Content -Raw -LiteralPath $templateFile) -replace "`r`n", "`n"
    $inTemplate = [regex]::Match($templateSource, $pattern)
    if (-not $inTemplate.Success) {
        $problems.Add("TEMPLATE_UNPARSEABLE: $($template.File) has no '$fullNs' body")
        continue
    }

    if ($inTemplate.Groups[1].Value.TrimEnd() -cne $inMod.Groups[1].Value.TrimEnd()) {
        $problems.Add("TEMPLATE_DRIFT: embedded $($template.File) differs from _templates/")
    } else {
        $matched++
    }
}

if ($problems.Count -gt 0) {
    foreach ($problem in $problems) { Write-Output "$modName $problem" }
    Write-Error "$modName TEMPLATE_PARITY_FAILED"
    exit 1
}

Write-Output "$modName TEMPLATE_PARITY_OK ($matched embedded, $skipped not used)"
