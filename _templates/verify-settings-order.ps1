param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$ModDirectory
)

# Machine-checks a mod's ==WindhawkModSettings== block against the settings
# contract in settings-profiles.md: the eight canonical groups, in the fixed
# assembly order, with canonical keys named and ordered as declared. A group
# may append mod-specific keys AFTER its canonical ones; it may not rename,
# reorder, or interleave them.
#
# Prints SETTINGS_ORDER_OK, or one line per violation and exits 1.

$ErrorActionPreference = 'Stop'
$modPath = (Resolve-Path -LiteralPath $ModDirectory).Path
$modName = Split-Path -Leaf $modPath
$sourceFile = Get-ChildItem -LiteralPath $modPath -Filter '*.wh.cpp' |
    Select-Object -First 1

if (-not $sourceFile) { throw "No .wh.cpp source found in $modPath" }

# Canonical assembly order. Keep in lockstep with settings-profiles.md.
$canonicalGroups = @(
    'Placement', 'Content', 'Layout', 'Size',
    'Adjust', 'Surface', 'State', 'Behavior'
)

# Canonical keys per group, in order. Content and Behavior are mod-defined by
# design, so they carry no required keys. A mod may omit a canonical key it
# does not need; the ones it does declare must keep this relative order.
$canonicalKeys = @{
    Placement = @('Position', 'AllTaskbars')
    Content   = @()
    Layout    = @('Arrangement', 'FillOrder', 'Justify', 'NewItems')
    Size      = @('ItemWidth', 'ItemHeight', 'ItemSize', 'ItemSpacing')
    Adjust    = @('PadX', 'PadY', 'OffsetX', 'OffsetY')
    Surface   = @('FontSize', 'FontFamily', 'TextColor', 'BackgroundColor',
                  'HoverBackgroundColor', 'PressedBackgroundColor',
                  'BorderColor', 'BorderThickness', 'CornerRadius', 'Opacity',
                  'ShineEffect', 'IdleOpacity', 'ActiveOpacity', 'GlowEnabled',
                  'GlowOpacity', 'SlashColor', 'SlashDirection', 'SlashOpacity')
    State     = @('ActiveTextColor', 'InactiveTextColor',
                  'ActiveBackgroundColor', 'InactiveBackgroundColor',
                  'ActiveBold')
    Behavior  = @()
}

$source = Get-Content -Raw -LiteralPath $sourceFile.FullName
$match = [regex]::Match(
    $source,
    '(?s)// ==WindhawkModSettings==\s*/\*\r?\n(.*?)\r?\n\*/\s*// ==/WindhawkModSettings=='
)
if (-not $match.Success) {
    throw "WindhawkModSettings block not found in $($sourceFile.Name)"
}

$lines = $match.Groups[1].Value -split "`r?`n"
$problems = New-Object System.Collections.Generic.List[string]

# Top-level entries are "- Key:" or "- Key: value" at column 0. A group is a
# top-level entry whose value is empty and which is followed by indented
# sub-entries.
$groups = New-Object System.Collections.Generic.List[object]
$flatKeys = New-Object System.Collections.Generic.List[string]
$current = $null

foreach ($line in $lines) {
    if ($line -match '^- ([A-Za-z0-9_]+):\s*(.*)$') {
        $name = $Matches[1]
        $value = $Matches[2]
        if ($value -eq '') {
            $current = [pscustomobject]@{
                Name = $name
                Keys = New-Object System.Collections.Generic.List[string]
            }
            $groups.Add($current)
        } else {
            $current = $null
            $flatKeys.Add($name)
        }
        continue
    }
    if ($current -and $line -match '^  - ([A-Za-z0-9_]+):') {
        $current.Keys.Add($Matches[1])
    }
}

# A top-level entry with no children is a flat setting that used the group
# spelling, not a group. Partition rather than mutate while enumerating.
$nested = New-Object System.Collections.Generic.List[object]
foreach ($group in $groups) {
    if ($group.Keys.Count -eq 0) {
        $flatKeys.Add($group.Name)
    } else {
        $nested.Add($group)
    }
}
$groups = $nested

if ($groups.Count -eq 0) {
    $problems.Add("NOT_GROUPED: no nested settings groups found; this mod has not adopted the settings contract")
} else {
    foreach ($key in $flatKeys) {
        $problems.Add("UNGROUPED_KEY: '$key' sits outside any group")
    }
}

# Groups must be known and must appear in canonical order.
$seenOrder = New-Object System.Collections.Generic.List[int]
foreach ($group in $groups) {
    $index = $canonicalGroups.IndexOf($group.Name)
    if ($index -lt 0) {
        $problems.Add("UNKNOWN_GROUP: '$($group.Name)' is not one of: $($canonicalGroups -join ', ')")
        continue
    }
    $seenOrder.Add($index)
}
for ($i = 1; $i -lt $seenOrder.Count; $i++) {
    if ($seenOrder[$i] -le $seenOrder[$i - 1]) {
        $earlier = $canonicalGroups[$seenOrder[$i - 1]]
        $later = $canonicalGroups[$seenOrder[$i]]
        $problems.Add("GROUP_ORDER: '$later' must come before '$earlier'")
    }
}

# Within a group, the canonical keys it uses must keep their canonical order,
# and mod-specific keys must all come after the last canonical one.
foreach ($group in $groups) {
    if (-not $canonicalKeys.ContainsKey($group.Name)) { continue }
    $expected = $canonicalKeys[$group.Name]
    if ($expected.Count -eq 0) { continue }

    $lastIndex = -1
    $sawExtra = $false
    foreach ($key in $group.Keys) {
        $index = $expected.IndexOf($key)
        if ($index -lt 0) {
            $sawExtra = $true
            continue
        }
        if ($sawExtra) {
            $problems.Add("KEY_PLACEMENT: $($group.Name).$key is canonical but follows a mod-specific key")
        }
        if ($index -le $lastIndex) {
            $problems.Add("KEY_ORDER: $($group.Name).$key is out of canonical order")
        }
        $lastIndex = $index
    }
}

# Retired keys must not reappear anywhere, in any spelling.
$retired = @('gridMode', 'smartLayout', 'layoutMode', 'primaryAxis',
             'crossAlign', 'shortGroupAlign', 'shortGroupPosition',
             'buttonRows', 'buttonColumns', 'gridRows', 'gridColumns',
             'paddingLeft', 'paddingRight', 'paddingTop', 'paddingBottom',
             'groupPaddingLeft', 'groupPaddingRight', 'groupPaddingTop',
             'groupPaddingBottom', 'gridVerticalOffset', 'nudge',
             'contentNudge')
foreach ($group in $groups) {
    foreach ($key in $group.Keys) {
        if ($retired -contains $key) {
            $problems.Add("RETIRED_KEY: $($group.Name).$key was retired by the settings contract")
        }
    }
}

if ($problems.Count -gt 0) {
    foreach ($problem in $problems) { Write-Output "$modName $problem" }
    Write-Error "$modName SETTINGS_ORDER_FAILED"
    exit 1
}

Write-Output "$modName SETTINGS_ORDER_OK"
