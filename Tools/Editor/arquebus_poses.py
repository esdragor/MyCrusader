"""Gestures of the Arquebusier (Retours/Q-arquebusier-modele.md, way A validated by the user on 2026-11-14): the shot with
its recoil, the reload with the RAMROD, idle and walk with the gun on the shoulder, the fall (the Crossbowman's). Pure
Python, no PIL (the Unreal baker imports it through soldier_poses, which imports this file at its end and receives
GESTURES: arquebus_idle, arquebus_walk, arquebus_attack, arquebus_fall).

Pieces and frames: arquebus_props.py (the gun in the crossbow's tool frame, X from the muzzle side toward the butt, Y the
top of the stock, Z across; hands at CROSSBOW_LEFT_GRIP and 45 cm behind). The ramrod moves on its own in the reload:
  - "ramrod_stowed" (ik_hand_r, follows the gun) is hidden while the rod is drawn ("bolt_hidden", like the crossbow's
    bolt);
  - "ramrod_drawn" (ik_hand_l, a bone never used otherwise) takes the right hand's frame by an "extra_bones" track
    (rod_track) and is shown only while the rod is drawn ("rod_hidden": its bone scaled to HIDDEN_SCALE otherwise;
    True = always, in the idle, the walk and the fall).

Attack, AttackInterval 4 s (UnitStats.md), the shot at 12 % (frame 15 of 120, ARQ_SHOT = 0.125 like the crossbow's
0.125 for 12 %): raise to the shoulder, aim, SHOT with a hard recoil, the gun let down by the left hand, its butt on the
ground (muzzle up), then the drill: a charge from the bandolier poured in the muzzle, the ball dropped in, the ramrod
drawn (pulled up its whole length), turned over the muzzle, rammed twice, drawn again, stowed; the gun lifted back to
the low carry. The powder charge and the ball are mimed (no piece in the hand: like the crossbow's bolt, AR1).
Previews: arquebus_preview.py (sheets, closeups), arquebus_new_gif.py (the GIF with the candidates' letters).
"""
import math
import os
import sys

sys.dont_write_bytecode = True  # no __pycache__ in Tools/Editor
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import anim_rig  # noqa: E402
import arquebus_props as apr  # noqa: E402
import soldier_poses as sg  # noqa: E402
import soldier_props as sp  # noqa: E402

SKELETON_JSON = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Data", "UE4MannequinSkeleton.json")
SK = anim_rig.Skeleton.load(SKELETON_JSON)
norm = anim_rig.normalize

# Ramrod geometry (arquebus_props)
MUZZLE_X, ROD_LEN, ROD_TAIL = apr.MUZZLE_X, apr.ROD_LEN, apr.ROD_TAIL
ROD_PIPE_Y, BORE_Y, GRAB_X = apr.ROD_PIPE_Y, apr.BORE_Y, apr.GRAB_X


def _body_of(v):
    return (anim_rig.dot(v, SK.right), anim_rig.dot(v, SK.forward), anim_rig.dot(v, SK.up))


def rod_hand(at, rod, fingers_hint):
    """The right hand gripping a rod: its grip point at `at` (body terms), the rod's axis (head side) along `rod`, the
    fingers square to the rod toward `fingers_hint`. Returns the free hand dict (palm, fingers in body terms): the
    rig's hand frame (palm, fingers, cross(palm, fingers)) carries the rod's axis (arquebus_props.rod_local)."""
    r = SK.body(norm(rod))
    f = SK.body(fingers_hint)
    f = norm(anim_rig.sub(f, anim_rig.scale(r, anim_rig.dot(f, r))))
    palm = norm(anim_rig.cross(f, r))
    return {"at": tuple(at), "palm": _body_of(palm), "fingers": _body_of(f)}


ROD_LOCAL = apr.rod_local(SK)


def rod_track(sk, times, worlds, rig):
    """The "extra_bones" track of ik_hand_l: the rod's frame (the right hand's, carrying rod_local) on every frame."""
    out = []
    lt, lq = ROD_LOCAL
    for world in worlds:
        ht, hq = world[sk.index["hand_r"]]
        out.append((anim_rig.add(ht, anim_rig.qrot(hq, lt)), anim_rig.qnormalize(anim_rig.qmul(hq, lq))))
    return out


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

PATH_OUT = (6.0, 10.0, 0.0)  # the pouch-to-muzzle hand path's middle, pushed out (right, forward, up)
HAND_SWAP_GRAB = at(P, T_GRAB)
HAND_SWAP_STOW = at(P, T_STOWED)


def drill_keys():
    keys = []
    add = keys.append
    add(reload_key(at(P, T_GROUND), B_GUARD, R_HIP, ELB_GROUND, flow=1.0))
    out = hand_mix(R_HIP, R_CHARGE, 0.5)  # hip to the strap: round the front of the belly, not through it
    out["at"] = (out["at"][0] + 8.0, out["at"][1] + 8.0, out["at"][2])
    add(reload_key(at(P, (T_GROUND + T_CHARGE) / 2.0), B_GUARD, out, ELB_CHARGE))
    add(reload_key(at(P, T_CHARGE), B_GUARD, R_CHARGE, ELB_CHARGE))
    away = hand_mix(R_CHARGE, R_OVER_MUZZLE, 0.5)  # strap to muzzle: out and forward round the chest
    away["at"] = (away["at"][0] + 6.0, away["at"][1] + 10.0, away["at"][2] + 2.0)
    add(reload_key(at(P, (T_CHARGE + T_POUR) / 2.0), B_GUARD, away, ELB_UP))
    add(reload_key(at(P, T_POUR), B_GUARD, R_OVER_MUZZLE, ELB_UP))
    add(reload_key(at(P, T_POUR2), B_GUARD, R_TIPPED, ELB_UP))
    down = hand_mix(R_TIPPED, R_POUCH, 0.5)  # muzzle to the pouch at the right hip: out to his right
    down["at"] = (down["at"][0] + 8.0, down["at"][1] + 8.0, down["at"][2])
    add(reload_key(at(P, (T_POUR2 + T_POUCH) / 2.0), B_GUARD, down, ELB_GROUND))
    add(reload_key(at(P, T_POUCH), B_GUARD, R_POUCH, ELB_GROUND))
    # Between the pouch and the muzzle the hand goes round the front of the belly (the straight way ran 4 cm into the trunk).
    path = hand_mix(R_POUCH, R_BELOW_MUZZLE, 0.5)
    path["at"] = (path["at"][0] + PATH_OUT[0], path["at"][1] + PATH_OUT[1], path["at"][2] + PATH_OUT[2])
    add(reload_key(at(P, (T_POUCH + T_BALL) / 2.0), B_GUARD, path, ELB_UP))
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


FPS = 30
PROPS = ("arquebus_gun", "arquebus_bandolier", "ramrod_stowed", "ramrod_drawn")
# The rod's swap, on whole frames: the stowed rod is hidden and the drawn one shown from frame ROD_IN to ROD_OUT (the hand
# takes the tail at ROD_IN's neighbourhood and puts it back at ROD_OUT's: the two coincide there, no jump).
ROD_IN, ROD_OUT = 63, 103


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
    g = sg.fighting("A_Arquebus_Attack", sg.CROSSBOW_TOOL, PROPS, keys, sg.CROSSBOW_ELBOWS, FPS, sg.CROSSBOW_STANCE,
                    [0, 10, 15, 18, 30, 45, 62, 78, 92, 105])
    stowed_hidden = [(ROD_IN / float(FPS), ROD_OUT / float(FPS))]
    drawn_hidden = [(0.0, (ROD_IN - 1) / float(FPS)), ((ROD_OUT + 1) / float(FPS), P)]
    g.update({"frames": 120, "curve": "hermite", "loop": True, "leads": ARQUEBUS_LEADS, "hit_fraction": ARQ_SHOT,
              "release": [(at(P, 0.185), at(P, T_CARRY))],
              "bolt_hidden": stowed_hidden,      # the baker: ik_hand_r (the stowed rod) at HIDDEN_SCALE
              "rod_hidden": drawn_hidden,        # the baker: ik_hand_l (the drawn rod) at HIDDEN_SCALE
              "extra_bones": {"ik_hand_l": rod_track}})
    g["hidden"] = {"ramrod_stowed": stowed_hidden, "ramrod_drawn": drawn_hidden}  # the preview's
    return g


ARQUEBUS_ATTACK = build_attack()

# Idle and walk with the gun ON THE SHOULDER (the user's option, Q-arquebusier-modele): the right hand holds the
# stock's wrist (67 cm from the muzzle side) at the hip, the barrel rises back over the right shoulder, the muzzle behind
# his head; the left hand free (hanging at rest, swinging in the walk). One-handed tool, like the Spearman's spear.
SHOULDER_TOOL = {"kind": "crossbow", "hands": 1, "handle_length": sp.CROSSBOW_LENGTH, "knob_offset": 67.0,
                 "edge": (0.0, 0.0, 1.0)}
SHOULDER_GRIP = {"grip": (21.0, 12.0, 101.0), "dir": norm((0.04, 0.36, -0.93))}
SHOULDER_ELBOWS = {"l": (-0.2, -1.0, -0.4), "r": (0.6, -0.6, -0.6)}
PROPS_CARRY = ("arquebus_gun", "arquebus_bandolier", "ramrod_stowed")  # the drawn rod is hidden ("rod_hidden")
ARQUEBUS_IDLE = sg.armed("A_Arquebus_Idle", sg.IDLE, SHOULDER_TOOL, PROPS_CARRY,
                         sg.idle_keys(sg.STAND, SHOULDER_GRIP, sg.LEFT_HANGING), SHOULDER_ELBOWS, {"steady_spine": 1.0})
ARQUEBUS_WALK = sg.armed("A_Arquebus_Walk", sg.UNARMED_WALK, SHOULDER_TOOL, PROPS_CARRY,
                         sg.walk_keys(sg.STAND, SHOULDER_GRIP, sg.LEFT_SWINGING), SHOULDER_ELBOWS,
                         {"steady_chest": 0.5})
# The fall: the Crossbowman's (the gun escapes his hands, the rod stays stowed).
ARQUEBUS_FALL = dict(sg.CROSSBOW_FALL, name="A_Arquebus_Fall", props=PROPS_CARRY)
for gesture in (ARQUEBUS_IDLE, ARQUEBUS_WALK, ARQUEBUS_FALL):
    gesture["rod_hidden"] = True
GESTURES = {"arquebus_idle": ARQUEBUS_IDLE, "arquebus_walk": ARQUEBUS_WALK, "arquebus_attack": ARQUEBUS_ATTACK,
            "arquebus_fall": ARQUEBUS_FALL}
sg.GESTURES.update(GESTURES)  # soldier_poses imports this file at its end; the baker reads soldier_poses.GESTURES
