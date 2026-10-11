"""Creates /Game/Maps/StressTest: flat ground, lights, navmesh bounds (M0, § 16.6).

Run in a full editor session (commandlets have no navigation system, and -ExecutePythonScript
quits before the navmesh can be built); the editor quits when done:
  UnrealEditor.exe <project> -ExecCmds="py <this file>"
"""
import os

import unreal

MAP_PATH = "/Game/Maps/StressTest"
HALF_SIZE_M = 200.0  # 400 m x 400 m ground, room for 2 000 units

level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

# Checked on disk: the asset registry cache can still list a deleted map at startup.
if os.path.exists(os.path.join(unreal.Paths.project_content_dir(), "Maps", "StressTest.umap")):
    unreal.SystemLibrary.quit_editor()
    raise RuntimeError(MAP_PATH + " already exists: delete Content/Maps/StressTest.umap first")

if not level_subsystem.new_level(MAP_PATH, False):
    raise RuntimeError("Could not create " + MAP_PATH)


def spawn(actor_class, location=unreal.Vector(0, 0, 0), rotation=unreal.Rotator(0, 0, 0), label=None):
    actor = actor_subsystem.spawn_actor_from_class(actor_class, location, rotation)
    if label:
        actor.set_actor_label(label)
    return actor


# Ground: engine plane is 100 x 100 cm.
ground = spawn(unreal.StaticMeshActor, label="Ground")
ground.static_mesh_component.set_static_mesh(unreal.load_asset("/Engine/BasicShapes/Plane.Plane"))
ground.static_mesh_component.set_material(0, unreal.load_asset("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"))
ground.set_actor_scale3d(unreal.Vector(HALF_SIZE_M * 2, HALF_SIZE_M * 2, 1))

# Lighting.
sun = spawn(unreal.DirectionalLight, unreal.Vector(0, 0, 1000), unreal.Rotator(0, -45, 30), "Sun")
sun.light_component.set_editor_property("atmosphere_sun_light", True)
spawn(unreal.SkyAtmosphere, label="SkyAtmosphere")
sky = spawn(unreal.SkyLight, label="SkyLight")
sky.light_component.set_editor_property("real_time_capture", True)
sky.light_component.set_mobility(unreal.ComponentMobility.MOVABLE)
spawn(unreal.ExponentialHeightFog, label="HeightFog")

# Navmesh bounds: brush volume is 200 x 200 x 200 cm.
nav_bounds = spawn(unreal.NavMeshBoundsVolume, label="NavMeshBounds")
nav_bounds.set_actor_scale3d(unreal.Vector(HALF_SIZE_M, HALF_SIZE_M, 5))

spawn(unreal.PlayerStart, unreal.Vector(0, 0, 200), label="PlayerStart")

# Static navmesh, saved with the map. The navigation system locks building until
# the editor has been idle for ~2 s after loading, so wait on the editor tick.
NAV_BUILD_DELAY_S = 5.0
elapsed = 0.0
tick_handle = None


def build_navigation_and_save(delta_seconds):
    global elapsed
    elapsed += delta_seconds
    if elapsed < NAV_BUILD_DELAY_S:
        return
    unreal.unregister_slate_post_tick_callback(tick_handle)
    unreal.log("StressTest: building navigation after %.1f s" % elapsed)

    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    unreal.SystemLibrary.execute_console_command(world, "BUILDPATHS")
    if not any(isinstance(a, unreal.RecastNavMesh) for a in actor_subsystem.get_all_level_actors()):
        unreal.log_error("Navmesh was not built")
    elif not level_subsystem.save_current_level():
        unreal.log_error("Could not save " + MAP_PATH)
    else:
        unreal.log("StressTest map created at " + MAP_PATH)
    unreal.SystemLibrary.quit_editor()


tick_handle = unreal.register_slate_post_tick_callback(build_navigation_and_save)
unreal.log("StressTest: actors spawned, waiting for the navigation build lock")
