"""Weapons of the soldiers (J7a.3): the Spearman's spear, the Man-at-arms' sword and shield.

Pure Python, same primitives and conventions as peasant_props.py (read its docstring): every piece is skinned 100 % to
ONE bone; light versions (light=True) for the mesh's last LOD, the in-game look (D326). One mesh per type (user's
choice, 2026-10-18): SKM_Spearman = mannequin + spear, SKM_ManAtArms = mannequin + sword + shield; the weapons are
ALWAYS shown (no per-instance choice), so their ids only name them (SOLDIER_MESHES lists each mesh's pieces).

  - spear (id 1): held upright in the right hand; tool frame on ik_hand_gun (origin at the butt, X up the shaft toward
    the point, Y the blade's side); the gestures key the bone (anim_rig "tool", one hand, fixed "edge").
  - sword (id 2): in the right hand, same tool frame (origin at the pommel, X along the blade, Y an edge).
  - shield (id 3): heater shield on the left forearm, skinned to hand_l ("bone" frame of peasant_props): its frame
    follows the left hand, which the gestures place with its palm and fingers (free hand).

PLAYER COLOUR: primitives with "player": True are drawn in PLAYER_PREVIEW here; the mesh builder must write them like
the body (vertex alpha 0: the material then shows the player's colour) - the shield's face, the spear's pennant.
Run alone to print the table:  python Tools/Editor/soldier_props.py
"""
import math
import os
import sys

sys.dont_write_bytecode = True  # no __pycache__ in Tools/Editor
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import anim_rig  # noqa: E402
import peasant_props as pp  # noqa: E402

PLAYER_PREVIEW = (52, 96, 200)  # stands for the player's colour on the previews
WOOD = pp.WOOD_HANDLE
LEATHER = pp.LEATHER
IRON, STEEL = pp.IRON, pp.STEEL
BRASS = (176, 140, 60)

# Spear: 240 cm from the butt to the point (taller than the man: reads at the RTS camera), held at SPEAR_GRIP.
SPEAR_LENGTH = 240.0
SPEAR_GRIP = 95.0          # butt to the right hand's grip (the gestures' "knob_offset")
SPEAR_RADIUS = 1.7
SPEAR_HEAD = 30.0          # length of the blade (leaf), ending at SPEAR_LENGTH
SPEAR_HEAD_WIDTH = 7.0

# Sword: 92 cm, pommel to point; grip 14 cm.
SWORD_LENGTH = 92.0
SWORD_GRIP = 8.0           # pommel to the right hand's grip
SWORD_GUARD = 16.0         # pommel to the crossguard
SWORD_WIDTH = 4.6

# Heater shield, on the left forearm (hand_l): 60 cm tall, 46 wide.
SHIELD_TOP, SHIELD_BOTTOM, SHIELD_HALF_WIDTH = 26.0, -34.0, 23.0
SHIELD_OUT = 5.5           # grip point (in the fist) to the board's back, toward the back of the hand
SHIELD_TOWARD_ELBOW = 9.0  # grip point to the board's centre, toward the elbow


def player(prim):
    prim["player"] = True
    return prim


def make_spear(light=False):
    sides = 4 if light else 6
    x0 = SPEAR_LENGTH - SPEAR_HEAD
    prims = [
        pp.frustum_between((0.0, 0.0, 0.0), (x0 + 2.0, 0.0, 0.0), (SPEAR_RADIUS, SPEAR_RADIUS),
                           (SPEAR_RADIUS * 0.9, SPEAR_RADIUS * 0.9), sides, WOOD, x_hint=pp.FLAT),
        # Iron socket under the head, then the leaf blade, flat across Z, widest at a third.
        pp.frustum_between((x0 - 6.0, 0.0, 0.0), (x0 + 1.0, 0.0, 0.0), (2.1, 2.1), (1.6, 1.6), sides, IRON,
                           x_hint=pp.FLAT),
        pp.prism([(x0, -1.2), (x0 + 10.0, -SPEAR_HEAD_WIDTH / 2), (SPEAR_LENGTH, 0.0),
                  (x0 + 10.0, SPEAR_HEAD_WIDTH / 2), (x0, 1.2)], 1.2, (0.0, 0.0, -0.6), pp.IDENTITY, STEEL),
        # Pennant in the player's colour, below the head, out on the blade's side.
        player(pp.prism([(x0 - 30.0, 0.0), (x0 - 23.0, 24.0), (x0 - 10.0, 0.0)], 0.6, (0.0, 0.0, -0.3), pp.IDENTITY,
                        PLAYER_PREVIEW)),
    ]
    if not light:
        # Iron butt cap.
        prims.append(pp.frustum_between((0.0, 0.0, 0.0), (5.0, 0.0, 0.0), (1.6, 1.6), (2.0, 2.0), 6, IRON,
                                        x_hint=pp.FLAT))
    return {"id": 1, "name": "spear", "label": "lance", "bone": "ik_hand_gun", "frame": "tool", "prims": prims}


def make_sword(light=False):
    sides = 4 if light else 6
    tip = SWORD_LENGTH
    prims = [
        # Leather grip, crossguard across the edges, blade tapering to the point.
        pp.frustum_between((1.5, 0.0, 0.0), (SWORD_GUARD, 0.0, 0.0), (1.5, 1.3), (1.3, 1.2), sides, LEATHER,
                           x_hint=pp.FLAT),
        pp.box((SWORD_GUARD + 0.8, 0.0, 0.0), (1.6, 18.0, 2.0), BRASS),
        pp.prism([(SWORD_GUARD + 1.6, -SWORD_WIDTH / 2), (tip - 12.0, -SWORD_WIDTH / 2 + 0.6), (tip, 0.0),
                  (tip - 12.0, SWORD_WIDTH / 2 - 0.6), (SWORD_GUARD + 1.6, SWORD_WIDTH / 2)], 0.9, (0.0, 0.0, -0.45),
                 pp.IDENTITY, STEEL),
    ]
    if not light:
        prims.append(pp.box((0.8, 0.0, 0.0), (3.2, 3.6, 3.0), BRASS))  # pommel
    return {"id": 2, "name": "sword", "label": "epee", "bone": "ik_hand_gun", "frame": "tool", "prims": prims}


def hand_axes(sk, side="l"):
    """The hand's reference axes as anim_rig.Rig reads them: fingers (wrist to middle knuckle), across (pinky to
    index), palm (the side the fingers curl to), and the grip point (in the closed fingers), all component space."""
    hand = "hand_" + side
    knuckle = sk.pos("middle_01_" + side)
    fingers = anim_rig.sub(knuckle, sk.pos(hand))
    across = anim_rig.sub(sk.pos("index_01_" + side), sk.pos("pinky_01_" + side))
    m1, m2, m3 = (sk.pos("middle_%02d_%s" % (k, side)) for k in (1, 2, 3))
    along = anim_rig.normalize(anim_rig.sub(m2, m1))
    curl = anim_rig.sub(anim_rig.sub(m3, m2), anim_rig.scale(along, anim_rig.dot(along, anim_rig.sub(m3, m2))))
    palm = anim_rig.normalize(anim_rig.cross(fingers, across))
    if anim_rig.dot(palm, curl) < 0.0:
        palm = anim_rig.scale(palm, -1.0)
    grip = anim_rig.add(anim_rig.lerp(sk.pos(hand), knuckle, 0.85), anim_rig.scale(palm, 2.5))
    return anim_rig.normalize(fingers), anim_rig.normalize(across), palm, grip


def shield_local(sk):
    """The shield's frame in hand_l's reference frame: X out of the back of the hand (the shield's face), Z toward the
    index (up when the thumb is up), Y = Z x X; origin at the board's back, in front of the forearm."""
    fingers, across, palm, grip = hand_axes(sk, "l")
    x = anim_rig.scale(palm, -1.0)
    z = anim_rig.normalize(anim_rig.sub(across, anim_rig.scale(x, anim_rig.dot(across, x))))
    y = anim_rig.cross(z, x)
    origin = anim_rig.add(anim_rig.add(grip, anim_rig.scale(x, SHIELD_OUT)), anim_rig.scale(fingers, -SHIELD_TOWARD_ELBOW))
    bt, bq = sk.ref_world[sk.index["hand_l"]]
    rotation = anim_rig.qfrom_matrix((x, y, z))
    return (anim_rig.qrot(anim_rig.qconj(bq), anim_rig.sub(origin, bt)),
            anim_rig.qnormalize(anim_rig.qmul(anim_rig.qconj(bq), rotation)))


def heater_outline(scale_by=1.0, steps=4):
    """Convex heater outline in (up, side) cm, counter-clockwise: flat top, straight sides, curved to the point."""
    top, bottom, half = SHIELD_TOP * scale_by, SHIELD_BOTTOM * scale_by, SHIELD_HALF_WIDTH * scale_by
    right = [(top, half), (top * 0.1, half)]
    # Arc from the side's lower end to the point.
    for k in range(1, steps):
        a = k / steps * math.pi / 2
        right.append((top * 0.1 + (bottom - top * 0.1) * math.sin(a), half * math.cos(a)))
    pts = right + [(bottom, 0.0)] + [(u, -s) for u, s in reversed(right)]
    # Counter-clockwise in the prism's (x, y): x = up, y = -side (see make_shield).
    return [(u, -s) for u, s in pts][::-1]


def make_shield(sk, light=False):
    # The prism extrudes along its local Z: turned so it extrudes along the frame's X (out), its local X up (frame Z).
    q = pp.rotation_z_to((1.0, 0.0, 0.0), (0.0, 0.0, 1.0))
    steps = 2 if light else 4
    prims = [
        # Iron rim: the board's outline, a little larger, behind the face...
        pp.prism(heater_outline(1.0, steps), 1.4, (0.0, 0.0, 0.0), q, IRON),
        # ...and the face in the player's colour, in front of it.
        player(pp.prism(heater_outline(0.94, steps), 0.8, (1.4, 0.0, 0.0), q, PLAYER_PREVIEW)),
    ]
    if not light:
        prims.append(pp.box((2.6, 0.0, 2.0), (1.6, 7.0, 7.0), IRON))  # boss
    return {"id": 3, "name": "shield", "label": "bouclier", "bone": "hand_l", "frame": "bone",
            "local": shield_local(sk), "prims": prims}


def build_props(sk, light=False):
    props = [make_spear(light), make_sword(light), make_shield(sk, light)]
    return {p["name"]: p for p in props}


# The soldiers' meshes (J7a.3) and their pieces, always shown.
SOLDIER_MESHES = {"SKM_Spearman": ("spear",), "SKM_ManAtArms": ("sword", "shield")}


def main():
    sk = anim_rig.Skeleton.load(os.path.join(os.path.dirname(os.path.abspath(__file__)), "Data",
                                             "UE4MannequinSkeleton.json"))
    for light in (False, True):
        print("Pieces%s :" % (" legeres" if light else ""))
        for prop in sorted(build_props(sk, light).values(), key=lambda p: p["id"]):
            size = pp.extents(prop)
            tris = pp.prop_triangles(prop)
            verts = len(pp.part_buffers({"key": prop["name"], "mask": 0, "owner": prop, "prims": prop["prims"]},
                                        sk)["vertices"])
            print("  %-3d %-7s os %-12s %-26s %4d triangles %4d sommets" % (
                prop["id"], prop["name"], prop["bone"], "%.0f x %.0f x %.0f cm" % size, len(tris), verts))


if __name__ == "__main__":
    main()
