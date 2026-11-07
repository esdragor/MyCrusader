"""What every match map is made of (D256, J1; TC2; the chantier Cartes, C0.3): a flat ground, light and sky, the start
slots, the map bounds, the navmesh and each player's base, the same whatever the map. A map (Plaine, La Steppe, El
Dorado...) is a small module beside this one, named by Tools/Data/Maps.csv (Name = asset name, Recipe = module name under
Tools/Editor/maps), and only adds its own obstacles and its own sources. Each map is made once per named size (TC-Q3 = A,
Tools/Data/MapSizes.csv), as /Game/Maps/<Map>_<Size> (Small, Medium, Normal, Large, Huge); Plain alone also has a fixed
test map, /Game/Maps/Plain, as it has always been (the scenarios and quick_check count on its slots, D494). The main
centres are not in the map: the game mode puts one on each occupied slot when a match starts (D259).

The contract of a map module (maps/plain.py is the model), called by make_map once the shared pieces are placed:
  obstacles(recipe) -> int   spawns the map's obstacles (its ridges, groves, rocks...) and returns how many actors it
                             spawned, for the summary line
  sources(recipe) -> (sources, groups)   the sources of the match in the order the server reads them, as a list of
                             HOLPlacedSource (base_sources first: source 0 is the first tree of slot 0's wood, the
                             scripted checks use it), and the number of neutral groups per player, for the summary line

The shared pieces, in the order they are placed:
  ground       the bounds and 50 m more on each side (TC-Q5), matte grid material, a line every tile of 4 m (D265, D344): the
               grey M_MatteGround, or the varied ground a map module names with GROUND (maps/ground.py)
  bounds       AHOLMapBounds, the size's side (Plain: 400 m): the camera's focus stays inside
  navmesh      over the bounds (NavMeshBoundsVolume), built here and saved with the map: the project's navmesh only
               takes modifiers at run time (D286, the buildings' footprints), so the map must carry it
  start slots  AHOLStartSlot 0 to N-1 (N: the size's places), facing the centre on a circle of SLOT_RADIUS_FRACTION of
               the half side, numbered in order around it (place k and k+1 neighbours, FHOLMapSizeRules::DrawPlaces):
               2 face to face, 4 at the points of the compass, 6 and 8 evenly. Plain's test map: 8 slots on a circle of
               150 m, the first ones far apart (SLOT_ANGLES: 0 and 1 face to face, 0 to 3 at the points of the compass...)
  obstacles    the map module's (D286, J4): so that the paths show. The ridge of three boulders in front of every slot is the
               shared piece spawn_ridges, which Plain and the Steppe both call
  sources      the resources of the match (D295, D297), as data in an AHOLSourceLayout the server reads at the start.
               Each player's base, the same at every size and on every map, relative to its slot (TC-Q7 = B,
               base_sources): a wood of 40 trees beside each slot (J5a), on its left seen from the slot, 18 to 27 m from
               it, out of the way of the Peasants born at the centre's foot. Trees 2.2 m apart: with the navmesh's agent
               radius, a wood is a wall, as in AoE, and its edge recedes tree by tree. The same small offsets for every
               slot. Beside each slot on the other side (J5b.1), 6 berry bushes, 20 to 23 m from it: farther than the
               nearest trees (the scripted checks aim at the tree nearest a start). Two veins ~60 m from the slot,
               towards each neighbour: the test map puts one between each two neighbouring slots, stone and gold in
               turn, as far from one as from the other (contested, D295); on the sized maps the bases are farther
               apart, so each slot has its own, stone on its wood's side, gold on the other, at the same place relative
               to it. The map module adds its neutral sources after the bases.
               A map that wants a natural look (C1c, the Steppe) asks base_sources for organic=True: the wood is one irregular grove
               and the berries one irregular patch (grove_points, berry_points), the same shapes at every slot. Those helpers,
               and the seeded generator behind them (Rng, Noise, mix_seed), are shared by the maps; recipe.layout_seed (1 by
               default, `seed=N` for create_match_map.py) is what they are drawn from. Plain keeps its grid wood.
"""
import csv
import glob
import math
import os

import unreal

from maps import ambience
from maps import ground

MAPS_ROOT = "/Game/Maps"
GROUND_SIZE = 50000.0  # cm
BOUNDS_HALF_SIZE = 20000.0
SLOT_RADIUS = 15000.0
# Slot i at this angle (degrees): every first n slots are spread around the circle.
SLOT_ANGLES = [180.0, 0.0, 270.0, 90.0, 225.0, 45.0, 315.0, 135.0]
PLAYER_START_HALF_HEIGHT = 92.0

# Sized maps (TC2). Tile of the building grid (D344) and the ground past the bounds (TC-Q5).
TILE_SIZE = 400.0
GROUND_MARGIN = 5000.0
# Slots on a circle of this fraction of the half side: Plain's 150 m on 400 m. Neighbouring bases 330 m apart on Small
# (face to face), 327 m on Medium, 274 m on Normal, 232 m on Large, 253 m on Huge; each slot 55 m (Small) to 110 m
# (Huge) inside the bounds, room for its wood and berries behind it.
SLOT_RADIUS_FRACTION = 0.75
# Slot 0 at this angle, the next ones counterclockwise (the same order around the circle as DrawPlaces').
FIRST_SLOT_ANGLE = 180.0
NAV_BUILD_DELAY = 5.0  # seconds
# What a base takes up around its slot (the wood, the berries, the main centre's room): a map's neutral sources stay at
# least this far from every slot (Plain's room per player is the side squared over the places, less a disc of this radius).
BASE_REACH = 6000.0

# Wood beside each slot (D295): WOOD_ROWS rows from WOOD_NEAR on the slot's left, WOOD_COLUMNS along the radius.
WOOD_NEAR = 1800.0
WOOD_SPACING = 220.0
WOOD_ROWS = 5
WOOD_COLUMNS = 8
# Offsets (cm) of the trees from their grid point, the same for every slot (symmetric map): a wood is not a grid.
# At most 20 cm: two neighbours never leave a way through for a unit (60 cm agent radius on each trunk of 60 cm).
WOOD_JITTER = [(16, -9), (-12, 18), (6, 14), (-19, -5), (10, -17), (-4, 7), (19, 11), (-15, -18), (3, -13), (-8, 20)]
# Berries on each slot's other side (J5b.1): BERRY_ROWS x BERRY_COLUMNS bushes from BERRY_NEAR. Bushes of 1.1 m radius
# 3 m apart: 80 cm between two, closed to a unit (60 cm agent radius), so a patch is gathered from its edge.
BERRY_NEAR = 2000.0
BERRY_SPACING = 300.0
BERRY_ROWS = 2
BERRY_COLUMNS = 3
# The organic base (C1c; base_sources(organic=True)): the grove's centre ORGANIC_WOOD_SIDE to the slot's left, the patch's centre
# ORGANIC_BERRY_SIDE to its right, their trees and bushes within the reach of them (the wood stays 17 m or more from the slot, out
# of the way of the Peasants born at the centre's foot).
ORGANIC_WOOD_SIDE = 2600.0
ORGANIC_WOOD_REACH = 850.0
ORGANIC_BERRY_SIDE = 2250.0
ORGANIC_BERRY_REACH = 420.0
# Veins between neighbouring slots (J5b.1): at the middle angle of two slots, VEIN_RADIUS from the map's centre (inside
# the slots' circle: the way from a slot to the vein on its wood's side passes ~7 m beside the wood). Stone first.
VEIN_RADIUS = 12000.0
VEIN_TYPES = ["StoneVein", "GoldVein"]
# The same place relative to the slot on the sized maps: VEIN_AHEAD towards the centre, VEIN_SIDE to each side (~60 m).
VEIN_AHEAD = SLOT_RADIUS - VEIN_RADIUS * math.cos(math.radians(22.5))
VEIN_SIDE = VEIN_RADIUS * math.sin(math.radians(22.5))

# Ridges (D286, J4). A ridge across the way from each slot to the middle, RIDGE_AHEAD in front of the slot: 30 m in front
# of the main centre, out of the 12 m centre's way and of the Peasants born at its foot.
RIDGE_AHEAD = 3000.0
RIDGE_BOULDERS = [(-650.0, unreal.Vector(7.0, 4.5, 3.0)), (0.0, unreal.Vector(8.0, 5.0, 3.6)), (650.0, unreal.Vector(6.5, 4.0, 2.8))]
# No navmesh on a boulder's top (MV.4, the user's choice A): its flat top would carry its own patch, cut off from the
# ground, where a unit born in the rock stays stuck. A NavArea_Null volume over each boulder, from ROCK_NULL_BOTTOM up
# (above the ground's spans: the ground around the rock keeps its navmesh), its footprint the rock's with a margin.
ROCK_NULL_BOTTOM = 50.0
ROCK_NULL_TOP = 450.0
ROCK_NULL_MARGIN = 100.0
ROCK_COLOR = unreal.LinearColor(0.33, 0.31, 0.29, 1.0)


class Recipe:
    """What one map file is made of: its map (asset name, Plain say), its path, its slots (angle of each, in slot order),
    their circle and its bounds. size_key is None for the fixed test map. layout_seed (C1c): the integer a map's organic
    shapes and irregular placements are drawn from (1 = the layout made now; another gives a variant of the same map)."""

    def __init__(self, map_name, map_path, angles, slot_radius, bounds_half, ground_size, size_key=None, layout_seed=1):
        self.map_name = map_name
        self.map_path = map_path
        self.angles = angles
        self.slot_radius = slot_radius
        self.bounds_half = bounds_half
        self.ground_size = ground_size
        self.size_key = size_key
        self.layout_seed = layout_seed

    def slot(self, index):
        """Place, outward direction and left (seen from the slot, facing the centre: counterclockwise) of a slot."""
        radians = math.radians(self.angles[index])
        radial = (math.cos(radians), math.sin(radians))
        return (self.slot_radius * radial[0], self.slot_radius * radial[1]), radial, (-radial[1], radial[0])


def test_recipe(map_name, layout_seed=1):
    """The fixed 400 m test map of 8 places, /Game/Maps/<Map> (Plain only, D494)."""
    return Recipe(map_name, "%s/%s" % (MAPS_ROOT, map_name), SLOT_ANGLES, SLOT_RADIUS, BOUNDS_HALF_SIZE, GROUND_SIZE, layout_seed=layout_seed)


def sized_recipe(map_name, key, places, side_tiles, layout_seed=1):
    """/Game/Maps/<Map>_<Size> for one row of MapSizes.csv."""
    half = 0.5 * side_tiles * TILE_SIZE
    angles = [(FIRST_SLOT_ANGLE + index * 360.0 / places) % 360.0 for index in range(places)]
    return Recipe(map_name, "%s/%s_%s" % (MAPS_ROOT, map_name, key), angles, SLOT_RADIUS_FRACTION * half, half,
                  2 * (half + GROUND_MARGIN), key, layout_seed)


def data_file(name):
    return os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()), "Tools", "Data", name)


def read_sizes():
    """Tools/Data/MapSizes.csv (TC1, the copy of FHOLMapSizeRules' table): key -> (places, side in tiles)."""
    with open(data_file("MapSizes.csv"), newline="", encoding="utf-8") as file:
        return {row["Name"]: (int(row["NumPlaces"]), int(row["SideTiles"])) for row in csv.DictReader(file)}


def read_maps():
    """Tools/Data/Maps.csv (Name,DisplayName,Recipe,Budget): asset name -> recipe module name under Tools/Editor/maps
    (Plain -> plain). The Budget column (standard / none) is the test's (Map.Table), not the recipe's."""
    with open(data_file("Maps.csv"), newline="", encoding="utf-8") as file:
        return {row["Name"]: row["Recipe"] for row in csv.DictReader(file)}


def spawn(actor_class, location=unreal.Vector(0, 0, 0), rotation=unreal.Rotator(0, 0, 0), label=None):
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actor = actors.spawn_actor_from_class(actor_class, location, rotation)
    if actor is None:
        raise RuntimeError("Could not spawn %s" % actor_class)
    if label:
        actor.set_actor_label(label)
    return actor


def spawn_mesh(mesh_path, location, scale, label, material_path=None):
    actor = spawn(unreal.StaticMeshActor, location, label=label)
    component = actor.get_editor_property("static_mesh_component")
    component.set_static_mesh(unreal.load_asset(mesh_path))
    if material_path:
        component.set_material(0, unreal.load_asset(material_path))
    actor.set_actor_scale3d(scale)
    return actor


def matte_instance(name, color):
    """A matte material instance of M_MatteColor (D265) of one colour, made when missing."""
    path = "/Game/Rendering/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    factory = unreal.MaterialInstanceConstantFactoryNew()
    instance = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, "/Game/Rendering", unreal.MaterialInstanceConstant, factory)
    if instance is None:
        raise RuntimeError("Could not create " + path)
    unreal.MaterialEditingLibrary.set_material_instance_parent(instance, unreal.load_asset("/Game/Rendering/M_MatteColor"))
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(instance, "Color", color)
    unreal.EditorAssetLibrary.save_loaded_asset(instance)
    return instance


def ground_material(module):
    """The ground's material of a map: the grey M_MatteGround, or the varied ground its module names (GROUND, a key of
    maps/ground.py GROUNDS: an instance of M_GroundVaried, made when missing)."""
    name = getattr(module, "GROUND", None)
    return ground.DEFAULT if name is None else ground.instance(name).get_path_name()


def spawn_shape(mesh_path, location, scale, label, material, yaw=0.0):
    actor = spawn(unreal.StaticMeshActor, location, unreal.Rotator(roll=0, pitch=0, yaw=yaw), label)
    component = actor.get_editor_property("static_mesh_component")
    component.set_static_mesh(unreal.load_asset(mesh_path))
    component.set_material(0, material)
    component.set_editor_property("cast_shadow", False)  # D258
    actor.set_actor_scale3d(scale)
    return actor


def spawn_ridges(recipe):
    """The ridge in front of every slot (D286, J4; D439 to D444), the shared piece of the maps that keep the paths
    showing: three boulders across the way from the slot to the middle, RIDGE_AHEAD in front of it, and a NavArea_Null
    volume over each. Returns the number of boulders spawned. Their matte material /Game/Rendering/MI_Rock is made when
    missing."""
    rock = matte_instance("MI_Rock", ROCK_COLOR)
    for index, angle in enumerate(recipe.angles):
        radians = math.radians(angle)
        radial = unreal.Vector(math.cos(radians), math.sin(radians), 0)
        across = unreal.Vector(-radial.y, radial.x, 0)
        for number, (offset, scale) in enumerate(RIDGE_BOULDERS):
            # The engine's 1 m sphere, flattened and half buried.
            location = radial * (recipe.slot_radius - RIDGE_AHEAD) + across * offset
            yaw = angle + 90.0 + (number - 1) * 12.0
            spawn_shape("/Engine/BasicShapes/Sphere", unreal.Vector(location.x, location.y, 0), scale, "Ridge%d_%d" % (index, number),
                        rock, yaw=yaw)
            # The volume's default brush is a 2 m cube centred on the actor.
            null_volume = spawn(unreal.NavModifierVolume, unreal.Vector(location.x, location.y, (ROCK_NULL_BOTTOM + ROCK_NULL_TOP) / 2),
                                unreal.Rotator(roll=0, pitch=0, yaw=yaw), "RidgeNoNav%d_%d" % (index, number))
            null_volume.set_area_class(unreal.NavArea_Null)
            null_volume.set_actor_scale3d(unreal.Vector((scale.x * 50 + ROCK_NULL_MARGIN) / 100, (scale.y * 50 + ROCK_NULL_MARGIN) / 100,
                                                        (ROCK_NULL_TOP - ROCK_NULL_BOTTOM) / 200))
    return len(recipe.angles) * len(RIDGE_BOULDERS)


# ---- Long rock walls (C2, L'Arriere-pays): irregular outcrops, not straight lines. A wall is a polyline of vertices (a pocket's
# boundary, the belt): it is cut into straight pieces of WALL_PIECE, each vertex pushed sideways by a smooth wobble (WALL_WOBBLE),
# every piece carrying a chain of flattened rocks (the engine's sphere, as the ridges) that overlap by a metre or more, and one
# NavArea_Null volume (the rocks' footprint plus a margin, so that a unit never stands in a rock). Gaps (the openings of a pocket, the
# passages of the belt) are free stretches of the wall of a given width between rock ends. Pure geometry: rock_wall returns the data,
# spawn_rock_wall puts it in the level. Deterministic from the integers it is seeded with (Rng).
WALL_PIECE = 2000.0  # cm: one straight piece of a wall, and one navigation volume
WALL_STEP = 330.0  # cm between two rocks of a chain
WALL_WOBBLE = 420.0  # cm: how far the wall wanders off its straight line
WALL_WAVELENGTH = (5500.0, 11000.0)
WALL_JITTER = 70.0  # cm: a rock's offset from the chain's axis
WALL_ROCK_ALONG = (6.6, 9.0)  # scale of the 1 m sphere along the wall: 6.6 to 9 m long
WALL_ROCK_ACROSS = (4.6, 6.0)
WALL_ROCK_HEIGHT = (3.4, 5.4)
WALL_YAW_JITTER = 8.0  # degrees off the chain's direction
WALL_HALF_LENGTH = 450.0  # cm: the farthest a rock of a chain reaches along it beyond its centre
WALL_NAV_HALF_WIDTH = 520.0  # cm: half the width of a piece's NavArea_Null volume (rock reach across + margin)


def polyline_length(points):
    return sum(math.dist(points[index], points[index + 1]) for index in range(len(points) - 1))


def point_along(points, distance):
    """The point of a polyline at an arc length, and the unit direction there."""
    for index in range(len(points) - 1):
        length = math.dist(points[index], points[index + 1])
        if distance <= length or index == len(points) - 2:
            t = min(1.0, distance / length) if length > 0.0 else 0.0
            direction = ((points[index + 1][0] - points[index][0]) / length, (points[index + 1][1] - points[index][1]) / length)
            return (points[index][0] + direction[0] * length * t, points[index][1] + direction[1] * length * t), direction
        distance -= length
    return points[-1], (1.0, 0.0)


def rock_wall(seed, points, gaps=()):
    """The rocks and navigation volumes of one wall along the polyline points (world xy, cm). gaps: (centre, width) pairs by arc
    length along the polyline: the wall is left open for width cm around centre (a unit-wide gap with rounded rock ends: the rocks
    stop WALL_HALF_LENGTH short of the gap's edges). Returns (rocks, volumes): rocks are (x, y, scale x, scale y, scale z, yaw)
    with x along the wall; volumes are (x, y, length, half width, yaw) of straight pieces (a box centred there, turned by yaw)."""
    rng = Rng(seed, 2)
    total = polyline_length(points)
    # The wobble: two cosines of the arc length, zero at the vertices of the polyline (they stay where they are).
    waves = [(rng.range(*WALL_WAVELENGTH), rng.range(0.0, 2 * math.pi), rng.range(0.6, 1.0)) for _ in range(2)]

    corners = [0.0, total]
    run = 0.0
    for index in range(len(points) - 1):
        run += math.dist(points[index], points[index + 1])
        corners.append(run)

    def wobble(distance):
        # Straight at a vertex and at a gap (the gap's marker and the wall's ends are where the recipe says), wandering between.
        nearest = min([abs(distance - corner) for corner in corners] + [max(0.0, abs(distance - centre) - width / 2.0) for centre, width in gaps])
        ends = min(1.0, nearest / 1500.0)
        return WALL_WOBBLE * ends * sum(a * math.cos(2 * math.pi * distance / w + p) for w, p, a in waves) / 1.6

    pieces = max(1, int(round(total / WALL_PIECE)))
    cuts = [total * index / pieces for index in range(pieces + 1)]
    # Vertices of the polyline are cuts too, so that a corner is a joint, not a wobble.
    run = 0.0
    for index in range(len(points) - 1):
        run += math.dist(points[index], points[index + 1])
        if 100.0 < run < total - 100.0:
            cuts.append(run)
    cuts = sorted(cuts)
    kept = []
    for cut in cuts:
        if not kept or cut - kept[-1] > 300.0:
            kept.append(cut)
    cuts = kept

    def pushed(distance):
        point, direction = point_along(points, distance)
        shift = wobble(distance)
        return (point[0] - direction[1] * shift, point[1] + direction[0] * shift)

    open_ranges = [(centre - width / 2.0 - WALL_HALF_LENGTH, centre + width / 2.0 + WALL_HALF_LENGTH) for centre, width in gaps]
    rocks, volumes = [], []
    for index in range(len(cuts) - 1):
        start, end = cuts[index], cuts[index + 1]
        # The stretches of this piece outside the gaps' ranges.
        stretches = [(start, end)]
        for low, high in open_ranges:
            stretches = [part for lo, hi in stretches for part in ((lo, min(hi, low)), (max(lo, high), hi)) if part[1] - part[0] > 1.0]
        for lo, hi in stretches:
            a, b = pushed(lo), pushed(hi)
            length = math.dist(a, b)
            yaw = math.degrees(math.atan2(b[1] - a[1], b[0] - a[0]))
            ends_open = (lo > start + 1.0) or (hi < end - 1.0)
            count = max(1, int(math.ceil(length / WALL_STEP)))
            # A stretch that ends at a gap (or a short one) has a rock at both ends; inside a wall the rocks sit half a step in
            # from the joints, so that the chain goes on from one piece to the next one step apart.
            both_ends = ends_open or length < WALL_STEP
            for number in range(count + 1 if both_ends else count):
                t = (number if both_ends else number + 0.5) / count
                x = a[0] + (b[0] - a[0]) * t
                y = a[1] + (b[1] - a[1]) * t
                side = rng.range(-WALL_JITTER, WALL_JITTER)
                rocks.append((x - math.sin(math.radians(yaw)) * side, y + math.cos(math.radians(yaw)) * side,
                              rng.range(*WALL_ROCK_ALONG), rng.range(*WALL_ROCK_ACROSS), rng.range(*WALL_ROCK_HEIGHT),
                              yaw + rng.range(-WALL_YAW_JITTER, WALL_YAW_JITTER)))
            # The volume covers the chain with the rocks' reach beyond its ends.
            volumes.append(((a[0] + b[0]) / 2.0, (a[1] + b[1]) / 2.0, length + 2 * WALL_HALF_LENGTH, WALL_NAV_HALF_WIDTH, yaw))
    return rocks, volumes


def spawn_rock_wall(rocks, volumes, material, name):
    """The rocks (StaticMeshActors of the engine's sphere, half buried, no shadow) and the NavArea_Null volumes of rock_wall."""
    for number, (x, y, scale_x, scale_y, scale_z, yaw) in enumerate(rocks):
        spawn_shape("/Engine/BasicShapes/Sphere", unreal.Vector(x, y, 0), unreal.Vector(scale_x, scale_y, scale_z), "%sRock%d" % (name, number), material, yaw=yaw)
    for number, (x, y, length, half_width, yaw) in enumerate(volumes):
        volume = spawn(unreal.NavModifierVolume, unreal.Vector(x, y, (ROCK_NULL_BOTTOM + ROCK_NULL_TOP) / 2), unreal.Rotator(roll=0, pitch=0, yaw=yaw),
                       "%sNoNav%d" % (name, number))
        volume.set_area_class(unreal.NavArea_Null)
        volume.set_actor_scale3d(unreal.Vector(length / 2.0 / 100.0, half_width / 100.0, (ROCK_NULL_TOP - ROCK_NULL_BOTTOM) / 200))
    return len(rocks) + len(volumes)


# Scenery (C1b, M-Q5: for every map; the map module opts in with DECOR = {kind: instances per hectare}): non-colliding
# low meshes of the engine's basic shapes (no new art), scattered over the whole bounded square, the corners too, none within
# DECOR_CLEAR of a slot, a source or a rock. One actor, one instanced mesh component per kind (one draw call each), no
# collision, no navigation, no shadow (D258), drawn up to DECOR_CULL from the camera. Deterministic (seeded, in patches: decor_transforms).
DECOR_CLEAR = 800.0
DECOR_CULL = 12000.0  # cm: the instances farther than this are not drawn (they start to thin out at 2/3 of it)
# kind -> (mesh, matte colour, scale of the 1 m shape (x, y, z), where its centre sits: the share of its half height above the ground)
DECOR_KINDS = {
    "GrassGreen": ("/Engine/BasicShapes/Cone", unreal.LinearColor(0.075, 0.115, 0.035, 1.0), (0.55, 0.55, 0.60), 1.0),
    "GrassStraw": ("/Engine/BasicShapes/Cone", unreal.LinearColor(0.20, 0.17, 0.075, 1.0), (0.50, 0.50, 0.75), 1.0),
    "Bush": ("/Engine/BasicShapes/Sphere", unreal.LinearColor(0.11, 0.09, 0.04, 1.0), (1.50, 1.50, 0.70), 0.3),
    "Stone": ("/Engine/BasicShapes/Sphere", unreal.LinearColor(0.27, 0.255, 0.23, 1.0), (0.70, 0.55, 0.35), 0.2),
    "Stump": ("/Engine/BasicShapes/Cylinder", unreal.LinearColor(0.11, 0.075, 0.045, 1.0), (0.50, 0.50, 0.45), 1.0),
    # The big pieces, seen from the camera's height (M-Q7: nothing may be empty): a boulder 2.6 m long, a thicket 3.6 m across. They are
    # what fill_holes puts where a map has nothing within MAX_EMPTY_RADIUS; a map's DECOR may ask for some too.
    "Boulder": ("/Engine/BasicShapes/Sphere", unreal.LinearColor(0.24, 0.23, 0.21, 1.0), (2.60, 2.00, 1.50), 0.35),
    "Thicket": ("/Engine/BasicShapes/Sphere", unreal.LinearColor(0.045, 0.080, 0.028, 1.0), (3.60, 3.20, 1.60), 0.35),
    # M-Q8 (the user: "the map looks desolate", a reference minimap full of small different clumps): more kinds, none a resource (nothing the
    # players could take for gold, stone or berries: no yellow lumps, no red bushes). Their names carry Boulder / Thicket when they are big
    # pieces: Map.Table counts those as landmarks (M-Q7), like the first two.
    "DryThicket": ("/Engine/BasicShapes/Sphere", unreal.LinearColor(0.150, 0.100, 0.040, 1.0), (3.20, 2.80, 1.30), 0.35),
    "SandBoulder": ("/Engine/BasicShapes/Sphere", unreal.LinearColor(0.330, 0.230, 0.110, 1.0), (2.40, 1.90, 1.30), 0.35),
    "MossBoulder": ("/Engine/BasicShapes/Sphere", unreal.LinearColor(0.100, 0.140, 0.070, 1.0), (2.80, 2.20, 1.40), 0.35),
    "FlowerWhite": ("/Engine/BasicShapes/Sphere", unreal.LinearColor(0.450, 0.450, 0.400, 1.0), (0.45, 0.45, 0.28), 0.5),
    "FlowerPurple": ("/Engine/BasicShapes/Sphere", unreal.LinearColor(0.220, 0.070, 0.300, 1.0), (0.45, 0.45, 0.28), 0.5),
    "FlowerOrange": ("/Engine/BasicShapes/Sphere", unreal.LinearColor(0.550, 0.180, 0.030, 1.0), (0.45, 0.45, 0.28), 0.5),
    "Reed": ("/Engine/BasicShapes/Cylinder", unreal.LinearColor(0.120, 0.140, 0.050, 1.0), (0.14, 0.14, 1.60), 1.0),
}
# The clumps (M-Q8): small mixed groups of scenery, a few metres across, scattered over the map like the reference minimap's patches of
# bushes, stones, flowers: name -> (radius in cm, [(kind, how many)]). A map module opts in with CLUMPS = {name: clumps per hectare}.
DECOR_CLUMPS = {
    "Meadow": (800.0, [("FlowerWhite", 8), ("FlowerPurple", 7), ("FlowerOrange", 5), ("GrassGreen", 7)]),
    "Cairn": (500.0, [("SandBoulder", 1), ("Boulder", 1), ("Stone", 5)]),
    "DryPatch": (700.0, [("DryThicket", 2), ("GrassStraw", 9), ("Stump", 2)]),
    "Copse": (650.0, [("MossBoulder", 1), ("Thicket", 1), ("GrassGreen", 6), ("Bush", 2)]),
    "Reeds": (600.0, [("Reed", 14), ("GrassGreen", 5), ("Stone", 2)]),
}
# fill_holes' clusters (M-Q7), by turns: the three big pieces of each.
FILL_STYLES = [("Thicket", "Boulder", "Boulder"), ("DryThicket", "SandBoulder", "SandBoulder"), ("Thicket", "MossBoulder", "Boulder"),
               ("DryThicket", "Boulder", "SandBoulder"), ("Thicket", "MossBoulder", "MossBoulder")]
BIG_KINDS = ("Boulder", "Thicket")  # a kind whose name contains one of these is a landmark
# M-Q7 (2026-10-30) asked that no point of the square be farther than MAX_EMPTY_RADIUS from a landmark; M-Q10b = A (D578) WITHDREW the rule
# (large empty spaces are accepted; Map.Table only measures the largest empty circle). Only a map that still has a DECOR uses fill_holes, which puts
# a cluster of the scenery's big pieces wherever a point of a 10 m grid is farther than FILL_RADIUS (26 m); the maps without DECOR are not filled.
MAX_EMPTY_RADIUS = 2800.0  # kept only as the size fill_holes aims at (no test enforces it any more)
FILL_RADIUS = 2600.0
FILL_STEP = 1000.0


def ridge_points(recipe):
    """The centres of the ridges' boulders (spawn_ridges), as xy: three in front of every slot."""
    points = []
    for index in range(len(recipe.angles)):
        slot, radial, left = recipe.slot(index)
        for offset, _ in RIDGE_BOULDERS:
            points.append((slot[0] - radial[0] * RIDGE_AHEAD + left[0] * offset, slot[1] - radial[1] * RIDGE_AHEAD + left[1] * offset))
    return points


def decor_transforms(recipe, avoid, per_hectare, salt):
    """The places of one kind of scenery: per_hectare of the bounded square, drawn from recipe.layout_seed and the kind's salt, in
    patches (a smooth noise thins some stretches out and crowds others: tufts, not a sprinkle), leaving out those within DECOR_CLEAR of
    any avoid point. Returns (x, y, turn in 0..1, size in 0..1) tuples."""
    half = recipe.bounds_half
    count = int(per_hectare * (2 * half) ** 2 / 1.0e8)
    cell = DECOR_CLEAR
    grid = {}
    for point in avoid:
        grid.setdefault((int(point[0] // cell), int(point[1] // cell)), []).append(point)
    rng = Rng(recipe.layout_seed, 71, salt)
    patches = Noise(mix_seed(recipe.layout_seed, 72, salt), waves=3, shortest=2500.0, longest=9000.0)
    places = []
    for _ in range(30 * count):
        if len(places) >= count:
            break
        x = rng.range(-half, half)
        y = rng.range(-half, half)
        if rng.uniform() > 0.2 + 0.8 * (0.5 + 0.5 * patches.value(x, y)):
            continue
        cx, cy = int(x // cell), int(y // cell)
        if any(math.hypot(x - px, y - py) < DECOR_CLEAR for dx in (-1, 0, 1) for dy in (-1, 0, 1) for px, py in grid.get((cx + dx, cy + dy), ())):
            continue
        places.append((x, y, rng.uniform(), rng.uniform()))
    return places


def add_component(actor, component_class):
    """A new component of a placed actor (an instance component, saved with the map), through the subobject subsystem."""
    subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = subsystem.k2_gather_subobject_data_for_instance(actor)
    parameters = unreal.AddNewSubobjectParams(parent_handle=handles[0], new_class=component_class, blueprint_context=None)
    handle, failure = subsystem.add_new_subobject(parameters)
    if not subsystem.k2_find_subobject_data_from_handle(handle)[0] if False else False:
        pass
    component = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(subsystem.k2_find_subobject_data_from_handle(handle))
    if component is None:
        raise RuntimeError("Could not add a %s to %s: %s" % (component_class, actor.get_name(), failure))
    return component


def fill_holes(recipe, landmarks):
    """The clusters that make the map hold M-Q7: a sweep of the square on a grid of FILL_STEP from the corner of the bounds, a cluster
    (a thicket and two boulders) at the first point farther than FILL_RADIUS from every landmark, and from the clusters already made. Returns
    {kind: [(x, y, turn, size)]}."""
    half = recipe.bounds_half
    cells = int((2 * half) // FILL_STEP) + 1
    reach = int(FILL_RADIUS // FILL_STEP) + 1
    near = [[math.inf] * cells for _ in range(cells)]

    def add(point):
        i0 = int(round((point[0] + half) / FILL_STEP))
        j0 = int(round((point[1] + half) / FILL_STEP))
        for i in range(max(0, i0 - reach), min(cells, i0 + reach + 1)):
            for j in range(max(0, j0 - reach), min(cells, j0 + reach + 1)):
                d = math.hypot(i * FILL_STEP - half - point[0], j * FILL_STEP - half - point[1])
                if d < near[i][j]:
                    near[i][j] = d

    for point in landmarks:
        add(point)
    rng = Rng(recipe.layout_seed, 91)
    made = {}
    for i in range(cells):
        for j in range(cells):
            if near[i][j] <= FILL_RADIUS:
                continue
            x = max(-half + 200.0, min(half - 200.0, i * FILL_STEP - half + rng.range(-300.0, 300.0)))
            y = max(-half + 200.0, min(half - 200.0, j * FILL_STEP - half + rng.range(-300.0, 300.0)))
            style = FILL_STYLES[rng.below(len(FILL_STYLES))]
            made.setdefault(style[0], []).append((x, y, rng.uniform(), rng.uniform()))
            for kind in style[1:]:
                angle = rng.range(0.0, 2 * math.pi)
                made.setdefault(kind, []).append((max(-half + 200.0, min(half - 200.0, x + 450.0 * math.cos(angle))), max(-half + 200.0, min(half - 200.0, y + 450.0 * math.sin(angle))),
                                                  rng.uniform(), rng.uniform()))
            add((x, y))
    return made


def clump_transforms(recipe, avoid, per_hectare, salt, name):
    """The members of the clumps of one kind of DECOR_CLUMPS (per_hectare of them per hectare, centres drawn like decor_transforms'), as
    {kind: [(x, y, turn, size)]}: each member at a drawn offset within the clump's radius, none within DECOR_CLEAR of an avoid point."""
    radius, members = DECOR_CLUMPS[name]
    rng = Rng(recipe.layout_seed, 73, salt)
    half = recipe.bounds_half
    grid = {}
    for point in avoid:
        grid.setdefault((int(point[0] // DECOR_CLEAR), int(point[1] // DECOR_CLEAR)), []).append(point)
    made = {}
    for cx, cy, _, _ in decor_transforms(recipe, avoid, per_hectare, salt):
        for kind, count in members:
            for _ in range(count):
                angle = rng.range(0.0, 2 * math.pi)
                distance = radius * math.sqrt(rng.uniform())
                x, y = cx + distance * math.cos(angle), cy + distance * math.sin(angle)
                if abs(x) > half - 100.0 or abs(y) > half - 100.0:
                    continue
                gx, gy = int(x // DECOR_CLEAR), int(y // DECOR_CLEAR)
                if any(math.hypot(x - px, y - py) < DECOR_CLEAR for dx in (-1, 0, 1) for dy in (-1, 0, 1) for px, py in grid.get((gx + dx, gy + dy), ())):
                    continue
                made.setdefault(kind, []).append((x, y, rng.uniform(), rng.uniform()))
    return made


def is_big(kind):
    return any(word in kind for word in BIG_KINDS)


def spawn_decor(recipe, density, sources, extra_avoid=(), clumps=None):
    """The scenery of a map (density: kind -> instances per hectare, keys of DECOR_KINDS): one actor, one instanced mesh
    component per kind, no collision, no navigation, no shadow, a cull distance; then the clusters of Boulders and Thickets that fill
    the holes (fill_holes, M-Q7). Returns {kind: instances}. extra_avoid: more points to keep clear (a map's own rocks) and that count
    as landmarks."""
    avoid = [recipe.slot(index)[0] for index in range(len(recipe.angles))] + ridge_points(recipe) + list(extra_avoid)
    avoid += [(source.get_editor_property("location").x, source.get_editor_property("location").y) for source in sources]
    actor = spawn(unreal.StaticMeshActor, label="Decor")
    actor.get_editor_property("static_mesh_component").set_mobility(unreal.ComponentMobility.STATIC)
    places = {}
    for number, kind in enumerate(sorted(density)):
        places[kind] = decor_transforms(recipe, avoid, density[kind], 1000 * (number + 1))
    for number, name in enumerate(sorted(clumps or {})):
        for kind, members in clump_transforms(recipe, avoid, clumps[name], 5000 + 100 * number, name).items():
            places[kind] = places.get(kind, []) + members
    landmarks = list(avoid) + [(x, y) for kind in places if is_big(kind) for x, y, _, _ in places[kind]]
    for kind, extra in fill_holes(recipe, landmarks).items():
        places[kind] = places.get(kind, []) + extra
    made = {}
    for kind in sorted(places):
        mesh, color, scale, lift = DECOR_KINDS[kind]
        component = add_component(actor, unreal.HierarchicalInstancedStaticMeshComponent)
        component.rename("Decor_" + kind)
        component.set_static_mesh(unreal.load_asset(mesh))
        component.set_material(0, matte_instance("MI_Decor_" + kind, color))
        component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        component.set_collision_profile_name("NoCollision")
        component.set_editor_property("can_ever_affect_navigation", False)
        component.set_editor_property("generate_overlap_events", False)
        component.set_editor_property("cast_shadow", False)  # D258
        component.set_editor_property("receives_decals", False)
        component.set_editor_property("instance_start_cull_distance", int(DECOR_CULL * 2 / 3))
        component.set_editor_property("instance_end_cull_distance", int(DECOR_CULL))
        transforms = []
        for x, y, turn, size in places[kind]:
            grow = 0.75 + 0.5 * size  # 0.75 to 1.25 of the kind's scale
            height = scale[2] * grow
            transforms.append(unreal.Transform(unreal.Vector(x, y, 50.0 * height * lift),
                                               unreal.Rotator(roll=0, pitch=0, yaw=360.0 * turn),
                                               unreal.Vector(scale[0] * grow, scale[1] * grow, height)))
        component.add_instances(transforms, False)
        made[kind] = len(transforms)
    return made


def movable_light(actor):
    """The light component of a light actor, made movable (nothing to build)."""
    component = actor.get_editor_property("light_component")
    component.set_mobility(unreal.ComponentMobility.MOVABLE)
    return component


def placed_source(type_name, x, y):
    source = unreal.HOLPlacedSource()
    source.set_editor_property("type", type_name)
    source.set_editor_property("location", unreal.Vector(x, y, 0))
    return source


# ---- Gold and stone veins in several sizes (M-Q9, the user, 2026-10-30: "small AND big", real game things in the middle, not decor).
# Four sizes of a vein, each a row of Tools/Data/SourceTypes.csv: Small 500, Medium 1 000, Standard 2 500 (the former GoldVein / StoneVein),
# Big 5 000. The value of a territory is the M-Q4 one (4 gold veins of 2 500 = 10 000 gold, 2 stone veins of 2 500 = 5 000 stone; Huge x1.5,
# M-Q4b) but it comes as a MIX of sizes, so that a territory holds MINERAL_MIX piles: a few big ones far out, mediums, many small ones near
# the middle and the belts. Map.Table checks the value, the number of piles and the variety on the saved files (HOLMapTableTests.cpp).
MINERAL_AMOUNTS = {"Small": 2000, "Medium": 2500, "Standard": 3000, "Big": 4000}  # 2026-11-05 (the user, Q-Maps-Budget = B: "a mini mine is 2000, a big one 4000", four amounts kept, the places kept; was 500 / 1000 / 2500 / 5000): per territory 21 000 gold, 11 500 stone (Huge 25 000 / 12 500)
MINERAL_RESOURCES = ("Gold", "Stone")
MINERAL_MIX = {
    "default": {"Gold": ["Big", "Medium", "Medium"] + ["Small"] * 6, "Stone": ["Standard", "Medium"] + ["Small"] * 3},   # 10 000 gold, 5 000 stone
    "Huge": {"Gold": ["Big", "Big", "Medium", "Medium"] + ["Small"] * 6, "Stone": ["Big", "Medium"] + ["Small"] * 3},    # 15 000 gold, 7 500 stone
}
# The tiers of the three veins of each resource nearest a base (the near layer, 110 m to 125 m: the "protected start"), the same at every place.
MINERAL_NEAR = {
    "default": {"Gold": ["Medium", "Small", "Small"], "Stone": ["Standard", "Small", "Small"]},
    "Huge": {"Gold": ["Medium", "Small", "Small"], "Stone": ["Medium", "Small", "Small"]},
}


def mineral_type(resource, tier):
    """The source type of a vein: GoldVeinSmall, StoneVeinMedium, GoldVein (the standard size), StoneVeinBig..."""
    return resource + "Vein" + ("" if tier == "Standard" else tier)


def mineral_mix(recipe):
    """{"Gold": tiers, "Stone": tiers}: the piles of each territory, by size (the value budget of M-Q9)."""
    return MINERAL_MIX["Huge" if recipe.size_key == "Huge" else "default"]


def mineral_near(recipe):
    """{"Gold": tiers, "Stone": tiers}: the sizes of the three veins of each resource nearest a base."""
    return MINERAL_NEAR["Huge" if recipe.size_key == "Huge" else "default"]


def mineral_far(recipe):
    """[(resource, tier)] of the piles of a territory beyond the near layer, biggest first: the mix less the near ones."""
    far = []
    for resource in MINERAL_RESOURCES:
        left = list(mineral_mix(recipe)[resource])
        for tier in mineral_near(recipe)[resource]:
            left.remove(tier)
        far += [(resource, tier) for tier in left]
    return sorted(far, key=lambda item: (-MINERAL_AMOUNTS[item[1]], item[0]))


def mineral_value(recipe, resource):
    """The value (amount of gold or of stone) a territory holds by its mix."""
    return sum(MINERAL_AMOUNTS[tier] for tier in mineral_mix(recipe)[resource])


def turn(point, angle):
    """A point turned by angle (radians) round the map's centre."""
    return (point[0] * math.cos(angle) - point[1] * math.sin(angle), point[0] * math.sin(angle) + point[1] * math.cos(angle))


def wood_points(centre, along_axis, side_axis, rows, columns, first_side):
    """The places of a wood's trees: rows on side_axis from first_side, columns along along_axis centred on centre."""
    points = []
    number = 0
    for row in range(rows):
        for column in range(columns):
            side = first_side + row * WOOD_SPACING
            along = (column - (columns - 1) / 2.0) * WOOD_SPACING
            jitter = WOOD_JITTER[number % len(WOOD_JITTER)]
            number += 1
            side += jitter[0]
            along += jitter[1]
            points.append((centre[0] + side_axis[0] * side + along_axis[0] * along, centre[1] + side_axis[1] * side + along_axis[1] * along))
    return points


def wood(centre, along_axis, side_axis, rows, columns, first_side):
    """The trees of a wood (wood_points) as sources."""
    return [placed_source("Tree", x, y) for x, y in wood_points(centre, along_axis, side_axis, rows, columns, first_side)]


# ---- Natural-looking groves and patches (C1c). Plain keeps its grid wood above; a map that wants organic woods (the
# Steppe) uses these. Everything is deterministic from integers: a small xorshift generator written here (no `random`, no
# state shared with anything), so a map file made twice, on any Python, is the same file.
GROVE_SPACING = float(os.environ.get("HOL_GROVE_SPACING", "80"))  # cm: 2026-11-01 (the user, REGLAGE after tree_spacing_compare: 0.8 m, the trunks of 60 cm radius overlap); 2026-10-31 (the user, after the tree_spacing trial: "a forest is trees that touch, compact, a minimum of white"; it was 160, 220 before): the closest two neighbouring trunks of a grove (60 cm radius, so they touch at 120) get; HOL_GROVE_SPACING sets it for the variants
GROVE_SPACING_SLACK = 10.0  # the spacing asked of a candidate tree is GROVE_SPACING plus up to this much
GROVE_NEVER = 80.0  # no two trees closer than this, whatever (2026-11-01: 0.8 m, trunks of 60 cm overlap; was 120, they touched)
GROVE_AREA = 1.09 * (GROVE_SPACING + GROVE_SPACING_SLACK / 3.0) ** 2  # cm2 a tree takes up in a blob (a dense random packing of trunks GROVE_SPACING apart: 29 600 at 1.6 to 1.74 m, 56 000 at 2.2 to 2.4 m)
GROVE_GLADES = os.environ.get("HOL_GROVE_GLADES", "1") != "0"  # a grove of 12 or more has a glade or a 3 m path through it (the default); HOL_GROVE_GLADES=0 makes them solid blocks of trees (question in Auto_Journey_Retours.md, 2026-10-31)
GLADE_RADIUS = (250.0, 380.0)
# 2026-11-07 (E-maps-review defect 5: "forests are solid blobs, the glades are nearly invisible"): the knobs of the glade sweep (maps/glade_sweep.py,
# scenario glade_sweep). GLADE_FORCE 0 (the default) = the draws below, unchanged: every map file stays the same. GLADE_FORCE N > 0: a grove of 12 or
# more has exactly N glades (no path), each of radius GLADE_RADIUS x GLADE_SCALE, centred up to 55 % of the way out. Nothing sets them in a match map.
GLADE_SCALE = float(os.environ.get("HOL_GLADE_SCALE", "1"))
GLADE_FORCE = int(os.environ.get("HOL_GLADE_FORCE", "0"))
GLADE_REACH = float(os.environ.get("HOL_GLADE_REACH", "0.55"))  # with GLADE_FORCE: how far out (fraction of the blob) a glade may be centred; far out it breaks the outline (a crescent)
PATH_HALF_WIDTH = 150.0  # a path: no trunk centre within this of its axis, so 3 m between the centres, 1.8 m between the trunks
BERRY_MIN = 300.0  # cm between the centres of two bushes of 1.1 m radius: 80 cm between the edges, closed to a unit
BERRY_MAX = 335.0
MASK32 = 0xFFFFFFFF


def mix_seed(*values):
    """One 32 bit seed from some integers (murmur-like finalizer: close inputs give unrelated seeds)."""
    h = 0x9E3779B9
    for value in values:
        h ^= int(value) & MASK32
        h = (h * 0x85EBCA6B) & MASK32
        h ^= h >> 13
        h = (h * 0xC2B2AE35) & MASK32
        h ^= h >> 16
    return h or 1


class Rng:
    """xorshift32, seeded from integers: the same numbers on every Python, no global state."""

    def __init__(self, *seed):
        self.state = mix_seed(*seed)
        for _ in range(4):
            self.next_u32()

    def next_u32(self):
        x = self.state
        x ^= (x << 13) & MASK32
        x ^= x >> 17
        x ^= (x << 5) & MASK32
        self.state = x
        return x

    def uniform(self):
        """In [0, 1)."""
        return self.next_u32() / 4294967296.0

    def range(self, low, high):
        return low + (high - low) * self.uniform()

    def below(self, count):
        """An integer in 0..count-1."""
        return min(count - 1, int(self.uniform() * count))

    def chance(self, probability):
        return self.uniform() < probability

    def shuffle(self, items):
        """The list, shuffled in place (Fisher-Yates); returns it."""
        for index in range(len(items) - 1, 0, -1):
            other = self.below(index + 1)
            items[index], items[other] = items[other], items[index]
        return items


class Noise:
    """A smooth field in about [-1, 1] over the plane: a few cosines of random direction, wavelength and phase."""

    def __init__(self, seed, waves=4, shortest=6000.0, longest=22000.0):
        rng = Rng(seed, 5)
        self.waves = [(rng.range(0.0, 2 * math.pi), 2 * math.pi / rng.range(shortest, longest), rng.range(0.0, 2 * math.pi)) for _ in range(waves)]

    def value(self, x, y):
        total = sum(math.cos(frequency * (x * math.cos(angle) + y * math.sin(angle)) + phase) for angle, frequency, phase in self.waves)
        return max(-1.0, min(1.0, total / math.sqrt(0.5 * len(self.waves))))


def big_room(count):
    """2026-11-04: the room a tree of a REAL forest (60 trees or more) is given, compared with a small grove's (random sprouting jams at that density
    otherwise): 1 up to 60 trees, 1.5 from there."""
    return 1.0 if count <= 60 else 1.5


def grove_reach(count):
    """How far from its centre the farthest tree of a grove of count trees may be (grove_local keeps to it)."""
    return 1.3 * math.sqrt(count * GROVE_AREA * big_room(count) / math.pi) + 90.0


def grove_long(count):
    """How far the farthest tree of a grove of count trees may be when the room allows it (a stretched or bent grove)."""
    return 2.0 * math.sqrt(count * GROVE_AREA * big_room(count) / math.pi) + 120.0


def berry_reach(count):
    """The same for a patch of count berry bushes."""
    return 150.0 + 190.0 * math.sqrt(count)


def rotate_local(points, axis):
    """Points in a frame whose first axis is the unit vector axis (the second is its left), as offsets in the world."""
    return [(x * axis[0] - y * axis[1], x * axis[1] + y * axis[0]) for x, y in points]


def centred(points):
    cx = sum(x for x, _ in points) / len(points)
    cy = sum(y for _, y in points) / len(points)
    return [(x - cx, y - cy) for x, y in points]


def grove_attempt(rng, count, compact):
    """One try at a grove of count trees around (0, 0), or None when the dart throwing did not find the room. An irregular
    blob (its radius a sum of three harmonics of the angle; stretched into an ellipse, bent into a kidney, some of the time),
    trees thrown into it one by one not closer than the spacing to another, denser towards the core, a glade and a path
    cut into the bigger ones, and a few lone trees and clumps just outside. compact: the plain blob, for the last tries."""
    lone, clumps = 0, []
    if not compact and count >= 10:
        lone = rng.below(3)
        most = 1 if count < 16 else (2 if count < 20 else 3)
        clumps = [2 + rng.below(2) for _ in range(rng.below(most + 1) if count < 12 else 1 + rng.below(most))]
        while lone + sum(clumps) > 0.28 * count:
            if clumps:
                clumps.pop()
            else:
                lone -= 1
    blob = count - lone - sum(clumps)
    elongated = not compact and rng.chance(0.35)
    aspect = rng.range(1.4, 2.3) if elongated else rng.range(1.0, 1.25)
    bend = rng.range(-0.4, 0.4) if (not compact and rng.chance(0.45)) else 0.0
    wobble = 0.4 if compact else 1.0
    harmonics = [(k, rng.range(0.15, 0.35) * wobble * (1.0, 0.8, 0.6)[k - 1], rng.range(0.0, 2 * math.pi)) for k in (1, 2, 3)]

    def outline(angle):
        return max(0.5, min(1.7, 1.0 + sum(amplitude * math.cos(k * angle + phase) for k, amplitude, phase in harmonics)))

    mean_square = sum(outline(2 * math.pi * i / 72) ** 2 for i in range(72)) / 72
    # What is cut out of the blob (a glade, a path) is made up for with a bigger blob.
    glades, paths = [], []
    cut_area = 0.0
    if not compact and count >= 12 and GROVE_GLADES:
        for _ in range(GLADE_FORCE if GLADE_FORCE > 0 else 1 + (1 if count >= 16 and rng.chance(0.4) else 0)):
            if GLADE_FORCE > 0 or rng.chance(0.5):
                glades.append((rng.range(0.0, GLADE_REACH if GLADE_FORCE > 0 else 0.4), rng.range(0.0, 2 * math.pi), rng.range(*GLADE_RADIUS) * GLADE_SCALE * (0.8 if count < 16 else 1.0)))
                cut_area += math.pi * glades[-1][2] ** 2
            else:
                paths.append((rng.range(0.0, 0.4), rng.range(0.0, 2 * math.pi), rng.range(0.0, 2 * math.pi), rng.chance(0.5)))
    radius = math.sqrt((blob + cut_area / GROVE_AREA) * GROVE_AREA * big_room(count) / math.pi / mean_square)
    for _, _, _, through in paths:
        radius = math.sqrt(radius ** 2 + (2 if through else 1) * 2 * PATH_HALF_WIDTH * radius / math.pi)
    semi_a, semi_b = radius * math.sqrt(aspect), radius / math.sqrt(aspect)

    def world(rho, angle):
        x = semi_a * rho * math.cos(angle)
        return x, semi_b * rho * math.sin(angle) + bend * x * x / semi_a

    middle = world(0.0, 0.0)

    def inside(x, y):
        # Back to the unit blob: undo the bend, then the stretch.
        u = x / semi_a
        v = (y - bend * x * x / semi_a) / semi_b
        rho = math.hypot(u, v)
        return rho <= outline(math.atan2(v, u)) / math.sqrt(mean_square)

    cuts = [(world(fraction, angle), size) for fraction, angle, size in glades]
    corridors = []  # (point, unit direction, both ways)
    for fraction, angle, heading, through in paths:
        corridors.append((world(fraction, angle), (math.cos(heading), math.sin(heading)), through))

    def cut_out(x, y, margin=0.0):
        if any(math.hypot(x - at[0], y - at[1]) < size + margin for at, size in cuts):
            return True
        for at, direction, through in corridors:
            along = (x - at[0]) * direction[0] + (y - at[1]) * direction[1]
            if along < 0.0 and not through:
                if math.hypot(x - at[0], y - at[1]) < PATH_HALF_WIDTH + margin:
                    return True
                continue
            if abs((x - at[0]) * direction[1] - (y - at[1]) * direction[0]) < PATH_HALF_WIDTH + margin:
                return True
        return False

    points = []

    def free(x, y, apart):
        return all((x - px) ** 2 + (y - py) ** 2 >= apart * apart for px, py in points)

    # Trees sprout one from another: the next one 220 to ~240 cm from a tree already there (a wall: trunks closer than 2.4 m
    # leave no way through), farther apart towards the edge where the grove thins out, always at least the spacing from every
    # other, and only inside the outline. A split grove (a path across) starts again from a free throw when it stalls.
    tries, stalled = 0, 0
    while len(points) < blob:
        tries += 1
        stalled += 1
        if tries > max(12000, 250 * count):
            return None
        spacing = GROVE_SPACING + GROVE_SPACING_SLACK * rng.uniform() ** 2
        if points and stalled < 300:
            # Two drawn, the one nearer the middle sprouts: a plump blob, not a branching tree.
            first, second = points[rng.below(len(points))], points[rng.below(len(points))]
            parent = min(first, second, key=lambda p: math.hypot(p[0] - middle[0], p[1] - middle[1]))
            angle = rng.range(0.0, 2 * math.pi)
            share_parent = math.hypot(parent[0] - middle[0], parent[1] - middle[1]) / (radius * 1.2)
            step = spacing + 8.0 + 28.0 * min(1.0, share_parent) ** 2 * rng.uniform()
            x, y = parent[0] + step * math.cos(angle), parent[1] + step * math.sin(angle)
        else:
            angle = rng.range(0.0, 2 * math.pi)
            share = math.sqrt(rng.uniform())
            x, y = world(share * outline(angle) / math.sqrt(mean_square), angle)
        if not inside(x, y) or cut_out(x, y) or not free(x, y, spacing):
            continue
        points.append((x, y))
        stalled = 0
    # The lone trees and the clumps just outside: from one of the outer trees, outwards, never closer than 3 m to another.
    middle = (sum(x for x, _ in points) / len(points), sum(y for _, y in points) / len(points))
    outer = sorted(points, key=lambda p: -math.hypot(p[0] - middle[0], p[1] - middle[1]))[:max(3, len(points) // 3)]
    for size in [1] * lone + clumps:
        for _ in range(200):
            parent = outer[rng.below(len(outer))]
            away = math.atan2(parent[1] - middle[1], parent[0] - middle[0]) + rng.range(-0.9, 0.9)
            distance = rng.range(320.0, 800.0)
            first = (parent[0] + distance * math.cos(away), parent[1] + distance * math.sin(away))
            clump = [first]
            for _ in range(size - 1):
                for _ in range(40):
                    turn = rng.range(0.0, 2 * math.pi)
                    step = rng.range(GROVE_SPACING, GROVE_SPACING + 40.0)
                    candidate = (clump[-1][0] + step * math.cos(turn), clump[-1][1] + step * math.sin(turn))
                    if all(math.dist(candidate, other) >= GROVE_SPACING for other in clump):
                        clump.append(candidate)
                        break
            if len(clump) == size and all(free(x, y, 300.0 if size == 1 else GROVE_SPACING) and not cut_out(x, y, 150.0) for x, y in clump):
                points += clump
                break
        else:
            return None
    return centred(points)


SHAPES = {}  # (kind, seed, count, reach) -> offsets: the same shape is asked for again and again by the layout's checks


def grove_local(seed, count, reach=None):
    """The trees of one grove of count trees, as offsets (x, y) from its centre: exactly count, irregular, none closer than
    GROVE_NEVER to another, nearly all within about 2.4 m of a neighbour (a wall, as in AoE), farthest one within reach
    (default grove_reach(count)). A grove of 12 or more has a glade or a path of 3 m through it. Deterministic from seed."""
    bound = grove_reach(count) if reach is None else reach
    if ("grove", seed, count, bound) in SHAPES:
        return SHAPES[("grove", seed, count, bound)]
    best = None
    for attempt in range(48):
        points = grove_attempt(Rng(seed, 11, attempt), count, compact=attempt >= 32)
        if points is None:
            continue
        extent = max(math.hypot(x, y) for x, y in points)
        if extent <= bound:
            SHAPES[("grove", seed, count, bound)] = points
            return points
        if best is None or extent < best[0]:
            best = (extent, points)
    if best is None:
        raise RuntimeError("No room for a grove of %d trees (seed %d)" % (count, seed))
    SHAPES[("grove", seed, count, bound)] = best[1]
    return best[1]


def grove_points(seed, centre, count, axis=(1.0, 0.0), reach=None):
    """grove_local, turned onto axis (a unit vector) and moved to centre: the world places of a grove's trees."""
    return [(centre[0] + x, centre[1] + y) for x, y in rotate_local(grove_local(seed, count, reach), axis)]


def berry_local(seed, count, reach=None):
    """The bushes of one patch of count bushes, as offsets from its centre: an irregular little cluster or a bent chain, never
    a straight row, BERRY_MIN to BERRY_MAX between a bush and the one it grew from (80 cm between the edges, closed to a unit,
    so a patch is gathered from its edge), none closer than BERRY_MIN to another."""
    bound = berry_reach(count) if reach is None else reach
    if ("berry", seed, count, bound) in SHAPES:
        return SHAPES[("berry", seed, count, bound)]
    best = None
    for attempt in range(80):
        rng = Rng(seed, 13, attempt)
        chain = rng.chance(0.5)
        heading = rng.range(0.0, 2 * math.pi)
        points = [(0.0, 0.0)]
        failed = False
        while len(points) < count and not failed:
            for _ in range(60):
                parent = points[-1] if chain else points[rng.below(len(points))]
                angle = heading + rng.range(-1.2, 1.2) if chain else rng.range(0.0, 2 * math.pi)
                step = rng.range(BERRY_MIN, BERRY_MAX)
                candidate = (parent[0] + step * math.cos(angle), parent[1] + step * math.sin(angle))
                if all(math.dist(candidate, other) >= BERRY_MIN for other in points):
                    points.append(candidate)
                    heading = angle
                    break
            else:
                failed = True
        if failed:
            continue
        points = centred(points)
        # Not a row: some bush at least 1.2 m off the line through the two farthest apart.
        first, second = max(((a, b) for a in points for b in points), key=lambda pair: math.dist(*pair))
        length = math.dist(first, second)
        off = max(abs((p[0] - first[0]) * (second[1] - first[1]) - (p[1] - first[1]) * (second[0] - first[0])) / length for p in points)
        extent = max(math.hypot(x, y) for x, y in points)
        if off >= 120.0 and extent <= bound:
            SHAPES[("berry", seed, count, bound)] = points
            return points
        if best is None or extent < best[0]:
            best = (extent, points)
    if best is None:
        raise RuntimeError("No room for a patch of %d berry bushes (seed %d)" % (count, seed))
    SHAPES[("berry", seed, count, bound)] = best[1]
    return best[1]


def berry_points(seed, centre, count, axis=(1.0, 0.0), reach=None):
    """berry_local turned onto axis and moved to centre."""
    return [(centre[0] + x, centre[1] + y) for x, y in rotate_local(berry_local(seed, count, reach), axis)]


PACK_TREES = 14  # M-Q8 (the user): a forest is a pack of this many trees (a few more or less); the budget is spread in many packs
BASE_PACK_STEP = 2400.0  # cm: a base's second pack is this much further ahead (towards the middle) than its first, 24 m between the centres


def base_sources(recipe, wood_rows=WOOD_ROWS, wood_columns=WOOD_COLUMNS, veins=True, organic=False, wood_packs=None):
    """Each player's base (D295), the same at every size: the woods first (source 0 is the first tree of slot 0's
    wood, the scripted checks use it), then the berries, then the veins. wood_rows x wood_columns trees in each wood
    (Plain: 5 x 8, 40 trees; a map with poorer bases, the Steppe's 4 x 5, says less); veins=False leaves the two veins
    out (a map that keeps its stone and gold away from the bases). organic (C1c): the wood is one irregular grove of
    wood_rows x wood_columns trees and the berries one irregular patch, the same shapes for every slot (seeded by
    recipe.layout_seed, drawn once and placed in each slot's frame), not a grid. wood_packs (organic only, M-Q8): the sizes of the base's
    wood as packs instead of one grove of wood_rows x wood_columns trees, e.g. [14, 6]: each its own grove on the wood's side of the slot, the
    next BASE_PACK_STEP further ahead of the first, so that two packs are two clusters."""
    sources = []
    for index in range(len(recipe.angles)):
        slot, radial, left = recipe.slot(index)
        if organic:
            for number, count in enumerate(wood_packs or [wood_rows * wood_columns]):
                at = (slot[0] + left[0] * ORGANIC_WOOD_SIDE - radial[0] * BASE_PACK_STEP * number, slot[1] + left[1] * ORGANIC_WOOD_SIDE - radial[1] * BASE_PACK_STEP * number)
                sources += [placed_source("Tree", x, y) for x, y in grove_points(mix_seed(recipe.layout_seed, 1, number) if number else mix_seed(recipe.layout_seed, 1), at, count, radial, ORGANIC_WOOD_REACH)]
        else:
            sources += wood(slot, radial, left, wood_rows, wood_columns, WOOD_NEAR)
    for index in range(len(recipe.angles)):
        slot, radial, left = recipe.slot(index)
        right = (-left[0], -left[1])
        if organic:
            at = (slot[0] + right[0] * ORGANIC_BERRY_SIDE, slot[1] + right[1] * ORGANIC_BERRY_SIDE)
            sources += [placed_source("BerryBush", x, y) for x, y in berry_points(mix_seed(recipe.layout_seed, 2), at, BERRY_ROWS * BERRY_COLUMNS, radial, ORGANIC_BERRY_REACH)]
            continue
        for row in range(BERRY_ROWS):
            for column in range(BERRY_COLUMNS):
                side = BERRY_NEAR + row * BERRY_SPACING
                along = (column - (BERRY_COLUMNS - 1) / 2.0) * BERRY_SPACING
                sources.append(placed_source("BerryBush", slot[0] + right[0] * side + radial[0] * along,
                                             slot[1] + right[1] * side + radial[1] * along))
    if not veins:
        return sources
    if recipe.size_key is None:
        # The test map's slot angles are multiples of 45 degrees: a vein at each odd multiple of 22.5 degrees, between
        # two neighbours.
        for number in range(len(recipe.angles)):
            radians = math.radians(22.5 + 45.0 * number)
            sources.append(placed_source(VEIN_TYPES[number % 2], VEIN_RADIUS * math.cos(radians), VEIN_RADIUS * math.sin(radians)))
    else:
        for index in range(len(recipe.angles)):
            slot, radial, left = recipe.slot(index)
            for type_name, sign in ((VEIN_TYPES[0], 1.0), (VEIN_TYPES[1], -1.0)):  # stone on the wood's side
                sources.append(placed_source(type_name, slot[0] - radial[0] * VEIN_AHEAD + sign * left[0] * VEIN_SIDE,
                                             slot[1] - radial[1] * VEIN_AHEAD + sign * left[1] * VEIN_SIDE))
    return sources


def map_file(recipe):
    return os.path.join(unreal.Paths.project_content_dir(), "Maps", recipe.map_path.rsplit("/", 1)[1] + ".umap")


# ---- Terrain zones (C3, El Dorado's marsh): round zones that change the walkers' speed (HOLTerrainRules), as data for the server plus a flat
# disc drawn on the ground over each. The module's zones(recipe) gives [(kind, x, y, radius)] (kind 0: marsh); its ZONE_COLORS {kind: (r, g, b)}
# says how a kind is drawn. The discs have no collision and no navigation (the zone is not an obstacle), 3 cm above the ground.
ZONE_COLORS = {0: (0.020, 0.052, 0.040), 1: (0.36, 0.29, 0.17)}  # 0 marsh, 1 road (C5, packed earth)
ZONE_HEIGHT = 3.0
ZONE_NAMES = {0: "Marsh", 1: "Road"}


def spawn_zones(zones, colors=None):
    layout_class = unreal.load_class(None, "/Script/HeroesOfLiberty.HOLTerrainLayout")
    if layout_class is None:
        raise RuntimeError("Missing HOLTerrainLayout (is the HeroesOfLiberty module compiled?)")
    colors = {**ZONE_COLORS, **(colors or {})}  # (dict(a, **b) refuses integer keys)
    layout = spawn(layout_class, label="TerrainLayout")
    placed = []
    for kind, x, y, radius in zones:
        zone = unreal.HOLPlacedTerrainZone()
        zone.set_editor_property("kind", kind)
        zone.set_editor_property("location", unreal.Vector(x, y, 0))
        zone.set_editor_property("radius", radius)
        placed.append(zone)
    layout.set_editor_property("zones", placed)
    materials = {kind: matte_instance("MI_Zone_" + ZONE_NAMES.get(kind, str(kind)), unreal.LinearColor(*colors[kind], 1.0)) for kind in set(z[0] for z in zones)}
    for index, (kind, x, y, radius) in enumerate(zones):
        # The engine's cylinder is 1 m across and 1 m high: a flat disc of the zone's radius, its top ZONE_HEIGHT above the ground.
        scale = unreal.Vector(radius / 50.0, radius / 50.0, 0.04)
        disc = spawn_shape("/Engine/BasicShapes/Cylinder", unreal.Vector(x, y, ZONE_HEIGHT - 2.0), scale, "%s%d" % (ZONE_NAMES.get(kind, "Zone"), index), materials[kind])
        component = disc.get_editor_property("static_mesh_component")
        component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        component.set_editor_property("can_ever_affect_navigation", False)


# ---- Relays (R2, D388, RelaysInProgress.md): neutral places the map lays, taken by standing in them. A module's relays(recipe) gives
# [(x, y, radius)] in cm; make_map lays them as an HOLRelayLayout (data for the server, the rings are drawn at run time). Rules the Map.Table test
# checks (FHOLRelayLayoutRules): symmetric by a turn of one place, no zone in a base's reach, all inside the bounds, a radius above 0, and (this
# file) no tree, bush or vein inside a zone: RELAY_ZONES is read by steppe.kept_out so that the far layer keeps RELAY_SOURCE_CLEAR away from it,
# and make_map drops what is left in a zone (the maps of other recipes).
# NUMBER AND PLACES (Q-relais-lieux, the user: "4 + number of players"; the reading is the session's, TO CONFIRM): one relay between every two neighbouring
# places (their middle angle) on a circle of about half the places' circle, plus 4 round the middle; the radius of the ruin's zone (1 200 cm).
RELAY_RADIUS = 1200.0
RELAY_SOURCE_CLEAR = 1500.0
RELAY_ROCK_CLEAR = 1000.0
RELAY_BASE_MARGIN = 500.0
RELAY_FRACTIONS = (0.5, 0.45, 0.55, 0.4, 0.6, 0.35, 0.65, 0.7, 0.75)
RELAY_FRACTIONS_WIDE = tuple(sorted((0.3 + 0.025 * k for k in range(0, 27) if round(0.3 + 0.025 * k, 3) not in RELAY_FRACTIONS), key=lambda f: abs(f - 0.5)))  # second tier of ring circles
RELAY_ZONES = []  # [(x, y, radius)] of the map being made, read by steppe.kept_out


RELAY_EXTRA = 5  # the fixed relays besides the ring: v4 (the user, E-relays-carte, "in the manner of" his picture of 5 circles): one in the middle and two symmetric pairs
RELAY_EXTRA_ZONE = RELAY_RADIUS  # the fixed relays' zone: 12 m (the user, E-relays-carte v2: "REGLAGE: rayon 12")
# E-relays-carte v4 (the user, 2026-11-02: "in the manner of" a picture: 5 circles spread over the WHOLE map, one in the middle, two near opposite corners,
# two at mid height near the sides; not a square nor a cross nor stuck to the middle): a SEEDED TEMPLATE, drawn once per map from its seed (recipe.layout_seed,
# the map, its places and HOL_RELAY_SEED, default 1: the same result every time the map is made), in the frame of the places (u from the middle towards place 0,
# v across), as fractions of the bounds' half side H: the middle (0, 0); a CORNER pair +-(u, v) with |u|, |v| in 0.60 to 0.85 H (random signs: the diagonal it
# lies on); a SIDE pair +-(du, v) with |v| in 0.72 to 0.88 H and du in +-0.10 H (across the axis of the places). The group is closed under a turn of 180 degrees:
# with 2 places the two bases see the same distances (exact); with 4, 6 and 8 places a base and its opposite are equal, the other bases are not (the places
# circle is of 0.75 H, so the pairs sit between and beyond the bases). A zone near a base, a rock or the ring is refused and the draw goes on.
RELAY_FIXED_SEED = int(os.environ.get("HOL_RELAY_SEED", "1"))
RELAY_VARIANT = 0  # 0 = the layout of HOL_RELAY_SEED; lay_relays_and_sources moves to 1, 2... (a neighbouring seed, always the same sequence) when a map has no room
RELAY_VARIANTS = 12
RELAY_VARIANT_STEP = 7919
RELAY_FIXED_APART = 500.0  # cm of free ground between two zones' edges
RELAY_FIXED_TRIES = 40000
SALT_RELAYS = 778
# D647 (Q-volcan-autel-relais = B, 2026-11-03): a map with a volcano (all of them) keeps the relay of the middle at least 24 m from the middle, so that the relay
# and the altar of the crater are two stakes. Not on a lava flow (LAVA_HALF_WIDTH + the zone) when the map has room, nor on the altar's zone. El Dorado
# (min_radius > 0) keeps its own rule (the marsh's edge, already further than that). The other 4 relays are not touched.
RELAY_MIDDLE_MIN = 2400.0
RELAY_MIDDLE_DISTANCES = tuple(RELAY_MIDDLE_MIN + 400.0 * k for k in range(0, 13))
RELAY_MIDDLE_OFFSETS = (-90.0, 90.0, -45.0, 45.0, -135.0, 135.0, 180.0, 0.0, -22.5, 22.5, -67.5, 67.5, -112.5, 112.5, -157.5, 157.5)
RELAY_LAVA_HALF_WIDTH = 800.0


def fixed_group(a0, rng, half, middle):
    """5 points (x, y): the middle (moved to `middle` along the axis of place 0 when a marsh or a mine fills the middle: El Dorado) and two symmetric pairs; a0: the angle of place 0."""
    ux, uy = math.cos(a0), math.sin(a0)
    vx, vy = -uy, ux

    def at(u, v):
        return u * ux + v * vx, u * uy + v * vy

    corner_u = rng.range(0.60, 0.85) * half * (1.0 if rng.uniform() < 0.5 else -1.0)
    corner_v = rng.range(0.60, 0.85) * half * (1.0 if rng.uniform() < 0.5 else -1.0)
    side_v = rng.range(0.72, 0.88) * half
    side_u = rng.range(-0.10, 0.10) * half
    cx, cy = at(corner_u, corner_v)
    sx, sy = at(side_u, side_v)
    mx, my = at(middle, 0.0)
    return [(mx, my), (cx, cy), (-cx, -cy), (sx, sy), (-sx, -sy)]


def segment_distance(point, a, b):
    """Distance from a point to the segment a-b."""
    dx, dy = b[0] - a[0], b[1] - a[1]
    length2 = dx * dx + dy * dy
    t = 0.0 if length2 <= 0.0 else max(0.0, min(1.0, ((point[0] - a[0]) * dx + (point[1] - a[1]) * dy) / length2))
    return math.dist(point, (a[0] + t * dx, a[1] + t * dy))


def far_middle(recipe, turn, accept):
    """D647: the point of the middle relay, RELAY_MIDDLE_MIN or more from the middle of the map, off the altar's zone and (if the map has room) off the lava
    flows; accept(point) says whether the rest of the map lets it be there (bounds, bases, rocks, the other relays). The places' middle angles first (more
    than 2 places), then the frame's turn plus RELAY_MIDDLE_OFFSETS (2 places: across the axis of the places, on the side opposite the altar, as far from one base as from
    the other); the nearest distance of each angle first. None if no point fits."""
    places = len(recipe.angles)
    site = volcano_site(recipe)
    altar_x, altar_y, altar_radius = site["altar"]
    angles = []
    if places > 2:
        angles += [math.radians(recipe.angles[0]) + (2 * k + 1) * math.pi / places for k in range(places)]
    angles += [turn + math.radians(offset) for offset in RELAY_MIDDLE_OFFSETS]
    for off_flows in (True, False):
        for angle in angles:
            for distance in RELAY_MIDDLE_DISTANCES:
                point = (distance * math.cos(angle), distance * math.sin(angle))
                if math.dist(point, (altar_x, altar_y)) < RELAY_RADIUS + altar_radius:
                    continue
                if off_flows and any(segment_distance(point, flow[0], flow[-1]) < RELAY_LAVA_HALF_WIDTH + RELAY_EXTRA_ZONE for flow in site["flows"]):
                    continue
                if accept(point):
                    return point
    return None


def fixed_relays(recipe, ring, slots, avoid, rock_clear, keep_whole, min_radius):
    """The 5 fixed relays (x, y, RELAY_EXTRA_ZONE) of a map, or None if no draw is clear: inside the bounds, outside every base's reach, rock_clear from every
    avoid point, whole veins left alone, apart from each other and from the ring's relays (RELAY_FIXED_APART between edges). min_radius > 0 (El Dorado's marsh):
    the middle relay stands that far from the middle, along the axis of place 0, and the others at least as far."""
    places = len(recipe.angles)
    rng = Rng(mix_seed(recipe.layout_seed, SALT_RELAYS, RELAY_FIXED_SEED + RELAY_VARIANT * RELAY_VARIANT_STEP, places, sum(ord(c) for c in recipe.map_name)))
    zone = RELAY_EXTRA_ZONE
    middle = (min_radius + zone + RELAY_BASE_MARGIN) if min_radius > 0.0 else 0.0
    for _ in range(RELAY_FIXED_TRIES):
        # 2 places: the frame is the places' own (the pairs lie across and beyond the two bases); 4, 6 and 8: the frame is turned at random (the group stays
        # symmetric by 180 degrees whatever its turn) so that the pairs find the room between the bases.
        turn = math.radians(recipe.angles[0]) if places == 2 else rng.range(0.0, math.pi)
        group = fixed_group(turn, rng, recipe.bounds_half, middle)
        if min_radius <= 0.0:  # D647: the middle relay is drawn apart (far_middle), the two pairs are checked as they always were
            pairs = group[1:]

            def point_ok(p, others):
                return (abs(p[0]) <= recipe.bounds_half - zone - RELAY_BASE_MARGIN and abs(p[1]) <= recipe.bounds_half - zone - RELAY_BASE_MARGIN
                        and all(math.dist(p, slot) >= BASE_REACH + zone + RELAY_BASE_MARGIN for slot in slots)
                        and all(math.dist(p, q) >= 2.0 * zone + RELAY_FIXED_APART for q in others)
                        and all(math.dist(p, (x, y)) >= zone + r + RELAY_FIXED_APART for x, y, r in ring)
                        and all(math.dist(p, rock) >= zone + rock_clear for rock in avoid)
                        and all(math.dist(p, pile) >= zone + 1.0 for pile in keep_whole))

            if not all(point_ok(p, [q for q in pairs if q is not p]) for p in pairs):
                continue
            mid = far_middle(recipe, turn, lambda p: point_ok(p, pairs))
            if mid is None:
                continue
            return [(x, y, zone) for x, y in [mid] + pairs]
        clear = all(math.hypot(x, y) >= min_radius - 1.0 for x, y in group)
        clear = clear and all(abs(x) <= recipe.bounds_half - zone - RELAY_BASE_MARGIN and abs(y) <= recipe.bounds_half - zone - RELAY_BASE_MARGIN for x, y in group)
        clear = clear and all(math.dist(p, slot) >= BASE_REACH + zone + RELAY_BASE_MARGIN for p in group for slot in slots)
        clear = clear and all(math.dist(p, q) >= 2.0 * zone + RELAY_FIXED_APART for i, p in enumerate(group) for q in group[i + 1:])
        clear = clear and all(math.dist(p, (x, y)) >= zone + r + RELAY_FIXED_APART for p in group for x, y, r in ring)
        clear = clear and all(math.dist(p, rock) >= zone + rock_clear for p in group for rock in avoid)
        clear = clear and all(math.dist(p, pile) >= zone + 1.0 for p in group for pile in keep_whole)
        if clear:
            return [(x, y, zone) for x, y in group]
    return None


RELAY_SNAP_STEP = 400.0
RELAY_SNAP_REACH_FRACTION = 0.3  # how far (of the bounds' half side) a point of the group may be moved to find a clear place
RELAY_SNAP_DRAWS = 40


def snapped_relays(recipe, ring, slots, avoid, rock_clear, keep_whole, min_radius):
    """The 5 fixed relays of a map that is full of rocks (the Hinterland's belt and pockets: the plain draw of fixed_relays finds no group whose five zones all
    stand clear), or None. A template is drawn as in fixed_relays, then each of its three independent parts (the middle one, the corner pair, the side pair) is
    moved to the nearest clear place found on a spiral round its drawn point (RELAY_SNAP_STEP, RELAY_SNAP_REACH_FRACTION of the half side); a pair moves as
    one (the point and its opposite, a turn of 180 degrees, are both clear), so the group stays closed under that turn. Deterministic (the same seeded draws)."""
    places = len(recipe.angles)
    rng = Rng(mix_seed(recipe.layout_seed, SALT_RELAYS + 1, RELAY_FIXED_SEED + RELAY_VARIANT * RELAY_VARIANT_STEP, places, sum(ord(c) for c in recipe.map_name)))
    zone = RELAY_EXTRA_ZONE
    half = recipe.bounds_half
    middle = (min_radius + zone + RELAY_BASE_MARGIN) if min_radius > 0.0 else 0.0
    reach = RELAY_SNAP_REACH_FRACTION * half

    def clear(point, others):
        if not (abs(point[0]) <= half - zone - RELAY_BASE_MARGIN and abs(point[1]) <= half - zone - RELAY_BASE_MARGIN):
            return False
        if math.hypot(point[0], point[1]) < min_radius - 1.0:
            return False
        if any(math.dist(point, slot) < BASE_REACH + zone + RELAY_BASE_MARGIN for slot in slots):
            return False
        if any(math.dist(point, other) < 2.0 * zone + RELAY_FIXED_APART for other in others):
            return False
        if any(math.dist(point, (x, y)) < zone + r + RELAY_FIXED_APART for x, y, r in ring):
            return False
        if any(math.dist(point, pile) < zone + 1.0 for pile in keep_whole):
            return False
        return not any(math.dist(point, rock) < zone + rock_clear for rock in avoid)

    def spiral(point):
        yield point
        step = RELAY_SNAP_STEP
        for number in range(1, int(reach / step) + 1):
            count = 6 * number
            for k in range(count):
                angle = 2.0 * math.pi * k / count
                yield (point[0] + number * step * math.cos(angle), point[1] + number * step * math.sin(angle))

    def find(point, others, pair):
        for candidate in spiral(point):
            if pair:
                twin = (-candidate[0], -candidate[1])
                if clear(candidate, others) and clear(twin, others + [candidate]):
                    return [candidate, twin]
            elif clear(candidate, others):
                return [candidate]
        return None

    for _ in range(RELAY_SNAP_DRAWS):
        turn = math.radians(recipe.angles[0]) if places == 2 else rng.range(0.0, math.pi)
        group = fixed_group(turn, rng, half, middle)
        placed = []
        # D647: with no marsh the middle relay is drawn last, far from the middle (far_middle); the pairs go first, as before.
        parts = ((group[1], True), (group[3], True)) if min_radius <= 0.0 else ((group[0], False), (group[1], True), (group[3], True))
        for point, pair in parts:
            found = find(point, placed, pair)
            if found is None:
                break
            placed += found
        else:
            if min_radius <= 0.0:
                mid = far_middle(recipe, turn, lambda p: clear(p, placed))
                if mid is None:
                    continue
                return [(x, y, zone) for x, y in [mid] + placed]
            return [(x, y, zone) for x, y in placed]
    return None


def ring_relays(recipe, avoid=(), extra_radius=0.0, on_slots=False, keep_whole=()):
    """The relays of a map, the ring first then the 4 fixed ones (the order the Map.Table test reads): one for every two neighbouring places (their middle
    angle; on_slots: on each place's own axis, for the maps whose ranges of mountains run along the middle angles), on the first circle (RELAY_FRACTIONS of the
    places' circle) where all of them are clear, then the 4 fixed ones drawn at random (fixed_relays; extra_radius: they stay at least this far from the middle,
    El Dorado's marsh). Clear: outside every base's reach and the bounds' edge, RELAY_ROCK_CLEAR from every avoid point (a rock). The ring is symmetric by a
    turn of one place."""
    if recipe.size_key is None:
        return []
    places = len(recipe.angles)
    first = math.radians(recipe.angles[0] + (0.0 if on_slots else 180.0 / places))
    slots = [recipe.slot(index)[0] for index in range(places)]
    # Two tiers of ring circles: the usual ones, then (only if none fits, so the maps that fit keep their relays) a finer and wider choice.
    for fractions in (RELAY_FRACTIONS, RELAY_FRACTIONS_WIDE):
        for rock_clear in (RELAY_ROCK_CLEAR, 0.0):  # UNTESTED fallback (R2): rocks 0 cm from a zone's edge if none of the clearances above fit
            for fraction in fractions:
                radius = fraction * recipe.slot_radius
                if radius < extra_radius + RELAY_RADIUS + RELAY_BASE_MARGIN:
                    continue
                ring = [(radius * math.cos(first + 2.0 * math.pi * k / places), radius * math.sin(first + 2.0 * math.pi * k / places), RELAY_RADIUS) for k in range(places)]
                clear = all(abs(x) <= recipe.bounds_half - RELAY_RADIUS - RELAY_BASE_MARGIN and abs(y) <= recipe.bounds_half - RELAY_RADIUS - RELAY_BASE_MARGIN for x, y, _ in ring)
                clear = clear and all(math.dist((x, y), slot) >= BASE_REACH + RELAY_RADIUS + RELAY_BASE_MARGIN for x, y, _ in ring for slot in slots)
                clear = clear and all(math.dist((x, y), rock) >= RELAY_RADIUS + rock_clear for x, y, _ in ring for rock in avoid)
                clear = clear and all(math.dist((x, y), pile) >= RELAY_RADIUS + 1.0 for x, y, _ in ring for pile in keep_whole)  # keep_whole: sources that must not fall in a zone
                if not clear:
                    continue
                fixed = fixed_relays(recipe, ring, slots, avoid, rock_clear, keep_whole, extra_radius)
                if fixed:
                    return ring + fixed
    # Last resort (a map full of rocks, the Hinterland's belt): the same circles, the fixed group's parts moved to the nearest clear ground (snapped_relays).
    for rock_clear in (RELAY_ROCK_CLEAR, 0.0):
        for fractions in (RELAY_FRACTIONS, RELAY_FRACTIONS_WIDE):
            for fraction in fractions:
                radius = fraction * recipe.slot_radius
                if radius < extra_radius + RELAY_RADIUS + RELAY_BASE_MARGIN:
                    continue
                ring = [(radius * math.cos(first + 2.0 * math.pi * k / places), radius * math.sin(first + 2.0 * math.pi * k / places), RELAY_RADIUS) for k in range(places)]
                clear = all(abs(x) <= recipe.bounds_half - RELAY_RADIUS - RELAY_BASE_MARGIN and abs(y) <= recipe.bounds_half - RELAY_RADIUS - RELAY_BASE_MARGIN for x, y, _ in ring)
                clear = clear and all(math.dist((x, y), slot) >= BASE_REACH + RELAY_RADIUS + RELAY_BASE_MARGIN for x, y, _ in ring for slot in slots)
                clear = clear and all(math.dist((x, y), rock) >= RELAY_RADIUS + rock_clear for x, y, _ in ring for rock in avoid)
                clear = clear and all(math.dist((x, y), pile) >= RELAY_RADIUS + 1.0 for x, y, _ in ring for pile in keep_whole)
                if not clear:
                    continue
                fixed = snapped_relays(recipe, ring, slots, avoid, rock_clear, keep_whole, extra_radius)
                if fixed:
                    return ring + fixed
    raise RuntimeError("No clear circle for the relays of %s" % recipe.map_path)


def spawn_relays(relays):
    layout_class = unreal.load_class(None, "/Script/HeroesOfLiberty.HOLRelayLayout")
    if layout_class is None:
        raise RuntimeError("Missing HOLRelayLayout (is the HeroesOfLiberty module compiled?)")
    layout = spawn(layout_class, label="RelayLayout")
    placed = []
    for x, y, radius in relays:
        relay = unreal.HOLPlacedRelay()
        relay.set_editor_property("location", unreal.Vector(x, y, 0))
        relay.set_editor_property("radius", radius)
        placed.append(relay)
    layout.set_editor_property("relays", placed)


def outside_relays(sources, relays):
    """The sources whose centre is not inside any relay's zone."""
    kept = []
    for source in sources:
        at = source.get_editor_property("location")
        if all(math.dist((at.x, at.y), (x, y)) >= radius for x, y, radius in relays):
            kept.append(source)
    return kept


def min_trees_per_territory(recipe, sources):
    """The fewest trees any territory holds (a tree belongs to the slot nearest to it, as Map.Table counts)."""
    slots = [recipe.slot(index)[0] for index in range(len(recipe.angles))]
    counts = [0] * len(slots)
    for source in sources:
        if str(source.get_editor_property("type")) == "Tree":
            at = source.get_editor_property("location")
            counts[min(range(len(slots)), key=lambda index: math.dist((at.x, at.y), slots[index]))] += 1
    return min(counts)


# ---- Volcano sites (V4b, D607, D573, VolcanoInProgress.md): the crater and the lava flows of a volcano, written here like the trees and the ores, SYMMETRIC BY
# CONSTRUCTION (one flow towards each place, the same length, so that the free draw of the direction (D573) favours no camp). A module's volcano(recipe)
# gives [site]; a site is {"flows": [[(x, y), ...], ...], "altar": (x, y, radius) or None}, the crater being the first point of each flow. make_map lays
# them as an HOLVolcanoLayout (data for the server; nothing is drawn: V9). Rules the Map.Table test checks (FHOLVolcanoLayoutRules) and volcano_unfair()
# below checks at once: the flows the same by a turn of one place, the altar as far from every base, everything inside the bounds.
# PROVISIONAL LEVEL DESIGN (Q-volcan-site, asked of the user): the crater in the middle of the map, one flow straight towards each place, VOLCANO_REACH of the
# way to the place at most and always VOLCANO_BASE_CLEAR short of its base's reach; the altar, 2 places: VOLCANO_ALTAR_OFFSET across the axis of the places
# from the crater (as far from both bases), 3 places and more: ON the crater (the only point as far from every base).
VOLCANO_REACH = float(os.environ.get("HOL_VOLCANO_REACH", "0.5"))  # fraction of the distance from the middle to a place that a flow runs
VOLCANO_BASE_CLEAR = 1600.0  # cm kept between the end of a flow and the reach of the base it aims at: lava half-width 800 cm + a margin
VOLCANO_ALTAR_OFFSET = float(os.environ.get("HOL_VOLCANO_ALTAR_OFFSET", "3000"))  # cm; outside the 16 m corridor of the flows
VOLCANO_ALTAR_RADIUS = 1200.0  # the ruin's zone (D348)
VOLCANO_FLOW_POINTS = 3  # the crater, the middle, the end
VOLCANO_AIM = os.environ.get("HOL_VOLCANO_AIM", "bases")  # "bases": a flow straight at each base; "between": at the middle of the gap between two neighbouring bases (Q-volcan-site-trace)
VOLCANO_TOLERANCE = 100.0  # cm: the same tolerance as FHOLVolcanoLayoutRules::SymmetryTolerance


def volcano_site(recipe):
    """The volcano site of a map, symmetric by construction: a flow from the middle straight towards each place, the altar where it is as far from every base."""
    slots = [recipe.slot(index)[0] for index in range(len(recipe.angles))]
    flows = []
    for x, y in slots:
        distance = math.hypot(x, y)
        length = min(VOLCANO_REACH * distance, distance - BASE_REACH - VOLCANO_BASE_CLEAR)
        if length <= 0.0:
            raise RuntimeError("%s: no room for a lava flow towards a place (%.0f cm away)" % (recipe.map_path, distance))
        ux, uy = x / distance, y / distance
        if VOLCANO_AIM == "between":  # half a place further round (counterclockwise): the same turn of one place still maps the flows on one another
            half = math.pi / len(slots)
            ux, uy = ux * math.cos(half) - uy * math.sin(half), ux * math.sin(half) + uy * math.cos(half)
        flows.append([(ux * length * step / (VOLCANO_FLOW_POINTS - 1), uy * length * step / (VOLCANO_FLOW_POINTS - 1)) for step in range(VOLCANO_FLOW_POINTS)])
    altar = (0.0, 0.0, VOLCANO_ALTAR_RADIUS)
    if len(slots) == 2:
        x, y = slots[0]
        distance = math.hypot(x, y)
        altar = (-y / distance * VOLCANO_ALTAR_OFFSET, x / distance * VOLCANO_ALTAR_OFFSET, VOLCANO_ALTAR_RADIUS)
    return {"flows": flows, "altar": altar}


def volcano_unfair(recipe, sites):
    """What is unfair or out of the map in these sites (a list of texts, empty = fair): the same checks as Map.Table's (FHOLVolcanoLayoutRules)."""
    problems = []
    places = len(recipe.angles)
    turn = math.radians(360.0 / places)
    cos, sin = math.cos(turn), math.sin(turn)
    slots = [recipe.slot(index)[0] for index in range(places)]
    for number, site in enumerate(sites):
        if not site["flows"]:
            problems.append("site %d has no flow" % number)
        for flow in site["flows"]:
            if len(flow) < 2 or math.dist(flow[0], flow[-1]) <= 0.0:
                problems.append("site %d: a flow has no length" % number)
            twin = any(len(other) == len(flow) and all(abs(ox - (x * cos - y * sin)) <= VOLCANO_TOLERANCE and abs(oy - (x * sin + y * cos)) <= VOLCANO_TOLERANCE
                                                       for (x, y), (ox, oy) in zip(flow, other)) for other in site["flows"])
            if not twin:
                problems.append("site %d: the flow from (%.0f, %.0f) to (%.0f, %.0f) has no twin after a turn of one place" % (number, flow[0][0], flow[0][1], flow[-1][0], flow[-1][1]))
            if any(abs(x) > recipe.bounds_half or abs(y) > recipe.bounds_half for x, y in flow):
                problems.append("site %d: a flow leaves the bounds" % number)
        if site["altar"] is not None:
            x, y, radius = site["altar"]
            distances = [math.dist((x, y), slot) for slot in slots]
            if len(distances) > 1 and max(distances) - min(distances) > VOLCANO_TOLERANCE:
                problems.append("site %d: the altar is %.0f cm nearer one base than another" % (number, max(distances) - min(distances)))
            if abs(x) + radius > recipe.bounds_half or abs(y) + radius > recipe.bounds_half:
                problems.append("site %d: the altar's zone leaves the bounds" % number)
    return problems


def spawn_volcano(sites):
    layout_class = unreal.load_class(None, "/Script/HeroesOfLiberty.HOLVolcanoLayout")
    if layout_class is None:
        raise RuntimeError("Missing HOLVolcanoLayout (is the HeroesOfLiberty module compiled?)")
    layout = spawn(layout_class, label="VolcanoLayout")
    placed = []
    for site in sites:
        volcano = unreal.HOLPlacedVolcano()
        flows = []
        for points in site["flows"]:
            flow = unreal.HOLPlacedLavaFlow()
            flow.set_editor_property("points", [unreal.Vector(x, y, 0) for x, y in points])
            flows.append(flow)
        volcano.set_editor_property("flows", flows)
        if site["altar"] is not None:
            x, y, radius = site["altar"]
            volcano.set_editor_property("altar", True)
            volcano.set_editor_property("altar_location", unreal.Vector(x, y, 0))
            volcano.set_editor_property("altar_radius", radius)
        placed.append(volcano)
    layout.set_editor_property("sites", placed)

def lay_relays_and_sources(recipe, module):
    """(relays, sources, groups) of a map. The relays' zones push the far layer's sources about, so a layout of relays may leave a territory without room
    for a pile (steppe.RoomError) or a map without a clear circle: then the next RELAY_VARIANT (the same fixed relays group drawn from a neighbouring seed;
    always the same sequence, so the same file every time) is tried, up to RELAY_VARIANTS. Variant 0 is HOL_RELAY_SEED's own."""
    global RELAY_VARIANT
    last = None
    try:
        for RELAY_VARIANT in range(RELAY_VARIANTS):
            try:
                relays = list(module.relays(recipe)) if hasattr(module, "relays") else []
                RELAY_ZONES[:] = relays
                try:
                    sources, groups = module.sources(recipe)
                finally:
                    RELAY_ZONES[:] = []
                floor = getattr(module, "tree_floor", None)  # the trees each territory must keep once the zones have taken theirs (Map.Table's budget)
                if floor is not None:
                    short = min_trees_per_territory(recipe, outside_relays(sources, relays))
                    if short < floor(recipe):
                        raise RuntimeError("%s: a territory holds %d trees outside the relays' zones, the budget is %d" % (recipe.map_path, short, floor(recipe)))
            except RuntimeError as error:
                last = error
                print("[maps] %s: relay variant %d refused (%s)" % (recipe.map_path, RELAY_VARIANT, str(error)[:120]))
                continue
            if RELAY_VARIANT:
                print("[maps] %s: relay variant %d" % (recipe.map_path, RELAY_VARIANT))
            return relays, sources, groups
    finally:
        RELAY_VARIANT = 0
    raise last


def edits_path(recipe):
    return os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()), "Tools", "Data", "MapEdits", recipe.map_path.rsplit("/", 1)[1] + ".json")


def edit_hash(x, y):
    """A value in [0, 1) from a position: the same tree is kept or dropped every time."""
    value = math.sin(x * 12.9898 + y * 78.233) * 43758.5453
    return value - math.floor(value)


def relattice(added, kept, spacing):
    """The painted trees re-seeded on a regular hex lattice of `spacing` cm (a little jitter) over the area they cover (settings "lattice"): the painter's
    own spacing and any thinning leave an uneven pattern (2026-11-05: after a gap filter the painted trees' nearest neighbour was 1.43 m against the
    recipe's groves 0.89 m, the user saw two kinds of forest); this gives the painted forests the spacing asked, whatever the painting. A node is kept
    if a painted tree is within 0.8 x spacing and no other source of the map is too near (a tree 0.7 x spacing, anything else 2 m)."""
    trees = [a for a in added if a["type"] == "Tree"]
    others = [a for a in added if a["type"] != "Tree"]
    reach = 0.8 * spacing
    cell = 200.0
    painted = {}
    for a in trees:
        painted.setdefault((int(a["x"] // cell), int(a["y"] // cell)), []).append((a["x"], a["y"]))
    fixed = {}
    for source in kept:
        at = source.get_editor_property("location")
        kind = str(source.get_editor_property("type"))
        fixed.setdefault((int(at.x // cell), int(at.y // cell)), []).append((at.x, at.y, kind == "Tree"))
    row = spacing * 0.8660254

    def near_painted(x, y):
        cx, cy = int(x // cell), int(y // cell)
        return any(math.dist((x, y), q) <= reach for i in (-1, 0, 1) for j in (-1, 0, 1) for q in painted.get((cx + i, cy + j), ()))

    def blocked(x, y):
        cx, cy = int(x // cell), int(y // cell)
        for i in (-1, 0, 1):
            for j in (-1, 0, 1):
                for ox, oy, is_tree in fixed.get((cx + i, cy + j), ()):
                    if math.dist((x, y), (ox, oy)) < (0.7 * spacing if is_tree else 200.0):
                        return True
        return False

    nodes = set()
    for a in trees:
        i0, j0 = int(round(a["x"] / row)), int(round(a["y"] / spacing))
        for i in range(i0 - 1, i0 + 2):
            for j in range(j0 - 1, j0 + 2):
                nodes.add((i, j))
    out = []
    for i, j in sorted(nodes):
        x = i * row + (edit_hash(i, j) - 0.5) * 0.16 * spacing
        y = j * spacing + (spacing / 2 if i & 1 else 0.0) + (edit_hash(j + 0.5, i) - 0.5) * 0.16 * spacing
        if near_painted(x, y) and not blocked(x, y):
            out.append({"type": "Tree", "x": round(x), "y": round(y)})
    return out + others


ROCK_MARGIN = 100.0  # cm: a painted tree this near an obstacle's bounds is left out too (the tests' raster of rocks is a cell coarser than the bounds)


def drop_buried(added):
    """2026-11-06: the painted (or stretched) trees that fall inside a collision obstacle of the level (a rock, a ridge, a wall: the static meshes of the map's
    own pieces, spawned before the sources; the ground and the flat water have none or are huge) are left out, as the tests' "no source inside a rock" asks.
    The box is the actor's axis-aligned bounds (a superset of its footprint) taken in by nothing."""
    boxes = []
    for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
        if not isinstance(actor, unreal.StaticMeshActor):
            continue
        origin, extent = actor.get_actor_bounds(True)
        if extent.x <= 0.0 or extent.y <= 0.0 or extent.x > 20000.0 or extent.y > 20000.0:
            continue
        boxes.append((origin.x - extent.x - ROCK_MARGIN, origin.x + extent.x + ROCK_MARGIN, origin.y - extent.y - ROCK_MARGIN, origin.y + extent.y + ROCK_MARGIN))
    if not boxes:
        return added
    cell = 1000.0
    grid = {}
    for number, box in enumerate(boxes):
        for i in range(int(box[0] // cell), int(box[1] // cell) + 1):
            for j in range(int(box[2] // cell), int(box[3] // cell) + 1):
                grid.setdefault((i, j), []).append(number)
    kept = []
    for source in added:
        at = source.get_editor_property("location")
        if str(source.get_editor_property("type")) == "Tree" and any(boxes[n][0] <= at.x <= boxes[n][1] and boxes[n][2] <= at.y <= boxes[n][3] for n in grid.get((int(at.x // cell), int(at.y // cell)), ())):
            continue
        kept.append(source)
    if len(kept) != len(added):
        print("[maps] %d painted trees inside an obstacle left out" % (len(added) - len(kept)))
    return kept


def reserved_discs(recipe, relays, module):
    """2026-11-06 (the user, on an Arriere-pays trial: "the altar, the camps, the capturable zones must have no resource on them"): [(x, y, radius)] where no
    painted or stretched source may stand: every slot's base (BASE_REACH), every relay's zone (+ 300 cm), every volcano altar (+ 300 cm)."""
    discs = [(recipe.slot(index)[0][0], recipe.slot(index)[0][1], BASE_REACH) for index in range(len(recipe.angles))]
    discs += [(relay[0], relay[1], (relay[2] if len(relay) > 2 else RELAY_RADIUS) + 300.0) for relay in relays]
    for site in (module.volcano(recipe) if hasattr(module, "volcano") else ()):
        discs.append((site["altar"][0], site["altar"][1], site["altar"][2] + 300.0))
    return discs


def drop_reserved(added, recipe, relays, module):
    """The painted or stretched sources (any type) inside a reserved disc (reserved_discs) left out."""
    discs = reserved_discs(recipe, relays, module)
    kept = []
    for source in added:
        at = source.get_editor_property("location")
        if any(math.dist((at.x, at.y), (x, y)) < radius for x, y, radius in discs):
            continue
        kept.append(source)
    if len(kept) != len(added):
        print("[maps] %s: %d painted sources on a camp, a relay or an altar left out" % (recipe.map_path, len(added) - len(kept)))
    return kept


def stretch_edits(recipe, sources, relays, module):
    """2026-11-05 (the user, after validating the painted Forest_Medium: "forget the ratio, it is an enlargement of the map, like a stretch, as far as the
    forests go"): a size with no edits file of its own takes the painted forests of the map's Medium file (Tools/Data/MapEdits/<Map>_Medium.json) when its
    settings say {"stretch": true, "source_half": <the Medium's bounds_half>}: the painted area is scaled by recipe.bounds_half / source_half, round the map's
    centre, and re-seeded on the same hex lattice (settings "lattice", cm), so that the trees follow the surface, not a budget. The recipe's own sources
    (bases, deposits, groves) stay where the recipe put them: a lattice node is dropped where it is too near one of them (relattice's rule), within BASE_REACH of a
    slot, in front of a ridge, in a relay's disc, or where the module's stretch_blocked(recipe, point) says (the Forest's roads). The Medium's own strokes (the
    erasers) are not carried over: they were drawn on the Medium recipe's groves."""
    import json
    name = recipe.map_path.rsplit("/", 1)[1]
    prefix = name.rsplit("_", 1)[0]
    folder = os.path.dirname(edits_path(recipe))
    # 2026-11-06 (the user: "do it for every map"): the reference is whichever size of the map the user painted (<Map>_<Size>.json, any size, not only the
    # Medium); its settings file may say {"stretch": false} to refuse, "lattice" (default 90 cm) and "source_half" (default: the half side of that size's
    # layout, Saved/MapLayouts/<file>.json, written by its generation). A size with an edits file of its own keeps its own strokes.
    if os.path.exists(os.path.join(folder, name + ".json")):
        return sources
    best = None
    for candidate in sorted(glob.glob(os.path.join(folder, prefix + "_*.json"))):
        stem = os.path.basename(candidate)[:-len(".json")]
        if stem.endswith(".settings") or stem == name:
            continue
        with open(candidate, encoding="utf-8") as handle:
            count = sum(1 for a in json.load(handle).get("add", []) if a["type"] == "Tree")
        if count > 0 and (best is None or count > best[1]):
            best = (stem, count)
    if best is None:
        return sources
    base = os.path.join(folder, best[0])
    settings = {}
    if os.path.exists(base + ".settings.json"):
        with open(base + ".settings.json", encoding="utf-8") as handle:
            settings = json.load(handle)
    if not settings.get("stretch", True):
        return sources
    settings.setdefault("lattice", 90.0)
    if "source_half" not in settings:
        layout = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_saved_dir()), "MapLayouts", best[0] + ".json")
        if not os.path.exists(layout):
            print("[maps] %s: no layout of %s (Saved/MapLayouts) and no source_half in its settings: its painted trees are not stretched" % (recipe.map_path, best[0]))
            return sources
        with open(layout, encoding="utf-8") as handle:
            settings["source_half"] = float(json.load(handle)["half"])
    if float(settings["lattice"]) <= 0:
        return sources
    with open(base + ".json", encoding="utf-8") as handle:
        painted_source = [(a["x"], a["y"]) for a in json.load(handle).get("add", []) if a["type"] == "Tree"]
    scale = recipe.bounds_half / float(settings["source_half"])
    spacing = float(settings["lattice"])
    row = spacing * 0.8660254
    cell = 200.0
    reach = 0.8 * spacing
    painted = {}
    for x, y in painted_source:
        painted.setdefault((int(x // cell), int(y // cell)), []).append((x, y))

    def near_painted(x, y):  # (x, y) in the Medium's coordinates
        cx, cy = int(x // cell), int(y // cell)
        return any(math.dist((x, y), q) <= reach for i in (-1, 0, 1) for j in (-1, 0, 1) for q in painted.get((cx + i, cy + j), ()))

    fixed = {}
    for source in sources:
        at = source.get_editor_property("location")
        kind = str(source.get_editor_property("type"))
        fixed.setdefault((int(at.x // cell), int(at.y // cell)), []).append((at.x, at.y, kind == "Tree"))
    slots = [recipe.slot(index)[0] for index in range(len(recipe.angles))]
    ridges = ridge_points(recipe)
    blocked_by_module = getattr(module, "stretch_blocked", None)
    margin = 600.0

    def blocked(x, y):
        if abs(x) > recipe.bounds_half - margin or abs(y) > recipe.bounds_half - margin:
            return True
        cx, cy = int(x // cell), int(y // cell)
        for i in (-1, 0, 1):
            for j in (-1, 0, 1):
                for ox, oy, is_tree in fixed.get((cx + i, cy + j), ()):
                    if math.dist((x, y), (ox, oy)) < (0.7 * spacing if is_tree else 200.0):
                        return True
        if any(math.dist((x, y), slot) < BASE_REACH for slot in slots):
            return True
        if any(math.dist((x, y), ridge) < 1000.0 for ridge in ridges):
            return True
        if any(math.dist((x, y), (relay[0], relay[1])) < (relay[2] if len(relay) > 2 else RELAY_RADIUS) + 300.0 for relay in relays):
            return True
        return bool(blocked_by_module and blocked_by_module(recipe, (x, y)))

    # Every lattice node of the target whose Medium-side image is near a painted tree: walk the 200 cm cells of the painted mask, scaled.
    nodes = set()
    for (cx, cy) in painted:
        x0, x1 = cx * cell * scale, (cx + 1) * cell * scale
        y0, y1 = cy * cell * scale, (cy + 1) * cell * scale
        for i in range(int(math.floor(x0 / row)), int(math.ceil(x1 / row)) + 1):
            for j in range(int(math.floor((y0 - spacing) / spacing)), int(math.ceil(y1 / spacing)) + 1):
                nodes.add((i, j))
    out = []
    for i, j in sorted(nodes):
        x = i * row + (edit_hash(i, j) - 0.5) * 0.16 * spacing
        y = j * spacing + (spacing / 2 if i & 1 else 0.0) + (edit_hash(j + 0.5, i) - 0.5) * 0.16 * spacing
        if near_painted(x / scale, y / scale) and not blocked(x, y):
            out.append(placed_source("Tree", round(x), round(y)))
    print("[maps] %s: painted forests of %s stretched x%.3f (lattice %.0f cm): %d trees added" % (recipe.map_path, best[0], scale, spacing, len(out)))
    return sources + drop_reserved(drop_buried(out), recipe, relays, module)


def apply_edits(recipe, sources, relays=(), module=None):
    """The map painter's strokes (2026-11-05, the user: "give me an editor, I paint on the map"), applied after the recipe: {"remove": [{"x", "y", "r", "types": [..] or absent}],
    "add": [{"type", "x", "y"}]} in centimetres, the world's coordinates. A remove drops the sources whose centre is within r of (x, y) (of the types listed, or
    any); an add places one source. No file = the recipe as it is."""
    path = edits_path(recipe)
    if not os.path.exists(path):
        return stretch_edits(recipe, sources, relays, module)
    import json
    with open(path, encoding="utf-8") as handle:
        edits = json.load(handle)
    kept = []
    for source in sources:
        at = source.get_editor_property("location")
        kind = str(source.get_editor_property("type"))
        if any(math.dist((at.x, at.y), (cut["x"], cut["y"])) <= cut["r"] and ("types" not in cut or kind in cut["types"]) for cut in edits.get("remove", [])):
            continue
        kept.append(source)
    # Tools/Data/MapEdits/<map file>.settings.json (2026-11-05, the user: "space the trees by 0.2 and take one tree in two out"): {"keep": 0.5, "gap": 110} -
    # keep = the fraction of the painted TREES kept (chosen by a hash of the position: even, no stripes), gap = the least distance in cm between two
    # painted trees (greedy, in the file's order, after the thinning). No file = every painted tree. The painter's forests are very dense (131 782 trees
    # on one Medium map): the numbers belong to the map, so that a plain regeneration gives the same file.
    keep, gap, lattice = 1.0, 0.0, 0.0
    settings_file = path.replace(".json", ".settings.json")
    if os.path.exists(settings_file):
        with open(settings_file, encoding="utf-8") as handle:
            settings = json.load(handle)
        keep, gap, lattice = float(settings.get("keep", 1.0)), float(settings.get("gap", 0.0)), float(settings.get("lattice", 0.0))
    wanted = [add for add in edits.get("add", []) if add["type"] != "Tree" or keep >= 1.0 or edit_hash(add["x"], add["y"]) < keep]
    if gap > 0:
        cells, spaced = {}, []
        for add in wanted:
            if add["type"] == "Tree":
                cx, cy = int(add["x"] // gap), int(add["y"] // gap)
                if any(math.dist((add["x"], add["y"]), other) < gap for dx in (-1, 0, 1) for dy in (-1, 0, 1) for other in cells.get((cx + dx, cy + dy), ())):
                    continue
                cells.setdefault((cx, cy), []).append((add["x"], add["y"]))
            spaced.append(add)
        wanted = spaced
    if lattice > 0:
        wanted = relattice(wanted, kept, lattice)
    added = drop_reserved(drop_buried([placed_source(add["type"], add["x"], add["y"]) for add in wanted]), recipe, relays, module)
    # 2026-11-06: the painter does not show the Forest's roads: a painted tree in a road's corridor (the module's stretch_blocked) is left out, as for a stretched one.
    blocked_by_module = getattr(module, "stretch_blocked", None)
    if blocked_by_module:
        before_roads = len(added)
        added = [a for a in added if str(a.get_editor_property("type")) != "Tree" or not blocked_by_module(recipe, (a.get_editor_property("location").x, a.get_editor_property("location").y))]
        if len(added) != before_roads:
            print("[maps] %s: %d painted trees in a road's corridor left out" % (recipe.map_path, before_roads - len(added)))
    print("[maps] %s: %d strokes applied, %d sources removed, %d added" % (recipe.map_path, len(edits.get("remove", [])) + len(edits.get("add", [])), len(sources) - len(kept), len(added)))
    return kept + added


def export_layout(recipe, relays, sources):
    """Saved/MapLayouts/<map file>.json for the map painter: bounds, slots, relays and every source (type, x, y)."""
    import json
    folder = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_saved_dir()), "MapLayouts")
    os.makedirs(folder, exist_ok=True)
    slots = [list(recipe.slot(index)[0]) for index in range(len(recipe.angles))]
    rows = []
    for source in sources:
        at = source.get_editor_property("location")
        rows.append([str(source.get_editor_property("type")), round(at.x), round(at.y)])
    data = {"map": recipe.map_path.rsplit("/", 1)[1], "half": recipe.bounds_half, "slots": slots, "relays": [list(relay) for relay in relays], "sources": rows}
    with open(os.path.join(folder, recipe.map_path.rsplit("/", 1)[1] + ".json"), "w", encoding="utf-8") as handle:
        json.dump(data, handle, separators=(",", ":"))


def make_map(recipe, module, on_saved):
    """One map file: the shared pieces, then the map module's obstacles and sources, then the navmesh and the save."""
    slot_class = unreal.load_class(None, "/Script/HeroesOfLiberty.HOLStartSlot")
    bounds_class = unreal.load_class(None, "/Script/HeroesOfLiberty.HOLMapBounds")
    layout_class = unreal.load_class(None, "/Script/HeroesOfLiberty.HOLSourceLayout")
    if slot_class is None or bounds_class is None or layout_class is None:
        raise RuntimeError("Missing HOLStartSlot, HOLMapBounds or HOLSourceLayout (is the HeroesOfLiberty module compiled?)")

    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not level_subsystem.new_level(recipe.map_path, False):
        raise RuntimeError("Could not create " + recipe.map_path)

    # Ground: the engine's 1 m plane, with a matte grid so that a moving camera shows (D265: no specular).
    spawn_mesh("/Engine/BasicShapes/Plane", unreal.Vector(0, 0, 0), unreal.Vector(recipe.ground_size / 100, recipe.ground_size / 100, 1),
               "Ground", ground_material(module))

    # Light and sky, all movable: nothing to build. No shadows in the game (D258). The map's ambience (maps/ambience.py: the sun's colour,
    # strength and angle, the sky's tint, a faint height fog; none = the engine's defaults) is set here, nothing else.
    sun_pitch, sun_yaw = ambience.rotation(recipe.map_name)
    sun = spawn(unreal.DirectionalLight, unreal.Vector(0, 0, 1000), unreal.Rotator(roll=0, pitch=sun_pitch, yaw=sun_yaw), "Sun")
    sun_light = movable_light(sun)
    sun_light.set_editor_property("atmosphere_sun_light", True)
    spawn(unreal.SkyAtmosphere, label="SkyAtmosphere")
    sky_light = spawn(unreal.SkyLight, label="SkyLight")
    sky = movable_light(sky_light)
    sky.set_editor_property("real_time_capture", True)
    fog = spawn(unreal.ExponentialHeightFog, label="HeightFog")
    ambience.apply(recipe.map_name, sun_light, sky, fog.get_editor_property("component"))

    # Bounds of the camera (its box's extent, read before the box is registered), and the navmesh over them (the
    # default volume brush is a 2 m cube).
    bounds = spawn(bounds_class, label="MapBounds")
    if recipe.bounds_half != BOUNDS_HALF_SIZE:
        bounds.get_editor_property("area").set_box_extent(unreal.Vector(recipe.bounds_half, recipe.bounds_half, 1000.0))
    nav = spawn(unreal.NavMeshBoundsVolume, label="NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(recipe.bounds_half / 100, recipe.bounds_half / 100, 5))

    # Start slots, each facing the centre.
    for index, angle in enumerate(recipe.angles):
        (x, y), _, _ = recipe.slot(index)
        slot = spawn(slot_class, unreal.Vector(x, y, PLAYER_START_HALF_HEIGHT), unreal.Rotator(roll=0, pitch=0, yaw=angle + 180.0),
                     "StartSlot%d" % index)
        slot.set_editor_property("slot_index", index)

    # The map's own pieces (the module contract above).
    obstacles = module.obstacles(recipe)

    # Resource sources (D297): data read by the server, nothing in the map itself (the navmesh is built without them).
    layout = spawn(layout_class, label="SourceLayout")
    relays, sources, groups = lay_relays_and_sources(recipe, module)
    sources = outside_relays(sources, relays)  # what the far layer's clearance did not keep out (R2)
    if not getattr(module, "NO_EDITS", False):  # the glade sweep (maps/glade_sweep.py) wears the name of a listed map and must not take its painted forests
        sources = apply_edits(recipe, sources, relays, module)  # the user's brush strokes (Tools/Data/MapEdits/<map file>.json, made by the map painter), over the recipe's sources
    export_layout(recipe, relays, sources)  # Saved/MapLayouts/<map file>.json: what the map painter shows
    layout.set_editor_property("sources", sources)
    if relays:
        spawn_relays(relays)

    # Volcano sites (V4b, D607): data read by the server, symmetric by construction; the module gives them (volcano(recipe)), the check refuses an unfair one.
    volcanoes = list(module.volcano(recipe)) if hasattr(module, "volcano") else []
    if volcanoes:
        unfair = volcano_unfair(recipe, volcanoes)
        if unfair:
            raise RuntimeError("%s: unfair volcano site: %s" % (recipe.map_path, "; ".join(unfair)))
        spawn_volcano(volcanoes)

    # Terrain zones (C3): data read by the server (the walkers' speed), and the ground drawn under them (zones(recipe) of the module).
    zones = module.zones(recipe) if hasattr(module, "zones") else ()
    if zones:
        spawn_zones(zones, getattr(module, "ZONE_COLORS", None))
        summary_zones = ", %d terrain zones" % len(zones)
    else:
        summary_zones = ""

    # Scenery, if the map wants some (DECOR in its module): no collision, nothing the navmesh or a click can touch.
    density = getattr(module, "DECOR", None)
    avoid = module.avoid_points(recipe) if hasattr(module, "avoid_points") else ()  # a map's own rocks (C2)
    scenery = spawn_decor(recipe, density, sources, avoid, getattr(module, "CLUMPS", None)) if density else None

    places = len(recipe.angles)
    neighbours = 2 * recipe.slot_radius * math.sin(math.pi / places) if places > 2 else 2 * recipe.slot_radius
    summary = ("Match map created at %s: %d start slots on a %.0f m circle (neighbours %.0f m apart), bounds %.0f m, %d obstacles, "
               "%d sources (%d neutral groups per player)%s, %d relays, navmesh built") % (
        recipe.map_path, places, recipe.slot_radius / 100, neighbours / 100, 2 * recipe.bounds_half / 100, obstacles, len(sources), groups, summary_zones, len(relays))
    if volcanoes:
        summary += ", %d volcano site(s) with %d flows" % (len(volcanoes), sum(len(site["flows"]) for site in volcanoes))
    # The navmesh is built once the editor lets it (it locks building for ~2 s after loading), then saved.
    if scenery:
        summary += ", scenery %d instances (%s)" % (sum(scenery.values()), ", ".join("%s %d" % item for item in sorted(scenery.items())))
    wait_for_navmesh(recipe, nav, level_subsystem, summary, on_saved)


def wait_for_navmesh(recipe, nav, level_subsystem, summary, on_saved):
    state = {"elapsed": 0.0, "handle": None, "done": False}

    def tick(delta):
        # A modal dialog (a save, a warning) ticks Slate again: never run twice.
        state["elapsed"] += delta
        if state["done"] or state["elapsed"] < NAV_BUILD_DELAY:
            return
        state["done"] = True
        unreal.unregister_slate_post_tick_callback(state["handle"])
        try:
            world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
            # The scale set from Python does not tell the navigation the volume's new size, and a navmesh made for
            # the default 2 m cube has a tile pool sized for it: the bounds are updated and that navmesh removed.
            unreal.NavigationSystemV1.get_navigation_system(world).on_navigation_bounds_updated(nav)
            for navmesh in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.RecastNavMesh):
                unreal.get_editor_subsystem(unreal.EditorActorSubsystem).destroy_actor(navmesh)
            unreal.SystemLibrary.execute_console_command(world, "BUILDPATHS")  # synchronous
            if not unreal.GameplayStatics.get_all_actors_of_class(world, unreal.RecastNavMesh):
                unreal.log_error("No navmesh was built in %s: map not saved" % recipe.map_path)
            elif not level_subsystem.save_current_level():
                unreal.log_error("Could not save " + recipe.map_path)
            else:
                unreal.log(summary)
        except Exception as error:  # never leave the editor open
            unreal.log_error("Match map: %s" % error)
        on_saved()

    state["handle"] = unreal.register_slate_post_tick_callback(tick)
