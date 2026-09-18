"""Refresh an already-embedded template block in a Windhawk single-file source.

embed-marked-templates.py only expands a `// EMBED_TEMPLATE:` placeholder once.
When a shared template changes afterwards, every adopter still carries the old
copy and verify-template-parity.ps1 fails. This rewrites the existing copy in
place.

Two embed styles exist in the lab:
  * `// Embedded from _templates/<name>; verify with ...` followed by the whole
    template (header comment and includes included).
  * a `// Template block: _templates/<name> vX.Y (verbatim copy - ...)` banner
    followed by the namespace alone, where the mod already has the includes.

Both are handled. The namespace body -- the only thing parity compares -- is
always replaced; a `vX.Y` in a banner is bumped to the template's own version.
"""
import pathlib
import re
import sys

root = pathlib.Path(__file__).resolve().parent
template_name = sys.argv[1]
namespace = 'windhawk_mod_templates::' + template_name[:-2].replace('-', '_')
template = (root / template_name).read_text(encoding='utf-8')
template = template.replace('#pragma once\n', '', 1).lstrip()

version = re.search(r'template v(\d+\.\d+)', template)
if not version:
    raise SystemExit(f'{template_name}: no "template vX.Y" line to read')

ns_pattern = re.compile(
    r'^namespace ' + re.escape(namespace) + r' \{\n.*?^\}\s*//\s*namespace '
    + re.escape(namespace) + r'\n', re.M | re.S)
ns_block = ns_pattern.search(template)
if not ns_block:
    raise SystemExit(f'{template_name}: namespace {namespace} not found')

full_pattern = re.compile(
    r'^// Embedded from _templates/' + re.escape(template_name) + r';[^\n]*\n'
    r'.*?^\}\s*//\s*namespace ' + re.escape(namespace) + r'\n', re.M | re.S)
full_block = (f'// Embedded from _templates/{template_name}; '
              f'verify with verify-template-parity.ps1.\n{template.rstrip()}\n')

banner_pattern = re.compile(
    r'^(// Template block: _templates/' + re.escape(template_name)
    + r' v)\d+\.\d+', re.M)

for arg in sys.argv[2:]:
    source = pathlib.Path(arg).resolve()
    text = source.read_text(encoding='utf-8')

    text, count = full_pattern.subn(lambda _: full_block, text)
    style = 'full'
    if count == 0:
        text, count = ns_pattern.subn(lambda _: ns_block.group(0), text)
        style = 'namespace-only'
        text = banner_pattern.sub(lambda m: m[1] + version[1], text)
    if count != 1:
        raise SystemExit(f'{source.name}: expected 1 embedded block, got {count}')

    source.write_text(text, encoding='utf-8', newline='\n')
    print(f'Refreshed {template_name} ({style}) in {source.name}')
