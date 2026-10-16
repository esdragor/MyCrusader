"""Props of the Peasant (J5c.2): tools and loads, each made of a few simple primitives skinned 100 % to ONE bone.

Pure Python (no unreal import, no numpy). Shared by preview_animation.py (each prop drawn on its bone, frame by frame,
so the contact sheets show what the mesh will show) and by the later mesh builder of SKM_Peasant (J5c.2c), which must
take its triangles from here: bind_mesh() gives every vertex in component space in the reference pose, i.e. exactly
where the skinned mesh must have it, with one colour per triangle.

Units and frames:
  - Centimetres, degrees; colours are sRGB (0-255).
  - Body terms as in peasant_poses.py: (right, forward, up) in cm, in the reference pose (component space).
  - Every prop has a PROP FRAME, right-handed, in which its primitives are written:
      * "body" props (loads): origin at the prop's "anchor" (body terms, reference pose); axes X = forward,
        Y = the character's right, Z = up (Unreal's actor convention). Carried rigidly by "bone".
      * "tool" props (axe, pickaxe): origin at the knob (end of the handle), X along the handle toward the head,
        Y toward the striking side (the axe's edge, the pick's point), Z = X x Y. The bone is ik_hand_gun, whose track
        in the tool gestures (anim_rig.Rig.tool_frame) puts the bone exactly on this frame: the prop's local transform
        in the bone is the identity, and the tool follows both hands exactly (the right hand slides along the handle).
  - bone_local(prop, sk) is the prop frame in its bone's reference frame (translation cm, quaternion (x, y, z, w) as
    FQuat): a vertex v of the prop sits at BoneWorld * Local * v in any pose.
Primitives (dicts; "t" translation and "q" rotation of the primitive in the prop frame; "color"):
  - "box": "size" (sx, sy, sz) full sizes, centred on its origin. 12 triangles.
  - "frustum": "bottom" (rx, ry) and "top" (rx, ry) radii along its local X and Y, "height", "sides"; from z = 0 to
    z = height on its local Z; "caps": which ends are closed ("bottom", "top"); optional "cap_color". A cylinder has
    equal radii, a cone a top of (0, 0) (no top cap). 2 x sides wall triangles (sides if pointed), sides - 2 per cap.
  - "prism": "points" [(x, y), ...] convex polygon, counter-clockwise, extruded on its local Z from 0 to "height".
Triangles are wound so that (b - a) x (c - a) points out of the solid (right-handed maths); the mesh builder must
check the winding Unreal expects once (normals out) rather than trust this sentence.
Ids (the 4th per-instance float of J5c.2d; 0 = body only, no prop):
  1 axe, 2 pickaxe, 3 basket of berries, 4 logs, 5 hotte of stone, 6 hotte of gold.
Run alone to print the table of props:  python Tools/Editor/peasant_props.py
"""
import math
import os
import sys

sys.dont_write_bytecode = True  # no __pycache__ in Tools/Editor
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import anim_rig  # noqa: E402

# Colours (sRGB).
WOOD_HANDLE = (150, 104, 58)
WOOD_DARK = (112, 74, 40)
IRON = (78, 80, 88)
STEEL = (168, 172, 178)
BARK = (104, 70, 42)
CUT_WOOD = (214, 178, 122)
WICKER = (194, 156, 92)
WICKER_DARK = (146, 106, 56)
LEATHER = (92, 58, 34)
BERRY = (184, 28, 40)
BERRY_DARK = (128, 16, 30)
LEAF = (70, 128, 52)
STONE = (132, 132, 138)
STONE_LIGHT = (164, 164, 170)
STONE_DARK = (98, 98, 104)
GOLD = (228, 186, 46)
GOLD_LIGHT = (250, 214, 92)
GOLD_DARK = (190, 140, 24)

# Tool sizes shared with the gestures (anim_rig defaults: handle 80 cm from the knob to the head, knob 6 cm below the
# lower hand).
HANDLE_LENGTH = 80.0

IDENTITY = (0.0, 0.0, 0.0, 1.0)


# ---------------------------------------------------------------------------------------------------------------------
# Helpers: placing primitives. In a body prop, positions are given in body terms (right, forward, up) relative to the
# anchor and converted to the prop frame (forward, right, up).


def body_to_prop(v):
    """Body terms (right, forward, up) to the body prop frame (X forward, Y right, Z up)."""
    return (v[1], v[0], v[2])


def rotation_z_to(axis, x_hint):
    """Rotation taking local Z onto `axis` and local X toward `x_hint` (both in the prop frame)."""
    z = anim_rig.normalize(axis)
    x = anim_rig.sub(x_hint, anim_rig.scale(z, anim_rig.dot(x_hint, z)))
    if anim_rig.length(x) < 1e-6:
        x = anim_rig.cross((0.0, 1.0, 0.0), z) if abs(z[1]) < 0.9 else anim_rig.cross((1.0, 0.0, 0.0), z)
    x = anim_rig.normalize(x)
    y = anim_rig.cross(z, x)
    return anim_rig.qfrom_matrix((x, y, z))


def rotation_xz(x_axis, z_hint):
    """Rotation taking local X onto `x_axis` and local Z toward `z_hint`."""
    x = anim_rig.normalize(x_axis)
    z = anim_rig.sub(z_hint, anim_rig.scale(x, anim_rig.dot(z_hint, x)))
    z = anim_rig.normalize(z)
    y = anim_rig.cross(z, x)
    return anim_rig.qfrom_matrix((x, y, z))


def box(centre, size, color, q=IDENTITY):
    return {"shape": "box", "t": tuple(centre), "q": q, "size": tuple(size), "color": color}


def frustum_between(start, end, bottom, top, sides, color, caps=("bottom", "top"), cap_color=None, x_hint=None):
    """Frustum whose axis runs from `start` to `end` (prop frame); radii (rx, ry) along its local X (toward x_hint)
    and Y."""
    axis = anim_rig.sub(end, start)
    q = rotation_z_to(axis, x_hint or (1.0, 0.0, 0.0))
    prim = {"shape": "frustum", "t": tuple(start), "q": q, "bottom": tuple(bottom), "top": tuple(top),
            "height": anim_rig.length(axis), "sides": sides, "caps": tuple(caps), "color": color}
    if cap_color:
        prim["cap_color"] = cap_color
    return prim


def prism(points, height, origin, q, color):
    return {"shape": "prism", "t": tuple(origin), "q": q, "points": [tuple(p) for p in points], "height": height,
            "color": color}


def mirror_right(v):
    return (-v[0], v[1], v[2])


# ---------------------------------------------------------------------------------------------------------------------
# Tools (bone ik_hand_gun, tool frame: X along the handle from the knob, Y toward the striking side)

TOOL_AXIS = (1.0, 0.0, 0.0)
FLAT = (0.0, 0.0, 1.0)  # the tool frame's Z: across the blade's flat


def make_axe():
    prims = [
        # Handle, slightly past the head; a swelled knob at the end.
        frustum_between((0.0, 0.0, 0.0), (HANDLE_LENGTH + 2.5, 0.0, 0.0), (1.6, 1.6), (1.5, 1.5), 6, WOOD_HANDLE,
                        x_hint=FLAT),
        frustum_between((0.0, 0.0, 0.0), (3.0, 0.0, 0.0), (2.3, 2.3), (1.9, 1.9), 6, WOOD_DARK, x_hint=FLAT),
        # Head: the eye around the handle and the poll behind it...
        box((HANDLE_LENGTH - 4.5, -0.8, 0.0), (9.0, 7.6, 3.8), IRON),
        # ...and the blade flaring toward the edge (y = 14), 1.6 cm thick.
        prism([(HANDLE_LENGTH - 8.5, 2.8), (HANDLE_LENGTH - 0.5, 2.8), (HANDLE_LENGTH + 4.5, 14.0),
               (HANDLE_LENGTH - 15.5, 14.0)], 1.6, (0.0, 0.0, -0.8), IDENTITY, STEEL),
    ]
    return {"id": 1, "name": "axe", "label": "hache", "bone": "ik_hand_gun", "frame": "tool", "prims": prims}


def make_pickaxe():
    head = (HANDLE_LENGTH - 0.5, 0.0, 0.0)
    prims = [
        frustum_between((0.0, 0.0, 0.0), (HANDLE_LENGTH + 2.5, 0.0, 0.0), (1.7, 1.7), (1.6, 1.6), 6, WOOD_HANDLE,
                        x_hint=FLAT),
        frustum_between((0.0, 0.0, 0.0), (3.0, 0.0, 0.0), (2.4, 2.4), (2.0, 2.0), 6, WOOD_DARK, x_hint=FLAT),
        # Collar around the handle, then the two arms, a little curved back toward the hands: the point on the
        # striking side, a flat chisel on the other.
        box(head, (6.5, 6.5, 4.6), IRON),
        frustum_between(head, (HANDLE_LENGTH - 6.0, 24.0, 0.0), (2.3, 2.3), (0.35, 0.35), 6, IRON,
                        caps=("bottom",), x_hint=FLAT),
        frustum_between(head, (HANDLE_LENGTH - 5.0, -22.0, 0.0), (2.3, 2.3), (0.6, 1.8), 6, IRON,
                        caps=("bottom", "top"), x_hint=FLAT),
    ]
    return {"id": 2, "name": "pickaxe", "label": "pioche", "bone": "ik_hand_gun", "frame": "tool", "prims": prims}


# ---------------------------------------------------------------------------------------------------------------------
# Loads (bone spine_03, body prop frame; positions written in body terms relative to the anchor)


# Basket of berries held against the belly by both hands (food carry and idle): the hands of CARRY_HANDS
# (peasant_poses.py) hold its sides half-way up (their grip points at right +-15, forward 26, up 108).
BASKET_ANCHOR = (0.0, 26.0, 99.0)       # centre of the basket's bottom, body terms (reference pose)
BASKET_BOTTOM = (11.0, 9.5)             # radii (right, forward) at the bottom...
BASKET_TOP = (14.5, 12.0)               # ...and at the rim
BASKET_HEIGHT = 17.0


def b(v):
    """Body terms to the body prop frame (shorthand)."""
    return body_to_prop(v)


def radii(right_forward):
    """Radii given as (right, forward) to the prop frame's (X = forward, Y = right)."""
    return (right_forward[1], right_forward[0])


def make_basket():
    up = (0.0, 0.0, 1.0)
    rim = (BASKET_TOP[0] + 0.8, BASKET_TOP[1] + 0.8)
    prims = [
        frustum_between(b((0, 0, 0)), b((0, 0, BASKET_HEIGHT)), radii(BASKET_BOTTOM), radii(BASKET_TOP), 10, WICKER,
                        caps=("bottom",), cap_color=WICKER_DARK),
        frustum_between(b((0, 0, BASKET_HEIGHT - 2.5)), b((0, 0, BASKET_HEIGHT + 0.5)), radii(rim), radii(rim), 10,
                        WICKER_DARK, caps=()),
        # Heap of berries above the rim, a few big berries and two leaves on it (readable from the RTS camera).
        frustum_between(b((0, 0, BASKET_HEIGHT - 3.0)), b((0, 0, BASKET_HEIGHT + 4.0)),
                        radii((BASKET_TOP[0] - 0.4, BASKET_TOP[1] - 0.4)), radii((6.0, 5.0)), 10, BERRY),
        box(b((5.0, 3.0, BASKET_HEIGHT + 4.5)), (3.4, 3.4, 3.4), BERRY_DARK, rotation_z_to(up, (1.0, 0.6, 0.0))),
        box(b((-4.0, -2.0, BASKET_HEIGHT + 4.6)), (3.4, 3.4, 3.4), BERRY_DARK, rotation_z_to(up, (0.7, -1.0, 0.0))),
        box(b((-1.0, 5.0, BASKET_HEIGHT + 3.4)), (3.0, 3.0, 3.0), BERRY, rotation_z_to(up, (1.0, -0.3, 0.0))),
        box(b((8.0, -3.0, BASKET_HEIGHT + 2.6)), (5.0, 2.6, 0.6), LEAF, rotation_z_to((0.2, -0.3, 1.0), (1.0, 1.0, 0))),
        box(b((-8.5, 2.5, BASKET_HEIGHT + 2.2)), (5.0, 2.6, 0.6), LEAF, rotation_z_to((0.3, 0.2, 1.0), (1.0, -1.0, 0))),
    ]
    return {"id": 3, "name": "basket_food", "label": "panier de baies", "bone": "spine_03", "frame": "body",
            "anchor": BASKET_ANCHOR, "prims": prims}


# Bundle of four logs on the right shoulder (wood carry and idle): the bundle of the preview validated for
# A_Peasant_CarryWood (2026-10-16, drawn as lines: centre (17, -8, 161), radius 5), same direction and length; with
# real 4.5 cm logs it went 4.5 cm into the head, shoulder and raised arm (checked against the mesh's vertices), so it
# was raised 9 cm and moved 4 cm to the right (J5c.2): it now rests on the shoulder, clear of the head.
LOGS_CENTRE = (21.0, -8.0, 170.0)
LOGS_DIR = (-0.1, 0.92, 0.36)
LOGS_LENGTH = 95.0
LOGS_RADIUS = 4.5


def make_logs(sk):
    """The axes across the bundle are those of the validated preview: u = axis x up, v = u x axis, both computed in
    component space (hence the skeleton)."""
    axis = anim_rig.normalize(sk.body(LOGS_DIR))
    u = anim_rig.normalize(anim_rig.cross(axis, sk.up))
    v = anim_rig.cross(u, axis)
    prims = []
    half = LOGS_LENGTH / 2
    r = LOGS_RADIUS
    for k, (a, c) in enumerate(((-r, -r), (r, -r), (-r, r), (r, r))):
        offset = anim_rig.add(anim_rig.scale(u, a), anim_rig.scale(v, c))
        # Logs a little uneven in length so the ends read as separate logs.
        trim = (0.0, 3.0, -2.5, 1.5)[k]
        start = anim_rig.add(anim_rig.scale(axis, -half + trim), offset)
        end = anim_rig.add(anim_rig.scale(axis, half + trim * 0.5), offset)
        to_prop = lambda p: body_to_prop((anim_rig.dot(p, sk.right), anim_rig.dot(p, sk.forward), anim_rig.dot(p, sk.up)))
        prims.append(frustum_between(to_prop(start), to_prop(end), (r, r), (r * 0.94, r * 0.94), 7, BARK,
                                     cap_color=CUT_WOOD, x_hint=(0.0, 0.0, 1.0)))
    return {"id": 4, "name": "logs", "label": "fagot de buches", "bone": "spine_03", "frame": "body",
            "anchor": LOGS_CENTRE, "prims": prims}


# Hotte (wicker back-basket) on the back, two shoulder straps; filled with stone (id 5) or gold (id 6).
HOTTE_ANCHOR = (0.0, -30.0, 100.0)     # centre of its bottom, body terms (reference pose); front wall ~2 cm off the
                                       # shoulder blades (back surface at -20.6 cm at 142 cm high), clear of the waist
HOTTE_TOP = (0.0, -6.0, 52.0)          # centre of its opening, relative to the anchor (leans back a little)
HOTTE_BOTTOM_RADII = (12.5, 9.0)       # (right, forward)
HOTTE_TOP_RADII = (18.0, 13.0)
# One strap (the right one, mirrored for the left), a polyline in body terms in the reference pose (absolute, like
# the anchor), about 1.2 cm off the mesh's surface (measured on Data/SKM_Mannequin_Body.json): from the top of the
# hotte, over the shoulder, down the front of the chest (where the hands hold it, about 140 cm high), under the arm
# and back to the bottom of the hotte.
STRAP_R = [(10.0, -24.0, 151.0), (12.0, -15.5, 159.5), (12.0, -8.0, 161.3), (12.0, 0.0, 155.6), (12.5, 8.5, 147.5),
           (13.0, 12.8, 139.0), (14.0, 10.8, 132.0), (17.5, 0.0, 126.0), (16.0, -11.0, 120.0), (10.5, -24.5, 114.0)]
STRAP_WIDTH, STRAP_THICKNESS = 3.4, 0.9


def strap(points, color):
    """Thin boxes along a polyline given in absolute body terms (reference pose)."""
    prims = []
    for p, q in zip(points, points[1:]):
        a, c = b(anim_rig.sub(p, HOTTE_ANCHOR)), b(anim_rig.sub(q, HOTTE_ANCHOR))
        along = anim_rig.sub(c, a)
        mid = anim_rig.scale(anim_rig.add(a, c), 0.5)
        # Flat side facing out of the body: thickness across the strap, toward the outside (away from the spine).
        out = anim_rig.sub(mid, b(anim_rig.sub((0.0, -4.0, 0.0), HOTTE_ANCHOR)))
        out = (out[0], out[1], 0.0)
        rot = rotation_xz(along, out)
        prims.append(box(mid, (anim_rig.length(along) + STRAP_WIDTH * 0.5, STRAP_WIDTH, STRAP_THICKNESS), color, rot))
    return prims


def make_hotte(content):
    up = (0.0, 0.0, 1.0)
    top = b(HOTTE_TOP)
    rim = (HOTTE_TOP_RADII[0] + 1.0, HOTTE_TOP_RADII[1] + 1.0)
    rim_start = anim_rig.scale(top, (HOTTE_TOP[2] - 3.0) / HOTTE_TOP[2])
    prims = [
        frustum_between((0.0, 0.0, 0.0), top, radii(HOTTE_BOTTOM_RADII), radii(HOTTE_TOP_RADII), 10, WICKER,
                        caps=("bottom",), cap_color=WICKER_DARK),
        frustum_between(rim_start, anim_rig.add(top, (0.0, 0.0, 0.5)), radii(rim), radii(rim), 10, WICKER_DARK,
                        caps=()),
        # Two hoops of darker wicker around the body of the hotte.
        frustum_between(anim_rig.scale(top, 0.30), anim_rig.scale(top, 0.36),
                        radii(tuple(a + (c - a) * 0.30 + 0.5 for a, c in zip(HOTTE_BOTTOM_RADII, HOTTE_TOP_RADII))),
                        radii(tuple(a + (c - a) * 0.36 + 0.5 for a, c in zip(HOTTE_BOTTOM_RADII, HOTTE_TOP_RADII))),
                        10, WICKER_DARK, caps=()),
    ]
    prims += strap(STRAP_R, LEATHER) + strap([mirror_right(p) for p in STRAP_R], LEATHER)
    if content == "stone":
        fill, light, dark = STONE, STONE_LIGHT, STONE_DARK
        ident, name, label = 5, "hotte_stone", "hotte de pierre"
    else:
        fill, light, dark = GOLD, GOLD_LIGHT, GOLD_DARK
        ident, name, label = 6, "hotte_gold", "hotte d'or"
    # The heap: a low mound filling the opening, and chunks standing out of it.
    prims.append(frustum_between(anim_rig.scale(top, (HOTTE_TOP[2] - 4.0) / HOTTE_TOP[2]),
                                 anim_rig.add(top, (0.0, 0.0, 5.0)),
                                 radii((HOTTE_TOP_RADII[0] - 0.6, HOTTE_TOP_RADII[1] - 0.6)), radii((8.0, 6.0)), 10,
                                 fill))
    chunks = [((6.0, -1.0, 5.5), (9.0, 7.0, 6.5), (1.0, 0.5, 0.0), dark),
              ((-7.0, 1.5, 5.0), (8.0, 7.5, 7.0), (0.6, -1.0, 0.0), light),
              ((0.0, -6.0, 6.5), (7.0, 6.0, 6.0), (1.0, 1.0, 0.0), light),
              ((-1.0, 6.0, 4.0), (7.0, 6.0, 5.0), (-1.0, 0.4, 0.0), dark),
              ((10.5, 5.0, 2.5), (6.0, 5.0, 5.0), (0.3, 1.0, 0.0), fill)]
    for at, size, x_hint, color in chunks:
        centre = anim_rig.add(top, b(at))
        tilt = anim_rig.add(up, (0.25 * x_hint[1], 0.25 * x_hint[0], 0.0))
        prims.append(box(centre, size, color, rotation_z_to(tilt, b(x_hint))))
    return {"id": ident, "name": name, "label": label, "bone": "spine_03", "frame": "body", "anchor": HOTTE_ANCHOR,
            "prims": prims}


def build_props(sk):
    """All props by name (the logs need the skeleton for their validated axes)."""
    props = [make_axe(), make_pickaxe(), make_basket(), make_logs(sk), make_hotte("stone"), make_hotte("gold")]
    return {p["name"]: p for p in props}


# ---------------------------------------------------------------------------------------------------------------------
# Geometry: primitives to triangles, prop frame to bone and component space


def _ring(rx, ry, z, sides):
    return [(rx * math.cos(2 * math.pi * k / sides), ry * math.sin(2 * math.pi * k / sides), z) for k in range(sides)]


def primitive_triangles(prim):
    """Triangles (three points each) and colours of a primitive, in the prop frame."""
    shape = prim["shape"]
    out = []
    color = prim["color"]
    cap_color = prim.get("cap_color", color)
    if shape == "box":
        sx, sy, sz = (s * 0.5 for s in prim["size"])
        c = {(i, j, k): (i * sx, j * sy, k * sz) for i in (-1, 1) for j in (-1, 1) for k in (-1, 1)}
        faces = [((1, -1, -1), (1, 1, -1), (1, 1, 1), (1, -1, 1)), ((-1, 1, -1), (-1, -1, -1), (-1, -1, 1), (-1, 1, 1)),
                 ((1, 1, -1), (-1, 1, -1), (-1, 1, 1), (1, 1, 1)), ((-1, -1, -1), (1, -1, -1), (1, -1, 1), (-1, -1, 1)),
                 ((-1, -1, 1), (1, -1, 1), (1, 1, 1), (-1, 1, 1)), ((-1, 1, -1), (1, 1, -1), (1, -1, -1), (-1, -1, -1))]
        for f in faces:
            out.append(((c[f[0]], c[f[1]], c[f[2]]), color))
            out.append(((c[f[0]], c[f[2]], c[f[3]]), color))
    elif shape == "frustum":
        n, h = prim["sides"], prim["height"]
        low = _ring(prim["bottom"][0], prim["bottom"][1], 0.0, n)
        top_r = prim["top"]
        pointed = top_r[0] < 1e-6 and top_r[1] < 1e-6
        high = [(0.0, 0.0, h)] * n if pointed else _ring(top_r[0], top_r[1], h, n)
        for k in range(n):
            a, c = low[k], low[(k + 1) % n]
            if pointed:
                out.append(((a, c, high[0]), color))
            else:
                out.append(((a, c, high[(k + 1) % n]), color))
                out.append(((a, high[(k + 1) % n], high[k]), color))
        if "bottom" in prim["caps"]:
            for k in range(1, n - 1):
                out.append(((low[0], low[k + 1], low[k]), cap_color))
        if "top" in prim["caps"] and not pointed:
            for k in range(1, n - 1):
                out.append(((high[0], high[k], high[k + 1]), cap_color))
    elif shape == "prism":
        pts, h = prim["points"], prim["height"]
        n = len(pts)
        low = [(x, y, 0.0) for x, y in pts]
        high = [(x, y, h) for x, y in pts]
        for k in range(1, n - 1):
            out.append(((low[0], low[k + 1], low[k]), color))
            out.append(((high[0], high[k], high[k + 1]), color))
        for k in range(n):
            a, c = low[k], low[(k + 1) % n]
            out.append(((a, c, high[(k + 1) % n]), color))
            out.append(((a, high[(k + 1) % n], high[k]), color))
    else:
        raise ValueError("Unknown shape " + shape)
    t, q = prim["t"], prim["q"]
    return [(tuple(anim_rig.add(t, anim_rig.qrot(q, p)) for p in tri), col) for tri, col in out]


def prop_triangles(prop):
    """Every triangle of a prop in its prop frame, with its colour."""
    return [tri for prim in prop["prims"] for tri in primitive_triangles(prim)]


def prop_frame_reference(prop, sk):
    """The prop frame in component space in the reference pose: (translation, rotation)."""
    if prop["frame"] == "tool":
        return sk.ref_world[sk.index[prop["bone"]]]
    # X forward, Y right, Z up (right-handed).
    rotation = anim_rig.qfrom_matrix((sk.forward, sk.right, sk.up))
    return sk.body(prop["anchor"]), rotation


def bone_local(prop, sk):
    """The prop frame in its bone's reference frame: (translation cm, quaternion (x, y, z, w))."""
    if prop["frame"] == "tool":
        return (0.0, 0.0, 0.0), IDENTITY
    bt, bq = sk.ref_world[sk.index[prop["bone"]]]
    pt, pq = prop_frame_reference(prop, sk)
    return anim_rig.qrot(anim_rig.qconj(bq), anim_rig.sub(pt, bt)), anim_rig.qnormalize(anim_rig.qmul(anim_rig.qconj(bq), pq))


def prop_frame(prop, sk, world):
    """The prop frame in component space in a solved pose (`world` from Skeleton.fk)."""
    bt, bq = world[sk.index[prop["bone"]]]
    lt, lq = bone_local(prop, sk)
    return anim_rig.add(bt, anim_rig.qrot(bq, lt)), anim_rig.qnormalize(anim_rig.qmul(bq, lq))


def posed_triangles(prop, sk, world, triangles=None):
    """The prop's triangles in component space for a solved pose."""
    t, q = prop_frame(prop, sk, world)
    triangles = triangles if triangles is not None else prop_triangles(prop)
    return [(tuple(anim_rig.add(t, anim_rig.qrot(q, p)) for p in tri), col) for tri, col in triangles]


def bind_mesh(prop, sk):
    """What the mesh builder writes: vertices in component space in the reference pose (bind pose), triangles as
    index triples, one colour per triangle, the bone every vertex is skinned to (weight 1)."""
    vertices, triangles, colors = [], [], []
    for tri, col in posed_triangles(prop, sk, sk.ref_world):
        base = len(vertices)
        vertices += list(tri)
        triangles.append((base, base + 1, base + 2))
        colors.append(col)
    return {"bone": prop["bone"], "vertices": vertices, "triangles": triangles, "colors": colors}


def extents(prop):
    """Size of the prop along its prop frame's axes (cm)."""
    points = [p for tri, _ in prop_triangles(prop) for p in tri]
    return tuple(max(p[k] for p in points) - min(p[k] for p in points) for k in range(3))


def main():
    sk = anim_rig.Skeleton.load(os.path.join(os.path.dirname(os.path.abspath(__file__)), "Data",
                                             "UE4MannequinSkeleton.json"))
    props = build_props(sk)
    print("%-3s %-12s %-12s %-6s %-26s %-5s %s" % ("id", "piece", "os", "repere", "taille X x Y x Z (cm)", "tri",
                                                   "local dans l'os : t (cm), q"))
    for prop in sorted(props.values(), key=lambda p: p["id"]):
        t, q = bone_local(prop, sk)
        size = extents(prop)
        print("%-3d %-12s %-12s %-6s %-26s %-5d (%.1f, %.1f, %.1f) (%.4f, %.4f, %.4f, %.4f)"
              % (prop["id"], prop["name"], prop["bone"], prop["frame"], "%.0f x %.0f x %.0f" % size,
                 len(prop_triangles(prop)), t[0], t[1], t[2], q[0], q[1], q[2], q[3]))


if __name__ == "__main__":
    main()
