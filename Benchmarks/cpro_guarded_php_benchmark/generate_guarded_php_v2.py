#!/usr/bin/env python3
"""Generate a c_pro-friendly indirect guarded pigeonhole benchmark.

Selectors are variables 1..w.  High-degree equality-copy boosters make c_pro
rank them first.  A small decoder maps each selector assignment to one private
activation variable.  Hard PHP clauses are guarded by activation variables,
not selectors, so c_pro node 0 is a proper subset while node 1 retains all
clauses.  The complete formula is UNSAT.
"""
import argparse, hashlib, json
from pathlib import Path


def php_clauses(fresh, pigeons, holes):
    x = [[fresh() for _ in range(holes)] for _ in range(pigeons)]
    clauses = [row[:] for row in x]
    for p in range(pigeons):
        for h1 in range(holes):
            for h2 in range(h1 + 1, holes):
                clauses.append([-x[p][h1], -x[p][h2]])
    for h in range(holes):
        for p1 in range(pigeons):
            for p2 in range(p1 + 1, pigeons):
                clauses.append([-x[p1][h], -x[p2][h]])
    return clauses


def build(width, holes, boosters):
    next_var = width + 1
    def fresh():
        nonlocal next_var
        v = next_var
        next_var += 1
        return v

    selectors = list(range(1, width + 1))
    clauses = []

    # Equality copies preserve both selector truth values and raise selector
    # interaction degree without introducing hard clauses into PHP cores.
    for s in selectors:
        for _ in range(boosters):
            b = fresh()
            clauses.append([-s, b])
            clauses.append([s, -b])

    activations = []
    for mask in range(1 << width):
        a = fresh()
        activations.append(a)
        pattern_lits = [s if ((mask >> i) & 1) else -s
                        for i, s in enumerate(selectors)]
        # a <-> conjunction(pattern_lits)
        for lit in pattern_lits:
            clauses.append([-a, lit])
        clauses.append([a] + [-lit for lit in pattern_lits])

        # If a is true, every clause of this private PHP core is active.
        # Guard literal -a is true for inactive branches.
        for core_clause in php_clauses(fresh, holes + 1, holes):
            clauses.append([-a] + core_clause)

    return next_var - 1, clauses, selectors, activations


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--width', type=int, default=2)
    ap.add_argument('--holes', type=int, default=7)
    ap.add_argument('--boosters', type=int, default=80,
                    help='equality copies per selector; keeps selectors top-ranked')
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    if not 1 <= args.width <= 12:
        ap.error('width must be 1..12')
    if not 2 <= args.holes <= 12:
        ap.error('holes must be 2..12')
    if not 1 <= args.boosters <= 10000:
        ap.error('boosters must be 1..10000')

    nvars, clauses, selectors, activations = build(
        args.width, args.holes, args.boosters)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open('w') as f:
        f.write('c indirect guarded PHP benchmark for c_pro\n')
        f.write(f'c selectors {selectors[0]}-{selectors[-1]}\n')
        f.write(f'c activations {activations[0]}-{activations[-1]}\n')
        f.write(f'c expected UNSAT; {1 << args.width} selector cubes\n')
        f.write(f'p cnf {nvars} {len(clauses)}\n')
        for clause in clauses:
            f.write(' '.join(map(str, clause)) + ' 0\n')

    digest = hashlib.sha256(args.output.read_bytes()).hexdigest()
    meta = {
        'family': 'indirect_guarded_php_v2',
        'expected': 'UNSAT',
        'width': args.width,
        'selectors': selectors,
        'activations': activations,
        'holes': args.holes,
        'pigeons': args.holes + 1,
        'boosters_per_selector': args.boosters,
        'cubes': 1 << args.width,
        'variables': nvars,
        'clauses': len(clauses),
        'sha256': digest,
    }
    args.output.with_suffix(args.output.suffix + '.json').write_text(
        json.dumps(meta, indent=2) + '\n')
    print(json.dumps(meta, indent=2))


if __name__ == '__main__':
    main()
