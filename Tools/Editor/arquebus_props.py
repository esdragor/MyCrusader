"""Pieces of the Arquebusier (production: no PIL, importable by the Unreal commandlets and by soldier_props).

The gun in the crossbow's tool frame (soldier_props, AR1): X from the muzzle side toward the butt, Y the top of the
stock, Z across; the hands stay where the crossbow gestures put them (left at CROSSBOW_LEFT_GRIP, right 45 cm behind).
Pieces of the mesh SKM_Arquebusier (soldier_props.SOLDIER_MESHES), ids 8 / 6 / 5 / 4 (one byte of mask per mesh):
  - arquebus_gun (8): the gun without its ramrod, on ik_hand_gun (tool frame);
  - arquebus_bandolier (6): the bandolier of charges ("twelve apostles") and the powder flask, on spine_03;
  - ramrod_stowed (5): the ramrod under the forestock, on ik_hand_r (a child of ik_hand_gun never keyed: it follows the
    gun; the gestures hide it with "bolt_hidden" while the rod is drawn, like the Crossbowman's loaded bolt);
  - ramrod_drawn (4): the same rod in the right hand, on ik_hand_l (a bone never used otherwise): in the attack its
    track is the right hand's frame (arquebus_poses, "extra_bones"), hidden ("rod_hidden", scale HIDDEN_SCALE) when the
    rod is stowed; its frame (origin at the grip, X toward the rod's head) is the bone's own frame.
Also the smoke of the shot (preview only, not in the mesh).
"""
import math
import os
import sys

sys.dont_write_bytecode = True  # no __pycache__ in Tools/Editor
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import anim_rig  # noqa: E402
import peasant_props as pp  # noqa: E402
import soldier_props as sp  # noqa: E402

ARQ, MUSKET = 110.0, 150.0
BUTT_X = 95.0          # same butt as the crossbow's tiller (the aim keeps it at the shoulder)
SMOKE = (206, 204, 198)
FLASH = (250, 196, 70)
MATCH = (230, 110, 40)  # the lit match cord's end


def make_gun(name, length, drop, light=False, ramrod=True):
    """A matchlock in the crossbow's tool frame: `length` muzzle to butt, `drop` how far the butt falls (cm).
    `ramrod` False leaves the stowed ramrod out (arquebus_poses draws it apart, so the gestures can draw it)."""
    sides = 4 if light else 8
    muzzle = BUTT_X - length
    prims = [
        # Barrel on top, forestock under it, ramrod under the forestock.
        pp.frustum_between((muzzle, 2.6, 0.0), (62.0, 2.6, 0.0), (1.5, 1.5), (1.9, 1.9), sides, sp.IRON,
                           x_hint=pp.FLAT),
        pp.prism([(muzzle + 6.0, -1.6), (62.0, -2.2), (62.0, 1.6), (muzzle + 6.0, 1.4)], 3.6, (0.0, 0.0, -1.8),
                 pp.IDENTITY, sp.WOOD),
        # Butt: from the lock back, falling by `drop`.
        pp.prism([(60.0, -2.4), (BUTT_X, -2.4 - drop), (BUTT_X, 1.0), (60.0, 3.2)], 4.4, (0.0, 0.0, -2.2),
                 pp.IDENTITY, pp.WOOD_DARK),
        # Lock plate on his right side, the serpentine holding the match, the trigger bar under the stock.
        pp.box((63.0, 1.0, 2.5), (9.0, 3.0, 0.8), sp.IRON),
        pp.box((60.0, 4.6, 2.5), (1.2, 4.4, 1.2), sp.BRASS),
        pp.box((70.0, -4.2, 0.0), (7.0, 1.0, 1.2), sp.IRON),
    ]
    if ramrod:
        prims.append(pp.frustum_between((muzzle + 2.0, -2.4, 0.0), (56.0, -2.4, 0.0), (0.45, 0.45), (0.45, 0.45), 3,
                                        pp.WOOD_DARK, x_hint=pp.FLAT))
    if not light:
        prims.append(pp.box((60.0, 7.2, 2.5), (1.4, 1.4, 1.4), MATCH))
        prims.append(pp.frustum_between((muzzle - 0.5, 2.6, 0.0), (muzzle + 2.5, 2.6, 0.0), (2.2, 2.2), (2.2, 2.2),
                                        sides, sp.IRON))  # muzzle ring
    return {"id": 8, "name": name, "label": name, "bone": "ik_hand_gun", "frame": "tool", "prims": prims}


def blob(centre, radius, color, sides=8):
    """A rough ball: two cones base to base along X."""
    a = anim_rig.add(centre, (radius, 0.0, 0.0))
    b = anim_rig.add(centre, (-radius, 0.0, 0.0))
    r = (radius, radius)
    small = (radius * 0.35, radius * 0.35)
    return [pp.frustum_between(a, centre, small, r, sides, color, caps=("bottom",), x_hint=pp.FLAT),
            pp.frustum_between(centre, b, r, small, sides, color, caps=("top",), x_hint=pp.FLAT)]


def make_smoke(name, length, size):
    """The shot's flash and powder smoke in front of the muzzle (tool frame), `size` 1 = small, 2 = big."""
    muzzle = BUTT_X - length
    prims = blob((muzzle - 4.0, 2.6, 0.0), 3.5, FLASH)
    for k, (dx, dy, dz, r) in enumerate(((-14, 3, 0, 7), (-28, 6, 3, 10), (-44, 10, -2, 12), (-60, 16, 4, 14))):
        if k >= 2 + size:
            break
        prims += blob((muzzle + dx * (0.5 + 0.25 * size), 2.6 + dy, dz), r * (0.6 + 0.3 * size), SMOKE)
    return {"id": 7, "name": name, "label": name, "bone": "ik_hand_gun", "frame": "tool", "prims": prims}


# Bandolier ("twelve apostles"): body terms (forward, right, up), left shoulder over the chest to the right hip.
STRAP = [(3.0, -17.0, 153.0), (12.0, -10.0, 143.0), (15.8, -2.0, 131.0), (16.0, 7.0, 119.0), (14.5, 15.0, 107.0),
         (9.0, 21.0, 98.0)]


def make_bandolier(light=False):
    prims = []
    for a, b in zip(STRAP, STRAP[1:]):
        prims.append(pp.frustum_between(a, b, (0.5, 2.2), (0.5, 2.2), 4, sp.LEATHER, caps=(), x_hint=(1.0, 0.0, 0.0)))
    for k in range(1, len(STRAP) - 1):
        top = anim_rig.add(STRAP[k], (1.4, 0.0, -1.0))
        bottom = anim_rig.add(top, (0.4, 0.0, -7.0))
        prims.append(pp.frustum_between(bottom, top, (1.3, 1.3), (1.3, 1.3), 6, pp.WOOD_DARK))
        prims.append(pp.frustum_between(top, anim_rig.add(top, (0.0, 0.0, 1.2)), (1.4, 1.4), (0.8, 0.8), 6, sp.BRASS))
    # Powder flask at the right hip, hanging from the strap's end.
    flask = (11.0, 22.0, 88.0)
    prims.append(pp.frustum_between(flask, anim_rig.add(flask, (0.0, 0.0, 9.0)), (4.0, 2.6), (1.6, 1.4), 8,
                                    (120, 80, 40)))
    return sp.body_piece(6, "bandolier", "bandouliere", "spine_03", prims)


# ---------------------------------------------------------------------------------------------------------------------
# The ramrod (cm, gun frame: x from the muzzle side toward the butt)
MUZZLE_X = BUTT_X - 110.0              # -15: the arquebus' muzzle
ROD_LEN = 64.0                         # stowed from the muzzle's side
ROD_TAIL = 7.0                         # the hand's grip to the rod's tail end (the end that sticks out upward)
ROD_PIPE_Y = -2.4                      # stowed under the forestock
BORE_Y = 2.6                           # the bore, on top of the stock
ROD_STOW_X = MUZZLE_X + 2.0            # the tail end when stowed
GRAB_X = ROD_STOW_X + ROD_TAIL         # the hand's grip when it holds the stowed rod's tail
ROD_RADIUS = 0.9


def hand_frame_axes(sk):
    """The right hand's rig frame as anim_rig.Rig reads it: (palm, fingers made square to the palm), unit vectors."""
    fingers0, across0, palm0, grip0 = sp.hand_axes(sk, "r")
    f0 = anim_rig.normalize(anim_rig.sub(fingers0, anim_rig.scale(palm0, anim_rig.dot(fingers0, palm0))))
    return palm0, f0, grip0


def rod_local(sk):
    """The rod's frame in hand_r's reference frame: origin at the grip point, X along cross(palm, fingers) (exactly
    square to the rig's palm and fingers, so a free hand's palm and fingers give the rod's axis exactly)."""
    palm0, f0, grip0 = hand_frame_axes(sk)
    bt, bq = sk.ref_world[sk.index["hand_r"]]
    to_local = lambda v: anim_rig.qrot(anim_rig.qconj(bq), v)  # noqa: E731
    q = pp.rotation_xz(to_local(anim_rig.cross(palm0, f0)), to_local(palm0))
    return (to_local(anim_rig.sub(grip0, bt)), anim_rig.qnormalize(q))


def make_ramrod_prims(origin_x, light=False):
    """A ramrod along +X from its tail (origin_x) to its head: the rod, a brass cup at the head, a knob at the tail."""
    sides = 4 if light else 6
    x0, x1 = origin_x, origin_x + ROD_LEN
    r = ROD_RADIUS
    return [
        pp.frustum_between((x0, 0.0, 0.0), (x1 - 4.0, 0.0, 0.0), (r, r), (r, r), sides, pp.WOOD_DARK, x_hint=pp.FLAT),
        pp.frustum_between((x1 - 4.0, 0.0, 0.0), (x1, 0.0, 0.0), (r * 1.5, r * 1.5), (r * 1.2, r * 1.2), sides, sp.BRASS,
                           x_hint=pp.FLAT),
        pp.frustum_between((x0 - 0.6, 0.0, 0.0), (x0 + 2.0, 0.0, 0.0), (r * 1.6, r * 1.6), (r * 1.1, r * 1.1), sides,
                           sp.BRASS, x_hint=pp.FLAT),
    ]


def make_ramrod_stowed(sk, light=False):
    """Under the forestock, bound to ik_hand_r (like the crossbow's bolt): origin at the grip of the hand that takes it
    by the tail, so the drawn rod (same frame) takes over without a jump."""
    prims = make_ramrod_prims(-ROD_TAIL, light)
    for prim in prims:  # the prop frame: gun frame shifted to the grip's place, so shift the pieces back
        prim["t"] = anim_rig.add(prim["t"], (GRAB_X, ROD_PIPE_Y, 0.0))
    return {"id": 5, "name": "ramrod_stowed", "label": "baguette rangee", "bone": "ik_hand_r", "frame": "bone",
            "local": sp.bolt_local(sk), "prims": prims}


def make_ramrod_drawn(sk, light=False):
    return {"id": 4, "name": "ramrod_drawn", "label": "baguette en main", "bone": "ik_hand_l", "frame": "bone",
            "local": ((0.0, 0.0, 0.0), pp.IDENTITY), "prims": make_ramrod_prims(-ROD_TAIL, light)}


def build_props(sk, light=False):
    """The mesh's pieces, by name (soldier_props.build_props adds them)."""
    gun = make_gun("arquebus_gun", ARQ, 6.0, light, ramrod=False)
    gun["name"] = "arquebus_gun"
    band = make_bandolier(light)
    band["name"], band["label"] = "arquebus_bandolier", "bandouliere"
    pieces = [gun, band, make_ramrod_stowed(sk, light), make_ramrod_drawn(sk, light)]
    return {p["name"]: p for p in pieces}
