"""Creates /Game/Maps/Plain, the first match map (D256, J1): a flat ground, light and sky, eight start slots and the
map bounds. Provisional: no terrain nor resources yet; a marker shows each start until the main centre of J2.

  ground       500 m x 500 m, grid material (the camera's movement shows)
  bounds       400 m x 400 m (AHOLMapBounds): the camera's focus stays inside
  navmesh      over the bounds (NavMeshBoundsVolume)
  start slots  AHOLStartSlot 0 to 7 on a circle of 150 m, facing the centre; the first ones far apart (slot 0 and 1
               face to face for one against one, 0 to 3 at the four points of the compass...)

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
MARKER_DIAMETER = 800.0
MARKER_HEIGHT = 200.0
PLAYER_START_HALF_HEIGHT = 92.0


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
    if slot_class is None or bounds_class is None:
        raise RuntimeError("Missing HOLStartSlot or HOLMapBounds (is the HeroesOfLiberty module compiled?)")

    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not level_subsystem.new_level(MAP_PATH, False):
        raise RuntimeError("Could not create " + MAP_PATH)

    # Ground: the engine's 1 m plane, with the world grid so that a moving camera shows.
    spawn_mesh("/Engine/BasicShapes/Plane", unreal.Vector(0, 0, 0), unreal.Vector(GROUND_SIZE / 100, GROUND_SIZE / 100, 1),
               "Ground", "/Engine/EngineMaterials/WorldGridMaterial")

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

    # Start slots, each facing the centre, with a marker on the ground.
    for index, angle in enumerate(SLOT_ANGLES):
        radians = math.radians(angle)
        x, y = SLOT_RADIUS * math.cos(radians), SLOT_RADIUS * math.sin(radians)
        slot = spawn(slot_class, unreal.Vector(x, y, PLAYER_START_HALF_HEIGHT), unreal.Rotator(roll=0, pitch=0, yaw=angle + 180.0),
                     "StartSlot%d" % index)
        slot.set_editor_property("slot_index", index)
        # The engine's cylinder is 1 m high, centred on its location.
        spawn_mesh("/Engine/BasicShapes/Cylinder", unreal.Vector(x, y, MARKER_HEIGHT / 2),
                   unreal.Vector(MARKER_DIAMETER / 100, MARKER_DIAMETER / 100, MARKER_HEIGHT / 100), "SlotMarker%d" % index)

    if not level_subsystem.save_current_level():
        raise RuntimeError("Could not save " + MAP_PATH)
    unreal.log("Match map created at %s: %d start slots on a %.0f m circle, bounds %.0f m" % (
        MAP_PATH, len(SLOT_ANGLES), SLOT_RADIUS / 100, 2 * BOUNDS_HALF_SIZE / 100))


try:
    main()
finally:
    unreal.SystemLibrary.quit_editor()
