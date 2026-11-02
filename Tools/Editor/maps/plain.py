"""The recipe of Plaine (D256, J1; TC2): what Plain adds to the shared pieces of maps/common.py (ground, light, bounds,
navmesh, start slots, each player's base). Made by create_match_map.py as /Game/Maps/Plain_<Size>, and without a size as
/Game/Maps/Plain, the fixed test map (D494).

  obstacles    so that the paths show (D286, J4): 30 m in front of each slot, towards the middle, a ridge of three
               boulders, no navmesh on their tops (a NavArea_Null volume each); in the middle of the map, a grove of
               trees (the units walk between the trunks), with rings of trees around it on the larger sizes.
               Their matte materials /Game/Rendering/MI_Rock and MI_Tree are made when missing.
  sources      the fixed test map: the bases (common.base_sources). A sized map holds the standard budget (D564, D576):
               the organic base, then each territory's packs of trees, berry patches and veins of several sizes (see
               sources). Uniform ground (no GROUND: the grey M_MatteGround), no scenery.
"""
import math
import os

import unreal

from maps import common
from maps.common import spawn_shape, matte_instance, placed_source

# Obstacles (D286): the ridges are the shared piece common.spawn_ridges; the grove is Plaine's own.
# Grove: trunks of 5 m, crowns from 4 m up (the units pass under them, the trunks block), on a disc of 14 m.
GROVE_TREES = [(0.0, 0.0), (700.0, 250.0), (-500.0, 600.0), (-650.0, -400.0), (250.0, -750.0), (1200.0, -300.0),
               (-1250.0, 100.0), (400.0, 1150.0), (-200.0, -1300.0), (1000.0, 900.0), (-1000.0, -1000.0), (-900.0, 1150.0)]
# Sized maps: one ring of trees round the grove for every GROVE_RING_PER of half side past Plain's, GROVE_RING_SPACING
# apart from GROVE_FIRST_RING, the trees ~GROVE_TREE_GAP apart (a multiple of the slots: symmetric).
GROVE_RING_PER = 6000.0
GROVE_FIRST_RING = 2100.0
GROVE_RING_SPACING = 700.0
GROVE_TREE_GAP = 700.0
TRUNK_COLOR = unreal.LinearColor(0.22, 0.15, 0.09, 1.0)
CROWN_COLOR = unreal.LinearColor(0.10, 0.22, 0.08, 1.0)

# The budgeted sources of the sized maps (D564, D576: Plaine holds the standard budget like the other maps). Slot 0's territory is
# drawn once, then turned onto every other slot (symmetric: the fairness of Map.Table is exact). The base is the organic one (two packs
# of BASE_PACKS trees and the berry patch), then the minerals (the near ones 60 m from the slot, where the former veins stood), then
# packs of PACK_SIDE x PACK_SIDE trees (a grid with the jitter of common.WOOD_JITTER, turned by a random yaw), berry patches and the far
# veins, each thrown on the cell of a 6 m grid of the territory nearest to the distance asked of it (growing outwards), CLEAR from
# everything else.
BUDGET_TREES = {"default": 300, "Huge": int(os.environ.get("HOL_HUGE_TREES", "360"))}  # HOL_HUGE_TREES=450 (the formula 1.5 x 300): the same setting as the Steppe's
BUDGET_BERRIES = {"default": 15, "Huge": 23}
BASE_PACKS = [16, 16]
PACK_SIDE = 4
PACK_SPACING = 140.0  # cm between neighbouring trunks (2026-10-31, the user: "trees that touch"; it was 170): the trunks (60 cm radius) touch at 120, the jitter (PACK_JITTER of common.WOOD_JITTER) never takes two below it
PACK_JITTER = 0.35
BERRY_PATCHES = {"default": [5, 4], "Huge": [6, 6, 5]}  # on top of the base's 6 bushes: 15 and 23 asked
MINERAL_RADIUS = {"Small": 150.0, "Medium": 220.0, "Standard": 300.0, "Big": 420.0}
CLEAR = 800.0
CANDIDATE_STEP = 600.0
BORDER = 2400.0  # a thing is nearer its own slot than any other by this much (12 m from the border of its territory)
EDGE = 3000.0
SLOT_KEEP = 5000.0
CENTRE_MARGIN = 1500.0
FIRST_DISTANCE = 6500.0


def grove_trees(recipe):
    """The grove's trees: Plain's disc, and on the sized maps rings round it, more of them the larger the map."""
    trees = list(GROVE_TREES)
    if recipe.size_key is None:
        return trees
    places = len(recipe.angles)
    for ring in range(int((recipe.bounds_half - common.BOUNDS_HALF_SIZE) / GROVE_RING_PER)):
        radius = GROVE_FIRST_RING + ring * GROVE_RING_SPACING
        count = places * max(1, int(math.ceil(2 * math.pi * radius / GROVE_TREE_GAP / places)))
        for number in range(count):
            # Every other ring turned by half a gap: no straight way through.
            radians = math.radians(common.FIRST_SLOT_ANGLE + (number + 0.5 * (ring % 2)) * 360.0 / count)
            trees.append((radius * math.cos(radians), radius * math.sin(radians)))
    return trees


def obstacles(recipe):
    """The ridges and the grove; the number of actors spawned."""
    ridges = common.spawn_ridges(recipe)
    trunk = matte_instance("MI_Trunk", TRUNK_COLOR)
    crown = matte_instance("MI_Crown", CROWN_COLOR)
    trees = grove_trees(recipe)
    for number, (x, y) in enumerate(trees):
        # The engine's 1 m cylinder is centred: a 5 m trunk stands on the ground at Z = 250 cm.
        spawn_shape("/Engine/BasicShapes/Cylinder", unreal.Vector(x, y, 250), unreal.Vector(0.8, 0.8, 5.0), "Trunk%d" % number, trunk)
        spawn_shape("/Engine/BasicShapes/Sphere", unreal.Vector(x, y, 650), unreal.Vector(4.5, 4.5, 3.5), "Crown%d" % number, crown)
    return ridges + 2 * len(trees)


def pack_points(centre, yaw):
    """A pack of PACK_SIDE x PACK_SIDE trees at centre, the grid turned by yaw (radians)."""
    points = []
    for row in range(PACK_SIDE):
        for column in range(PACK_SIDE):
            jitter = common.WOOD_JITTER[(row * PACK_SIDE + column) % len(common.WOOD_JITTER)]
            x = (column - (PACK_SIDE - 1) / 2.0) * PACK_SPACING + jitter[0] * PACK_JITTER
            y = (row - (PACK_SIDE - 1) / 2.0) * PACK_SPACING + jitter[1] * PACK_JITTER
            points.append((centre[0] + x * math.cos(yaw) - y * math.sin(yaw), centre[1] + x * math.sin(yaw) + y * math.cos(yaw)))
    return points


def territory(recipe, base):
    """Slot 0's budgeted sources beyond its base: [(type, x, y)]. base: the placed sources of the organic base (slot 0's are kept clear)."""
    huge = "Huge" if recipe.size_key == "Huge" else "default"
    slots = [recipe.slot(index)[0] for index in range(len(recipe.angles))]
    slot, radial, left = recipe.slot(0)
    own = []
    for source in base:
        at = source.get_editor_property("location")
        if math.dist((at.x, at.y), slot) < 4500.0:
            own.append((at.x, at.y))
    centre_keep = max(math.hypot(x, y) for x, y in grove_trees(recipe)) + CENTRE_MARGIN
    half = recipe.bounds_half - EDGE
    noise = common.Rng(recipe.layout_seed, 31)
    candidates = []
    x = -half
    while x <= half:
        y = -half
        while y <= half:
            near = math.dist((x, y), slot)
            others = min([math.dist((x, y), other) for other in slots[1:]] or [1.0e9])
            ahead = -((x - slot[0]) * radial[0] + (y - slot[1]) * radial[1])
            side = (x - slot[0]) * left[0] + (y - slot[1]) * left[1]
            in_ridge = 1000.0 <= ahead <= 4500.0 and abs(side) <= 2000.0
            if others - near >= BORDER and near >= SLOT_KEEP and math.hypot(x, y) >= centre_keep and not in_ridge:
                candidates.append((x, y, near, noise.uniform() * 1200.0))
            y += CANDIDATE_STEP
        x += CANDIDATE_STEP
    placed = [(px, py, 150.0) for px, py in own]
    items = []

    def put(kind, radius, target_distance=None, target_point=None):
        best = None
        for cx, cy, near, jitter in candidates:
            if any(math.dist((cx, cy), (px, py)) < radius + pr + CLEAR for px, py, pr in placed):
                continue
            score = (math.dist((cx, cy), target_point) if target_point else abs(near - target_distance)) + jitter
            if best is None or score < best[0]:
                best = (score, cx, cy)
        if best is None:
            raise RuntimeError("Plain %s: no room left for a %s in slot 0's territory" % (recipe.size_key, kind))
        placed.append((best[1], best[2], radius))
        return best[1], best[2]

    # The near minerals: stone on the wood's side, gold on the other, round the former veins' place.
    for resource, sign in (("Stone", 1.0), ("Gold", -1.0)):
        target = (slot[0] - radial[0] * common.VEIN_AHEAD + sign * left[0] * common.VEIN_SIDE,
                  slot[1] - radial[1] * common.VEIN_AHEAD + sign * left[1] * common.VEIN_SIDE)
        for tier in common.mineral_near(recipe)[resource]:
            x, y = put(tier, MINERAL_RADIUS[tier], target_point=target)
            items.append((common.mineral_type(resource, tier), x, y))
    # The rest, spread outwards: far veins (the small ones first), berry patches, packs of trees.
    far = [(common.mineral_type(resource, tier), MINERAL_RADIUS[tier], 1) for resource, tier in reversed(common.mineral_far(recipe))]
    patches = [("BerryBush", common.berry_reach(count) * 0.9, count) for count in BERRY_PATCHES[huge]]
    wanted_trees = BUDGET_TREES[huge] - sum(BASE_PACKS)
    packs = [("Tree", 450.0, PACK_SIDE * PACK_SIDE) for _ in range(-(-wanted_trees // (PACK_SIDE * PACK_SIDE)))]
    queue = []
    for kind_list in (far, patches, packs):
        queue += [((index + 0.5) / len(kind_list), kind_list[index]) for index in range(len(kind_list))]
    queue.sort(key=lambda item: item[0])
    distances = sorted(near for _, _, near, _ in candidates)
    last = distances[int(0.8 * len(distances))]
    for number, (_, item) in enumerate(queue):
        target = FIRST_DISTANCE + (last - FIRST_DISTANCE) * (number + 0.5) / len(queue)
        x, y = put(item[0], item[1], target_distance=target)
        if item[0] == "Tree":
            for tx, ty in pack_points((x, y), noise.uniform() * 2 * math.pi):
                items.append(("Tree", tx, ty))
        elif item[0] == "BerryBush":
            for bx, by in common.berry_points(common.mix_seed(recipe.layout_seed, 20 + number), (x, y), item[2]):
                items.append(("BerryBush", bx, by))
        else:
            items.append((item[0], x, y))
    return items


def sources(recipe):
    """The sources of the match (D297), as AHOLSourceLayout data. The fixed test map keeps its base (common.base_sources); a sized map is
    the organic base and slot 0's territory turned onto every slot (BUDGET_*, D564)."""
    if recipe.size_key is None:
        return common.base_sources(recipe), 0
    placed = common.base_sources(recipe, organic=True, wood_packs=BASE_PACKS, veins=False)
    items = territory(recipe, placed)
    for index in range(len(recipe.angles)):
        angle = math.radians(recipe.angles[index] - recipe.angles[0])
        for kind, x, y in items:
            turned = common.turn((x, y), angle)
            placed.append(placed_source(kind, turned[0], turned[1]))
    return placed, len(items)


def relays(recipe):
    """R2 (provisional level design): one relay between every two neighbouring places (common.ring_relays)."""
    return common.ring_relays(recipe)
