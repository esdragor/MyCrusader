"""The recipe of Plaine (D256, J1; TC2): what Plain adds to the shared pieces of maps/common.py (ground, light, bounds,
navmesh, start slots, each player's base). Made by create_match_map.py as /Game/Maps/Plain_<Size>, and without a size as
/Game/Maps/Plain, the fixed test map (D494).

  obstacles    so that the paths show (D286, J4): 30 m in front of each slot, towards the middle, a ridge of three
               boulders, no navmesh on their tops (a NavArea_Null volume each); in the middle of the map, a grove of
               trees (the units walk between the trunks), with rings of trees around it on the larger sizes.
               Their matte materials /Game/Rendering/MI_Rock and MI_Tree are made when missing.
  sources      the bases (common.base_sources), then Plaine's neutral sources (sized maps only, TC-Q7 = B: each map has
               its own recipe, see plain_neutral_sources): between each two neighbouring slots, along the line from the
               middle of their arc to the centre, woods and veins in turn, more of them the more room each player has.
"""
import math

import unreal

from maps import common
from maps.common import spawn_shape, matte_instance, placed_source, wood

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

# Plaine's neutral sources (TC-Q7 = B). Each player's room: the side squared over the places, less its base (a disc of
# common.BASE_REACH); one group per NEUTRAL_AREA of it (Small 4, Medium 4, Normal 3, Large 3, Huge 4 per player). The groups of
# each arc on the line from its middle to the centre, from NEUTRAL_OUTER to NEUTRAL_INNER of the slots' radius, in the
# order of NEUTRAL_PATTERN from the outside.
NEUTRAL_AREA = 2.0e8  # cm2: 2 ha
NEUTRAL_OUTER = 0.9
NEUTRAL_INNER = 0.3
NEUTRAL_PATTERN = ["Wood", "GoldVein", "Wood", "StoneVein"]
NEUTRAL_WOOD_ROWS = 4
NEUTRAL_WOOD_COLUMNS = 5


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


def plain_neutral_sources(recipe):
    """Plaine's own neutral sources (TC-Q7 = B; another map, El Dorado say, has its own function): on the line from
    the middle of each two neighbouring slots' arc to the centre, NEUTRAL_PATTERN's groups in turn, as many as the
    room of each player holds. Symmetric: the same for each arc, turned."""
    places = len(recipe.angles)
    side = 2 * recipe.bounds_half
    room = side * side / places - math.pi * common.BASE_REACH * common.BASE_REACH
    groups = int(room / NEUTRAL_AREA)
    sources = []
    for index in range(places):
        radians = math.radians(recipe.angles[index] + 180.0 / places)
        radial = (math.cos(radians), math.sin(radians))
        across = (-radial[1], radial[0])
        for group in range(groups):
            fraction = NEUTRAL_OUTER - (NEUTRAL_OUTER - NEUTRAL_INNER) * group / max(1, groups - 1)
            at = (radial[0] * fraction * recipe.slot_radius, radial[1] * fraction * recipe.slot_radius)
            kind = NEUTRAL_PATTERN[group % len(NEUTRAL_PATTERN)]
            if kind == "Wood":
                # Centred on its point: rows across the line, columns along it.
                first = -(NEUTRAL_WOOD_ROWS - 1) / 2.0 * common.WOOD_SPACING
                sources += wood(at, radial, across, NEUTRAL_WOOD_ROWS, NEUTRAL_WOOD_COLUMNS, first)
            else:
                sources.append(placed_source(kind, at[0], at[1]))
    return sources, groups


def sources(recipe):
    """The sources of the match (D297), as AHOLSourceLayout data: the bases, then the neutral groups (sized maps)."""
    placed = common.base_sources(recipe)
    groups = 0
    if recipe.size_key is not None:
        neutral, groups = plain_neutral_sources(recipe)
        placed += neutral
    return placed, groups
