"""The recipe of L'Arriere-pays (the chantier Cartes, C2; GDD 8.5): what the Hinterland adds to the shared pieces of
maps/common.py. The riches are behind the bases, along the edges of the map: each base has a POCKET behind it, between the base
and the edge, rich in gold and stone and with a big forest, closed in by long irregular walls of rock and reached by the paths along
the border, never through the middle (A-Q1 = A, A-Q2 = A). The middle of the map is not empty (M-Q7, 2026-10-30, which cancels A-Q2's empty middle): small groves and outcrops, and a share of the resources in front of each base. Made by create_match_map.py as
/Game/Maps/Hinterland_<Size> (no fixed test map: only Plain has one, D494).

  the belt      a closed wall of rock round the map, BELT_FROM_EDGE (26 m) in from the edge: between its outer face and the edge, a
                border corridor of about 20 m runs all the way round. The belt is the square of the map (half side Hb) except behind
                a pocket: there it is the pocket's back wall, and between two pockets it cuts the corner (a chord from one pocket's
                back corner to the next one's), so that every pocket's back is at the same distance from its base.
  the pockets   one chamber behind each base, the same shape at every base (turned onto each slot: the sources in it are the same
                distance from every place, GDD 8.1): its back wall is part of the belt, its sides and its inner wall are walls of
                its own; the inner wall steps toward the middle on both sides of the base (a crescent: two wings, deeper than the
                part behind the base, which is shallow or shut on the small maps). OPENINGS_PER_POCKET gaps of GAP_WIDTH (14 m) in
                the back wall open onto the border corridor. Nothing else leads in.
  the passages  rare gaps of GAP_WIDTH in the belt, between two pockets (PASSAGE_SPACING: one every 200 m of belt, three at most per
                stretch): the only way from the middle of the map to the border corridor, and so to the pockets.
  contents      in each pocket, the same for every place: the BIG piles of gold and stone (POCKET_MINERALS: 6 or 7 piles of several sizes, M-Q9) in
                two or three staggered rows along the back wall, the big forests (groves of 28 to 40 trees) further in, and the berry patches; the base itself is a small
                organic wood of BASE_TREES and 6 berry bushes, with the ridge of three boulders of D286 in front of it, and no
                vein. The resources hold the budget per player of M-Q4 (maps/steppe.py BUDGET): the trees and the berries by number, the gold and the stone by
                VALUE in a mix of sizes (M-Q9, common.MINERAL_MIX), shared out between the pocket (the big piles), the front (a medium, two small) and the middle
                of the map (small piles and a medium of stone, thrown in each territory: the same list for each, M-Q6).
  ground        the varied ground of the Hinterland (GROUND: MI_Ground_Hinterland, a dark moss and rock earth).

Everything is in cm in the world frame unless a name says slot frame: x radial outward from the map's middle through the slot, y on
its left (counterclockwise). All the values to tune at the user's trial are the named constants just below.
"""
import math

import unreal

from maps import common, steppe
from maps.common import Rng, mix_seed, placed_source

GROUND = "MI_Ground_Hinterland"

# M-Q10 (the user, 2026-10-30): no scenery (DECOR) and no clumps (CLUMPS): nothing without a strategic, economic or military interest on a map
# without water; the empty-circle rule (M-Q7) is held by the resources and the rocks alone.

# ---- The base: a small organic wood and the berries, no vein (the veins are in the pocket).
BASE_WOOD_ROWS = 5
BASE_WOOD_COLUMNS = 5
BASE_PACKS = [common.PACK_TREES, common.PACK_TREES]  # M-Q8: the base's wood is two packs of 14 (it was one grove of 25)
BASE_TREES = sum(BASE_PACKS)
BASE_BERRIES = common.BERRY_ROWS * common.BERRY_COLUMNS

# ---- The belt, the pockets, the gaps (cm).
BELT_FROM_EDGE = 2600.0  # the belt's axis this far in from the edge of the bounds: with a rock's half thickness, a corridor of about 20 m
GAP_WIDTH = 1400.0  # the free width of an opening or a passage (a formation of an army passes)
OPENINGS_PER_POCKET = 2
OPENING_FRACTIONS = (0.2, 0.8)  # along the back wall, from one corner to the other
PASSAGE_SPACING = 20000.0  # one passage per this much belt between two pockets (rounded), at most MAX_PASSAGES
MAX_PASSAGES = 3
PASSAGE_FROM_VERTEX = 3000.0  # a passage's centre is at least this far from a corner of the belt or a pocket's corner
POCKET_HALF_WIDTH_MAX = 18000.0  # half the width of a pocket (along the border): fitted down so that two pockets keep MIN_VALLEY apart
MIN_VALLEY = 3500.0  # the least distance between the walls of two neighbouring pockets
MIN_CHORD = 5500.0  # the shortest stretch of belt between two pockets: room for a passage and two rock ends
WING_DEPTH = 9000.0  # how far in from the back wall the inner wall of a wing is
CORE_HALF = 4500.0  # the part of the inner wall behind the base: this half width ...
RAMP = 1500.0  # ... and this much more to step down to the wing
CENTRE_WALL = 2200.0  # the inner wall behind the base is never nearer the slot than this (the base's wood and the centre's room)
MIN_AISLE = 1400.0  # if the part behind the base is shallower than this, it is shut (a double wall)
SHUT_GAP = 700.0

# ---- The contents of a pocket (slot frame, u = how far in from the back wall's axis).
WALL_CLEAR = 900.0  # a vein is this far from a wall's axis at least (a wall's half thickness is 5 m)
OPENING_CLEAR = 1100.0  # and from the middle of an opening
VEIN_ROWS = (1600.0, 2900.0, 4200.0)
VEIN_STEP = 2400.0  # between two veins of a row
VEIN_FIRST = 5500.0  # the first vein of a row, from the pocket's axis
GROVE_SIZES = [common.PACK_TREES]  # M-Q8: the pocket's woods are packs of 14 (they were groves of 28 to 40)
POCKET_MIN_PACKS = 4  # the pocket keeps a real forest whatever the middle brings: 4 packs of 14
TREES_MARGIN = 1.0  # the pocket, the front and the base hold the budget by themselves; the packs of the middle come on top
BERRIES_MARGIN = 1.15
PATCH_SIZE = 5
LATTICE = 300.0  # step of the lattice the groves and patches are searched on
GROVE_WALL = 780.0  # a grove's farthest tree is this far from the walls' axes at least
WOOD_GAP = 700.0  # M-Q10b: between two packs of wood, more than the 6 m a pack is linked at (reaches are measured from the centres: two packs 5 m apart read as one cluster of 28)
ITEM_GAP = 500.0  # between the reach of two deposits of the pocket (M-Q8: two packs are two clusters, more than the 6 m a pack is linked at)

# ---- In front of each base (M-Q7: the riches are not only behind; slot frame, x ahead of the slot is negative): a lane of 80 m towards the
# middle, beyond the ridge of D286, with a small share of everything: two gold veins, a stone vein, a grove of 24 and one of 16, a patch of berries. The
# same at every slot (the nearest three of each type stay the same distance from every place). The pocket keeps the big veins.
# M-Q9: the veins are piles of several sizes (a vein's size is its tier, common.MINERAL_AMOUNTS).
FRONT = [("GoldVein", "Medium", -5500.0, -3300.0), ("StoneVein", "Small", -5500.0, 3300.0), ("GoldVein", "Small", -8500.0, -1200.0), ("Wood", 14, -9000.0, 2900.0),
         ("Wood", 14, -7000.0, -5600.0), ("BerryBush", 5, -5900.0, 100.0)]
FRONT_TREES = sum(size for kind, size, _, _ in FRONT if kind == "Wood")
FRONT_MINERALS = [(kind, size) for kind, size, _, _ in FRONT if kind.endswith("Vein")]

# ---- The value budget of the minerals (M-Q9, common.MINERAL_MIX) shared out between the zones, the same in every territory: the pocket keeps
# the BIG piles (the order is the order of the places, the back wall first: the biggest sits against the back wall), the front a medium and two
# small ones, the middle of the map the rest: small piles and a medium of stone, thrown in each territory (the same list for each).
POCKET_MINERALS = {
    "default": [("GoldVein", "Big"), ("StoneVein", "Standard"), ("GoldVein", "Medium"), ("GoldVein", "Small"), ("StoneVein", "Small"), ("GoldVein", "Small")],
    "Huge": [("GoldVein", "Big"), ("GoldVein", "Big"), ("StoneVein", "Big"), ("GoldVein", "Medium"), ("GoldVein", "Small"), ("StoneVein", "Small"), ("GoldVein", "Small")],
}
MIDDLE_MINERALS = [("GoldVein", "Small"), ("StoneVein", "Medium"), ("GoldVein", "Small"), ("StoneVein", "Small"), ("GoldVein", "Small")]
MINERAL_GAP = 3500.0  # between two piles of the middle, and from any other pile or pack
MINERAL_FROM_PACK = 3000.0  # a pile of the middle from a pack's or an outcrop's centre (a pack reaches 11 m)
MINERAL_SPREAD = 4000.0  # two piles of one resource
MINERAL_DRAWS = 40000
MINERAL_FROM_SLOT = 14000.0  # a pile of the middle is farther than this from every slot
FRONT_LANE = (-11000.0, -3500.0, 4600.0)  # the lane's x range and half width: the middle's scatter keeps out of it
# ---- The middle of the map (M-Q7: nothing may be empty; A-Q2's empty centre is cancelled): small groves and isolated outcrops scattered inside
# the belt, away from the bases, the pockets, the walls and the passages. The groves are sources of wood (more than the budget asks).
MIDDLE_GAP = 3800.0  # between the centres of two of its deposits
MIDDLE_GROVES = (common.PACK_TREES,)  # M-Q8: packs of 14
MIDDLE_ROCK_EVERY = 2  # one outcrop for every this many deposits (M-Q8: half the middle's deposits are packs of 14)
MIDDLE_FROM_SLOT = 6000.0
MIDDLE_FROM_FRONT = 3200.0
MIDDLE_FROM_POCKET = 2200.0
MIDDLE_FROM_WALL = 2500.0
MIDDLE_FROM_PASSAGE = 4500.0
MIDDLE_TRIES = 6000
SALT_MIDDLE = 204
SALT_OUTCROP = 205
SALT_MINERAL = 206

SALT_WALL = 201
SALT_GROVE = 202
SALT_PATCH = 203

PLANS = {}


# ---- Plane geometry.
def segment_distance(point, a, b):
    ax, ay = a
    bx, by = b
    dx, dy = bx - ax, by - ay
    length2 = dx * dx + dy * dy
    t = 0.0 if length2 == 0.0 else max(0.0, min(1.0, ((point[0] - ax) * dx + (point[1] - ay) * dy) / length2))
    return math.hypot(point[0] - (ax + dx * t), point[1] - (ay + dy * t))


def outline_distance(point, outline, closed=True):
    count = len(outline)
    return min(segment_distance(point, outline[index], outline[(index + 1) % count]) for index in range(count if closed else count - 1))


def inside(point, outline):
    """Whether a point is in the closed polygon (ray casting)."""
    result = False
    for index in range(len(outline)):
        (x1, y1), (x2, y2) = outline[index], outline[(index + 1) % len(outline)]
        if (y1 > point[1]) != (y2 > point[1]) and point[0] < (x2 - x1) * (point[1] - y1) / (y2 - y1) + x1:
            result = not result
    return result


def outlines_apart(first, second):
    """The least distance between two polygons' edges (sampled every 2 m)."""
    least = math.inf
    for index in range(len(first)):
        a, b = first[index], first[(index + 1) % len(first)]
        steps = max(1, int(math.dist(a, b) // 200.0))
        for step in range(steps + 1):
            point = (a[0] + (b[0] - a[0]) * step / steps, a[1] + (b[1] - a[1]) * step / steps)
            least = min(least, outline_distance(point, second))
    return least


# ---- The layout.
def world(recipe, index, x, y):
    """A point of slot index's frame in the world."""
    slot, radial, left = recipe.slot(index)
    return (slot[0] + radial[0] * x + left[0] * y, slot[1] + radial[1] * x + left[1] * y)


def angle_of(point):
    return math.degrees(math.atan2(point[1], point[0])) % 360.0


def outline_of(x_back, half_width, x_core):
    """A pocket's closed outline in the slot frame: the back wall, both sides, and the inner wall with its core behind the base."""
    x_wing = x_back - WING_DEPTH
    return [(x_back, -half_width), (x_wing, -half_width), (x_wing, -(CORE_HALF + RAMP)), (x_core, -CORE_HALF), (x_core, CORE_HALF),
            (x_wing, CORE_HALF + RAMP), (x_wing, half_width), (x_back, half_width)]


class Pocket:
    """One slot's pocket: its outline in the slot frame, the world points of its back corners, its openings."""

    def __init__(self, recipe, index, x_back, half_width, x_core):
        self.index = index
        self.x_back = x_back
        self.half_width = half_width
        self.x_core = x_core
        self.outline = outline_of(x_back, half_width, x_core)
        self.world_outline = [world(recipe, index, x, y) for x, y in self.outline]
        self.minus = world(recipe, index, x_back, -half_width)  # the corner on the clockwise side
        self.plus = world(recipe, index, x_back, half_width)  # the corner on the counterclockwise side
        self.opening_ys = [(2 * fraction - 1) * half_width for fraction in OPENING_FRACTIONS][:OPENINGS_PER_POCKET]
        self.openings = [world(recipe, index, x_back, y) for y in self.opening_ys]


def pockets_of(recipe):
    """The pockets of every slot with the widest half width that keeps the walls of neighbours MIN_VALLEY apart and the belt
    between them MIN_CHORD long. Returns (pockets in the slots' order, half belt Hb, chords as (first, second))."""
    belt = recipe.bounds_half - BELT_FROM_EDGE
    x_back = belt - recipe.slot_radius
    x_core = max(x_back - WING_DEPTH, CENTRE_WALL)
    if x_back - x_core < MIN_AISLE:
        x_core = x_back - SHUT_GAP
    order = sorted(range(len(recipe.angles)), key=lambda index: recipe.angles[index])
    half_width = POCKET_HALF_WIDTH_MAX
    while half_width >= 4000.0:
        pockets = [Pocket(recipe, index, x_back, half_width, x_core) for index in range(len(recipe.angles))]
        chords = [(order[number], order[(number + 1) % len(order)]) for number in range(len(order))]
        inside_belt = all(abs(x) <= belt + 1.0 and abs(y) <= belt + 1.0 for pocket in pockets for x, y in pocket.world_outline)
        apart = all(outlines_apart(pockets[a].world_outline, pockets[b].world_outline) >= MIN_VALLEY for a, b in chords if a != b)
        long_enough = all(polyline_length(chord_points(recipe, pockets, belt, a, b)) >= MIN_CHORD for a, b in chords)
        if inside_belt and apart and long_enough:
            return pockets, belt, chords
        half_width -= 250.0
    raise RuntimeError("Hinterland: no pocket width keeps the pockets apart at %s" % recipe.size_key)


polyline_length = common.polyline_length


def chord_points(recipe, pockets, belt, first, second):
    """The belt between pocket first's counterclockwise corner and pocket second's clockwise corner, with the corners of the
    square between them when no pocket hides them."""
    start, end = pockets[first].plus, pockets[second].minus
    start_angle = angle_of(start)
    span = (angle_of(end) - start_angle) % 360.0
    if first == second:
        span = 360.0
    points = [start]
    corners = []
    for corner_angle, corner in ((45.0, (belt, belt)), (135.0, (-belt, belt)), (225.0, (-belt, -belt)), (315.0, (belt, -belt))):
        along = (corner_angle - start_angle) % 360.0
        if 0.5 < along < span - 0.5:
            corners.append((along, corner))
    points += [corner for _, corner in sorted(corners)]
    points.append(end)
    return points


def passage_centres(points):
    """The arc lengths of the passages on a stretch of belt: round(length / PASSAGE_SPACING) of them, 1 to MAX_PASSAGES, evenly
    spread, each at least PASSAGE_FROM_VERTEX from every vertex of the stretch (moved along it when not)."""
    total = polyline_length(points)
    count = max(1, min(MAX_PASSAGES, int(round(total / PASSAGE_SPACING))))
    vertices = []
    run = 0.0
    for index, point in enumerate(points):
        if index:
            run += math.dist(points[index - 1], point)
        vertices.append(run)
    centres = []
    for number in range(count):
        target = total * (number + 0.5) / count
        best = None
        for shift in range(0, int(total), 100):
            for candidate in (target + shift, target - shift):
                if PASSAGE_FROM_VERTEX <= candidate <= total - PASSAGE_FROM_VERTEX and all(abs(candidate - v) >= PASSAGE_FROM_VERTEX for v in vertices) \
                        and all(abs(candidate - c) >= 4 * GAP_WIDTH for c in centres):
                    best = candidate
                    break
            if best is not None:
                break
        if best is None:
            raise RuntimeError("Hinterland: no room for passage %d on a stretch of belt %.0f cm long" % (number, total))
        centres.append(best)
    return centres


class Plan:
    """Everything the recipe puts in one map file."""

    def __init__(self):
        self.rocks = []
        self.volumes = []
        self.markers = []  # (label, x, y)
        self.sources = []
        self.points = []  # (type, x, y, zone) of the sources placed beyond the bases, as the sources hold them; zone: pocket, front or middle
        self.pockets = []
        self.belt = 0.0
        self.walls = []  # (name, points, gaps) as given to rock_wall
        self.passages = []  # world xy
        self.openings = []  # (slot, world xy)
        self.items = {}


def pocket_minerals(recipe):
    """The piles of the pocket, [(kind, tier)], and a check that the pocket, the front and the middle together are the mix of the budget
    (common.mineral_mix): nothing missing, nothing more."""
    pocket = POCKET_MINERALS["Huge" if recipe.size_key == "Huge" else "default"]
    held = sorted((kind[:-len("Vein")], tier) for kind, tier in pocket + FRONT_MINERALS + MIDDLE_MINERALS)
    wanted = sorted((resource, tier) for resource in common.MINERAL_RESOURCES for tier in common.mineral_mix(recipe)[resource])
    if held != wanted:
        raise RuntimeError("Hinterland: the pocket, the front and the middle hold %s, the mix wants %s" % (held, wanted))
    return pocket


def budget_counts(recipe, middle_least=0):
    """The pocket's needs: the packs and berry patches that, with the base, the front and at least middle_least trees of the
    middle (the fewest any territory's middle holds), come to the budget."""
    trees, berries = steppe.budget_of(recipe)
    groves = []
    need = max(math.ceil(trees * TREES_MARGIN) - BASE_TREES - FRONT_TREES - middle_least, POCKET_MIN_PACKS * common.PACK_TREES)
    while sum(groves) < need:
        groves.append(GROVE_SIZES[len(groves) % len(GROVE_SIZES)])
    patches = max(0, math.ceil((math.ceil(berries * BERRIES_MARGIN) - BASE_BERRIES) / PATCH_SIZE))
    return groves, patches


def place_contents(recipe, pocket, middle_least=0):
    """The pocket's contents in the slot frame, [(kind, size, x, y)], the same for every slot (the slot frame is the same whatever
    the place, so this is computed once): veins first, the groves and the berry patches in the room that is left."""
    groves, patches = budget_counts(recipe, middle_least)
    outline = pocket.outline
    x_back = pocket.x_back
    half_width = pocket.half_width
    keepout = [(x_back, y) for y in pocket.opening_ys]

    def wall_clear(point, margin):
        return inside(point, outline) and outline_distance(point, outline) >= margin and all(math.dist(point, k) >= OPENING_CLEAR + margin - WALL_CLEAR
                                                                                             for k in keepout)

    # Veins: staggered rows from the back wall, the nearest to the pocket's axis first, both wings in turn.
    candidates = []
    for row, depth in enumerate(VEIN_ROWS):
        for step in range(8):
            for side in (1.0, -1.0):
                y = side * (VEIN_FIRST + step * VEIN_STEP + (0.5 * VEIN_STEP if row % 2 else 0.0))
                point = (x_back - depth, y)
                if abs(y) <= half_width - WALL_CLEAR and wall_clear(point, WALL_CLEAR):
                    candidates.append((row, abs(y), -side, point))
    candidates.sort()
    minerals = pocket_minerals(recipe)
    if len(candidates) < len(minerals):
        raise RuntimeError("Hinterland: room for %d veins in a pocket, %d wanted" % (len(candidates), len(minerals)))
    items = [(kind, tier, candidate[3][0], candidate[3][1]) for (kind, tier), candidate in zip(minerals, candidates)]
    reaches = [0.0] * len(items)

    def free(point, reach, wood=False):
        for (kind, size, x, y), other_reach in zip(items, reaches):
            gap = WOOD_GAP if wood and kind == "Wood" else ITEM_GAP
            if math.hypot(point[0] - x, point[1] - y) < reach + other_reach + gap:
                return False
        return True

    # Groves then patches: on a lattice, the deepest and most central place that fits. Each deposit's shape is drawn from its own
    # seed and the reach of the real shape is used (the shape is drawn first and its farthest tree measured).
    lattice = []
    x = x_back - WALL_CLEAR
    while x > x_back - WING_DEPTH:
        y = -half_width
        while y <= half_width:
            lattice.append((x, y))
            y += LATTICE
        x -= LATTICE
    lattice.sort(key=lambda p: (-(x_back - p[0]) // 600.0, abs(p[1])))
    for number, size in enumerate(groves):
        seed = mix_seed(recipe.layout_seed, SALT_GROVE, number)
        shape = common.grove_local(seed, size)
        reach = max(math.hypot(px, py) for px, py in shape)
        for point in lattice:
            if inside(point, outline) and outline_distance(point, outline) >= reach + GROVE_WALL and free(point, reach, True) \
                    and all(math.dist(point, k) >= reach + OPENING_CLEAR + 600.0 for k in keepout):
                items.append(("Wood", size, point[0], point[1]))
                reaches.append(reach)
                break
        else:
            raise RuntimeError("Hinterland: no room for grove %d (%d trees, reach %.0f cm) in a pocket" % (number, size, reach))
    for number in range(patches):
        seed = mix_seed(recipe.layout_seed, SALT_PATCH, number)
        shape = common.berry_local(seed, PATCH_SIZE)
        reach = max(math.hypot(px, py) for px, py in shape)
        for point in reversed(lattice):
            if inside(point, outline) and outline_distance(point, outline) >= reach + GROVE_WALL and free(point, reach) \
                    and all(math.dist(point, k) >= reach + OPENING_CLEAR + 600.0 for k in keepout):
                items.append(("BerryBush", PATCH_SIZE, point[0], point[1]))
                reaches.append(reach)
                break
        else:
            raise RuntimeError("Hinterland: no room for berry patch %d in a pocket" % number)
    return items


def belt_polygon(recipe, pockets, belt, chords):
    """The belt as a closed polygon (the middle's boundary), counterclockwise: each pocket's back wall, then the stretch of belt to the next."""
    points = []
    for first, second in chords:
        points += [pockets[first].minus, pockets[first].plus] + chord_points(recipe, pockets, belt, first, second)[1:-1]
    return points


def middle_content(recipe, result, pockets, belt, chords):
    """The scatter of the middle (MIDDLE_*): first the piles of MIDDLE_MINERALS (M-Q9: territory by territory, the same list for each, so that the value is exact), then groves of MIDDLE_GROVES trees and outcrops (a short wall of rock,
    one navigation volume), dart-thrown from the seed inside the belt, away from every slot, front lane, pocket, wall and passage, MIDDLE_GAP
    from each other and MINERAL_FROM_PACK from a pile. The outcrops' rocks and volumes go to result.rocks / result.volumes; returns the groves
    as (size, centre, seed, radial) and the piles as (kind, tier, centre)."""
    groves = []
    minerals = []
    polygon = belt_polygon(recipe, pockets, belt, chords)
    slots = [recipe.slot(index)[0] for index in range(len(recipe.angles))]
    rng = Rng(recipe.layout_seed, SALT_MIDDLE)
    placed = []
    count = 0

    def clear(point, own_gap=MIDDLE_GAP):
        if not inside(point, polygon) or outline_distance(point, polygon) < MIDDLE_FROM_WALL:
            return False
        if any(math.dist(point, other) < own_gap for other in placed):
            return False
        for index, slot in enumerate(slots):
            if math.dist(point, slot) < MIDDLE_FROM_SLOT:
                return False
            _, radial, left = recipe.slot(index)
            x = (point[0] - slot[0]) * radial[0] + (point[1] - slot[1]) * radial[1]
            y = (point[0] - slot[0]) * left[0] + (point[1] - slot[1]) * left[1]
            if FRONT_LANE[0] - 1500.0 <= x <= FRONT_LANE[1] + 1500.0 and abs(y) <= FRONT_LANE[2] + 1500.0:
                return False
            # M-Q8: a pack of the middle is a cluster of its own, far enough from the front's packs (a grove of 14 reaches 11 m).
            if any(math.dist(point, world(recipe, index, fx, fy)) < MIDDLE_FROM_FRONT for kind, _, fx, fy in FRONT if kind == "Wood"):
                return False
        if any(inside(point, pocket.world_outline) or outline_distance(point, pocket.world_outline) < MIDDLE_FROM_POCKET for pocket in pockets):
            return False
        return not any(math.dist(point, passage) < MIDDLE_FROM_PASSAGE for passage in result.passages)

    # The piles: territory by territory, each its own draws (the territories do not look alike, M-Q6), the same list for each (the value is
    # exact). Beyond MINERAL_FROM_SLOT of every slot: farther than the 3 nearest piles of the resource, which are the pocket's and the front's,
    # the same around every slot (the fairness of Map.Table).
    mineral_rng = Rng(recipe.layout_seed, SALT_MINERAL)
    pile_points = []  # every pile placed: (kind, point)
    for index in range(len(slots)):
        for kind, tier in MIDDLE_MINERALS:
            for _ in range(MINERAL_DRAWS):
                point = (mineral_rng.range(-belt, belt), mineral_rng.range(-belt, belt))
                if min(range(len(slots)), key=lambda other: math.dist(point, slots[other])) != index:
                    continue
                if any(math.dist(point, slot) < MINERAL_FROM_SLOT for slot in slots) or not clear(point, MINERAL_GAP):
                    continue
                if any(math.dist(point, other) < MINERAL_GAP for _, other in pile_points):
                    continue
                if any(math.dist(point, other) < MINERAL_SPREAD for other_kind, other in pile_points if other_kind == kind):
                    continue
                pile_points.append((kind, point))
                minerals.append((kind, tier, point))
                break
            else:
                raise RuntimeError("Hinterland: no place for a %s %s pile in the middle, territory of slot %d" % (tier, kind, index))

    for _ in range(MIDDLE_TRIES):
        point = (rng.range(-belt, belt), rng.range(-belt, belt))
        if not clear(point) or any(math.dist(point, other) < MINERAL_FROM_PACK for _, other in pile_points):
            continue
        placed.append(point)
        if count % MIDDLE_ROCK_EVERY == MIDDLE_ROCK_EVERY - 1:
            angle = rng.range(0.0, 2 * math.pi)
            half = rng.range(300.0, 500.0)
            line = [(point[0] - half * math.cos(angle), point[1] - half * math.sin(angle)), (point[0] + half * math.cos(angle), point[1] + half * math.sin(angle))]
            rocks, volumes = common.rock_wall(mix_seed(recipe.layout_seed, SALT_OUTCROP, count), line)
            result.rocks += rocks
            result.volumes += volumes
        else:
            size = MIDDLE_GROVES[rng.below(len(MIDDLE_GROVES))]
            angle = rng.range(0.0, 2 * math.pi)
            groves.append((size, point, mix_seed(recipe.layout_seed, SALT_MIDDLE, count), (math.cos(angle), math.sin(angle))))
        count += 1
    return groves, minerals


def plan(recipe):
    """The whole layout of one map file (cached): the walls with their rocks and volumes, the gap markers, the sources."""
    key = (recipe.map_path, recipe.layout_seed)
    if key in PLANS:
        return PLANS[key]
    result = Plan()
    pockets, belt, chords = pockets_of(recipe)
    result.pockets = pockets
    result.belt = belt
    places = len(recipe.angles)

    def add_wall(name, points, centres):
        seed = mix_seed(recipe.layout_seed, SALT_WALL, len(result.walls))
        rocks, volumes = common.rock_wall(seed, points, [(centre, GAP_WIDTH) for centre in centres])
        result.walls.append((name, points, centres))
        result.rocks += rocks
        result.volumes += volumes

    for pocket in pockets:
        index = pocket.index
        # The back wall (part of the belt) with the openings, and the pocket's own side and inner walls.
        back = [pocket.minus, pocket.plus]
        add_wall("Back%d" % index, back, [fraction * math.dist(*back) for fraction in OPENING_FRACTIONS][:OPENINGS_PER_POCKET])
        for number, point in enumerate(pocket.openings):
            result.markers.append(("PocketGap%d_%d" % (index, number), point[0], point[1]))
            result.openings.append((index, point))
        # The side and inner walls, from the clockwise back corner round to the counterclockwise one.
        add_wall("Inner%d" % index, pocket.world_outline, [])
    for number, (first, second) in enumerate(chords):
        points = chord_points(recipe, pockets, belt, first, second)
        centres = passage_centres(points)
        add_wall("Belt%d" % number, points, centres)
        for gap, centre in enumerate(centres):
            point, _ = common.point_along(points, centre)
            result.markers.append(("BeltGap%d_%d" % (number, gap), point[0], point[1]))
            result.passages.append(point)

    # The sources: the bases (source 0 is the first tree of slot 0's wood), then each pocket's contents and each front lane's, in the slot frame.
    result.sources = common.base_sources(recipe, BASE_WOOD_ROWS, BASE_WOOD_COLUMNS, veins=False, organic=True, wood_packs=BASE_PACKS)
    # The middle first: its packs count to the territories' budget (M-Q8), the pocket holds what is missing from the fewest of them.
    middle, middle_minerals = middle_content(recipe, result, pockets, belt, chords)
    slots = [recipe.slot(index)[0] for index in range(places)]
    held = [0] * places
    for size, point, seed, radial in middle:
        for px, py in common.grove_points(seed, point, size, radial):
            held[min(range(places), key=lambda index: math.dist((px, py), slots[index]))] += 1
    contents = place_contents(recipe, pockets[0], min(held))
    pocket_count = len(contents)
    contents = contents + [(kind, size, x, y) for kind, size, x, y in FRONT]
    result.items = contents

    def add_deposit(kind, size, at, seed, radial, zone):
        if kind == "Wood":
            points = common.grove_points(seed, at, size, radial)
        elif kind == "BerryBush":
            points = common.berry_points(seed, at, size, radial)
        else:
            points = [at]
        type_name = "Tree" if kind == "Wood" else common.mineral_type(kind[:-len("Vein")], size) if kind.endswith("Vein") else kind
        for px, py in points:
            result.points.append((type_name, px, py, zone))
            result.sources.append(placed_source(type_name, px, py))

    for pocket in pockets:
        _, radial, _ = recipe.slot(pocket.index)
        for number, (kind, size, x, y) in enumerate(contents):
            salt = SALT_PATCH if kind == "BerryBush" else SALT_GROVE
            add_deposit(kind, size, world(recipe, pocket.index, x, y), mix_seed(recipe.layout_seed, salt, number), radial, "pocket" if number < pocket_count else "front")
    for size, point, seed, radial in middle:
        add_deposit("Wood", size, point, seed, radial, "middle")
    for kind, tier, point in middle_minerals:
        add_deposit(kind, tier, point, 0, (1.0, 0.0), "middle")
    PLANS[key] = result
    return result


def obstacles(recipe):
    """The ridges (common.spawn_ridges), the belt and the pockets' walls with their rocks and volumes, the gap markers; the number
    of actors spawned."""
    result = plan(recipe)
    count = common.spawn_ridges(recipe)
    rock = common.matte_instance("MI_Rock", common.ROCK_COLOR)
    count += common.spawn_rock_wall(result.rocks, result.volumes, rock, "Wall")
    for label, x, y in result.markers:
        common.spawn(unreal.TargetPoint, unreal.Vector(x, y, 0), label=label)
    return count + len(result.markers)


def avoid_points(recipe):
    """The rocks' places, kept clear of scenery (common.spawn_decor)."""
    return [(rock[0], rock[1]) for rock in plan(recipe).rocks]


def sources(recipe):
    """The sources of the match (D297), as AHOLSourceLayout data: the bases, then the pockets' contents; groups per player."""
    result = plan(recipe)
    return result.sources, len(result.items)


def relays(recipe):
    """R2 (provisional level design): one relay between every two neighbouring places, clear of the rocks of the belt and the pockets' walls
    (common.ring_relays, avoid = avoid_points)."""
    # The piles of gold and stone too: make_map drops what falls in a zone, and the budget per territory (Map.Table) must stay whole.
    veins = [(x, y) for type_name, x, y, _ in plan(recipe).points if type_name not in ("Tree", "BerryBush")]
    return common.ring_relays(recipe, avoid=avoid_points(recipe), keep_whole=veins)


def tree_floor(recipe):
    """The trees every territory must still hold once the relays' zones have taken theirs (common.lay_relays_and_sources): the budget."""
    return steppe.budget_of(recipe)[0]
