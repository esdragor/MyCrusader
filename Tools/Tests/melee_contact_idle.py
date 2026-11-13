"""BUG-melee-stuck-attack (CELEBORN-1113f): a soldier at contact of a living enemy that does not strike.
Reads SanityDump3 (hol.Sanity.DumpUnit -3) and 'Combat: step N #a ... hits #b' lines.
Flags (1) IDLE: alive, a living hostile within REACH cm, speed < 30, no blow given, >= MIN steps in a row;
      (2) LOOP: 'at1' (attacking) >= LOOP steps in a row without a blow (the attack start that never lands).
Usage: python -I contact_idle.py log [log...] [--min 40] [--reach 230] [--loop 60]"""
import re, sys
from collections import defaultdict

def opt(name, default):
    return type(default)(sys.argv[sys.argv.index(name) + 1]) if name in sys.argv else default
MIN, REACH, LOOP = opt('--min', 40), opt('--reach', 230.0), opt('--loop', 60)
logs = [a for i, a in enumerate(sys.argv[1:], 1) if not a.startswith('--') and not sys.argv[i - 1].startswith('--')]
DUMP = re.compile(r'SanityDump3 #(\d+) step (\d+) at \((-?\d+), (-?\d+)\) vel \((-?\d+), (-?\d+)\) ph (\d+) .*? ap(\d) at(\d) tgt #(-?\d+) .*?place (\S+) np (\S+)')
HIT = re.compile(r'Combat: step (\d+) #(\d+) \S+ hits #(\d+)')

def camp(u):
    return 0 if u <= 6 else 1

for log in logs:
    units = defaultdict(dict)  # unit -> step -> (x, y, speed, at, tgt, place, np, ph)
    hits = defaultdict(set)    # attacker -> steps
    struck = defaultdict(set)  # victim -> steps
    for line in open(log, encoding='utf-8', errors='replace'):
        m = DUMP.search(line)
        if m:
            u, s = int(m.group(1)), int(m.group(2))
            x, y, vx, vy = (float(m.group(k)) for k in (3, 4, 5, 6))
            units[u][s] = (x, y, (vx * vx + vy * vy) ** 0.5, int(m.group(9)), int(m.group(10)), m.group(11), float(m.group(12)), int(m.group(7)))
            continue
        m = HIT.search(line)
        if m:
            hits[int(m.group(2))].add(int(m.group(1)))
            struck[int(m.group(3))].add(int(m.group(1)))
    last = {u: max(st) for u, st in units.items() if st}
    first_hit = min((min(v) for v in hits.values() if v), default=10 ** 9)
    found = []
    for u, st in sorted(units.items()):
        run_idle, run_loop = [], []
        for s in sorted(st):
            x, y, sp, at, tgt, place, np, ph = st[s]
            near = [(((units[e][s][0] - x) ** 2 + (units[e][s][1] - y) ** 2) ** 0.5, e) for e in units if camp(e) != camp(u) and s in units[e]]
            near = [d for d in near if d[0] <= REACH]
            blow = any(abs(s - h) <= 1 for h in hits[u])
            idle = s >= first_hit and near and sp < 30 and not blow and not any(h in range(s - 30, s + 1) for h in hits[u])
            run_idle = run_idle + [(s, min(near), tgt, at, place, np, ph)] if idle else (found.append(('IDLE', u, run_idle)) if len(run_idle) >= MIN else None) or []
            looping = at == 1 and not any(h in range(s - 40, s + 1) for h in hits[u])
            run_loop = run_loop + [(s, min(near) if near else (0, -1), tgt, at, place, np, ph)] if looping else (found.append(('LOOP', u, run_loop)) if len(run_loop) >= LOOP else None) or []
        for kind, run, n in (('IDLE', run_idle, MIN), ('LOOP', run_loop, LOOP)):
            if len(run) >= n:
                found.append((kind, u, run))
    print('%s : %d case(s) (min %d steps, reach %.0f cm, loop %d)' % (log.split('\\')[-1], len(found), MIN, REACH, LOOP))
    for kind, u, run in found:
        a, b = run[0], run[-1]
        tgts = sorted({r[2] for r in run})
        gotstruck = sorted(h for h in struck[u] if a[0] - 40 <= h <= b[0])
        print('  %s #%d (camp %d) steps %d-%d (%d) nearest enemy #%d %.0f->%.0f cm, tgt %s, at %d->%d, place %s, np %.1f->%.1f, ph %d, struck at %s, died %s' % (
            kind, u, camp(u), a[0], b[0], len(run), a[1][1], a[1][0], b[1][0], tgts, a[3], b[3], b[4], a[5], b[5], b[6], gotstruck[:6], last[u] if last[u] < max(last.values()) else 'no'))
