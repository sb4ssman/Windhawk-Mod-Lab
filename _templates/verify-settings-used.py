"""Every setting a mod declares must actually be read by its code.

Learned from the AI review on PR #4855, which found `Placement.Status`: a
declared text box that no code read, presented to the user purely as a note.
Windhawk has no read-only control, so a user who edits such a box gets no
feedback and no effect — the settings surface promises something it does not
do. Explanations belong in the README, which has room for them.

The same check catches the more dangerous version of this bug: a setting that
was RENAMED in the settings block but whose old key is still what the loader
asks for. Windhawk answers an unknown key with a default instead of failing, so
the control silently stops working. That already cost this lab a release (the
Indicator symbols regression behind settings-io.h's LoadChoice).

Usage:  python verify-settings-used.py <mod.wh.cpp> [...]
Exit 0 and print "<name> SETTINGS_ALL_READ_OK", or exit 1 naming the orphans.
"""
import pathlib
import re
import sys

SETTINGS_BLOCK = re.compile(
    r'==WindhawkModSettings==(?P<body>.*?)==/WindhawkModSettings==', re.S)
# "  - Name:" with nothing after it is a group; with a value it is a setting.
ENTRY = re.compile(r'^(?P<indent>\s*)-\s+(?P<dashes>(?:-\s+)*)'
                   r'(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*:(?P<rest>.*)$')


def collect_settings(body):
    """Yield (dotted_path, is_under_array) for every leaf setting declared."""
    stack = []  # (indent, name, is_array)
    leaves = []
    # An "$options:" list holds VALUES, not settings. Written unquoted they look
    # exactly like settings ("- rows: Rows first"), so they must be skipped or
    # every option is reported as an unread setting.
    options_indent = None
    for line in body.splitlines():
        stripped = line.strip()
        if stripped.startswith('$'):
            if stripped.startswith('$options:'):
                options_indent = len(line) - len(line.lstrip())
            continue
        match = ENTRY.match(line)
        if not match:
            continue
        indent = len(match.group('indent'))
        if options_indent is not None:
            if indent >= options_indent:
                continue
            options_indent = None
        name = match.group('name')
        has_value = match.group('rest').strip() != ''
        # "- - Label:" marks the first field of a list ITEM, so its parent is
        # an array of records rather than a plain group.
        in_array_item = match.group('dashes') != ''

        while stack and stack[-1][0] >= indent and not in_array_item:
            stack.pop()

        if has_value:
            prefix = [entry[1] for entry in stack]
            leaves.append(('.'.join(prefix + [name]), prefix, name))
        else:
            stack.append((indent, name, False))
    return leaves


def referenced(code, path, prefix, leaf):
    """Whether the mod's code actually asks Windhawk for this setting."""
    if f'"{path}"' in code or f"'{path}'" in code:
        return True
    # Array settings are read with an index format: "Content.Folders[%d].Label".
    if prefix:
        array_form = '.'.join(prefix) + r'[%d].' + leaf
        if f'"{array_form}"' in code:
            return True
        # Some mods index the group itself: "Content.Folders[%d]".
        if f'"{".".join(prefix)}[%d]' in code and f'.{leaf}"' in code:
            return True
    return False


def main(argv):
    failures = 0
    for arg in argv:
        source = pathlib.Path(arg).resolve()
        text = source.read_text(encoding='utf-8')
        block = SETTINGS_BLOCK.search(text)
        if not block:
            print(f'{source.name}: no settings block found')
            failures += 1
            continue
        body = block.group('body')
        code = text[block.end():]

        orphans = []
        for path, prefix, leaf in collect_settings(body):
            if not referenced(code, path, prefix, leaf):
                orphans.append(path)

        if orphans:
            failures += 1
            print(f'{source.name}: declared but never read by the mod: '
                  + ', '.join(sorted(orphans)))
            print('  A setting the code never reads is a control that does '
                  'nothing when the user changes it.')
            print('  Either read it, or remove it and put the explanation in '
                  'the README.')
        else:
            print(f'{source.stem.replace(".wh", "")} SETTINGS_ALL_READ_OK')
    return 1 if failures else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
