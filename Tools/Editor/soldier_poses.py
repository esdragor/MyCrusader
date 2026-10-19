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

# Peasant attack (proposal, preview only until the user validates it: in PROPOSED, not in GESTURES, so --soldiers does
# not bake it). User's choice (2026-10-19): the Peasant fights with a knife, not the axe: a one-handed stab into the
# enemy's belly ~1 m in front. He is not a fighter (user's review of the first stab, 2026-10-19): a frightened villager
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

# Gestures shown by the previews but not baked yet (waiting for the user's review).
PROPOSED = {"peasant_attack": PEASANT_ATTACK}
