"""The Banner of the Dawn (J8c.4, loop 4): the standard the hero plants (J8c-Q3), drawn as simple primitives.

Pure Python (no unreal import, no numpy), on the primitives of peasant_props.py (box, frustum, prism): shared by
preview_banner.py (the PNG previews the user validates) and make_banner_mesh.py (SM_Banner, built from the very same
triangles). Centimetres, sRGB colours; the prop frame of the other props: X forward, Y right, Z up, the origin at the
pole's foot on the ground. The cloth lies in the Y-Z plane (its faces look along +X and -X): the RTS camera never turns (D254) and looks along
+X, so it always sees the face looking along -X; the pole stands 5 cm behind the cloth (+X), hidden by it, so the
sun is whole from the camera (from behind the pole stands in front of the cloth).

Pieces (about 3.6 m, a hero being about 2.2 m): a pole of dark wood on a small stone footing, a gold spearhead on a
collar, a crossbar with a gold knob at each end, a white cloth with a swallow tail hung from it, a gold hem, side and
tail edges, and a gold sun (disc and twelve rays, the Dawn's) through the cloth, seen from both faces.
"""
import math
import os
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import peasant_props as pp  # noqa: E402

WHITE = (236, 234, 224)
GOLD = (222, 178, 52)
WOOD = (92, 60, 32)
STONE = (122, 118, 110)
COLORS = (("white", WHITE), ("gold", GOLD), ("wood", WOOD), ("stone", STONE))  # the mesh's four material slots

POLE_TOP = 326.0
POLE_X = 5.0  # the pole stands behind the cloth (away from the camera)
BAR_Z = 300.0
CLOTH_HALF = 52.0
CLOTH_TOP = 298.0
CLOTH_FOOT = 140.0   # the tail's two points
CLOTH_NOTCH = 172.0  # the notch's apex, on the axis
CLOTH_THICK = 1.6
TRIM_THICK = 2.4
TRIM_WIDTH = 5.0
SUN_Z = 236.0
SUN_RADIUS = 27.0
SUN_RAY = 43.0
SUN_RAYS = 12

# Local frame of a flat piece standing in the Y-Z plane: its X along Y, its Y along Z, extruded along X (the faces').
FACING = pp.rotation_xz((0.0, 1.0, 0.0), (1.0, 0.0, 0.0))


def ccw(points):
    """The points in counter-clockwise order (a prism's rule)."""
    area = sum(a[0] * b[1] - b[0] * a[1] for a, b in zip(points, points[1:] + points[:1]))
    return list(points) if area > 0.0 else list(reversed(points))


def flat(points, thickness, color):
    """A convex outline in (y, z), thick along X and centred on the cloth's plane."""
    return pp.prism(ccw(points), thickness, (-thickness * 0.5, 0.0, 0.0), FACING, color)


def sun():
    """The gold sun through the cloth: a disc along X and twelve rays (long and short in turn), flat in the Y-Z plane."""
    thick = CLOTH_THICK + 1.2
    prims = [pp.frustum_between((-thick * 0.5, 0.0, SUN_Z), (thick * 0.5, 0.0, SUN_Z), (SUN_RADIUS, SUN_RADIUS),
                                (SUN_RADIUS, SUN_RADIUS), 16, GOLD, x_hint=(0.0, 0.0, 1.0))]
    for k in range(SUN_RAYS):
        angle = 2.0 * math.pi * (k + 0.5) / SUN_RAYS
        length = SUN_RAY if k % 2 == 0 else SUN_RAY * 0.78
        along, across = (math.cos(angle), math.sin(angle)), (-math.sin(angle), math.cos(angle))
        base, half = SUN_RADIUS - 1.5, SUN_RADIUS * 0.26
        tri = [(along[0] * base + across[0] * half, along[1] * base + across[1] * half), (along[0] * length, along[1] * length),
               (along[0] * base - across[0] * half, along[1] * base - across[1] * half)]
        prims.append(flat([(y, SUN_Z + z) for y, z in tri], thick, GOLD))
    return prims


def cloth():
    """The white cloth with a swallow tail (two convex halves) and its gold hem, sides and tail edges."""
    prims = []
    for side in (-1.0, 1.0):
        prims.append(flat([(0.0, CLOTH_TOP), (side * CLOTH_HALF, CLOTH_TOP), (side * CLOTH_HALF, CLOTH_FOOT), (0.0, CLOTH_NOTCH)],
                          CLOTH_THICK, WHITE))
        # The side edge, the tail edge (from the point to the notch), under the hem.
        prims.append(flat([(side * CLOTH_HALF, CLOTH_TOP), (side * (CLOTH_HALF - TRIM_WIDTH), CLOTH_TOP),
                           (side * (CLOTH_HALF - TRIM_WIDTH), CLOTH_FOOT + TRIM_WIDTH * 0.6), (side * CLOTH_HALF, CLOTH_FOOT)],
                          TRIM_THICK, GOLD))
        prims.append(flat([(side * CLOTH_HALF, CLOTH_FOOT), (0.0, CLOTH_NOTCH), (0.0, CLOTH_NOTCH + TRIM_WIDTH),
                           (side * (CLOTH_HALF - TRIM_WIDTH), CLOTH_FOOT + TRIM_WIDTH * 0.6)], TRIM_THICK, GOLD))
    prims.append(flat([(-CLOTH_HALF, CLOTH_TOP), (CLOTH_HALF, CLOTH_TOP), (CLOTH_HALF, CLOTH_TOP - 8.0), (-CLOTH_HALF, CLOTH_TOP - 8.0)],
                      TRIM_THICK, GOLD))
    return prims + sun()


def make_banner():
    """Every primitive of the standard, in the prop frame."""
    pole = [pp.frustum_between((POLE_X, 0.0, -20.0), (POLE_X, 0.0, POLE_TOP), (3.6, 3.6), (3.0, 3.0), 8, WOOD, caps=("top",)),
            # The footing: a low stone mound the pole stands in.
            pp.frustum_between((POLE_X, 0.0, 0.0), (POLE_X, 0.0, 9.0), (17.0, 17.0), (11.0, 11.0), 8, STONE, caps=("top",)),
            # The gold collar and the spearhead.
            pp.frustum_between((POLE_X, 0.0, POLE_TOP - 2.0), (POLE_X, 0.0, POLE_TOP + 7.0), (5.2, 5.2), (4.4, 4.4), 8, GOLD),
            pp.frustum_between((POLE_X, 0.0, POLE_TOP + 7.0), (POLE_X, 0.0, POLE_TOP + 40.0), (5.4, 3.0), (0.0, 0.0), 6, GOLD, caps=("bottom",)),
            # The crossbar and its two knobs.
            pp.box((POLE_X - 1.0, 0.0, BAR_Z + 1.5), (3.6, 2.0 * (CLOTH_HALF + 6.0), 3.6), WOOD),
            pp.box((POLE_X - 1.0, -(CLOTH_HALF + 8.0), BAR_Z + 1.5), (6.0, 6.0, 6.0), GOLD),
            pp.box((POLE_X - 1.0, CLOTH_HALF + 8.0, BAR_Z + 1.5), (6.0, 6.0, 6.0), GOLD)]
    return pole + cloth()


def triangles():
    """Every triangle (three points) with its colour, in the prop frame."""
    return [tri for prim in make_banner() for tri in pp.primitive_triangles(prim)]


if __name__ == "__main__":
    tris = triangles()
    xs, ys, zs = zip(*(p for tri, _ in tris for p in tri))
    print("Banner: %d primitives, %d triangles; x %.1f..%.1f, y %.1f..%.1f, z %.1f..%.1f cm" %
          (len(make_banner()), len(tris), min(xs), max(xs), min(ys), max(ys), min(zs), max(zs)))
