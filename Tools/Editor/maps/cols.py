"""Les Cols (chantier Cartes, C4; GDD 8.5, D496): mountains cut the map into corridors between the bases, each with its resources.
Registered in Tools/Data/Maps.csv with the Budget `standard` (the Steppe's per-player budget); the shape below is PROVISIONAL until the user
answers CQ4 (Auto_Journey_Retours.md): every named constant is a setting, HOL_COLS_PASSES and HOL_COLS_PLAZA are read from the environment.

The map is the Steppe's (steppe.sources: the same bases, the same near and far layers, the same budget) with a RANGE of mountains between
every two neighbouring places: a massif of two chains of rock (common.rock_wall, rocks the units cannot cross, NavArea_Null under them,
D439) along the ray from the map's centre through the middle angle of the two places, which is the border of their territories. A range
runs from the PLAZA (a free disc round the centre, PLAZA_FRACTION of the half side) to the edge of the bounds, and has COLS_PER_RANGE
gaps of COL_WIDTH (the cols, "passes"), a pair of neighbouring bases being linked by the plaza and by the cols of the range between them:
with 2 cols a range, three corridors. The places sit in a ring at equal angles, so the ranges are the same shape turned onto every place:
the fairness of the table stays exact (the Steppe's layers are copies of place 0's turned, and a point in a range is forbidden to all of them,
steppe.KEEP_OUT). The deposits (far layer) keep MOUNTAIN_CLEAR off the massifs' axis: the cols are lanes with resources on both sides, not
resources in the rock.

Heights (D23) are not drawn: M-Q2 = A (only the visible terrain is coded; relief to a combat chantier after the prototype). The mountains
are the rocks; no scenery (D576: nothing generic, and the rock already says what the terrain is).
  HOL_COLS_PASSES  gaps per range (default 2).
  HOL_COLS_PLAZA   the plaza's radius as a fraction of the bounds' half side (default 0.2).
"""
import math
import os

import unreal

from maps import common, steppe
from maps.common import mix_seed

GROUND = steppe.GROUND
COLS_PER_RANGE = int(os.environ.get("HOL_COLS_PASSES", "2"))
PLAZA_FRACTION = float(os.environ.get("HOL_COLS_PLAZA", "0.2"))
CHAIN_OFFSETS = (-700.0, 700.0)  # cm: the two chains of a massif, side by side (about 20 m of rock across with the rocks' own width)
COL_WIDTH = 1600.0  # cm: the free width of a col (a formation of 12 m passes; the rocks stop WALL_HALF_LENGTH short of the edges of the gap)
COL_FRACTIONS = {1: (0.55,), 2: (0.35, 0.78), 3: (0.25, 0.55, 0.85)}  # along the range from the plaza to the edge
MOUNTAIN_HALF = 1500.0  # cm: a massif's half width (the outer chains' axes plus the rocks' reach)
MOUNTAIN_CLEAR = 500.0  # cm: no deposit's centre nearer than this to a massif's edge (the deposit's own extent comes on top, steppe.MIN_EXTENT)
EDGE_OVER = 500.0  # cm: a range goes this much beyond the bounds, so that nothing slips between its end and the edge
SALT_WALL = 301
SOURCE_CLEAR = 200.0  # cm: a source's centre is this far from a massif's edge at least
VEIN_DRAWS = 20000  # steppe.VEIN_DRAWS for this map (4000 is not always enough in the narrow territories of 8 places)

PLANS = {}


class Range:
    def __init__(self, index, angle, start, end):
        self.index = index
        self.angle = angle  # radians, the ray's direction
        self.start = start  # (x, y) of the plaza end
        self.end = end  # (x, y) at the edge
        self.length = math.dist(start, end)

    def offset(self, shift):
        """The range's two end points moved sideways by shift (counterclockwise)."""
        nx, ny = -math.sin(self.angle), math.cos(self.angle)
        return [(self.start[0] + nx * shift, self.start[1] + ny * shift), (self.end[0] + nx * shift, self.end[1] + ny * shift)]

    def gaps(self):
        return [self.length * fraction for fraction in COL_FRACTIONS[COLS_PER_RANGE]]


def ranges_of(recipe):
    """The ranges, one between each two neighbouring places (sorted by angle, the last one wrapping round to the first)."""
    angles = sorted(angle % 360.0 for angle in recipe.angles)
    plaza = PLAZA_FRACTION * recipe.bounds_half
    out = []
    for index, angle in enumerate(angles):
        following = angles[(index + 1) % len(angles)]
        if following <= angle:
            following += 360.0
        middle = math.radians((angle + following) / 2.0)
        reach = recipe.bounds_half / max(abs(math.cos(middle)), abs(math.sin(middle))) + EDGE_OVER
        out.append(Range(index, middle, (plaza * math.cos(middle), plaza * math.sin(middle)), (reach * math.cos(middle), reach * math.sin(middle))))
    return out


def plan(recipe):
    """(rocks, volumes, markers) of every range (cached)."""
    key = (recipe.map_path, recipe.layout_seed, COLS_PER_RANGE, PLAZA_FRACTION)
    if key in PLANS:
        return PLANS[key]
    rocks, volumes, markers = [], [], []
    for mountain in ranges_of(recipe):
        for chain, shift in enumerate(CHAIN_OFFSETS):
            points = mountain.offset(shift)
            seed = mix_seed(recipe.layout_seed, SALT_WALL, mountain.index * 10 + chain)
            wall_rocks, wall_volumes = common.rock_wall(seed, points, [(centre, COL_WIDTH) for centre in mountain.gaps()])
            rocks += wall_rocks
            volumes += wall_volumes
        for number, centre in enumerate(mountain.gaps()):
            point, _ = common.point_along(mountain.offset(0.0), centre)
            markers.append(("ColGap%d_%d" % (mountain.index, number), point[0], point[1]))
    PLANS[key] = (rocks, volumes, markers)
    return PLANS[key]


def distance_to_range(point, mountain):
    a, b = mountain.start, mountain.end
    dx, dy = b[0] - a[0], b[1] - a[1]
    t = max(0.0, min(1.0, ((point[0] - a[0]) * dx + (point[1] - a[1]) * dy) / (dx * dx + dy * dy)))
    return math.dist(point, (a[0] + t * dx, a[1] + t * dy))


def keep_out_of(recipe):
    ranges = ranges_of(recipe)

    def keep_out(point):
        return any(distance_to_range(point, mountain) < MOUNTAIN_HALF + MOUNTAIN_CLEAR for mountain in ranges)

    return keep_out


def sources(recipe):
    """The Steppe's sources, none of them within MOUNTAIN_CLEAR of a massif (steppe.KEEP_OUT, near and far layers alike)."""
    before, draws = steppe.KEEP_OUT, steppe.VEIN_DRAWS
    steppe.KEEP_OUT = keep_out_of(recipe)
    steppe.VEIN_DRAWS = VEIN_DRAWS  # the wedges are narrow: a far vein's random points seldom fall in a territory outside the massifs
    try:
        placed, groups = steppe.sources(recipe)
    finally:
        steppe.KEEP_OUT, steppe.VEIN_DRAWS = before, draws
    # A grove's centre is kept out of the massifs but its far trees can reach into one (seen on Medium: one tree): such a source is dropped
    # (the territories hold a little more than the budget, Map.Table checks it still holds it).
    ranges = ranges_of(recipe)
    kept = []
    for source in placed:
        at = source.get_editor_property("location")
        if all(distance_to_range((at.x, at.y), mountain) >= MOUNTAIN_HALF + SOURCE_CLEAR for mountain in ranges):
            kept.append(source)
    return kept, groups


def obstacles(recipe):
    """The ridges in front of the places, the ranges' rocks with their navigation volumes, the markers of the cols; the number of actors."""
    rocks, volumes, markers = plan(recipe)
    count = common.spawn_ridges(recipe)
    count += common.spawn_rock_wall(rocks, volumes, common.matte_instance("MI_Rock", common.ROCK_COLOR), "Range")
    for label, x, y in markers:
        common.spawn(unreal.TargetPoint, unreal.Vector(x, y, 0), label=label)
    return count + len(markers)


def relays(recipe):
    """R2 (provisional level design): the 4 extra relays in the plaza and one in each place's own valley (common.ring_relays on_slots: the ranges of
    mountains run along the middle angles where the other maps put their ring)."""
    return common.ring_relays(recipe, on_slots=True)


def tree_floor(recipe):
    """The trees every territory must still hold once the relays' zones have taken theirs (common.lay_relays_and_sources): the budget."""
    return steppe.budget_of(recipe)[0]


def volcano(recipe):
    """V4b (D607): the volcano site of the map, symmetric by construction (common.volcano_site); inert unless the match has the Catastrophe option (D398)."""
    return [common.volcano_site(recipe)]
