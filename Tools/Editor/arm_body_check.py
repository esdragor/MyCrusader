"""Check of the arms against the trunk (BUG 2026-11-12, "the Spearman's free arm is inside his body"): for each frame of
a gesture, how deep the axis of each forearm (elbow to wrist, and the hand 8 cm past the wrist) goes inside the trunk.

The trunk is the Mannequin body cloud of Data/SKM_Mannequin_Body.json carried by the solved bones (pelvis, spine_01,
spine_02, spine_03): its surface radius around the spine polyline (pelvis -> neck_01) is read in the direction of the
arm point, at the same height (vertices within 3 cm of the same place along the spine and 20 degrees of the same
direction). Depth = that radius minus the point's distance to the spine; > 0 means the arm's axis is inside the trunk.
No Unreal, no numpy: the same chain as preview_animation.py (anim_rig solve, the frames that the bake writes).
Run:
  python -I Tools/Editor/arm_body_check.py spear_attack [maa_attack ...] [--max 1.0]
Exit code 1 when a forearm axis goes deeper than --max cm (default 1.0) into the trunk on any frame.
"""
import math
import os
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import anim_rig  # noqa: E402
import preview_animation as pa  # noqa: E402

TRUNK = ("pelvis", "spine_01", "spine_02", "spine_03")
SPINE = ("pelvis", "spine_01", "spine_02", "spine_03", "neck_01")
SLAB_CM, CONE_DEG, HAND_CM = 3.0, 20.0, 8.0


def project(q, axis):
    """(arc length, axis point) of the point of the polyline `axis` nearest to q."""
    best, arc = None, 0.0
    for a, b in zip(axis, axis[1:]):
        ab = anim_rig.sub(b, a)
        n = anim_rig.length(ab)
        s = max(0.0, min(1.0, anim_rig.dot(anim_rig.sub(q, a), ab) / (n * n)))
        c = anim_rig.add(a, anim_rig.scale(ab, s))
        d = anim_rig.length(anim_rig.sub(q, c))
        if best is None or d < best[0]:
            best = (d, arc + s * n, c)
        arc += n
    return best[1], best[2]


def depth(q, axis, trunk):
    s, c = project(q, axis)
    out = anim_rig.sub(q, c)
    d = anim_rig.length(out)
    if d < 1e-6:
        return 99.0
    u = anim_rig.scale(out, 1.0 / d)
    cos_cone = math.cos(math.radians(CONE_DEG))
    radius = 0.0
    for vs, vc, v in trunk:
        if abs(vs - s) > SLAB_CM:
            continue
        w = anim_rig.sub(v, vc)
        n = anim_rig.length(w)
        if n > 1e-6 and anim_rig.dot(w, u) / n >= cos_cone:
            radius = max(radius, n)
    return radius - d


def check(shared, name):
    sk = shared.sk
    ctx = pa.prepare(shared, name)
    trunk_bones = {sk.index[b] for b in TRUNK}
    worst = {"l": (-99.0, 0, ""), "r": (-99.0, 0, "")}
    for k, world in enumerate(ctx["worlds"]):
        axis = [world[sk.index[b]][0] for b in SPINE]
        trunk = []
        for i, local, _ in shared.body[::2]:
            if i in trunk_bones:
                t, q = world[i]
                v = anim_rig.add(t, anim_rig.qrot(q, local))
                vs, vc = project(v, axis)
                trunk.append((vs, vc, v))
        for side in ("l", "r"):
            elbow, wrist = world[sk.index["lowerarm_" + side]][0], world[sk.index["hand_" + side]][0]
            fore = anim_rig.normalize(anim_rig.sub(wrist, elbow))
            points = [("elbow", elbow)] + [("forearm %d%%" % (25 * j), anim_rig.lerp(elbow, wrist, 0.25 * j))
                                           for j in (1, 2, 3)]
            points += [("wrist", wrist), ("hand", anim_rig.add(wrist, anim_rig.scale(fore, HAND_CM)))]
            for label, p in points:
                dp = depth(p, axis, trunk)
                if dp > worst[side][0]:
                    worst[side] = (dp, k, label)
    return ctx, worst


def main():
    args = sys.argv[1:]
    limit = 1.0
    if "--max" in args:
        i = args.index("--max")
        limit = float(args[i + 1])
        del args[i:i + 2]
    shared = pa.Shared(False)
    bad = False
    for name in args or ["spear_attack"]:
        ctx, worst = check(shared, name)
        fps = ctx["gesture"].get("fps", 30)
        for side, (dp, k, label) in sorted(worst.items()):
            verdict = "FAIL" if dp > limit else "ok"
            bad = bad or dp > limit
            print("%s %s arm: deepest %s at frame %d (%.2f s): %.1f cm inside the trunk (limit %.1f) %s"
                  % (name, "left" if side == "l" else "right", label, k, k / float(fps), dp, limit, verdict))
    print("arm_body_check: " + ("FAIL" if bad else "PASS"))
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
