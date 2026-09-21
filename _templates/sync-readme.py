"""Copy a mod README into its embedded README, expanding local asset links."""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parent.parent
mod = (root / sys.argv[1]).resolve()
if mod.parent != root:
    raise SystemExit('Expected a mod directory directly under the lab')
source, = mod.glob('*.wh.cpp')
readme = (mod / 'README.md').read_text(encoding='utf-8').rstrip()
readme = re.sub(r'\]\((assets/[^)]+)\)',
    lambda m: '](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/'
    + mod.name + '/' + m[1] + ')', readme)
# Links to elsewhere in the lab ("../other-mod/...") resolve to nothing once
# the readme is shown on windhawk.net; point them at the repository instead.
# verify-readme-sync.ps1 maps this prefix back to "../" when comparing.
readme = readme.replace(
    '](../', '](https://github.com/sb4ssman/Windhawk-Mod-Lab/blob/main/')
text = source.read_text(encoding='utf-8')
text, count = re.subn(
    r'// ==WindhawkModReadme==\s*/\*.*?\*/\s*// ==/WindhawkModReadme==',
    lambda _: '// ==WindhawkModReadme==\n/*\n' + readme
    + '\n*/\n// ==/WindhawkModReadme==', text, flags=re.S)
if count != 1:
    raise SystemExit(f'Expected exactly one README block, found {count}')
source.write_text(text, encoding='utf-8', newline='\n')
print(f'{mod.name} README synchronized')
