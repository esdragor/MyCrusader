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
