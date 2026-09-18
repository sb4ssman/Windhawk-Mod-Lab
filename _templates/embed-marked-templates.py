"""Expand explicit template placeholders in a Windhawk single-file source."""
import pathlib
import re
import sys

root = pathlib.Path(__file__).resolve().parent
source = pathlib.Path(sys.argv[1]).resolve()
if source.parent.parent != root.parent or source.suffix != '.cpp':
    raise SystemExit('Expected a mod source directly under this lab')
text = source.read_text(encoding='utf-8')

def expand(match):
    name = match[1]
    template = root / name
    body = template.read_text(encoding='utf-8').replace('#pragma once\n', '', 1).lstrip()
    print(f'Embedding {name}')
    return f'// Embedded from _templates/{name}; verify with verify-template-parity.ps1.\n{body}'

text, count = re.subn(r'^// EMBED_TEMPLATE: ([a-z-]+\.h)$', expand, text, flags=re.M)
if not count:
    raise SystemExit('No template placeholders found; source unchanged')
source.write_text(text, encoding='utf-8', newline='\n')
