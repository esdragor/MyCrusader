"""Creates /Game/Units/Mannequin/SM_Mannequin, a static copy of SK_Mannequin (M1, § 16.6).

Light units are drawn as instances, which need a static mesh (AnimToTexture also bakes onto one).
Run (the editor quits when done):
  UnrealEditor.exe <project> -ExecutePythonScript=<this file>
"""
import unreal

SOURCE_PATH = "/Game/CloseCombat/Character/UE4Mannequin/Meshes/SK_Mannequin"
TARGET_PATH = "/Game/Units/Mannequin/SM_Mannequin"

if unreal.EditorAssetLibrary.does_asset_exist(TARGET_PATH):
    raise RuntimeError(TARGET_PATH + " already exists")

skeletal_mesh = unreal.load_asset(SOURCE_PATH)
if not skeletal_mesh:
    raise RuntimeError("Missing " + SOURCE_PATH)

dynamic_mesh = unreal.DynamicMesh()
read_lod = unreal.GeometryScriptMeshReadLOD()
dynamic_mesh, outcome = unreal.GeometryScript_AssetUtils.copy_mesh_from_skeletal_mesh(
    skeletal_mesh, dynamic_mesh, unreal.GeometryScriptCopyMeshFromAssetOptions(), read_lod)
if outcome != unreal.GeometryScriptOutcomePins.SUCCESS:
    raise RuntimeError("Could not read " + SOURCE_PATH)

options = unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
options.enable_collision = False
options.use_original_vertex_order = True
static_mesh, outcome = unreal.GeometryScript_NewAssetUtils.create_new_static_mesh_asset_from_mesh(
    dynamic_mesh, TARGET_PATH, options)
if outcome != unreal.GeometryScriptOutcomePins.SUCCESS:
    raise RuntimeError("Could not create " + TARGET_PATH)

# Keep the skeletal mesh materials, slot by slot.
static_materials = []
for skeletal_material in skeletal_mesh.materials:
    static_material = unreal.StaticMaterial()
    static_material.material_interface = skeletal_material.material_interface
    static_material.material_slot_name = skeletal_material.material_slot_name
    static_materials.append(static_material)
static_mesh.set_editor_property("static_materials", static_materials)

# The source is ~41k triangles: generate LODs so 2 000 instances stay affordable (LOD0 untouched).
reduction = unreal.StaticMeshReductionOptions()
reduction.auto_compute_lod_screen_size = False
reduction.reduction_settings = [
    unreal.StaticMeshReductionSettings(1.0, 1.0),
    unreal.StaticMeshReductionSettings(0.25, 0.5),
    unreal.StaticMeshReductionSettings(0.06, 0.2),
    unreal.StaticMeshReductionSettings(0.015, 0.07),
]
mesh_subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
if mesh_subsystem.set_lods(static_mesh, reduction) != len(reduction.reduction_settings):
    raise RuntimeError("Could not generate LODs")

unreal.EditorAssetLibrary.save_loaded_asset(static_mesh)
triangles = [static_mesh.get_num_triangles(lod) for lod in range(static_mesh.get_num_lods())]
unreal.log("Static mannequin created at %s (triangles per LOD: %s)" % (TARGET_PATH, triangles))
