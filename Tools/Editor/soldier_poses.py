"""Gestures of the soldiers (J7a.3): armed idle and armed walk of the Spearman and of the Man-at-arms.

Same format as peasant_poses.py (see anim_rig.py), body terms everywhere. The weapons are those of soldier_props.py:
the spear and the sword are one-handed tools held by the right hand (tool bone ik_hand_gun, fixed "edge": no hit),
the shield is skinned to hand_l, placed by the free left hand's palm and fingers.
  - Idles on A_SW_Idle (pelvis and legs copied, chest turned back to face forward, three calm breaths), like the
    Peasant's loaded idles.
  - Walks on A_SW_Unarmed_Walk (pelvis and legs copied; foot speed 168.2 cm/s, so WalkPlayRate = speed / 168.2 before
    the client's division by MeshScale: Spearman 583 -> 3.47, Man-at-arms 523 -> 3.11).
User's choices (2026-10-18): the spear upright in the right hand, butt near the ground at rest, point up, and leaning
45 degrees forward in the walk (point forward and up, butt behind); the Man-at-arms holds his sword upright in the right hand at rest, 45 degrees forward in the walk, the shield on the left arm in front.
"""
import math
import os
import sys

sys.dont_write_bytecode = True  # no __pycache__ in Tools/Editor
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from peasant_poses import IDLE, UNARMED_WALK, breathing, merge, torso  # noqa: E402
import soldier_props as sp  # noqa: E402

UP = (0.0, 0.0, 1.0)
SPEAR_TOOL = {"kind": "spear", "hands": 1, "handle_length": sp.SPEAR_LENGTH, "knob_offset": sp.SPEAR_GRIP,
              "edge": (1.0, 0.0, 0.0)}  # blade and pennant to his right: the blade's flat faces forward
SWORD_TOOL = {"kind": "sword", "hands": 1, "handle_length": sp.SWORD_LENGTH, "knob_offset": sp.SWORD_GRIP,
              "edge": (1.0, 0.0, 0.0)}  # edges to his right and left: the blade's flat faces the camera
NONE = {"kind": "none"}
STAND = torso(0.0, -2.0, 0.0, nod=2.0, pelvis=False)


def armed(name, base, tool, props, keys, elbows, steady):
    g = {"name": name, "base": base, "hands_frame": "chest", "tool": tool, "props": props, "scene": NONE,
         "elbows": elbows, "fist": (55.0, 70.0, 45.0), "keys": keys}
    g.update(steady)
    return g


def idle_keys(rot, tool_key, hands):
    keys = breathing(rot)
    for key in keys:
        key["tool"] = tool_key
        key["hands"] = hands
    return keys


def walk_keys(rot, tool_key, hands):
    return [{"t": 0.0, "rot": rot, "tool": tool_key, "hands": hands},
            {"t": 1.0, "rot": rot, "tool": tool_key, "hands": hands}]


# Spearman: the spear upright beside the right foot, the right hand at the hip, elbow a little bent, the butt a few cm
# off the ground; in the walk leaning 45 degrees forward (point 113 cm ahead at 208 cm, butt ~45 cm off the ground
# behind; user's review of the previews, 2026-10-18), steady while the left arm swings.
SPEAR_IDLE_GRIP = {"grip": (27.0, 10.0, 100.0), "dir": UP}
SPEAR_WALK_GRIP = {"grip": (27.0, 12.0, 112.0), "dir": (0.0, 0.7071, 0.7071)}
LEFT_HANGING = {"l": {"at": (-22.0, -2.0, 92.0), "palm": (1.0, 0.0, 0.0), "fingers": (0.0, 0.1, -1.0)}}
LEFT_SWINGING = {"l": {"at": (-21.0, -2.0, 90.0), "palm": (1.0, 0.0, 0.0), "fingers": (0.0, 0.15, -1.0),
                       "swing": 0.35}}
SPEAR_ELBOWS = {"l": (-0.2, -1.0, -0.4), "r": (0.6, -0.6, -0.6)}
SPEAR_IDLE = armed("A_Spearman_Idle", IDLE, SPEAR_TOOL, ("spear",),
                   idle_keys(STAND, SPEAR_IDLE_GRIP, LEFT_HANGING), SPEAR_ELBOWS, {"steady_spine": 1.0})
SPEAR_WALK = armed("A_Spearman_Walk", UNARMED_WALK, SPEAR_TOOL, ("spear",),
                   walk_keys(STAND, SPEAR_WALK_GRIP, LEFT_SWINGING), SPEAR_ELBOWS, {"steady_chest": 0.5})

# Man-at-arms: the sword in the right hand, upright at rest (point in the air) and leaning 45 degrees forward in the
# walk (point forward and up; user's choice at the previews, 2026-10-18); the shield on the left forearm held across
# the front of the body, its face turned forward and a little to his left.
SWORD_UP = {"grip": (25.0, 14.0, 100.0), "dir": (0.0, 0.0, 1.0)}
SWORD_FORWARD = {"grip": (25.0, 16.0, 100.0), "dir": (0.0, 0.7071, 0.7071)}
SHIELD_HAND = {"l": {"at": (-6.0, 30.0, 112.0), "palm": (0.5, -0.85, 0.0), "fingers": (0.85, 0.5, 0.1)}}
MAA_ELBOWS = {"l": (-1.0, -0.4, -0.6), "r": (0.6, -0.6, -0.6)}
MAA_IDLE = armed("A_ManAtArms_Idle", IDLE, SWORD_TOOL, ("sword", "shield"),
                 idle_keys(STAND, SWORD_UP, SHIELD_HAND), MAA_ELBOWS, {"steady_spine": 1.0})
MAA_WALK = armed("A_ManAtArms_Walk", UNARMED_WALK, SWORD_TOOL, ("sword", "shield"),
                 walk_keys(STAND, SWORD_FORWARD, SHIELD_HAND), MAA_ELBOWS, {"steady_chest": 0.5})

GESTURES = {"spear_idle": SPEAR_IDLE, "spear_walk": SPEAR_WALK, "maa_idle": MAA_IDLE, "maa_walk": MAA_WALK}


# ---------------------------------------------------------------------------------------------------------------------
# Attacks and fall (J7b.5). The attack keys span one attack period of the unit table (Spearman 1.875 s, Man-at-arms
# 1.375 s) and the HIT lands at HALF the loop (ATTACK_HIT_FRACTION): the server's first hit comes after a 50 % wind-up,
# then one hit per period, so a loop started with the attack keeps every hit on the hit frame. Baked, a loop is a whole
# number of frames at 30 fps (Unreal resamples at 30 fps), an even one so the hit is a whole frame: 56 frames (1.867 s)
# for the Spearman, 42 (1.4 s) for the Man-at-arms; the game plays it at its length / AttackInterval (HOLUnitAnimation::Choose), so one loop is one
# period exactly and the hit stays at half. Feet planted (no base: legs by IK).
ATTACK_HIT_FRACTION = 0.5


def at(period, share):
    """Key time (s) at `share` of a loop of `period` seconds."""
    return period * share


def fighting(name, tool, props, keys, elbows, fps, stance, picks):
    g = {"name": name, "fps": fps, "tool": tool, "props": props, "scene": NONE, "elbows": elbows,
         "fist": (55.0, 70.0, 45.0), "stance": stance, "keys": keys,
         "preview": {"frames": 8, "zoom_picks": picks}}
    return g


def hip_turn(twist, bend, hip, side=0.0, nod=0.0):
    """torso() whose pelvis takes `hip` degrees of the whole `twist` (the three spine bones share the rest): a real cut
    swings the hips around the vertical axis, not only the chest. The shoulders still end `twist` from the legs and
    the head turns back as much (eyes on the enemy)."""
    rot = torso(twist, bend, side, nod=nod)
    rest = (twist - hip) / 3.0
    for bone in ("spine_01", "spine_02", "spine_03"):
        rot[bone] = (rest,) + rot[bone][1:]
    rot["pelvis"] = (hip,) + rot["pelvis"][1:]
    return rot


# Spearman thrust: both hands on the shaft (left hand behind, right hand in front), the spear level at the waist on
# his right side, left foot forward. Guard, draw back, fast stab forward (point ~2 m ahead at chest height), short
# hold, recover to the guard.
SPEAR_PERIOD = 1.875
SPEAR_THRUST_TOOL = {"kind": "spear", "hands": 2, "handle_length": sp.SPEAR_LENGTH, "knob_offset": 70.0,
                     "hand_spacing": 40.0, "edge": (1.0, 0.0, 0.0)}
SPEAR_STANCE = {"l": (-4.0, 16.0, 0.0), "r": (6.0, -14.0, 0.0)}
SPEAR_FIGHT_ELBOWS = {"l": (-0.3, -0.5, -1.0), "r": (0.8, -0.4, -0.6)}


def spear_key(t, pose, flow=0.0):
    return dict(pose, t=t, flow=flow, elbows=SPEAR_FIGHT_ELBOWS)


# Polish (2026-10-19, "robot" at the trial; same method as the Man-at-arms' cut): same guard spear, same draw, same
# lunge spear, the hit still on frame 28 at the same point. The thrust is carried by the body: drawing back, the hips
# and the shoulders turn back to his right (the spear's side, its right shoulder going back), the weight onto the
# back (right) foot; at the stab the pelvis drives first (forward and to his left, onto the front knee), the trunk
# turns through (the right shoulder forward), the arms extend last and the point arrives at full speed (flow 2.4),
# overshoots a little, comes back and settles into the guard without a stop. Hermite keys, the pelvis leading the
# spine, the spine the shoulders, the spear on its keys. Feet planted (IK).
SPEAR_GUARD = {"pelvis": (0.0, 0.0, -6.0), "rot": hip_turn(4.0, 6.0, 2.0, nod=-4.0),
               "tool": {"grip": (17.0, 12.0, 104.0), "dir": (-0.098, 0.99, 0.12), "spacing": 40.0}}
SPEAR_SET = {"pelvis": (0.0, 1.0, -7.5), "rot": hip_turn(0.0, 9.0, 0.0, nod=-5.0),
             "tool": {"grip": (18.0, 6.0, 103.0), "dir": (-0.105, 0.99, 0.09), "spacing": 40.0}}
SPEAR_DRAWN = {"pelvis": (1.5, -6.0, -7.0), "rot": merge(hip_turn(30.0, 3.0, 12.0, nod=-4.0), {"clavicle_r": (-10.0, 0.0, 0.0)}),
               "tool": {"grip": (21.0, -6.0, 108.0), "dir": (-0.11, 0.99, 0.10), "spacing": 40.0}}
SPEAR_LOADED = {"pelvis": (2.0, -7.5, -6.5), "rot": merge(hip_turn(36.0, 2.0, 14.0, nod=-4.0), {"clavicle_r": (-12.0, 0.0, 0.0)}),
                "tool": {"grip": (21.0, -9.0, 109.0), "dir": (-0.115, 0.99, 0.10), "spacing": 40.0}}
SPEAR_LUNGE = {"pelvis": (-1.5, 11.0, -10.5), "rot": hip_turn(-18.0, 16.0, -7.0, nod=-10.0),
               "tool": {"grip": (15.0, 36.0, 116.0), "dir": (-0.088, 0.995, 0.10), "spacing": 36.0}}
SPEAR_OVER = {"pelvis": (-1.5, 12.0, -11.0), "rot": hip_turn(-24.0, 17.0, -9.0, nod=-10.0),
              "tool": {"grip": (14.0, 38.5, 116.0), "dir": (-0.085, 0.995, 0.10), "spacing": 35.0}}
SPEAR_BACK = {"pelvis": (-1.0, 9.5, -10.0), "rot": hip_turn(-16.0, 15.0, -6.0, nod=-9.0),
              "tool": {"grip": (16.0, 24.0, 114.0), "dir": (-0.092, 0.995, 0.10), "spacing": 37.0}}
SPEAR_SETTLE = {"pelvis": (0.5, -1.5, -5.0), "rot": merge(hip_turn(8.0, 4.0, 4.0, nod=-4.0), {"clavicle_r": (-6.0, 0.0, 0.0)}),
                "tool": {"grip": (17.5, 4.0, 105.0), "dir": (-0.10, 0.99, 0.13), "spacing": 40.0}}
SPEAR_LEADS = {"pelvis": 0.12, "rot": {"pelvis": 0.08, "spine_01": 0.065, "spine_02": 0.05, "spine_03": 0.038,
                                       "neck_01": 0.03, "head": 0.025, "clavicle_r": 0.028},
               "elbows": 0.014, "tool": 0.0}

SPEAR_ATTACK = fighting("A_Spearman_Attack", SPEAR_THRUST_TOOL, ("spear",), [
    spear_key(0.0, SPEAR_GUARD),
    spear_key(at(SPEAR_PERIOD, 0.12), SPEAR_SET),                 # anticipation: settles on the knees
    spear_key(at(SPEAR_PERIOD, 0.36), SPEAR_DRAWN, 0.5),          # still drawing back, turning to his right
    spear_key(at(SPEAR_PERIOD, 0.43), SPEAR_LOADED),              # loaded on the back foot: hangs a moment
    spear_key(at(SPEAR_PERIOD, 0.50), SPEAR_LUNGE, (2.4, 0.7)),   # THE HIT at full speed (frame 28 of 56)
    spear_key(at(SPEAR_PERIOD, 0.56), SPEAR_OVER),                # small overshoot
    spear_key(at(SPEAR_PERIOD, 0.64), SPEAR_BACK, 1.0),           # draws back off the target
    spear_key(at(SPEAR_PERIOD, 0.86), SPEAR_SETTLE),              # the guard overshot
    spear_key(SPEAR_PERIOD, SPEAR_GUARD),
], SPEAR_FIGHT_ELBOWS, 30, SPEAR_STANCE, [0, 12, 22, 24, 28, 31])
SPEAR_ATTACK.update({"frames": 56, "curve": "hermite", "loop": True, "leads": SPEAR_LEADS})

# Man-at-arms strike: a diagonal cut from over the right shoulder down to his left in front, the shield raised in
# front of the chest the whole time (left hand at chest height, face forward). 42 frames at 30 fps (1.4 s, played at
# 1.4 / 1.375 in the game) so the hit is a whole frame (21; 1.375 s is 41.25 frames, the hit would fall between two).
MAA_PERIOD = 1.375
MAA_STRIKE_TOOL = {"kind": "sword", "hands": 1, "handle_length": sp.SWORD_LENGTH, "knob_offset": sp.SWORD_GRIP,
                   "edge": (0.0, 1.0, 0.0)}  # the cutting edge forward at the guard (sword upright): it leads the cut
MAA_STANCE = {"l": (-4.0, 14.0, 0.0), "r": (6.0, -12.0, 0.0)}
SHIELD_RAISED = {"l": {"at": (-4.0, 34.0, 128.0), "palm": (0.5, -0.85, 0.0), "fingers": (0.85, 0.5, 0.1)}}
MAA_FIGHT_ELBOWS = {"l": (-1.0, -0.3, -0.5), "r": (1.0, -0.4, -0.3)}


# Hips (user's review of the polish, 2026-10-19: "more pelvis to make a real cut"): the pelvis turns with the
# shoulders (angles: see "Upper body" below). Wind-up: hips and shoulders turned well back to his right (the sword's
# shoulder going back), the weight back and a little right over the back (right) foot; cut: the hips turn through
# first, the weight onto the front (left) foot, the trunk ending turned to his left at the follow-through. Feet planted (IK).
MAA_GUARD = {"pelvis": (0.0, 0.0, -5.0), "rot": torso(0.0, 4.0, 0.0), "hands": SHIELD_RAISED,
             "tool": {"grip": (26.0, 18.0, 112.0), "dir": (0.0, 0.3, 0.95)}}


def maa_key(t, pose, flow=0.0):
    return dict(pose, t=t, flow=flow, elbows=MAA_FIGHT_ELBOWS)


# Polish (2026-10-19, "robot" at the trial): same guard, cocked, cut and follow poses, same loop, the hit still on
# frame 21; the keys were joined one ease per segment, every bone leaving and reaching each key at once and stopping
# on it. Now (anim_rig "curve": "hermite", "flow", "leads"): a small dip of the sword and the knees before the wind-up
# (anticipation), the wind-up still rising through the cocked pose to a loaded pose a little further back where it
# hangs, the cut passing the hit at full speed (flow 1.3) into the follow-through, a small rebound, the guard
# overshot and settled; the weight on the back foot at the wind-up, on the front knee at the cut; the pelvis leading
# the spine, the spine the shoulder and the arm, the sword last (on its keys), the shield trailing.
MAA_SET = {"pelvis": (-0.5, 1.5, -7.0), "rot": merge(hip_turn(-6.0, 6.0, -3.0), {"clavicle_r": (0.0, 0.0, 2.0)}),
           "tool": {"grip": (27.0, 21.0, 107.0), "dir": (0.0, 0.42, 0.9)}}
MAA_SETTLE = {"pelvis": (0.5, -1.5, -4.5), "rot": merge(hip_turn(4.0, 3.0, 2.0), {"clavicle_r": (1.0, 0.0, -1.0)}),
              "tool": {"grip": (26.0, 17.0, 115.0), "dir": (0.02, 0.2, 0.98)}}


def turned(x, y, deg):
    """(x, y) in body terms (right, forward) turned `deg` degrees to his right around the spine's vertical axis."""
    a = math.radians(deg)
    return (x * math.cos(a) + y * math.sin(a), -x * math.sin(a) + y * math.cos(a))


def shield_turned(deg, back=0.0, out=0.0, up=0.0):
    """SHIELD_RAISED carried around by the trunk's turn (`deg` to his right, like the twist): place, palm and fingers
    turned together, then the hand drawn `back` cm toward his back and `out` cm to his left (the arm opening)."""
    raised = SHIELD_RAISED["l"]
    x, y = turned(raised["at"][0], raised["at"][1], deg)
    palm, fingers = raised["palm"], raised["fingers"]
    return {"l": {"at": (x - out, y - back, raised["at"][2] + up), "palm": turned(palm[0], palm[1], deg) + (palm[2],),
                  "fingers": turned(fingers[0], fingers[1], deg) + (fingers[2],)}}


# Upper body (user's review of the hips, 2026-10-19: "the sword's guard must end in front of the shield, the upper
# body must really turn"): the cut is carried by the trunk turning, not by the arm. The shoulders turn ~120 degrees
# over the loop (about +55 to his right at the wind-up, the right shoulder far back, to about -65 to his left at the
# follow-through), the pelvis ~38 and still leading. At the follow-through the right arm is across the chest and the
# sword's guard in front of the shield, which the trunk has carried to his left and the left arm opened a little
# (back and out) to let the sword arm pass in front of it.
MAA_COCKED = {"pelvis": (1.5, -5.0, -5.0), "rot": merge(hip_turn(46.0, -2.0, 16.0), {"clavicle_r": (8.0, 0.0, -12.0)}),
              "tool": {"grip": (27.0, -6.0, 162.0), "dir": (0.15, -0.6, 0.79)}}
MAA_LOADED = {"pelvis": (2.5, -7.0, -4.5), "rot": merge(hip_turn(55.0, -3.0, 19.0), {"clavicle_r": (9.0, 0.0, -14.0)}),
              "tool": {"grip": (27.0, -11.0, 164.0), "dir": (0.18, -0.68, 0.71)}}
MAA_CUT = {"pelvis": (-2.0, 8.0, -9.0), "rot": merge(hip_turn(-28.0, 14.0, -14.0), {"clavicle_r": (-12.0, 0.0, 0.0)}),
           "tool": {"grip": (6.0, 48.0, 110.0), "dir": (-0.55, 0.8, -0.25)}}
MAA_FOLLOW = {"pelvis": (-2.5, 8.0, -9.5),
              "rot": merge(hip_turn(-66.0, 16.0, -20.0), {"clavicle_r": (-16.0, 0.0, 0.0)}),
              "tool": {"grip": (-30.0, 42.0, 108.0), "dir": (-0.75, 0.35, -0.56)}}
MAA_REBOUND = {"pelvis": (-2.0, 6.5, -8.5),
               "rot": merge(hip_turn(-58.0, 14.0, -18.0), {"clavicle_r": (-14.0, 0.0, 0.0)}),
               "tool": {"grip": (-25.0, 44.0, 112.0), "dir": (-0.68, 0.5, -0.53)}}
# The shield: carried around by the trunk; a little to his front-right while he winds up (covering the side he
# opens), then to his left with the cut, the left arm opening back and out at the follow-through.
SHIELD_COVER = shield_turned(22.0)
SHIELD_TIGHT = shield_turned(-25.0)
SHIELD_OPEN = shield_turned(-50.0, back=4.0, out=4.0)
SHIELD_BACK = shield_turned(-42.0, back=3.0, out=3.0)
MAA_LEADS = {"pelvis": 0.07, "rot": {"pelvis": 0.07, "spine_01": 0.05, "spine_02": 0.035, "spine_03": 0.02,
                                     "neck_01": 0.02, "head": 0.02, "clavicle_r": 0.012},
             "elbows": 0.008, "tool": 0.0, "hands": -0.04}

MAA_ATTACK = fighting("A_ManAtArms_Attack", MAA_STRIKE_TOOL, ("sword", "shield"), [
    maa_key(0.0, MAA_GUARD),
    maa_key(at(MAA_PERIOD, 0.14), dict(MAA_SET, hands=SHIELD_RAISED)),           # anticipation: dip before the rise
    maa_key(at(MAA_PERIOD, 0.38), dict(MAA_COCKED, hands=SHIELD_COVER), 0.5),    # still rising over the shoulder
    maa_key(at(MAA_PERIOD, 0.43), dict(MAA_LOADED, hands=SHIELD_COVER)),         # loaded: hangs a moment
    maa_key(at(MAA_PERIOD, 0.50), dict(MAA_CUT, hands=SHIELD_TIGHT), (2.4, 0.7)),  # THE HIT at full speed (frame 21)
    maa_key(at(MAA_PERIOD, 0.59), dict(MAA_FOLLOW, hands=SHIELD_OPEN)),          # follow-through: guard before shield
    maa_key(at(MAA_PERIOD, 0.66), dict(MAA_REBOUND, hands=SHIELD_BACK), 0.4),    # small rebound
    maa_key(at(MAA_PERIOD, 0.88), dict(MAA_SETTLE, hands=SHIELD_RAISED)),        # the guard overshot
    maa_key(MAA_PERIOD, MAA_GUARD),
], MAA_FIGHT_ELBOWS, 30, MAA_STANCE, [0, 6, 16, 18, 21, 25])
MAA_ATTACK.update({"frames": 42, "curve": "hermite", "loop": True, "leads": MAA_LEADS})

# Fall (dying), 1 s, not looped (Mass leaves a non-looped sequence on its last frame until the unit is destroyed):
# knees buckle, then he topples backward onto his back, knees still bent, arms out on the ground; the weapon stays in
# the right hand (the tool bone follows the hand; no separate prop to drop), the shield stays on the left arm.
# One body motion for every troop; only what the hands hold changes (FALL_BODY + per-type hands).
FALL_LENGTH = 1.0
FALL_BODY = [
    {"t": 0.0, "pelvis": (0.0, 0.0, -2.0), "rot": torso(0.0, 0.0)},
    {"t": 0.12, "ease": "out", "pelvis": (0.0, -4.0, -6.0), "rot": torso(5.0, -12.0, 0.0, nod=-10.0)},   # struck
    {"t": 0.40, "ease": "in", "pelvis": (0.0, -18.0, -40.0), "rot": torso(10.0, 10.0, 4.0, nod=8.0)},    # knees go
    {"t": 0.75, "ease": "in", "pelvis": (0.0, -62.0, -78.0),
     "rot": merge(torso(5.0, -25.0, 4.0, nod=-10.0, pelvis=False), {"pelvis": (0.0, -60.0, 0.0)})},      # topples
    {"t": 0.85, "ease": "out", "pelvis": (0.0, -66.0, -82.0),
     "rot": merge(torso(5.0, -10.0, 4.0, nod=6.0, pelvis=False), {"pelvis": (0.0, -78.0, 0.0)})},        # lands
    {"t": 1.0, "ease": "inout", "pelvis": (0.0, -68.0, -82.0),
     "rot": merge(torso(5.0, -8.0, 4.0, nod=4.0, pelvis=False), {"pelvis": (0.0, -76.0, 0.0)})},         # lies
]
# Right hand (holding the weapon or free) and left hand (free or with the shield), one place per key of FALL_BODY.
FALL_R = [(27.0, 10.0, 100.0), (30.0, 4.0, 104.0), (34.0, -6.0, 90.0), (50.0, -70.0, 40.0), (55.0, -95.0, 14.0),
          (55.0, -95.0, 12.0)]
FALL_L = [(-22.0, -2.0, 92.0), (-30.0, 0.0, 104.0), (-36.0, -10.0, 92.0), (-50.0, -72.0, 42.0), (-56.0, -96.0, 16.0),
          (-56.0, -96.0, 14.0)]
FALL_ELBOWS = {"l": (-1.0, -0.2, -0.4), "r": (1.0, -0.2, -0.4)}


def fall(name, tool, props, weapon_dirs, left_palm, left_fingers, left_places=FALL_L):
    keys = []
    for i, body in enumerate(FALL_BODY):
        key = dict(body, elbows=FALL_ELBOWS)
        left = {"at": left_places[i], "palm": left_palm[i]}
        if left_fingers:
            left["fingers"] = left_fingers[i]
        if tool is NONE:
            key["hands"] = {"r": {"at": FALL_R[i], "palm": (-left_palm[i][0], left_palm[i][1], left_palm[i][2])},
                            "l": left}
        else:
            key["tool"] = {"grip": FALL_R[i], "dir": weapon_dirs[i]}
            key["hands"] = {"l": left}
        keys.append(key)
    g = {"name": name, "fps": 30, "props": props, "scene": NONE, "elbows": FALL_ELBOWS,
         "fist": (55.0, 70.0, 45.0), "stance": {"l": (-4.0, 4.0, 0.0), "r": (4.0, -2.0, 0.0)}, "keys": keys,
         "preview": {"frames": 6, "zoom_picks": [0, 4, 12, 22, 26, 30]}}
    if tool is not NONE:
        g["tool"] = tool
    return g


FREE_PALMS = [(1.0, 0.0, 0.0), (1.0, 0.2, 0.0), (0.8, 0.0, -0.5), (0.3, 0.0, -0.95), (0.0, 0.0, -1.0),
              (0.0, 0.0, -1.0)]
SPEAR_FALL_DIRS = [UP, (0.1, 0.2, 0.97), (0.3, 0.5, 0.8), (0.3, 0.92, 0.25), (0.25, 0.97, 0.0), (0.25, 0.97, 0.0)]
SWORD_FALL_DIRS = [UP, (0.2, 0.3, 0.93), (0.5, 0.4, 0.75), (0.8, 0.5, 0.3), (0.9, 0.44, 0.0), (0.9, 0.44, 0.0)]
SHIELD_PALMS = [(0.5, -0.85, 0.0), (0.6, -0.8, 0.0), (0.5, -0.6, 0.6), (0.2, -0.3, 0.93), (0.0, 0.0, 1.0),
                (0.0, 0.0, 1.0)]
SHIELD_FINGERS = [(0.85, 0.5, 0.1), (0.8, 0.5, 0.1), (0.8, 0.5, 0.1), (0.3, 0.95, 0.0), (0.0, 1.0, 0.0),
                  (0.0, 1.0, 0.0)]
SPEAR_FALL = fall("A_Spearman_Fall", SPEAR_TOOL, ("spear",), SPEAR_FALL_DIRS, FREE_PALMS, None)
MAA_FALL = fall("A_ManAtArms_Fall", SWORD_TOOL, ("sword", "shield"), SWORD_FALL_DIRS, SHIELD_PALMS, SHIELD_FINGERS,
                [(-6.0, 30.0, 112.0), (-8.0, 26.0, 116.0)] + FALL_L[2:])
PEASANT_FALL = fall("A_Peasant_Fall", NONE, (), None, FREE_PALMS, None)

GESTURES.update({"spear_attack": SPEAR_ATTACK, "maa_attack": MAA_ATTACK, "spear_fall": SPEAR_FALL,
                 "maa_fall": MAA_FALL, "peasant_fall": PEASANT_FALL})

# Peasant attack (validated in preview, baked with --soldiers --only=peasant_attack). User's choice (2026-10-19): the
# Peasant fights with a knife, not the axe: a one-handed stab into the enemy's belly ~1 m in front. He is not a fighter (user's review of the first stab, 2026-10-19): a frightened villager
# defending himself, not a boxer's guard. The knife (prop "knife", peasant_props.py, id 8) is held by the right hand
# alone and shown only while he attacks (as the hammer while he builds). 2.0 s (AttackInterval of the Peasant): 60
# frames at 30 fps, the hit at frame 30 (ATTACK_HIT_FRACTION).
import peasant_props as pprops  # noqa: E402

PEASANT_PERIOD = 2.0
PEASANT_KNIFE_TOOL = {"kind": "knife", "hands": 1, "handle_length": pprops.KNIFE_LENGTH,
                      "knob_offset": pprops.KNIFE_GRIP, "edge": (0.0, 0.0, -1.0)}  # the edge down, the flat sideways
PEASANT_STANCE = {"l": (-6.0, 14.0, 0.0), "r": (6.0, -10.0, 0.0)}
PEASANT_FIGHT_ELBOWS = {"l": (-0.5, -0.2, -1.0), "r": (0.35, -0.6, -0.9)}  # the knife's elbow tucked against the ribs


def peasant_key(t, pose, flow=0.0):
    return merge({"elbows": PEASANT_FIGHT_ELBOWS}, pose, {"t": t, "flow": flow})


def peasant_cover(at_, palm=(0.25, -0.95, 0.15), fingers=(0.15, 0.25, 0.95)):
    """The free left forearm raised in front of his face, the back of the hand toward the enemy (shielding himself)."""
    return {"l": {"at": at_, "palm": palm, "fingers": fingers}}


def afraid(rot, fear=1.0, away=8.0):
    """`rot` hunched by fear: the back rounded, both shoulders raised, the head pulled into them (the neck bowed
    forward, the head tipped back up to keep the enemy in sight) and turned `away` degrees to his right, shying."""
    out = dict(rot)

    def add(bone, d):
        out[bone] = tuple(a + b for a, b in zip(out.get(bone, (0.0, 0.0, 0.0)), d))
    add("spine_02", (0.0, 4.0 * fear, 0.0))
    add("spine_03", (0.0, 5.0 * fear, 0.0))
    add("clavicle_r", (0.0, 0.0, -11.0 * fear))
    add("clavicle_l", (0.0, 0.0, 11.0 * fear))
    add("neck_01", (away * 0.4, 9.0 * fear, 0.0))
    add("head", (away * 0.6, -12.0 * fear, 0.0))
    return out


# Same method as the soldiers' validated attacks (hermite keys, "flow", "leads"), but a frightened body: guard hunched,
# knees bent, the weight back over the right foot (ready to back away), the left forearm up in front of his face, the
# knife held tight by the right hip, point forward, elbow against the ribs; a small tremble in the guard. The blow:
# a false start (he leans in, flinches back), he gathers himself (lower, curled up, the knife drawn back), then a
# quick short jab: the hips barely turn and the weight hardly comes forward (no lunge), the arm does most of it, the
# point arrives in the body's axis at belly height; at once the trunk snaps back (the return is faster than the
# jab), the forearm back up in front of his face, cowering a moment further back before the guard. Feet planted (IK).
PEASANT_GUARD = {"pelvis": (0.0, -5.0, -9.0), "rot": afraid(hip_turn(10.0, 12.0, 4.0, nod=-4.0)),
                 "tool": {"grip": (14.0, 12.0, 99.0), "dir": (-0.15, 0.86, 0.49)},
                 "hands": peasant_cover((-5.0, 25.0, 140.0))}
PEASANT_TREMBLE = {"pelvis": (0.3, -5.4, -9.4), "rot": afraid(hip_turn(11.0, 12.5, 4.0, nod=-4.0), 1.1, 10.0),
                   "tool": {"grip": (14.5, 11.0, 98.0), "dir": (-0.17, 0.85, 0.50)},
                   "hands": peasant_cover((-6.0, 24.0, 141.5))}
PEASANT_LEAN = {"pelvis": (-0.5, -2.5, -9.5), "rot": afraid(hip_turn(6.0, 14.0, 2.0, nod=-5.0), 0.9, 6.0),
                "tool": {"grip": (13.0, 17.0, 100.0), "dir": (-0.12, 0.90, 0.42)},
                "hands": peasant_cover((-5.0, 27.0, 139.0))}
PEASANT_FLINCH = {"pelvis": (0.5, -6.5, -9.5), "rot": afraid(hip_turn(13.0, 10.0, 5.0, nod=-3.0), 1.15, 12.0),
                  "tool": {"grip": (14.5, 10.0, 99.0), "dir": (-0.16, 0.85, 0.50)},
                  "hands": peasant_cover((-5.0, 24.0, 142.0))}
PEASANT_GATHER = {"pelvis": (1.0, -6.0, -12.0),
                  "rot": afraid(merge(hip_turn(24.0, 16.0, 8.0, nod=-6.0), {"clavicle_r": (-6.0, 0.0, 0.0)}), 1.2, 4.0),
                  "tool": {"grip": (18.5, 2.0, 100.0), "dir": (-0.08, 0.95, 0.30)},
                  "hands": peasant_cover((-4.0, 28.0, 137.0))}
PEASANT_STAB = {"pelvis": (-0.5, 1.5, -11.0), "rot": afraid(hip_turn(-12.0, 16.0, -4.0, nod=-8.0), 0.8, 0.0),
                "tool": {"grip": (5.0, 42.0, 104.0), "dir": (-0.06, 0.997, -0.04)},
                "hands": peasant_cover((-7.0, 26.0, 138.0))}
PEASANT_OVER = {"pelvis": (-0.5, 1.0, -11.0), "rot": afraid(hip_turn(-12.0, 15.0, -4.0, nod=-8.0), 0.85, 2.0),
                "tool": {"grip": (5.0, 44.0, 104.0), "dir": (-0.06, 0.997, -0.05)},
                "hands": peasant_cover((-7.0, 26.0, 138.5))}
PEASANT_RECOIL = {"pelvis": (0.5, -6.5, -10.0), "rot": afraid(hip_turn(8.0, 6.0, 3.0, nod=-2.0), 1.2, 12.0),
                  "tool": {"grip": (13.5, 13.0, 100.5), "dir": (-0.14, 0.88, 0.46)},
                  "hands": peasant_cover((-5.0, 24.0, 143.0))}
PEASANT_COWER = {"pelvis": (0.5, -7.0, -10.0), "rot": afraid(hip_turn(13.0, 10.0, 5.0, nod=-3.0), 1.25, 14.0),
                 "tool": {"grip": (14.5, 10.0, 99.0), "dir": (-0.16, 0.85, 0.50)},
                 "hands": peasant_cover((-5.0, 23.0, 143.0))}
PEASANT_LEADS = {"pelvis": 0.05, "rot": {"pelvis": 0.045, "spine_01": 0.03, "spine_02": 0.02, "spine_03": 0.012,
                                         "neck_01": 0.01, "head": 0.01, "clavicle_r": 0.006},
                 "elbows": 0.005, "tool": 0.0, "hands": -0.02}

PEASANT_ATTACK = fighting("A_Peasant_Attack", PEASANT_KNIFE_TOOL, ("knife",), [
    peasant_key(0.0, PEASANT_GUARD),
    peasant_key(at(PEASANT_PERIOD, 0.07), PEASANT_TREMBLE),             # a small tremble
    peasant_key(at(PEASANT_PERIOD, 0.14), PEASANT_GUARD),
    peasant_key(at(PEASANT_PERIOD, 0.23), PEASANT_LEAN),                # a false start: he leans in...
    peasant_key(at(PEASANT_PERIOD, 0.31), PEASANT_FLINCH),              # ...and flinches back
    peasant_key(at(PEASANT_PERIOD, 0.42), PEASANT_GATHER),              # gathers himself, the knife drawn back
    peasant_key(at(PEASANT_PERIOD, 0.50), PEASANT_STAB, (2.0, 1.0)),    # THE HIT, a short quick jab (frame 30 of 60)
    peasant_key(at(PEASANT_PERIOD, 0.52), PEASANT_OVER),                # the arm's last 2 cm, the trunk already going
    peasant_key(at(PEASANT_PERIOD, 0.57), PEASANT_RECOIL, 0.8),         # snaps back at once, faster than the jab
    peasant_key(at(PEASANT_PERIOD, 0.72), PEASANT_COWER),               # cowers, further back than the guard
    peasant_key(PEASANT_PERIOD, PEASANT_GUARD),
], PEASANT_FIGHT_ELBOWS, 30, PEASANT_STANCE, [0, 4, 18, 25, 30, 34])
PEASANT_ATTACK.update({"frames": 60, "curve": "hermite", "loop": True, "leads": PEASANT_LEADS})

# Validated by the user in preview (2026-10-19, f7b0b27) and baked (J7b): A_Peasant_Attack, sequence 13 of TP_Peasant.
GESTURES["peasant_attack"] = PEASANT_ATTACK

# Gestures shown by the previews but not baked yet (waiting for the user's review).
PROPOSED = {}

# ---------------------------------------------------------------------------------------------------------------------
# Archer (J7c, validated in preview, baked in J7c.1a). The longbow (soldier_props "bow", id 4) in the LEFT fist,
# the quiver (id 6) on the back, opening above the right shoulder; the arrow (id 5) is the right hand's tool (on
# ik_hand_gun), shown only from the nock to the release.
from anim_rig import add, normalize, sub  # noqa: E402

ARCHER_ELBOWS = {"l": (-0.6, -0.6, -0.6), "r": (0.2, -1.0, -0.4)}
BOW_CARRIED = {"at": (-20.0, 14.0, 98.0), "palm": (1.0, 0.0, 0.0), "fingers": (0.0, 1.0, -0.15)}  # bow upright
RIGHT_HANGING = {"at": (22.0, -2.0, 92.0), "palm": (-1.0, 0.0, 0.0), "fingers": (0.0, 0.1, -1.0)}
ARCHER_IDLE = {"name": "A_Archer_Idle", "base": IDLE, "hands_frame": "chest", "props": ("bow", "quiver"),
               "scene": NONE, "elbows": ARCHER_ELBOWS, "fist": (55.0, 70.0, 45.0), "steady_spine": 1.0,
               "keys": idle_keys(STAND, None, {"l": BOW_CARRIED, "r": RIGHT_HANGING})}
for _key in ARCHER_IDLE["keys"]:
    del _key["tool"]
ARCHER_WALK = {"name": "A_Archer_Walk", "base": UNARMED_WALK, "hands_frame": "chest", "props": ("bow", "quiver"),
               "scene": NONE, "elbows": ARCHER_ELBOWS, "fist": (55.0, 70.0, 45.0), "steady_chest": 0.5,
               "keys": [{"t": t, "rot": STAND, "hands": {"l": BOW_CARRIED, "r": dict(RIGHT_HANGING, swing=0.35)}}
                        for t in (0.0, 1.0)]}

# Attack: AttackInterval of the Archer 1.625 s (Tools/Data/UnitTypes.csv); 48 frames at 30 fps (1.6 s, played at
# 1.6 / 1.625 in the game, like the soldiers), the RELEASE on frame 6 (ARCHER_RELEASE, 0.125 of the loop, ~0.2 s: its
# AttackWindUpPercent 12 in the table, HOLUnitAnimation::GetStrikeHitShare): the client launches the visual arrow there.
# The 0.005 of a loop between the two (0.125 against 0.12, ~8 ms in the game) is accepted: frame 6 is the nearest one.
# Retimed for the hit and run (trial feedback 8, 2026-10-21): release on frame 24 at first, frame 12 in loop 2, frame 6
# in loop 3 ("halve the time to shoot an arrow"). The draw twice as fast again (raise 0.06, full draw 0.095, aim
# 0.113, release 0.125; the settle key dropped, too cramped at 0.025), the follow-through at its pace (0.185), the
# rest of the loop spread out (hold 0.29, quiver 0.48, arrow out 0.62, swing 0.78, nocked 0.88), then the ready pose
# held to the end of the loop. Feet planted side-on (left foot toward the target). The loop starts nocked, bow low:
#   raise and draw together, the hips then the trunk turning to his right (left shoulder to the
#   target), the right shoulder blade pulled back, the string to the corner of the jaw -> a short aim hold creeping
#   back a cm -> RELEASE (the hand stops, then snaps back past the ear: flow (0, 2.5)) -> follow-through held, bow arm
#   still on the target -> the bow drops, the right hand reaches over the shoulder to the quiver -> draws an arrow
#   out, swings it down onto the bow and nocks it. Hermite keys, the pelvis leading the spine, the spine the
#   shoulders, the hands last.
ARCHER_PERIOD = 1.6
ARCHER_RELEASE = 0.125  # share of the loop at the release, frame 6 of 48 (AttackWindUpPercent 12 / 100, ~8 ms apart)
ARROW_TOOL = {"kind": "arrow", "hands": 1, "handle_length": sp.ARROW_LENGTH, "knob_offset": sp.ARROW_GRIP,
              "edge": (0.0, 0.0, 1.0)}
ARCHER_STANCE = {"l": (-6.0, 16.0, 0.0), "r": (8.0, -14.0, 0.0)}


# Round 4 (user: "je crois que la main fait un 90 degrés"): the drawing hand's fingers are keyed (anim_rig tool
# "fingers") instead of pointing away from the shoulder, which flipped the hand ~73 degrees in one frame (45) when
# the reach ran along the arrow while nocking, and twisted the wrist ~130 degrees over the draw. On the string they
# hook it from his right, pointing to his left and a little down; at the quiver, back and down to the nock.
F_STRING = (-1.0, 0.0, -0.3)
F_FLUNG = (-0.9, -0.1, -0.2)
F_QUIVER = (-0.3, -0.9, 0.2)
F_SWING = (-0.8, -0.2, 0.2)


def nocked(right, left, rise=3.0, fingers=F_STRING):
    """The arrow's tool key: nock in the right hand, pointing at the bow's grip (resting just above the fist)."""
    return {"grip": right, "dir": normalize(sub((left[0], left[1], left[2] + rise), right)), "fingers": fingers}


def bow_hand(at_, cant=0.0, elevation=0.0):
    """The bow fist: thumb up, knuckles toward the shot; `cant` tips the top of the bow to his right, `elevation` raises
    the knuckles (the shot line) above the horizontal, the bow staying across the shot (degrees)."""
    a, e = math.radians(cant), math.radians(elevation)
    return {"l": {"at": at_, "palm": (math.cos(a), 0.0, -math.sin(a)),
                  "fingers": (0.0, math.cos(e), math.sin(e))}}


def archer_key(t, pose, flow=0.0):
    return merge({"elbows": ARCHER_ELBOWS}, pose, {"t": t, "flow": flow})


DRAW_ELBOWS = {"l": (-1.0, -0.1, -0.4), "r": (0.6, -1.0, 0.15)}
A_READY_L, A_READY_R = (-12.0, 40.0, 110.0), (3.0, 12.0, 110.0)
ARCHER_READY = {"pelvis": (0.0, 0.0, -4.0), "rot": hip_turn(30.0, 5.0, 10.0, nod=-4.0),
                "tool": nocked(A_READY_R, A_READY_L), "hands": bow_hand(A_READY_L, 20.0)}
# Round 2 (user's review of the GIF, 2026-10-20: "il tire en cloche donc plus haut l'arc"): the arrows fly in a light
# arc (J7c-Q4), so he shoots upward. The draw line rises to DRAW_ELEVATION at full draw and release: the drawing hand
# stays at the jaw, the bow fist goes up the shot line (shot_left), the bow across the shot (upright in its own
# plane, tipped back with the line); the trunk leans back to open the angle, the pelvis a little forward under it,
# leading as before. The rise is part of the raise and draw (half the elevation at the raise key), no separate pop.
DRAW_ELEVATION = 32.0
DRAW_REACH = 62.0  # drawing hand to the bow fist at full draw (cm)
SHOT_HEADING = (-0.146, 0.989)  # the shot's direction on the ground, body terms (right, forward): his target


def shot_left(right, elevation, reach=DRAW_REACH):
    """The bow fist `reach` cm from the drawing hand along a shot line `elevation` degrees above the horizontal."""
    e = math.radians(elevation)
    return (right[0] + reach * math.cos(e) * SHOT_HEADING[0], right[1] + reach * math.cos(e) * SHOT_HEADING[1],
            right[2] + reach * math.sin(e))


A_RAISE_R = (5.0, 22.0, 146.0)
A_RAISE_L = shot_left(A_RAISE_R, DRAW_ELEVATION * 0.55, 48.0)
ARCHER_RAISE = {"pelvis": (-0.5, 0.0, -5.0), "rot": merge(hip_turn(45.0, -3.0, 16.0, nod=-3.0),
                                                          {"clavicle_r": (5.0, 0.0, -2.0)}),
                "tool": nocked(A_RAISE_R, A_RAISE_L), "hands": bow_hand(A_RAISE_L, 6.0, DRAW_ELEVATION * 0.55),
                "elbows": DRAW_ELBOWS}
A_FULL_R = (3.0, 0.0, 152.0)
A_FULL_L = shot_left(A_FULL_R, DRAW_ELEVATION)
ARCHER_FULL = {"pelvis": (-1.0, 1.5, -4.0), "rot": merge(hip_turn(55.0, -9.0, 20.0, nod=-4.0),
                                                         {"clavicle_r": (10.0, 0.0, -4.0)}),
               "tool": nocked(A_FULL_R, A_FULL_L), "hands": bow_hand(A_FULL_L, 3.0, DRAW_ELEVATION),
               "elbows": DRAW_ELBOWS}
A_AIM_R = (3.5, -1.5, 152.0)
ARCHER_AIM = dict(ARCHER_FULL, rot=merge(hip_turn(56.0, -9.5, 20.0, nod=-4.0), {"clavicle_r": (12.0, 0.0, -4.0)}),
                  tool=nocked(A_AIM_R, A_FULL_L))
A_LOOSE_R = (4.0, -2.5, 152.0)
ARCHER_LOOSE = dict(ARCHER_AIM, tool=nocked(A_LOOSE_R, A_FULL_L),
                    rot=merge(hip_turn(56.0, -9.5, 20.0, nod=-4.0), {"clavicle_r": (13.0, 0.0, -4.0)}))
A_FOLLOW_R = (13.0, -17.0, 156.0)
A_FOLLOW_L = shot_left(A_FULL_R, DRAW_ELEVATION + 1.5, DRAW_REACH + 2.0)  # the bow arm kicks on along the line
ARCHER_FOLLOW = {"pelvis": (-1.5, 1.5, -4.0), "rot": merge(hip_turn(57.0, -10.0, 20.0, nod=-4.0),
                                                           {"clavicle_r": (16.0, 0.0, -3.0)}),
                 "tool": {"grip": A_FOLLOW_R, "dir": (-0.2, 0.8, 0.55), "fingers": F_FLUNG},
                 "hands": bow_hand(A_FOLLOW_L, 5.0, DRAW_ELEVATION + 1.5), "elbows": DRAW_ELBOWS}
A_HOLD_R = (14.0, -17.0, 153.0)
A_HOLD_L = shot_left(A_FULL_R, DRAW_ELEVATION - 4.0, DRAW_REACH)
ARCHER_HOLD = dict(ARCHER_FOLLOW, rot=merge(hip_turn(52.0, -7.0, 18.0, nod=-4.0), {"clavicle_r": (12.0, 0.0, -2.0)}),
                   tool={"grip": A_HOLD_R, "dir": (-0.2, 0.8, 0.55), "fingers": F_FLUNG},
                   hands=bow_hand(A_HOLD_L, 10.0, DRAW_ELEVATION - 4.0))
QUIVER_DOWN = (-0.41, -0.05, -0.91)  # the quiver's axis, top to bottom (soldier_props QUIVER_TOP)
A_REACH_L, A_REACH_R = (-13.0, 38.0, 114.0), (14.0, -8.0, 170.0)
ARCHER_REACH = {"pelvis": (0.0, 0.0, -4.0), "rot": hip_turn(25.0, 3.0, 8.0, nod=-3.0),
                "tool": {"grip": A_REACH_R, "dir": QUIVER_DOWN, "fingers": F_QUIVER}, "hands": bow_hand(A_REACH_L, 22.0),
                "elbows": {"l": (-0.6, -0.6, -0.6), "r": (0.8, 0.2, 0.3)}}
A_OUT_R = (18.0, 6.0, 178.0)
ARCHER_OUT = dict(ARCHER_REACH, tool={"grip": A_OUT_R, "dir": (-0.3, -0.15, -0.94), "fingers": F_QUIVER},
                  hands=bow_hand((-13.0, 39.0, 113.0), 22.0))
A_SWING_R = (8.0, 22.0, 132.0)
ARCHER_SWING = {"pelvis": (0.0, 0.5, -5.0), "rot": hip_turn(28.0, 5.0, 9.0, nod=-4.0),
                "tool": {"grip": A_SWING_R, "dir": (-0.3, 0.85, -0.43), "fingers": F_SWING}, "hands": bow_hand((-12.0, 40.0, 111.0), 20.0),
                "elbows": {"l": (-0.6, -0.6, -0.6), "r": (0.8, -0.6, -0.2)}}
ARCHER_LEADS = {"pelvis": 0.06, "rot": {"pelvis": 0.06, "spine_01": 0.045, "spine_02": 0.03, "spine_03": 0.018,
                                        "neck_01": 0.015, "head": 0.015, "clavicle_r": 0.01},
                "elbows": 0.006, "tool": 0.0, "hands": 0.0}

ARCHER_ATTACK = fighting("A_Archer_Attack", ARROW_TOOL, ("bow", "quiver", "arrow"), [
    archer_key(0.0, ARCHER_READY),
    archer_key(at(ARCHER_PERIOD, 0.06), ARCHER_RAISE, 0.8),           # raising and drawing together
    archer_key(at(ARCHER_PERIOD, 0.095), ARCHER_FULL),                # full draw: string to the jaw
    archer_key(at(ARCHER_PERIOD, 0.113), ARCHER_AIM),                 # aim hold, creeping back
    archer_key(at(ARCHER_PERIOD, ARCHER_RELEASE), ARCHER_LOOSE, (0.0, 2.5)),  # RELEASE (frame 6 of 48): arrow leaves
    archer_key(at(ARCHER_PERIOD, 0.185), ARCHER_FOLLOW),              # the hand snapped back past the ear
    archer_key(at(ARCHER_PERIOD, 0.29), ARCHER_HOLD, 0.4),            # follow-through held, bow on the target
    archer_key(at(ARCHER_PERIOD, 0.48), ARCHER_REACH, 0.6),           # bow down, the hand at the quiver
    archer_key(at(ARCHER_PERIOD, 0.62), ARCHER_OUT, 1.0),             # draws an arrow out
    archer_key(at(ARCHER_PERIOD, 0.78), ARCHER_SWING, 0.8),           # swings it down onto the bow
    archer_key(at(ARCHER_PERIOD, 0.88), ARCHER_READY),                # nocked, then ready until the loop ends
    archer_key(ARCHER_PERIOD, ARCHER_READY),
], ARCHER_ELBOWS, 30, ARCHER_STANCE, [0, 2, 3, 5, 6, 9])
ARCHER_ATTACK.update({"frames": 48, "curve": "hermite", "loop": True, "leads": ARCHER_LEADS,
                      "hit_fraction": ARCHER_RELEASE,  # the baker's readback looks for the release there
                      # Preview: the arrow in the hand is hidden from just after the release to the quiver (seconds).
                      "hidden": {"arrow": (at(ARCHER_PERIOD, ARCHER_RELEASE) + 0.01, at(ARCHER_PERIOD, 0.48))}})

# Fall: the soldiers' body motion (FALL_BODY), both hands free-placed (the bow follows the left fist), quiver on.
# Round 3 (user's review, 2026-10-20: "horrible, on dirait qu'il a les pieds cimentés, fais comme s'il prenait une
# flèche ou une balle"): a dedicated fall, not FALL_BODY. 1.1 s (33 frames at 30 fps), not looped. Struck in the chest
# from the front: the chest snaps back first, then the head, the arms fling, the pelvis follows (reversed leads) with
# an overshoot; the feet are FREE (anim_rig "feet"): the right foot lifts and stumbles back, the left drags then
# steps, the left knee buckles first (the pelvis drops to his left, turning a little to his right), the knees give,
# he sits back and falls onto his back, heavy (the feet kicked forward, the legs loose), a small bounce on contact,
# the head settling last and rolling to the side. The bow stays in the slack left hand and lands flat beside him,
# along his body (a real drop needs a free bone: see the J7c report).
def _fall_key(t, pelvis, rot, feet, left, right, flow=0.0):
    return {"t": t, "flow": flow, "pelvis": pelvis, "rot": rot, "feet": {"l": feet[0], "r": feet[1]},
            "hands": {"l": {"at": left[0], "palm": left[1], "fingers": left[2]},
                      "r": {"at": right[0], "palm": right[1], "fingers": right[2]}}, "elbows": FALL_ELBOWS}


def _body(pelvis=(0.0, 0.0, 0.0), s1=(0.0, 0.0, 0.0), s2=(0.0, 0.0, 0.0), s3=(0.0, 0.0, 0.0), neck=(0.0, 0.0, 0.0),
          head=(0.0, 0.0, 0.0), clav=0.0):
    return {"pelvis": pelvis, "spine_01": s1, "spine_02": s2, "spine_03": s3, "neck_01": neck, "head": head,
            "clavicle_l": (0.0, 0.0, clav), "clavicle_r": (0.0, 0.0, -clav)}


_ZERO = (0.0, 0.0, 0.0)
_BOW_UP = ((1.0, 0.0, 0.0), (0.0, 1.0, -0.15))      # palm, fingers of the bow fist (bow upright)
_BOW_FLAT = ((0.0, 0.0, -1.0), (-1.0, 0.0, 0.0))   # on the ground: the bow flat along his body
_R_DOWN = ((-1.0, 0.0, 0.0), (0.0, 0.1, -1.0))
_R_LIMP = ((0.0, 0.0, 1.0), (1.0, 0.0, 0.0))       # palm up on the ground
# The right hand rolls from palm down to palm up about its fingers, spread from 0.72 s to 1.04 s (palm down to palm up
# in one segment turned it 166 deg in one frame at the contact): 40 deg rolled at 0.72 s, 110 at the contact, 155 at
# the bounce, 180 (_R_LIMP) once still.
_R_ROLL_110 = ((0.0, -0.94, 0.34), (1.0, 0.0, 0.0))
_R_ROLL_155 = ((0.0, -0.42, 0.91), (1.0, 0.0, 0.0))
ARCHER_FALL_KEYS = [
    _fall_key(0.0, (0.0, 0.0, -2.0), _body(), (_ZERO, _ZERO),
              (BOW_CARRIED["at"],) + _BOW_UP, (RIGHT_HANGING["at"],) + _R_DOWN),
    # IMPACT: the chest knocked back, the bow arm jerks up and out, the right arm flung.
    _fall_key(0.07, (0.0, -3.0, 0.0), _body((0.0, 3.0, 0.0), (0.0, -3.0, 0.0), (2.0, -8.0, 0.0), (3.0, -13.0, 2.0),
                                            (0.0, -6.0, 0.0), (0.0, -14.0, 0.0), -8.0), (_ZERO, (0.0, 0.0, 2.0)),
              ((-28.0, 22.0, 120.0), (1.0, 0.0, 0.2), (0.0, 0.9, 0.4)),
              ((30.0, 12.0, 112.0), (-0.6, 0.4, -0.6), (0.3, 0.6, -0.7)), (2.2, 0.6)),
    # Overshoot: knocked off his heels, the pelvis lifted, the right foot leaving the ground.
    _fall_key(0.15, (0.0, -9.0, 2.0), _body((0.0, 2.0, -2.0), (0.0, -4.0, 0.0), (3.0, -11.0, 0.0), (5.0, -17.0, 3.0),
                                            (0.0, -8.0, 0.0), (2.0, -22.0, 0.0), -10.0),
              ((0.0, -2.0, 0.0), (1.0, -10.0, 8.0)),
              ((-36.0, 12.0, 128.0), (0.9, 0.0, 0.4), (-0.2, 0.6, 0.8)),
              ((38.0, 2.0, 124.0), (-0.3, 0.6, -0.7), (0.5, 0.5, 0.7))),
    # First stumble: the right foot planted back, the left dragging; he twists a little to his right.
    _fall_key(0.30, (2.0, -22.0, -4.0), _body((8.0, -4.0, -6.0), (2.0, -3.0, -2.0), (2.0, -6.0, 0.0),
                                              (3.0, -6.0, 0.0), (0.0, -2.0, 0.0), (0.0, -6.0, 0.0), -6.0),
              ((-1.0, -6.0, 2.0), (3.0, -32.0, 0.0)),
              ((-42.0, -6.0, 116.0), (0.8, 0.0, -0.5), (-0.4, 0.5, 0.75)),
              ((40.0, -16.0, 116.0), (-0.3, 0.3, -0.9), (0.6, 0.2, -0.7)), 1.0),
    # Second step: the left foot in the air, going back.
    _fall_key(0.44, (3.0, -33.0, -7.0), _body((12.0, -6.0, -10.0), (2.0, -3.0, -3.0), (3.0, -4.0, -2.0),
                                              (2.0, -2.0, 0.0), (0.0, 0.0, 0.0), (0.0, 2.0, 0.0), -4.0),
              ((-3.0, -24.0, 9.0), (3.0, -34.0, 0.0)),
              ((-44.0, -22.0, 108.0), (0.7, 0.0, -0.7), (-0.6, 0.4, 0.6)),
              ((42.0, -26.0, 108.0), (-0.2, 0.2, -1.0), (0.7, 0.1, -0.6)), 1.0),
    # The left knee buckles first as the left foot lands: the pelvis drops to his left.
    _fall_key(0.55, (2.0, -40.0, -20.0), _body((14.0, -10.0, -14.0), (2.0, -2.0, -4.0), (2.0, -2.0, -2.0),
                                               (0.0, 2.0, 0.0), (0.0, 6.0, 0.0), (0.0, 6.0, 0.0), -2.0),
              ((-5.0, -38.0, 0.0), (3.0, -35.0, 0.0)),
              ((-46.0, -30.0, 96.0), (0.6, 0.0, -0.8), (-0.75, 0.4, 0.5)),
              ((44.0, -32.0, 98.0), (-0.1, 0.2, -1.0), (0.8, 0.0, -0.5)), 0.8),
    # The knees give: he sits back, the trunk tipping back, the head lagging forward.
    _fall_key(0.72, (4.0, -56.0, -56.0), _body((15.0, -35.0, -10.0), (0.0, 0.0, -3.0), (0.0, 4.0, 0.0),
                                               (0.0, 6.0, 0.0), (0.0, 10.0, 0.0), (0.0, 10.0, 0.0)),
              ((-6.0, -40.0, 0.0), (6.0, -28.0, 0.0)),
              ((-52.0, -50.0, 70.0), (0.4, 0.0, -0.9), (-0.9, 0.3, 0.2)),
              ((48.0, -52.0, 70.0), (0.0, -0.64, -0.77), (0.9, 0.2, -0.3)), 1.0),
    # Contact: on his back, heavy (fast in, stopped), the feet kicked forward.
    _fall_key(0.86, (6.0, -88.0, -68.0), _body((12.0, -72.0, -6.0), (0.0, -4.0, 0.0), (0.0, -2.0, 0.0),
                                               (0.0, 2.0, 0.0), (0.0, 12.0, 0.0), (0.0, 10.0, 0.0)),
              ((-10.0, -28.0, 0.0), (10.0, -20.0, 0.0)),
              ((-58.0, -92.0, 7.0),) + _BOW_FLAT, ((55.0, -95.0, 14.0),) + _R_ROLL_110, (1.6, 0.3)),
    # Small bounce.
    _fall_key(0.95, (6.0, -89.0, -64.0), _body((12.0, -74.0, -6.0), (0.0, -1.0, 0.0), (0.0, 0.0, 0.0),
                                               (0.0, 0.0, 0.0), (0.0, 6.0, 0.0), (0.0, 4.0, 0.0)),
              ((-10.0, -27.0, 1.0), (10.0, -19.0, 1.0)),
              ((-59.0, -94.0, 10.0),) + _BOW_FLAT, ((56.0, -97.0, 17.0),) + _R_ROLL_155),
    # Lies still; the head settles last, rolling to his right.
    _fall_key(1.04, (6.0, -90.0, -69.0), _body((12.0, -78.0, -6.0), (0.0, -2.0, 0.0), (0.0, -2.0, 0.0),
                                               (0.0, -2.0, 0.0), (0.0, -2.0, 0.0), (6.0, -8.0, 6.0)),
              ((-12.0, -26.0, 0.0), (12.0, -18.0, 0.0)),
              ((-60.0, -96.0, 4.0),) + _BOW_FLAT, ((58.0, -100.0, 11.0),) + _R_LIMP),
]
ARCHER_FALL_KEYS.append(dict(ARCHER_FALL_KEYS[-1], t=1.1))
ARCHER_FALL = {"name": "A_Archer_Fall", "fps": 30, "props": ("bow", "quiver"), "scene": NONE,
               "elbows": FALL_ELBOWS, "fist": (55.0, 70.0, 45.0), "keys": ARCHER_FALL_KEYS, "curve": "hermite",
               # Reversed leads: the chest first, then the head, the arms, the pelvis and the feet last.
               "leads": {"pelvis": -0.03, "rot": {"spine_03": 0.03, "spine_02": 0.02, "spine_01": 0.01,
                                                  "neck_01": 0.0, "head": -0.015, "pelvis": -0.03,
                                                  "clavicle_l": 0.0, "clavicle_r": 0.0},
                         "hands": -0.02, "elbows": -0.02, "feet": -0.04},
               "preview": {"frames": 11, "zoom_picks": [0, 2, 5, 9, 17, 26]}}

# Validated by the user in preview (2026-10-20, j7c-archer-preview) and baked (J7c.1a, --soldiers
# --only=archer_idle,archer_walk,archer_attack,archer_fall). SKM_Archer always carries the arrow on ik_hand_gun, so the
# baker scales that bone to 0 where the arrow is not in the hand ("tool_hidden": spans in seconds, or True = every
# frame): from just after the release to the quiver in the attack (the preview's "hidden" span), all the time in the
# idle, the walk and the fall (no arrow nocked at rest).
ARCHER_ATTACK["tool_hidden"] = [ARCHER_ATTACK["hidden"]["arrow"]]
for _gesture in (ARCHER_IDLE, ARCHER_WALK, ARCHER_FALL):
    _gesture["tool_hidden"] = True
GESTURES.update({"archer_idle": ARCHER_IDLE, "archer_walk": ARCHER_WALK, "archer_attack": ARCHER_ATTACK,
                 "archer_fall": ARCHER_FALL})


# ---------------------------------------------------------------------------------------------------------------------
# Falls, round 2 (2026-10-20, PROPOSED: previews only). User's review: "les 3 falls sont nulles" (FALL_BODY: "pieds
# cimentés"); the Archer's dedicated fall was liked ("fall très bien"), so each troop gets its own fall in that style:
# a hit reaction in the chest (reversed leads: chest, head, arms, then the pelvis and the free feet last, with an
# overshoot), stumbling feet (anim_rig "feet"), the body down by ~1.0 s (the server destroys the unit at 1.0 s; the
# client keeps the body, plays the fall to its end and holds the LAST frame for 30 s, so the last pose lies still,
# nothing in the air, nothing in the ground). Each starts on the unit's attack guard (frame 0 of its attack: same
# pelvis, trunk, weapon and left hand, so no snap) and keeps the weapon in hand. They override the built falls of
# GESTURES in the previews only (preview_animation merges PROPOSED over GESTURES); the baker still bakes GESTURES.
FALL_LEADS = {"pelvis": -0.03, "rot": {"spine_03": 0.03, "spine_02": 0.02, "spine_01": 0.01, "neck_01": 0.0,
                                       "head": -0.015, "pelvis": -0.03, "clavicle_l": 0.0, "clavicle_r": 0.0},
              "tool": -0.01, "hands": -0.02, "elbows": -0.02, "feet": -0.04}


def _dk(t, pelvis, rot, feet, tool, left=None, flow=0.0, elbows=FALL_ELBOWS):
    key = {"t": t, "flow": flow, "pelvis": pelvis, "rot": rot, "feet": {"l": feet[0], "r": feet[1]}, "tool": tool,
           "elbows": elbows}
    if left:
        key["hands"] = {"l": {"at": left[0], "palm": left[1], "fingers": left[2]}}
    return key


def _start(pose, elbows):
    return dict(pose, t=0.0, flow=0.0, feet={"l": _ZERO, "r": _ZERO}, elbows=elbows)


def _sp(grip, d, spacing=40.0, roll=0.0):
    return {"grip": grip, "dir": normalize(d), "spacing": spacing, "roll": roll}


def _tl(grip, d):
    return {"grip": grip, "dir": normalize(d)}


# Spearman (1.0 s, 30 frames; user's choice, 2026-10-20: "comme s'il prenait une charge, il décolle un peu du sol (un
# peu, pas beaucoup) et tombe en arrière"): hit by a charge, a violent push knocks him BACKWARD THROUGH THE AIR A LITTLE.
# Released on frame 0 (gesture "release": the hands free, the spear on its own path, anim_rig): the chest driven back
# first, the head whipping (reversed leads), the arms flying forward and up, the hands off the shaft; a short airborne
# moment on a ballistic arc (fast out of the hit, a short hang, then gravity): the feet a little off the ground, the
# body tilting back as he flies, the legs trailing forward slightly bent; the back, hips and shoulders hit FAST (0.44 s),
# the head a hair later, a small rebound, a short slide back along the ground, settled; the legs drop last. The spear,
# knocked out of the hands, tumbles point first to his right and lands flat beside his legs, a small bounce.
# Dead on his back (the last keys): flat, the legs nearly straight, the arms splayed unevenly (the left one up beside
# the head, the right one out low), palms up, the head rolled to his left.
_SPL_PIVOT = (0.0, -10.0, 5.0)      # the heels: the body's pivot (right, forward, up), carried by the feet
_SPL_HIP = (0.0, -1.1, 96.8)        # the reference pelvis


def _plank(theta, lift=0.0, shift=(0.0, 0.0, 0.0)):
    """Pelvis offset of a body tilted `theta` degrees backward about the heels (lift: the knees bending/straightening),
    the heels moved by `shift`."""
    r = sub(add(_SPL_HIP, (0.0, 0.0, lift)), _SPL_PIVOT)
    c, s = math.cos(math.radians(theta)), math.sin(math.radians(theta))
    turned = (r[0], r[1] * c - r[2] * s, r[1] * s + r[2] * c)
    return add(sub(add(_SPL_PIVOT, turned), _SPL_HIP), shift)


def _hd(left, right, shift=(0.0, 0.0, 0.0)):
    """Both free hands: (at, palm, fingers) each, the places moved by `shift`."""
    return {"l": {"at": add(left[0], shift), "palm": left[1], "fingers": left[2]},
            "r": {"at": add(right[0], shift), "palm": right[1], "fingers": right[2]}}


def _sk(t, pelvis, rot, feet, spear, hands, flow=0.0, elbows=FALL_ELBOWS):
    key = _dk(t, pelvis, rot, feet, spear, flow=flow, elbows=elbows)
    key["hands"] = hands
    return key


def _ft(shift, left=(0.0, 0.0, 0.0), right=(0.0, 0.0, 0.0)):
    return add(shift, left), add(shift, right)


# Frame 0: the hands exactly where spear_attack's frame 0 holds the shaft (measured on the solved guard).
_SPL_GRIP_HANDS = _hd(((17.0, 12.0, 104.0), (0.707, -0.004, 0.707), (0.592, 0.386, -0.708)),
                      ((13.1, 50.2, 109.4), (-0.995, -0.098, -0.012), (-0.013, 0.271, -0.962)))
_SPL_LIE = 90.0
_SPL_UP = (0.0, 0.0, 6.0)           # lying: the pelvis on the thickness of the back, not in the ground
_SPL_LAND = (0.0, -78.0, 0.0)       # where the heels land (thrown back 0.78 m)...
_SPL_REST = (0.0, -92.0, 0.0)       # ...and come to rest after the slide
_SPL_FEET = ((-6.0, 2.0, -10.0), (2.0, 14.0, -5.5))  # z: the ankles down onto the heels (feet limp, toes up)
_SPL_ROT = _body((0.0, -_SPL_LIE, 0.0), (0.0, 2.0, 0.0), (0.0, 2.0, 0.0), (0.0, 0.0, 0.0), (8.0, 2.0, 0.0),
                 (24.0, 4.0, -8.0), 0.0)
_SPL_HANDS = (((-62.0, -168.0, 4.5), (0.0, 0.0, 1.0), (-0.6, -0.8, 0.0)),
              ((58.0, -100.0, 4.5), (0.2, 0.0, 0.98), (0.9, 0.4, 0.0)))
_SPL_ELBOWS = {"l": (-0.2, 0.3, -0.93), "r": (0.8, 0.0, -0.6)}
_SPL_SPEAR = ((46.0, -36.0, 2.0), (0.12, 1.0, 0.0))
SPEAR_FALL_KEYS = [
    dict(_start(SPEAR_GUARD, SPEAR_FIGHT_ELBOWS), hands=_SPL_GRIP_HANDS),
    # IMPACT: the chest driven back, the head snapped, the arms thrown forward and up, the hands off the shaft.
    _sk(0.06, _plank(4.0, -8.0, (0.0, -4.0, 0.0)),
        _body((0.0, -4.0, 0.0), (0.0, -4.0, 0.0), (0.0, -8.0, 0.0), (0.0, -16.0, 0.0), (0.0, -8.0, 0.0),
              (0.0, -18.0, 0.0), -6.0), _ft((0.0, -4.0, 0.0)),
        _sp((22.0, 14.0, 106.0), (-0.04, 0.98, 0.18)),
        _hd(((8.0, 18.0, 116.0), (0.55, -0.2, 0.8), (0.5, 0.75, -0.3)),
            ((24.0, 44.0, 120.0), (-0.9, -0.2, 0.35), (0.1, 0.6, -0.75))),
        flow=(2.4, 0.8), elbows=SPEAR_FIGHT_ELBOWS),
    # Off the ground, fast out of the hit: the body tilting back, the legs trailing forward.
    _sk(0.14, _plank(16.0, -10.0, (0.0, -30.0, 9.0)),
        _body((0.0, -16.0, 0.0), (0.0, -2.0, 0.0), (0.0, -4.0, 0.0), (0.0, -8.0, 0.0), (0.0, -2.0, 0.0),
              (0.0, -2.0, 0.0), -8.0), _ft((0.0, -30.0, 9.0), (0.0, 4.0, 2.0), (0.0, 8.0, 0.0)),
        _sp((32.0, 6.0, 108.0), (0.08, 0.96, 0.25)),
        _hd(((-2.0, 14.0, 132.0), (0.3, -0.4, 0.86), (0.3, 0.9, 0.0)),
            ((32.0, 34.0, 132.0), (-0.75, -0.3, 0.6), (0.25, 0.9, -0.2)), (0.0, -42.0, 4.0)),
        flow=1.2, elbows={"l": (-0.6, -0.4, -0.7), "r": (0.8, -0.3, -0.5)}),
    # The top of the arc: a short hang, the feet highest.
    _sk(0.24, _plank(34.0, -12.0, (0.0, -52.0, 14.0)),
        _body((0.0, -34.0, 0.0), (0.0, 0.0, 0.0), (0.0, -2.0, 0.0), (0.0, -4.0, 0.0), (0.0, 4.0, 0.0),
              (0.0, 6.0, 0.0), -8.0), _ft((0.0, -52.0, 14.0), (0.0, 8.0, 2.0), (0.0, 12.0, 0.0)),
        _sp((38.0, -6.0, 90.0), (0.12, 0.98, 0.05)),
        _hd(((-20.0, 4.0, 150.0), (0.3, -0.5, 0.8), (-0.1, 0.8, 0.6)),
            ((46.0, 12.0, 146.0), (-0.5, -0.5, 0.7), (0.4, 0.7, 0.6)), (0.0, -82.0, 0.0)),
        flow=0.9, elbows={"l": (-0.8, -0.3, -0.5), "r": (0.9, -0.3, -0.4)}),
    # Gravity: down, accelerating; the spear's point hits the ground.
    _sk(0.36, _plank(66.0, -10.0, (0.0, -70.0, 9.0)),
        _body((0.0, -66.0, 0.0), (0.0, 0.0, 0.0), (0.0, 0.0, 0.0), (0.0, -2.0, 0.0), (0.0, 8.0, 0.0),
              (0.0, 12.0, 0.0), -6.0), _ft((0.0, -70.0, 9.0), (-3.0, 6.0, 2.0), (1.0, 12.0, 0.0)),
        _sp((44.0, -26.0, 34.0), (0.12, 0.98, -0.19)),
        _hd(((-72.0, -58.0, 118.0), (0.4, -0.4, 0.8), (-0.8, -0.2, 0.55)),
            ((70.0, -22.0, 110.0), (-0.3, -0.5, 0.8), (0.8, 0.0, 0.6)), (0.0, -100.0, -20.0)),
        flow=1.6, elbows={"l": (-0.5, -0.1, -0.85), "r": (0.95, -0.2, -0.2)}),
    # CONTACT: the back, the hips and the shoulders slam into the ground (fast in); the head still up, the legs above.
    _sk(0.44, add(_plank(_SPL_LIE, -2.0, _SPL_LAND), _SPL_UP), dict(_SPL_ROT, neck_01=(0.0, 14.0, 0.0), head=(6.0, 10.0, 0.0)),
        _ft(_SPL_LAND, add(_SPL_FEET[0], (0.0, 0.0, 9.0)), add(_SPL_FEET[1], (0.0, 0.0, 7.0))),
        _sp(add(_SPL_SPEAR[0], (0.0, 0.0, 0.5)), _SPL_SPEAR[1]),
        _hd(((-64.0, -150.0, 22.0), (0.3, 0.0, 0.95), (-0.7, -0.6, 0.0)),
            ((62.0, -96.0, 18.0), (0.0, 0.0, 1.0), (0.9, 0.3, 0.0)), _SPL_LAND),
        flow=(2.6, 0.5), elbows=_SPL_ELBOWS),
    # The head hits a hair later; the rebound (3 cm) while sliding back, the arms slapping down, the spear bouncing.
    _sk(0.50, add(_plank(_SPL_LIE, -2.0, (0.0, -86.0, 0.0)), (0.0, 0.0, _SPL_UP[2] + 3.0)),
        dict(_SPL_ROT, neck_01=(4.0, -2.0, 0.0), head=(14.0, 2.0, -4.0)),
        _ft((0.0, -86.0, 0.0), add(_SPL_FEET[0], (0.0, 0.0, 3.0)), add(_SPL_FEET[1], (0.0, 0.0, 2.5))),
        _sp(add(_SPL_SPEAR[0], (0.0, 2.0, 3.0)), (0.12, 1.0, 0.02)),
        _hd((add(_SPL_HANDS[0][0], (0.0, 0.0, 2.0)),) + _SPL_HANDS[0][1:],
            (add(_SPL_HANDS[1][0], (0.0, 0.0, 2.0)),) + _SPL_HANDS[1][1:], (0.0, -86.0, 0.0)),
        flow=0.8, elbows=_SPL_ELBOWS),
    # Settled, dead (the legs down last).
    _sk(0.58, add(_plank(_SPL_LIE, -2.0, _SPL_REST), _SPL_UP), _SPL_ROT, _ft(_SPL_REST, *_SPL_FEET),
        _sp(add(_SPL_SPEAR[0], (0.0, 2.0, 0.0)), _SPL_SPEAR[1]), _hd(*_SPL_HANDS, shift=_SPL_REST), elbows=_SPL_ELBOWS),
]
# Dead on his back, the feet limp (anim_rig "foot_rot"): the toes up and forward (foot drop), each foot fallen outward
# about the leg into a loose V, the left more (about 50 deg) than the right (about 28 deg), the heels on the ground.
# Euler terms of a roll about the lying leg; they relax a little in the flight and arrive with the legs (0.58 s).
_SPL_FOOT_ROT = {"l": (-83.5, -40.0, -86.0), "r": (69.5, -60.4, 72.0)}
for _key in SPEAR_FALL_KEYS:
    _share = {0.36: 0.08, 0.44: 0.55, 0.50: 0.85, 0.58: 1.0}.get(_key["t"])
    if _share:
        _key["foot_rot"] = {s: tuple(_share * a for a in _SPL_FOOT_ROT[s]) for s in "lr"}
SPEAR_FALL_KEYS.append(dict(SPEAR_FALL_KEYS[-1], t=1.0))
SPEAR_FALL_PROPOSED = {"name": "A_Spearman_Fall", "fps": 30, "tool": SPEAR_THRUST_TOOL, "props": ("spear",),
                       "scene": NONE, "elbows": FALL_ELBOWS, "fist": (55.0, 70.0, 45.0), "stance": SPEAR_STANCE,
                       "keys": SPEAR_FALL_KEYS, "curve": "hermite", "release": 0.0,
                       # The hands and the spear on time (the release keyed exactly), the head lagging (whip).
                       "leads": dict(FALL_LEADS, tool=0.0, hands=0.0),
                       "preview": {"frames": 12, "zoom_picks": [0, 2, 4, 7, 9, 11, 13, 15, 17, 30]}}


# Man-at-arms (1.0 s, 30 frames; user's choice, 2026-10-20, option A: forward, face down): the FORWARD fall. Hit, he
# takes ONE HEAVY STEP FORWARD as if to keep fighting; the sword slips from his hand (gesture "release", 0.18 s: the
# sword falls on its own path, tumbles and lands flat to his right, a small bounce); the stepping leg lands with a
# stumble, the legs give, the knees hit the ground (fast in, a jolt, no hold) and the whole weight pitches forward,
# accelerating, and slams face down, the arms not catching him; a solid, low rebound (heavy armour) and settled.
# The shield cannot leave the left forearm (skinned to hand_l): the left arm is thrown out to his left, palm up, so the
# shield lies flat on the ground beside him. Every contact arrives fast (flow (2+, low)), then a 2-3 cm rebound.
# Dead face down (the last keys): the head turned to his side, the left arm thrown out with the shield, the right arm
# limp along his side palm up, the left leg straight, the right one a little bent; the sword loose by his right hand.
_MAL_PELVIS = (-2.0, 74.0, -83.0)
_MAL_ROT = _body((6.0, 86.0, 4.0), (0.0, 2.0, -3.0), (2.0, 2.0, -2.0), (4.0, 0.0, 0.0), (24.0, -10.0, 0.0),
                 (50.0, -12.0, 4.0), 0.0)
_MAL_FEET = ((-6.0, -24.0, 1.0), (12.0, 2.0, 6.0))
# Face down, the feet extended (anim_rig "foot_rot"; user: "des orteils tournes vers l'arriere"): the toes pointing
# back, their tips on the ground; the left foot straight, the right one rolled a little outward. They start turning
# as he pitches forward (0.52 s), the heels lifting a few cm while the toes pivot (feet keys of 0.63-0.78 s raised),
# so the toes never dig in.
_MAL_FOOT_ROT = {"l": (0.0, 125.0, 0.0), "r": (20.0, 110.0, 30.0)}
_MAL_HANDS = {"l": {"at": (-74.0, 136.0, 9.0), "palm": (0.0, 0.0, 1.0), "fingers": (-0.5, 0.85, 0.0)},
              "r": {"at": (28.0, 60.0, 5.0), "palm": (0.2, 0.0, 0.98), "fingers": (0.0, -1.0, 0.0)}}
_MAL_ELBOWS = {"l": (-0.4, -0.8, -0.4), "r": (0.9, -0.4, 0.0)}
_MAL_STUMBLE_ELBOWS = {"l": (-0.9, -0.3, -0.5), "r": (1.0, -0.2, -0.3)}
_MAL_SWORD = ((44.0, 80.0, 7.0), (0.25, 0.97, 0.0), 90.0)
# Before the release the right hand is on the sword (ignored by the rig, but the curve's neighbour); at the release it
# is where the solved hand holds the hilt, so the hand does not snap.
_MAL_R0 = {"at": (26.0, 18.0, 112.0), "palm": (-0.96, 0.27, -0.02), "fingers": (0.27, 0.96, -0.07)}
_MAL_SHIELD_LOW = {"at": (-22.0, 26.0, 92.0), "palm": (0.9, -0.2, -0.4), "fingers": (0.1, 0.4, -0.9)}


def _mk(t, pelvis, rot, feet, sword, left, right, flow=0.0, elbows=_MAL_ELBOWS):
    key = _dk(t, pelvis, rot, feet, sword, flow=flow, elbows=elbows)
    key["hands"] = {"l": left, "r": right}
    return key


def _sw(grip, d, roll=0.0):
    return {"grip": grip, "dir": normalize(d), "roll": roll}


def _at(hand, at):
    return dict(hand, at=at)


MAA_FALL_KEYS = [
    dict(_start(MAA_GUARD, MAA_FIGHT_ELBOWS), hands={"l": SHIELD_RAISED["l"], "r": _MAL_R0}),
    # IMPACT: a heavy jolt, the chest knocked back a little, the head dropping, the shield jarred.
    _mk(0.07, (0.0, -3.0, -7.0), _body((0.0, -2.0, 0.0), (0.0, -3.0, 0.0), (0.0, -4.0, 0.0), (0.0, -6.0, 0.0),
                                       (0.0, 10.0, 0.0), (0.0, 16.0, 0.0), -3.0), (_ZERO, _ZERO),
        _sw((27.0, 15.0, 109.0), (0.05, 0.25, 0.97)), _at(SHIELD_RAISED["l"], (-4.0, 28.0, 124.0)),
        _at(_MAL_R0, (27.0, 15.0, 109.0)), flow=(1.8, 0.5), elbows=MAA_FIGHT_ELBOWS),
    # RELEASE: the heavy step forward starts (the back, right foot lifting), the trunk leaning in, the hand opening.
    _mk(0.18, (1.0, 10.0, -8.0), _body((0.0, 6.0, 0.0), (0.0, 4.0, 0.0), (0.0, 4.0, 0.0), (0.0, 6.0, 0.0),
                                       (0.0, 6.0, 0.0), (0.0, 12.0, 0.0), -2.0), (_ZERO, (2.0, 22.0, 12.0)),
        _sw((28.0, 24.0, 102.0), (0.12, 0.45, 0.88)),
        {"at": (-10.0, 26.0, 112.0), "palm": (0.6, -0.8, 0.0), "fingers": (0.8, 0.5, -0.2)},
        _at(_MAL_R0, (28.0, 24.0, 102.0)), flow=1.0, elbows=MAA_FIGHT_ELBOWS),
    # The step lands, heavy (fast in): the momentum carries the trunk on, the sword falling, tumbling.
    _mk(0.30, (1.0, 28.0, -15.0), _body((0.0, 12.0, 0.0), (0.0, 6.0, 0.0), (0.0, 8.0, 0.0), (0.0, 10.0, 0.0),
                                        (0.0, 8.0, 0.0), (0.0, 14.0, 0.0), 0.0), ((0.0, -2.0, 0.0), (2.0, 46.0, 0.0)),
        _sw((38.0, 40.0, 66.0), (0.3, 0.85, -0.42), 20.0), _MAL_SHIELD_LOW,
        {"at": (26.0, 40.0, 94.0), "palm": (-0.9, 0.3, -0.3), "fingers": (0.1, 0.8, -0.6)},
        flow=(2.0, 0.4), elbows=_MAL_STUMBLE_ELBOWS),
    # The stumble: the legs giving (barely moving: the drop that follows accelerates).
    _mk(0.36, (1.0, 32.0, -19.0), _body((0.0, 14.0, 0.0), (0.0, 8.0, 0.0), (0.0, 10.0, 0.0), (0.0, 12.0, 0.0),
                                        (0.0, 8.0, 0.0), (0.0, 16.0, 0.0), 0.0), ((0.0, -4.0, 1.0), (2.0, 46.0, 0.0)),
        _sw((42.0, 54.0, 42.0), (0.35, 0.86, -0.36), 50.0), _at(_MAL_SHIELD_LOW, (-24.0, 28.0, 86.0)),
        {"at": (26.0, 44.0, 86.0), "palm": (-0.9, 0.2, -0.2), "fingers": (0.1, 0.6, -0.8)},
        flow=0.25, elbows=_MAL_STUMBLE_ELBOWS),
    # The sword lands flat (fast in); the legs folding.
    _mk(0.42, (1.0, 40.0, -34.0), _body((0.0, 18.0, 0.0), (0.0, 8.0, 0.0), (0.0, 10.0, 0.0), (0.0, 12.0, 0.0),
                                        (0.0, 8.0, 0.0), (0.0, 18.0, 0.0), 0.0), ((-1.0, -6.0, 2.0), (3.0, 44.0, 0.0)),
        _sw(add(_MAL_SWORD[0], (0.0, -4.0, 0.5)), _MAL_SWORD[1], _MAL_SWORD[2]),
        _at(_MAL_SHIELD_LOW, (-26.0, 32.0, 70.0)),
        {"at": (26.0, 48.0, 70.0), "palm": (-0.95, 0.1, -0.1), "fingers": (0.1, 0.4, -0.9)},
        flow=1.6, elbows=_MAL_STUMBLE_ELBOWS),
    # THE KNEES HIT (fast in, a jolt): the trunk thrown on, the head dropping, the sword bouncing.
    _mk(0.47, (1.0, 46.0, -46.0), _body((0.0, 22.0, 0.0), (0.0, 10.0, 0.0), (0.0, 12.0, 0.0), (0.0, 14.0, 0.0),
                                        (0.0, 12.0, 0.0), (0.0, 22.0, 0.0), 0.0), ((-2.0, -8.0, 3.0), (4.0, 40.0, 2.0)),
        _sw(add(_MAL_SWORD[0], (0.0, -2.0, 3.0)), (0.25, 0.97, 0.03), _MAL_SWORD[2]),
        _at(_MAL_SHIELD_LOW, (-30.0, 40.0, 54.0)),
        {"at": (28.0, 50.0, 54.0), "palm": (-0.9, -0.4, 0.0), "fingers": (0.1, 0.3, -0.95)},
        flow=(2.4, 0.3), elbows=_MAL_STUMBLE_ELBOWS),
    # The rebound off the knees (2 cm), already pitching on: no hold.
    _mk(0.52, (1.0, 50.0, -44.0), _body((0.0, 32.0, 0.0), (0.0, 12.0, 0.0), (0.0, 14.0, 0.0), (0.0, 14.0, 0.0),
                                        (0.0, 12.0, 0.0), (0.0, 22.0, 0.0), 0.0), ((-2.0, -10.0, 3.0), (5.0, 36.0, 2.0)),
        _sw(*_MAL_SWORD),
        {"at": (-36.0, 46.0, 50.0), "palm": (0.85, 0.35, 0.2), "fingers": (-0.3, 0.5, -0.8)},
        {"at": (30.0, 54.0, 50.0), "palm": (-0.4, -0.9, 0.0), "fingers": (0.0, 0.3, -0.95)},
        flow=0.6, elbows=_MAL_STUMBLE_ELBOWS),
    # Pitching forward like a felled tree, accelerating; the arms trailing, not reaching for the ground.
    _mk(0.63, (0.0, 64.0, -62.0), _body((2.0, 58.0, 2.0), (0.0, 6.0, -2.0), (0.0, 6.0, -2.0), (2.0, 6.0, 0.0),
                                        (6.0, 4.0, 0.0), (10.0, 6.0, 0.0), 0.0), ((-4.0, -18.0, 5.0), (10.0, 10.0, 8.0)),
        _sw(*_MAL_SWORD),
        {"at": (-58.0, 80.0, 50.0), "palm": (0.5, 0.5, 0.7), "fingers": (-0.7, 0.7, 0.0)},
        {"at": (30.0, 58.0, 40.0), "palm": (0.6, -0.6, 0.5), "fingers": (0.0, -0.5, -0.85)},
        flow=1.8, elbows={"l": (-0.7, -0.6, -0.4), "r": (0.9, -0.4, 0.0)}),
    # CONTACT, face down (fast in): the chest and the hips slam; the head still a little up, the arms flung down.
    _mk(0.72, add(_MAL_PELVIS, (0.0, 0.0, -1.0)), dict(_MAL_ROT, neck_01=(10.0, -14.0, 0.0), head=(20.0, -18.0, 2.0)),
        (add(_MAL_FEET[0], (0.0, 0.0, 11.0)), add(_MAL_FEET[1], (0.0, 0.0, 7.0))), _sw(*_MAL_SWORD),
        dict(_MAL_HANDS["l"], at=add(_MAL_HANDS["l"]["at"], (6.0, -10.0, 12.0)), palm=(0.35, 0.0, 0.94)),
        dict(_MAL_HANDS["r"], at=add(_MAL_HANDS["r"]["at"], (0.0, 0.0, 8.0)), palm=(0.6, 0.0, 0.8)), flow=(2.6, 0.3)),
    # The rebound (3 cm, low and solid), the head hitting a hair later, the arms slapping down.
    _mk(0.78, add(_MAL_PELVIS, (0.0, 0.0, 2.0)), dict(_MAL_ROT, neck_01=(20.0, -8.0, 0.0), head=(46.0, -8.0, 4.0)),
        (add(_MAL_FEET[0], (0.0, 0.0, 4.5)), add(_MAL_FEET[1], (0.0, 0.0, 2.5))), _sw(*_MAL_SWORD),
        _at(_MAL_HANDS["l"], add(_MAL_HANDS["l"]["at"], (0.0, 0.0, 2.0))),
        _at(_MAL_HANDS["r"], add(_MAL_HANDS["r"]["at"], (0.0, 0.0, 2.0))), flow=0.5),
    # Settled, dead.
    _mk(0.86, _MAL_PELVIS, _MAL_ROT, _MAL_FEET, _sw(*_MAL_SWORD), _MAL_HANDS["l"], _MAL_HANDS["r"]),
]
# The feet extending (from flat at 0.52 s): (left, right) "foot_rot" by key time.
for _key in MAA_FALL_KEYS:
    _rot = {0.63: ((0.0, 20.0, 0.0), (0.0, 20.0, 0.0)), 0.72: ((0.0, 70.0, 0.0), (5.0, 65.0, 10.0)),
            0.78: ((0.0, 115.0, 0.0), (15.0, 105.0, 20.0)),
            0.86: (_MAL_FOOT_ROT["l"], _MAL_FOOT_ROT["r"])}.get(_key["t"])
    if _rot:
        _key["foot_rot"] = {"l": _rot[0], "r": _rot[1]}
MAA_FALL_KEYS.append(dict(MAA_FALL_KEYS[-1], t=1.0))
MAA_FALL_PROPOSED = {"name": "A_ManAtArms_Fall", "fps": 30, "tool": MAA_STRIKE_TOOL, "props": ("sword", "shield"),
                     "scene": NONE, "elbows": FALL_ELBOWS, "fist": (55.0, 70.0, 45.0), "stance": MAA_STANCE,
                     "keys": MAA_FALL_KEYS, "curve": "hermite", "release": 0.18,
                     # The hands and the sword on time (the release keyed exactly), the head lagging.
                     "leads": dict(FALL_LEADS, tool=0.0, hands=0.0),
                     "preview": {"frames": 12, "zoom_picks": [0, 2, 5, 9, 11, 14, 16, 19, 22, 30]}}

# Peasant (1.15 s, 35 frames): a dead-weight collapse, not a lying down (user's review of round 2: "they seemed to lie
# down to sleep"). Every contact with the ground ARRIVES FAST (flow (2+, low): fast in, no ease-out into a rest) and
# is followed by a tiny rebound and an immediate settle. A small jolt in the chest (reversed leads: chest, head, arms,
# the pelvis last), then the legs give at once: the pelvis drops ACCELERATING (the key before the drop barely moves,
# flow 0.2), the knees forward and down onto the ground, the feet in place; the torso jolts on the knees. A limp moment
# on his knees (the trunk sways, the head drops forward), then the whole upper body topples to his left like a log,
# pivoting about the knees, accelerating to the ground; the arms trail, the head (lagging, lead -0.015) hits last.
# Dead on his left side (the last keys): the legs still folded from kneeling, twisted by the topple; the under (left)
# arm flung back behind him on the ground, the top (knife) arm thrown out forward, the knife flat on the ground in the
# loose hand, away from the face; the head on the ground, tipped.
_PL_PELVIS = (-36.0, 18.0, -72.0)
_PL_ROT = _body((6.0, 0.0, -82.0), (4.0, 2.0, -4.0), (-4.0, 4.0, 2.0), (-6.0, 2.0, 4.0), (8.0, 10.0, -2.0),
                (10.0, 6.0, 12.0), -6.0)
_PL_ROT["clavicle_r"] = (-20.0, 0.0, 6.0)
_PL_FEET = ((25.0, -20.0, 1.5), (-27.0, -1.0, 4.0))  # the ankles down (feet limp), the upper one toward the lower
_PL_GRIP, _PL_DIR, _PL_ROLL = (-70.0, 62.0, 5.0), (0.4, 0.9, -0.05), 90.0
_PL_LEFT = ((-75.0, -20.0, 7.5), (0.2, 0.0, -0.98), (0.0, -1.0, 0.0))
_PL_ELBOWS = {"l": (-0.35, -0.7, 0.45), "r": (-0.45, 0.2, -0.75)}
_PL_KNEEL_FEET = ((12.0, -14.0, 6.0), (-12.0, -2.0, 6.0))
_PL_LIMP_ELBOWS = {"l": (-0.4, -0.3, -1.0), "r": (0.4, -0.3, -1.0)}


def _pk(grip, d, roll=0.0):
    return {"grip": grip, "dir": normalize(d), "roll": roll}


PEASANT_FALL_KEYS = [
    _start(dict(PEASANT_GUARD, tool=dict(PEASANT_GUARD["tool"], roll=0.0)), PEASANT_FIGHT_ELBOWS),
    # IMPACT: the chest knocked back, the head snapped back, the cover arm flung up and out.
    _dk(0.07, (0.0, -8.0, -10.0), afraid(merge(hip_turn(10.0, 4.0, 4.0, nod=-12.0),
                                               {"spine_03": (0.0, -8.0, 0.0), "spine_02": (0.0, -4.0, 0.0)}), 1.0, 8.0),
        (_ZERO, _ZERO), _pk((18.0, 8.0, 104.0), (-0.3, 0.7, 0.65)),
        ((-14.0, 18.0, 136.0), (0.4, -0.9, 0.2), (0.0, 0.3, 0.95)), flow=(2.2, 0.6), elbows=PEASANT_FIGHT_ELBOWS),
    # Overshoot; the legs just starting to give (barely moving: the drop that follows accelerates).
    _dk(0.13, (0.0, -7.0, -13.0), afraid(merge(hip_turn(8.0, 2.0, 3.0, nod=-14.0),
                                               {"spine_03": (0.0, -10.0, 0.0), "spine_02": (0.0, -5.0, 0.0)}), 0.6, 6.0),
        (_ZERO, _ZERO), _pk((20.0, 6.0, 100.0), (-0.2, 0.6, 0.75)),
        ((-18.0, 14.0, 128.0), (0.75, -0.65, 0.0), (0.0, 0.6, 0.8)), flow=0.2,
        elbows={"l": PEASANT_FIGHT_ELBOWS["l"], "r": _PL_LIMP_ELBOWS["r"]}),
    # The knees hit the ground, hard (fast in): the trunk jolts forward on the contact, the arms flop down.
    _dk(0.33, (0.0, 18.0, -50.0), _body((0.0, 4.0, 0.0), (0.0, 6.0, 0.0), (0.0, 10.0, 0.0), (0.0, 12.0, 0.0),
                                        (0.0, 16.0, 0.0), (4.0, 14.0, 0.0), -6.0),
        _PL_KNEEL_FEET, _pk((20.0, 22.0, 52.0), (0.1, 0.65, -0.75)),
        ((-20.0, 20.0, 50.0), (0.8, 0.0, -0.6), (0.0, 0.7, -0.7)), flow=(2.4, 0.3), elbows=_PL_LIMP_ELBOWS),
    # Small rebound off the knees.
    _dk(0.39, (0.0, 17.0, -47.0), _body((0.0, 2.0, 0.0), (0.0, 4.0, 0.0), (0.0, 6.0, 0.0), (0.0, 8.0, 0.0),
                                        (0.0, 20.0, 0.0), (4.0, 18.0, 0.0), -5.0),
        tuple(add(f, (0.0, 0.0, 0.5)) for f in _PL_KNEEL_FEET), _pk((20.0, 20.0, 54.0), (0.1, 0.55, -0.83)),
        ((-20.0, 18.0, 52.0), (0.8, 0.0, -0.6), (0.0, 0.7, -0.7)), flow=0.6, elbows=_PL_LIMP_ELBOWS),
    # Limp on his knees: nothing holds him up, the trunk sags and starts to sway to his left, the head hanging.
    _dk(0.52, (-3.0, 16.0, -49.0), _body((0.0, 6.0, -6.0), (0.0, 6.0, -3.0), (0.0, 8.0, -3.0), (0.0, 8.0, -2.0),
                                         (0.0, 26.0, -2.0), (6.0, 20.0, -6.0), -8.0),
        (add(_PL_KNEEL_FEET[0], (-4.0, 0.0, 0.0)), _PL_KNEEL_FEET[1]), _pk((18.0, 18.0, 50.0), (0.05, 0.4, -0.92)),
        ((-24.0, 16.0, 48.0), (0.8, 0.0, -0.6), (0.0, 0.5, -0.87)), flow=0.25, elbows=_PL_LIMP_ELBOWS),
    # Toppling to his left about the knees, accelerating; the arms trailing up, the head lagging.
    _dk(0.74, (-18.0, 17.0, -55.0), _body((3.0, 14.0, -42.0), (2.0, 4.0, -4.0), (-2.0, 6.0, -2.0), (-3.0, 6.0, 0.0),
                                          (4.0, 22.0, 4.0), (6.0, 16.0, 6.0), -10.0),
        ((14.0, -16.0, 40.0), (-14.0, -4.0, 8.0)), _pk((8.0, 30.0, 44.0), (0.3, 0.6, -0.74)),
        ((-40.0, 4.0, 52.0), (0.6, 0.2, -0.7), (-0.4, -0.2, -0.9)), flow=1.5,
        elbows={"l": (-0.6, -0.2, 0.2), "r": (0.5, -0.1, -0.8)}),
    # Contact, the shoulder and the side (fast in).
    _dk(0.88, add(_PL_PELVIS, (0.0, 0.0, -1.0)), dict(_PL_ROT, head=(10.0, 12.0, 4.0), neck_01=(8.0, 16.0, 0.0)),
        (add(_PL_FEET[0], (0.0, 0.0, 4.0)), _PL_FEET[1]), _pk(add(_PL_GRIP, (0.0, 0.0, 1.0)), _PL_DIR, _PL_ROLL),
        _PL_LEFT, flow=(2.4, 0.3),
        elbows=_PL_ELBOWS),
    # Rebound (3 cm).
    _dk(0.94, add(_PL_PELVIS, (0.0, 0.0, 2.0)), dict(_PL_ROT, head=(10.0, 4.0, 16.0)),
        tuple(add(f, (0.0, 0.0, 1.5)) for f in _PL_FEET), _pk(add(_PL_GRIP, (0.0, 0.0, 2.0)), _PL_DIR, _PL_ROLL),
        ((add(_PL_LEFT[0], (0.0, 0.0, 1.5)),) + _PL_LEFT[1:]), flow=0.5, elbows=_PL_ELBOWS),
    # Settled, dead.
    _dk(1.02, _PL_PELVIS, _PL_ROT, _PL_FEET, _pk(_PL_GRIP, _PL_DIR, _PL_ROLL), _PL_LEFT, elbows=_PL_ELBOWS),
]
# Dead on his side, the feet limp (anim_rig "foot_rot"): rolled with the body, the under (left) foot on its outer edge,
# extended along the shin (ankle about 140 deg); the upper (right) one rolled less and drooping more, lying on and just
# in front of it. They roll over during the topple and arrive with the side's contact (0.88 s).
_PL_FOOT_ROT = {"l": (109.7, 15.0, -80.1), "r": (118.3, 36.5, -92.5)}
for _key in PEASANT_FALL_KEYS:
    _share = {0.74: 0.2, 0.88: 0.85, 0.94: 1.0, 1.02: 1.0}.get(_key["t"])
    if _share:
        _key["foot_rot"] = {s: tuple(_share * a for a in _PL_FOOT_ROT[s]) for s in "lr"}
PEASANT_FALL_KEYS.append(dict(PEASANT_FALL_KEYS[-1], t=1.15))
PEASANT_FALL_PROPOSED = {"name": "A_Peasant_Fall", "fps": 30, "tool": PEASANT_KNIFE_TOOL, "props": ("knife",),
                         "scene": NONE, "elbows": FALL_ELBOWS, "fist": (40.0, 55.0, 35.0), "stance": PEASANT_STANCE,
                         "keys": PEASANT_FALL_KEYS, "curve": "hermite",
                         # The feet on time (no lag): the knees stay on the ground as he topples.
                         "leads": dict(FALL_LEADS, feet=0.0),
                         "preview": {"frames": 12, "zoom_picks": [0, 2, 10, 12, 16, 22, 26, 28, 31, 34]}}

# Validated by the user in preview (2026-10-20, 2c8859b / 54faf78) and baked (J7c, --soldiers
# --only=spear_fall,maa_fall,peasant_fall): they replace the refused falls of J7b.5 under the same keys and names
# (A_Spearman_Fall, A_ManAtArms_Fall, A_Peasant_Fall, rebuilt in place). Not loops: the last frame is the pose the
# client holds (30 s, not implemented yet; the server still destroys the body at 1 s).
GESTURES.update({"spear_fall": SPEAR_FALL_PROPOSED, "maa_fall": MAA_FALL_PROPOSED,
                 "peasant_fall": PEASANT_FALL_PROPOSED})

# ---------------------------------------------------------------------------------------------------------------------
# Crossbowman (AR1 pieces, AR2 idle and walk, 2026-10-26; validated, baked in AR6a). The stirrup crossbow (soldier_props
# "crossbow", id 8) is the two-handed tool on ik_hand_gun, lying flat in front of the chest, its point forward and to
# his left: the left hand under the fore-stock behind the prod, the right hand at the trigger in front of the right
# hip, the elbows low and close to the body; the loaded bolt (id 5, ik_hand_r) in the groove, the bolt quiver (id 6) on
# the right hip. Idle on A_SW_Idle (breathing and sway of the base), walk on A_SW_Unarmed_Walk with the same carry
# (hands_frame "chest", steady_chest like the soldiers' walks); WalkPlayRate = 720 / 168.2 = 4.28 (UnitTypes.csv, AR5).
CROSSBOW_TOOL = {"kind": "crossbow", "hands": 2, "handle_length": sp.CROSSBOW_LENGTH,
                 "knob_offset": sp.CROSSBOW_LEFT_GRIP, "hand_spacing": sp.CROSSBOW_HAND_SPACING,
                 "edge": (0.0, 0.0, 1.0)}  # the groove up: lying flat
CROSSBOW_ELBOWS = {"l": (-0.6, -0.4, -0.7), "r": (0.3, -0.6, -0.8)}
CROSSBOW_REST = {"grip": (-16.0, 36.0, 106.0), "dir": normalize((0.8, -0.58, -0.1))}


def crossbow_gesture(name, base, keys, steady):
    g = armed(name, base, CROSSBOW_TOOL, ("crossbow", "bolt", "bolt_quiver"), keys, CROSSBOW_ELBOWS, steady)
    for key in g["keys"]:
        del key["hands"]  # both hands on the crossbow
    return g


CROSSBOW_IDLE = crossbow_gesture("A_Crossbow_Idle", IDLE, idle_keys(STAND, CROSSBOW_REST, None),
                                 {"steady_spine": 1.0})
CROSSBOW_WALK = crossbow_gesture("A_Crossbow_Walk", UNARMED_WALK, walk_keys(STAND, CROSSBOW_REST, None),
                                 {"steady_chest": 0.5})

# Attack (AR3, 2026-10-26; validated, baked in AR6a): AttackInterval 2.125 s (AR-Q1), the SHOT at 12 % of the loop
# (AttackWindUpPercent 12, HOLUnitAnimation::GetStrikeHitShare, like the Archer's release: the client launches the
# visual bolt there). 64 frames at 30 fps (2.133 s, played at 2.133 / 2.125 in the game, like the soldiers), the shot
# on frame 8 (CROSSBOW_SHOT, 0.125 of the loop, 0.266 s in the game): 0.12 x 64 = 7.68, frame 8 is the nearest one;
# the 0.005 of a loop between the two (~11 ms in the game) is accepted, as the Archer's (frame 6 of 48). 63 frames
# would put it at 0.127 (frame 8 of 63), farther. The loop STARTS LOADED, crossbow low (the idle's carry, so it blends
# from the idle and the walk); AR-Q4:
#   quick raise to the shoulder (the trunk turning to his right, the left shoulder toward the target, the butt against
#   the right shoulder under the cheek, the head bowed to the tiller), a short aim -> SHOT (frame 8: the shoulder and
#   the crossbow kicked back a few cm, the point up a few degrees: flow (0, 2)) -> settle -> the point lowered to the
#   ground in front, the hands leave the stock (gesture "release" spans, anim_rig: the crossbow on its own path) -> the
#   RIGHT foot steps forward into the stirrup ("feet": the ball of the foot on the stirrup's bar) -> he bends over,
#   both hands on the string by the prod, and pulls it up to the nut straightening up -> the foot back, the crossbow
#   raised low by the left hand on the fore-stock -> the right hand to the bolt quiver on the right hip and back to
#   the groove (the bolt placed: mimed, no bolt in the hand, AR1) -> the right hand back at the trigger (the release
#   span ends: both hands on the stock again) -> ready, loaded, to the loop's end. The string is rigid on the crossbow
#   (always drawn, one bone; accepted). Hermite keys, the pelvis leading the spine, the spine the shoulders, the hands
#   last; the left foot planted, the right one planted except its step into the stirrup and back.
# The roll of the crossbow is held by the tool "fingers" of every key: the groove (the tool's Y, its "edge") is the
# opposite of the hands' fingers squared to the tiller (exact: the edge is fixed in the right hand, read at frame 0
# where the fingers point straight down and the groove up), so a key says where the groove faces by its fingers.
# The loaded bolt (ik_hand_r) is hidden from just after the shot to the placing (CROSSBOW_ATTACK["bolt_hidden"]).
CROSSBOW_PERIOD = 2.125
CROSSBOW_SHOT = 0.125  # share of the loop at the shot, frame 8 of 64 (AttackWindUpPercent 12 / 100, ~11 ms apart)
CROSSBOW_STANCE = {"l": (-4.0, 12.0, 0.0), "r": (6.0, -8.0, 0.0)}
# Free hand's palm per side from its fingers and the tiller (body terms): a hand on the stock seen as a free hand
# (anim_rig: the palm side is cross(fingers, across) for the right hand, the other way for the left; body terms are
# mirrored against the component space), and its reference fingers 77 deg from its pinky-to-index axis.
_CB_PALM_SIGN = {"l": 1.0, "r": -1.0}
_CB_FINGERS_ALONG = 0.22


def _cross_b(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def cb_tool(grip, d, fingers, spacing=None):
    t = {"grip": grip, "dir": normalize(d), "fingers": normalize(fingers)}
    if spacing is not None:
        t["spacing"] = spacing
    return t


def cb_axes(tool):
    """The crossbow's frame in body terms for a tool key: knob (stirrup's end), X (toward the butt), Y (the groove),
    Z (across, toward his left when the groove faces up and the butt is behind)."""
    x = normalize(tool["dir"])
    f = tool["fingers"]
    fp = normalize(sub(f, tuple(x[k] * sum(f[j] * x[j] for j in range(3)) for k in range(3))))
    y = tuple(-c for c in fp)
    z = normalize(_cross_b(x, y))
    knob = sub(tool["grip"], tuple(c * sp.CROSSBOW_LEFT_GRIP for c in x))
    return knob, x, y, z


def cb_point(tool, x, y=0.0, z=0.0):
    knob, ax, ay, az = cb_axes(tool)
    return tuple(knob[k] + x * ax[k] + y * ay[k] + z * az[k] for k in range(3))


def cb_hand(tool, side, x=None):
    """A hand on the stock as a free hand (palm and fingers of the tool hand there): the left at its grip, the right
    at the trigger, or at `x` cm from the stirrup's end."""
    knob, ax, ay, az = cb_axes(tool)
    if x is None:
        x = sp.CROSSBOW_LEFT_GRIP + (0.0 if side == "l" else tool.get("spacing", sp.CROSSBOW_HAND_SPACING))
    fp = tuple(-c for c in ay)
    palm = normalize(tuple(_CB_PALM_SIGN[side] * c for c in _cross_b(fp, ax)))
    fingers = normalize(tuple(fp[k] * 0.976 + ax[k] * _CB_FINGERS_ALONG for k in range(3)))
    return {"at": cb_point(tool, x), "palm": palm, "fingers": fingers}


def cb_key(t, pose, flow=0.0):
    """A key of the attack: on the stock when the pose has no "hands" (both hands keyed on their grips)."""
    key = merge({"elbows": CROSSBOW_ELBOWS}, pose, {"t": t, "flow": flow})
    if "hands" not in key:
        key["hands"] = {side: cb_hand(key["tool"], side) for side in ("l", "r")}
    return key


DOWN = (0.0, 0.0, -1.0)
# Ready: the idle's carry (CROSSBOW_REST), fingers straight down (the groove up).
CB_READY = {"pelvis": (0.0, 0.0, -2.0), "rot": hip_turn(8.0, 2.0, 3.0, nod=-2.0),
            "tool": cb_tool(CROSSBOW_REST["grip"], CROSSBOW_REST["dir"], DOWN)}
# Aim: level forward, the point up AIM_ELEVATION, the butt in front of the right shoulder.
AIM_ELEVATION = 3.0
_AIM_DIR = (0.0, -math.cos(math.radians(AIM_ELEVATION)), -math.sin(math.radians(AIM_ELEVATION)))
CB_BUTT_AIM = (9.0, -14.0, 151.0)


def cb_butt_tool(butt, d, fingers=DOWN):
    d = normalize(d)
    return cb_tool(tuple(butt[k] - (sp.CROSSBOW_LENGTH - sp.CROSSBOW_LEFT_GRIP) * d[k] for k in range(3)), d, fingers)


def cb_aim_tool(butt, elevation=AIM_ELEVATION, spacing=None):
    d = (0.0, -math.cos(math.radians(elevation)), -math.sin(math.radians(elevation)))
    grip = tuple(butt[k] - (sp.CROSSBOW_LENGTH - sp.CROSSBOW_LEFT_GRIP) * d[k] for k in range(3))
    return cb_tool(grip, d, DOWN, spacing)


AIM_ELBOWS = {"l": (-0.6, -0.2, -1.0), "r": (1.0, -0.2, 0.1)}
CB_RAISE = {"pelvis": (0.0, 0.0, -3.0), "rot": hip_turn(25.0, 1.0, 9.0, nod=0.0),
            "tool": cb_butt_tool((34.0, -10.0, 118.0), (0.55, -0.83, -0.1)), "elbows": AIM_ELBOWS}
CB_SHOULDER = {"pelvis": (-0.5, 0.5, -4.0), "rot": merge(hip_turn(58.0, 2.0, 19.0, nod=8.0),
                                                          {"clavicle_r": (4.0, 0.0, 2.0), "clavicle_l": (14.0, 0.0, 0.0)}),
               "tool": cb_aim_tool(CB_BUTT_AIM), "elbows": AIM_ELBOWS}


def _cb_mix(a, b, s):
    """a + (b - a) * s, per component (tuples, or dicts of tuples: a missing entry counts as zero)."""
    if isinstance(a, dict):
        zero = (0.0, 0.0, 0.0)
        return {k: _cb_mix(a.get(k, zero), b.get(k, zero), s) for k in set(a) | set(b)}
    return tuple(x + (y - x) * s for x, y in zip(a, b))


def cb_between(a, b, share):
    """A key `share` of the way from pose a to pose b (the raise's in-between key)."""
    ta, tb = a["tool"], b["tool"]
    return {"pelvis": _cb_mix(a["pelvis"], b["pelvis"], share), "rot": _cb_mix(a["rot"], b["rot"], share),
            "tool": cb_tool(_cb_mix(ta["grip"], tb["grip"], share), _cb_mix(ta["dir"], tb["dir"], share),
                            _cb_mix(ta["fingers"], tb["fingers"], share)),
            "elbows": {s: normalize(_cb_mix(a["elbows"][s], b["elbows"][s], share)) for s in ("l", "r")}}


CB_RAISE_2 = cb_between(CB_RAISE, CB_SHOULDER, 0.55)
CB_AIM = dict(CB_SHOULDER, rot=merge(hip_turn(59.0, 2.0, 19.0, nod=8.5), {"clavicle_r": (5.0, 0.0, 2.0), "clavicle_l": (14.0, 0.0, 0.0)}),
              tool=cb_aim_tool((9.0, -14.5, 151.0)))
CB_SHOT = dict(CB_AIM, tool=cb_aim_tool((9.0, -15.0, 151.0)))
CB_KICK = {"pelvis": (-0.5, -1.0, -4.0), "rot": merge(hip_turn(60.0, -1.0, 19.0, nod=6.0),
                                                       {"clavicle_r": (10.0, 0.0, 1.0), "clavicle_l": (14.0, 0.0, 0.0)}),
           "tool": cb_aim_tool((9.5, -19.0, 152.5), AIM_ELEVATION + 4.0), "elbows": AIM_ELBOWS}
# Reload: the stirrup on the ground at CB_STIRRUP, where the ball of the right foot comes (its place + the step
# CB_STEP: on the stirrup's bar, 2 cm up), the crossbow leaning RELOAD_LEAN forward (the butt away from him, clear of
# his knee and head), the groove toward him (fingers forward).
RELOAD_LEAN = 14.0
_RELOAD_DIR = (0.0, math.sin(math.radians(RELOAD_LEAN)), math.cos(math.radians(RELOAD_LEAN)))
CB_STEP = (-11.9, 27.6, 2.0)  # right foot's offset in the stirrup (ball at the stirrup's bar)
CB_STIRRUP = (12.0, 28.0, 0.3)
FORWARD = (0.0, 1.0, 0.0)


def cb_reload_tool(knob=CB_STIRRUP, d=_RELOAD_DIR, fingers=FORWARD):
    d = normalize(d)
    return cb_tool(tuple(knob[k] + sp.CROSSBOW_LEFT_GRIP * d[k] for k in range(3)), d, fingers)


def on_string(tool, x, y=3.0, z=6.0):
    """Both hands on the string either side of the stock, `x` cm from the stirrup's end (the left hand to his left),
    palms toward the stock... seen from him: palms forward-down, fingers hooking the string."""
    a, b = cb_point(tool, x, y, z), cb_point(tool, x, y, -z)
    left, right = (a, b) if a[0] < b[0] else (b, a)
    return {"l": {"at": left, "palm": (0.5, 0.6, -0.6), "fingers": (0.2, 0.3, -0.93)},
            "r": {"at": right, "palm": (-0.5, 0.6, -0.6), "fingers": (-0.2, 0.3, -0.93)}}


def stoop(hip, bend, nod=-12.0, twist=0.0):
    """Bent over from the hips: the pelvis takes `hip` degrees (hip_turn's pelvis takes only 15 % of a bend, the
    hinge would sit at the waist), the spine `bend` more."""
    rot = hip_turn(twist, bend, twist * 0.3, nod=nod)
    rot["pelvis"] = (rot["pelvis"][0], hip, 0.0)
    return rot


RELOAD_ELBOWS = {"l": (-1.0, -0.2, 0.0), "r": (1.0, -0.2, 0.0)}
_DOWN_TOOL = cb_reload_tool()
# The left shoulder forward (clavicle_l) so the left hand reaches its place on the stock (AR3b-2).
CB_DOWN = {"pelvis": (0.0, -10.0, -14.0), "rot": merge(stoop(30.0, 25.0), {"clavicle_l": (22.0, 0.0, 0.0)}),
           "tool": _DOWN_TOOL,
           "hands": {"l": cb_hand(_DOWN_TOOL, "l", 80.0), "r": cb_hand(_DOWN_TOOL, "r", 90.0)},
           "elbows": RELOAD_ELBOWS}
# Lowering (AR3b-2): from the aim to the stirrup the crossbow makes the shortest turn, a half-turn about
# _CB_TURN_AXIS (body terms): halfway the butt is up on his right, the point down to his left, clear of the chest.
# Both hands stay ON the stock and slide along it (the release starts right after the recoil, the hands keyed free
# on points of the stock): the left from its grip (22 cm from the stirrup's end) to 80, the right from the trigger
# (67) to 90, as in CB_DOWN; the stirrup's end travels ~180 cm in 16 frames (~11 cm a frame, the keys evenly spaced
# along its path, no stop). Each key: the turn (degrees), the left hand's place (body terms), the hands' distances
# along the stock, the share of the way from the recoil's body to CB_DOWN's (squared for the elbows: the left forearm
# kept low, under the prod's limb).
_CB_TURN_AXIS = normalize((0.0, -1.0, 1.0))


def _cb_turned(v, deg):
    """v turned `deg` degrees about _CB_TURN_AXIS (Rodrigues)."""
    n, c, s = _CB_TURN_AXIS, math.cos(math.radians(deg)), math.sin(math.radians(deg))
    nv = sum(n[k] * v[k] for k in range(3))
    nxv = _cross_b(n, v)
    return tuple(v[k] * c + nxv[k] * s + n[k] * nv * (1.0 - c) for k in range(3))


def cb_lowering(turn, left, x_l, x_r, share, start, end=None, elbow_l=None):
    """A key of the lowering (see above); `elbow_l` overrides the left elbow's mixed direction."""
    end = end or CB_DOWN
    d = normalize(_cb_turned(_AIM_DIR, turn))
    tool = cb_tool(tuple(left[k] - d[k] * (x_l - sp.CROSSBOW_LEFT_GRIP) for k in range(3)), d,
                   _cb_turned(DOWN, turn))
    key = {"pelvis": _cb_mix(start["pelvis"], end["pelvis"], share),
           "rot": _cb_mix(start["rot"], end["rot"], share), "tool": tool,
           "hands": {"l": cb_hand(tool, "l", x_l), "r": cb_hand(tool, "r", x_r)},
           "elbows": {s: normalize(_cb_mix(start["elbows"][s], end["elbows"][s], share * share))
                      for s in ("l", "r")}}
    if elbow_l is not None:
        key["elbows"]["l"] = normalize(elbow_l)
    return key


CB_SETTLE = cb_lowering(17.3, (8.5, 52.8, 148.4), 22.0, 67.0, 0.09, CB_KICK, elbow_l=(-0.4, -0.3, -1.0))
CB_LOWER_1 = cb_lowering(51.8, (16.2, 40.6, 123.7), 37.8, 73.3, 0.27, CB_KICK, elbow_l=(0.05, -0.3, -1.0))
CB_LOWER_2 = cb_lowering(86.4, (25.5, 39.8, 107.8), 48.4, 77.5, 0.45, CB_KICK)
CB_LOWER_3 = cb_lowering(120.9, (28.9, 42.7, 99.3), 58.9, 81.6, 0.64, CB_KICK)
CB_LOWER_4 = cb_lowering(155.5, (20.5, 42.7, 92.5), 69.5, 85.8, 0.82, CB_KICK)
CB_STEPPING = dict(CB_DOWN, pelvis=(-3.0, -10.0, -14.0), rot=stoop(32.0, 25.0),
                   feet={"r": (-6.0, 14.0, 11.0)})
CB_FOOT = dict(CB_DOWN, pelvis=(0.0, -14.0, -18.0), rot=stoop(38.0, 28.0), feet={"r": CB_STEP},
               hands={"l": cb_hand(_DOWN_TOOL, "l", 64.0), "r": cb_hand(_DOWN_TOOL, "r", 76.0)})
CB_BENT = {"pelvis": (0.0, -20.0, -34.0), "rot": stoop(65.0, 40.0), "tool": _DOWN_TOOL,
           "hands": on_string(_DOWN_TOOL, 28.0, 3.0, 7.0), "feet": {"r": CB_STEP}, "elbows": RELOAD_ELBOWS}
CB_PULLED = {"pelvis": (0.0, -18.0, -28.0), "rot": stoop(55.0, 35.0), "tool": _DOWN_TOOL,
             "hands": on_string(_DOWN_TOOL, 38.0, 3.5, 4.0), "feet": {"r": CB_STEP}, "elbows": RELOAD_ELBOWS}
CB_GRAB = dict(CB_PULLED, pelvis=(0.0, -18.0, -28.0), rot=merge(stoop(55.0, 36.0), {"clavicle_l": (22.0, 0.0, 0.0)}),
               hands={"l": cb_hand(_DOWN_TOOL, "l", 42.0), "r": on_string(_DOWN_TOOL, 39.0, 3.5, 4.0)["r"]})
# Lift (AR3c): from the grab to the bolt laid in the groove the crossbow goes straight from the stirrup on the ground
# to the low carry (_CARRY_TOOL), its stirrup's end on a line, its direction on the great circle, evenly in time
# (_LIFT_N segments, the first and last half as long: the ease out of the grab and into the placing), the left hand
# ON the stock (keyed on the key's tool, sliding from 42 cm to its grip), the body from the grab's stoop to the
# placing's carry, the right foot out of the stirrup back to its place in the first half (lifted CB_LIFT_FOOT cm),
# the right hand off the string to the bolt quiver and the bolt laid in the groove (_cb_right_lift).
_LIFT_T0, _LIFT_T1, _LIFT_N = 0.665, 0.88, 12
CB_LIFT_FOOT = 8.0
CB_LIFT_LAG = 2.0  # the body's share of the way is u ** CB_LIFT_LAG: it straightens up behind the hands (reach)
_CARRY_TOOL = cb_tool((-22.0, 40.0, 106.0), (0.95, -0.2, -0.2), DOWN)
_QUIVER_R = {"at": (29.0, -4.0, 110.0), "palm": (-1.0, 0.0, 0.0), "fingers": (0.0, -0.15, -1.0)}
_PLACE_R = {"at": cb_point(_CARRY_TOOL, 36.0, 5.0), "palm": DOWN, "fingers": normalize((-0.8, 0.5, 0.0))}
CB_PLACE_BODY = {"pelvis": (0.0, -2.0, -4.0), "rot": hip_turn(10.0, 8.0, 3.0, nod=-10.0)}


def _cb_slerp(a, b, s):
    a, b = normalize(a), normalize(b)
    w = math.acos(max(-1.0, min(1.0, sum(x * y for x, y in zip(a, b)))))
    if w < 1e-6:
        return a
    return normalize(tuple((x * math.sin((1.0 - s) * w) + y * math.sin(s * w)) / math.sin(w) for x, y in zip(a, b)))


def _cb_right_lift(u):
    """The right hand `u` of the way along its path during the lift (by length, so at the lift's even pace): off the
    string -> the bolt quiver's mouth (a bolt taken in passing, mimed) -> laid in the groove."""
    path = [CB_GRAB["hands"]["r"], _QUIVER_R, _PLACE_R]
    lengths = [math.dist(a["at"], b["at"]) for a, b in zip(path, path[1:])]
    left = u * sum(lengths)
    for ha, hb, n in zip(path, path[1:], lengths):
        if left <= n + 1e-9:
            s = left / n
            return {"at": _cb_mix(ha["at"], hb["at"], s), "palm": _cb_slerp(ha["palm"], hb["palm"], s),
                    "fingers": _cb_slerp(ha["fingers"], hb["fingers"], s)}
        left -= n
    return dict(_PLACE_R)


def cb_lift(u):
    """The lift's key `u` of the way (0 the grab, 1 the placing)."""
    k0, d0 = cb_axes(_DOWN_TOOL)[:2]
    k1, d1 = cb_axes(_CARRY_TOOL)[:2]
    d = _cb_slerp(d0, d1, u)
    knob = _cb_mix(k0, k1, u)
    tool = cb_tool(tuple(knob[k] + d[k] * sp.CROSSBOW_LEFT_GRIP for k in range(3)), d,
                   _cb_slerp(_DOWN_TOOL["fingers"], _CARRY_TOOL["fingers"], u))
    f = min(1.0, u / 0.5)
    foot = _cb_mix(CB_STEP, (0.0, 0.0, 0.0), f * f * (3.0 - 2.0 * f))
    b = u ** CB_LIFT_LAG
    return {"pelvis": _cb_mix(CB_GRAB["pelvis"], CB_PLACE_BODY["pelvis"], b),
            "rot": _cb_mix(CB_GRAB["rot"], CB_PLACE_BODY["rot"], b), "tool": tool,
            "hands": {"l": cb_hand(tool, "l", 42.0 + (sp.CROSSBOW_LEFT_GRIP - 42.0) * u), "r": _cb_right_lift(u)},
            "feet": {"r": (foot[0], foot[1], foot[2] + CB_LIFT_FOOT * math.sin(math.pi * f))},
            "elbows": {s: normalize(_cb_mix(RELOAD_ELBOWS[s], CROSSBOW_ELBOWS[s], u)) for s in ("l", "r")}}


_LIFT_KEYS = [(_LIFT_T0 + i * (_LIFT_T1 - _LIFT_T0) / _LIFT_N, (i - 0.5) / (_LIFT_N - 1)) for i in range(1, _LIFT_N)]
CB_PLACE = cb_lift(1.0)
CB_GRASP = {"pelvis": (0.0, -1.0, -3.0), "rot": hip_turn(9.0, 4.0, 3.0, nod=-4.0), "tool": _CARRY_TOOL,
            "elbows": CROSSBOW_ELBOWS}
CROSSBOW_LEADS = {"pelvis": 0.04, "rot": {"pelvis": 0.04, "spine_01": 0.03, "spine_02": 0.02, "spine_03": 0.012,
                                          "neck_01": 0.01, "head": 0.01, "clavicle_r": 0.006},
                  "elbows": 0.004, "tool": 0.0, "hands": 0.0}
_P = CROSSBOW_PERIOD
CROSSBOW_ATTACK = fighting("A_Crossbow_Attack", CROSSBOW_TOOL, ("crossbow", "bolt", "bolt_quiver"), [
    cb_key(0.0, CB_READY, (0.0, 1.0)),
    cb_key(at(_P, 0.05), CB_RAISE, 1.0),          # quick raise
    cb_key(at(_P, 0.075), CB_RAISE_2, 1.0),       # on the way up
    cb_key(at(_P, 0.1), CB_SHOULDER),             # butt to the shoulder, cheek on the stock
    cb_key(at(_P, 0.11), CB_AIM),                 # aim, a cm of creep
    cb_key(at(_P, CROSSBOW_SHOT), CB_SHOT, (0.0, 2.0)),  # SHOT (frame 8 of 64): the bolt leaves
    cb_key(at(_P, 0.155), CB_KICK, (0.0, 1.0)),   # recoil: shoulder and crossbow kicked back, the point up
    cb_key(at(_P, 0.18), CB_SETTLE, 1.0),         # settled back: the hands let go of the grips (release)
    cb_key(at(_P, 0.225), CB_LOWER_1, 1.0),       # lowering: the half-turn, the hands sliding along the stock
    cb_key(at(_P, 0.27), CB_LOWER_2, 1.0),        # halfway: the butt up on his right, the point to his left
    cb_key(at(_P, 0.315), CB_LOWER_3, 1.0),
    cb_key(at(_P, 0.36), CB_LOWER_4, 1.0),        # the point down in front
    cb_key(at(_P, 0.41), CB_DOWN, (0.6, 0.0)),    # stirrup on the ground, held upright by the stock
    cb_key(at(_P, 0.45), CB_STEPPING, 0.8),       # the right foot lifted, stepping forward
    cb_key(at(_P, 0.50), CB_FOOT),                # the foot in the stirrup
    cb_key(at(_P, 0.57), CB_BENT, 0.4),           # bent over, both hands on the string by the prod
    cb_key(at(_P, 0.64), CB_PULLED),              # pulled up to the nut, straightening
    cb_key(at(_P, _LIFT_T0), CB_GRAB),            # the left hand to the fore-stock
] + [cb_key(at(_P, t), cb_lift(u), 1.0) for t, u in _LIFT_KEYS] + [  # the lift (see above)
    cb_key(at(_P, _LIFT_T1), CB_PLACE),           # carried low, the bolt laid in the groove: it shows again
    cb_key(at(_P, 0.94), CB_GRASP, 0.5),          # the right hand back at the trigger (end of the release)
    cb_key(_P, CB_READY),
], CROSSBOW_ELBOWS, 30, CROSSBOW_STANCE, [0, 3, 8, 10, 22, 26, 33, 37, 45, 52, 57, 60])
CROSSBOW_ATTACK.update({
    "frames": 64, "curve": "hermite", "loop": True, "leads": CROSSBOW_LEADS,
    "hit_fraction": CROSSBOW_SHOT,  # the baker's readback looks for the shot there
    # Both hands off the grips from just after the recoil to the right hand back at the trigger (anim_rig "release"
    # span).
    "release": [(at(_P, 0.18), at(_P, 0.94))],
    # The loaded bolt hidden from just after the shot to its placing (seconds). The preview hides the piece by
    # "hidden"; the BAKER (make_peasant_animations.py, AR6) must scale ik_hand_r (the bolt's bone, never keyed
    # otherwise) to ~0 in these spans, as it scales ik_hand_gun for "tool_hidden" (which would hide the crossbow).
    "bolt_hidden": [(at(_P, CROSSBOW_SHOT) + 0.01, at(_P, _LIFT_T1))]})
CROSSBOW_ATTACK["hidden"] = {"bolt": CROSSBOW_ATTACK["bolt_hidden"][0]}

# Fall (AR4a, 2026-10-26; validated, baked in AR6a; AR-Q6 precised: forward, face down). 1.0 s, 30 frames, not looped.
# Struck on his left: the chest knocked first (reversed leads, FALL_LEADS), the crossbow escaping his hands on frame 0
# (gesture "release": it flies forward-left on its own path, tumbles and lands flat in front of him, a small bounce,
# the loaded bolt kept in its groove); he twists about a quarter turn to his LEFT, stumbling (the right foot then the
# left step round, anim_rig "feet"), the legs give, the knees hit (fast in), and he pitches forward onto his belly
# (fast in, a low rebound). The body is keyed in his turned frame (as if facing forward) and turned about the vertical
# through the feet's centre by _cbf (positions, palms, elbows, the pelvis twist and the feet's twist), each foot on its
# own schedule (the stumble); the crossbow is keyed in the final turned frame (_CBF_TURN). Dead face down (the last
# keys): the chest and the face on the ground, the head turned to his left, the right arm thrown forward beside the
# head palm down, the left arm along his side palm up, the left leg straight, the right one a little bent, the toes
# back (foot_rot).
_CBF_TURN = -80.0
_CBF_PLACES = {"l": (-21.1, 3.9, 0.0), "r": (23.1, -16.1, 0.0)}  # reference feet + CROSSBOW_STANCE (right, forward)


def _cbf_v(v, psi):
    c, s = math.cos(math.radians(psi)), math.sin(math.radians(psi))
    return (v[0] * c + v[1] * s, -v[0] * s + v[1] * c, v[2])


def _cbf_hand(h, psi):
    return {"at": _cbf_v(h[0], psi), "palm": normalize(_cbf_v(h[1], psi)), "fingers": normalize(_cbf_v(h[2], psi))}


def _cbf(t, psi, pelvis, rot, feet, hands, elbows, tool, flow=0.0, foot_bend=(_ZERO, _ZERO), back=0.0):
    """A key of the fall in his turned frame: `psi` = (body, left foot, right foot) turns in degrees; `back` (cm)
    pulls the body, the hands and the feet back toward his start (AR4b: he lies within his cell)."""
    body, fl, fr = psi
    bk = (0.0, -back, 0.0)
    pelvis, feet = add(pelvis, bk), (add(feet[0], bk), add(feet[1], bk))
    hands = tuple((add(h[0], bk),) + tuple(h[1:]) for h in hands)
    p0 = (0.0, -1.1, 96.8)
    pel = sub(_cbf_v(add(p0, pelvis), body), p0)
    rot = dict(rot, pelvis=(rot["pelvis"][0] + body,) + tuple(rot["pelvis"][1:]))
    fe = {}
    for side, psi_f, off in (("l", fl, feet[0]), ("r", fr, feet[1])):
        place = _CBF_PLACES[side]
        fe[side] = sub(_cbf_v(add(place, off), psi_f), place)
    fb = {"l": (foot_bend[0][0] + fl,) + tuple(foot_bend[0][1:]),
          "r": (foot_bend[1][0] + fr,) + tuple(foot_bend[1][1:])}
    return {"t": t, "flow": flow, "pelvis": pel, "rot": rot, "feet": fe, "foot_rot": fb,
            "hands": {"l": _cbf_hand(hands[0], body), "r": _cbf_hand(hands[1], body)},
            "elbows": {s: normalize(_cbf_v(elbows[s], body)) for s in ("l", "r")}, "tool": tool}


def _cbf_mix(a, b, u):
    """A rotation dict between two others (u = 0..1)."""
    return {k: tuple(x + (y - x) * u for x, y in zip(a[k], b[k])) for k in a}


def _cbf_tool(grip, d, fingers, psi=_CBF_TURN):
    """The crossbow on its own path, keyed in his final turned frame."""
    return cb_tool(_cbf_v(grip, psi), _cbf_v(d, psi), _cbf_v(fingers, psi))


_CBF_START = cb_key(0.0, CB_READY)
_CBF_DEAD_PELVIS = (-2.0, 74.0, -83.0)
_CBF_DEAD_ROT = _body((-6.0, 86.0, -4.0), (0.0, 2.0, 3.0), (-2.0, 2.0, 2.0), (-4.0, 0.0, 0.0), (-24.0, -10.0, 0.0),
                      (-50.0, -12.0, -4.0), 0.0)
_CBF_DEAD_FEET = ((-6.0, -24.0, 1.0), (12.0, 2.0, 6.0))
_CBF_DEAD_FOOT = ((0.0, 125.0, 0.0), (-20.0, 110.0, -30.0))
_CBF_DEAD_HANDS = (((-30.0, 58.0, 5.0), (-0.2, 0.0, 0.98), (0.0, -1.0, 0.0)),
                   ((62.0, 126.0, 6.0), (0.0, 0.0, -1.0), (0.4, 0.9, 0.0)))
_CBF_DEAD_ELBOWS = {"l": (-0.9, -0.4, 0.0), "r": (0.4, -0.8, -0.4)}
_CBF_STUMBLE_ELBOWS = {"l": (-0.9, -0.2, -0.5), "r": (0.9, -0.3, -0.5)}
_CBF_BACK = (14.0, 20.0, 32.0, 42.0, 50.0)  # AR4b: pulled back toward his start at 0.43, 0.49, 0.59, 0.67, 0.75+
_CBF_REST_GRIP, _CBF_REST_DIR = (-70.0, 86.0, 8.5), (0.8, -0.6, 0.0)
_CBF_REST = _cbf_tool(_CBF_REST_GRIP, _CBF_REST_DIR, DOWN)
_CBF_ROT_HIT = _body((0.0, 0.0, 2.0), (-4.0, -2.0, 3.0), (-5.0, -3.0, 4.0), (-8.0, -6.0, 5.0), (0.0, -6.0, 0.0),
                     (6.0, -14.0, -6.0), -6.0)
_CBF_ROT_OVER = _body((0.0, -2.0, 3.0), (-6.0, -2.0, 4.0), (-6.0, -2.0, 4.0), (-6.0, -4.0, 4.0), (0.0, -2.0, 0.0),
                      (4.0, -4.0, -4.0), -8.0)


def _cbf_bend(pelvis_bend, s, neck, head):
    return _body((0.0, pelvis_bend, 0.0), (0.0, s, 0.0), (0.0, s, 0.0), (0.0, s + 2.0, 0.0), (0.0, neck, 0.0),
                 (0.0, head, 0.0), 0.0)


_CBF_ZF = (_ZERO, _ZERO)
CROSSBOW_FALL_KEYS = [
    dict(_start(_CBF_START, CROSSBOW_ELBOWS), hands=_CBF_START["hands"], foot_rot={"l": _ZERO, "r": _ZERO}),
    # IMPACT (his left shoulder): the chest knocked back and to his right, the head snapped, the hands flung open.
    _cbf(0.06, (-4.0, 0.0, 0.0), (-3.0, -2.0, -2.0), _CBF_ROT_HIT, _CBF_ZF,
         (((-30.0, 30.0, 118.0), (-0.5, -0.8, 0.3), (0.1, 0.3, -0.95)),
          ((30.0, 14.0, 114.0), (0.5, 0.8, 0.3), (0.1, 0.3, -0.95))), _CBF_STUMBLE_ELBOWS,
         cb_tool((-26.0, 46.0, 116.0), (0.85, -0.45, -0.25), (0.2, 0.0, -1.0)), flow=(2.2, 0.6)),
    # Overshoot, turning to his left: the left foot pivots, the right one lifts and steps round.
    _cbf(0.15, (-18.0, -10.0, -14.0), (-4.0, 2.0, -5.0), _CBF_ROT_OVER, (_ZERO, (0.0, 4.0, 10.0)),
         (((-42.0, 20.0, 114.0), (-0.2, -0.8, 0.55), (0.0, 0.55, -0.8)),
          ((38.0, 8.0, 108.0), (0.2, 0.8, 0.55), (0.0, 0.55, -0.8))), _CBF_STUMBLE_ELBOWS,
         cb_tool((-52.0, 60.0, 110.0), (0.9, -0.2, -0.4), (0.5, 0.0, -0.85)), flow=1.0),
    # The right foot lands round, the left lifts; the legs start giving.
    _cbf(0.25, (-42.0, -24.0, -42.0), (-3.0, 10.0, -14.0), _cbf_bend(8.0, 5.0, 6.0, 10.0), ((0.0, 4.0, 8.0), _ZERO),
         (((-38.0, 30.0, 96.0), (0.62, -0.78, 0.05), (0.38, 0.24, -0.89)),
          ((36.0, 24.0, 94.0), (-0.62, 0.78, 0.05), (0.3, 0.29, -0.91))), _CBF_STUMBLE_ELBOWS,
         cb_tool((-74.0, 56.0, 80.0), (0.9, 0.1, -0.4), (0.6, 0.0, -0.8)), flow=1.0),
    # The knees buckle, the left foot down; he keeps turning.
    _cbf(0.35, (-64.0, -56.0, -58.0), (-2.0, 24.0, -30.0), _cbf_bend(16.0, 8.0, 8.0, 14.0),
         ((0.0, -4.0, 2.0), (0.0, -4.0, 2.0)),
         (((-34.0, 42.0, 74.0), (0.85, 0.0, -0.5), (0.0, 0.5, -0.85)),
          ((34.0, 40.0, 72.0), (-0.85, 0.0, -0.5), (0.0, 0.5, -0.85))), _CBF_STUMBLE_ELBOWS,
         _cbf_tool(add(_CBF_REST_GRIP, (6.0, -24.0, 26.0)), (0.85, -0.5, -0.15), (0.2, 0.0, -1.0)), flow=1.4,
         back=6.0),
    # The crossbow lands (fast in); THE KNEES HIT (fast in, a jolt), the trunk thrown on, the feet kicked back.
    _cbf(0.43, (-76.0, -76.0, -72.0), (-1.0, 40.0, -46.0), _cbf_bend(22.0, 10.0, 12.0, 22.0),
         ((0.0, -10.0, 4.0), (0.0, -10.0, 4.0)),
         (((-32.0, 50.0, 54.0), (0.9, -0.4, 0.0), (0.0, 0.3, -0.95)),
          ((30.0, 52.0, 52.0), (-0.9, -0.4, 0.0), (0.0, 0.3, -0.95))), _CBF_STUMBLE_ELBOWS,
         _cbf_tool(add(_CBF_REST_GRIP, (2.0, -6.0, 0.5)), _CBF_REST_DIR, DOWN), flow=(2.4, 0.3), back=_CBF_BACK[0]),
    # The rebound off the knees, already pitching on; the crossbow bounces and slides; the right hand turning.
    _cbf(0.49, (-80.0, -80.0, -76.0), (-1.0, 50.0, -44.0), _cbf_bend(32.0, 12.0, 12.0, 22.0),
         ((0.0, -12.0, 4.0), (0.0, -12.0, 4.0)),
         (((-36.0, 54.0, 50.0), (0.85, 0.35, 0.2), (-0.3, 0.5, -0.8)),
          ((34.0, 56.0, 50.0), (-0.9, -0.4, -0.15), (0.05, 0.4, -0.9))), _CBF_STUMBLE_ELBOWS,
         _cbf_tool(add(_CBF_REST_GRIP, (1.0, -3.0, 3.0)), _CBF_REST_DIR, DOWN), flow=0.6, back=_CBF_BACK[1]),
    # Pitching forward, accelerating; the left arm trailing, the right one swinging forward, palm turning down.
    _cbf(0.59, (-80.0, -80.0, -78.0), (-1.0, 64.0, -62.0), _cbf_bend(58.0, 6.0, 6.0, 10.0),
         ((-4.0, -14.0, 5.0), (4.0, -20.0, 6.0)),
         (((-40.0, 68.0, 44.0), (0.81, 0.2, 0.56), (-0.18, 0.0, -0.98)),
          ((44.0, 80.0, 44.0), (-0.4, 0.0, -0.92), (0.3, 0.9, -0.15))), _CBF_STUMBLE_ELBOWS, _CBF_REST, flow=1.6,
         foot_bend=((0.0, 20.0, 0.0), (0.0, 20.0, 0.0)), back=_CBF_BACK[2]),
    # Half-way down: the trunk nearly flat, the head still up, the right hand reaching forward.
    _cbf(0.67, (-80.0, -80.0, -79.0), (-1.5, 70.0, -73.0),
         dict(_cbf_mix(_cbf_bend(58.0, 6.0, 6.0, 10.0), _CBF_DEAD_ROT, 0.55), neck_01=(-6.0, -10.0, 0.0),
              head=(-10.0, -14.0, -2.0)),
         ((-5.0, -19.0, 12.0), (8.0, -9.0, 11.0)),
         (((-34.0, 68.0, 26.0), (0.6, 0.0, 0.8), (0.0, -0.5, -0.85)),
          ((50.0, 100.0, 30.0), (-0.15, 0.2, -0.97), (0.4, 0.9, -0.05))), _CBF_DEAD_ELBOWS, _CBF_REST, flow=1.6,
         foot_bend=((-2.0, 45.0, -5.0), (0.0, 45.0, 0.0)), back=_CBF_BACK[3]),
    # CONTACT face down (fast in): the chest and the hips slam, the head still a little up.
    _cbf(0.75, (-80.0, -80.0, -80.0), _CBF_DEAD_PELVIS,
         dict(_CBF_DEAD_ROT, neck_01=(-10.0, -14.0, 0.0), head=(-20.0, -18.0, -2.0)),
         (add(_CBF_DEAD_FEET[0], (0.0, 0.0, 11.0)), add(_CBF_DEAD_FEET[1], (0.0, 0.0, 7.0))),
         ((add(_CBF_DEAD_HANDS[0][0], (0.0, 0.0, 8.0)), (0.2, 0.0, 0.98), (0.0, -1.0, 0.0)),
          (add(_CBF_DEAD_HANDS[1][0], (-6.0, -10.0, 12.0)), (0.0, 0.3, -0.95), (0.4, 0.9, 0.0))), _CBF_DEAD_ELBOWS,
         _CBF_REST, flow=(1.8, 0.3), foot_bend=((-5.0, 70.0, -10.0), (0.0, 70.0, 0.0)), back=_CBF_BACK[4]),
    # The rebound (3 cm, low), the head hitting a hair later, the arms slapping down.
    _cbf(0.80, (-80.0, -80.0, -80.0), add(_CBF_DEAD_PELVIS, (0.0, 0.0, 2.0)),
         dict(_CBF_DEAD_ROT, neck_01=(-20.0, -8.0, 0.0), head=(-46.0, -8.0, -4.0)),
         (add(_CBF_DEAD_FEET[0], (0.0, 0.0, 4.5)), add(_CBF_DEAD_FEET[1], (0.0, 0.0, 2.5))),
         ((add(_CBF_DEAD_HANDS[0][0], (0.0, 0.0, 2.0)),) + _CBF_DEAD_HANDS[0][1:],
          (add(_CBF_DEAD_HANDS[1][0], (0.0, 0.0, 2.0)),) + _CBF_DEAD_HANDS[1][1:]), _CBF_DEAD_ELBOWS,
         _CBF_REST, flow=0.5, foot_bend=((-15.0, 105.0, -20.0), (0.0, 115.0, 0.0)), back=_CBF_BACK[4]),
    # Settled, dead.
    _cbf(0.86, (-80.0, -80.0, -80.0), _CBF_DEAD_PELVIS, _CBF_DEAD_ROT, _CBF_DEAD_FEET, _CBF_DEAD_HANDS,
         _CBF_DEAD_ELBOWS, _CBF_REST, foot_bend=_CBF_DEAD_FOOT, back=_CBF_BACK[4]),
]
CROSSBOW_FALL_KEYS.append(dict(CROSSBOW_FALL_KEYS[-1], t=1.0))
CROSSBOW_FALL = {"name": "A_Crossbow_Fall", "fps": 30, "tool": CROSSBOW_TOOL,
                 "props": ("crossbow", "bolt", "bolt_quiver"), "scene": NONE, "elbows": FALL_ELBOWS,
                 "fist": (55.0, 70.0, 45.0), "stance": CROSSBOW_STANCE, "keys": CROSSBOW_FALL_KEYS,
                 "curve": "hermite", "release": 0.0,
                 # The hands and the crossbow on time (the release keyed exactly), the head lagging.
                 "leads": dict(FALL_LEADS, tool=0.0, hands=0.0),
                 "preview": {"frames": 12, "zoom_picks": [0, 2, 5, 8, 11, 13, 15, 18, 21, 25, 30]}}


# Validated by the user in preview (AR1-AR4, 2026-10-26) and baked (AR6a, --soldiers
# --only=crossbow_idle,crossbow_walk,crossbow_attack,crossbow_fall). The crossbow stays on ik_hand_gun all the time;
# the baker scales ik_hand_r (the loaded bolt) to ~0 in the attack's "bolt_hidden" spans.
GESTURES.update({"crossbow_idle": CROSSBOW_IDLE, "crossbow_walk": CROSSBOW_WALK, "crossbow_attack": CROSSBOW_ATTACK,
                 "crossbow_fall": CROSSBOW_FALL})
