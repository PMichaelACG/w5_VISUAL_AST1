#!/usr/bin/env python3
"""Verify exported structure independently, then render and inspect SVG."""
from pathlib import Path
import json, re, subprocess, xml.etree.ElementTree as ET
ROOT = Path(__file__).resolve().parent.parent
def tree(dot):
    labels, children, incoming = {}, {}, {}
    for line in dot.splitlines():
        node = re.fullmatch(r'  (n\d+) \[label=("(?:[^"\\]|\\.)*")\];', line)
        edge = re.fullmatch(r'  (n\d+) -> (n\d+) \[label=("(?:[^"\\]|\\.)*")\];', line)
        if node:
            key, label = node.groups()
            assert key not in labels
            labels[key] = json.loads(label)
            children.setdefault(key, [])
        if edge:
            a, b, role = edge.groups()
            assert b not in incoming
            incoming[b] = a
            children.setdefault(a, []).append((json.loads(role), b))
    roots = set(labels) - set(incoming)
    assert len(roots) == 1 and set(incoming) <= set(labels)
    seen = set()
    def visit(key):
        assert key not in seen
        seen.add(key)
        return [labels[key], [[role, visit(child)] for role, child in children[key]]]
    result = visit(roots.pop())
    assert len(seen) == len(labels)
    return result

cases = json.loads((ROOT/'tests/trees.json').read_text())
for case in cases:
    p = subprocess.run([ROOT/'build/quectoc-ast'], input=case['input'].encode(),
                       capture_output=True, timeout=10)
    assert p.returncode == 0 and not p.stderr
    assert tree(p.stdout.decode()) == case['tree'], case['id']
print(f'PASS {len(cases)} independent exported AST shape cases')
sample = subprocess.run([ROOT/'build/quectoc-ast', ROOT/'examples/sample.qc'],
                        capture_output=True, check=True, timeout=10).stdout
svg = subprocess.run(['dot','-Tsvg'], input=sample, capture_output=True, check=True, timeout=20).stdout
doc = ET.fromstring(svg)
assert len([g for g in doc.iter() if g.attrib.get('class') == 'node']) == 4
assert len([g for g in doc.iter() if g.attrib.get('class') == 'edge']) == 3
escaped = subprocess.run(['dot','-Tsvg',ROOT/'build/escaping.dot'],
                         capture_output=True, check=True, timeout=20).stdout
doc = ET.fromstring(escaped)
assert len([g for g in doc.iter() if g.attrib.get('class') == 'node']) == 3
texts = [''.join(t.itertext()) for t in doc.iter() if t.tag.endswith('}text')]
assert any('\\N' in t and 'evil' in t for t in texts)
assert any('quote "' in t and '\\n' in t for t in texts)
print('PASS 2 Graphviz SVG renders including label escaping')
