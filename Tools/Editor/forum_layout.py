"""What the Forum is made of (VisualInProgress.md, V2): the kit pieces it imports and, once laid out, where each one goes.
Shared by make_forum_mesh.py (Unreal) and preview_forum.py (offline, numpy and Pillow). Pure data, no unreal import.

The kit (Kenney's Retro Fantasy Kit 2.0, CC0, outside the repository: Docs/assets_art.csv) comes in modules of 1 m
(100 cm at scale 1); a piece is /Game/Buildings/Forum/Kit/SM_Kit_<name with _ for ->.
"""
KIT = r"C:\Users\baudo\Documents\MyCrusader_Art\Kenney\retro-fantasy-kit\Models\FBX format"
FOLDER = "/Game/Buildings/Forum/Kit"
PIECES = [
    "tower-base", "tower", "tower-edge", "tower-top", "tower-paint-base", "tower-paint",
    "wall-fortified", "wall-fortified-door", "wall-fortified-gate", "wall-fortified-window", "wall-fortified-half",
    "wall-fortified-paint", "wall-fortified-paint-door", "wall-fortified-paint-gate",
    "wall", "wall-door", "wall-window", "wall-half", "wall-paint", "wall-paint-door", "wall-paint-window",
    "battlement", "battlement-half", "battlement-corner-inner", "battlement-corner-outer",
    "roof", "roof-edge", "roof-corner", "roof-side", "roof-side-corner", "roof-high-side",
    "floor", "floor-flat", "stairs-stone", "stairs-wood", "column", "column-paint", "wood-floor",
    "detail-barrel", "detail-crate", "barrels", "pulley",
]

# The layout (v4, 2026-11-03: ten variants, A to J, for the user to choose from; v2 was "a hangar", v3's keep and wall
# pleased, its towers did not join the wall). Units: modules of the kit (1 = MODULE cm at scale 1); a cell's centre (x, y)
# in -2..2, z the level (0 = ground). Everything is then scaled by SCALE so that the 5 x 5 cells fill the building's VISIBLE
# size: the table's footprint (1600 cm) less VisualInset (30 cm) on every side (UHOLBuildingSettings::GetVisualSize, D324).
# The RTS camera has no rotation and looks along +X (HOLRTSCamera: yaw 0), so the front of the building, facing the camera,
# is its -X side.
MODULE = 100.0
VISUAL = 1540.0
GRID = 5
SCALE = VISUAL / GRID / MODULE

# Which way a piece faces (measured on the preview, Saved/forum_yaws.png): a door or window piece shows its opening towards
# -Y at yaw 0, so it faces the camera (-X) at 270, +X at 90, +Y at 180; the gate piece is a passage, open on both sides:
# along X at yaw 90 or 270.
FRONT = 270.0   # door and window pieces towards the camera (-X)
BACK = 90.0     # towards +X
LEFT = 0.0      # towards -Y
RIGHT = 180.0   # towards +Y
GATE_X = 90.0   # the gate's passage along X (through the front wall)


def pyramid_unit(overhang=2.0, tiles=3.0):
    """The piece "@pyramid" (the kit has no symmetric pyramid roof), in cm at scale 1: a square base of 100 + 2 x overhang
    centred on the origin at z 0, the apex at (0, 0, 100). Flat faces (one set of vertices a face). Returns (vertices, normals,
    uvs, triangles); the triangles are wound as Unreal's meshes are (the cross product (b - a) x (c - a) points into the solid)."""
    half = 50.0 + overhang
    base = [(-half, -half), (half, -half), (half, half), (-half, half)]
    apex = (0.0, 0.0, 100.0)
    vertices, normals, uvs, triangles = [], [], [], []
    for k in range(4):
        a, b = base[k], base[(k + 1) % 4]
        va, vb = (a[0], a[1], 0.0), (b[0], b[1], 0.0)
        u = (vb[0] - va[0], vb[1] - va[1], 0.0)
        w = (apex[0] - va[0], apex[1] - va[1], apex[2] - va[2])
        n = (u[1] * w[2] - u[2] * w[1], u[2] * w[0] - u[0] * w[2], u[0] * w[1] - u[1] * w[0])  # outward
        length = (n[0] ** 2 + n[1] ** 2 + n[2] ** 2) ** 0.5
        n = (n[0] / length, n[1] / length, n[2] / length)
        i = len(vertices)
        vertices += [va, vb, apex]
        normals += [n, n, n]
        uvs += [(0.0, 0.0), (tiles, 0.0), (tiles / 2.0, tiles)]
        triangles.append((i, i + 2, i + 1))
    return vertices, normals, uvs, triangles


def box_unit():
    """The piece "@pole" / "@cloth": a box of 100 cm on each side (the placement scales it), centred on x and y, base at z 0,
    flat faces. Returns (vertices, normals, uvs, triangles), wound like pyramid_unit's."""
    h = 50.0
    corners = [(-h, -h), (h, -h), (h, h), (-h, h)]
    vertices, normals, uvs, triangles = [], [], [], []
    def quad(a, b, c, d, n):
        i = len(vertices)
        vertices.extend([a, b, c, d])
        normals.extend([n] * 4)
        uvs.extend([(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)])
        triangles.extend([(i, i + 2, i + 1), (i, i + 3, i + 2)])  # as pyramid_unit: the cross product points into the solid
    for k in range(4):  # the four sides, counter-clockwise seen from outside
        a, b = corners[k], corners[(k + 1) % 4]
        n = (b[1] - a[1], -(b[0] - a[0]), 0.0)
        length = (n[0] ** 2 + n[1] ** 2) ** 0.5
        quad((a[0], a[1], 0.0), (b[0], b[1], 0.0), (b[0], b[1], 100.0), (a[0], a[1], 100.0), (n[0] / length, n[1] / length, 0.0))
    quad((-h, -h, 100.0), (h, -h, 100.0), (h, h, 100.0), (-h, h, 100.0), (0.0, 0.0, 1.0))
    quad((-h, h, 0.0), (h, h, 0.0), (h, -h, 0.0), (-h, -h, 0.0), (0.0, 0.0, -1.0))
    return vertices, normals, uvs, triangles


class Plan:
    """The placements of one variant: (piece, x, y, z, yaw, (sx, sy, sz)), cells in modules, z in levels."""

    def __init__(self):
        self.items = []

    def add(self, name, x, y, z=0, yaw=0.0, scale=(1.0, 1.0, 1.0)):
        self.items.append((name, float(x), float(y), float(z), float(yaw), tuple(float(v) for v in scale)))

    def court(self, cells=None):
        """The paved court: every cell inside the wall (or the given ones)."""
        for cx, cy in (cells if cells is not None else [(a, b) for a in (-1, 0, 1) for b in (-1, 0, 1)]):
            self.add("floor", cx, cy, 0)

    def ring(self, corner="block"):
        """The wall of the 5 x 5 cells, one block high: the gate (paved under) at the front centre, a window in the middle of
        the other sides; corner: "block" (a fortified block), "turret" (a block with a round turret on it)."""
        self.add("wall-fortified-gate", -2, 0, 0, GATE_X)
        self.add("floor", -2, 0, 0)
        self.add("wall-fortified-window", 2, 0, 0, BACK)
        self.add("wall-fortified-window", 0, 2, 0, RIGHT)
        self.add("wall-fortified-window", 0, -2, 0, LEFT)
        for k in (-1, 1):
            self.add("wall-fortified", -2, k, 0)
            self.add("wall-fortified", 2, k, 0)
            self.add("wall-fortified", k, 2, 0)
            self.add("wall-fortified", k, -2, 0)
        for tx in (-2, 2):
            for ty in (-2, 2):
                self.add("wall-fortified", tx, ty, 0)
                if corner == "turret":
                    self.add("tower-paint", tx, ty, 1)
                    self.add("tower-top", tx, ty, 2)

    def pyramid(self, x, y, z, width, height):
        """A square pyramid roof, width and height in modules. The kit has none (its roof-edge is half a hip, with a ridge that
        stops at the middle: not symmetric), so make_forum_mesh.py builds one: the piece "@pyramid" (base 104 cm, apex 100 cm)."""
        self.add("@pyramid", x, y, z, 0.0, (float(width), float(width), float(height)))

    def turret(self, x, y, levels=2):
        """A round tower `levels` modules high (the base counts) and its crown, centred on (x, y)."""
        for level in range(levels):
            self.add("tower-paint-base" if level == 0 else "tower-paint", x, y, level)
        self.add("tower-top", x, y, levels)

    def keep3(self, levels=2, roof_height=1.2):
        """The symmetric keep: 3 x 3 cells, the door in the middle of the front face, windows either side and above, a window
        in the middle of each other side on the upper levels, a pyramid roof."""
        for kx in (-1, 0, 1):
            for ky in (-1, 0, 1):
                for level in range(levels):
                    if kx == -1:
                        piece = "wall-paint-door" if level == 0 and ky == 0 else "wall-paint-window"
                        self.add(piece, kx, ky, level, FRONT)
                    elif level >= 1 and kx == 0 and ky != 0:
                        self.add("wall-paint-window", kx, ky, level, RIGHT if ky > 0 else LEFT)
                    elif level >= 1 and kx == 1 and ky == 0:
                        self.add("wall-paint-window", kx, ky, level, BACK)
                    else:
                        self.add("wall-paint", kx, ky, level, FRONT)
        if roof_height:
            self.pyramid(0, 0, levels, 3.0, roof_height)

    def keep(self, levels=2, roof="pyramid", cx=0.0, cy=0.0, size=2):
        """A block of size x size cells (2 or 3), a door on the front face (low), windows above, then the roof."""
        if size == 2:
            cells = [(-0.5, -0.5), (-0.5, 0.5), (0.5, -0.5), (0.5, 0.5)]
        else:
            cells = [(a, b) for a in (-1, 0, 1) for b in (-1, 0, 1)]
        front_x = min(c[0] for c in cells)
        door_y = -0.5 if size == 2 else 0
        for kx, ky in cells:
            front = kx == front_x
            for level in range(levels):
                if level == 0 and front and ky == door_y:
                    piece = "wall-paint-door"
                elif front:
                    piece = "wall-paint-window"
                else:
                    piece = "wall-paint"
                self.add(piece, kx + cx, ky + cy, level, FRONT)
        if roof == "pyramid":
            self.add("roof-edge", cx, cy, levels, 0.0, (float(size), float(size), 1.2))
        elif roof == "gable":
            for ky in sorted({c[1] for c in cells}):
                for kx in sorted({c[0] for c in cells}):
                    self.add("roof", kx + cx, ky + cy, levels, 0.0)


def variant_a():  # the keep and its wall, nothing else: no tower, a post at each corner
    p = Plan()
    p.court()
    p.ring("block")
    p.keep(2, "pyramid")
    return p.items


def variant_b():  # as A, with a round turret on each corner block: the tower rises out of the wall, no gap
    p = Plan()
    p.court()
    p.ring("turret")
    p.keep(2, "pyramid")
    return p.items


def variant_c():  # as A, a taller keep (three levels)
    p = Plan()
    p.court()
    p.ring("block")
    p.keep(3, "pyramid")
    return p.items


def variant_d():  # as A, the keep roofed as a hall (gables along Y)
    p = Plan()
    p.court()
    p.ring("block")
    p.keep(2, "gable")
    return p.items


def variant_e():  # a forum with a colonnade: no wall, a row of columns round a paved square, the keep in the middle
    p = Plan()
    p.court([(a, b) for a in (-2, -1, 0, 1, 2) for b in (-2, -1, 0, 1, 2)])
    for a in (-2, -1, 0, 1, 2):
        for b in (-2, 2):
            p.add("column-paint", a, b, 0)
    for b in (-1, 0, 1):
        for a in (-2, 2):
            p.add("column-paint", a, b, 0)
    p.keep(2, "pyramid")
    return p.items


def variant_f():  # the keep on a terrace (one block high) with steps in front, inside the wall
    p = Plan()
    p.court()
    p.ring("block")
    for kx in (-1, 0, 1):
        for ky in (-1, 0, 1):
            if kx != -1 or ky != 0:
                p.add("wall-paint", kx, ky, 0)
    p.add("stairs-stone", -1, 0, 0, 0.0)
    for kx, ky in ((-0.5, -0.5), (-0.5, 0.5), (0.5, -0.5), (0.5, 0.5)):
        p.add("wall-paint-window" if kx < 0 else "wall-paint", kx, ky, 1, FRONT)
        p.add("wall-paint-window" if kx < 0 else "wall-paint", kx, ky, 2, FRONT)
    p.add("roof-edge", 0, 0, 3, 0.0, (2.0, 2.0, 1.0))
    return p.items


def variant_g():  # the keep to one side, a hall along the front of the court, a turret on each corner block
    p = Plan()
    p.court()
    p.ring("turret")
    p.keep(2, "pyramid", cx=0.5, cy=0.0)
    for ky in (-1, 0, 1):
        p.add("wall-paint-window" if ky == 0 else "wall-paint", -1, ky, 0, FRONT)
    return p.items


def variant_h():  # a gatehouse (two levels and a roof) and a lone tower keep in the court
    p = Plan()
    p.court()
    p.ring("block")
    p.add("wall-fortified-window", -2, 0, 1, FRONT)
    p.add("roof-edge", -2, 0, 2, 0.0)
    for level in range(3):
        p.add("tower-paint-base" if level == 0 else "tower-paint", 0, 0, level)
    p.add("tower-top", 0, 0, 3)
    return p.items


def variant_i():  # no wall: a three-cell keep with a round turret on each of its four corners
    p = Plan()
    p.court([(a, b) for a in (-2, -1, 0, 1, 2) for b in (-2, -1, 0, 1, 2)])
    p.keep(2, "pyramid", size=3)
    for tx in (-1.5, 1.5):
        for ty in (-1.5, 1.5):
            p.add("tower-paint-base", tx, ty, 0)
            p.add("tower-paint", tx, ty, 1)
            p.add("tower-top", tx, ty, 2)
    return p.items


def variant_j():  # as A, with a parapet on the wall and barrels and crates in the court: lived in
    p = Plan()
    p.court()
    p.ring("block")
    p.keep(2, "pyramid")
    for k in (-1, 0, 1):
        if k != 0:
            p.add("battlement", -2, k, 1, FRONT)
        p.add("battlement", 2, k, 1, BACK)
        p.add("battlement", k, 2, 1, RIGHT)
        p.add("battlement", k, -2, 1, LEFT)
    p.add("barrels", -1.3, -1.2, 0, 90.0)
    p.add("detail-crate", -1.3, 1.2, 0, 20.0)
    p.add("detail-barrel", 1.3, -1.2, 0, 0.0)
    return p.items


# ---- Second round (K to T), after the user's answer on A to J: "I is good, but the roof looks cut in two like every other letter,
# symmetry is better, and the towers are sunk too far in". So: a symmetric keep (3 x 3 cells, the door in the middle), a true
# pyramid roof, turrets standing out of the keep's corners (centres at 1.8 modules, the keep's corner being at 1.5).
COURT5 = [(a, b) for a in (-2, -1, 0, 1, 2) for b in (-2, -1, 0, 1, 2)]
TURRET_AT = 1.8


def corner_turrets(p, levels=2, at=TURRET_AT):
    for tx in (-at, at):
        for ty in (-at, at):
            p.turret(tx, ty, levels)


def variant_k():  # I, corrected: the symmetric keep, a true pyramid, turrets standing out
    p = Plan()
    p.court(COURT5)
    p.keep3(2, 1.2)
    corner_turrets(p, 2)
    return p.items


def variant_l():  # as K, the turrets rising above the keep's walls
    p = Plan()
    p.court(COURT5)
    p.keep3(2, 1.2)
    corner_turrets(p, 3)
    return p.items


def variant_m():  # the wall back, with a turret on each corner block, the keep inside
    p = Plan()
    p.court()
    p.ring("turret")
    p.keep3(2, 1.2)
    return p.items


def variant_n():  # as K, a steeper roof
    p = Plan()
    p.court(COURT5)
    p.keep3(2, 1.9)
    corner_turrets(p, 2)
    return p.items


def variant_o():  # as K, a low wide roof
    p = Plan()
    p.court(COURT5)
    p.keep3(2, 0.8)
    corner_turrets(p, 2)
    return p.items


def variant_p():  # as K, a gate in front on the axis, leading to the door
    p = Plan()
    p.court(COURT5)
    p.keep3(2, 1.2)
    corner_turrets(p, 2)
    p.add("wall-fortified-gate", -2.4, 0, 0, GATE_X)
    p.add("wall-fortified", -2.4, -1, 0)
    p.add("wall-fortified", -2.4, 1, 0)
    return p.items


def variant_q():  # a taller keep, three levels, the turrets to match
    p = Plan()
    p.court(COURT5)
    p.keep3(3, 1.4)
    corner_turrets(p, 3)
    return p.items


def variant_r():  # the keep crowned with a crenellated platform and a small pyramid in the middle
    p = Plan()
    p.court(COURT5)
    p.keep3(2, 0)
    for kx in (-1, 0, 1):
        for ky in (-1, 0, 1):
            if kx != 0 or ky != 0:
                p.add("wall-fortified", kx, ky, 2)
    p.pyramid(0, 0, 2, 1.0, 1.6)
    corner_turrets(p, 2)
    return p.items


def variant_s():  # a portico: four columns in front of the keep, no turret
    p = Plan()
    p.court(COURT5)
    p.keep3(2, 1.2)
    for y in (-1.4, -0.7, 0.7, 1.4):
        p.add("column-paint", -2.0, y, 0)
    return p.items


def variant_t():  # as K, a low house either side of the keep
    p = Plan()
    p.court(COURT5)
    p.keep3(2, 1.2)
    corner_turrets(p, 2)
    for hy, face in ((-2.2, LEFT), (2.2, RIGHT)):
        p.add("wall-paint-window", 0, hy, 0, face)
        p.pyramid(0, hy, 1, 1.0, 0.8)
    return p.items


# ---- Team colour (V3b, VI-Q3: roof, banner or both; the user's to choose). A material slot named "owner" wears the player's colour
# (AHOLBuilding::BuildFinalMesh): the pyramid roof's in "roof" and "both", the cloth of the banners in "banner" and "both".
COLOUR_OPTIONS = ("roof", "banner", "both")
DEFAULT_COLOUR = "both"   # until the user answers VI-Q3 (Retours/Q-forum-colour.md): both is the one that can not be missed
TURRET_TOP = 2.3   # the turrets' crown (tower-top, 30 cm) stands on two modules
POLE_HEIGHT = 0.9  # the pole's top reaches the keep's apex (3.2 modules)


def banner_items():
    """A pole and a cloth on each turret, the cloth flying outward (along Y), seen face on by the RTS camera (it looks along +X)."""
    items = []
    for tx in (-TURRET_AT, TURRET_AT):
        for ty in (-TURRET_AT, TURRET_AT):
            side = 1.0 if ty > 0 else -1.0
            items.append(("@pole", tx, ty, TURRET_TOP, 0.0, (0.04, 0.04, POLE_HEIGHT)))
            items.append(("@cloth", tx, ty + side * 0.37, TURRET_TOP + POLE_HEIGHT - 0.5, 0.0, (0.03, 0.7, 0.5)))
    return items


VARIANTS = {name[-1].upper(): fn for name, fn in sorted(globals().items()) if name.startswith("variant_") and callable(fn)}
CHOSEN = "K"   # the user kept K on 2026-11-03 "for now"
LAYOUT = VARIANTS[CHOSEN]()
