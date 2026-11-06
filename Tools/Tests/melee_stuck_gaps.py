# BUG-melee-stuck-attack: reads the logs of repeat_scenario.ps1 melee_stuck (hol.Combat.LogHits 1) and fails (exit 1) when a soldier
# stays more than -Limit steps (default 100 = 5 s) without striking while it is alive and the fight goes on. Red before the fix
# (the soldier #10 shut between two friends: 338 steps), green after. Usage:
#   python Tools\Tests\melee_stuck_gaps.py <Root>\Saved_Tests\Logs\scenario_melee_stuck_run*.log [--limit 100]
import os, re, sys, collections
args = [a for a in sys.argv[1:] if not a.startswith('--')]
limit = 100
if '--limit' in sys.argv:
    limit = int(sys.argv[sys.argv.index('--limit') + 1])
    args = [a for a in args if a != str(limit)]
# Second rule (the user's): a soldier that strikes a living target keeps it until it dies. Read from the DumpUnit -3 lines
# (SanityDump3 #id step N ... atX tgt #T): at1 on a step and another target on the next while the first is not dead = a fault.
def switches(path):
    dead = {}; rows = collections.defaultdict(list)
    for l in open(path, encoding='utf-8', errors='ignore'):
        m = re.search(r'Combat: step (\d+) #(\d+) \w+ of slot \d killed by', l)
        if m: dead[int(m[2])] = int(m[1])
        m = re.search(r'SanityDump3 #(\d+) step (\d+) .*? at(\d) tgt #(-?\d+)', l)
        if m: rows[int(m[1])].append((int(m[2]), int(m[3]), int(m[4])))
    out = []
    for u, r in sorted(rows.items()):
        for a, b in zip(r, r[1:]):
            if a[0] >= 60 and a[1] == 1 and a[2] != -1 and b[2] != a[2] and not (a[2] in dead and dead[a[2]] <= b[0]):  # before step 60 the attack-move order is still arriving (it disengages)
                out.append((u, a[0], a[2], b[2]))
    return out

# Third detector (tester 2026-11-06): the "stuck pair". Two living soldiers of the SAME camp, less than 250 cm apart, both at speed
# < 15 cm/s, neither striking (at0), for >= PAIR_STEPS consecutive steps (default 60 = 3 s at 20 steps/s) while an enemy is alive.
# Printed: both ids, steps, target / claimed place (bHasPlace/angle) / np of each, so the "same place claimed" or "blocking each
# other" hypotheses can be read from the SanityDump3 lines. Option --pair N changes the length.
PAIR_STEPS = 60
if '--pair' in sys.argv:
    PAIR_STEPS = int(sys.argv[sys.argv.index('--pair') + 1])
    args = [a for a in args if a != str(PAIR_STEPS)]
ROW = re.compile(r'SanityDump3 #(\d+) step (\d+) at \((-?\d+), (-?\d+)\) vel \((-?\d+), (-?\d+)\).*? slot \((-?\d+), (-?\d+)\) ap\d at(\d) tgt #(-?\d+) \((-?\d+), (-?\d+)\) slide \((-?[\d.]+), (-?[\d.]+)\) flank (\d) place (\d)/(-?[\d.]+) np ([\d.]+)(.*?near:(.*))?$')
def pairs(path):
    dead = {}; owner = {}; rows = collections.defaultdict(dict)
    for l in open(path, encoding='utf-8', errors='ignore'):
        m = re.search(r'Combat: step (\d+) #(\d+) \w+ of slot (\d) killed by', l)
        if m: dead[int(m[2])] = int(m[1]); owner[int(m[2])] = int(m[3])
        m = ROW.search(l)
        if m:
            u, st = int(m[1]), int(m[2])
            rows[st][u] = dict(x=int(m[3]), y=int(m[4]), vx=int(m[5]), vy=int(m[6]), at=int(m[9]), tgt=int(m[10]), tx=int(m[11]), ty=int(m[12]), sl=(m[13], m[14]), place=int(m[16]), ang=m[17], np=float(m[18]), px=int(m[7]), py=int(m[8]))
            for o in re.finditer(r'#(\d+)\((\d)\)', m[20] or ''): owner[int(o[1])] = int(o[2])
    run = {}; out = []; done = set()
    for st in sorted(rows):
        alive = {u: r for u, r in rows[st].items() if not (u in dead and dead[u] <= st)}
        enemy = {o for u in alive for o in [owner.get(u)] if o is not None}
        cur = set()
        for a in alive:
            for b in alive:
                if a < b and owner.get(a) is not None and owner.get(a) == owner.get(b) and len(enemy) > 1:
                    A, B = alive[a], alive[b]
                    if (A['x']-B['x'])**2 + (A['y']-B['y'])**2 < 250**2 and A['vx']**2+A['vy']**2 < 15**2 and B['vx']**2+B['vy']**2 < 15**2 and A['at'] == 0 and B['at'] == 0:
                        cur.add((a, b)); run.setdefault((a, b), [st, st, A, B]); run[(a, b)][1] = st; run[(a, b)][2] = A; run[(a, b)][3] = B
        for k in list(run):
            if k not in cur:
                s0, s1, A, B = run.pop(k)
                if s1 - s0 + 1 >= PAIR_STEPS: out.append((k, s0, s1, A, B))
    for k, (s0, s1, A, B) in run.items():
        if s1 - s0 + 1 >= PAIR_STEPS: out.append((k, s0, s1, A, B))
    return out
def fmt_pair(p):
    (a, b), s0, s1, A, B = p
    def d(U, x, y): return ((U['x']-x)**2 + (U['y']-y)**2) ** 0.5
    same = A['tgt'] == B['tgt'] and A['place'] and B['place'] and abs(A['px']-B['px']) < 30 and abs(A['py']-B['py']) < 30
    f = lambda n, U: '#%d tgt #%d (%.0f cm) slot (%d,%d) %.0f cm away, place %d/%s np %.1f slide %s' % (n, U['tgt'], d(U, U['tx'], U['ty']), U['px'], U['py'], d(U, U['px'], U['py']), U['place'], U['ang'], U['np'], U['sl'])
    return 'pair #%d/#%d steps %d-%d (%d steps) %s | %s | %s' % (a, b, s0, s1, s1-s0+1, 'SAME PLACE CLAIMED (H1)' if same else 'different places', f(a, A), f(b, B))
bad = 0
for path in args:
    hits = collections.defaultdict(list); death = {}; last = 0
    for l in open(path, encoding='utf-8', errors='ignore'):
        m = re.search(r'Combat: step (\d+) #(\d+) \w+ hits #(\d+)', l)
        if m: hits[int(m[2])].append(int(m[1])); last = max(last, int(m[1]))
        m = re.search(r'Combat: step (\d+) #(\d+) \w+ of slot \d killed by', l)
        if m: death[int(m[2])] = int(m[1])
    gaps = []
    for u, h in sorted(hits.items()):
        pts = [h[0]] + h + [death.get(u, last)]
        g = max((b - a, a, b) for a, b in zip(pts, pts[1:]))
        if g[0] > limit: gaps.append((u,) + g)
    sw = switches(path)
    pr = pairs(path)
    bad += 1 if (gaps or sw or pr) else 0
    print(('FAIL ' if (gaps or sw or pr) else 'OK   ') + os.path.basename(path), ('gaps ' + str(gaps)) if gaps else '', ('switches while striking (unit, step, from, to) ' + str(sw[:5])) if sw else '')
    for p in pr[:4]: print('     STUCK ' + fmt_pair(p))
print('melee_stuck_gaps: %d of %d runs have a soldier without a blow for more than %d steps, or one that left a living target while striking it' % (bad, len(args), limit))
sys.exit(1 if bad else 0)


