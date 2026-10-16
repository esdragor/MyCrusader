"""Gestures of the Peasant, written key by key (pure Python: read by make_peasant_animations.py and preview_animation.py).

Format: see anim_rig.py; "props" names the props of peasant_props.py the preview draws on their bones (the mesh
will show the same, J5c.2). Body terms everywhere: (right, forward, up) in cm for positions and directions,
(twist, bend, side) in degrees for rotations (twist toward his right, bend forward, lean toward his right).
The first key is at t = 0 and the last key repeats the first (clean loop).
"""


def torso(twist, bend, side=0.0, look=0.7, nod=0.0, pelvis=True):
    """Spine rotations for a torso turned by `twist`, leaning by `bend` and `side`, spread from the pelvis up; the
    neck and head turn back by `look` of the twist so the eyes stay on the work, and bow by `nod` degrees (looking
    down). `pelvis=False` leaves the pelvis out (a gesture whose pelvis comes from a source walk)."""
    spread = (("pelvis", 0.25, 0.15), ("spine_01", 0.25, 0.3), ("spine_02", 0.25, 0.3), ("spine_03", 0.25, 0.25))
    rot = {bone: (twist * k, bend * b, side * k) for bone, k, b in spread if pelvis or bone != "pelvis"}
    rot["neck_01"] = (-twist * look * 0.5, -bend * 0.2 + nod * 0.5, -side * 0.3)
    rot["head"] = (-twist * look * 0.5, -bend * 0.2 + nod * 0.5, -side * 0.3)
    return rot


def merge(*dicts):
    out = {}
    for d in dicts:
        out.update(d)
    return out


# Clavicles (degrees in body terms). Measured effect on the shoulder joints: a "side" rotation toward his right lowers
# the right shoulder, a "twist" toward his right takes the right shoulder back (and the reverse on the left side).
# The axe chop was made and validated with these two (a few cm each: shoulders slightly down, slightly back); their
# names say what they do, their values stay.
SHOULDERS_DOWN = {"clavicle_r": (0.0, 0.0, 12.0), "clavicle_l": (0.0, 0.0, -8.0)}
SHOULDERS_BACK = {"clavicle_r": (8.0, 0.0, 0.0), "clavicle_l": (-14.0, 0.0, 0.0)}
# For the later gestures: both shoulders up (arms overhead) or forward (reaching low in front).
SHOULDERS_UP = {"clavicle_r": (0.0, 0.0, -14.0), "clavicle_l": (0.0, 0.0, 14.0)}
SHOULDERS_FORWARD = {"clavicle_r": (-12.0, 0.0, 0.0), "clavicle_l": (12.0, 0.0, 0.0)}

# Two-handed axe chop on a trunk in front of him, about 1.1 m away. Seen from far above: wide wind-up over the right
# shoulder, fast diagonal stroke down to waist height in front, short rebound, slower lift back to the wind-up.
WIND_UP = {
    "t": 0.0,
    "pelvis": (2.0, -3.0, -4.0),
    "rot": merge(torso(45.0, -10.0, -4.0), SHOULDERS_DOWN),
    "axe": {"grip": (13.0, -2.0, 157.0), "dir": (0.25, -0.65, 0.72), "spacing": 24.0},
    "elbows": {"l": (-0.3, 0.6, -1.0), "r": (1.0, -0.2, -0.4)},
}
CHOP_WOOD = {
    "name": "A_Peasant_ChopWood",
    "fps": 30,
    "props": ("axe",),
    "stance": {"l": (-6.0, 8.0, 0.0), "r": (6.0, -6.0, 0.0)},
    "elbows": {"l": (-0.4, 0.1, -1.0), "r": (0.5, -0.2, -1.0)},
    "fist": (55.0, 70.0, 45.0),
    "keys": [
        WIND_UP,
        # Downswing: accelerates out of the wind-up, the top hand sliding down the handle...
        {"t": 0.10, "ease": "in",
         "pelvis": (0.0, 0.0, -5.0),
         "rot": merge(torso(15.0, 2.0, -2.0), SHOULDERS_BACK),
         "axe": {"grip": (20.0, 18.0, 138.0), "dir": (0.55, 0.35, 0.75), "spacing": 18.0},
         "elbows": {"l": (-0.4, 0.2, -1.0), "r": (0.8, -0.2, -0.8)}},
        # ...and stops dead on the trunk, waist high, a little left of centre (the hit).
        {"t": 0.20, "ease": "linear",
         "pelvis": (-1.0, 4.0, -7.0),
         "rot": merge(torso(-14.0, 16.0, 3.0), SHOULDERS_BACK),
         "axe": {"grip": (0.0, 38.0, 100.0), "dir": (-0.06, 0.97, 0.22), "spacing": 10.0},
         "elbows": {"l": (-0.6, 0.0, -1.0), "r": (0.4, -0.2, -1.0)}},
        # Short rebound off the wood.
        {"t": 0.32, "ease": "out",
         "pelvis": (-1.0, 3.0, -6.0),
         "rot": merge(torso(-10.0, 12.0, 2.0), SHOULDERS_BACK),
         "axe": {"grip": (1.0, 35.0, 106.0), "dir": (0.0, 0.85, 0.5), "spacing": 12.0}},
        # Lift: the axe comes up in front of the chest, the top hand back up the handle...
        {"t": 0.62, "ease": "inout",
         "pelvis": (1.0, 0.0, -5.0),
         "rot": merge(torso(15.0, 2.0, -1.0)),
         "axe": {"grip": (10.0, 20.0, 132.0), "dir": (0.0, 0.25, 0.97), "spacing": 22.0},
         "elbows": {"l": (-0.4, 0.5, -1.0), "r": (0.8, -0.1, -0.8)}},
        # ...over the right shoulder, nearly cocked...
        {"t": 0.95, "ease": "inout",
         "pelvis": (2.0, -2.0, -4.0),
         "rot": merge(torso(34.0, -7.0, -3.0), SHOULDERS_DOWN),
         "axe": {"grip": (12.0, 0.0, 155.0), "dir": (0.2, -0.5, 0.84), "spacing": 24.0},
         "elbows": {"l": (-0.3, 0.6, -1.0), "r": (1.0, -0.2, -0.4)}},
        # ...and the last loading inch before the stroke (= the first key).
        dict(WIND_UP, t=1.20, ease="inout"),
    ],
}

# Pickaxe on a rock (stone and gold), one sequence: two hands on an 80 cm handle with a two-sided head. Slower, more
# vertical rhythm than the axe: wind-up straight overhead (head behind the head), heavy downward stroke onto a low rock
# 70-80 cm in front at shin height, deep forward lean and knee bend at the hit, short rebound, slow lift.
PICK_TOP = {
    "t": 0.0,
    "pelvis": (0.0, -2.0, -3.0),
    "rot": merge(torso(8.0, -12.0, 0.0), SHOULDERS_UP),
    "tool": {"grip": (6.0, 2.0, 178.0), "dir": (0.05, -0.75, 0.66), "spacing": 26.0},
    "elbows": {"l": (-1.0, 0.4, -0.2), "r": (1.0, 0.4, -0.2)},
}
MINE_PICK = {
    "name": "A_Peasant_MinePick",
    "fps": 30,
    "tool": {"kind": "pick", "handle_length": 80.0, "knob_offset": 6.0},
    "props": ("pickaxe",),
    "scene": {"kind": "rock"},
    "stance": {"l": (-9.0, 7.0, 0.0), "r": (9.0, -5.0, 0.0)},
    "elbows": {"l": (-0.6, 0.0, -1.0), "r": (0.6, 0.0, -1.0)},
    "fist": (55.0, 70.0, 45.0),
    "keys": [
        PICK_TOP,
        # Downswing: over the head and down in front, accelerating...
        {"t": 4 / 30, "ease": "in",
         "pelvis": (0.0, 0.0, -7.0),
         "rot": merge(torso(4.0, 8.0, 0.0, nod=5.0), SHOULDERS_UP),
         "tool": {"grip": (4.0, 32.0, 160.0), "dir": (0.0, 0.45, 0.89), "spacing": 20.0},
         "elbows": {"l": (-0.8, 0.3, -0.6), "r": (0.8, 0.3, -0.6)}},
        # ...the hit: the head bites the rock at shin height, body folded over it.
        {"t": 8 / 30, "ease": "linear",
         "pelvis": (0.0, -7.0, -15.0),
         "rot": merge(torso(0.0, 46.0, 0.0, nod=10.0), SHOULDERS_FORWARD),
         "tool": {"grip": (1.0, 28.0, 84.0), "dir": (0.0, 0.78, -0.62), "spacing": 6.0},
         "elbows": {"l": (-0.7, -0.2, -1.0), "r": (0.7, -0.2, -1.0)}},
        # Rebound: the pick jumps back a little.
        {"t": 12 / 30, "ease": "out",
         "pelvis": (0.0, -6.0, -13.0),
         "rot": merge(torso(0.0, 40.0, 0.0, nod=8.0), SHOULDERS_FORWARD),
         "tool": {"grip": (1.0, 27.0, 92.0), "dir": (0.0, 0.92, -0.38), "spacing": 9.0}},
        # Slow lift: straightening up, the pick rising in front of the face...
        {"t": 24 / 30, "ease": "inout",
         "pelvis": (0.0, -1.0, -6.0),
         "rot": merge(torso(4.0, 6.0, 0.0)),
         "tool": {"grip": (4.0, 22.0, 140.0), "dir": (0.0, 0.2, 0.98), "spacing": 22.0},
         "elbows": {"l": (-0.9, 0.3, -0.6), "r": (0.9, 0.3, -0.6)}},
        # ...then overhead, nearly cocked...
        {"t": 34 / 30, "ease": "inout",
         "pelvis": (0.0, -2.0, -3.0),
         "rot": merge(torso(7.0, -9.0, 0.0), SHOULDERS_UP),
         "tool": {"grip": (6.0, 4.0, 175.0), "dir": (0.05, -0.6, 0.8), "spacing": 26.0},
         "elbows": {"l": (-1.0, 0.4, -0.2), "r": (1.0, 0.4, -0.2)}},
        # ...and the last inch back before the stroke (= the first key).
        dict(PICK_TOP, t=40 / 30, ease="inout"),
    ],
}

# Picking berries: bent over a bush in front (50 to 110 cm high, about 60 cm away), hands taking turns: one reaches
# into the bush, picks, and brings the berries to the pouch at his left hip while the other holds a branch (right)
# or rests on the pouch (left). Calm loop, feet planted.
BUSH_R = (8.0, 42.0, 88.0)      # right hand in the bush
BUSH_R2 = (15.0, 40.0, 78.0)    # right hand, a second spot
BUSH_L = (-8.0, 42.0, 90.0)     # left hand in the bush
BUSH_L2 = (-14.0, 40.0, 80.0)   # left hand, a second spot
HOLD_R = (17.0, 36.0, 100.0)    # right hand holding a branch aside
DROP_R = (-14.0, 14.0, 98.0)    # right hand over the pouch
POUCH_L = (-24.0, 8.0, 93.0)    # left hand at the pouch
PICK_PALM = (0.0, 0.5, -0.85)   # palm toward the berries (forward and down)
DROP_PALM = (0.0, 0.0, -1.0)    # palm down over the pouch
HOLD_PALM = (-0.5, 0.85, 0.0)   # palm on the branch
POUCH_PALM = (0.85, 0.0, -0.5)  # left palm against the pouch


def hands(right, right_palm, left, left_palm):
    return {"r": {"at": right, "palm": right_palm}, "l": {"at": left, "palm": left_palm}}


GATHER_BERRIES = {
    "name": "A_Peasant_GatherBerries",
    "fps": 30,
    "scene": {"kind": "bush", "centre": (0.0, 58.0, 78.0), "radii": (34.0, 24.0, 30.0), "pouch": (-27.0, 6.0, 88.0)},
    "stance": {"l": (-7.0, 4.0, 0.0), "r": (7.0, 0.0, 0.0)},
    "elbows": {"l": (-0.8, -0.3, -1.0), "r": (0.8, -0.3, -1.0)},
    "fist": (30.0, 40.0, 30.0),
    "keys": [
        # The left hand drops its berries in the pouch while the right one reaches into the bush.
        {"t": 0.0, "pelvis": (0.0, -8.0, -13.0), "rot": merge(torso(-4.0, 40.0, 0.0, nod=12.0), SHOULDERS_FORWARD),
         "hands": hands(HOLD_R, HOLD_PALM, POUCH_L, POUCH_PALM)},
        {"t": 0.30, "pelvis": (0.0, -8.0, -14.0), "rot": merge(torso(-6.0, 44.0, 0.0, nod=12.0), SHOULDERS_FORWARD),
         "hands": hands(BUSH_R, PICK_PALM, POUCH_L, POUCH_PALM)},
        {"t": 0.60, "pelvis": (0.0, -8.0, -14.0), "rot": merge(torso(-4.0, 45.0, 0.0, nod=12.0), SHOULDERS_FORWARD),
         "hands": hands(BUSH_R2, PICK_PALM, POUCH_L, POUCH_PALM)},
        # The right hand brings its berries to the pouch, across the body.
        {"t": 0.90, "pelvis": (0.0, -8.0, -13.0), "rot": merge(torso(-10.0, 38.0, 0.0, nod=10.0), SHOULDERS_FORWARD),
         "hands": hands(DROP_R, DROP_PALM, POUCH_L, POUCH_PALM)},
        # Then it holds a branch aside while the left hand reaches into the bush...
        {"t": 1.20, "pelvis": (0.0, -8.0, -14.0), "rot": merge(torso(4.0, 44.0, 0.0, nod=12.0), SHOULDERS_FORWARD),
         "hands": hands(HOLD_R, HOLD_PALM, BUSH_L, PICK_PALM)},
        {"t": 1.50, "pelvis": (0.0, -8.0, -14.0), "rot": merge(torso(6.0, 45.0, 0.0, nod=12.0), SHOULDERS_FORWARD),
         "hands": hands(HOLD_R, HOLD_PALM, BUSH_L2, PICK_PALM)},
        # ...and brings its berries back to the pouch (= the first key).
        {"t": 1.80, "pelvis": (0.0, -8.0, -13.0), "rot": merge(torso(-4.0, 40.0, 0.0, nod=12.0), SHOULDERS_FORWARD),
         "hands": hands(HOLD_R, HOLD_PALM, POUCH_L, POUCH_PALM)},
    ],
}

# Loaded walks, one per kind of load, all on the same source walk: the legs and pelvis are exactly those of the walk
# (same feet, same cadence, same length, so one play rate for the three); only the upper body changes. With a base,
# the key times are stretched onto the source's length. In the source, the pelvis is lowest just after each foot
# lands (about 0.37 and 0.9 of the loop) and highest at about 0.13 and 0.63.
UNARMED_WALK = "/Game/CloseCombat/Character/UE4Mannequin/Animations/Swordsman/Movement/Unarmed/A_SW_Unarmed_Walk"

# Food: a basket of berries (peasant_props.py) held against the belly at about 110 cm, torso leaning back a little and
# steady on the swinging pelvis (validated on the preview by the user, 2026-10-16: keep as is).
CARRY_HANDS = hands((15.0, 26.0, 108.0), (-0.75, 0.0, 0.65), (-15.0, 26.0, 108.0), (0.75, 0.0, 0.65))
CARRY_FOOD = {
    "name": "A_Peasant_CarryFood",
    "base": UNARMED_WALK,
    "hands_frame": "chest",
    "steady_chest": 0.6,
    "props": ("basket_food",),
    "elbows": {"l": (-0.6, -0.4, -1.0), "r": (0.6, -0.4, -1.0)},
    "fist": (45.0, 55.0, 35.0),
    "keys": [
        {"t": 0.0, "rot": merge(torso(0.0, -8.0, 0.0, nod=4.0, pelvis=False), SHOULDERS_FORWARD),
         "hands": CARRY_HANDS},
        {"t": 1.0, "rot": merge(torso(0.0, -8.0, 0.0, nod=4.0, pelvis=False), SHOULDERS_FORWARD),
         "hands": CARRY_HANDS},
    ],
}

# Wood: a bundle of four logs (95 cm) on the right shoulder, front end a little up, steadied by the right hand raised
# beside the face; the left arm swings with the walk (forward when the left foot is back); the body leans a little to
# the left against the load.
# J5c.2: the right hand follows the bundle, raised to rest on the shoulder (was at (12, 14, 162)).
WOOD_HANDS = {"r": {"at": (17.0, 14.0, 167.5), "palm": (-0.3, 0.0, 0.95)},
              "l": {"at": (-21.0, -4.0, 90.0), "palm": (1.0, 0.0, 0.0), "fingers": (0.0, 0.15, -1.0), "swing": 0.35}}
WOOD_ROT = merge(torso(0.0, -3.0, -10.0, look=0.0, pelvis=False), {"clavicle_r": (0.0, 0.0, -10.0)})
CARRY_WOOD = {
    "name": "A_Peasant_CarryWood",
    "base": UNARMED_WALK,
    "hands_frame": "chest",
    "steady_chest": 0.5,
    "props": ("logs",),
    "elbows": {"l": (-0.2, -1.0, -0.4), "r": (0.6, 0.4, -1.0)},
    "fist": (40.0, 50.0, 35.0),
    "keys": [
        {"t": 0.0, "rot": WOOD_ROT, "hands": WOOD_HANDS},
        {"t": 1.0, "rot": WOOD_ROT, "hands": WOOD_HANDS},
    ],
}

# Stone and gold (one sequence): a hotte (wicker back-basket, peasant_props.py) on the back, both hands holding its
# straps in front of the shoulders (J5c.2; was the neck of a sack at the right shoulder), torso bent forward about 20
# degrees more than standing (the spine keys say more: the walk and the steady chest take part of it back), head up to
# look ahead; the torso dips a little more each time a foot lands (weight). Legs, lean and bob unchanged.
# The hands' grip points sit just in front of the straps (STRAP_R of peasant_props.py, at about 140 cm), fists closed
# on them, palms toward the chest, elbows out.
STRAP_HANDS = {"r": {"at": (13.5, 15.0, 140.0), "palm": (-0.25, -1.0, 0.1), "fingers": (-0.15, 0.1, -1.0)},
               "l": {"at": (-13.5, 15.0, 140.0), "palm": (0.25, -1.0, 0.1), "fingers": (0.15, 0.1, -1.0)}}
STRAP_ELBOWS = {"l": (-1.0, -0.3, -0.7), "r": (1.0, -0.3, -0.7)}
STONE_HANDS = STRAP_HANDS


def stone_key(t, bend):
    return {"t": t, "rot": merge(torso(0.0, bend, 0.0, nod=-6.0, pelvis=False), SHOULDERS_FORWARD),
            "hands": STONE_HANDS}


CARRY_STONE = {
    "name": "A_Peasant_CarryStone",
    "base": UNARMED_WALK,
    "hands_frame": "chest",
    "steady_chest": 0.4,
    "props": ("hotte_stone",),
    "elbows": STRAP_ELBOWS,
    "fist": (50.0, 60.0, 40.0),
    "keys": [stone_key(0.0, 44.0), stone_key(0.13, 41.0), stone_key(0.37, 48.0), stone_key(0.63, 41.0),
             stone_key(0.9, 48.0), stone_key(1.0, 44.0)],
}

# Loaded idles (J5c.2): a carrier standing still keeps his load held. Pelvis and legs copied from the idle every type
# plays (A_SW_Idle: 11.333 s, in place, a fencing stance with the pelvis turned about 50 degrees to his right); the
# pelvis' turn is taken back along the spine ("steady_spine") so the chest and the load face forward like in the
# loaded walks; calm breathing (three breaths over the loop: chest up and a little back, shoulders up). Same arms
# as the loaded walk of the same load (no swing). Gold uses the stone idle.
IDLE = "/Game/CloseCombat/Character/UE4Mannequin/Animations/Swordsman/Movement/A_SW_Idle"
IDLE_LENGTH = 34 / 3.0  # the idle's length (s): the key times below are in its seconds
BREATHS = 3


def breathing(rot, depth=1.0):
    """Keys of a calm breathing loop on top of a static upper body `rot`: chest lifted and opened at each inhale."""
    period = IDLE_LENGTH / BREATHS
    keys = []
    for k in range(BREATHS):
        start = k * period
        inhale = {bone: tuple(a + c for a, c in zip(rot.get(bone, (0.0, 0.0, 0.0)), delta)) for bone, delta in (
            ("spine_02", (0.0, -1.0 * depth, 0.0)), ("spine_03", (0.0, -1.5 * depth, 0.0)),
            ("neck_01", (0.0, 1.2 * depth, 0.0)), ("head", (0.0, 1.0 * depth, 0.0)),
            ("clavicle_r", (0.0, 0.0, -2.0 * depth)), ("clavicle_l", (0.0, 0.0, 2.0 * depth)))}
        keys.append({"t": start, "ease": "inout", "rot": rot})
        keys.append({"t": start + period * 0.42, "ease": "inout", "rot": merge(rot, inhale)})
    keys.append({"t": IDLE_LENGTH, "ease": "inout", "rot": rot})
    return keys


def loaded_idle(name, hands_pose, rot, elbows, fist, props, depth=1.0):
    keys = breathing(rot, depth)
    for key in keys:
        key["hands"] = hands_pose
    return {"name": name, "base": IDLE, "hands_frame": "chest", "steady_spine": 1.0, "props": props,
            "elbows": elbows, "fist": fist, "keys": keys}


IDLE_CARRY_FOOD = loaded_idle(
    "A_Peasant_IdleCarryFood", CARRY_HANDS,
    merge(torso(0.0, -8.0, 0.0, nod=4.0, pelvis=False), SHOULDERS_FORWARD),
    CARRY_FOOD["elbows"], CARRY_FOOD["fist"], ("basket_food",))
# Wood: the right hand steadies the logs on the shoulder as in the walk, the left arm hangs relaxed.
IDLE_WOOD_HANDS = {"r": WOOD_HANDS["r"],
                   "l": {"at": (-22.0, -2.0, 92.0), "palm": (1.0, 0.0, 0.0), "fingers": (0.0, 0.1, -1.0)}}
IDLE_CARRY_WOOD = loaded_idle(
    "A_Peasant_IdleCarryWood", IDLE_WOOD_HANDS, WOOD_ROT, {"l": (-0.2, -1.0, -0.4), "r": (0.6, 0.4, -1.0)},
    CARRY_WOOD["fist"], ("logs",))
# Stone and gold: hands on the hotte's straps, slight forward lean under the weight, head up.
IDLE_CARRY_STONE = loaded_idle(
    "A_Peasant_IdleCarryStone", STRAP_HANDS,
    merge(torso(0.0, 24.0, 0.0, nod=-8.0, pelvis=False), SHOULDERS_FORWARD),
    STRAP_ELBOWS, CARRY_STONE["fist"], ("hotte_stone",))

# Baked in this order; new gestures at the end (make_skinned_mannequin.py lists the sequences by index on its own).
GESTURES = {"chop_wood": CHOP_WOOD, "mine_pick": MINE_PICK, "gather_berries": GATHER_BERRIES,
            "carry_food": CARRY_FOOD, "carry_wood": CARRY_WOOD, "carry_stone": CARRY_STONE,
            "idle_carry_food": IDLE_CARRY_FOOD, "idle_carry_wood": IDLE_CARRY_WOOD,
            "idle_carry_stone": IDLE_CARRY_STONE}
