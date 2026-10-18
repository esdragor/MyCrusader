"""Pure-Python rig for authoring mannequin animations by script (no unreal import, no numpy).

Used both by make_peasant_animations.py (inside Unreal, to bake) and by preview_animation.py (offline contact sheet),
so the preview shows exactly what gets baked.

Conventions (checked on the dumped UE4 mannequin, see AnimationsInProgress.md):
  - Vectors are (x, y, z) tuples in component (mesh) space, centimetres. The mesh faces +Y, Z is up; the character's
    right side is read from the skeleton (Skeleton.right: -X on the UE4 mannequin) rather than assumed.
  - Quaternions are (x, y, z, w) tuples, Hamilton product, rotating vectors as q v q*; this is the same algebra as
    Unreal's FQuat (A * B applies B first), so values pass to unreal.Quat unchanged.
  - Bone scales are 1 on this skeleton (the dump checks it); transforms here are rotation + translation only.

Gesture description (one dict, see peasant_poses.py):
  "name", "fps"  asset name and frame rate; "keys" the list of keys below (first at t = 0, last = first for a loop).
  "stance"  {"l": (right, forward, up), "r": ...}: constant offsets of the feet from their reference place, in cm.
  "elbows"  default elbow directions (see below); "fist" (degrees) curl of the finger joints 01, 02, 03 (constant).
  "tool"    {"kind": "axe" | "pick" | "hammer", "hand_spacing", "knob_offset", "handle_length", "hands"}: tool sizes
            in cm (defaults 22, 6, 80; also read at the top level, as the axe chop has them); "hands" 2 (default, both
            hands on the handle) or 1 (the right hand alone, the hammer: "knob_offset" is then the right hand's grip
            from the knob, the left hand is free, given by the keys' "hands"). A gesture with a tool also keys the
            TOOL BONE ik_hand_gun (the only ik_* bone ever written, J5c.2b): Rig.pose puts it on the tool frame
            (origin at the knob, X along the handle toward the head, Y toward the striking edge, Z = X x Y), so a prop
            skinned to it (peasant_props.py) follows the hands exactly. The edge is fixed in the right hand (the
            tool is held rigidly by it): the direction the head moves in at the hit, read once from the solved loop;
            or, for a weapon carried still (no hit: the soldiers' spear and sword, J7a.3), "edge": (right, forward,
            up) direction of the edge (the blade's side) at the first frame, then fixed in the right hand.
  "base"    optional path of a source animation (the walk under the loaded walk): its pelvis and legs are copied
            frame by frame (sampled by make_peasant_animations.py into Data/<name>.json), it sets the length and the
            frame rate, the key times are stretched onto its length and the legs are not solved.
  "steady_chest"  0-1: share of the pelvis' own swing taken back at spine_03 (a carrier keeps his load still).
  "steady_spine"  0-1: the same, spread along the spine (a third at spine_01, two thirds at spine_02, all of it at
                  spine_03): for a base whose pelvis turns a lot (the idle's fencing stance), the chest faces forward
                  without a kink at one joint.
  "hands_frame"   "chest": free hands' positions follow the chest (bob, sway) instead of staying fixed in the mesh;
                  a tool's grip follows it too (position only: the tool keeps its keyed direction, J7a.3).
  "scene"   preview only: what the gesture works on ("trunk", "rock", "bush", "beam").
  "preview" preview only: {"frames": N, "zoom_picks": [frame indices]} of the sheets made by --all.
  "props"   preview only: names of the props of peasant_props.py drawn on their bones (what the mesh will show).
Key (one dict per key):
  "t"       time in seconds.
  "ease"    how the segment ending at this key is timed: "linear", "in" (accelerates), "out" (decelerates), "inout".
  "pelvis"  (right, forward, up) offset of the pelvis in cm, in body terms.
  "rot"     {bone: (twist, bend, side)} in degrees, in body terms, applied at the bone in the reference posture:
            twist = turn toward the character's right (about up), bend = lean forward (about the right axis),
            side = lean toward the character's right (about the forward axis). Rotation order: side, then bend, then
            twist. The rotation is expressed in the reference pose's axes and carried by the parents' motion, so a key
            on spine_03 adds to the twist of spine_01 below it.
  "tool"    two-handed grip ("axe" is accepted too): {"grip": (right, forward, up) position of the lower (left) hand
            on the handle in cm, "dir": (right, forward, up) direction from the knob toward the head, "spacing": cm
            from the lower to the upper (right) hand}. The arms are solved by two-bone IK every frame so both hands
            stay on the handle; each hand grips with its palm across the handle, fingers pointing away from its
            shoulder. One-handed tool ("hands": 1): "grip" is the right hand's place on the handle, no "spacing", and
            the key also has "hands" with the free left hand only.
  "hands"   instead of a tool, free hands: {"l": {"at": (right, forward, up) cm, "palm": direction the palm faces,
            optional "fingers": direction, optional "swing": share of the same-side foot's travel the hand makes the
            other way (a walking arm, needs a base)}, "r": ...}; fingers default to pointing away from the shoulder.
  "elbows"  optional {"l": (right, forward, up), "r": ...}: directions the elbows bend toward, in the chest frame
            (carried by spine_03).
Values are interpolated between keys (linear for positions and angles, spherical for directions), after the key's
easing. A missing entry counts as the reference value (zero offset or rotation). Without a base, the legs are solved
by IK so the feet stay planted on their reference place (plus the stance offsets), whatever the pelvis does.
"""
import json
import math
import os

# ---------------------------------------------------------------------------------------------------------------------
# Vector and quaternion maths


def add(a, b):
    return (a[0] + b[0], a[1] + b[1], a[2] + b[2])


def sub(a, b):
    return (a[0] - b[0], a[1] - b[1], a[2] - b[2])


def scale(a, s):
    return (a[0] * s, a[1] * s, a[2] * s)


def dot(a, b):
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]


def cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def length(a):
    return math.sqrt(dot(a, a))


def normalize(a):
    n = length(a)
    return (a[0] / n, a[1] / n, a[2] / n) if n > 1e-9 else (0.0, 0.0, 0.0)


def lerp(a, b, s):
    return tuple(x + (y - x) * s for x, y in zip(a, b))


def qmul(a, b):
    ax, ay, az, aw = a
    bx, by, bz, bw = b
    return (aw * bx + ax * bw + ay * bz - az * by,
            aw * by - ax * bz + ay * bw + az * bx,
            aw * bz + ax * by - ay * bx + az * bw,
            aw * bw - ax * bx - ay * by - az * bz)


def qconj(q):
    return (-q[0], -q[1], -q[2], q[3])


def qnormalize(q):
    n = math.sqrt(sum(c * c for c in q))
    return tuple(c / n for c in q)


def qrot(q, v):
    """Rotates vector v by unit quaternion q."""
    u = (q[0], q[1], q[2])
    t = scale(cross(u, v), 2.0)
    return add(add(v, scale(t, q[3])), cross(u, t))


def qaxis(axis, degrees):
    """Rotation of `degrees` about `axis`; positive turns cross(a, b)'s a toward b (right-hand rule on coordinates)."""
    axis = normalize(axis)
    half = math.radians(degrees) * 0.5
    s = math.sin(half)
    return (axis[0] * s, axis[1] * s, axis[2] * s, math.cos(half))


def qfrom_matrix(cols):
    """Quaternion of the rotation matrix whose columns are the images of the X, Y and Z axes."""
    (m00, m10, m20), (m01, m11, m21), (m02, m12, m22) = cols
    trace = m00 + m11 + m22
    if trace > 0.0:
        s = math.sqrt(trace + 1.0) * 2.0
        q = ((m21 - m12) / s, (m02 - m20) / s, (m10 - m01) / s, 0.25 * s)
    elif m00 > m11 and m00 > m22:
        s = math.sqrt(1.0 + m00 - m11 - m22) * 2.0
        q = (0.25 * s, (m01 + m10) / s, (m02 + m20) / s, (m21 - m12) / s)
    elif m11 > m22:
        s = math.sqrt(1.0 + m11 - m00 - m22) * 2.0
        q = ((m01 + m10) / s, 0.25 * s, (m12 + m21) / s, (m02 - m20) / s)
    else:
        s = math.sqrt(1.0 + m22 - m00 - m11) * 2.0
        q = ((m02 + m20) / s, (m12 + m21) / s, 0.25 * s, (m10 - m01) / s)
    return qnormalize(q)


def frame(primary, secondary):
    """Orthonormal basis (columns) built from a primary direction and a secondary hint."""
    a = normalize(primary)
    c = normalize(cross(a, secondary))
    b = cross(c, a)
    return (a, b, c)


def qframes(from_primary, from_secondary, to_primary, to_secondary):
    """Rotation taking the frame (from_primary, from_secondary) onto (to_primary, to_secondary)."""
    fa, fb, fc = frame(from_primary, from_secondary)
    ta, tb, tc = frame(to_primary, to_secondary)
    # R = T * F^T, built column by column: R e_i = sum_k T_k F_k[i].
    cols = []
    for i in range(3):
        cols.append(add(add(scale(ta, fa[i]), scale(tb, fb[i])), scale(tc, fc[i])))
    return qfrom_matrix(cols)


def qbetween(u, v):
    """Smallest rotation turning direction u onto direction v."""
    u, v = normalize(u), normalize(v)
    c = dot(u, v)
    if c < -0.999999:
        axis = cross(u, (1.0, 0.0, 0.0))
        if length(axis) < 1e-6:
            axis = cross(u, (0.0, 1.0, 0.0))
        return qaxis(axis, 180.0)
    w = cross(u, v)
    return qnormalize((w[0], w[1], w[2], 1.0 + c))


def slerp_dir(a, b, s):
    """Spherical interpolation between two directions."""
    a, b = normalize(a), normalize(b)
    angle = math.acos(max(-1.0, min(1.0, dot(a, b))))
    if angle < 1e-6:
        return a
    sa = math.sin(angle)
    return normalize(add(scale(a, math.sin((1.0 - s) * angle) / sa), scale(b, math.sin(s * angle) / sa)))


def qslerp(a, b, s):
    """Spherical interpolation between two unit quaternions (shortest way)."""
    d = sum(x * y for x, y in zip(a, b))
    if d < 0.0:
        b, d = tuple(-c for c in b), -d
    if d > 0.9995:
        return qnormalize(tuple(x + (y - x) * s for x, y in zip(a, b)))
    angle = math.acos(d)
    sa = math.sin(angle)
    wa, wb = math.sin((1.0 - s) * angle) / sa, math.sin(s * angle) / sa
    return qnormalize(tuple(x * wa + y * wb for x, y in zip(a, b)))


EASES = {
    "linear": lambda s: s,
    "in": lambda s: s * s,
    "out": lambda s: 1.0 - (1.0 - s) * (1.0 - s),
    "inout": lambda s: s * s * (3.0 - 2.0 * s),
}

# ---------------------------------------------------------------------------------------------------------------------
# Skeleton


class Skeleton:
    """Reference skeleton: bones in parent-first order with local and component-space reference transforms."""

    def __init__(self, data):
        self.names = [bone["name"] for bone in data["bones"]]
        self.index = {name: i for i, name in enumerate(self.names)}
        self.parent = [self.index.get(bone["parent"], -1) if bone["parent"] else -1 for bone in data["bones"]]
        self.ref_local = [(tuple(b["local_t"]), qnormalize(tuple(b["local_q"]))) for b in data["bones"]]
        self.ref_world = self.fk(self.ref_local)
        for i in range(len(self.names)):
            if self.parent[i] >= i:
                raise ValueError("Bones are not in parent-first order: " + self.names[i])
        # Body axes: up is +Z, forward is +Y (MeshYawOffset -90); right is the side of hand_r.
        self.up = (0.0, 0.0, 1.0)
        self.forward = (0.0, 1.0, 0.0)
        side = 1.0 if self.pos("hand_r")[0] > 0.0 else -1.0
        self.right = (side, 0.0, 0.0)

    @staticmethod
    def load(path):
        with open(path, "r", encoding="utf-8") as f:
            return Skeleton(json.load(f))

    def pos(self, name, world=None):
        return (world or self.ref_world)[self.index[name]][0]

    def rot(self, name, world=None):
        return (world or self.ref_world)[self.index[name]][1]

    def fk(self, local):
        """Component-space (position, rotation) of every bone from local transforms."""
        world = []
        for i, (t, q) in enumerate(local):
            p = self.parent[i]
            if p < 0:
                world.append((t, q))
            else:
                pt, pq = world[p]
                world.append((add(pt, qrot(pq, t)), qnormalize(qmul(pq, q))))
        return world

    def body(self, v):
        """Body terms (right, forward, up) to a component-space vector."""
        return add(add(scale(self.right, v[0]), scale(self.forward, v[1])), scale(self.up, v[2]))

    def body_rotation(self, twist, bend, side):
        """Body-term rotation (degrees) as a component-space quaternion; order side, bend, twist."""
        q_twist = qaxis(cross(self.forward, self.right), twist)
        q_bend = qaxis(cross(self.up, self.forward), bend)
        q_side = qaxis(cross(self.up, self.right), side)
        return qmul(q_twist, qmul(q_bend, q_side))


# ---------------------------------------------------------------------------------------------------------------------
# Rig: body keys + IK arms and legs


ARM_CHAINS = {"l": ("upperarm_l", "lowerarm_l", "hand_l"), "r": ("upperarm_r", "lowerarm_r", "hand_r")}
LEG_CHAINS = {"l": ("thigh_l", "calf_l", "foot_l"), "r": ("thigh_r", "calf_r", "foot_r")}
# Bones the rig writes; every other bone keeps its reference pose (fingers, twist bones, ball, ik_*, root).
SPINE_BONES = ("pelvis", "spine_01", "spine_02", "spine_03", "neck_01", "head", "clavicle_l", "clavicle_r")
FINGERS = ("index", "middle", "ring", "pinky")
FINGER_BONES = tuple("%s_%02d_%s" % (f, k, side) for side in ("l", "r") for f in FINGERS for k in (1, 2, 3))
ANIMATED_BONES = (SPINE_BONES + ARM_CHAINS["l"] + ARM_CHAINS["r"] + LEG_CHAINS["l"] + LEG_CHAINS["r"]
                  + FINGER_BONES)
# The tool bone: unused by the mannequin's mesh, child of ik_hand_root (at the origin, never keyed); keyed only by the
# gestures with a tool, on the tool frame (J5c.2b). Every other ik_* bone stays untouched.
TOOL_BONE = "ik_hand_gun"


class Rig:
    """Turns a gesture description into local bone transforms, frame by frame."""

    def __init__(self, skeleton, gesture, base=None):
        """`base`: the sampled source animation (load_base) when the gesture has one."""
        self.sk = skeleton
        self.g = gesture
        self.base = base
        sk = skeleton
        # Two-handed grip: the lower (left) hand at "grip", the upper (right) hand further up the handle ("spacing"
        # of the key, so the top hand can slide down for the hit).
        tool = gesture.get("tool", {})
        self.hand_spacing = tool.get("hand_spacing", gesture.get("hand_spacing", 22.0))
        self.knob_offset = tool.get("knob_offset", gesture.get("knob_offset", 6.0))
        self.handle_length = tool.get("handle_length", gesture.get("handle_length", 80.0))
        # Hands on the tool's handle: both, or the right one alone (the left is then free).
        self.tool_hands = ("r",) if tool.get("hands", 2) == 1 else ("l", "r")
        self.grip_local = {}
        self.hand_axes = {}
        self.palm = {}
        self.fist = {}
        curl_angles = gesture.get("fist", (0.0, 0.0, 0.0))
        for side in ("l", "r"):
            hand = "hand_" + side
            knuckle = sk.pos("middle_01_" + side)
            # Hand axes in the reference pose: fingers (wrist to knuckle) and across the palm (pinky to index).
            fingers = sub(knuckle, sk.pos(hand))
            across = sub(sk.pos("index_01_" + side), sk.pos("pinky_01_" + side))
            self.hand_axes[side] = (fingers, across)
            # Palm side: where the reference fingers already curl (the skeleton is mirrored, so it differs per side).
            m1, m2, m3 = (sk.pos("middle_%02d_%s" % (k, side)) for k in (1, 2, 3))
            along = normalize(sub(m2, m1))
            curl = sub(sub(m3, m2), scale(along, dot(along, sub(m3, m2))))
            palm = normalize(cross(fingers, across))
            if dot(palm, curl) < 0.0:
                palm = scale(palm, -1.0)
            self.palm[side] = palm
            # Grip point in the hand's frame: in the closed fingers, just past the knuckles on the palm side.
            grip = add(lerp(sk.pos(hand), knuckle, 0.85), scale(palm, 2.5))
            self.grip_local[side] = qrot(qconj(sk.rot(hand)), sub(grip, sk.pos(hand)))
            # Closed hand: each finger joint turns toward the palm by the gesture's angles (constant, local).
            for finger in FINGERS:
                for k in (1, 2, 3):
                    bone = "%s_%02d_%s" % (finger, k, side)
                    child = "%s_%02d_%s" % (finger, k + 1, side) if k < 3 else None
                    start = sk.pos(bone) if child else sk.pos("%s_02_%s" % (finger, side))
                    end = sk.pos(child) if child else sk.pos(bone)
                    axis = cross(sub(end, start), palm)
                    i = sk.index[bone]
                    delta = qaxis(axis, curl_angles[k - 1])
                    parent_ref = sk.ref_world[sk.parent[i]][1]
                    self.fist[i] = qnormalize(qmul(qconj(parent_ref), qmul(delta, sk.ref_world[i][1])))
        stance = gesture.get("stance", {})
        self.feet = {}
        for side in ("l", "r"):
            foot = "foot_" + side
            offset = sk.body(stance.get(side, (0.0, 0.0, 0.0)))
            self.feet[side] = (add(sk.pos(foot), offset), sk.rot(foot))
        keys = gesture["keys"]
        if any(keys[i + 1]["t"] <= keys[i]["t"] for i in range(len(keys) - 1)):
            raise ValueError("Keys must be in increasing time")
        self.key_span = keys[-1]["t"] - keys[0]["t"]
        self.has_tool = any("tool" in key or "axe" in key for key in keys)
        self._strike = None
        if base is not None:
            # The source animation sets the length and the frames; the key times are stretched onto it.
            self.length = base["length"]
            self.fps = base["fps"]
            self.times = list(base["times"])
            self.base_index = {name: sk.index[name] for name in base["bones"]}
            # How far forward each foot is at every source key, around its mean (drives the arm swing).
            self.base_feet = {"l": [], "r": []}
            for k in range(len(self.times)):
                local = list(sk.ref_local)
                for name, i in self.base_index.items():
                    row = base["bones"][name][k]
                    local[i] = (tuple(row[:3]), tuple(row[3:]))
                world = sk.fk(local)
                for side in ("l", "r"):
                    self.base_feet[side].append(dot(sk.pos("foot_" + side, world), sk.forward))
            for side in ("l", "r"):
                mean = sum(self.base_feet[side]) / len(self.base_feet[side])
                self.base_feet[side] = [f - mean for f in self.base_feet[side]]
        else:
            self.length = self.key_span
            self.fps = gesture["fps"]
            self.times = frame_times(self.length, self.fps)

    # -- key interpolation ----------------------------------------------------------------------------------------

    def sample(self, t):
        """Interpolated key values at time t (seconds from the first key)."""
        keys = self.g["keys"]
        t = keys[0]["t"] + max(0.0, min(self.length, t)) * self.key_span / self.length
        for i in range(len(keys) - 1):
            if t <= keys[i + 1]["t"] or i == len(keys) - 2:
                a, b = keys[i], keys[i + 1]
                break
        s = (t - a["t"]) / (b["t"] - a["t"])
        s = EASES[b.get("ease", "inout")](max(0.0, min(1.0, s)))
        zero = (0.0, 0.0, 0.0)
        out = {"pelvis": lerp(a.get("pelvis", zero), b.get("pelvis", zero), s), "rot": {}}
        for bone in set(a.get("rot", {})) | set(b.get("rot", {})):
            out["rot"][bone] = lerp(a.get("rot", {}).get(bone, zero), b.get("rot", {}).get(bone, zero), s)
        tool_a, tool_b = a.get("tool", a.get("axe")), b.get("tool", b.get("axe"))
        if tool_a is not None:
            out["grip"] = lerp(tool_a["grip"], tool_b["grip"], s)
            out["dir"] = slerp_dir(tool_a["dir"], tool_b["dir"], s)
            spacing = (tool_a.get("spacing", self.hand_spacing), tool_b.get("spacing", self.hand_spacing))
            out["spacing"] = spacing[0] + (spacing[1] - spacing[0]) * s
        free = ("l", "r") if tool_a is None else tuple(side for side in ("l", "r") if side not in self.tool_hands)
        if free:
            out["hands"] = {}
            for side in free:
                ha, hb = a["hands"][side], b["hands"][side]
                hand = {"at": lerp(ha["at"], hb["at"], s), "palm": slerp_dir(ha["palm"], hb["palm"], s),
                        "swing": ha.get("swing", 0.0) + (hb.get("swing", 0.0) - ha.get("swing", 0.0)) * s}
                if "fingers" in ha and "fingers" in hb:
                    hand["fingers"] = slerp_dir(ha["fingers"], hb["fingers"], s)
                out["hands"][side] = hand
        default_elbows = self.g.get("elbows", {"l": (-0.5, -0.2, -1.0), "r": (0.5, -0.2, -1.0)})
        out["elbows"] = {}
        for side in ("l", "r"):
            ea = a.get("elbows", default_elbows).get(side, default_elbows.get(side))
            eb = b.get("elbows", default_elbows).get(side, default_elbows.get(side))
            out["elbows"][side] = normalize(lerp(ea, eb, s))
        return out

    # -- solving --------------------------------------------------------------------------------------------------

    def axe(self, values):
        """Component-space tool of a sampled pose: grip points of the hands on it, knob, head, handle direction."""
        sk = self.sk
        direction = normalize(sk.body(values["dir"]))
        if self.tool_hands == ("r",):
            # One-handed: "grip" is the right hand's place, knob_offset below it.
            grip_r = sk.body(values["grip"])
            knob = sub(grip_r, scale(direction, self.knob_offset))
            head = add(knob, scale(direction, self.handle_length))
            return {"grip_r": grip_r, "knob": knob, "head": head, "dir": direction, "trace": head}
        grip_l = sk.body(values["grip"])
        grip_r = add(grip_l, scale(direction, values["spacing"]))
        knob = sub(grip_l, scale(direction, self.knob_offset))
        head = add(knob, scale(direction, self.handle_length))
        return {"grip_l": grip_l, "grip_r": grip_r, "knob": knob, "head": head, "dir": direction, "trace": head}

    def _set_world_rotation(self, local, world, i, rotation):
        """Writes bone i's local rotation so that its component-space rotation becomes `rotation`."""
        parent_rot = world[self.sk.parent[i]][1]
        local[i] = (local[i][0], qnormalize(qmul(qconj(parent_rot), rotation)))

    def _two_bone(self, local, chain, target, pole, end_rotation):
        """Two-bone IK: places chain[2] at `target`, bending toward `pole`, then gives chain[2] `end_rotation`
        (component space). Returns the solved world."""
        sk = self.sk
        a, b, c = (sk.index[n] for n in chain)
        world = sk.fk(local)
        ref_a, ref_b, ref_c = sk.ref_world[a][0], sk.ref_world[b][0], sk.ref_world[c][0]
        l1, l2 = length(sub(ref_b, ref_a)), length(sub(ref_c, ref_b))
        root = world[a][0]
        to_target = sub(target, root)
        d = max(abs(l1 - l2) + 0.1, min((l1 + l2) * 0.9995, length(to_target)))
        direction = normalize(to_target)
        along = (l1 * l1 - l2 * l2 + d * d) / (2.0 * d)
        height = math.sqrt(max(0.0, l1 * l1 - along * along))
        bend = normalize(sub(pole, scale(direction, dot(pole, direction))))
        mid = add(add(root, scale(direction, along)), scale(bend, height))
        end = add(root, scale(direction, d))

        # Reference bend: where the middle joint sits off the root-to-end line in the reference pose.
        ref_dir = normalize(sub(ref_c, ref_a))
        ref_off = sub(sub(ref_b, ref_a), scale(ref_dir, dot(sub(ref_b, ref_a), ref_dir)))
        ref_bend = normalize(ref_off) if length(ref_off) > 0.5 else self._ref_pole(chain)
        ref_normal = cross(ref_dir, ref_bend)
        new_normal = cross(direction, bend)

        r_upper = qframes(sub(ref_b, ref_a), ref_normal, sub(mid, root), new_normal)
        self._set_world_rotation(local, world, a, qmul(r_upper, sk.ref_world[a][1]))
        world = sk.fk(local)
        r_lower = qframes(sub(ref_c, ref_b), ref_normal, sub(end, mid), new_normal)
        self._set_world_rotation(local, world, b, qmul(r_lower, sk.ref_world[b][1]))
        world = sk.fk(local)
        self._set_world_rotation(local, world, c, end_rotation)
        return sk.fk(local)

    def _ref_pole(self, chain):
        """Bend direction used when a reference limb is straight: knees forward, elbows backward."""
        return self.sk.forward if chain[0].startswith("thigh") else scale(self.sk.forward, -1.0)

    def _hand_rotation(self, side, shoulder, grip, handle):
        """Hand gripping the handle: across-the-palm axis along the handle (index side toward the head), fingers
        pointing away from the shoulder, square to the handle (a relaxed wrist; known before solving the arm)."""
        sk = self.sk
        fingers0, across0 = self.hand_axes[side]
        reach = sub(grip, shoulder)
        fingers = sub(reach, scale(handle, dot(reach, handle)))
        # The reference frame is built across-first so the handle direction is matched exactly.
        r = qframes(across0, fingers0, handle, fingers)
        return qmul(r, sk.rot("hand_" + side))

    def _free_hand_rotation(self, side, shoulder, grip, palm, fingers=None):
        """Free hand: palm facing `palm`; fingers along `fingers` if given, else pointing away from the shoulder."""
        sk = self.sk
        fingers0 = self.hand_axes[side][0]
        if fingers is None:
            fingers = sub(grip, shoulder)
        r = qframes(self.palm[side], fingers0, palm, fingers)
        return qmul(r, sk.rot("hand_" + side))

    def _base_key(self, t):
        """Source keys around time t and the blend between them."""
        times = self.times
        k0 = max([i for i in range(len(times) - 1) if times[i] <= t + 1e-6] or [0])
        k1 = k0 + 1
        return k0, k1, max(0.0, min(1.0, (t - times[k0]) / (times[k1] - times[k0])))

    def _base_locals(self, local, t):
        """Copies the source animation's local transforms (interpolated at t) onto its bones."""
        k0, k1, s = self._base_key(t)
        for name, i in self.base_index.items():
            a, b = self.base["bones"][name][k0], self.base["bones"][name][k1]
            local[i] = (lerp(a[:3], b[:3], s), qslerp(tuple(a[3:]), tuple(b[3:]), s))

    def pose(self, t):
        """Local (translation, rotation) of every bone at time t, and the tool placement (for a tool gesture also
        "edge" and "frame", and the tool bone's local transform written into the pose)."""
        local, axe = self._solve(t)
        if "dir" in axe:
            frame = self.tool_frame(local, axe)
            axe["frame"] = frame
            axe["edge"] = qrot(frame[1], (0.0, 1.0, 0.0))
            sk = self.sk
            i = sk.index[TOOL_BONE]
            parent = sk.ref_world[sk.parent[i]]  # ik_hand_root: never keyed, its reference is its pose
            local[i] = (qrot(qconj(parent[1]), sub(frame[0], parent[0])), qnormalize(qmul(qconj(parent[1]), frame[1])))
        return local, axe

    def strike(self):
        """(hit frame index, striking edge in the right hand's frame) of a tool gesture: the hit is the frame where
        the head stops hardest (largest drop of speed), the edge the direction the head moves in just before it."""
        if self._strike is None and "edge" in self.g.get("tool", {}):
            # A weapon carried still: no hit, the edge given in body terms at the first frame.
            local, _ = self._solve(self.times[0])
            hand_rot = self.sk.fk(local)[self.sk.index["hand_r"]][1]
            self._strike = (0, qrot(qconj(hand_rot), normalize(self.sk.body(self.g["tool"]["edge"]))))
        if self._strike is None:
            hand = self.sk.index["hand_r"]
            heads, hand_rots = [], []
            for t in self.times:
                local, axe = self._solve(t)
                heads.append(axe["head"])
                hand_rots.append(self.sk.fk(local)[hand][1])
            speeds = [length(sub(heads[i + 1], heads[i])) * self.fps for i in range(len(heads) - 1)]
            hit = max(range(1, len(speeds)), key=lambda i: speeds[i - 1] - speeds[i])
            self._strike = (hit, qrot(qconj(hand_rots[hit]), sub(heads[hit], heads[hit - 1])))
        return self._strike

    def tool_frame(self, local, axe):
        """Component-space tool frame (knob, rotation): X along the handle, Y the striking edge, fixed in the right
        hand and square to the handle."""
        world = self.sk.fk(local)
        edge = qrot(world[self.sk.index["hand_r"]][1], self.strike()[1])
        d = axe["dir"]
        edge = normalize(sub(edge, scale(d, dot(edge, d))))
        return axe["knob"], qfrom_matrix((d, edge, cross(d, edge)))

    def _solve(self, t):
        """Body solve at time t: local transforms of every bone and the tool (or free hands) placement."""
        sk = self.sk
        values = self.sample(t)
        local = list(sk.ref_local)
        for i, rotation in self.fist.items():
            local[i] = (local[i][0], rotation)
        if self.base is not None:
            self._base_locals(local, t)

        # Pelvis offset (body terms, component space) and spine rotations, applied in the reference pose's axes.
        pelvis = sk.index["pelvis"]
        world = sk.fk(local)
        parent_rot = world[sk.parent[pelvis]][1]
        offset = qrot(qconj(parent_rot), sk.body(values["pelvis"]))
        local[pelvis] = (add(local[pelvis][0], offset), local[pelvis][1])
        for bone in SPINE_BONES:
            if bone not in values["rot"] or (self.base is not None and bone in self.base_index):
                continue
            i = sk.index[bone]
            delta = sk.body_rotation(*values["rot"][bone])
            parent_ref = sk.ref_world[sk.parent[i]][1]
            # local = P0^-1 * D * W0: the delta acts in the reference axes and follows the parents' motion.
            local[i] = (local[i][0], qnormalize(qmul(qconj(parent_ref), qmul(delta, sk.ref_world[i][1]))))

        # Legs: feet planted on their place, knees bending forward (carried by the pelvis twist); with a base, the
        # source's legs are kept as they are.
        world = sk.fk(local)
        pelvis_motion = qmul(world[pelvis][1], qconj(sk.ref_world[pelvis][1]))
        if self.base is None:
            for side, chain in LEG_CHAINS.items():
                target, foot_rot = self.feet[side]
                pole = qrot(pelvis_motion, add(sk.forward, scale(sk.right, 0.25 if side == "r" else -0.25)))
                self._two_bone(local, chain, target, pole, foot_rot)
        # Steady chest: takes back part of the pelvis' own swing (a carrier keeps his load still).
        steady = self.g.get("steady_chest", 0.0)
        if steady > 0.0:
            chest = sk.index["spine_03"]
            world = sk.fk(local)
            current = world[chest][1]
            self._set_world_rotation(local, world, chest,
                                     qslerp(current, qnormalize(qmul(qconj(pelvis_motion), current)), steady))

        # Steady spine: the pelvis' motion taken back progressively along the spine.
        spread = self.g.get("steady_spine", 0.0)
        if spread > 0.0:
            world = sk.fk(local)
            spine = ("spine_01", "spine_02", "spine_03")
            original = {bone: world[sk.index[bone]][1] for bone in spine}
            for k, bone in enumerate(spine):
                world = sk.fk(local)
                target = qnormalize(qmul(qconj(pelvis_motion), original[bone]))
                self._set_world_rotation(local, world, sk.index[bone],
                                         qslerp(original[bone], target, spread * (k + 1) / 3.0))

        # Arms: both hands on the handle; hand rotation first, then the wrist where that grip puts it.
        world = sk.fk(local)
        chest = sk.index["spine_03"]
        chest_motion = qmul(world[chest][1], qconj(sk.ref_world[chest][1]))
        axe = self.axe(values) if "grip" in values else {}
        follow = self.g.get("hands_frame") == "chest"
        chest_ref = sk.ref_world[chest][0]
        if follow and "grip" in values:
            # The tool follows the chest's motion (position only, its direction stays as keyed).
            moved = sub(add(world[chest][0], qrot(chest_motion, sub(axe["grip_r"], chest_ref))), axe["grip_r"])
            for k in ("grip_l", "grip_r", "knob", "head", "trace"):
                if k in axe:
                    axe[k] = add(axe[k], moved)
        if "hands" in values:
            # Free hands, in body terms; "hands_frame": "chest" makes them follow the chest (bob, sway).
            for side in values["hands"]:
                hand = values["hands"][side]
                at, palm = sk.body(hand["at"]), sk.body(hand["palm"])
                if hand["swing"] and self.base is not None:
                    # Arm swing: the hand goes forward when the foot on the same side goes back.
                    k0, k1, blend = self._base_key(t)
                    foot = self.base_feet[side][k0] + (self.base_feet[side][k1] - self.base_feet[side][k0]) * blend
                    at = add(at, scale(sk.forward, -hand["swing"] * foot))
                fingers = sk.body(hand["fingers"]) if "fingers" in hand else None
                if follow:
                    at = add(world[chest][0], qrot(chest_motion, sub(at, chest_ref)))
                    palm = qrot(chest_motion, palm)
                    fingers = qrot(chest_motion, fingers) if fingers else None
                axe["grip_" + side] = at
                axe["palm_" + side] = palm
                axe["fingers_" + side] = fingers
            if "grip" not in values:
                axe["trace"] = axe["grip_r"]
        for side, chain in ARM_CHAINS.items():
            grip = axe["grip_" + side]
            pole = qrot(chest_motion, sk.body(values["elbows"][side]))
            shoulder = world[sk.index[chain[0]]][0]
            if "dir" in axe and side in self.tool_hands:
                rotation = self._hand_rotation(side, shoulder, grip, axe["dir"])
            else:
                rotation = self._free_hand_rotation(side, shoulder, grip, axe["palm_" + side], axe["fingers_" + side])
            wrist = sub(grip, qrot(rotation, self.grip_local[side]))
            self._two_bone(local, chain, wrist, pole, rotation)
        return local, axe

    def grip_errors(self, world, axe):
        """Distance (cm) between each hand's grip point and its place on the handle (IK out of reach if large)."""
        sk = self.sk
        out = {}
        for side in ("l", "r"):
            hand = sk.index["hand_" + side]
            grip = add(world[hand][0], qrot(world[hand][1], self.grip_local[side]))
            out[side] = length(sub(grip, axe["grip_" + side]))
        return out

    def handle_distances(self, world):
        """Distance (cm) from the grip point of each hand on the tool to the handle's axis as carried by the tool bone
        (the segment from the knob to the head along the bone's X): the tool really stays in the hands if ~0."""
        sk = self.sk
        knob, rot = world[sk.index[TOOL_BONE]]
        along = qrot(rot, (1.0, 0.0, 0.0))
        out = {}
        for side in self.tool_hands:
            hand = sk.index["hand_" + side]
            grip = add(world[hand][0], qrot(world[hand][1], self.grip_local[side]))
            s = max(0.0, min(self.handle_length, dot(sub(grip, knob), along)))
            out[side] = length(sub(grip, add(knob, scale(along, s))))
        return out


def load_base(gesture, data_dir):
    """The sampled source animation of a gesture (written by make_peasant_animations.py), or None."""
    if "base" not in gesture:
        return None
    path = os.path.join(data_dir, gesture["base"].rsplit("/", 1)[-1] + ".json")
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def frame_times(length, fps):
    """Times of every key of a sequence of `length` seconds at `fps` (frames + 1 keys, last = length)."""
    frames = int(round(length * fps))
    return [length * i / frames for i in range(frames + 1)]
