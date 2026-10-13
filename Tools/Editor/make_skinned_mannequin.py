"""Creates the M2 trial assets: instanced skeletal mannequins and their animation providers (D232 to D235).

From the project's UE4 mannequin (/Game/CloseCombat/Character/UE4Mannequin, the source of SM_Mannequin):
  /Game/Units/Materials/M_UnitPlayerColor           flat material tinted by the instance's custom data 0-2 (D233)
  /Game/Units/Mannequin/SKM_Mannequin_Classic       copy with a generated LOD chain (D235)
  /Game/Units/Mannequin/SKM_Mannequin_Nanite        copy built as Nanite (D235)
  /Game/Units/Mannequin/TP_Mannequin_Classic        animation provider of each copy: sequence 0 = idle, 1 = walk (D234)
  /Game/Units/Mannequin/TP_Mannequin_Nanite
Re-running the script updates these assets in place (deleting a referenced asset can leave it loaded, and the
creation that follows then fails); delete a mesh by hand to copy it again from the source.
Run (the editor quits when done):
  UnrealEditor.exe <project> -ExecutePythonScript=<this file>
"""
import unreal

SOURCE_MESH = "/Game/CloseCombat/Character/UE4Mannequin/Meshes/SK_Mannequin"
ANIMATIONS = "/Game/CloseCombat/Character/UE4Mannequin/Animations/Swordsman/Movement/"
# Order = sequence index read by the presentation (D234).
SEQUENCES = [ANIMATIONS + "A_SW_Idle", ANIMATIONS + "A_SW_Walk_F"]

MESH_FOLDER = "/Game/Units/Mannequin"
MATERIAL_FOLDER = "/Game/Units/Materials"
MATERIAL_NAME = "M_UnitPlayerColor"
CLASSIC_LOD_COUNT = 4

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
assets = unreal.EditorAssetLibrary


def load(path):
    asset = unreal.load_asset(path)
    if asset is None:
        raise RuntimeError("Missing " + path)
    return asset


def existing(path):
    return unreal.load_asset(path) if assets.does_asset_exist(path) else None


def make_material():
    """Base colour = the instance's custom data 0-2 (player colour, written by the client, D233)."""
    path = MATERIAL_FOLDER + "/" + MATERIAL_NAME
    lib = unreal.MaterialEditingLibrary
    material = existing(path)
    if material is not None:
        lib.delete_all_material_expressions(material)
    else:
        material = asset_tools.create_asset(MATERIAL_NAME, MATERIAL_FOLDER, unreal.Material, unreal.MaterialFactoryNew())
    if material is None:
        raise RuntimeError("Could not create " + path)
    # Without both usages the engine draws the default material instead (SkeletalMeshSceneProxy.cpp): instanced
    # skinned meshes skin inline on the GPU and need their own shader permutations.
    for usage in (unreal.MaterialUsage.MATUSAGE_SKELETAL_MESH, unreal.MaterialUsage.MATUSAGE_INSTANCED_SKINNED_MESH):
        lib.set_base_material_usage(material, usage, True)

    color = lib.create_material_expression(material, unreal.MaterialExpressionPerInstanceCustomData3Vector, -400, 0)
    color.set_editor_property("data_index", 0)
    # Light grey when no colour is written (a unit with no owner stays readable).
    color.set_editor_property("const_default_value", unreal.LinearColor(0.6, 0.6, 0.6, 1.0))
    lib.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
    roughness = lib.create_material_expression(material, unreal.MaterialExpressionConstant, -400, 200)
    roughness.set_editor_property("r", 0.7)
    lib.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    lib.recompile_material(material)
    assets.save_loaded_asset(material)
    return material


def make_mesh(name, material, nanite):
    path = MESH_FOLDER + "/" + name
    mesh = existing(path) or assets.duplicate_asset(SOURCE_MESH, path)
    if mesh is None:
        raise RuntimeError("Could not duplicate " + SOURCE_MESH + " to " + path)

    # Every slot takes the player colour material: the trial mannequin is tinted whole (D233).
    # New structs: editing the ones get_editor_property returns changes copies, and the mesh keeps its materials.
    slots = []
    for source_slot in mesh.get_editor_property("materials"):
        slot = unreal.SkeletalMaterial()
        slot.set_editor_property("material_slot_name", source_slot.get_editor_property("material_slot_name"))
        slot.set_editor_property("material_interface", material)
        slots.append(slot)
    mesh.set_editor_property("materials", slots)
    if any(slot.get_editor_property("material_interface") != material for slot in mesh.get_editor_property("materials")):
        raise RuntimeError("Could not set the player colour material on " + path)

    if nanite:
        settings = mesh.get_editor_property("nanite_settings")
        settings.set_editor_property("enabled", True)
        mesh.set_editor_property("nanite_settings", settings)
    elif not unreal.SkeletalMeshEditorSubsystem.regenerate_lod(mesh, CLASSIC_LOD_COUNT, True, False):
        raise RuntimeError("Could not generate the LODs of " + path)

    # The animation provider needs the unified bone map that "Optimize for Instancing" builds, on every LOD.
    lod_count = unreal.SkeletalMeshEditorSubsystem.get_lod_count(mesh)
    for lod in range(lod_count):
        build_settings = unreal.SkeletalMeshEditorSubsystem.get_lod_build_settings(mesh, lod)
        build_settings.set_editor_property("optimize_for_instancing", True)
        unreal.SkeletalMeshEditorSubsystem.set_lod_build_settings(mesh, lod, build_settings)

    assets.save_loaded_asset(mesh)
    subsystem = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
    vertices = [subsystem.get_num_verts(mesh, lod) for lod in range(lod_count)]
    unreal.log("%s: Nanite %s, vertices per LOD %s" % (path, nanite, vertices))
    return mesh


def make_provider(name, mesh, sequences):
    path = MESH_FOLDER + "/" + name
    provider = existing(path)
    if provider is None:
        factory = unreal.TransformProviderDataFactory()
        factory.set_editor_property("provider_data_class", unreal.AnimSequenceTransformProviderData)
        provider = asset_tools.create_asset(name, MESH_FOLDER, unreal.AnimSequenceTransformProviderData, factory)
    if provider is None:
        raise RuntimeError("Could not create " + path)

    provider.set_editor_property("skinned_asset", mesh)
    entries = []
    for sequence in sequences:
        entry = unreal.AnimSequenceTransformProviderSequence()
        entry.set_editor_property("sequence", sequence)
        entries.append(entry)
    provider.set_editor_property("sequences", entries)
    # The runtime needs at least one layer (InitLayerPools); one full-body layer, nothing blended over it.
    provider.set_editor_property("layers", [unreal.AnimSequenceTransformProviderLayer()])
    assets.save_loaded_asset(provider)
    unreal.log("%s: %d sequences on %s" % (path, len(entries), mesh.get_path_name()))
    return provider


def log_sequence(sequence):
    """Length and root travel: an animation with root motion would drift the drawn unit off its entity."""
    length = sequence.get_play_length()
    start = unreal.AnimationLibrary.get_bone_pose_for_time(sequence, "root", 0.0, False)
    end = unreal.AnimationLibrary.get_bone_pose_for_time(sequence, "root", length, False)
    travel = end.translation - start.translation
    unreal.log("%s: %.2f s, root motion %s, root travel %.1f cm"
               % (sequence.get_name(), length, sequence.get_editor_property("enable_root_motion"), travel.length()))


def main():
    sequences = [load(path) for path in SEQUENCES]
    for sequence in sequences:
        log_sequence(sequence)
    material = make_material()
    for variant, nanite in (("Classic", False), ("Nanite", True)):
        mesh = make_mesh("SKM_Mannequin_" + variant, material, nanite)
        make_provider("TP_Mannequin_" + variant, mesh, sequences)
    unreal.log("Skinned mannequin assets created")


main()
