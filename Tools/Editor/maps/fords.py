"""Les Gues (chantier Cartes, C7; GDD 8.5, D496, D595): water that cannot be crossed except at the fords and the wide bridges. Registered in
Tools/Data/Maps.csv with the Budget `standard` (the Steppe's per-player budget).

CQ5 = C (the user, 2026-11-01, D595): from 4 to 8 places a RING of water round the middle (the bases outside, the middle on the other side of
the water), with a FORD and a wide BRIDGE in front of every base; with 2 places a straight RIVER between the two halves of the map (one player
on each side), a wide bridge in the middle and 2 fords, one each side of it. The fords do not slow (D496: a deviation from D33): they are free
stretches of the water, narrow (FORD_WIDTH, a file); a bridge is a wide free stretch (BRIDGE_WIDTH, a formation of 12 m passes). M-Q3 = A (the
bridge that falls becomes a site a villager lifts again for nothing) is NOT coded here: the bridge is a stretch of free ground; the building
that falls and rises is a later step (the mechanism of the ruin, V3).

The map is the Steppe's (steppe.sources: the same bases, the same near and far layers, the same budget) with the water laid over it. The water is
a flat band of dark blue (a stretched cube of the engine, no collision, no navigation of its own), WATER_WIDTH across, made of straight pieces;
under every piece a NavArea_Null volume (as the rocks of the Cols, D439) cuts the navmesh, so that no unit crosses. The places sit on a circle at
equal angles, so the ring and its gaps are the same shape turned onto every place: the fairness of the table stays exact (the Steppe's layers
are copies of place 0's turned, and a point in the water is forbidden to all of them, steppe.KEEP_OUT). No tree, bush or vein in the water or
within WATER_CLEAR of its edge. The relays keep clear of the water too (common.ring_relays avoid): the middle's 4 relays are inside the ring.

Heights (D23) are not drawn: M-Q2 = A (only the visible terrain). The decor of identity (sand, reeds at the edge of the water, D576) is NOT
laid: left to the user's word.
  HOL_FORDS_RING   the ring's radius as a fraction of the places' circle (default 0.35).
"""
import math
import os

import unreal

from maps import common, steppe

GROUND = steppe.GROUND
WATER_WIDTH = 1800.0  # cm: the water band, 18 m across
WATER_CLEAR = 500.0  # cm: no deposit's centre nearer than this to the water's edge (the deposit's own extent comes on top, steppe.MIN_EXTENT)
FORD_WIDTH = 500.0  # cm: a ford, a file of units (5 m free in the band)
BRIDGE_WIDTH = 1600.0  # cm: a wide bridge, a formation of 12 m passes (16 m free in the band)
FORD_OFFSET = 0.3  # a ring's ford is this fraction of the angle between two places beside the bridge (counterclockwise)
RING_FRACTION = float(os.environ.get("HOL_FORDS_RING", "0.35"))
RIVER_FORD_AT = 0.33  # a river's fords: this fraction of the bounds' half side each side of the bridge
PIECE = 800.0  # cm: a straight piece of the water (a ring's chord, a river's piece)
JOINT = 100.0  # cm: two pieces overlap by this much (a ring's chords are at an angle)
EDGE_OVER = 500.0  # cm: a river goes this much beyond the bounds
VEIN_DRAWS = 20000  # steppe.VEIN_DRAWS for this map
WATER_COLOR = (0.035, 0.13, 0.30)
FORD_COLOR = (0.30, 0.52, 0.60)
BRIDGE_COLOR = (0.36, 0.26, 0.14)
WATER_Z = 1.0  # the pieces' centre; the engine's cube is 1 m, scaled to 4 cm thick: the band spans -1 to 3 cm (as the zones' discs)
NULL_BOTTOM = -100.0  # cm: the bottom of a water piece's NavArea_Null volume
SAMPLE = 500.0  # cm between two points of the water's axis given to the relays (common.ring_relays avoid)

PLANS = {}


def ring_radius(recipe):
    return RING_FRACTION * recipe.slot_radius


def plan(recipe):
    """(runs, gaps) (cached): runs = [[(x, y), ...]] the water's axis between two gaps (a polyline per run); gaps = [(is_bridge, x, y, yaw, width)],
    yaw the direction across the water in degrees (the radial of a ring, the x axis of a river)."""
    key = (recipe.map_path, recipe.layout_seed, RING_FRACTION)
    if key in PLANS:
        return PLANS[key]
    places = len(recipe.angles)
    runs, gaps = [], []
    if places == 2:
        reach = recipe.bounds_half + EDGE_OVER
        ford = RIVER_FORD_AT * recipe.bounds_half
        along = [(False, -ford, FORD_WIDTH), (True, 0.0, BRIDGE_WIDTH), (False, ford, FORD_WIDTH)]
        for is_bridge, at, width in along:
            gaps.append((is_bridge, 0.0, at, 0.0, width))
        edges = [-reach] + [value for _, at, width in along for value in (at - width / 2.0, at + width / 2.0)] + [reach]
        for low, high in zip(edges[0::2], edges[1::2]):
            count = max(1, int(math.ceil((high - low) / PIECE)))
            runs.append([(0.0, low + (high - low) * index / count) for index in range(count + 1)])
    else:
        radius = ring_radius(recipe)
        spacing = 2.0 * math.pi / places
        found = []  # (angle, is_bridge, width)
        for angle in recipe.angles:
            base = math.radians(angle)
            found.append((base % (2.0 * math.pi), True, BRIDGE_WIDTH))
            found.append(((base + FORD_OFFSET * spacing) % (2.0 * math.pi), False, FORD_WIDTH))
        found.sort()
        for is_bridge_angle in found:
            angle, is_bridge, width = is_bridge_angle
            gaps.append((is_bridge, radius * math.cos(angle), radius * math.sin(angle), math.degrees(angle), width))
        for index, (angle, _, width) in enumerate(found):
            following, _, following_width = found[(index + 1) % len(found)]
            start = angle + width / 2.0 / radius
            end = following - following_width / 2.0 / radius
            if end <= start:
                end += 2.0 * math.pi
            count = max(1, int(math.ceil((end - start) * radius / PIECE)))
            runs.append([(radius * math.cos(start + (end - start) * step / count), radius * math.sin(start + (end - start) * step / count)) for step in range(count + 1)])
    PLANS[key] = (runs, gaps)
    return PLANS[key]


def pieces(recipe):
    """[(x, y, length, yaw)] the straight pieces of the water: a run's consecutive points, each overlapping the next by JOINT inside a run."""
    runs, _ = plan(recipe)
    out = []
    for run in runs:
        for index in range(len(run) - 1):
            a, b = run[index], run[index + 1]
            length = math.dist(a, b)
            yaw = math.degrees(math.atan2(b[1] - a[1], b[0] - a[0]))
            extra = (JOINT if index > 0 else 0.0) + (JOINT if index < len(run) - 2 else 0.0)
            shift = (JOINT if index > 0 else 0.0) / 2.0 - (JOINT if index < len(run) - 2 else 0.0) / 2.0
            ux, uy = (b[0] - a[0]) / length, (b[1] - a[1]) / length
            out.append(((a[0] + b[0]) / 2.0 - ux * shift, (a[1] + b[1]) / 2.0 - uy * shift, length + extra, yaw))
    return out


def distance_to_water(recipe, point):
    """The distance from a point to the water's AXIS (the gaps count as water here: nothing is laid in a gap either)."""
    if len(recipe.angles) == 2:
        return abs(point[0])
    return abs(math.hypot(*point) - ring_radius(recipe))


def axis_points(recipe):
    """Points of the water's axis, SAMPLE apart, gaps included (what the relays keep away from)."""
    out = []
    if len(recipe.angles) == 2:
        reach = recipe.bounds_half + EDGE_OVER
        count = int(2 * reach / SAMPLE)
        out = [(0.0, -reach + 2 * reach * index / count) for index in range(count + 1)]
    else:
        radius = ring_radius(recipe)
        count = int(2.0 * math.pi * radius / SAMPLE)
        out = [(radius * math.cos(2.0 * math.pi * index / count), radius * math.sin(2.0 * math.pi * index / count)) for index in range(count)]
    return out


def keep_out_of(recipe):
    reach = WATER_WIDTH / 2.0 + WATER_CLEAR

    def keep_out(point):
        return distance_to_water(recipe, point) < reach

    return keep_out


def sources(recipe):
    """The Steppe's sources, none of them in the water or within WATER_CLEAR of it (steppe.KEEP_OUT, near and far layers alike), then the ones whose
    centre is still too near dropped (a grove's far trees can reach in)."""
    before, draws = steppe.KEEP_OUT, steppe.VEIN_DRAWS
    steppe.KEEP_OUT = keep_out_of(recipe)
    steppe.VEIN_DRAWS = VEIN_DRAWS
    try:
        placed, groups = steppe.sources(recipe)
    finally:
        steppe.KEEP_OUT, steppe.VEIN_DRAWS = before, draws
    kept = []
    for source in placed:
        at = source.get_editor_property("location")
        if distance_to_water(recipe, (at.x, at.y)) >= WATER_WIDTH / 2.0 + 200.0:
            kept.append(source)
    return kept, groups


def flat_piece(label, x, y, z, length, width, thickness, yaw, material):
    """A flat box of the engine's cube (1 m), no collision and no navigation of its own."""
    actor = common.spawn_shape("/Engine/BasicShapes/Cube", unreal.Vector(x, y, z), unreal.Vector(length / 100.0, width / 100.0, thickness / 100.0), label, material, yaw=yaw)
    component = actor.get_editor_property("static_mesh_component")
    component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    component.set_editor_property("can_ever_affect_navigation", False)
    return actor


def obstacles(recipe):
    """The ridges in front of the places, the water with its navigation volumes, the fords and bridges (drawn and marked); the number of actors."""
    count = common.spawn_ridges(recipe)
    water = common.matte_instance("MI_Water", unreal.LinearColor(*WATER_COLOR, 1.0))
    bed = common.matte_instance("MI_FordBed", unreal.LinearColor(*FORD_COLOR, 1.0))
    deck = common.matte_instance("MI_BridgeDeck", unreal.LinearColor(*BRIDGE_COLOR, 1.0))
    for number, (x, y, length, yaw) in enumerate(pieces(recipe)):
        flat_piece("Water%d" % number, x, y, WATER_Z, length, WATER_WIDTH, 4.0, yaw, water)
        # From below the ground (the rocks' volumes start at 50 cm because the rocks themselves cut the navmesh; flat water does not).
        volume = common.spawn(unreal.NavModifierVolume, unreal.Vector(x, y, (NULL_BOTTOM + common.ROCK_NULL_TOP) / 2),
                              unreal.Rotator(roll=0, pitch=0, yaw=yaw), "WaterNoNav%d" % number)
        volume.set_area_class(unreal.NavArea_Null)
        volume.set_actor_scale3d(unreal.Vector(length / 200.0, WATER_WIDTH / 200.0, (common.ROCK_NULL_TOP - NULL_BOTTOM) / 200))
        count += 2
    _, gaps = plan(recipe)
    fords = bridges = 0
    for is_bridge, x, y, yaw, width in gaps:
        if is_bridge:
            flat_piece("BridgeDeckPlank%d" % bridges, x, y, 2.5, WATER_WIDTH + 300.0, width, 6.0, yaw, deck)
            common.spawn(unreal.TargetPoint, unreal.Vector(x, y, 0), label="BridgeGap%d" % bridges)
            bridges += 1
        else:
            flat_piece("FordBedShallows%d" % fords, x, y, 1.5, WATER_WIDTH + 100.0, width, 4.0, yaw, bed)
            common.spawn(unreal.TargetPoint, unreal.Vector(x, y, 0), label="FordGap%d" % fords)
            fords += 1
        count += 2
    return count


def relays(recipe):
    """R2 (provisional level design): the ring's relays between the places and the 4 round the middle, all kept away from the water (axis_points as
    the rocks to avoid: a zone's edge touches no water). A river map (2 places) has its relays on each place's own axis, the ring's usual circle being the river."""
    # 2 places: the middle is the river's bridge, so the middle fixed relay is moved along the axis of the places out of the water (extra_radius).
    river = len(recipe.angles) == 2
    return common.ring_relays(recipe, avoid=axis_points(recipe), on_slots=river, extra_radius=WATER_WIDTH / 2.0 if river else 0.0)


def tree_floor(recipe):
    """The trees every territory must still hold once the relays' zones have taken theirs (common.lay_relays_and_sources): the budget."""
    return steppe.budget_of(recipe)[0]


def volcano(recipe):
    """V4b (D607): the volcano site of the map, symmetric by construction (common.volcano_site); inert unless the match has the Catastrophe option (D398)."""
    return [common.volcano_site(recipe)]
