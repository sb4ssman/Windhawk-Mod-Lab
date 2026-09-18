"""One-time, checked migration of Folder Menus' flat settings to family groups."""
from pathlib import Path
import re

path = Path('taskbar-folder-menus/taskbar-folder-menus.wh.cpp')
source = path.read_text(encoding='utf-8')
start = source.index('// ==WindhawkModSettings==')
end = source.index('// ==/WindhawkModSettings==', start)
block = source[start:end]
records = dict(re.findall(r'^- (\w+):(.*?)(?=^- |\n\*/)', block, re.M | re.S))
groups = {
    'Placement': [('position', 'Position')],
    'Content': [('folders', 'Folders'), ('buttonText', 'DefaultLabel')],
    'Layout': [],
    'Size': [('buttonWidth', 'ItemWidth'), ('buttonHeight', 'ItemHeight'), ('buttonSpacing', 'ItemSpacing')],
    'Adjust': [('groupOffsetX', 'OffsetX'), ('groupOffsetY', 'OffsetY')],
    'Surface': [(key, key[0].upper() + key[1:]) for key in (
        'fontSize', 'textColor', 'backgroundColor', 'hoverBackgroundColor',
        'pressedBackgroundColor', 'borderColor', 'borderThickness',
        'cornerRadius', 'opacity', 'shineEffect')],
    'Behavior': [('maxMenuItems', 'MaxMenuItems'), ('maxDepth', 'MaxDepth'), ('showHidden', 'ShowHidden')],
}
layout = '''  - Arrangement: auto
    $name: Arrangement
    $description: "auto fits the taskbar height. Use 1 | 2 for a row, 1, 2 for a column; parentheses nest groups. Numbers follow the Folders list."
  - FillOrder: rows
    $name: Fill order
    $options:
    - rows: Rows first
    - columns: Columns first
  - Justify: center
    $name: Cross-axis alignment
    $options:
    - start: Start
    - center: Center
    - end: End
  - NewItems: append
    $name: Unlisted folders
    $options:
    - append: Append automatically
    - hide: Hide
'''
parts = []
for group, fields in groups.items():
    lines = f'- {group}:\n'
    if group == 'Layout':
        lines += layout
    if group == 'Adjust':
        lines += '  - PadX: 0\n    $name: Horizontal padding (DIP)\n  - PadY: 0\n    $name: Vertical padding (DIP)\n'
    for old, new in fields:
        record = '- ' + new + ':' + records[old].rstrip() + '\n'
        if old == 'position':
            record += '  - "afterTaskbarIcons": "After pinned/running app icons"\n'
        if old == 'folders':
            record = record.replace('label:', 'Label:').replace('target:', 'Target:')
            record = record.replace('  - - Label: "⚙"',
                '    - UseDefaultIcon: false\n      $name: Use native Shell icon\n      $description: Show the target icon instead of the label; fall back to the label if unavailable.\n  - - Label: "⚙"')
            record = record.replace('    - Target: "shell:ControlPanelFolder"',
                '    - Target: "shell:ControlPanelFolder"\n    - UseDefaultIcon: false')
        if old == 'buttonText':
            record = '- DefaultLabel: "📁"\n  $name: Default button label\n  $description: Used when a folder label is empty.\n'
        record = record.replace('(px)', '(DIP)').replace('(pt)', '(DIP)')
        lines += ''.join('  ' + line + '\n' for line in record.rstrip().splitlines())
    parts.append(lines + f'  $name: {group}\n')
source = source[:start] + '// ==WindhawkModSettings==\n/*\n' + '\n'.join(parts) + '\n*/\n' + source[end:]
for group, fields in groups.items():
    for old, new in fields:
        source = source.replace(f'L"{old}"', f'L"{group}.{new}"')
source = source.replace('L"folders[%d].target"', 'L"Content.Folders[%d].Target"')
source = source.replace('L"folders[%d].label"', 'L"Content.Folders[%d].Label"')

def cut(first, after):
    global source
    a = source.index(first)
    b = source.index(after, a)
    source = source[:a] + source[b:]

cut('// Smart-grid template:', '// Embedded from _templates/settings-io.h;')
cut('    grid::GridMode gridMode', '    std::wstring buttonText')
cut('    int gridColumns = 0;', '    int buttonWidth = 24;')
cut('    std::wstring gridMode = GetStringSetting', '    g_settings.buttonText =')
cut('    g_settings.gridColumns =', '    g_settings.buttonWidth =')
source = re.sub(r'^    (?:int groupPadding(?:Left|Right) = 0;|g_settings.groupPadding(?:Left|Right) = .*;)\n', '', source, flags=re.M)
# Shared surface implementation replaces the old local copy.
cut('static Brush ParseColorBrush(', 'static grid::Config MakeFolderGridConfig(')
cut('static grid::Config MakeFolderGridConfig(', 'static Grid BuildFolderButtonGrid(')
path.write_text(source, encoding='utf-8', newline='\n')
print('Grouped settings; removed superseded layout and surface implementations.')
