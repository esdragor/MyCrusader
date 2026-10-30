"""Creates /Game/Units/Props/SM_Banner, the Banner of the Dawn the hero plants (J8c.4, loop 4; drawn by banner_prims.py).

One static mesh for every banner in the world (an instanced mesh, like the arrows): four slots, the same instances of
/Game/Rendering/M_MatteColor (white cloth, gold, dark wood, stone footing). The origin is the pole's foot on the ground, the
cloth faces -X (the RTS camera never turns and looks along +X, D254). No collision (a banner is invulnerable, J8c-Q3, and
blocks nothing); no shadow (set on the component).
Unreal's triangles are wound the other way round from banner_prims' (as the props of the mannequin, J5c.2c): reversed here.
Run (no window):
  UnrealEditor-Cmd.exe <project> -run=pythonscript -script="<this file>" -unattended -nullrhi -nosplash -nosound
"""
import os
import sys

import unreal

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import anim_rig  # noqa: E402
import banner_prims  # noqa: E402

FOLDER = "/Game/Units/Props"
TARGET_PATH = FOLDER + "/SM_Banner"
PARENT = "/Game/Rendering/M_MatteColor"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
lib = unreal.MaterialEditingLibrary
parent = unreal.load_asset(PARENT)
if not parent:
    raise RuntimeError("Missing " + PARENT)


def material_instance(name, rgb):
    path = FOLDER + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        mi = unreal.load_asset(path)  # kept and set again (deleting an asset in a commandlet can hang)
    else:
        mi = asset_tools.create_asset(name, FOLDER, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    lib.set_material_instance_parent(mi, parent)
    # M_MatteColor's Color is linear (the first pass gave the sRGB shares as they are: washed out in the light): converted.
    lin = [((c / 255.0 + 0.055) / 1.055) ** 2.4 if c / 255.0 > 0.04045 else c / 255.0 / 12.92 for c in rgb]
    lib.set_material_instance_vector_parameter_value(mi, "Color", unreal.LinearColor(lin[0], lin[1], lin[2], 1.0))
    unreal.EditorAssetLibrary.save_loaded_asset(mi)
    return mi


names = ["MI_Banner" + name.capitalize() for name, _ in banner_prims.COLORS]
materials = [material_instance(n, rgb) for n, (_, rgb) in zip(names, banner_prims.COLORS)]
slot_of = {rgb: k for k, (_, rgb) in enumerate(banner_prims.COLORS)}

if unreal.EditorAssetLibrary.does_asset_exist(TARGET_PATH):
    unreal.EditorAssetLibrary.delete_asset(TARGET_PATH)

mesh = unreal.DynamicMesh()
total = 0
for slot in range(len(banner_prims.COLORS)):
    vertices, normals, triangles = [], [], []
    for tri, color in banner_prims.triangles():
        if slot_of[color] != slot:
            continue
        a, b, c = tri
        n = anim_rig.cross(anim_rig.sub(b, a), anim_rig.sub(c, a))
        if anim_rig.length(n) < 1e-9:
            continue
        n = anim_rig.normalize(n)
        base = len(vertices)
        vertices += [a, b, c]
        normals += [n, n, n]
        triangles.append((base, base + 2, base + 1))
    if not triangles:
        continue
    buffers = unreal.GeometryScriptSimpleMeshBuffers()
    buffers.set_editor_property("vertices", [unreal.Vector(*v) for v in vertices])
    buffers.set_editor_property("normals", [unreal.Vector(*n) for n in normals])
    buffers.set_editor_property("uv0", [unreal.Vector2D(v[1] / 100.0, v[2] / 100.0) for v in vertices])
    buffers.set_editor_property("triangles", [unreal.IntVector(*t) for t in triangles])
    unreal.GeometryScript_MeshEdits.append_buffers_to_mesh(mesh, buffers, slot)
    total += len(triangles)

new_options = unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
new_options.enable_collision = False
static_mesh, outcome = unreal.GeometryScript_NewAssetUtils.create_new_static_mesh_asset_from_mesh(mesh, TARGET_PATH, new_options)
if outcome != unreal.GeometryScriptOutcomePins.SUCCESS:
    raise RuntimeError("Could not create " + TARGET_PATH)
static_materials = []
for name, material in zip(names, materials):
    sm = unreal.StaticMaterial()
    sm.material_interface = material
    sm.material_slot_name = name
    static_materials.append(sm)
static_mesh.set_editor_property("static_materials", static_materials)
unreal.EditorAssetLibrary.save_loaded_asset(static_mesh)
bounds = static_mesh.get_bounding_box()
unreal.log("Banner mesh created at %s (%d of %d triangles, bounds %s to %s)" %
           (TARGET_PATH, static_mesh.get_num_triangles(0), total, bounds.min, bounds.max))
