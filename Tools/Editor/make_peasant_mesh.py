"""Builds the Peasant's own skinned mesh with its props, its material and its animation provider (J5c.2c-e).

  /Game/Units/Materials/M_UnitPlayerColorProps   the player colour material (M_UnitPlayerColor, D233, D265) plus the
                                                 props: a prop vertex takes its vertex colour, and is folded onto the
                                                 object's origin by World Position Offset unless the instance's custom
                                                 data 3 names its prop
  /Game/Units/Mannequin/SKM_Peasant              the mannequin's body (SKM_Mannequin_Classic, same LODs) and every prop
                                                 of peasant_props.py, each vertex skinned 100 % to its prop's bone
  /Game/Units/Mannequin/TP_Peasant               its animation provider: TP_Mannequin_Classic's sequences, same order
                                                 (so the same indices in the type table), then the loaded idles (8 food,
                                                 9 wood, 10 stone and gold)

The props (peasant_props.mesh_parts) are written in the bind pose, in component space, as peasant_props.bind_mesh() puts
them: flat faceted primitives, one sRGB colour per face. A vertex's colour alpha holds the MASK of the props it belongs
to (bit id - 1; 0 = the body, always shown): the material shows a vertex when its mask is 0 or has the bit of the prop
in the instance's custom data 3 (FHOLResourceProps, 0 = none). Colour space: Geometry Script hands the vertex colours
to the mesh description through an sRGB-to-linear transform and the skeletal mesh build stores them as they are, so the
shader reads the linear colour of the sRGB value written here (the alpha is not transformed: mask / 255 exactly).

Each LOD is written on its own (CopyMeshToSkeletalMesh with no reduction): the body of SKM_Mannequin_Classic's same
LOD, then every prop at full detail (a prop drawn at the RTS camera's distance must not be reduced away). The LOD
screen sizes and build settings are those of SKM_Mannequin_Classic (SKM_Peasant starts as its duplicate);
"Optimize for Instancing" is set on every LOD (the provider needs its unified bone map; ik_hand_gun now carries the
tools' weights, so it stays in it).

Run (headless, editor closed, from PowerShell; log lines start with "HOLPeasantMesh"):
  UnrealEditor-Cmd.exe <project> -run=pythonscript -script="<this file>" -unattended -nop4 -nosplash
  add "--provider-only" inside the -script value to refresh TP_Peasant alone (mesh and material kept), or
  "--material-only" for the material alone.
"""
import os
import sys

import unreal

TOOLS_DIR = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()), "Tools", "Editor")
sys.dont_write_bytecode = True  # no __pycache__ in Tools/Editor
sys.path.insert(0, TOOLS_DIR)
import anim_rig  # noqa: E402
import peasant_props  # noqa: E402

SOURCE_MESH = "/Game/Units/Mannequin/SKM_Mannequin_Classic"
SOURCE_PROVIDER = "/Game/Units/Mannequin/TP_Mannequin_Classic"
MESH_FOLDER = "/Game/Units/Mannequin"
MESH_NAME = "SKM_Peasant"
PROVIDER_NAME = "TP_Peasant"
MATERIAL_FOLDER = "/Game/Units/Materials"
MATERIAL_NAME = "M_UnitPlayerColorProps"
PEASANT_ANIMATIONS = "/Game/Units/Mannequin/Animations/"
# Appended after TP_Mannequin_Classic's sequences (indices 8, 9, 10 in Tools/Data/UnitTypes.csv, CarryIdleSequences).
# Add at the end only: an index that moves changes what the Peasant plays.
LOADED_IDLES = [
    PEASANT_ANIMATIONS + "A_Peasant_IdleCarryFood",   # 8 standing with food
    PEASANT_ANIMATIONS + "A_Peasant_IdleCarryWood",   # 9 standing with wood
    PEASANT_ANIMATIONS + "A_Peasant_IdleCarryStone",  # 10 standing with stone and gold
]
SKELETON_JSON = os.path.join(TOOLS_DIR, "Data", "UE4MannequinSkeleton.json")

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
assets = unreal.EditorAssetLibrary
geo = unreal.GeometryScript_AssetUtils
queries = unreal.GeometryScript_MeshQueries
weights = unreal.GeometryScript_BoneWeights


def log(text):
    unreal.log("HOLPeasantMesh: " + text)


def load(path):
    asset = unreal.load_asset(path)
    if asset is None:
        raise RuntimeError("Missing " + path)
    return asset


def existing(path):
    return unreal.load_asset(path) if assets.does_asset_exist(path) else None


def values(result, kind):
    """The values of type `kind` in a Python binding's return (a value or a tuple of out parameters)."""
    return [value for value in (result if isinstance(result, tuple) else (result,)) if isinstance(value, kind)]


def first(result, kind):
    found = values(result, kind)
    return found[0] if found else None


# ---------------------------------------------------------------------------------------------------------------------
# Material


# A vertex is hidden when its mask (colour alpha, one byte) is not 0 and does not hold the bit of the prop named by the
# instance (custom data 3: 0 none, 1-8 a prop).
HIDDEN_CODE = """uint Bits = (uint)round(saturate(Mask) * 255.0);
uint Shown = (uint)round(clamp(Prop, 0.0, 255.0));
bool bVisible = Bits == 0u || (Shown >= 1u && Shown <= 8u && ((Bits >> (Shown - 1u)) & 1u) != 0u);
return bVisible ? 0.0 : 1.0;"""
IS_PROP_CODE = "return Mask * 255.0 > 0.5 ? 1.0 : 0.0;"


def custom_node(lib, material, name, code, inputs, x, y):
    node = lib.create_material_expression(material, unreal.MaterialExpressionCustom, x, y)
    node.set_editor_property("description", name)
    node.set_editor_property("code", code)
    node.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT1)
    custom_inputs = []
    for input_name, _ in inputs:
        custom_input = unreal.CustomInput()
        custom_input.set_editor_property("input_name", input_name)
        custom_inputs.append(custom_input)
    node.set_editor_property("inputs", custom_inputs)
    for input_name, (source, output) in inputs:
        if not lib.connect_material_expressions(source, output, node, input_name):
            raise RuntimeError("Could not connect %s to %s.%s" % (source.get_name(), name, input_name))
    return node


def make_material():
    """Player colour on the body as M_UnitPlayerColor (custom data 0-2, matte); vertex colour on the props; the props
    not named by custom data 3 folded by World Position Offset onto the object's origin (zero-area triangles: nothing
    drawn, in the shadows too)."""
    path = MATERIAL_FOLDER + "/" + MATERIAL_NAME
    lib = unreal.MaterialEditingLibrary
    material = existing(path)
    if material is not None:
        lib.delete_all_material_expressions(material)
    else:
        material = asset_tools.create_asset(MATERIAL_NAME, MATERIAL_FOLDER, unreal.Material, unreal.MaterialFactoryNew())
    if material is None:
        raise RuntimeError("Could not create " + path)
    # Without both usages the engine draws the default material instead (see make_skinned_mannequin.py).
    for usage in (unreal.MaterialUsage.MATUSAGE_SKELETAL_MESH, unreal.MaterialUsage.MATUSAGE_INSTANCED_SKINNED_MESH):
        lib.set_base_material_usage(material, usage, True)

    player = lib.create_material_expression(material, unreal.MaterialExpressionPerInstanceCustomData3Vector, -900, 0)
    player.set_editor_property("data_index", 0)
    player.set_editor_property("const_default_value", unreal.LinearColor(0.6, 0.6, 0.6, 1.0))
    prop = lib.create_material_expression(material, unreal.MaterialExpressionPerInstanceCustomData, -900, 400)
    prop.set_editor_property("data_index", 3)
    prop.set_editor_property("const_default_value", 0.0)
    vertex_color = lib.create_material_expression(material, unreal.MaterialExpressionVertexColor, -900, 200)

    # Base colour: the player's on the body, the vertex colour on a prop.
    is_prop = custom_node(lib, material, "IsProp", IS_PROP_CODE, [("Mask", (vertex_color, "A"))], -600, 200)
    color = lib.create_material_expression(material, unreal.MaterialExpressionLinearInterpolate, -300, 0)
    lib.connect_material_expressions(player, "", color, "A")
    lib.connect_material_expressions(vertex_color, "", color, "B")
    lib.connect_material_expressions(is_prop, "", color, "Alpha")
    lib.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)

    # World Position Offset: hidden props to the object's origin.
    hidden = custom_node(lib, material, "PropHidden", HIDDEN_CODE,
                         [("Mask", (vertex_color, "A")), ("Prop", (prop, ""))], -600, 400)
    origin = lib.create_material_expression(material, unreal.MaterialExpressionObjectPositionWS, -900, 600)
    position = lib.create_material_expression(material, unreal.MaterialExpressionWorldPosition, -900, 700)
    to_origin = lib.create_material_expression(material, unreal.MaterialExpressionSubtract, -600, 650)
    lib.connect_material_expressions(origin, "", to_origin, "A")
    lib.connect_material_expressions(position, "", to_origin, "B")
    offset = lib.create_material_expression(material, unreal.MaterialExpressionMultiply, -300, 500)
    lib.connect_material_expressions(to_origin, "", offset, "A")
    lib.connect_material_expressions(hidden, "", offset, "B")
    lib.connect_material_property(offset, "", unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)

    # Matte (D265), as M_UnitPlayerColor.
    specular = lib.create_material_expression(material, unreal.MaterialExpressionConstant, -300, 200)
    specular.set_editor_property("r", 0.0)
    lib.connect_material_property(specular, "", unreal.MaterialProperty.MP_SPECULAR)
    roughness = lib.create_material_expression(material, unreal.MaterialExpressionConstant, -300, 300)
    roughness.set_editor_property("r", 1.0)
    lib.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    lib.recompile_material(material)
    assets.save_loaded_asset(material)
    log("materiau %s : %d noeuds" % (path, lib.get_num_material_expressions(material)))
    return material


# ---------------------------------------------------------------------------------------------------------------------
# Mesh


def bone_names(mesh):
    """The mesh's reference skeleton, in order (the skin weights' bone indices)."""
    component = unreal.new_object(unreal.SkeletalMeshComponent)
    component.set_skeletal_mesh_asset(mesh)
    return [str(component.get_bone_name(index)) for index in range(component.get_num_bones())]


def read_lod(mesh, lod, kind="SOURCE_MODEL"):
    dynamic = unreal.DynamicMesh()
    requested = unreal.GeometryScriptMeshReadLOD()
    requested.set_editor_property("lod_type", getattr(unreal.GeometryScriptLODType, kind))
    requested.set_editor_property("lod_index", lod)
    options = unreal.GeometryScriptCopyMeshFromAssetOptions()
    result = geo.copy_mesh_from_skeletal_mesh(mesh, dynamic, options, requested)
    outcome = first(result, unreal.GeometryScriptOutcomePins)
    if outcome != unreal.GeometryScriptOutcomePins.SUCCESS:
        raise RuntimeError("Could not read LOD %d (%s) of %s" % (lod, kind, mesh.get_path_name()))
    return dynamic


def has_weights(dynamic):
    """Whether the copy kept the skin weights (a vertex of the torso has one)."""
    count = queries.get_vertex_count(dynamic)
    for vid in range(0, count, max(1, count // 50)):
        weight = first(weights.get_largest_vertex_bone_weight(dynamic, vid), unreal.GeometryScriptBoneWeight)
        if weight is not None and weight.get_editor_property("weight") > 0.0:
            return True
    return False


def body_lod(source, lod, skeleton, weighted):
    """The body of the source mesh's LOD with its skin weights. LOD 0 keeps its own; the generated LODs are read
    without theirs (render data, no CPU copy of the weights), so they get LOD 0's, transferred from its closest
    surface point (`weighted`: LOD 0 with the skeleton's bones, whose order is the mesh's, checked by make_mesh)."""
    dynamic = read_lod(source, lod)
    if has_weights(dynamic):
        return dynamic, "poids du LOD"
    options = unreal.GeometryScriptTransferBoneWeightsOptions()
    weights.transfer_bone_weights_from_mesh(weighted, dynamic, options)
    # The weights index the source's bones, in the mesh's order: the bone attributes themselves are not written
    # (as for LOD 0, whose copy has none).
    weights.discard_bones_from_mesh(dynamic)
    if not has_weights(dynamic):
        raise RuntimeError("LOD %d of %s: no skin weights after the transfer" % (lod, source.get_path_name()))
    return dynamic, "poids du LOD 0 transferes (point de surface le plus proche)"


def face_uv(vertex, normal):
    """A planar UV in the plane of the vertex's face (cm / 50): the material samples no texture, but the build
    recomputes the tangents (MikkTSpace), which want UVs that are not degenerate."""
    hint = (0.0, 0.0, 1.0) if abs(normal[2]) < 0.9 else (1.0, 0.0, 0.0)
    u = anim_rig.normalize(anim_rig.cross(hint, normal))
    v = anim_rig.cross(normal, u)
    return anim_rig.dot(vertex, u) / 50.0, anim_rig.dot(vertex, v) / 50.0


def append_props(dynamic, parts, bone_index):
    """Appends every part (peasant_props.part_buffers) to the body: vertices, normals, colours (alpha = mask), planar
    UVs for every UV set of the body (AppendBuffersToMesh sets the UV set count to the buffers'), material 0, and skin
    weight 1 on the part's bone. Returns the number of vertices and triangles added."""
    unreal.GeometryScript_MeshRepair.compact_mesh(dynamic)
    uv_sets = queries.get_num_uv_sets(dynamic)
    added_vertices = added_triangles = 0
    for part in parts:
        base = queries.get_vertex_count(dynamic)
        if queries.get_num_vertex_i_ds(dynamic) != base:
            raise RuntimeError("Mesh not compact before part " + part["key"])
        buffers = unreal.GeometryScriptSimpleMeshBuffers()
        buffers.set_editor_property("vertices", [unreal.Vector(*v) for v in part["vertices"]])
        buffers.set_editor_property("normals", [unreal.Vector(*n) for n in part["normals"]])
        alpha = part["mask"] / 255.0
        buffers.set_editor_property("vertex_colors", [unreal.LinearColor(c[0] / 255.0, c[1] / 255.0, c[2] / 255.0, alpha)
                                                      for c in part["colors"]])
        planar = [unreal.Vector2D(*face_uv(v, n)) for v, n in zip(part["vertices"], part["normals"])]
        for uv in range(uv_sets):
            buffers.set_editor_property("uv%d" % uv, planar)
        # Reversed: see check_winding (Unreal's triangles are wound the other way round from peasant_props').
        buffers.set_editor_property("triangles", [unreal.IntVector(t[0], t[2], t[1]) for t in part["triangles"]])
        result = unreal.GeometryScript_MeshEdits.append_buffers_to_mesh(dynamic, buffers, 0)
        index_list = first(result, unreal.GeometryScriptIndexList)
        new_triangles = unreal.GeometryScript_List.get_index_list_length(index_list) if index_list is not None else -1
        if new_triangles != len(part["triangles"]):
            raise RuntimeError("Part %s: %d triangles added of %d" % (part["key"], new_triangles, len(part["triangles"])))
        count = len(part["vertices"])
        if queries.get_vertex_count(dynamic) != base + count:
            raise RuntimeError("Part %s: vertex ids not appended in order" % part["key"])
        bone_weight = unreal.GeometryScriptBoneWeight()
        bone_weight.set_editor_property("bone_index", bone_index[part["bone"]])
        bone_weight.set_editor_property("weight", 1.0)
        for vid in range(base, base + count):
            weights.set_vertex_bone_weights(dynamic, vid, [bone_weight])
        added_vertices += count
        added_triangles += new_triangles
    return added_vertices, added_triangles


def check_winding(dynamic, triangles=4000):
    """Share of the body's triangles whose (b - a) x (c - a), computed here from their corners, points against their
    vertex normals. Unreal's meshes are wound that way round (left-handed: (b - a) x (c - a) points into the solid),
    the opposite of peasant_props' triangles, which append_props therefore reverses (b and c swapped), their normals
    kept pointing out. Measured on SKM_Mannequin_Classic (2026-10-16): every sampled triangle; props written unreversed
    were drawn inside out (far faces seen from inside, lit from behind). Not get_triangle_face_normal, which already
    follows Unreal's convention."""
    count = queries.get_num_triangle_i_ds(dynamic)
    against = total = 0
    for tid in range(0, count, max(1, count // triangles)):
        corners = values(queries.get_triangle_positions(dynamic, tid), unreal.Vector)
        normals = values(queries.get_triangle_normals(dynamic, tid), unreal.Vector)
        if len(corners) < 3 or not normals:
            continue
        a, b, c = [(v.x, v.y, v.z) for v in corners[:3]]
        cross = anim_rig.cross(anim_rig.sub(b, a), anim_rig.sub(c, a))
        if anim_rig.length(cross) < 1e-9:
            continue
        total += 1
        if sum(anim_rig.dot(cross, (n.x, n.y, n.z)) for n in normals) < 0.0:
            against += 1
    return against, total


def write_lod(dynamic, mesh, lod):
    options = unreal.GeometryScriptCopyMeshToAssetOptions()
    options.set_editor_property("enable_recompute_normals", False)
    # The props have no tangents of their own; the material has no normal map: the build recomputes them all.
    options.set_editor_property("enable_recompute_tangents", True)
    options.set_editor_property("enable_remove_degenerates", False)
    target = unreal.GeometryScriptMeshWriteLOD()
    target.set_editor_property("lod_index", lod)
    result = geo.copy_mesh_to_skeletal_mesh(dynamic, mesh, options, target)
    if first(result, unreal.GeometryScriptOutcomePins) != unreal.GeometryScriptOutcomePins.SUCCESS:
        raise RuntimeError("Could not write LOD %d of %s" % (lod, mesh.get_path_name()))


def verify_lod(mesh, lod, bone_index, parts):
    """Reads the written LOD back: vertices per mask, bone of the prop vertices."""
    dynamic = read_lod(mesh, lod)
    names = {index: name for name, index in bone_index.items()}
    expected = {part["mask"]: (len(part["vertices"]), part["bone"]) for part in parts}
    found = {}
    count = queries.get_vertex_count(dynamic)
    colors = first(unreal.GeometryScript_VertexColors.get_mesh_per_vertex_colors(dynamic), unreal.GeometryScriptColorList)
    color_list = []
    if colors is not None:
        color_list = list(unreal.GeometryScript_List.convert_color_list_to_array(colors))
    for vid in range(count):
        if vid >= len(color_list):
            break
        mask = int(round(color_list[vid].a * 255.0))
        if mask == 0:
            continue
        weight = first(weights.get_largest_vertex_bone_weight(dynamic, vid), unreal.GeometryScriptBoneWeight)
        bone = names.get(weight.get_editor_property("bone_index")) if weight is not None else None
        entry = found.setdefault(mask, [0, set()])
        entry[0] += 1
        entry[1].add(bone)
    lines = []
    ok = True
    for mask, (vertices, bone) in sorted(expected.items()):
        got = found.get(mask, [0, set()])
        good = got[0] == vertices and got[1] == {bone}
        ok = ok and good
        lines.append("masque %d : %d/%d sommets sur %s%s" % (mask, got[0], vertices, sorted(b or "?" for b in got[1]),
                                                             "" if good else " ECART"))
    extra = sorted(set(found) - set(expected))
    if extra:
        ok = False
        lines.append("masques inattendus %s" % extra)
    log("relecture LOD %d (%d sommets) : %s" % (lod, count, " ; ".join(lines)))
    return ok


def make_mesh(material):
    source = load(SOURCE_MESH)
    path = MESH_FOLDER + "/" + MESH_NAME
    mesh = existing(path) or assets.duplicate_asset(SOURCE_MESH, path)
    if mesh is None:
        raise RuntimeError("Could not duplicate " + SOURCE_MESH + " to " + path)
    names = bone_names(source)
    bone_index = {name: index for index, name in enumerate(names)}
    sk = anim_rig.Skeleton.load(SKELETON_JSON)
    if [n for n in names if n in sk.index] != names or len(names) != len(sk.names):
        raise RuntimeError("The mesh's bones differ from Data/UE4MannequinSkeleton.json")
    parts = [peasant_props.part_buffers(part, sk) for part in peasant_props.mesh_parts(sk)]
    for part in parts:
        if part["bone"] not in bone_index:
            raise RuntimeError("Bone %s of part %s not in the mesh" % (part["bone"], part["key"]))
    log("pieces : %s" % ", ".join("%s (masque %d, %s, %d sommets, %d triangles)"
                                 % (p["key"], p["mask"], p["bone"], len(p["vertices"]), len(p["triangles"]))
                                 for p in parts))

    subsystem = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
    lod_count = unreal.SkeletalMeshEditorSubsystem.get_lod_count(source)
    source_vertices = [subsystem.get_num_verts(source, lod) for lod in range(lod_count)]
    weighted = read_lod(source, 0)
    weights.copy_bones_from_skeleton(source.get_editor_property("skeleton"), weighted)
    for lod in range(lod_count):
        dynamic, how = body_lod(source, lod, source.get_editor_property("skeleton"), weighted)
        body_vertices, body_triangles = queries.get_vertex_count(dynamic), queries.get_num_triangle_i_ds(dynamic)
        if lod == 0:
            against, total = check_winding(dynamic)
            log("sens des triangles du corps : %d/%d avec (b - a) x (c - a) contre la normale (pieces inversees en "
                "consequence)" % (against, total))
            if against < 0.9 * total:
                raise RuntimeError("The body's winding is not the expected one: do not reverse the props' triangles")
        # The body is always shown: mask 0 (alpha 0) on every body vertex.
        flags = unreal.GeometryScriptColorFlags()
        unreal.GeometryScript_VertexColors.set_mesh_constant_vertex_color(dynamic, unreal.LinearColor(0.0, 0.0, 0.0, 0.0),
                                                                          flags, True)
        vertices, triangles = append_props(dynamic, parts, bone_index)
        write_lod(dynamic, mesh, lod)
        log("LOD %d : corps %d sommets %d triangles (%s), pieces +%d sommets +%d triangles"
            % (lod, body_vertices, body_triangles, how, vertices, triangles))

    # Every slot takes the props material; LOD settings of the source, Optimize for Instancing on every LOD.
    slots = []
    for source_slot in mesh.get_editor_property("materials"):
        slot = unreal.SkeletalMaterial()
        slot.set_editor_property("material_slot_name", source_slot.get_editor_property("material_slot_name"))
        slot.set_editor_property("material_interface", material)
        slots.append(slot)
    mesh.set_editor_property("materials", slots)
    for lod in range(unreal.SkeletalMeshEditorSubsystem.get_lod_count(mesh)):
        build_settings = unreal.SkeletalMeshEditorSubsystem.get_lod_build_settings(mesh, lod)
        build_settings.set_editor_property("optimize_for_instancing", True)
        unreal.SkeletalMeshEditorSubsystem.set_lod_build_settings(mesh, lod, build_settings)
    assets.save_loaded_asset(mesh)

    peasant_vertices = [subsystem.get_num_verts(mesh, lod) for lod in range(lod_count)]
    log("%s : sommets par LOD %s (source %s)" % (path, peasant_vertices, source_vertices))
    ok = all([verify_lod(mesh, lod, bone_index, parts) for lod in range(lod_count)])
    log("relecture des pieces : %s" % ("OK" if ok else "ECART"))
    return mesh


# ---------------------------------------------------------------------------------------------------------------------
# Provider


def make_provider(mesh):
    source = load(SOURCE_PROVIDER)
    sequences = [entry.get_editor_property("sequence") for entry in source.get_editor_property("sequences")]
    sequences += [load(path) for path in LOADED_IDLES]
    path = MESH_FOLDER + "/" + PROVIDER_NAME
    provider = existing(path)
    if provider is None:
        factory = unreal.TransformProviderDataFactory()
        factory.set_editor_property("provider_data_class", unreal.AnimSequenceTransformProviderData)
        provider = asset_tools.create_asset(PROVIDER_NAME, MESH_FOLDER, unreal.AnimSequenceTransformProviderData, factory)
    if provider is None:
        raise RuntimeError("Could not create " + path)
    provider.set_editor_property("skinned_asset", mesh)
    entries = []
    for sequence in sequences:
        entry = unreal.AnimSequenceTransformProviderSequence()
        entry.set_editor_property("sequence", sequence)
        entries.append(entry)
    provider.set_editor_property("sequences", entries)
    provider.set_editor_property("layers", [unreal.AnimSequenceTransformProviderLayer()])
    assets.save_loaded_asset(provider)
    log("%s : %d sequences sur %s (%s)" % (path, len(entries), mesh.get_name(),
                                         ", ".join("%d %s" % (i, s.get_name()) for i, s in enumerate(sequences))))
    return provider


def main():
    arguments = sys.argv[1:]
    if "--material-only" in arguments:
        make_material()
        return
    if "--provider-only" in arguments:
        make_provider(load(MESH_FOLDER + "/" + MESH_NAME))
        return
    material = make_material()
    mesh = make_mesh(material)
    make_provider(mesh)
    log("termine")


main()
