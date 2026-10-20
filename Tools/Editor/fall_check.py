"""Checks of the falls: ground penetration of the body and props, hand turn per frame (world and vs forearm)."""
import math
import os
import sys
sys.dont_write_bytecode = True
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "Tools", "Editor"))
import preview_animation as pa  # noqa: E402
import anim_rig  # noqa: E402
import peasant_props  # noqa: E402


def qang(a, b):
    d = abs(sum(x * y for x, y in zip(a, b)))
    return math.degrees(2 * math.acos(min(1.0, d)))


def qinv(q):
    return (-q[0], -q[1], -q[2], q[3])


shared = pa.Shared(False)
sk = shared.sk
for name in sys.argv[1:]:
    ctx = pa.prepare(shared, name)
    worlds, rig = ctx["worlds"], ctx["rig"]
    ground0 = min(p[2] for p, _ in shared.posed_body(worlds[0], 7))
    body_min, prop_min, turn, rel = (0.0, 0), {}, (0.0, 0, ""), (0.0, 0, "")
    pelvis_z = []
    for k, world in enumerate(worlds):
        z = min(p[2] for p, _ in shared.posed_body(world, 3)) - ground0
        if z < body_min[0]:
            body_min = (z, k)
        pelvis_z.append(world[sk.index["pelvis"]][0][2])
        for prop in ctx["props"]:
            pz = min(p[2] for tri, _ in peasant_props.posed_triangles(prop, sk, world, shared.prop_tris[prop["name"]])
                     for p in tri) - ground0
            if pz < prop_min.get(prop["name"], (0.0, 0))[0]:
                prop_min[prop["name"]] = (pz, k)
        if k:
            for side in ("l", "r"):
                h, f = sk.index["hand_" + side], sk.index["lowerarm_" + side]
                a = qang(worlds[k - 1][h][1], world[h][1])
                if a > turn[0]:
                    turn = (a, k, side)
                r0 = anim_rig.qmul(qinv(worlds[k - 1][f][1]), worlds[k - 1][h][1])
                r1 = anim_rig.qmul(qinv(world[f][1]), world[h][1])
                b = qang(r0, r1)
                if b > rel[0]:
                    rel = (b, k, side)
    print("%s: %d frames %.2fs | body min %.1f cm @%d | props %s | hand turn world max %.0f deg @%d %s, wrist %.0f @%d %s"
          % (name, len(worlds) - 1, rig.length, body_min[0], body_min[1],
             ", ".join("%s %.1f @%d" % (n, v[0], v[1]) for n, v in prop_min.items()), turn[0], turn[1], turn[2],
             rel[0], rel[1], rel[2]))
    print("   pelvis z: " + " ".join("%.0f" % z for z in pelvis_z))
    print("   " + pa.report(ctx))
