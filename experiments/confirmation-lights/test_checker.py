import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import checker

class Checks(unittest.TestCase):
    def test_evidence_and_staleness(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            c = {'id': 'a', 'type': 'json', 'path': 'a.json', 'timestampField': 'time',
                 'maxAgeSeconds': 120, 'rules': [{'equals': 'ready', 'state': 'good'}]}
            self.assertEqual(checker.evaluate(c, base)['state'], 'unknown')
            (base / 'a.json').write_text(json.dumps({'state': 'ready', 'time': '2026-10-09T00:00:00Z'}))
            self.assertEqual(checker.evaluate(c, base, 1791504000)['state'], 'good')
            self.assertEqual(checker.evaluate(c, base, 1791504200)['state'], 'bad')
            (base / 'a.json').write_text('{')
            self.assertEqual(checker.evaluate(c, base)['state'], 'unknown')

    def test_aggregation_never_hides_unknown(self):
        with patch.object(checker, 'evaluate', side_effect=[{'state':'good'}, {'state':'unknown'}]):
            self.assertEqual(checker.evaluate_panel({'checks':[{'id':'a'}, {'id':'b'}]}, Path('.'))['state'], 'unknown')

    def test_dcg_checks_json_not_just_exit_zero(self):
        from subprocess import CompletedProcess
        with patch.object(checker, 'run', return_value=CompletedProcess([], 0, '{"ok":false}', '')):
            self.assertEqual(checker.evaluate({'id':'guard','type':'dcg'}, Path('.'))['state'], 'bad')

if __name__ == '__main__': unittest.main()
