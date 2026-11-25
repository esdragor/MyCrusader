"""Creates /Game/Units/Props/SM_Arrow, the arrow drawn in flight (J7c.2b): one instanced mesh for every arrow in the air;
with "--bolt", /Game/Units/Props/SM_Bolt instead, the Crossbowman's bolt in flight (AR-Q5, AR6); with "--bullet",
/Game/Units/Props/SM_Bullet, the Arquebusier's ball (a dark ball with a pale trail, drawn through UHOLArrowSubsystem's
ArrowScale 1.5 x 4 x 4, so the mesh ellipsoid is 6 x 2.25 x 2.25 cm: ~18 cm across in flight); with "--smoke",
/Game/Units/Props/SM_PowderSmoke, the puff at the muzzle at each shot (DA_Art_Arquebusier MuzzleSmoke, art_visual_io.py): a 50 cm ball, centred, in
the translucent /Game/Units/Props/M_PowderSmoke (make_smoke_material.py, to run first; its opacity is per-instance data 0).

SM_Arrow, ~78 cm (soldier_props.ARROW_LENGTH, make_arrow): a thin wooden shaft, an iron point, three white fletchings.
SM_Bolt, 35 cm (soldier_props.BOLT_LENGTH, bolt_prims): a thicker shaft, a square iron head, two vanes across.
Both: the origin is the point and the mesh lies along +X (the flight draws it along its velocity, the point reaching the
target's chest). Three slots, the same instances of /Game/Rendering/M_MatteColor (wood, iron, feather). No collision; no
shadow (set on the component, D258).
Run (no window):
  UnrealEditor-Cmd.exe <project> -run=pythonscript -script="<this file> [--bolt]" -unattended -nullrhi -nosplash -nosound
"""
import os
import sys

import unreal

sys.dont_write_bytecode = True  # no __pycache__ in Tools/Editor
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import art_visual_io  # noqa: E402  (AR0-d2 3b: the shared writer of the DA_Art fields)

BOLT = "--bolt" in sys.argv[1:]
BULLET = "--bullet" in sys.argv[1:]
SMOKE = "--smoke" in sys.argv[1:]
FOLDER = "/Game/Units/Props"
TARGET_PATH = FOLDER + ("/SM_PowderSmoke" if SMOKE else "/SM_Bullet" if BULLET else "/SM_Bolt" if BOLT else "/SM_Arrow")
PARENT = "/Game/Rendering/M_MatteColor"
# The art assets that draw this mesh (D925): type -> DA_Art field (formerly the Projectile / MuzzleSmoke columns).
ART_FIELDS = {
    FOLDER + "/SM_Arrow": [("Archer", "Projectile")],
    FOLDER + "/SM_Bolt": [("Crossbowman", "Projectile"), ("PaviseCrossbowman", "Projectile")],
    FOLDER + "/SM_Bullet": [("Arquebusier", "Projectile")],
    FOLDER + "/SM_PowderSmoke": [("Arquebusier", "MuzzleSmoke")],
}
LENGTH = 35.0 if BOLT else 78.0
COLORS = {"MI_ArrowWood": (0.42, 0.27, 0.13), "MI_ArrowIron": (0.30, 0.30, 0.32), "MI_ArrowFeather": (0.89, 0.87, 0.82)}
if BULLET:
    COLORS = {"MI_BulletLead": (0.10, 0.10, 0.11), "MI_BulletTrail": (0.78, 0.77, 0.73)}
if SMOKE:
    COLORS = {"M_PowderSmoke": None}  # the slot's material is the translucent M_PowderSmoke itself, not a matte instance

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
lib = unreal.MaterialEditingLibrary
parent = unreal.load_asset(PARENT)
if not parent:
    raise RuntimeError("Missing " + PARENT)


def material_instance(name, rgb):
    path = FOLDER + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    mi = asset_tools.create_asset(name, FOLDER, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    lib.set_material_instance_parent(mi, parent)
    lib.set_material_instance_vector_parameter_value(mi, "Color", unreal.LinearColor(rgb[0], rgb[1], rgb[2], 1.0))
    unreal.EditorAssetLibrary.save_loaded_asset(mi)
    return mi


if SMOKE:
    smoke_material = unreal.load_asset(FOLDER + "/M_PowderSmoke")
    if not smoke_material:
        raise RuntimeError("Missing " + FOLDER + "/M_PowderSmoke: run make_smoke_material.py first")
    materials = [smoke_material]
else:
    materials = [material_instance(n, c) for n, c in COLORS.items()]

if unreal.EditorAssetLibrary.does_asset_exist(TARGET_PATH):
    unreal.EditorAssetLibrary.delete_asset(TARGET_PATH)

P = unreal.GeometryScript_Primitives
mesh = unreal.DynamicMesh()
BASE = unreal.GeometryScriptPrimitiveOriginMode.BASE
CENTER = unreal.GeometryScriptPrimitiveOriginMode.CENTER
ALONG_X = unreal.Rotator(roll=0.0, pitch=-90.0, yaw=0.0)  # the primitives' +Z turned to +X


def options(material_id):
    o = unreal.GeometryScriptPrimitiveOptions()
    o.material_id = material_id
    return o


if SMOKE:
    # A ball of 50 cm radius centred on the muzzle point, 10 x 8 facets.
    P.append_sphere_lat_long(mesh, options(0), unreal.Transform(unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0)), 50.0, 8, 10, CENTER)
elif BULLET:
    # The ball: an ellipsoid 6 x 2.25 x 2.25 cm (x 1.5 x 4 x 4 in flight = 9 cm radius) ending at the origin, and a thin
    # pale trail behind it along the flight (x 1.5 in flight: ~60 cm).
    P.append_sphere_lat_long(mesh, options(0), unreal.Transform(unreal.Vector(-6.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0), unreal.Vector(6.0, 2.25, 2.25)), 1.0, 8, 12, CENTER)
    P.append_cylinder(mesh, options(1), unreal.Transform(unreal.Vector(-46.0, 0.0, 0.0), ALONG_X), 0.5, 34.0, 4, 1, True, BASE)
elif BOLT:
    # Shaft: from the nock (-35) to the head's base (-4.5), thicker than the arrow's (1.0 against 0.6) to read in flight.
    P.append_cylinder(mesh, options(0), unreal.Transform(unreal.Vector(-LENGTH, 0.0, 0.0), ALONG_X), 1.0, LENGTH - 4.5, 4, 1, True, BASE)
    # Square head: a four-sided cone from -4.5 to 0 (the bolt's bodkin).
    P.append_cone(mesh, options(1), unreal.Transform(unreal.Vector(-4.5, 0.0, 0.0), ALONG_X), 1.7, 0.05, 4.5, 4, 1, True, BASE)
    # Two vanes across (above and below the shaft, as soldier_props.bolt_prims), 6 cm long, 4 cm from the nock.
    for sign in (1.0, -1.0):
        P.append_box(mesh, options(2), unreal.Transform(unreal.Vector(-LENGTH + 4.0, 0.0, sign * 2.1), unreal.Rotator(0.0, 0.0, 0.0)),
                     6.0, 0.3, 2.2, 0, 0, 0, CENTER)
else:
    # Shaft: from the nock (-78) to the point's base (-6).
    P.append_cylinder(mesh, options(0), unreal.Transform(unreal.Vector(-LENGTH, 0.0, 0.0), ALONG_X), 0.6, LENGTH - 6.0, 6, 1, True, BASE)
    # Point: a cone from -6 to 0.
    P.append_cone(mesh, options(1), unreal.Transform(unreal.Vector(-6.0, 0.0, 0.0), ALONG_X), 1.4, 0.05, 6.0, 6, 1, True, BASE)
    # Three fletchings near the nock, 120 degrees apart.
    for k in range(3):
        roll = 120.0 * k
        # A thin plate 14 cm long, 2.6 cm high, standing out of the shaft.
        offset = unreal.Vector(0.0, 0.0, 1.6).rotate_angle_axis(roll, unreal.Vector(1.0, 0.0, 0.0)) if hasattr(unreal.Vector, "rotate_angle_axis") else None
        loc = unreal.Vector(-LENGTH + 10.0, 0.0, 0.0)
        if offset is not None:
            loc = unreal.Vector(loc.x + offset.x, loc.y + offset.y, loc.z + offset.z)
        P.append_box(mesh, options(2), unreal.Transform(loc, unreal.Rotator(roll=roll, pitch=0.0, yaw=0.0)), 14.0, 0.3, 2.6, 0, 0, 0,
                     CENTER)

new_options = unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
new_options.enable_collision = False
static_mesh, outcome = unreal.GeometryScript_NewAssetUtils.create_new_static_mesh_asset_from_mesh(mesh, TARGET_PATH, new_options)
if outcome != unreal.GeometryScriptOutcomePins.SUCCESS:
    raise RuntimeError("Could not create " + TARGET_PATH)
static_materials = []
for i, (name, _) in enumerate(COLORS.items()):
    sm = unreal.StaticMaterial()
    sm.material_interface = materials[i]
    sm.material_slot_name = name
    static_materials.append(sm)
static_mesh.set_editor_property("static_materials", static_materials)
unreal.EditorAssetLibrary.save_loaded_asset(static_mesh)
bounds = static_mesh.get_bounding_box()
unreal.log("%s mesh created at %s (%d triangles, bounds %s to %s)" % ("Smoke" if SMOKE else "Bullet" if BULLET else "Bolt" if BOLT else "Arrow", TARGET_PATH,
           static_mesh.get_num_triangles(0), bounds.min, bounds.max))

# AR0-d2 (3b, D925): the mesh goes into the art asset of every type that draws it (DA_Art_<Type>), never into a CSV.
for type_name, field in ART_FIELDS[TARGET_PATH]:
    art_visual_io.set_art_field("Units", type_name, field, TARGET_PATH + "." + TARGET_PATH.rsplit("/", 1)[1])
