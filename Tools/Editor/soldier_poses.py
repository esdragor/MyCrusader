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
# Attacks and fall (J7b.5, previews only: not baked yet). The attack loops last exactly one attack period of the unit
# table (Spearman 1.875 s, Man-at-arms 1.375 s), so they play at rate 1 (before the client's MeshScale division) and
# the HIT lands at HALF the loop (ATTACK_HIT_FRACTION): the server's first hit comes after a 50 % wind-up, then one hit
# per period, so a loop started with the attack keeps every hit on the hit frame. Feet planted (no base: legs by IK).
ATTACK_HIT_FRACTION = 0.5


def at(period, share):
    """Key time (s) at `share` of a loop of `period` seconds."""
    return period * share


def fighting(name, tool, props, keys, elbows, fps, stance, picks):
    g = {"name": name, "fps": fps, "tool": tool, "props": props, "scene": NONE, "elbows": elbows,
         "fist": (55.0, 70.0, 45.0), "stance": stance, "keys": keys,
         "preview": {"frames": 8, "zoom_picks": picks}}
    return g


# Spearman thrust: both hands on the shaft (left hand behind, right hand in front), the spear level at the waist on
# his right side, left foot forward. Guard, draw back, fast stab forward (point ~2.2 m ahead at chest height), short
# hold, recover to the guard.
SPEAR_PERIOD = 1.875
SPEAR_THRUST_TOOL = {"kind": "spear", "hands": 2, "handle_length": sp.SPEAR_LENGTH, "knob_offset": 70.0,
                     "hand_spacing": 40.0, "edge": (1.0, 0.0, 0.0)}
SPEAR_STANCE = {"l": (-4.0, 16.0, 0.0), "r": (6.0, -14.0, 0.0)}
SPEAR_FIGHT_ELBOWS = {"l": (-0.3, -0.5, -1.0), "r": (0.8, -0.4, -0.6)}
SPEAR_GUARD = {"pelvis": (0.0, 0.0, -6.0), "rot": torso(-10.0, 6.0, 0.0, nod=-4.0),
               "tool": {"grip": (20.0, 10.0, 104.0), "dir": (0.04, 0.99, 0.12), "spacing": 40.0}}
SPEAR_DRAWN = {"pelvis": (0.0, -6.0, -7.0), "rot": torso(-20.0, 0.0, 0.0, nod=-4.0),
               "tool": {"grip": (22.0, -6.0, 108.0), "dir": (0.03, 0.99, 0.10), "spacing": 40.0}}
SPEAR_LUNGE = {"pelvis": (0.0, 10.0, -10.0), "rot": torso(5.0, 16.0, 0.0, nod=-10.0),
               "tool": {"grip": (17.0, 36.0, 116.0), "dir": (0.0, 0.995, 0.10), "spacing": 36.0}}


def spear_key(t, pose, ease="inout"):
    return dict(pose, t=t, ease=ease, elbows=SPEAR_FIGHT_ELBOWS)


SPEAR_ATTACK = fighting("A_Spearman_Attack", SPEAR_THRUST_TOOL, ("spear",), [
    spear_key(0.0, SPEAR_GUARD),
    spear_key(at(SPEAR_PERIOD, 0.12), SPEAR_GUARD),
    spear_key(at(SPEAR_PERIOD, 0.42), SPEAR_DRAWN),           # drawn back, cocked
    spear_key(at(SPEAR_PERIOD, 0.50), SPEAR_LUNGE, "in"),     # THE HIT: point fully out (frame 28 of 56)
    spear_key(at(SPEAR_PERIOD, 0.58), SPEAR_LUNGE, "out"),    # short hold at full reach
    spear_key(at(SPEAR_PERIOD, 0.85), SPEAR_GUARD),           # back to the guard
    spear_key(SPEAR_PERIOD, SPEAR_GUARD),
], SPEAR_FIGHT_ELBOWS, 30, SPEAR_STANCE, [0, 14, 24, 28, 32, 44])

# Man-at-arms strike: a diagonal cut from over the right shoulder down to his left in front, the shield raised in
# front of the chest the whole time (left hand at chest height, face forward). 32 fps so the hit is a whole frame
# (22 of 44).
MAA_PERIOD = 1.375
MAA_STRIKE_TOOL = {"kind": "sword", "hands": 1, "handle_length": sp.SWORD_LENGTH, "knob_offset": sp.SWORD_GRIP,
                   "edge": (0.0, 1.0, 0.0)}  # the cutting edge forward at the guard (sword upright): it leads the cut
MAA_STANCE = {"l": (-4.0, 14.0, 0.0), "r": (6.0, -12.0, 0.0)}
SHIELD_RAISED = {"l": {"at": (-4.0, 34.0, 128.0), "palm": (0.5, -0.85, 0.0), "fingers": (0.85, 0.5, 0.1)}}
MAA_FIGHT_ELBOWS = {"l": (-1.0, -0.3, -0.5), "r": (1.0, -0.4, -0.3)}
MAA_GUARD = {"pelvis": (0.0, 0.0, -5.0), "rot": torso(0.0, 4.0, 0.0), "hands": SHIELD_RAISED,
             "tool": {"grip": (26.0, 18.0, 112.0), "dir": (0.0, 0.3, 0.95)}}
MAA_COCKED = {"pelvis": (0.0, -3.0, -5.0), "rot": merge(torso(25.0, -2.0, 0.0), {"clavicle_r": (6.0, 0.0, -12.0)}),
              "hands": SHIELD_RAISED, "tool": {"grip": (26.0, 2.0, 162.0), "dir": (0.15, -0.55, 0.82)}}
MAA_CUT = {"pelvis": (0.0, 6.0, -9.0), "rot": merge(torso(-20.0, 14.0, 0.0), {"clavicle_r": (-10.0, 0.0, 0.0)}),
           "hands": SHIELD_RAISED, "tool": {"grip": (8.0, 46.0, 110.0), "dir": (-0.55, 0.8, -0.25)}}
MAA_FOLLOW = {"pelvis": (0.0, 5.0, -9.0), "rot": merge(torso(-26.0, 16.0, 0.0), {"clavicle_r": (-10.0, 0.0, 0.0)}),
              "hands": SHIELD_RAISED, "tool": {"grip": (-2.0, 40.0, 100.0), "dir": (-0.8, 0.5, -0.35)}}


def maa_key(t, pose, ease="inout"):
    return dict(pose, t=t, ease=ease, elbows=MAA_FIGHT_ELBOWS)


MAA_ATTACK = fighting("A_ManAtArms_Attack", MAA_STRIKE_TOOL, ("sword", "shield"), [
    maa_key(0.0, MAA_GUARD),
    maa_key(at(MAA_PERIOD, 0.10), MAA_GUARD),
    maa_key(at(MAA_PERIOD, 0.40), MAA_COCKED),                # sword raised over the right shoulder
    maa_key(at(MAA_PERIOD, 0.50), MAA_CUT, "in"),             # THE HIT: blade across the front at the waist
    maa_key(at(MAA_PERIOD, 0.58), MAA_FOLLOW, "out"),         # follow-through to his left
    maa_key(at(MAA_PERIOD, 0.88), MAA_GUARD),
    maa_key(MAA_PERIOD, MAA_GUARD),
], MAA_FIGHT_ELBOWS, 32, MAA_STANCE, [0, 10, 17, 22, 26, 38])

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
