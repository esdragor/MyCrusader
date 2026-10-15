"""Creates /Game/Maps/Plain, the first match map (D256, J1): a flat ground, light and sky, eight start slots and the
map bounds. Provisional: no terrain nor resources yet. The main centres are not in the map: the game mode puts one
on each occupied slot when a match starts (D259).

  ground       500 m x 500 m, matte grid material, a line every 10 m (the camera's movement shows; D265)
  bounds       400 m x 400 m (AHOLMapBounds): the camera's focus stays inside
  navmesh      over the bounds (NavMeshBoundsVolume), built here and saved with the map: the project's navmesh only
               takes modifiers at run time (D286, the buildings' footprints), so the map must carry it
  start slots  AHOLStartSlot 0 to 7 on a circle of 150 m, facing the centre; the first ones far apart (slot 0 and 1
               face to face for one against one, 0 to 3 at the four points of the compass...)
  obstacles    so that the paths show (D286, J4): in front of each slot, towards the middle, a ridge of three
               boulders; in the middle of the map, a grove of trees (the units walk between the trunks). Symmetric.
               Their matte materials /Game/Rendering/MI_Rock and MI_Tree are made when missing.
  sources      the resources of the match (D295, D297), as data in an AHOLSourceLayout the server reads at the start:
               a wood of 40 trees beside each slot (J5a), on its left seen from the slot, 18 to 27 m from it, out of the
               way of the Peasants born at the centre's foot. Trees 2.2 m apart: with the navmesh's agent radius, a
               wood is a wall, as in AoE, and its edge recedes tree by tree. The same small offsets for every slot.

Needs the HeroesOfLiberty module compiled. Run in a full editor session (the editor quits when done):
  UnrealEditor.exe <project> -nosplash -unattended -ExecCmds="py <this file>"
"""
import math
import os

import unreal

MAP_PATH = "/Game/Maps/Plain"
GROUND_SIZE = 50000.0  # cm
BOUNDS_HALF_SIZE = 20000.0
SLOT_RADIUS = 15000.0
# Slot i at this angle (degrees): every first n slots are spread around the circle.
SLOT_ANGLES = [180.0, 0.0, 270.0, 90.0, 225.0, 45.0, 315.0, 135.0]
PLAYER_START_HALF_HEIGHT = 92.0

# Obstacles (D286). A ridge across the way from each slot to the middle, RIDGE_RADIUS from the map's centre: 30 m in
# front of the main centre, out of the 15 m centre's way and of the Peasants born at its foot.
RIDGE_RADIUS = 12000.0
RIDGE_BOULDERS = [(-650.0, unreal.Vector(7.0, 4.5, 3.0)), (0.0, unreal.Vector(8.0, 5.0, 3.6)), (650.0, unreal.Vector(6.5, 4.0, 2.8))]
# Grove: trunks of 5 m, crowns from 4 m up (the units pass under them, the trunks block), on a disc of 14 m.
GROVE_TREES = [(0.0, 0.0), (700.0, 250.0), (-500.0, 600.0), (-650.0, -400.0), (250.0, -750.0), (1200.0, -300.0),
               (-1250.0, 100.0), (400.0, 1150.0), (-200.0, -1300.0), (1000.0, 900.0), (-1000.0, -1000.0), (-900.0, 1150.0)]
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


def spawn_obstacles():
    rock = matte_instance("MI_Rock", ROCK_COLOR)
    trunk = matte_instance("MI_Trunk", TRUNK_COLOR)
    crown = matte_instance("MI_Crown", CROWN_COLOR)
    for index, angle in enumerate(SLOT_ANGLES):
        radians = math.radians(angle)
        radial = unreal.Vector(math.cos(radians), math.sin(radians), 0)
        across = unreal.Vector(-radial.y, radial.x, 0)
        for number, (offset, scale) in enumerate(RIDGE_BOULDERS):
            # The engine's 1 m sphere, flattened and half buried.
            location = radial * RIDGE_RADIUS + across * offset
            spawn_shape("/Engine/BasicShapes/Sphere", unreal.Vector(location.x, location.y, 0), scale, "Ridge%d_%d" % (index, number),
                        rock, yaw=angle + 90.0 + (number - 1) * 12.0)
    for number, (x, y) in enumerate(GROVE_TREES):
        # The engine's 1 m cylinder is centred: a 5 m trunk stands on the ground at Z = 250 cm.
        spawn_shape("/Engine/BasicShapes/Cylinder", unreal.Vector(x, y, 250), unreal.Vector(0.8, 0.8, 5.0), "Trunk%d" % number, trunk)
        spawn_shape("/Engine/BasicShapes/Sphere", unreal.Vector(x, y, 650), unreal.Vector(4.5, 4.5, 3.5), "Crown%d" % number, crown)


def make_sources():
    """The sources of the match (D297): a wood beside each slot, as AHOLSourceLayout data."""
    sources = []
    for angle in SLOT_ANGLES:
        radians = math.radians(angle)
        radial = (math.cos(radians), math.sin(radians))
        left = (-radial[1], radial[0])
        slot = (SLOT_RADIUS * radial[0], SLOT_RADIUS * radial[1])
        number = 0
        for row in range(WOOD_ROWS):
            for column in range(WOOD_COLUMNS):
                side = WOOD_NEAR + row * WOOD_SPACING
                along = (column - (WOOD_COLUMNS - 1) / 2.0) * WOOD_SPACING
                jitter = WOOD_JITTER[number % len(WOOD_JITTER)]
                number += 1
                side += jitter[0]
                along += jitter[1]
                x = slot[0] + left[0] * side + radial[0] * along
                y = slot[1] + left[1] * side + radial[1] * along
                source = unreal.HOLPlacedSource()
                source.set_editor_property("type", "Tree")
                source.set_editor_property("location", unreal.Vector(x, y, 0))
                sources.append(source)
    return sources


def movable_light(actor):
    """The light component of a light actor, made movable (nothing to build)."""
    component = actor.get_editor_property("light_component")
    component.set_mobility(unreal.ComponentMobility.MOVABLE)
    return component


def main():
    if os.path.exists(os.path.join(unreal.Paths.project_content_dir(), "Maps", "Plain.umap")):
        raise RuntimeError(MAP_PATH + " already exists: delete Content/Maps/Plain.umap first")
    slot_class = unreal.load_class(None, "/Script/HeroesOfLiberty.HOLStartSlot")
    bounds_class = unreal.load_class(None, "/Script/HeroesOfLiberty.HOLMapBounds")
    layout_class = unreal.load_class(None, "/Script/HeroesOfLiberty.HOLSourceLayout")
    if slot_class is None or bounds_class is None or layout_class is None:
        raise RuntimeError("Missing HOLStartSlot, HOLMapBounds or HOLSourceLayout (is the HeroesOfLiberty module compiled?)")

    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not level_subsystem.new_level(MAP_PATH, False):
        raise RuntimeError("Could not create " + MAP_PATH)

    # Ground: the engine's 1 m plane, with a matte grid so that a moving camera shows (D265: no specular).
    spawn_mesh("/Engine/BasicShapes/Plane", unreal.Vector(0, 0, 0), unreal.Vector(GROUND_SIZE / 100, GROUND_SIZE / 100, 1),
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

    # Bounds of the camera, and the navmesh over them (the default volume brush is a 2 m cube).
    spawn(bounds_class, label="MapBounds")
    nav = spawn(unreal.NavMeshBoundsVolume, label="NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(BOUNDS_HALF_SIZE / 100, BOUNDS_HALF_SIZE / 100, 5))

    # Start slots, each facing the centre.
    for index, angle in enumerate(SLOT_ANGLES):
        radians = math.radians(angle)
        x, y = SLOT_RADIUS * math.cos(radians), SLOT_RADIUS * math.sin(radians)
        slot = spawn(slot_class, unreal.Vector(x, y, PLAYER_START_HALF_HEIGHT), unreal.Rotator(roll=0, pitch=0, yaw=angle + 180.0),
                     "StartSlot%d" % index)
        slot.set_editor_property("slot_index", index)

    spawn_obstacles()

    # Resource sources (D297): data read by the server, nothing in the map itself (the navmesh is built without them).
    layout = spawn(layout_class, label="SourceLayout")
    layout.set_editor_property("sources", make_sources())

    # The navmesh is built once the editor lets it (it locks building for ~2 s after loading), then saved.
    wait_for_navmesh(nav, level_subsystem)


def wait_for_navmesh(nav, level_subsystem):
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
                unreal.log_error("No navmesh was built in %s: map not saved" % MAP_PATH)
            elif not level_subsystem.save_current_level():
                unreal.log_error("Could not save " + MAP_PATH)
            else:
                unreal.log("Match map created at %s: %d start slots on a %.0f m circle, bounds %.0f m, %d obstacles, %d sources, navmesh built" % (
                    MAP_PATH, len(SLOT_ANGLES), SLOT_RADIUS / 100, 2 * BOUNDS_HALF_SIZE / 100,
                    len(SLOT_ANGLES) * len(RIDGE_BOULDERS) + 2 * len(GROVE_TREES), len(SLOT_ANGLES) * WOOD_ROWS * WOOD_COLUMNS))
        except Exception as error:  # never leave the editor open
            unreal.log_error("Match map: %s" % error)
        unreal.SystemLibrary.quit_editor()

    state["handle"] = unreal.register_slate_post_tick_callback(tick)


try:
    main()
except Exception:
    unreal.SystemLibrary.quit_editor()
    raise
