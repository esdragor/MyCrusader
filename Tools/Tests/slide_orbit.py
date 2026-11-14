"""D641 residue #2 (CELEBORN-1114c): does a melee soldier ORBIT round a still friend (rear-slide way recomputed as the blocker's tangent)?
Reads SanityDump3 lines (hol.Sanity.DumpUnit -3, units 1-16, 'near:' = bodies within 2.5 m) of a scenario log.
For the unit (--unit N, default: every unit named by a 'Sanity: FAIL unit #N' line), takes the window of --window steps where its
velocity heading turns the most, then prints: path / net / mean speed / heading turn (total and signed) / heading reversals;
slide way set (steps, side sign vs the nearest friend = Hand); target and slot changes; and, for the friend that is nearest most often:
its own drift, the distance to it (min / median / max) and the signed angle of the unit's position round it (in turns).
Verdict ORBIT if: slide set on >= 50 % of the window with one side only, one target and one slot, the friend drifts < 60 cm,
the distance stays within 70..260 cm and the angle round it is >= 1 full turn; otherwise NOT-ORBIT with the failed criteria.
Usage: python -I slide_orbit.py log [log...] [--unit 2] [--window 240]"""
import math, re, sys, statistics
from collections import Counter

def opt(name, default):
    return type(default)(sys.argv[sys.argv.index(name) + 1]) if name in sys.argv else default
UNIT, WINDOW = opt('--unit', -1), opt('--window', 240)
logs = [a for i, a in enumerate(sys.argv[1:], 1) if not a.startswith('--') and not sys.argv[i - 1].startswith('--')]
DUMP = re.compile(r'SanityDump3 #(\d+) step (\d+) at \((-?\d+), (-?\d+)\) vel \((-?\d+), (-?\d+)\) ph (-?\d+) slot \((-?\d+), (-?\d+)\) '
                  r'ap(\d) at(\d) tgt #(-?\d+) \((-?\d+), (-?\d+)\) slide \((-?[\d.]+), (-?[\d.]+)\) flank (\d) place (\d)/(-?[\d.]+) np ([\d.]+) room (\d) loop ([\d.]+) near:(.*)')
NEAR = re.compile(r'#(\d+)\((\d+)\)\((-?\d+),(-?\d+)\)v\((-?\d+),(-?\d+)\)')
FAIL = re.compile(r'Sanity: FAIL unit #(\d+) .*')

def owner(u):
    return 0 if u <= 6 else 1

def wrap(a):
    while a > math.pi: a -= 2 * math.pi
    while a < -math.pi: a += 2 * math.pi
    return a

for log in logs:
    rows, fails = {}, []
    for line in open(log, encoding='utf-8', errors='replace'):
        m = DUMP.search(line)
        if m:
            g = m.groups()
            u, s = int(g[0]), int(g[1])
            near = [(int(a), int(b), float(c), float(d), float(e), float(f)) for a, b, c, d, e, f in NEAR.findall(g[22])]
            rows.setdefault(u, {})[s] = dict(x=float(g[2]), y=float(g[3]), vx=float(g[4]), vy=float(g[5]), slot=(g[7], g[8]), tgt=int(g[11]),
                                             sx=float(g[14]), sy=float(g[15]), place=(g[17], g[18]), near=near)
            continue
        m = FAIL.search(line)
        if m:
            fails.append(m.group(0).strip())
    print(f'== {log}')
    for f in fails:
        print('   ' + f)
    units = [UNIT] if UNIT > 0 else sorted({int(FAIL.search(f).group(1)) for f in fails})
    for u in units:
        st = rows.get(u, {})
        steps = sorted(st)
        if len(steps) < WINDOW:
            print(f'#{u}: {len(steps)} dump rows, fewer than the window'); continue
        head = [math.atan2(st[s]['vy'], st[s]['vx']) if math.hypot(st[s]['vx'], st[s]['vy']) > 30 else None for s in steps]
        dturn = [0.0] + [abs(wrap(head[i] - head[i - 1])) if head[i] is not None and head[i - 1] is not None else 0.0 for i in range(1, len(steps))]
        best, bi = -1, 0
        acc = sum(dturn[:WINDOW])
        for i in range(0, len(steps) - WINDOW + 1):
            if i:
                acc += dturn[i + WINDOW - 1] - dturn[i - 1]
            if acc > best:
                best, bi = acc, i
        win = steps[bi:bi + WINDOW]
        W = [st[s] for s in win]
        path = sum(math.hypot(W[i]['x'] - W[i - 1]['x'], W[i]['y'] - W[i - 1]['y']) for i in range(1, len(W)))
        net = math.hypot(W[-1]['x'] - W[0]['x'], W[-1]['y'] - W[0]['y'])
        signed, rev, last = 0.0, 0, 0
        for i in range(bi + 1, bi + WINDOW):
            if head[i] is not None and head[i - 1] is not None:
                d = wrap(head[i] - head[i - 1]); signed += d
                sg = 1 if d > 0.02 else (-1 if d < -0.02 else 0)
                if sg and last and sg != last: rev += 1
                if sg: last = sg
        speeds = [math.hypot(r['vx'], r['vy']) for r in W]
        friends = Counter()
        for r in W:
            fr = [(math.hypot(n[2] - r['x'], n[3] - r['y']), n[0]) for n in r['near'] if n[1] == owner(u)]
            if fr: friends[min(fr)[1]] += 1
        print(f'#{u} window steps {win[0]}-{win[-1]} ({WINDOW}): path {path:.0f} cm, net {net:.0f} cm, mean speed {statistics.mean(speeds):.0f} cm/step-vel, '
              f'heading turn {best:.1f} rad total / {signed:+.1f} rad signed, {rev} reversals')
        slid = [r for r in W if abs(r['sx']) + abs(r['sy']) > 0.01]
        tg, sl, pl = Counter(r['tgt'] for r in W), Counter(r['slot'] for r in W), Counter(r['place'] for r in W)
        print(f'   slide set {len(slid)}/{WINDOW} steps; targets {dict(tg)}; slots {len(sl)}; places {len(pl)}; nearest friends {dict(friends.most_common(4))}')
        crit = []
        if not friends:
            print('   no friend within 2.5 m: NOT-ORBIT'); continue
        fid = friends.most_common(1)[0][0]
        fpos, dist, ang, hands = [], [], [], Counter()
        for r in W:
            n = [n for n in r['near'] if n[0] == fid]
            if not n: ang.append(None); continue
            fx, fy = n[0][2], n[0][3]
            fpos.append((fx, fy)); dist.append(math.hypot(r['x'] - fx, r['y'] - fy))
            ang.append(math.atan2(r['y'] - fy, r['x'] - fx))
            if abs(r['sx']) + abs(r['sy']) > 0.01:
                tx, ty = fx - r['x'], fy - r['y']
                hands['+' if (tx * r['sy'] - ty * r['sx']) > 0 else '-'] += 1
        orbit = sum(wrap(ang[i] - ang[i - 1]) for i in range(1, len(ang)) if ang[i] is not None and ang[i - 1] is not None) / (2 * math.pi)
        drift = max(math.hypot(p[0] - fpos[0][0], p[1] - fpos[0][1]) for p in fpos)
        print(f'   friend #{fid}: seen {len(fpos)}/{WINDOW} steps, drift {drift:.0f} cm, distance min {min(dist):.0f} / median {statistics.median(dist):.0f} / max {max(dist):.0f} cm, '
              f'angle round it {orbit:+.2f} turns; slide side vs friend {dict(hands)}')
        if len(slid) < WINDOW / 2: crit.append('slide set < 50 %')
        if len(hands) > 1: crit.append('slide side changes')
        if len(tg) > 1: crit.append('target changes')
        if len(sl) > 1: crit.append('slot changes')
        if drift >= 60: crit.append('friend moves')
        if min(dist) < 70 or max(dist) > 260: crit.append('distance not ~constant')
        if abs(orbit) < 1: crit.append('less than 1 turn round the friend')
        print('   verdict: ' + ('ORBIT' if not crit else 'NOT-ORBIT (' + '; '.join(crit) + ')'))
