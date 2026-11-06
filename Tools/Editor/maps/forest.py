"""La Foret profonde (chantier Cartes, C5; GDD 8.5, D496): a dense forest with fast roads. Registered in Tools/Data/Maps.csv with the Budget
`standard` (the Steppe's per-player budget). CQ6a = A (the user, 2026-11-01): the roads are terrain zones like the marsh, a walker on one walks
at 130 % (hol.Terrain.RoadSpeedPercent, HOLTerrainRules), the discs laid side by side in a corridor of 8 m; the forest itself has NO effect
(neither on speed nor on sight: the view blocked waits for M7, M-Q2 = A).

The map is the Steppe's (steppe.sources: the same bases, the same near and far layers, the same budget: the trees ARE the forest) with ROADS
cut through it: a road from every place to the map's centre (a spoke) and one from every place to the next one (a chord), each starting
ROAD_START off the place's slot (outside the base's reach) so that no road crosses a base, and each ending ROAD_START off the next place or
CENTRE_STOP off the centre. The places sit in a ring at equal angles, so the roads are the same shape turned onto every place: the fairness
of the table stays exact, and nobody has a shorter way to the middle or to a neighbour (2 places: one straight road through the centre, the
spokes of both). No tree, bush or vein stands in a road nor within ROAD_CLEAR of its axis (steppe.KEEP_OUT, the grove's own extent is added).

The forest's density (how big the groves are, how close the trees stand) is the Steppe's until the user settles the trees' spacing (CQ1,
Auto_Journey_Retours.md): nothing is regenerated for it here. Settings: HOL_FOREST_CHORDS=0 turns the chords off (spokes only).
"""
import math
import os

from maps import common, steppe

GROUND = "MI_Ground_Forest"  # 2026-11-04: a textured ground of its own (maps/ground.py GROUNDS)

ROAD_KIND = 1
ROAD_RADIUS = 400.0  # cm: a disc of 8 m across (CQ6a = A: "zones rondes alignees en couloir de 8 m")
ROAD_STEP = 400.0  # cm between two disc centres along a road: they overlap by half a diameter, a continuous corridor of 8 m
ROAD_START = 6500.0  # cm: a road starts this far from a slot (common.BASE_REACH is 6000: no road in a base)
CENTRE_STOP = 1200.0  # cm: a spoke stops this far from the map's centre (a free hub round it, the Steppe's centre is clear to 20 m)
ROAD_CLEAR = 300.0  # cm: no source's centre nearer than this to a road's edge (the grove's extent comes on top for the layout's keep-out)
CHORDS = os.environ.get("HOL_FOREST_CHORDS", "1") != "0"
VEIN_DRAWS = 20000  # steppe.VEIN_DRAWS for this map (the corridors make the territories narrower)

# CQ7 = B (D598): the Budget `dense` of Tools/Data/Maps.csv: twice the Steppe's trees per player (Map.Table: 600, Huge 720), in groves of
# 30 to 40 trees (the Steppe's are 12 to 18); the berries are the standard ones.
DENSE_BUDGET = {"default": (600, 15), "Huge": (int(os.environ.get("HOL_FOREST_HUGE_TREES", "720")), 23)}
DENSE_LADDER = [30, 34, 37, 40]
DENSE_CYCLE = [34, 34, 30, 37, 40, 34, 34, 30, 37, 40]
DENSE_MEANS = [34.0, 35.0, 36.0, 37.0, 38.0]
DENSE_PACK = 35  # common.PACK_TREES for the forest: how many groves a territory is asked to hold

PLANS = {}


def segments(recipe):
    """[((x, y), (x, y))] the straight roads of the whole map: the spokes, then the chords (each pair of neighbours once)."""
    places = len(recipe.angles)
    slots = [recipe.slot(i)[0] for i in range(places)]
    out = []
    for slot in slots:
        distance = math.hypot(*slot)
        unit = (slot[0] / distance, slot[1] / distance)
        start = max(distance - ROAD_START, CENTRE_STOP)
        if start - CENTRE_STOP >= ROAD_STEP:
            out.append(((unit[0] * start, unit[1] * start), (unit[0] * CENTRE_STOP, unit[1] * CENTRE_STOP)))
    if CHORDS and places >= 3:
        order = sorted(range(places), key=lambda i: math.atan2(slots[i][1], slots[i][0]))
        for k in range(places):
            a, b = slots[order[k]], slots[order[(k + 1) % places]]
            length = math.dist(a, b)
            if length <= 2.0 * ROAD_START + ROAD_STEP:
                continue
            ux, uy = (b[0] - a[0]) / length, (b[1] - a[1]) / length
            out.append(((a[0] + ux * ROAD_START, a[1] + uy * ROAD_START), (b[0] - ux * ROAD_START, b[1] - uy * ROAD_START)))
    return out


def discs(recipe):
    """[(x, y)] the centres of the road discs, ROAD_STEP apart along every segment (the end point included)."""
    key = (recipe.map_path, len(recipe.angles), recipe.bounds_half, CHORDS)
    if key not in PLANS:
        points = []
        for (ax, ay), (bx, by) in segments(recipe):
            length = math.dist((ax, ay), (bx, by))
            count = max(1, int(math.ceil(length / ROAD_STEP)))
            for i in range(count + 1):
                t = i / count
                points.append((ax + (bx - ax) * t, ay + (by - ay) * t))
        PLANS[key] = points
    return PLANS[key]


def distance_to_road(recipe, point):
    best = math.inf
    for (ax, ay), (bx, by) in segments(recipe):
        dx, dy = bx - ax, by - ay
        t = max(0.0, min(1.0, ((point[0] - ax) * dx + (point[1] - ay) * dy) / (dx * dx + dy * dy)))
        best = min(best, math.dist(point, (ax + t * dx, ay + t * dy)))
    return best


def sources(recipe):
    """The Steppe's sources, none of them on or within ROAD_CLEAR of a road (the layout keeps the groves' extent off it, then the sources
    whose centre is still too near are dropped: the territories hold a little more than the budget, Map.Table checks it still holds it)."""
    before = (steppe.KEEP_OUT, steppe.VEIN_DRAWS, steppe.BUDGET, steppe.GROVE_LADDER, steppe.GROVE_CYCLE, steppe.GROVE_MEANS, steppe.MIN_EXTENT,
              steppe.FAR_FLOOR, common.PACK_TREES)
    # 2026-11-04 (the user): the forest holds 600 trees per player (Huge 900, steppe.WOODS["dense"]) in real forests of 200 trees or more.
    profile = "dense" if recipe.size_key == "Medium" else "dense_x2"  # the painted Medium's layout; the other sizes stretch its forests (common.stretch_edits)
    reach = ROAD_RADIUS + ROAD_CLEAR + steppe.extent("Wood", steppe.WOODS[profile][1][0])
    steppe.KEEP_OUT = lambda point: distance_to_road(recipe, point) < reach
    steppe.VEIN_DRAWS = VEIN_DRAWS
    try:
        placed, groups = steppe.sources(recipe, profile, camp_veins=True)
    finally:
        (steppe.KEEP_OUT, steppe.VEIN_DRAWS, steppe.BUDGET, steppe.GROVE_LADDER, steppe.GROVE_CYCLE, steppe.GROVE_MEANS, steppe.MIN_EXTENT,
         steppe.FAR_FLOOR, common.PACK_TREES) = before
    kept = []
    for source in placed:
        at = source.get_editor_property("location")
        if distance_to_road(recipe, (at.x, at.y)) >= ROAD_RADIUS + ROAD_CLEAR:
            kept.append(source)
    return kept, groups


def stretch_blocked(recipe, point):
    """common.stretch_edits: no stretched painted tree in a road's corridor (ROAD_RADIUS + ROAD_CLEAR from its axis, the recipe's own rule)."""
    return distance_to_road(recipe, point) < ROAD_RADIUS + ROAD_CLEAR


def obstacles(recipe):
    """The ridges in front of the places, nothing else (no rock: the forest is the trees); the number of actors."""
    return common.spawn_ridges(recipe)


def zones(recipe):
    """[(kind, x, y, radius)] for common.spawn_zones: the road discs (kind 1)."""
    return [(ROAD_KIND, x, y, ROAD_RADIUS) for x, y in discs(recipe)]


def relays(recipe):
    """R2 (provisional level design): one relay between every two neighbouring places (common.ring_relays); the roads are zones that do not stop them."""
    return common.ring_relays(recipe)


def volcano(recipe):
    """V4b (D607): the volcano site of the map, symmetric by construction (common.volcano_site); inert unless the match has the Catastrophe option (D398)."""
    return [common.volcano_site(recipe)]
