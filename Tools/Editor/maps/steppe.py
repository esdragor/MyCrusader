"""The recipe of La Steppe (the chantier Cartes, C1, C1b and C1c; GDD 8.5): what the Steppe adds to the shared pieces of
maps/common.py. An open plain where the resources are MANY SMALL deposits spread over the whole square, corners included:
you expand early and there is no single point to hold. Made by create_match_map.py as /Game/Maps/Steppe_<Size> (no fixed
test map: only Plain has one, D494).

  obstacles    only the ridge of three boulders in front of every slot (S-Q2 = A, D286, D439 to D444: common.spawn_ridges, as
               Plain); no grove, no natural obstacle (M-Q5: the Steppe's principle is the opposite), nothing in the middle.
  sources      a reduced base (S-Q1 = A): a small organic wood of BASE_TREES = 20 trees and a patch of 6 berry bushes, no stone
               and no gold in the base (common.base_sources, organic); then the deposits in two layers.
  ground       the varied ground of the Steppe (GROUND: MI_Ground_Steppe, a muted straw / grass / ochre earth).

The resources hold the budget per player of M-Q4 = A (farms left out): 150 trees, 4 gold veins, 2 stone veins and 15 berry
bushes in each player's territory (the sources whose nearest slot is his), so that nobody runs out before 3/4 of a 30 min
game; the recipe aims a little above for the trees and the berries (targets_of: TREES_MARGIN, BERRIES_MARGIN); the gold and the stone are a VALUE in a
mix of sizes (M-Q9: common.MINERAL_MIX, 14 piles of 500, 1 000, 2 500 or 5 000 per territory, exactly 10 000 gold and 5 000 stone, Huge 15 000 / 7 500).
Tools/Data/Maps.csv says Budget = standard for the Steppe and Map.Table checks it on the saved files at every size.

Nothing is on a grid and nothing is random from one run to the next (C1c): every number comes from a small generator seeded by
recipe.layout_seed (common.Rng; 1 is the layout made now, another seed is a variant of the same map, `py create_match_map.py
Steppe Small seed=3`). The shapes: the groves are irregular blobs (common.grove_points: a ragged outline, some stretched or bent,
trees thrown in with a spacing of 1.6 m, denser in the middle, a few lone trees and clumps outside, a glade or a path of 3 m in
the bigger ones), the berry patches small irregular clusters (common.berry_points), a vein is alone.

The deposits:
  - the NEAR layer, around each slot: one pattern made once, in the frame of slot 0, and turned onto every slot: the slots
    are on a circle, so each one sees the same surroundings to the float (GDD 8.1, D493: the 3 nearest sources of each type
    are the same distance away), the same groves and patches too. It holds the NEAR_VEINS gold veins and NEAR_VEINS stone veins
    the fairness needs (a 3rd nearest that would be a far vein would differ between slots), and the first woods and berries.
    Inside the disc of the bounds' half side, so that turned by any number of places it stays inside the square.
  - the FAR layer, which is NOT the same around every slot: a Poisson disc by dart throwing (FAR_GAP_START, shrinking until every
    territory has what it needs) over the whole bounded square, the corners too, beyond NEAR_REACH of every slot, whose spacing
    varies from place to place (a smooth field, and a few places where deposits crowd together by two or three: density_factor),
    then the holes it left are filled (fill_holes: no point farther than HOLE_CENTRES from a deposit). Every deposit belongs
    to its nearest slot, and the kinds are drawn territory by territory (a seeded shuffle, not a cycle): veins spread over the
    territory, berry patches rather near the slot, and wood on all the rest, the sizes of the groves chosen so that every
    territory comes to its target. The far layer is sparser in a roomier territory (territory_scales), so that all the
    players come to about the same number of deposits.
No point of the square is farther than ~90 m from a source or a slot (tested by Map.Steppe, and checked here on the layout
itself: layout retries with tighter holes until it holds).

The rules, in cm: a deposit is at least DEPOSIT_GAP (18 m) from any other, source to source; at least common.BASE_REACH
(60 m) from every slot, a vein VEIN_FROM_SLOT (110 m); at least CENTRE_CLEAR (20 m) from the map's centre; inside the bounds
with a 15 m margin; the veins of one kind of one territory VEIN_SPREAD (60 m) apart; the far deposits NEAR_REACH from every
slot at least, and the far veins farther than the 3rd nearest near vein of their type (the fairness).
"""
import math

import unreal

from maps import common
from maps.common import placed_source, Rng, Noise, mix_seed

# The ground (an instance of M_GroundVaried, set in maps/ground.py GROUNDS; the parent is made by
# Tools/Editor/make_ground_materials.py).
GROUND = "MI_Ground_Steppe"

# M-Q10 (the user, 2026-10-30): no scenery (DECOR) and no clumps (CLUMPS): nothing without a strategic, economic or military interest on a map
# without water; the empty-circle rule (M-Q7) is held by the resources and the rocks alone.

# S-Q1 = A: the base's wood, 4 x 5 = 20 trees (Plain: 40), its 6 berry bushes, and no veins.
BASE_WOOD_ROWS = 4
BASE_WOOD_COLUMNS = 5
BASE_TREES = BASE_WOOD_ROWS * BASE_WOOD_COLUMNS
BASE_PACKS = [common.PACK_TREES, BASE_TREES - common.PACK_TREES]  # M-Q8: the base's 20 trees are a pack of 14 and a small one of 6
BASE_BERRIES = common.BERRY_ROWS * common.BERRY_COLUMNS

# The budget per player (M-Q4 = A, M-Q4b: from Small to Large 150 trees and 15 berry bushes; Huge, whose room per player is the same but
# whose games are longer, 1.5 times that), and what a territory comes to: a margin above it. The gold and the stone are a VALUE budget in
# a mix of sizes (M-Q9: common.MINERAL_MIX, 10 000 gold and 5 000 stone, Huge 15 000 / 7 500), exact, with no margin. Map.Table checks the
# budget with the same numbers (HOLMapTableTests.cpp).
BUDGET = {"default": (300, 15), "Huge": (360, 23)}  # M-Q10b (the user: "double the trees"): 300, it was 150; Huge 360 (225 doubled = 450 does not fit: the far layer of a Huge territory holds 23 packs at the least, 360 is the most it holds)
TREES_MARGIN = 1.04
BERRIES_MARGIN = 1.2
NEAR_VEINS = 3  # of each resource in the near layer: the fairness reads the 3 nearest (their sizes: common.MINERAL_NEAR)

# The deposits' shapes. A grove is GROVE_LADDER trees (common.grove_points), by size; a berry patch is BERRY_SIZES bushes
# (common.berry_points); a vein is alone. GROVE_CYCLE: the sizes first given to the groves of a territory (shuffled), then
# adjusted up or down to the territory's target.
# M-Q8 (the user: "make the forests packs of 14 trees"): every grove is a PACK of 14 trees, 12 or 16 where the room asks for it.
GROVE_LADDER = [12, 14, 16, 18]  # M-Q10b: 18 for Huge, whose territories hold few points for 360 trees
GROVE_CYCLE = [14, 14, 12, 14, 16, 14, 14, 12, 14, 16]
BERRY_SIZES = [4, 3, 5]

# Distances (cm).
DEPOSIT_GAP = 1800.0  # M-Q10b: it was 25 m; with tighter packs and twice as many, 18 m between two deposits
VEIN_GAP = 1800.0  # M-Q9: between a pile and another deposit (source to source): the piles are small bodies and a territory holds 14 of them
VEIN_FROM_SLOT = 11000.0
VEIN_SPREAD = 4000.0  # M-Q9: veins of one resource this far apart (was 60 m with 4 and 2 veins a territory; now 14 piles, many of them small)
CENTRE_CLEAR = 2000.0
EDGE_MARGIN = 1500.0
NEAR_REACH = 7500.0  # no far deposit's source closer than this to a slot: the near layer is the same around every slot
FAR_GAP_START = 8000.0  # between the centres of two deposits of the far layer (the smallest territory's), thrown at first
FAR_GAP_STEP = 300.0
FAR_GAP_MIN = 2700.0  # 27 m (M-Q10b: it was 33 m): the deposits' sizes are capped by the room they have, so that DEPOSIT_GAP is kept between sources
GRID_STEP = 1000.0  # the grid that measures a territory's room
GROVE_MEANS = [14.0, 15.0, 16.0, 17.0, 18.0]  # the mean grove a territory is asked to be comfortable with, the smallest that works
BERRY_FROM_BERRY = 12000.0  # a territory's berry patches are spread at least this far apart
VEIN_SLACK = 100.0  # a far vein is this much farther than the 3rd nearest near vein of its type
NEAR_VEIN_END = 13000.0  # and at least this far from every slot: the veins within it are all near ones (tested by Map.Steppe)

# The irregular far layer (C1c). The spacing of the far deposits is the gap times the territory's scale times density_factor: a
# smooth noise of FIELD_AMPLITUDE (the gap varies by about a quarter: wide airy stretches and tighter ones), and one crowding
# place per DIP_PER_AREA of the square (a dip of DIP_DEPTH of the spacing over DIP_SIGMA: two or three deposits close together,
# still DEPOSIT_GAP apart from source to source: the sizes fit the room).
FIELD_AMPLITUDE = 0.28
DIP_PER_AREA = 7.0e8  # cm2: 7 ha
DIP_DEPTH = 0.42
DIP_SIGMA = 2600.0
# No point of the square (grid of EMPTY_STEP, as Map.Steppe measures it) farther than HOLE_LIMIT from a source or a slot: the far
# layer's holes are filled down to the first of HOLE_CENTRES (from a deposit's centre) that makes the measured one at most that.
# M-Q8: the scenery fills the rest (M-Q7: common.fill_holes, 35 m); the deposits are the budget in packs of 14, not more of them to cover the square.
HOLE_CENTRES = [12000.0, 11000.0, 10000.0, 9000.0]
HOLE_LIMIT = 14000.0
EMPTY_STEP = 2000.0
# The far veins are placed first, one by one: the best of VEIN_CHOICE places drawn, the one farthest from the territory's veins.
VEIN_CHOICE = 5
VEIN_DRAWS = 4000

# The near layer: (kind, size, nearest, farthest) from the slot, in order (the veins first, the hardest to place); the veins
# end before NEAR_VEIN_END. Each item draws its position from its own seeded generator.
NEAR = [("GoldVein", 1, 11000.0, 12500.0), ("StoneVein", 1, 11000.0, 12500.0), ("GoldVein", 1, 11000.0, 12500.0),
        ("StoneVein", 1, 11000.0, 12500.0), ("GoldVein", 1, 11000.0, 12500.0), ("StoneVein", 1, 11000.0, 12500.0),
        ("Wood", 14, 7000.0, 10500.0), ("BerryBush", 4, 7000.0, 10500.0), ("Wood", 14, 7500.0, 10500.0),
        ("Wood", 14, 7500.0, 10500.0)]
MAX_ATTEMPTS = 3000
FAR_CANDIDATES = 40000

# Salts of the generators: one family of numbers each, so that changing one never shifts another.
SALT_NEAR, SALT_NEAR_SHAPE, SALT_NOISE, SALT_DIPS, SALT_FAR, SALT_KINDS, SALT_SHAPE, SALT_FRAME, SALT_FILL, SALT_VEINS = range(101, 111)


def group_points(kind, size, at, radial, seed, reach=None):
    """The places of one deposit's sources, centred on at, in the frame whose first axis is radial: an organic grove of size
    trees (common.grove_points), an irregular patch of size berry bushes (common.berry_points), or one vein. seed draws the
    shape: the same seed gives the same shape, whatever the frame."""
    if kind == "Wood":
        return common.grove_points(seed, at, size, radial, reach)
    if kind == "BerryBush":
        return common.berry_points(seed, at, size, radial)
    return [at]


def extent(kind, size):
    """How far a source of the deposit may be from its centre, whatever its seed and frame (the generators keep to it)."""
    if kind == "Wood":
        return common.grove_reach(size)
    if kind == "BerryBush":
        return common.berry_reach(size)
    return 0.0


def near_room(kind, size):
    """What a near deposit is given: its reach is more than a far one's guaranteed (the near layer is placed with this much room
    round it), so that its groves stretch, bend and have glades like the others."""
    return min(common.grove_long(size), 1.45 * extent(kind, size)) if kind == "Wood" else extent(kind, size)


MIN_EXTENT = extent("Wood", GROVE_LADDER[0])  # the smallest grove: a far centre is placed with room for this much, the biggest ones are fitted to the room they find (room_caps)
# Two far centres are never closer than this: the smallest grove's reach twice and DEPOSIT_GAP between (the crowded places of the
# far layer come down to it, never under).
FAR_FLOOR = DEPOSIT_GAP + 2 * MIN_EXTENT + 10.0


def rotate(point, angle):
    return (point[0] * math.cos(angle) - point[1] * math.sin(angle), point[0] * math.sin(angle) + point[1] * math.cos(angle))


class Deposit:
    """One deposit: its kind (Wood, BerryBush, GoldVein, StoneVein), its size (a vein's: the tier name of common.MINERAL_AMOUNTS), its centre, its frame (a unit vector
    radial), the seed of its shape and the room it was given (how far its farthest source may be from its centre: a grove stretches
    or bends when it has the room, never beyond; None = the reach every grove of its size keeps to)."""

    def __init__(self, kind, size, at, radial, seed, room=None):
        self.kind = kind
        self.size = size
        self.at = at
        self.radial = radial
        self.seed = seed
        self.room = room
        self.cached = None

    @property
    def reach(self):
        return extent(self.kind, self.size) if self.room is None else self.room

    @property
    def is_vein(self):
        return self.kind.endswith("Vein")

    def points(self):
        if self.cached is None:
            reach = None
            if self.kind == "Wood" and self.room is not None:
                reach = max(self.reach, min(self.room, common.grove_long(self.size)))
            self.cached = group_points(self.kind, self.size, self.at, self.radial, self.seed, reach)
        return self.cached

    @property
    def source_type(self):
        """The source type of the deposit's sources: a tree, a berry bush, or the vein of its resource and size (size = the tier name, M-Q9)."""
        if self.kind == "Wood":
            return "Tree"
        return common.mineral_type(self.kind[:-len("Vein")], self.size) if self.is_vein else self.kind

    def sources(self):
        return [placed_source(self.source_type, x, y) for x, y in self.points()]


def nearest(point, slots):
    """The index of the nearest slot of a point (ties to the lowest index)."""
    best = 0
    for index in range(1, len(slots)):
        if math.dist(point, slots[index]) < math.dist(point, slots[best]):
            best = index
    return best


def slot_positions(recipe):
    return [recipe.slot(index)[0] for index in range(len(recipe.angles))]


def near_layer(recipe):
    """The near layer: the pattern of NEAR made in the frame of slot 0 (a point at a distance and an angle off the
    direction ahead, towards the centre, drawn from the seed) and turned onto every slot, each copy with the same grove or
    patch shape. Returns the deposits of all the slots, slot by slot. Every distance of the module's docstring holds between the
    points of all the slots."""
    places = len(recipe.angles)
    slots = slot_positions(recipe)
    step = 2 * math.pi / places
    disc = recipe.bounds_half - EDGE_MARGIN  # inside it whatever the number of places
    ahead = (-recipe.slot(0)[1][0], -recipe.slot(0)[1][1])  # slot 0's direction towards the centre
    placed = []  # (kind, size, copies as xy points)
    pattern = []  # (kind, size, point of slot 0, seed of the shape)
    tiers = {resource + "Vein": iter(common.mineral_near(recipe)[resource]) for resource in common.MINERAL_RESOURCES}  # M-Q9: the veins' sizes, in order
    for number, (kind, size, nearest_from, farthest_from) in enumerate(NEAR):
        if kind.endswith("Vein"):
            size = next(tiers[kind])
        rng = Rng(recipe.layout_seed, SALT_NEAR, number)
        reach = near_room(kind, size)
        slot_gap = max(VEIN_FROM_SLOT if kind.endswith("Vein") else common.BASE_REACH + reach, nearest_from)
        for attempt in range(MAX_ATTEMPTS):
            u, v = rng.uniform(), rng.uniform()
            distance = math.sqrt(nearest_from ** 2 + u * (farthest_from ** 2 - nearest_from ** 2))
            angle = (v - 0.5) * 2 * math.pi
            direction = rotate(ahead, angle)
            point = (slots[0][0] + direction[0] * distance, slots[0][1] + direction[1] * distance)
            if math.hypot(*point) > disc - reach or math.hypot(*point) < CENTRE_CLEAR + reach:
                continue
            copies = [rotate(point, turn * step) for turn in range(places)]
            if any(math.dist(copy, slot) < slot_gap for copy in copies for slot in slots):
                continue
            fits = True
            for other_kind, other_size, other_copies in placed + [(kind, size, copies[1:])]:
                gap = DEPOSIT_GAP + reach + near_room(other_kind, other_size)
                if kind.endswith("Vein") and kind == other_kind:
                    gap = max(gap, VEIN_SPREAD)
                if any(math.dist(copies[0], other) < gap for other in other_copies):
                    fits = False
                    break
            if fits:
                placed.append((kind, size, copies))
                pattern.append((kind, size, point, mix_seed(recipe.layout_seed, SALT_NEAR_SHAPE, number)))
                break
        else:
            raise RuntimeError("Steppe: no place for near deposit %d (%s) in %d attempts" % (number, kind, MAX_ATTEMPTS))
    deposits = []
    for index in range(places):
        _, radial, _ = recipe.slot(index)
        for kind, size, point, seed in pattern:
            # The frame of the deposit: the slot's own (radial outward), as in the base: the same shape at every slot.
            deposits.append(Deposit(kind, size, rotate(point, index * step), radial, seed, near_room(kind, size)))
    return deposits


def budget_of(recipe):
    """(trees, berries) of the budget for a recipe's size (BUDGET); the gold and the stone are a value, common.mineral_value."""
    return BUDGET.get(recipe.size_key, BUDGET["default"])


def targets_of(recipe):
    """What each territory comes to, a margin above the budget: (trees, berries)."""
    trees, berries = budget_of(recipe)
    return math.ceil(trees * TREES_MARGIN), math.ceil(berries * BERRIES_MARGIN)


def territory_scales(recipe, slots):
    """For each slot, the square root of the room of its territory (the points of a grid GRID_STEP apart, in the bounds,
    that are its nearest and far from the slots) over the smallest one: the far layer is sparser in a roomier territory,
    so that every player comes to about the same number of deposits whatever the shape of its territory."""
    half = recipe.bounds_half - EDGE_MARGIN
    steps = int(2 * half // GRID_STEP)
    rooms = [0] * len(slots)
    for i in range(steps + 1):
        for j in range(steps + 1):
            point = (-half + i * GRID_STEP, -half + j * GRID_STEP)
            if all(math.dist(point, slot) >= NEAR_REACH for slot in slots):
                rooms[nearest(point, slots)] += 1
    return [math.sqrt(room / min(rooms)) for room in rooms]


def density_factor(recipe):
    """The factor the far layer's spacing is multiplied by at a point: 1 plus a smooth noise of FIELD_AMPLITUDE, times a dip at each
    crowding place (a few drawn over the square). A function of (x, y) with values in about 0.55 to 1.3."""
    noise = Noise(mix_seed(recipe.layout_seed, SALT_NOISE))
    rng = Rng(recipe.layout_seed, SALT_DIPS)
    half = recipe.bounds_half - EDGE_MARGIN
    dips = [(rng.range(-half, half), rng.range(-half, half)) for _ in range(max(3, int((2 * half) ** 2 / DIP_PER_AREA)))]

    def factor(x, y):
        value = 1.0 + FIELD_AMPLITUDE * noise.value(x, y)
        for dip_x, dip_y in dips:
            value *= 1.0 - DIP_DEPTH * math.exp(-((x - dip_x) ** 2 + (y - dip_y) ** 2) / (2 * DIP_SIGMA ** 2))
        return value

    return factor


def far_allowed(recipe, slots, point):
    """Whether a far deposit's centre may be at point (the rules of the module's docstring that depend neither on its kind nor on
    its neighbours): inside the bounds, away from the centre of the map and from the slots."""
    half = recipe.bounds_half - EDGE_MARGIN - MIN_EXTENT
    if abs(point[0]) > half or abs(point[1]) > half:
        return False
    if math.hypot(*point) < CENTRE_CLEAR + MIN_EXTENT:
        return False
    return not any(math.dist(point, slot) < NEAR_REACH + MIN_EXTENT for slot in slots)


def far_veins(recipe, near):
    """The far veins (M-Q9: common.mineral_far, the piles of the mix beyond the near layer: a few big ones, a medium, many small ones), placed
    before every other far deposit, territory by territory (each its own seeded draws: the territories do NOT look alike, M-Q6), biggest
    first, every territory with the same list, so that the value budget is exact. Each pile is at the best of VEIN_CHOICE places drawn that keep
    the rules: farther than the 3rd nearest near vein of its resource from every slot (the fairness, the near layer being the same around every
    slot), inside the margin, off the centre, VEIN_GAP from every other deposit, VEIN_SPREAD from the others of its resource; a big pile prefers
    the far reaches of the territory (far from the slot and from the other piles), the others spread out (so some land near the middle)."""
    slots = slot_positions(recipe)
    edge = recipe.bounds_half - EDGE_MARGIN
    third = {}
    for resource in common.MINERAL_RESOURCES:
        kind = resource + "Vein"
        distances = sorted(math.dist(deposit.at, slots[0]) for deposit in near if deposit.kind == kind)
        third[kind] = max(distances[NEAR_VEINS - 1] + VEIN_SLACK, NEAR_VEIN_END, VEIN_FROM_SLOT)
    placed = list(near)
    for index in range(len(slots)):
        rng = Rng(recipe.layout_seed, SALT_VEINS, index)
        for number, (resource, tier) in enumerate(common.mineral_far(recipe)):
            kind = resource + "Vein"
            best = None
            valid = 0
            for _ in range(VEIN_DRAWS):
                point = (rng.range(-edge, edge), rng.range(-edge, edge))
                if nearest(point, slots) != index or math.hypot(*point) < CENTRE_CLEAR:
                    continue
                if any(math.dist(point, slot) < third[kind] for slot in slots):
                    continue
                if any(math.dist(point, deposit.at) < VEIN_GAP + deposit.reach for deposit in placed):
                    continue
                if any(math.dist(point, deposit.at) < VEIN_SPREAD for deposit in placed if deposit.kind == kind):
                    continue
                own = [deposit.at for deposit in placed if deposit.is_vein and nearest(deposit.at, slots) == index]
                apart = min([math.dist(point, other) for other in own] + [1.0e9])
                score = math.dist(point, slots[index]) + 0.5 * apart if tier == "Big" else apart
                if best is None or score > best[0]:
                    best = (score, point)
                valid += 1
                if valid >= VEIN_CHOICE:
                    break
            if best is None:
                raise RoomError("Steppe: no place for a far %s %s pile (number %d) in the territory of slot %d" % (tier, kind, number, index))
            placed.append(Deposit(kind, tier, best[1], (1.0, 0.0), 0))
    return placed[len(near):]


def far_points(recipe, near, veins, gap, scales, factor):
    """The centres of the far layer: a Poisson disc by dart throwing over the whole bounded square, the darts drawn from the seed.
    The distance between two centres is gap, scaled by the territories' scales and by density_factor (the mean of the two
    points')."""
    slots = slot_positions(recipe)
    half = recipe.bounds_half - EDGE_MARGIN - MIN_EXTENT
    cell = gap * max(scales) * 1.35
    grid = {}
    rng = Rng(recipe.layout_seed, SALT_FAR, int(gap))

    def crowded(point, own):
        cx, cy = int(point[0] // cell), int(point[1] // cell)
        return any(math.dist(point, other) < 0.5 * (own + other_gap) for dx in (-1, 0, 1) for dy in (-1, 0, 1)
                   for other, other_gap in grid.get((cx + dx, cy + dy), ()))

    near_gaps = [(deposit.at, (VEIN_GAP if deposit.is_vein else DEPOSIT_GAP) + deposit.reach + MIN_EXTENT) for deposit in near + veins]
    points = []
    for _ in range(FAR_CANDIDATES):
        point = ((2 * rng.uniform() - 1) * half, (2 * rng.uniform() - 1) * half)
        if not far_allowed(recipe, slots, point):
            continue
        own = max(FAR_FLOOR, gap * scales[nearest(point, slots)] * factor(*point))
        if crowded(point, own):
            continue
        if any(math.dist(point, at) < gap_near for at, gap_near in near_gaps):
            continue
        grid.setdefault((int(point[0] // cell), int(point[1] // cell)), []).append((point, own))
        points.append(point)
    return points


class Coverage:
    """The grid Map.Steppe measures the largest empty circle on (EMPTY_STEP, from the corner of the bounds): the distance of each
    of its points to the nearest point added. Distances beyond `window` are not tracked (they read as infinity)."""

    def __init__(self, recipe, window):
        half = recipe.bounds_half
        self.low = -half
        self.cells = int((2 * half + 1.0) // EMPTY_STEP) + 1
        self.reach = int(window // EMPTY_STEP) + 1
        self.distance = [[math.inf] * self.cells for _ in range(self.cells)]

    def cell_point(self, i, j):
        return (self.low + i * EMPTY_STEP, self.low + j * EMPTY_STEP)

    def add(self, point):
        i0 = int((point[0] - self.low) // EMPTY_STEP)
        j0 = int((point[1] - self.low) // EMPTY_STEP)
        for i in range(max(0, i0 - self.reach), min(self.cells, i0 + self.reach + 2)):
            column = self.distance[i]
            x = self.low + i * EMPTY_STEP
            for j in range(max(0, j0 - self.reach), min(self.cells, j0 + self.reach + 2)):
                d = math.hypot(x - point[0], self.low + j * EMPTY_STEP - point[1])
                if d < column[j]:
                    column[j] = d

    def largest(self):
        return max(max(column) for column in self.distance)


def fill_holes(recipe, fixed, points, hole):
    """The far layer's centres plus those added where the square is emptier than hole (cm from a slot, a near deposit's source or a
    deposit's centre; fixed: the near layer and the far veins): at the emptiest allowed grid point, a little shifted, until none is."""
    slots = slot_positions(recipe)
    coverage = Coverage(recipe, 10000.0)
    for point in slots + [p for deposit in fixed for p in deposit.points()] + points:
        coverage.add(point)
    rng = Rng(recipe.layout_seed, SALT_FILL)
    filled = list(points)
    while True:
        best = None
        for i in range(coverage.cells):
            for j in range(coverage.cells):
                d = coverage.distance[i][j]
                if d > hole and (best is None or d > best[0]) and far_allowed(recipe, slots, coverage.cell_point(i, j)):
                    best = (d, coverage.cell_point(i, j))
        if best is None:
            return filled
        point = best[1]
        for _ in range(20):  # a little off the grid point when it may
            shifted = (point[0] + rng.range(-700.0, 700.0), point[1] + rng.range(-700.0, 700.0))
            if far_allowed(recipe, slots, shifted):
                point = shifted
                break
        filled.append(point)
        coverage.add(point)


def needed_points(recipe, owned_near, grove_mean):
    """How many far deposits (not counting the far veins, placed first) a territory needs to be comfortable: its berry patches, and
    wood groves of about grove_mean trees for the rest of its trees."""
    trees, berries = targets_of(recipe)
    patches = max(0, math.ceil((berries - BASE_BERRIES - sum(d.size for d in owned_near if d.kind == "BerryBush")) / 4.0))
    wanted = trees - BASE_TREES - sum(d.size for d in owned_near if d.kind == "Wood")
    return patches + math.ceil(wanted / grove_mean)


class RoomError(RuntimeError):
    """The far layer, as thrown, cannot hold the territory's kinds: layout throws it again, denser or with bigger groves."""


def room_caps(recipe, fixed, points):
    """For each far point, how far its deposit's farthest source may be from its centre so that DEPOSIT_GAP holds from
    source to source with every other deposit, whatever the other's size: half the distance to the nearest other far
    centre (the other takes the same half), the whole distance to a near deposit less its own extent; and so that it stays
    inside the bounds' margin, off the centre of the map and NEAR_REACH from every slot."""
    slots = slot_positions(recipe)
    edge = recipe.bounds_half - EDGE_MARGIN
    caps = []
    for index, point in enumerate(points):
        room = min([0.5 * (math.dist(point, other) - DEPOSIT_GAP) for other_index, other in enumerate(points) if other_index != index]
                   + [math.dist(point, deposit.at) - deposit.reach - (VEIN_GAP if deposit.is_vein else DEPOSIT_GAP) for deposit in fixed]
                   + [edge - abs(point[0]), edge - abs(point[1]), math.hypot(*point) - CENTRE_CLEAR]
                   + [math.dist(point, slot) - NEAR_REACH for slot in slots])
        caps.append(room)
    return caps


def fit_size(kind, size, cap):
    """The biggest size of the kind that is at most size and whose extent is within cap; the smallest one if none is."""
    sizes = sorted(GROVE_LADDER) if kind == "Wood" else sorted(BERRY_SIZES)
    fitting = [s for s in sizes if s <= size and extent(kind, s) <= cap]
    return fitting[-1] if fitting else sizes[0]


def give_kinds(recipe, fixed, points):
    """The far layer's deposits (fixed: the near layer and the far veins): its points given to their territories, then the kinds
    territory by territory, drawn from the seed (the berry patches picked among the best candidates, not the single best, the groves'
    sizes shuffled)."""
    slots = slot_positions(recipe)
    places = len(slots)
    trees_target, berries_target = targets_of(recipe)
    territories = [[] for _ in range(places)]
    for index, point in enumerate(points):
        territories[nearest(point, slots)].append((index, point))
    caps = room_caps(recipe, fixed, points)
    far = []
    for index in range(places):
        rng = Rng(recipe.layout_seed, SALT_KINDS, index)
        free = list(territories[index])
        own = [d for d in fixed if nearest(d.at, slots) == index]
        chosen = []  # (point index, point, kind, size)

        def take(kind, score, least=1):
            """One of the best free points by score, drawn among the best `least` of them."""
            scored = sorted(((score(entry[1]), entry) for entry in free), key=lambda item: (-item[0], item[1][0]))
            if not scored:
                raise RoomError("Steppe: no room left for a %s in the territory of slot %d (%d points)" % (kind, index, len(territories[index])))
            entry = scored[rng.below(min(len(scored), least))][1]
            free.remove(entry)
            return entry

        # Berry patches: rather near the slot, spread BERRY_FROM_BERRY from the territory's others when it can, one of the best three.
        berries = [d.at for d in own if d.kind == "BerryBush"] + [slots[index]]
        have = BASE_BERRIES + sum(d.size for d in own if d.kind == "BerryBush")
        sizes_cycle = rng.shuffle(list(BERRY_SIZES))
        number = 0
        while have < berries_target:
            apart = lambda point: min(math.dist(point, other) for other in berries)
            entry = take("BerryBush", lambda point: -math.dist(point, slots[index]) if apart(point) >= BERRY_FROM_BERRY else apart(point) - 1.0e9, 3)
            size = fit_size("BerryBush", sizes_cycle[number % len(sizes_cycle)], caps[entry[0]])
            number += 1
            berries.append(entry[1])
            have += size
            chosen.append((entry[0], entry[1], "BerryBush", size))
        # Wood on all the rest: the sizes of the groves, first a shuffled cycle, then to the territory's target.
        wanted = trees_target - BASE_TREES - sum(d.size for d in own if d.kind == "Wood")
        # M-Q8: the points the territory has beyond the packs of PACK_TREES its budget needs are let go (the scenery fills them, M-Q7), the most
        # crowded first (the nearest to another point of the territory), so that the packs stay 14 and the trees stay at the budget.
        while len(free) > math.ceil(wanted / common.PACK_TREES):
            others = lambda entry: [o[1] for o in free if o is not entry] + [c[1] for c in chosen] + [d.at for d in own]
            free.remove(min(free, key=lambda entry: min(math.dist(entry[1], o) for o in others(entry))))
        free.sort()
        cycle = rng.shuffle(list(GROVE_CYCLE))
        sizes = [fit_size("Wood", cycle[number % len(cycle)], caps[entry[0]]) for number, entry in enumerate(free)]
        while sum(sizes) < wanted:
            upgrades = [i for i, s in enumerate(sizes)
                        if s < GROVE_LADDER[-1] and extent("Wood", GROVE_LADDER[GROVE_LADDER.index(s) + 1]) <= caps[free[i][0]]]
            if not upgrades:
                raise RoomError("Steppe: territory of slot %d cannot hold %d trees in %d groves" % (index, wanted, len(free)))
            smallest = min(sizes[i] for i in upgrades)
            candidates = [i for i in upgrades if sizes[i] == smallest]
            number = candidates[rng.below(len(candidates))]
            sizes[number] = GROVE_LADDER[GROVE_LADDER.index(sizes[number]) + 1]
        while True:
            biggest = max((s for s in sizes if s > GROVE_LADDER[0]), default=None)
            if biggest is None:
                break
            lower = GROVE_LADDER[GROVE_LADDER.index(biggest) - 1]
            if sum(sizes) - (biggest - lower) < wanted:
                break
            candidates = [i for i, s in enumerate(sizes) if s == biggest]
            sizes[candidates[rng.below(len(candidates))]] = lower
        for (point_index, point), size in zip(free, sizes):
            chosen.append((point_index, point, "Wood", size))
        for point_index, point, kind, size in sorted(chosen):
            # A frame turned by a drawn angle, and the shape's own seed: no two groves alike.
            angle = 2 * math.pi * Rng(recipe.layout_seed, SALT_FRAME, point_index).uniform()
            far.append(Deposit(kind, size, point, (math.cos(angle), math.sin(angle)), mix_seed(recipe.layout_seed, SALT_SHAPE, point_index),
                               caps[point_index]))
    return far


def base_sources(recipe):
    """The bases' sources, as (type, x, y)."""
    return [(str(source.get_editor_property("type")), source.get_editor_property("location").x, source.get_editor_property("location").y)
            for source in common.base_sources(recipe, BASE_WOOD_ROWS, BASE_WOOD_COLUMNS, veins=False, organic=True, wood_packs=BASE_PACKS)]


def base_points(recipe):
    """The slots and the sources of the bases, as xy: what the largest empty circle counts besides the deposits."""
    return slot_positions(recipe) + [(x, y) for _, x, y in base_sources(recipe)]


def holds_budget(recipe, base, deposits):
    """Whether every territory holds the budget, counting source by source as Map.Table does (a source belongs to the slot nearest
    to it: a grove on the border between two territories gives to both): the trees and the berries by number, the gold and the stone by
    value (M-Q9: exactly the mix's). base: the base's sources as (type, x, y)."""
    slots = slot_positions(recipe)
    counts = [{} for _ in slots]
    sources = base + [(deposit.source_type, x, y) for deposit in deposits for x, y in deposit.points()]
    for kind, x, y in sources:
        owner = nearest((x, y), slots)
        counts[owner][kind] = counts[owner].get(kind, 0) + 1
    trees, berries = budget_of(recipe)
    needs = {"Tree": trees, "BerryBush": berries}
    amounts = {common.mineral_type(resource, tier): amount for resource in common.MINERAL_RESOURCES for tier, amount in common.MINERAL_AMOUNTS.items()}
    for count in counts:
        if any(count.get(kind, 0) < needed for kind, needed in needs.items()):
            return False
        for resource in common.MINERAL_RESOURCES:
            if sum(amount * count.get(kind, 0) for kind, amount in amounts.items() if kind.startswith(resource + "Vein")) != common.mineral_value(recipe, resource):
                return False
    return True


def empty_circle(recipe, points):
    """The largest empty circle of the square (cm), as Map.Steppe measures it: from the nearest of points, on the grid of EMPTY_STEP."""
    coverage = Coverage(recipe, 2 * HOLE_LIMIT)
    for point in points:
        coverage.add(point)
    return coverage.largest()


def layout(recipe):
    """All the deposits of the match beside the bases: the near layer, then the far layer. Deterministic from recipe.layout_seed.
    The far layer is thrown at FAR_GAP_START and the gap shrinks by FAR_GAP_STEP until every territory has the points it needs
    for groves of the smallest mean of GROVE_MEANS that works (many small deposits first); its holes are filled down to
    the first of HOLE_CENTRES that makes the square's largest empty circle at most HOLE_LIMIT."""
    near = near_layer(recipe)
    veins = far_veins(recipe, near)
    fixed = near + veins
    slots = slot_positions(recipe)
    scales = territory_scales(recipe, slots)
    factor = density_factor(recipe)
    owned = [[d for d in near if nearest(d.at, slots) == index] for index in range(len(slots))]
    base = base_points(recipe)
    last = None
    thrown = {}  # gap -> (points, points per territory)

    def throw(gap):
        if gap not in thrown:
            points = far_points(recipe, near, veins, gap, scales, factor)
            counts = [0] * len(slots)
            for point in points:
                counts[nearest(point, slots)] += 1
            thrown[gap] = (points, counts)
        return thrown[gap]

    counts = []
    for grove_mean in GROVE_MEANS:
        needs = [needed_points(recipe, owned[index], grove_mean) for index in range(len(slots))]
        gap = FAR_GAP_START
        while gap >= FAR_GAP_MIN:
            points, counts = throw(gap)
            if all(counts[index] >= needs[index] for index in range(len(slots))):
                for hole in HOLE_CENTRES:
                    try:
                        deposits = fixed + give_kinds(recipe, fixed, fill_holes(recipe, fixed, points, hole))
                    except RoomError as error:
                        last = error
                        break
                    largest = empty_circle(recipe, base + [p for deposit in deposits for p in deposit.points()])
                    if largest <= HOLE_LIMIT and holds_budget(recipe, base_sources(recipe), deposits):
                        return deposits
                    last = RoomError("Steppe: an empty circle of %.0f m with the holes filled to %.0f m, or under the budget" % (largest / 100, hole / 100))
            gap -= FAR_GAP_STEP
    raise RuntimeError("Steppe: the far layer cannot hold the territories (%s at the smallest gap; last: %s)" % (counts, last))


def obstacles(recipe):
    """The ridges (common.spawn_ridges), nothing else; the number of actors spawned."""
    return common.spawn_ridges(recipe)


def sources(recipe):
    """The sources of the match (D297), as AHOLSourceLayout data: the reduced bases, then the deposits, each in turn."""
    placed = common.base_sources(recipe, BASE_WOOD_ROWS, BASE_WOOD_COLUMNS, veins=False, organic=True, wood_packs=BASE_PACKS)
    deposits = layout(recipe)
    for deposit in deposits:
        placed += deposit.sources()
    return placed, len(deposits) // len(recipe.angles)
