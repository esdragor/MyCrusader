"""The recipe of Plaine (D256, J1; TC2): a flat ground, light and sky, the start slots, the map bounds, the obstacles and
the sources. Made once per named size (TC-Q3 = A, Tools/Data/MapSizes.csv), as /Game/Maps/Plain_<Size> (Small,
Medium, Normal, Large, Huge); without a size, /Game/Maps/Plain as it has always been (the scenarios and quick_check
count on its slots). The main centres are not in the map: the game mode puts one on each occupied slot when a match
starts (D259).

  ground       the bounds and 50 m more on each side (TC-Q5), matte grid material, a line every tile of 4 m (D265, D344)
  bounds       AHOLMapBounds, the size's side (Plain: 400 m): the camera's focus stays inside
  navmesh      over the bounds (NavMeshBoundsVolume), built here and saved with the map: the project's navmesh only
               takes modifiers at run time (D286, the buildings' footprints), so the map must carry it
  start slots  AHOLStartSlot 0 to N-1 (N: the size's places), facing the centre on a circle of SLOT_RADIUS_FRACTION of
               the half side, numbered in order around it (place k and k+1 neighbours, FHOLMapSizeRules::DrawPlaces):
               2 face to face, 4 at the points of the compass, 6 and 8 evenly. Plain: 8 slots on a circle of 150 m,
               the first ones far apart (SLOT_ANGLES: 0 and 1 face to face, 0 to 3 at the points of the compass...)
  obstacles    so that the paths show (D286, J4): 30 m in front of each slot, towards the middle, a ridge of three
               boulders, no navmesh on their tops (a NavArea_Null volume each); in the middle of the map, a grove of
               trees (the units walk between the trunks), with rings of trees around it on the larger sizes.
               Their matte materials /Game/Rendering/MI_Rock and MI_Tree are made when missing.
  sources      the resources of the match (D295, D297), as data in an AHOLSourceLayout the server reads at the start.
               Each player's base, the same at every size, relative to its slot (TC-Q7 = B):
               a wood of 40 trees beside each slot (J5a), on its left seen from the slot, 18 to 27 m from it, out of the
               way of the Peasants born at the centre's foot. Trees 2.2 m apart: with the navmesh's agent radius, a
               wood is a wall, as in AoE, and its edge recedes tree by tree. The same small offsets for every slot.
               Beside each slot on the other side (J5b.1), 6 berry bushes, 20 to 23 m from it: farther than the nearest
               trees (the scripted checks aim at the tree nearest a start). Two veins ~60 m from the slot, towards each
               neighbour: Plain puts one between each two neighbouring slots, stone and gold in turn, as far from one
               as from the other (contested, D295); on the sized maps the bases are farther apart, so each slot has its
               own, stone on its wood's side, gold on the other, at the same place relative to it.
               Plaine's neutral sources (sized maps only, TC-Q7 = B: each map has its own recipe, see
               plain_neutral_sources): between each two neighbouring slots, along the line from the middle of their
               arc to the centre, woods and veins in turn, more of them the more room each player has.

Needs the HeroesOfLiberty module compiled. Run in a full editor session (the editor quits when done):
  UnrealEditor.exe <project> -nosplash -unattended -ExecCmds="py <this file> [all | <size>...]"
No argument: /Game/Maps/Plain. Sizes: the keys of Tools/Data/MapSizes.csv (Small Medium Normal Large Huge), or all.
A map file already there is never overwritten: delete Content/Maps/<map>.umap first.
"""
import csv
import math
import os
import sys

import unreal

MAP_PATH = "/Game/Maps/Plain"
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

# Obstacles (D286). A ridge across the way from each slot to the middle, RIDGE_AHEAD in front of the slot: 30 m in front
# of the main centre, out of the 12 m centre's way and of the Peasants born at its foot.
RIDGE_AHEAD = 3000.0
RIDGE_BOULDERS = [(-650.0, unreal.Vector(7.0, 4.5, 3.0)), (0.0, unreal.Vector(8.0, 5.0, 3.6)), (650.0, unreal.Vector(6.5, 4.0, 2.8))]
# No navmesh on a boulder's top (MV.4, the user's choice A): its flat top would carry its own patch, cut off from the
# ground, where a unit born in the rock stays stuck. A NavArea_Null volume over each boulder, from ROCK_NULL_BOTTOM up
# (above the ground's spans: the ground around the rock keeps its navmesh), its footprint the rock's with a margin.
ROCK_NULL_BOTTOM = 50.0
ROCK_NULL_TOP = 450.0
ROCK_NULL_MARGIN = 100.0
# Grove: trunks of 5 m, crowns from 4 m up (the units pass under them, the trunks block), on a disc of 14 m.
GROVE_TREES = [(0.0, 0.0), (700.0, 250.0), (-500.0, 600.0), (-650.0, -400.0), (250.0, -750.0), (1200.0, -300.0),
               (-1250.0, 100.0), (400.0, 1150.0), (-200.0, -1300.0), (1000.0, 900.0), (-1000.0, -1000.0), (-900.0, 1150.0)]
# Sized maps: one ring of trees round the grove for every GROVE_RING_PER of half side past Plain's, GROVE_RING_SPACING
# apart from GROVE_FIRST_RING, the trees ~GROVE_TREE_GAP apart (a multiple of the slots: symmetric).
GROVE_RING_PER = 6000.0
GROVE_FIRST_RING = 2100.0
GROVE_RING_SPACING = 700.0
GROVE_TREE_GAP = 700.0
ROCK_COLOR = unreal.LinearColor(0.33, 0.31, 0.29, 1.0)
TRUNK_COLOR = unreal.LinearColor(0.22, 0.15, 0.09, 1.0)
CROWN_COLOR = unreal.LinearColor(0.10, 0.22, 0.08, 1.0)
NAV_BUILD_DELAY = 5.0  # seconds

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
# Veins between neighbouring slots (J5b.1): at the middle angle of two slots, VEIN_RADIUS from the map's centre (inside
# the slots' circle: the way from a slot to the vein on its wood's side passes ~7 m beside the wood). Stone first.
VEIN_RADIUS = 12000.0
VEIN_TYPES = ["StoneVein", "GoldVein"]
# The same place relative to the slot on the sized maps: VEIN_AHEAD towards the centre, VEIN_SIDE to each side (~60 m).
VEIN_AHEAD = SLOT_RADIUS - VEIN_RADIUS * math.cos(math.radians(22.5))
VEIN_SIDE = VEIN_RADIUS * math.sin(math.radians(22.5))

# Plaine's neutral sources (TC-Q7 = B). Each player's room: the side squared over the places, less its base (a disc of
# BASE_REACH); one group per NEUTRAL_AREA of it (Small 4, Medium 4, Normal 3, Large 3, Huge 4 per player). The groups of
# each arc on the line from its middle to the centre, from NEUTRAL_OUTER to NEUTRAL_INNER of the slots' radius, in the
# order of NEUTRAL_PATTERN from the outside.
BASE_REACH = 6000.0
NEUTRAL_AREA = 2.0e8  # cm2: 2 ha
NEUTRAL_OUTER = 0.9
NEUTRAL_INNER = 0.3
NEUTRAL_PATTERN = ["Wood", "GoldVein", "Wood", "StoneVein"]
NEUTRAL_WOOD_ROWS = 4
NEUTRAL_WOOD_COLUMNS = 5


class Recipe:
    """What one map file is made of: its path, its slots (angle of each, in slot order), their circle and its bounds."""

    def __init__(self, map_path, angles, slot_radius, bounds_half, ground_size, size_key=None):
        self.map_path = map_path
        self.angles = angles
        self.slot_radius = slot_radius
        self.bounds_half = bounds_half
        self.ground_size = ground_size
        self.size_key = size_key

    def slot(self, index):
        """Place, outward direction and left (seen from the slot, facing the centre: counterclockwise) of a slot."""
        radians = math.radians(self.angles[index])
        radial = (math.cos(radians), math.sin(radians))
        return (self.slot_radius * radial[0], self.slot_radius * radial[1]), radial, (-radial[1], radial[0])


def plain_recipe():
    return Recipe(MAP_PATH, SLOT_ANGLES, SLOT_RADIUS, BOUNDS_HALF_SIZE, GROUND_SIZE)


def sized_recipe(key, places, side_tiles):
    half = 0.5 * side_tiles * TILE_SIZE
    angles = [(FIRST_SLOT_ANGLE + index * 360.0 / places) % 360.0 for index in range(places)]
    return Recipe("%s_%s" % (MAP_PATH, key), angles, SLOT_RADIUS_FRACTION * half, half, 2 * (half + GROUND_MARGIN), key)


def read_sizes():
    """Tools/Data/MapSizes.csv (TC1, the copy of FHOLMapSizeRules' table): key -> (places, side in tiles)."""
    path = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()), "Tools", "Data", "MapSizes.csv")
    with open(path, newline="", encoding="utf-8") as file:
        return {row["Name"]: (int(row["NumPlaces"]), int(row["SideTiles"])) for row in csv.DictReader(file)}


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


def spawn_shape(mesh_path, location, scale, label, material, yaw=0.0):
    actor = spawn(unreal.StaticMeshActor, location, unreal.Rotator(roll=0, pitch=0, yaw=yaw), label)
    component = actor.get_editor_property("static_mesh_component")
    component.set_static_mesh(unreal.load_asset(mesh_path))
    component.set_material(0, material)
    component.set_editor_property("cast_shadow", False)  # D258
    actor.set_actor_scale3d(scale)
    return actor


def grove_trees(recipe):
    """The grove's trees: Plain's disc, and on the sized maps rings round it, more of them the larger the map."""
    trees = list(GROVE_TREES)
    if recipe.size_key is None:
        return trees
    places = len(recipe.angles)
    for ring in range(int((recipe.bounds_half - BOUNDS_HALF_SIZE) / GROVE_RING_PER)):
        radius = GROVE_FIRST_RING + ring * GROVE_RING_SPACING
        count = places * max(1, int(math.ceil(2 * math.pi * radius / GROVE_TREE_GAP / places)))
        for number in range(count):
            # Every other ring turned by half a gap: no straight way through.
            radians = math.radians(FIRST_SLOT_ANGLE + (number + 0.5 * (ring % 2)) * 360.0 / count)
            trees.append((radius * math.cos(radians), radius * math.sin(radians)))
    return trees


def spawn_obstacles(recipe):
    rock = matte_instance("MI_Rock", ROCK_COLOR)
    trunk = matte_instance("MI_Trunk", TRUNK_COLOR)
    crown = matte_instance("MI_Crown", CROWN_COLOR)
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
    trees = grove_trees(recipe)
    for number, (x, y) in enumerate(trees):
        # The engine's 1 m cylinder is centred: a 5 m trunk stands on the ground at Z = 250 cm.
        spawn_shape("/Engine/BasicShapes/Cylinder", unreal.Vector(x, y, 250), unreal.Vector(0.8, 0.8, 5.0), "Trunk%d" % number, trunk)
        spawn_shape("/Engine/BasicShapes/Sphere", unreal.Vector(x, y, 650), unreal.Vector(4.5, 4.5, 3.5), "Crown%d" % number, crown)
    return len(recipe.angles) * len(RIDGE_BOULDERS) + 2 * len(trees)


def placed_source(type_name, x, y):
    source = unreal.HOLPlacedSource()
    source.set_editor_property("type", type_name)
    source.set_editor_property("location", unreal.Vector(x, y, 0))
    return source


def wood(centre, along_axis, side_axis, rows, columns, first_side):
    """The trees of a wood: rows on side_axis from first_side, columns along along_axis centred on centre."""
    trees = []
    number = 0
    for row in range(rows):
        for column in range(columns):
            side = first_side + row * WOOD_SPACING
            along = (column - (columns - 1) / 2.0) * WOOD_SPACING
            jitter = WOOD_JITTER[number % len(WOOD_JITTER)]
            number += 1
            side += jitter[0]
            along += jitter[1]
            trees.append(placed_source("Tree", centre[0] + side_axis[0] * side + along_axis[0] * along,
                                       centre[1] + side_axis[1] * side + along_axis[1] * along))
    return trees


def base_sources(recipe):
    """Each player's base (D295), the same at every size: the woods first (source 0 is the first tree of slot 0's
    wood, the scripted checks use it), then the berries, then the veins."""
    sources = []
    for index in range(len(recipe.angles)):
        slot, radial, left = recipe.slot(index)
        sources += wood(slot, radial, left, WOOD_ROWS, WOOD_COLUMNS, WOOD_NEAR)
    for index in range(len(recipe.angles)):
        slot, radial, left = recipe.slot(index)
        right = (-left[0], -left[1])
        for row in range(BERRY_ROWS):
            for column in range(BERRY_COLUMNS):
                side = BERRY_NEAR + row * BERRY_SPACING
                along = (column - (BERRY_COLUMNS - 1) / 2.0) * BERRY_SPACING
                sources.append(placed_source("BerryBush", slot[0] + right[0] * side + radial[0] * along,
                                             slot[1] + right[1] * side + radial[1] * along))
    if recipe.size_key is None:
        # Plain's slot angles are multiples of 45 degrees: a vein at each odd multiple of 22.5 degrees, between two
        # neighbours.
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


def plain_neutral_sources(recipe):
    """Plaine's own neutral sources (TC-Q7 = B; another map, El Dorado say, would have its own function): on the line
    from the middle of each two neighbouring slots' arc to the centre, NEUTRAL_PATTERN's groups in turn, as many as the
    room of each player holds. Symmetric: the same for each arc, turned."""
    places = len(recipe.angles)
    side = 2 * recipe.bounds_half
    room = side * side / places - math.pi * BASE_REACH * BASE_REACH
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
                first = -(NEUTRAL_WOOD_ROWS - 1) / 2.0 * WOOD_SPACING
                sources += wood(at, radial, across, NEUTRAL_WOOD_ROWS, NEUTRAL_WOOD_COLUMNS, first)
            else:
                sources.append(placed_source(kind, at[0], at[1]))
    return sources, groups


def make_sources(recipe):
    """The sources of the match (D297), as AHOLSourceLayout data."""
    sources = base_sources(recipe)
    groups = 0
    if recipe.size_key is not None:
        neutral, groups = plain_neutral_sources(recipe)
        sources += neutral
    return sources, groups


def movable_light(actor):
    """The light component of a light actor, made movable (nothing to build)."""
    component = actor.get_editor_property("light_component")
    component.set_mobility(unreal.ComponentMobility.MOVABLE)
    return component


def map_file(recipe):
    return os.path.join(unreal.Paths.project_content_dir(), "Maps", recipe.map_path.rsplit("/", 1)[1] + ".umap")


def make_map(recipe, on_saved):
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
               "Ground", "/Game/Rendering/M_MatteGround")

    # Light and sky, all movable: nothing to build. No shadows in the game (D258).
    sun = spawn(unreal.DirectionalLight, unreal.Vector(0, 0, 1000), unreal.Rotator(roll=0, pitch=-50, yaw=30), "Sun")
    sun_light = movable_light(sun)
    sun_light.set_editor_property("atmosphere_sun_light", True)
    sun_light.set_editor_property("cast_shadows", False)
    spawn(unreal.SkyAtmosphere, label="SkyAtmosphere")
    sky_light = spawn(unreal.SkyLight, label="SkyLight")
    sky = movable_light(sky_light)
    sky.set_editor_property("real_time_capture", True)
    sky.set_editor_property("cast_shadows", False)
    spawn(unreal.ExponentialHeightFog, label="HeightFog")

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

    obstacles = spawn_obstacles(recipe)

    # Resource sources (D297): data read by the server, nothing in the map itself (the navmesh is built without them).
    layout = spawn(layout_class, label="SourceLayout")
    sources, groups = make_sources(recipe)
    layout.set_editor_property("sources", sources)

    places = len(recipe.angles)
    neighbours = 2 * recipe.slot_radius * math.sin(math.pi / places) if places > 2 else 2 * recipe.slot_radius
    summary = ("Match map created at %s: %d start slots on a %.0f m circle (neighbours %.0f m apart), bounds %.0f m, %d obstacles, "
               "%d sources (%d neutral groups per player), navmesh built") % (
        recipe.map_path, places, recipe.slot_radius / 100, neighbours / 100, 2 * recipe.bounds_half / 100, obstacles, len(sources), groups)
    # The navmesh is built once the editor lets it (it locks building for ~2 s after loading), then saved.
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


def recipes_of(arguments):
    """No argument: Plain. Otherwise the sizes named (keys of MapSizes.csv), or all of them."""
    if not arguments:
        return [plain_recipe()]
    sizes = read_sizes()
    keys = list(sizes) if [argument.lower() for argument in arguments] == ["all"] else arguments
    unknown = [key for key in keys if key not in sizes]
    if unknown:
        raise RuntimeError("Unknown map sizes %s (Tools/Data/MapSizes.csv: %s)" % (unknown, ", ".join(sizes)))
    return [sized_recipe(key, *sizes[key]) for key in keys]


def main():
    recipes = recipes_of(sys.argv[1:])
    present = [map_file(recipe) for recipe in recipes if os.path.exists(map_file(recipe))]
    if present:
        raise RuntimeError("Already there, delete first: %s" % ", ".join(present))
    queue = list(recipes)

    def next_map():
        if not queue:
            unreal.SystemLibrary.quit_editor()
            return
        try:
            make_map(queue.pop(0), next_map)
        except Exception as error:
            unreal.log_error("Match map: %s" % error)
            unreal.SystemLibrary.quit_editor()

    next_map()


try:
    main()
except Exception:
    unreal.SystemLibrary.quit_editor()
    raise
