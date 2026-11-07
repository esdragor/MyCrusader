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
# Celeborn-12 (walk to the slot, FinalLeg = phase 3, no fight): a soldier that does not move for more than WALK_STALL steps
# (default 20 = 1 s) or that shakes (WALK_SHAKE or more reversals of its step in a window of 20 steps, each step over 5 cm).
# Read in the SanityDump3 lines of hol.Sanity.DumpUnit -3. Option --stall N / --shake N changes the limits.
WALK_STALL = 20
WALK_SHAKE = 8
for opt in ('--stall', '--shake'):
    if opt in sys.argv:
        value = sys.argv[sys.argv.index(opt) + 1]
        args = [a for a in args if a != value]
        if opt == '--stall': WALK_STALL = int(value)
        else: WALK_SHAKE = int(value)
WALK_ROW = re.compile(r'SanityDump3 #(\d+) step (\d+) at \((-?\d+), (-?\d+)\) vel \((-?\d+), (-?\d+)\) ph (\d) slot \((-?\d+), (-?\d+)\) ap(\d) at(\d) tgt #(-?\d+)')
def walks(path):
    dead = {}; seq = collections.defaultdict(list)
    for l in open(path, encoding='utf-8', errors='ignore'):
        m = re.search(r'Combat: step (\d+) #(\d+) \w+ of slot (\d) killed by', l)
        if m: dead[int(m[2])] = int(m[1])
        m = WALK_ROW.search(l)
        if m:
            seq[int(m[1])].append((int(m[2]), int(m[3]), int(m[4]), int(m[7]), int(m[10]), int(m[11]), int(m[12]), int(m[8]), int(m[9])))
    out = []
    for u, rows in sorted(seq.items()):
        rows.sort()
        run = 0; start = 0
        for a, b in zip(rows, rows[1:]):
            walking = b[3] == 3 and b[4] == 0 and b[5] == 0 and b[6] == -1 and b[0] == a[0] + 1 and not (u in dead and dead[u] <= b[0])
            still = walking and abs(b[1]-a[1]) + abs(b[2]-a[2]) <= 2
            if still:
                if run == 0: start = a[0]
                run += 1
            else:
                if run > WALK_STALL: out.append(('STALL', u, start, start + run, rows[0], b))
                run = 0
        if run > WALK_STALL: out.append(('STALL', u, start, start + run, rows[0], rows[-1]))
        # reversals: the step's vector against the one before
        rev = []
        for i in range(2, len(rows)):
            p, q, r = rows[i-2], rows[i-1], rows[i]
            if not (r[3] == 3 and r[4] == 0 and r[5] == 0 and r[6] == -1 and r[0] == q[0] + 1 == p[0] + 2): continue
            d1 = (q[1]-p[1], q[2]-p[2]); d2 = (r[1]-q[1], r[2]-q[2])
            if (d1[0]**2+d1[1]**2) > 25 and (d2[0]**2+d2[1]**2) > 25 and d1[0]*d2[0] + d1[1]*d2[1] < 0: rev.append(r[0])
        for i in range(len(rev)):
            window = [s for s in rev if rev[i] <= s < rev[i] + 20]
            if len(window) >= WALK_SHAKE:
                out.append(('SHAKE', u, rev[i], rev[i] + 20, rows[0], rows[-1])); break
    return out
# Celeborn-13 (Q5 = C, the EFFECT): a soldier struck while its walk makes no headway (ph 3, no fight, no target) must strike back within
# RIPOSTE_STEPS (default 25 = 1.25 s) of the first blow it takes in that state: its OWN hit on any enemy shows in the log ("Combat: step N #U ... hits").
# Not flagged if it or its attacker dies first, or when the first blow came in the last RIPOSTE_STEPS of the run. Option --riposte N.
RIPOSTE_STEPS = 25
if '--riposte' in sys.argv:
    value = sys.argv[sys.argv.index('--riposte') + 1]
    args = [a for a in args if a != value]
    RIPOSTE_STEPS = int(value)
def unanswered(path):
    dead = {}; last = 0; struck = collections.defaultdict(list); own = collections.defaultdict(list); state = {}
    for l in open(path, encoding='utf-8', errors='ignore'):
        m = re.search(r'Combat: step (\d+) #(\d+) \w+ of slot (\d) killed by', l)
        if m: dead[int(m[2])] = int(m[1])
        m = re.search(r'Combat: step (\d+) #(\d+) \w+ hits #(\d+)', l)
        if m:
            s, a, v = int(m[1]), int(m[2]), int(m[3])
            own[a].append(s); struck[v].append((s, a)); last = max(last, s)
        m = WALK_ROW.search(l)
        if m: state[(int(m[1]), int(m[2]))] = (int(m[7]), int(m[10]), int(m[11]), int(m[12]))
    out = []
    for v, blows in sorted(struck.items()):
        for s, a in sorted(blows):
            st = state.get((v, s)) or state.get((v, s - 1)) or state.get((v, s + 1))
            if not st or not (st[0] == 3 and st[1] == 0 and st[2] == 0 and st[3] == -1): continue
            if s + RIPOSTE_STEPS > last or (v in dead and dead[v] <= s + RIPOSTE_STEPS) or (a in dead and dead[a] <= s + RIPOSTE_STEPS): break
            if not any(s <= t <= s + RIPOSTE_STEPS for t in own[v]):
                out.append(('UNANSWERED', v, s, s + RIPOSTE_STEPS, None, None))
            break
    return out
def fmt_walk(w):
    kind, u, s0, s1 = w[0], w[1], w[2], w[3]
    return '%s #%d steps %d-%d' % (kind, u, s0, s1)
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
    wk = walks(path) + unanswered(path)
    if '--walk' in sys.argv:
        # only the walk checks (Celeborn-12): the other checks stay as they are for the melee_stuck scenarios
        gaps, sw, pr = [], [], []
    bad += 1 if (gaps or sw or pr or wk) else 0
    print(('FAIL ' if (gaps or sw or pr or wk) else 'OK   ') + os.path.basename(path), ('gaps ' + str(gaps)) if gaps else '', ('switches while striking (unit, step, from, to) ' + str(sw[:5])) if sw else '')
    for p in pr[:4]: print('     STUCK ' + fmt_pair(p))
    for w in sorted(wk, key=lambda x: x[0] != 'UNANSWERED')[:5]: print('     WALK ' + fmt_walk(w))
print('melee_stuck_gaps: %d of %d runs have a soldier without a blow for more than %d steps, or one that left a living target while striking it' % (bad, len(args), limit))
sys.exit(1 if bad else 0)


