#!/usr/bin/env python3
"""Structural checker for a generated guarded-PHP CNF and optional solver result."""
import argparse, json, hashlib, re
from pathlib import Path
ap=argparse.ArgumentParser()
ap.add_argument('cnf',type=Path)
ap.add_argument('--solver-output',type=Path)
a=ap.parse_args()
meta=json.loads(a.cnf.with_suffix(a.cnf.suffix+'.json').read_text())
sha=hashlib.sha256(a.cnf.read_bytes()).hexdigest()
assert sha==meta['sha256'], 'SHA-256 mismatch'
header=None; actual=0
with a.cnf.open() as f:
    for line in f:
        if line.startswith('p '):
            z=line.split(); header=(int(z[2]),int(z[3]))
        elif line and line[0] not in 'cp' and line.strip(): actual+=1
assert header==(meta['variables'],meta['clauses'])
assert actual==meta['clauses']
if a.solver_output:
    text=a.solver_output.read_text(errors='replace').upper()
    assert 'UNSAT' in text and 'SATISFIABLE' not in text.replace('UNSATISFIABLE',''), 'expected UNSAT solver result'
print('CHECK OK:', a.cnf.name, meta['expected'], meta['cubes'], 'cubes')
