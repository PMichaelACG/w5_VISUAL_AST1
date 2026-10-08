#!/usr/bin/env python3
"""Independent source-language expectations and command-line checks."""
from pathlib import Path
import json, subprocess
ROOT = Path(__file__).resolve().parent.parent
EXE = str(ROOT / 'build/quectoc-ast')
def run(text=None, args=()):
    return subprocess.run([EXE, *args], input=None if text is None else text.encode(),
                          capture_output=True, timeout=10)
def check(ok, message):
    if not ok: raise AssertionError(message)
cases = json.loads((ROOT / 'tests/syntax_cases.json').read_text())
for case in cases:
    result = run(case['input'])
    check(result.returncode == case['status'], (case['id'], result.returncode, result.stderr))
    check(result.stderr.decode() == case['stderr'], (case['id'], result.stderr))
    if result.returncode == 0:
        check(result.stdout.startswith(b'digraph AST {\n'), case['id'])
    else:
        check(result.stdout == b'', (case['id'], 'output after source failure'))
print(f'PASS {len(cases)} syntax regression cases')
sample = ROOT / 'examples/sample.qc'
a = run(sample.read_text())
b = run(args=(str(sample),))
c = run(sample.read_text(), args=('-',))
check((a.returncode, a.stdout, a.stderr) == (b.returncode, b.stdout, b.stderr) ==
      (c.returncode, c.stdout, c.stderr), 'input routes')
for args in [('a','b'), (str(ROOT / 'missing-file.qc'),)]:
    result = run(args=args)
    check(result.returncode == 2 and result.stdout == b'', ('usage/file', result))
if Path('/dev/full').exists():
    with open('/dev/full', 'wb') as output:
        result = subprocess.run([EXE, str(sample)], stdout=output, stderr=subprocess.PIPE, timeout=10)
    check(result.returncode == 2 and b'DOT output failed' in result.stderr, 'output error')
print('PASS command line, input routes and output-error checks')
