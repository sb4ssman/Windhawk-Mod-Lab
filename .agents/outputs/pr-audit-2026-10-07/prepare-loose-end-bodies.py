import json
import subprocess
from pathlib import Path

out = Path(__file__).parent
def body(number):
    return json.loads(subprocess.check_output(['gh', 'pr', 'view', str(number), '--repo', 'ramensoftware/windhawk-mods', '--json', 'body'], text=True, encoding='utf-8'))['body']
clock = body(4443)
clock = clock.replace('Max clock width / Line width override', 'Max clock width (`maxWidth`); `minSpacerWidth` controls the minimum gap')
clock = '\n'.join('''- Namespace-scope state stores weak XAML references and integers, with no `no_destroy` attribute. Cleanup unregisters callbacks and restores native blocks on the taskbar UI thread.''' if line.startswith('- Namespace-scope XAML state') else line.rstrip() for line in clock.splitlines())
clock = clock.replace('- - [ ] ChatGPT', '- - [x] ChatGPT')
(out / '4443-publish-body.md').write_text(clock + '\n', encoding='utf-8')
vd = body(4844)
tail = vd[vd.index('## Mod authorship'):].replace('- - [ ] ChatGPT', '- - [x] ChatGPT')
intro = '''Updates Taskbar Virtual Desktop Switcher to 2.1: clickable desktop buttons with grouped settings, a nestable Arrangement expression, custom labels/indicator symbols and fonts, native checked states, and hover previews. Renamed 1.x settings reset once; the README explains how to copy settings before updating.

Windows 11 supports native horizontal and side taskbars, literal written layouts and screen-axis offsets. The experimental Windows 10 backend uses native tray windows and clock-layout reservation, three tray positions, and compact columns-first automatic grids with an equally sized Task View cell. Windows 10 appearance and exhaustive lifecycle/edge coverage remain experimental limitations.

Latest reviewer findings are addressed: deferred nonblocking taskbar rebuild recovery, UI-thread settings loading, helper/destructor cleanup, clamped typography defaults, DPI-aware preview fonts, and bounded same-process caption reads. The shared edge watcher ignores length-only changes.

### Screenshots

![Default Windows 11 tray placement](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-vd-switcher/assets/simple3.png)

![Windows 11 native side-taskbar grid](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-vd-switcher/assets/win11-side-greek-grid.png)

![Experimental Windows 10 compact grid](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-vd-switcher/assets/win10-experimental-grid.png)

### Tested live

The author live-tested the exact Windows 11 candidate and gave it the seal of approval. The Windows 10 compact-grid checkpoint was separately live-tested and accepted for experimental use. Full local submission preflight, real-font native-layout regression and edge-watch regression passed.

## Changelog

<!-- changelog:start -->
* Added grouped settings and one nestable Arrangement expression; old renamed keys reset once
* Added native side-taskbar layouts and literal written arrangements
* Added experimental Windows 10 tray-window placement and compact automatic grids
* Hardened rebuild, settings, teardown and retry lifecycle handling
* Added DPI-aware preview fonts and bounded same-process caption reads
* Retained configurable labels/symbols/fonts, native checked states and hover previews
<!-- changelog:end -->

'''
(out / '4844-publish-body.md').write_text(intro + '\n'.join(line.rstrip() for line in tail.splitlines()) + '\n', encoding='utf-8')
p = out / '4844-reply-draft.md'
s = p.read_text(encoding='utf-8').replace('Fresh Windows 11 live-test results and the full 2.1 change summary must be added\nbefore posting this draft. The PR still carries the older 2.0 source.', 'The author live-tested this exact Windows 11 candidate and gave it the seal\nof approval. The 2.1 source also includes the separately approved experimental\nWindows 10 tray backend and compact automatic grids; its limitations and\nsettings reset are documented. PR description and screenshots now match 2.1.')
p.write_text(s, encoding='utf-8')
