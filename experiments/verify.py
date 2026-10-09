"""Static/runtime checks without opening experiment windows or changing settings."""
from pathlib import Path
import subprocess
import tempfile
import re
root = Path(__file__).resolve().parent.parent
subprocess.run(['python', str(root/'experiments/confirmation-lights/test_checker.py')],check=True)
html = (root/'experiments/stats-dashboard/index.html').read_text(encoding='utf-8')
script = re.search(r'<script>(.*?)</script>', html, re.S).group(1)
with tempfile.TemporaryDirectory() as directory:
    path = Path(directory)/'dashboard.js'
    path.write_text(script,encoding='utf-8')
    subprocess.run(['node','--check',str(path)],check=True)
subprocess.run(['pwsh','-NoProfile','-STA','-File',str(root/'experiments/verify-ui.ps1')],check=True)
print('EXPERIMENT_CHECKS_OK (UI rendering/interaction still needs live review)')
