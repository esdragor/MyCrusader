"""La Couronne (chantier Cartes, C6; GDD 8.5, D496, D596 = CQ6b = A): every base is a keep behind a ring of cliffs with ONE ramp, and the gold lies in a rich
bowl in the middle of the map. No height is drawn (M-Q2 = A, D567): the cliffs are the rocks of the other maps (common.rock_wall, NavArea_Null under them, D439),
the ramp is a 16 m gap in the ring facing the middle; the bonus of height for the archers (D23) waits for the relief. Registered in Tools/Data/Maps.csv with the
Budget `centre` (the gold is all in the middle, 10 000 per player, 15 000 on Huge, as El Dorado: eldorado.cluster, without its marsh).

The map is the Steppe's (steppe.sources: the same bases, the same near and far layers, the same budget) less its gold, plus the cluster of El Dorado in the
middle, and a RING of cliffs round every place: a closed wall at RING_RADIUS from the slot (just outside the base's reach, common.BASE_REACH), opened by one
RAMP (GATE_WIDTH, 16 m: a formation of 12 m passes) on the ray from the slot to the middle. The places sit at equal angles, so the rings are the same shape turned
onto every place and the fairness of the table stays exact (the deposits' layers are copies of place 0's turned, and a point in a ring's band is forbidden to all).
The deposits (near and far layers) keep WALL_CLEAR off every ring's axis: the base is inside, its wood and berries with it, the stone and the rest outside.
PROVISIONAL LEVEL DESIGN (the GDD says only "bases on plateaus round a rich central bowl, few ramps"): the ring's radius and the ramp's width are settings.
  HOL_CROWN_RING  the ring's radius, cm from the slot (default 6800: 8 m beyond the base's reach).
  HOL_CROWN_GATE  the ramp's width, cm (default 1600).
"""
import math
import os

import unreal

from maps import common, eldorado, steppe
from maps.common import mix_seed

GROUND = "MI_Ground_Crown"  # 2026-11-04: a textured ground of its own (maps/ground.py GROUNDS)

RING_RADIUS = float(os.environ.get("HOL_CROWN_RING", "6800"))
GATE_WIDTH = float(os.environ.get("HOL_CROWN_GATE", "1600"))
RING_VERTICES = 24  # the ring is a polygon of this many sides (even: the ramp sits on a vertex, opposite the first one)
WALL_CLEAR = 1100.0  # cm: no deposit's centre nearer than this to a ring's axis (the wall is 6 m across at most, its navigation volume 10 m)
SOURCE_CLEAR = 800.0  # cm: a source (a tree of a grove) is dropped if it is nearer than this to a ring's axis
BOWL_MARGIN = 1500.0  # cm between the cluster's outermost vein and the nearest deposit of the territories
SALT_RING = 311
EDGE_OVER = 500.0  # cm: a rock's centre may lie this far beyond the bounds (as the Cols' ranges)
VEIN_DRAWS = 20000

PLANS = {}


def ring_points(recipe, index):
    """The ring of place `index`: the polygon (first point repeated at the end) starting opposite the ramp, and the ramp's point, on the ray to the middle."""
    (sx, sy), _, _ = recipe.slot(index)
    toward = math.atan2(-sy, -sx)
    start = toward + math.pi
    points = [(sx + RING_RADIUS * math.cos(start + 2.0 * math.pi * k / RING_VERTICES), sy + RING_RADIUS * math.sin(start + 2.0 * math.pi * k / RING_VERTICES))
              for k in range(RING_VERTICES + 1)]
    points[-1] = points[0]
    gate = (sx + RING_RADIUS * math.cos(toward), sy + RING_RADIUS * math.sin(toward))
    return points, gate


def plan(recipe):
    """(rocks, volumes, markers) of every ring (cached): markers = [(label, x, y)] the ramps' middles."""
    key = (recipe.map_path, recipe.layout_seed, RING_RADIUS, GATE_WIDTH)
    if key in PLANS:
        return PLANS[key]
    rocks, volumes, markers = [], [], []
    for index in range(len(recipe.angles)):
        points, gate = ring_points(recipe, index)
        gate_at = common.polyline_length(points) / 2.0
        wall_rocks, wall_volumes = common.rock_wall(mix_seed(recipe.layout_seed, SALT_RING, index), points, [(gate_at, GATE_WIDTH)])
        # On Small (2 places) the ring of the place at the end of the axis does not fit: what lies beyond the map's edge is not made (the edge closes the ring there).
        limit = recipe.bounds_half + EDGE_OVER
        rocks += [rock for rock in wall_rocks if abs(rock[0]) <= limit and abs(rock[1]) <= limit]
        volumes += [volume for volume in wall_volumes if abs(volume[0]) <= limit and abs(volume[1]) <= limit]
        markers.append(("CrownRamp%d" % index, gate[0], gate[1]))
    PLANS[key] = (rocks, volumes, markers)
    return PLANS[key]


def distance_to_ring(recipe, point):
    """The distance from a point to the nearest ring's axis (the circle of the polygon)."""
    return min(abs(math.dist(point, recipe.slot(index)[0]) - RING_RADIUS) for index in range(len(recipe.angles)))


def bowl_radius(recipe):
    """The free disc round the middle that the territories' deposits keep out of: the cluster and a margin."""
    return eldorado.cluster_radius(recipe) + BOWL_MARGIN


def sources(recipe):
    """The Steppe's sources less its gold, none within WALL_CLEAR of a ring (steppe.KEEP_OUT, near and far layers alike) nor in the bowl, plus the gold
    cluster of El Dorado in the middle."""
    before = steppe.KEEP_OUT, steppe.VEIN_DRAWS, steppe.CENTRE_CLEAR, steppe.NEAR, common.MINERAL_RESOURCES
    steppe.KEEP_OUT = lambda point: distance_to_ring(recipe, point) < WALL_CLEAR
    steppe.VEIN_DRAWS = VEIN_DRAWS
    steppe.CENTRE_CLEAR = bowl_radius(recipe)
    steppe.NEAR = [row for row in steppe.NEAR if row[0] != "GoldVein"]
    common.MINERAL_RESOURCES = ("Stone",)
    try:
        placed, groups = steppe.sources(recipe, "forests")
    finally:
        steppe.KEEP_OUT, steppe.VEIN_DRAWS, steppe.CENTRE_CLEAR, steppe.NEAR, common.MINERAL_RESOURCES = before
    placed = [source for source in placed if not eldorado.is_gold(source)]
    # A grove's centre is kept out of the band but its far trees can reach into the wall: such a source is dropped (the territories hold a little more
    # than the budget, Map.Table checks it still holds it).
    kept = []
    for source in placed:
        at = source.get_editor_property("location")
        if distance_to_ring(recipe, (at.x, at.y)) >= SOURCE_CLEAR:
            kept.append(source)
    kept += [common.placed_source(name, x, y) for name, x, y in eldorado.cluster(recipe)]
    return kept, groups


def obstacles(recipe):
    """The ridges in front of the places, the rings' rocks with their navigation volumes, the markers of the ramps; the number of actors."""
    rocks, volumes, markers = plan(recipe)
    count = common.spawn_ridges(recipe)
    count += common.spawn_rock_wall(rocks, volumes, common.matte_instance("MI_Rock", common.ROCK_COLOR), "Ring")
    for label, x, y in markers:
        common.spawn(unreal.TargetPoint, unreal.Vector(x, y, 0), label=label)
    return count + len(markers)


def avoid_points(recipe):
    """The rings' rocks' places, kept clear of the relays."""
    return [(rock[0], rock[1]) for rock in plan(recipe)[0]]


def relays(recipe):
    """R2 (provisional level design): the ring of relays (between the keeps, on the middle angles) and the fixed ones, clear of the rocks and of the bowl
    (common.ring_relays, avoid = the rocks, extra_radius = the bowl's radius)."""
    return common.ring_relays(recipe, avoid=avoid_points(recipe), extra_radius=bowl_radius(recipe))


def tree_floor(recipe):
    """The trees every territory must still hold once the relays' zones have taken theirs (common.lay_relays_and_sources): the budget."""
    return steppe.floor_of("forests", recipe)


def volcano(recipe):
    """V4b (D607): the volcano site of the map, symmetric by construction (common.volcano_site); inert unless the match has the Catastrophe option (D398)."""
    return [common.volcano_site(recipe)]
