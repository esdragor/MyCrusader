"""NEW gestures of the Arquebusier (hors jeu, no Unreal, nothing baked): the shot with its recoil, the reload with the
RAMROD, a walk with the gun on the shoulder. Retours/Q-arquebusier-modele.md: the user saw the Crossbowman's gestures
re-used (the stirrup reload is wrong for a firearm) and asked for real gestures of a firearm.

Pieces and frames are those of arquebus_sheet.py (the gun in the crossbow's tool frame: X from the muzzle side toward the
butt, Y the top of the stock, Z across; hands at CROSSBOW_LEFT_GRIP and 45 cm behind). The reload needs the ramrod to
move on its own, so it is two props of this file:
  - "ramrod_stowed": under the forestock, bound to ik_hand_r like the crossbow's loaded bolt (a child of ik_hand_gun
    never keyed: it follows the gun exactly; the baker will scale ik_hand_r to nothing while the rod is drawn);
  - "ramrod_drawn": in the right hand (bone hand_r, frame bone), its axis along the hand's "across" axis, its head
    (the end that goes into the barrel) on the axis' positive side.
The gestures hide one while the other shows ("hidden" spans of key time, the preview's; the baker has its own).

Attack, AttackInterval 4 s (UnitStats.md), the shot at 12 % (frame 15 of 120, ARQ_SHOT = 0.125 like the crossbow's
0.125 for 12 %): raise to the shoulder, aim, SHOT with a hard recoil, the gun let down by the left hand, its butt on the
ground (muzzle up), then the drill: a charge from the bandolier poured in the muzzle, the ball dropped in, the ramrod
drawn (pulled up its whole length), turned over the muzzle, rammed twice, drawn again, stowed; the gun lifted back to
the low carry. The powder charge and the ball are mimed (no piece in the hand: like the crossbow's bolt, AR1).
Run: python Tools/Editor/arquebus_poses.py [--frames N] [--zoom Z]   -> Saved/AnimPreview/arquebus_attack_sheet.png
"""
import math
import os
import sys

sys.dont_write_bytecode = True  # no __pycache__ in Tools/Editor
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import anim_rig  # noqa: E402
import arquebus_sheet as aq  # noqa: E402
import peasant_props as pp  # noqa: E402
import preview_animation as pa  # noqa: E402
import soldier_poses as sg  # noqa: E402
import soldier_props as sp  # noqa: E402

SK = anim_rig.Skeleton.load(pa.SKELETON_JSON)
norm = anim_rig.normalize

# ---------------------------------------------------------------------------------------------------------------------
# Ramrod geometry (cm, gun frame: x from the muzzle side toward the butt)
MUZZLE_X = aq.BUTT_X - aq.ARQ          # -15: the muzzle
ROD_LEN = 64.0                        # stowed from the muzzle's side
ROD_TAIL = 7.0                        # the hand's grip to the rod's tail end (the end that sticks out upward)
ROD_PIPE_Y = -2.4                     # stowed under the forestock (aq.make_gun)
BORE_Y = 2.6                          # the bore, on top of the stock
ROD_STOW_X = MUZZLE_X + 2.0           # the tail end when stowed (aq.make_gun's old ramrod began at muzzle + 2)
GRAB_X = ROD_STOW_X + ROD_TAIL        # the hand's grip when it holds the stowed rod's tail
ROD_RADIUS = 0.9


def _sign(x):
    return 1.0 if x >= 0.0 else -1.0


def _body_of(v):
    return (anim_rig.dot(v, SK.right), anim_rig.dot(v, SK.forward), anim_rig.dot(v, SK.up))


def hand_axes_right():
    fingers, across, palm, grip = sp.hand_axes(SK, "r")
    return fingers, across, palm, grip


def rod_hand(at, rod, fingers_hint):
    """The right hand gripping a rod: its grip point at `at` (body terms), the rod's axis (head side) along `rod`, the
    fingers square to the rod toward `fingers_hint`. Returns the free hand dict (palm, fingers in body terms). The hand's
    across axis (pinky to index) lies along the rod; the palm side follows from the reference hand's handedness."""
    fingers0, across0, palm0, _ = hand_axes_right()
    f0 = norm(anim_rig.sub(fingers0, anim_rig.scale(palm0, anim_rig.dot(fingers0, palm0))))
    r = SK.body(norm(rod))
    f = SK.body(fingers_hint)
    f = norm(anim_rig.sub(f, anim_rig.scale(r, anim_rig.dot(f, r))))
    palm = norm(anim_rig.cross(f, r))  # the hand's frame (palm, fingers, cross(palm, fingers)) carries the rod's axis
    return {"at": tuple(at), "palm": _body_of(palm), "fingers": _body_of(f)}


def ramrod_prop_local():
    """The ramrod's frame in hand_r's reference frame: origin at the grip point, X along the hand's across axis."""
    fingers0, across0, palm0, grip0 = hand_axes_right()
    bt, bq = SK.ref_world[SK.index["hand_r"]]
    to_local = lambda v: anim_rig.qrot(anim_rig.qconj(bq), v)  # noqa: E731
    f0 = norm(anim_rig.sub(fingers0, anim_rig.scale(palm0, anim_rig.dot(fingers0, palm0))))
    x = to_local(anim_rig.cross(palm0, f0))  # exactly square to the rig's palm and fingers (across0 is a little skewed)
    q = pp.rotation_xz(x, to_local(palm0))
    return (to_local(anim_rig.sub(grip0, bt)), anim_rig.qnormalize(q))


def make_ramrod_prims(origin_x, light=False):
    """A ramrod along +X from its tail (origin_x) to its head: the rod, a cup at the head, a knob at the tail."""
    sides = 4 if light else 6
    x0, x1 = origin_x, origin_x + ROD_LEN
    return [
        pp.frustum_between((x0, 0.0, 0.0), (x1 - 4.0, 0.0, 0.0), (ROD_RADIUS, ROD_RADIUS), (ROD_RADIUS, ROD_RADIUS),
                           sides, pp.WOOD_DARK, x_hint=pp.FLAT),
        pp.frustum_between((x1 - 4.0, 0.0, 0.0), (x1, 0.0, 0.0), (ROD_RADIUS * 1.5, ROD_RADIUS * 1.5),
                           (ROD_RADIUS * 1.2, ROD_RADIUS * 1.2), sides, sp.BRASS, x_hint=pp.FLAT),
        pp.frustum_between((x0 - 0.6, 0.0, 0.0), (x0 + 2.0, 0.0, 0.0), (ROD_RADIUS * 1.6, ROD_RADIUS * 1.6),
                           (ROD_RADIUS * 1.1, ROD_RADIUS * 1.1), sides, sp.BRASS, x_hint=pp.FLAT),
    ]


def make_ramrod_stowed():
    prims = make_ramrod_prims(ROD_STOW_X)
    for prim in prims:
        prim["t"] = anim_rig.add(prim["t"], (0.0, ROD_PIPE_Y, 0.0))
    return {"id": 5, "name": "ramrod_stowed", "label": "baguette rangee", "bone": "ik_hand_r", "frame": "bone",
            "local": sp.bolt_local(SK), "prims": prims}


def make_ramrod_drawn():
    return {"id": 4, "name": "ramrod_drawn", "label": "baguette en main", "bone": "hand_r", "frame": "bone",
            "local": ramrod_prop_local(), "prims": make_ramrod_prims(-ROD_TAIL)}


def register(shared):
    """The gun without its ramrod (the two ramrod props above), the smoke and the bandolier of arquebus_sheet, then
    the gestures (arq_attack ... under pa.ALL_GESTURES)."""
    aq.register(shared)
    pieces = [aq.make_gun("arquebus_r", aq.ARQ, 6.0, ramrod=False), make_ramrod_stowed(), make_ramrod_drawn()]
    for piece in pieces:
        shared.props[piece["name"]] = piece
        shared.prop_tris[piece["name"]] = pp.prop_triangles(piece)
    pa.ALL_GESTURES.update(GESTURES)


# ---------------------------------------------------------------------------------------------------------------------
# Tool and hand helpers (the crossbow's, soldier_poses)
cb_tool, cb_hand, cb_point, cb_axes = sg.cb_tool, sg.cb_hand, sg.cb_point, sg.cb_axes
mix, slerp = sg._cb_mix, sg._cb_slerp
GRIP_L, SPACING = sp.CROSSBOW_LEFT_GRIP, sp.CROSSBOW_HAND_SPACING
DOWN = sg.DOWN


def tool_mix(a, b, s):
    return cb_tool(mix(a["grip"], b["grip"], s), slerp(a["dir"], b["dir"], s), slerp(a["fingers"], b["fingers"], s))


def hand_mix(a, b, s):
    return {"at": mix(a["at"], b["at"], s), "palm": slerp(a["palm"], b["palm"], s),
            "fingers": slerp(a["fingers"], b["fingers"], s)}


def pose_mix(a, b, s, keys=("pelvis", "rot")):
    out = {k: mix(a[k], b[k], s) for k in keys}
    out["elbows"] = {side: norm(mix(a["elbows"][side], b["elbows"][side], s)) for side in ("l", "r")}
    return out


def free_key(t, pose, hands, flow=0.0):
    """A key of the released spans: the tool on its own path and both hands free (given)."""
    key = sg.merge({"elbows": sg.CROSSBOW_ELBOWS}, pose, {"t": t, "flow": flow, "hands": hands})
    return key


# ---------------------------------------------------------------------------------------------------------------------
# The shot: the crossbow's raise and aim (CB_*), a harder recoil
P = 4.0
ARQ_SHOT = 0.125
at = sg.at

AQ_SHOT = dict(sg.CB_AIM, tool=sg.cb_aim_tool((9.0, -15.5, 151.0)))
# The recoil: the shoulder driven back, the whole trunk rocking, the muzzle thrown up 14 degrees.
AQ_KICK = {"pelvis": (-0.5, -2.5, -3.5), "rot": sg.merge(sg.hip_turn(61.0, -3.0, 19.0, nod=3.0),
                                                        {"clavicle_r": (13.0, 0.0, 1.0), "clavicle_l": (14.0, 0.0, 0.0)}),
           "tool": sg.cb_aim_tool((10.0, -23.0, 153.5), sg.AIM_ELEVATION + 14.0), "elbows": sg.AIM_ELBOWS}
AQ_STEADY = {"pelvis": (-0.5, -0.5, -4.0), "rot": sg.merge(sg.hip_turn(59.0, 0.0, 19.0, nod=6.0),
                                                          {"clavicle_r": (7.0, 0.0, 2.0), "clavicle_l": (14.0, 0.0, 0.0)}),
             "tool": sg.cb_aim_tool((9.5, -19.0, 152.0), sg.AIM_ELEVATION + 7.0), "elbows": sg.AIM_ELBOWS}

# ---------------------------------------------------------------------------------------------------------------------
# The grounded gun: butt on the ground in front of him, muzzle up and leaning back toward him; its top (the bore)
# faces forward, so the ramrod's pipe (under the forestock) is on his side.
GROUND_BUTT = (8.0, 34.0, 0.5)
GROUND_DIR = norm((0.0, 0.04, -1.0))       # muzzle -> butt
GROUND_FINGERS = (0.0, -1.0, 0.0)          # the groove (the bore side) faces forward
GROUND_LEFT_X = -6.0                       # the left hand on the barrel, from the muzzle end


def ground_tool(butt=GROUND_BUTT, d=GROUND_DIR, fingers=GROUND_FINGERS):
    d = norm(d)
    return cb_tool(tuple(butt[k] - (sp.CROSSBOW_LENGTH - GRIP_L) * d[k] for k in range(3)), d, fingers)


G = ground_tool()


def gpoint(x, y=0.0, z=0.0, tool=None):
    return cb_point(tool or G, x, y, z)


def left_on(tool, x=GROUND_LEFT_X):
    return cb_hand(tool, "l", x)


# Right hand's free poses (body terms (right, forward, up); palm = where the palm faces, fingers = where they point).
R_HIP = {"at": (24.0, 6.0, 96.0), "palm": (-1.0, 0.1, 0.0), "fingers": (0.0, 0.2, -1.0)}
R_CHARGE = {"at": (-1.0, 20.0, 122.0), "palm": (0.1, -0.9, -0.3), "fingers": (-0.85, 0.2, 0.4)}   # takes a charge off the strap
R_POUCH = {"at": (23.0, 9.0, 92.0), "palm": (-0.9, 0.2, -0.3), "fingers": (0.0, 0.6, -0.8)}
R_BELOW_MUZZLE = {"at": gpoint(MUZZLE_X - 9.0, BORE_Y + 5.0), "palm": (0.0, 0.2, -1.0), "fingers": (0.0, 1.0, 0.1)}
R_OVER_MUZZLE = {"at": gpoint(MUZZLE_X - 7.0, BORE_Y + 4.0), "palm": (0.2, 0.6, -0.7), "fingers": (0.3, 1.0, -0.2)}
R_TIPPED = {"at": gpoint(MUZZLE_X - 8.0, BORE_Y + 2.0), "palm": (0.2, 0.9, -0.2), "fingers": (0.3, 0.7, -0.7)}

ROD_DOWN = norm(anim_rig.sub(gpoint(1.0), gpoint(0.0)))      # the rod's axis along the grounded gun (tail -> head)
ROD_FINGERS = (-1.0, 0.3, 0.0)   # the fingers wrap the rod toward his left: the wrist stays outside the chest


def rod_at(x_g, y=ROD_PIPE_Y, tool=None):
    """The right hand on the drawn rod, its grip at gun position x_g (the rod's axis along the gun's)."""
    t = tool or G
    return rod_hand(cb_point(t, x_g, y), cb_axes(t)[1], ROD_FINGERS)


# Bodies. The upper body looks down at the gun, a little turned to his left so the right arm is free.
def body(twist, bend, hip, nod, lean=(0.0, 0.0, 0.0)):
    return {"pelvis": lean, "rot": sg.hip_turn(twist, bend, hip, nod=nod)}


B_GUARD = body(-6.0, 28.0, -2.0, -8.0, (0.0, 0.0, -14.0))
B_REACH = body(-10.0, 22.0, -3.0, -2.0, (0.0, 0.0, -12.0))        # leaning back a little to draw the rod up
B_RAM = body(-6.0, 28.0, -2.0, -8.0, (0.0, 0.0, -14.0))

ELB_GROUND = {"l": norm((-0.6, -0.4, -0.7)), "r": norm((1.0, -0.1, -0.5))}
ELB_CHARGE = {"l": norm((-0.6, -0.4, -0.7)), "r": norm((0.7, -0.2, -0.7))}
ELB_UP = {"l": norm((-0.6, -0.4, -0.7)), "r": norm((1.0, 0.0, -0.1))}      # the right arm up: the elbow out and level


def reload_key(t, body_pose, hands_r, elbows, tool=None, x_l=GROUND_LEFT_X, flow=0.0):
    tool = tool or G
    pose = {"pelvis": body_pose["pelvis"], "rot": body_pose["rot"], "tool": tool, "elbows": elbows}
    return free_key(t, pose, {"l": left_on(tool, x_l), "r": hands_r}, flow)


# Lowering the gun: from the steady aim the gun swings about the left hand to the upright, the right hand leaving the
# stock for the hip (the release begins on the steady key, both hands on their grips).
SWING = []
for share, u in ((0.215, 0.18), (0.255, 0.45), (0.295, 0.78)):
    tool = tool_mix(AQ_STEADY["tool"], G, u)
    SWING.append((share, u, tool))


def swing_key(share, u, tool):
    bodies = pose_mix(dict(AQ_STEADY), dict(B_GUARD, elbows=ELB_GROUND), u)
    x_l = GRIP_L + (GROUND_LEFT_X - GRIP_L) * u
    hs = min(1.0, u * 1.4)
    right = hand_mix(cb_hand(AQ_STEADY["tool"], "r"), R_HIP, hs)
    right["at"] = (right["at"][0] + 12.0 * math.sin(math.pi * hs),) + tuple(right["at"][1:])  # out around the chest
    pose = {"pelvis": bodies["pelvis"], "rot": bodies["rot"], "tool": tool, "elbows": bodies["elbows"]}
    return free_key(at(P, share), pose, {"l": cb_hand(tool, "l", x_l), "r": right}, 1.0)


# ---------------------------------------------------------------------------------------------------------------------
# The drill. Times are shares of the loop.
T_GROUND, T_CHARGE, T_POUR, T_POUR2, T_POUCH, T_BALL, T_GRAB = 0.325, 0.365, 0.405, 0.425, 0.455, 0.490, 0.520
T_DRAWN, T_TURN, T_INSERT, T_RAM1, T_LIFT1, T_RAM2, T_OUT, T_OVER, T_SEAT, T_STOWED = (
    0.600, 0.625, 0.650, 0.690, 0.715, 0.750, 0.790, 0.820, 0.855, 0.865)
T_REGAIN1, T_REGAIN2, T_CARRY = 0.900, 0.935, 0.970

HAND_SWAP_GRAB = at(P, T_GRAB)
HAND_SWAP_STOW = at(P, T_STOWED)


def drill_keys():
    keys = []
    add = keys.append
    add(reload_key(at(P, T_GROUND), B_GUARD, R_HIP, ELB_GROUND, flow=1.0))
    add(reload_key(at(P, T_CHARGE), B_GUARD, R_CHARGE, ELB_CHARGE))
    add(reload_key(at(P, T_POUR), B_GUARD, R_OVER_MUZZLE, ELB_UP))
    add(reload_key(at(P, T_POUR2), B_GUARD, R_TIPPED, ELB_UP))
    add(reload_key(at(P, T_POUCH), B_GUARD, R_POUCH, ELB_GROUND))
    add(reload_key(at(P, T_BALL), B_GUARD, R_BELOW_MUZZLE, ELB_UP))
    add(reload_key(at(P, T_GRAB), B_GUARD, rod_at(GRAB_X), ELB_UP))                         # the tail of the stowed rod
    # Drawn: pulled up along the pipe its whole length, the body leaning back.
    add(reload_key(at(P, T_DRAWN), B_REACH, rod_at(GRAB_X - ROD_LEN - 6.0), ELB_UP))
    # Over the muzzle: the rod moves from the pipe line to the bore line, still clear above.
    add(reload_key(at(P, T_TURN), B_REACH, rod_at(MUZZLE_X - ROD_LEN + ROD_TAIL - 6.0, BORE_Y), ELB_UP))
    add(reload_key(at(P, T_INSERT), B_RAM, rod_at(MUZZLE_X - ROD_LEN + ROD_TAIL - 1.0, BORE_Y), ELB_UP))
    add(reload_key(at(P, T_RAM1), B_RAM, rod_at(MUZZLE_X - 4.0, BORE_Y), ELB_CHARGE, flow=1.0))
    add(reload_key(at(P, T_LIFT1), B_RAM, rod_at(MUZZLE_X - 32.0, BORE_Y), ELB_UP))
    add(reload_key(at(P, T_RAM2), B_RAM, rod_at(MUZZLE_X - 3.0, BORE_Y), ELB_CHARGE, flow=1.0))
    add(reload_key(at(P, T_OUT), B_REACH, rod_at(MUZZLE_X - ROD_LEN + ROD_TAIL - 4.0, BORE_Y), ELB_UP))
    add(reload_key(at(P, T_OVER), B_REACH, rod_at(MUZZLE_X - ROD_LEN + ROD_TAIL - 4.0, ROD_PIPE_Y), ELB_UP))
    add(reload_key(at(P, T_SEAT), B_GUARD, rod_at(GRAB_X + 1.0), ELB_UP, flow=1.0))
    add(reload_key(at(P, T_STOWED), B_GUARD, rod_at(GRAB_X), ELB_UP))
    return keys


# Regaining the gun: from the grounded gun to the low carry (the crossbow's CB_GRASP carry), the right hand back on
# the stock where the release ends.
CARRY = sg.CB_GRASP
REGAIN = []
for share, u in ((T_REGAIN1, 0.35), (T_REGAIN2, 0.72)):
    tool = tool_mix(G, CARRY["tool"], u)
    shift = (3.0, -9.0, 5.0)  # the gun comes up close to him (the left arm cannot reach the carry's 40 cm ahead early)
    tool["grip"] = tuple(tool["grip"][i] + shift[i] * math.sin(math.pi * u) for i in range(3))
    REGAIN.append((share, u, tool))


def regain_key(share, u, tool):
    bodies = pose_mix(dict(B_GUARD, elbows=ELB_GROUND), dict(CARRY), u ** 2)  # the body straightens behind the gun
    x_l = GROUND_LEFT_X + (GRIP_L - GROUND_LEFT_X) * u
    right = hand_mix(rod_at(GRAB_X), cb_hand(CARRY["tool"], "r"), min(1.0, u * 1.25))
    pose = {"pelvis": bodies["pelvis"], "rot": bodies["rot"], "tool": tool, "elbows": bodies["elbows"]}
    return free_key(at(P, share), pose, {"l": cb_hand(tool, "l", x_l), "r": right}, 1.0)


ARQUEBUS_LEADS = {"pelvis": 0.04, "rot": {"pelvis": 0.04, "spine_01": 0.03, "spine_02": 0.02, "spine_03": 0.012,
                                         "neck_01": 0.01, "head": 0.01, "clavicle_r": 0.006},
                  "elbows": 0.004, "tool": 0.0, "hands": 0.0}


def build_attack():
    keys = [
        sg.cb_key(0.0, sg.CB_READY, (0.0, 1.0)),
        sg.cb_key(at(P, 0.045), sg.CB_RAISE, 1.0),
        sg.cb_key(at(P, 0.075), sg.CB_RAISE_2, 1.0),
        sg.cb_key(at(P, 0.100), sg.CB_SHOULDER),
        sg.cb_key(at(P, 0.112), sg.CB_AIM),
        sg.cb_key(at(P, ARQ_SHOT), AQ_SHOT, (0.0, 2.0)),            # SHOT: frame 15 of 120
        sg.cb_key(at(P, 0.150), AQ_KICK, (0.0, 1.0)),               # the recoil
        sg.cb_key(at(P, 0.185), AQ_STEADY, 1.0),                   # steadied: the right hand lets go (release begins)
    ]
    keys += [swing_key(*s) for s in SWING]
    keys += drill_keys()
    keys += [regain_key(*r) for r in REGAIN]
    keys += [sg.cb_key(at(P, T_CARRY), CARRY, 0.5),               # both hands on the stock again (release ends)
             sg.cb_key(P, sg.CB_READY)]
    g = sg.fighting("A_Arquebus_Attack", sg.CROSSBOW_TOOL, ("arquebus_r", "bandolier", "ramrod_stowed", "ramrod_drawn"),
                    keys, sg.CROSSBOW_ELBOWS, 30, sg.CROSSBOW_STANCE, [0, 10, 15, 18, 30, 45, 62, 78, 92, 105])
    g.update({"frames": 120, "curve": "hermite", "loop": True, "leads": ARQUEBUS_LEADS, "hit_fraction": ARQ_SHOT,
              "release": [(at(P, 0.185), at(P, T_CARRY))]})
    g["hidden"] = {"ramrod_stowed": [(HAND_SWAP_GRAB, HAND_SWAP_STOW)],
                   "ramrod_drawn": [(0.0, HAND_SWAP_GRAB), (HAND_SWAP_STOW, P)]}
    return g


def lists_as_spans(hidden):
    """The preview reads one (start, end) span per prop; this file's gestures give lists of spans."""
    return hidden


ARQUEBUS_ATTACK = build_attack()

# Idle and walk with the gun ON THE SHOULDER (the user's option, Q-arquebusier-modele): the right hand holds the
# stock's wrist (67 cm from the muzzle side) at the hip, the barrel rises back over the right shoulder, the muzzle behind
# his head; the left hand free (hanging at rest, swinging in the walk). One-handed tool, like the Spearman's spear.
SHOULDER_TOOL = {"kind": "crossbow", "hands": 1, "handle_length": sp.CROSSBOW_LENGTH, "knob_offset": 67.0,
                 "edge": (0.0, 0.0, 1.0)}
SHOULDER_GRIP = {"grip": (21.0, 12.0, 101.0), "dir": norm((0.04, 0.36, -0.93))}
SHOULDER_ELBOWS = {"l": (-0.2, -1.0, -0.4), "r": (0.6, -0.6, -0.6)}
PROPS_CARRY = ("arquebus", "bandolier")
ARQUEBUS_IDLE = sg.armed("A_Arquebus_Idle", sg.IDLE, SHOULDER_TOOL, PROPS_CARRY,
                         sg.idle_keys(sg.STAND, SHOULDER_GRIP, sg.LEFT_HANGING), SHOULDER_ELBOWS, {"steady_spine": 1.0})
ARQUEBUS_WALK = sg.armed("A_Arquebus_Walk", sg.UNARMED_WALK, SHOULDER_TOOL, PROPS_CARRY,
                         sg.walk_keys(sg.STAND, SHOULDER_GRIP, sg.LEFT_SWINGING), SHOULDER_ELBOWS,
                         {"steady_chest": 0.5})
GESTURES = {"arq_new_attack": ARQUEBUS_ATTACK, "arq_new_idle": ARQUEBUS_IDLE, "arq_new_walk": ARQUEBUS_WALK}


def main():
    args = sys.argv[1:]
    frames = int(args[args.index("--frames") + 1]) if "--frames" in args else 12
    zoom = float(args[args.index("--zoom") + 1]) if "--zoom" in args else 1.0
    shared = pa.Shared(False)
    register(shared)
    os.makedirs(pa.OUT_DIR, exist_ok=True)
    picks = [int(v) for v in args[args.index("--picks") + 1].split(",")] if "--picks" in args else None
    out = os.path.join(pa.OUT_DIR, "arquebus_attack_sheet.png")
    ctx = pa.render(shared, "arq_new_attack", frames, out, zoom, picks=picks)
    print("  " + pa.report(ctx))


if __name__ == "__main__":
    main()


def closeup(ctx, shared, picks, out, view_index=1, zoom=2.6):
    """One view (0 front, 1 side, 2 RTS) of the chosen frames, large: to judge a gesture by eye."""
    pa.set_zoom(zoom)
    view = pa.views(shared.sk)[view_index]
    canvas = pa.Canvas(pa.CELL_W * len(picks), pa.CELL_H + pa.LABEL_H)
    for col, frame in enumerate(picks):
        pa.draw_cell(canvas, shared, ctx, frame, col * pa.CELL_W, 0, view, "%.2fs f%d" % (ctx["rig"].times[frame], frame))
        if col:
            canvas.line((col * pa.CELL_W, 0), (col * pa.CELL_W, pa.CELL_H + pa.LABEL_H - 1), (210, 210, 205), 0.4)
    canvas.save(out)
    pa.set_zoom(1.0)
