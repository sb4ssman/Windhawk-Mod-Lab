"""Read-only confirmation checks. Python 3.10+, no third-party packages."""
import argparse
import datetime as dt
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import time

STATES = {'good', 'working', 'bad', 'unknown'}

def result(check, state, reason):
    return {'id': check['id'], 'label': check.get('label', check['id']),
            'state': state, 'reason': reason}

def resolve(path, base):
    path = Path(os.path.expandvars(os.path.expanduser(path)))
    return path if path.is_absolute() else base / path

def field(data, key):
    for part in key.split('.') if key else []:
        data = data[int(part)] if isinstance(data, list) else data[part]
    return data

def version(text):
    match = re.search(r'\b(\d+)\.(\d+)\.(\d+)\b', text)
    if not match:
        raise ValueError('Cannot read semantic version')
    return tuple(map(int, match.groups()))

def run(args, timeout=8):
    return subprocess.run(args, capture_output=True, text=True, timeout=timeout,
                          creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))

def check_dcg(check, base):
    binary = str(resolve(check['executable'], base)) if 'executable' in check else 'dcg'
    doctor = run([binary, 'doctor', '--format', 'json', '--strict'])
    if not doctor.stdout.strip():
        return result(check, 'bad', 'DCG cannot supply diagnostics: ' + doctor.stderr.strip()[:240])
    data = json.loads(doctor.stdout)
    if not isinstance(data, dict) or not isinstance(data.get('checks', []), list):
        return result(check, 'unknown', 'DCG supplied an unexpected diagnostic schema')
    if any(not isinstance(c, dict) for c in data.get('checks', [])):
        return result(check, 'unknown', 'DCG supplied malformed diagnostic checks')
    if doctor.returncode or data.get('ok') is not True:
        failed = [c for c in data.get('checks', []) if c.get('status') == 'error']
        unreadable = any('access is denied' in c.get('message', '').lower() or
                         'permission denied' in c.get('message', '').lower() for c in failed)
        names = ', '.join(c.get('name', 'unnamed check') for c in failed)
        return result(check, 'unknown' if unreadable else 'bad',
                      ('Diagnostics cannot read required evidence: ' if unreadable else 'DCG checks failed: ') + (names or 'inspect doctor output'))
    if 'minimumVersion' in check:
        current = run([binary, '--version'])
        if current.returncode or version(current.stdout) < version(check['minimumVersion']):
            return result(check, 'bad', 'Installed DCG is below the expected minimum version')
    if 'expectedConfig' in check:
        try:
            import tomllib
        except ImportError:
            return result(check, 'unknown', 'TOML expectations require Python 3.11 or newer')
        expected = tomllib.loads(resolve(check['expectedConfig'], base).read_text(encoding='utf-8-sig'))
        actual = tomllib.loads(resolve(check['actualConfig'], base).read_text(encoding='utf-8-sig'))
        # Compare effective intent in the sections explicitly selected by the user.
        # TOML formatting and comments do not affect this comparison.
        for section in check.get('sections', ['packs', 'policy', 'heredoc', 'overrides']):
            if field(expected, section) != field(actual, section):
                return result(check, 'bad', 'Configuration differs from expectation: ' + section)
    return result(check, 'good', 'DCG diagnostics pass; selected expectations match. Running-session interception is not attested.')

def evaluate(check, base, now=None):
    now = time.time() if now is None else now
    try:
        kind = check['type']
        if kind == 'dcg':
            return check_dcg(check, base)
        path = resolve(check['path'], base)
        if not path.is_file():
            return result(check, 'unknown', 'Expected evidence file is missing')
        if path.stat().st_size > 1024 * 1024:
            return result(check, 'unknown', 'Evidence exceeds the 1 MiB read limit')
        if 'maxAgeSeconds' in check and now - path.stat().st_mtime > check['maxAgeSeconds']:
            return result(check, 'bad', 'Evidence file is stale')
        if kind == 'file':
            return result(check, 'good', 'Evidence file exists and meets freshness expectation')
        if kind != 'json':
            return result(check, 'unknown', 'Unsupported detector type: ' + kind)
        data = json.loads(path.read_text(encoding='utf-8-sig'))
        if 'timestampField' in check:
            stamp = dt.datetime.fromisoformat(str(field(data, check['timestampField'])).replace('Z', '+00:00'))
            if stamp.tzinfo is None:
                raise ValueError('Report timestamp must include a timezone')
            age = now - stamp.timestamp()
            if age < -60:
                return result(check, 'unknown', 'Report timestamp is in the future')
            if age > check.get('maxAgeSeconds', 300):
                return result(check, 'bad', 'Report timestamp is stale')
        value = field(data, check.get('field', 'state'))
        for rule in check.get('rules', []):
            if value == rule['equals']:
                state = rule['state']
                if state not in STATES:
                    raise ValueError('Invalid rule state')
                return result(check, state, rule.get('reason', 'Matched configured expectation'))
        return result(check, 'unknown', 'No rule matches the reported value')
    except subprocess.TimeoutExpired:
        return result(check, 'unknown', 'Checker timed out; no healthy result assumed')
    except (OSError, ValueError, KeyError, TypeError, IndexError) as exc:
        return result(check, 'unknown', str(exc)[:280])

def evaluate_panel(config, base):
    checks = config['checks']
    if len({c['id'] for c in checks}) != len(checks):
        raise ValueError('Check identifiers must be unique')
    lights = [evaluate(c, base) for c in checks if c.get('enabled', True)]
    state = next((s for s in ['bad', 'unknown', 'working'] if any(l['state'] == s for l in lights)), 'good' if lights else 'unknown')
    return {'state': state, 'checkedAt': dt.datetime.now(dt.timezone.utc).isoformat(), 'lights': lights}

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--config', type=Path, default=Path(__file__).with_name('panel.json'))
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    try:
        panel = evaluate_panel(json.loads(args.config.read_text(encoding='utf-8-sig')), args.config.resolve().parent)
    except (OSError, ValueError, KeyError, TypeError) as exc:
        panel = {'state': 'unknown', 'lights': [], 'error': str(exc)}
    text = json.dumps(panel, indent=2)
    if args.output:
        temporary = args.output.with_suffix('.tmp')
        temporary.write_text(text, encoding='utf-8')
        temporary.replace(args.output)
    else:
        print(text)
    return 0

if __name__ == '__main__':
    sys.exit(main())
