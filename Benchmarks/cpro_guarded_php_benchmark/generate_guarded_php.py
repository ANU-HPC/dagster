#!/usr/bin/env python3
"""Generate a standalone UNSAT SAT instance tailored to c_pro's high-degree split.

For each complete assignment a to w selector variables, one private PHP(p,h)
core is activated.  Since p=h+1, every selector assignment activates an UNSAT
pigeonhole core, so the whole CNF is UNSAT.  Clauses in core a are guarded by
the clause that is false exactly under assignment a.
"""
import argparse, json, hashlib
from pathlib import Path


def guard_for(pattern, selectors):
    # Guard is false exactly when selector values equal pattern.
    return [(-v if bit else v) for v, bit in zip(selectors, pattern)]


def php_clauses(var, pigeons, holes):
    x = [[var() for _ in range(holes)] for _ in range(pigeons)]
    clauses = []
    # Every pigeon occupies at least one hole.
    clauses += [row[:] for row in x]
    # Every pigeon occupies at most one hole.
    for i in range(pigeons):
        for h1 in range(holes):
            for h2 in range(h1 + 1, holes):
                clauses.append([-x[i][h1], -x[i][h2]])
    # No hole contains two pigeons.
    for h in range(holes):
        for p1 in range(pigeons):
            for p2 in range(p1 + 1, pigeons):
                clauses.append([-x[p1][h], -x[p2][h]])
    return clauses


def build(width, holes):
    next_var = width + 1
    def fresh():
        nonlocal next_var
        v = next_var; next_var += 1; return v
    selectors = list(range(1, width + 1))
    clauses = []
    for mask in range(1 << width):
        pattern = [(mask >> i) & 1 for i in range(width)]
        guard = guard_for(pattern, selectors)
        core = php_clauses(fresh, holes + 1, holes)
        clauses.extend(guard + c for c in core)
    return next_var - 1, clauses


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--width', type=int, default=8, help='selector width; use the same c_pro split width')
    ap.add_argument('--holes', type=int, default=7, help='holes in each PHP core; pigeons=holes+1')
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    if not (1 <= args.width <= 16): ap.error('width must be 1..16')
    if not (2 <= args.holes <= 12): ap.error('holes must be 2..12')
    nvars, clauses = build(args.width, args.holes)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open('w') as f:
        f.write('c guarded pigeonhole benchmark for c_pro\n')
        f.write(f'c selectors 1-{args.width}; split width should be {args.width}\n')
        f.write(f'c each selector cube activates PHP({args.holes+1},{args.holes}); expected UNSAT\n')
        f.write(f'p cnf {nvars} {len(clauses)}\n')
        for c in clauses: f.write(' '.join(map(str,c)) + ' 0\n')
    digest = hashlib.sha256(args.output.read_bytes()).hexdigest()
    meta = {'family':'guarded_php','expected':'UNSAT','width':args.width,
            'selectors':[1,args.width], 'holes':args.holes, 'pigeons':args.holes+1,
            'cubes':1 << args.width, 'variables':nvars, 'clauses':len(clauses),
            'sha256':digest}
    args.output.with_suffix(args.output.suffix+'.json').write_text(json.dumps(meta,indent=2)+'\n')
    print(json.dumps(meta, indent=2))

if __name__ == '__main__': main()
