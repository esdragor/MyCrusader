"""Imports the pieces of Kenney's Retro Fantasy Kit (CC0) that the Forum is made of (VisualInProgress.md, V2a), under
/Game/Buildings/Forum/Kit, and prints every piece's size in cm so the composition (V2b) can be laid out from measures.
With "--compose" it instead builds /Game/Buildings/Forum/SM_Forum: the pieces of forum_layout.LAYOUT merged into ONE static
mesh (the building table takes one), at the building's visible size (forum_layout.SCALE), origin at the centre of the
footprint on the ground, every material slot named after the kit's material; no team colour yet (V3).

The kit lives outside the repository (Docs/assets_art.csv): C:\\Users\\baudo\\Documents\\MyCrusader_Art\\Kenney\\retro-fantasy-kit.
A piece is /Game/Buildings/Forum/Kit/SM_Kit_<name with _ for ->. Run (no window):
  UnrealEditor-Cmd.exe <project> -run=pythonscript -script="<this file>" -unattended -nullrhi -nosplash -nosound
"""
import os
import sys

import unreal

TOOLS_DIR = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()), "Tools", "Editor")
sys.dont_write_bytecode = True  # no __pycache__ in Tools/Editor
sys.path.insert(0, TOOLS_DIR)
from forum_layout import FOLDER, KIT, PIECES  # noqa: E402

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
COMPOSE = "--compose" in sys.argv[1:]


def import_task(name):
    path = os.path.join(KIT, name + ".fbx")
    if not os.path.isfile(path):
        raise RuntimeError("Missing " + path)
    task = unreal.AssetImportTask()
    task.filename = path
    task.destination_path = FOLDER
    task.destination_name = "SM_Kit_" + name.replace("-", "_")
    task.automated = True
    task.replace_existing = True
    task.save = True
    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_as_skeletal = False
    options.import_materials = True
    options.import_textures = True
    options.import_animations = False
    options.static_mesh_import_data.combine_meshes = True
    options.static_mesh_import_data.generate_lightmap_u_vs = False
    task.options = options
    return task


if not COMPOSE:
    tasks = [import_task(n) for n in PIECES]
    asset_tools.import_asset_tasks(tasks)

    lines = []
    for n in PIECES:
        path = "%s/SM_Kit_%s" % (FOLDER, n.replace("-", "_"))
        mesh = unreal.load_asset(path)
        if not mesh:
            lines.append("Forum kit: %s NOT IMPORTED" % n)
            continue
        b = mesh.get_bounds()
        e = b.box_extent
        o = b.origin
        lines.append("Forum kit: %-28s size cm X=%7.1f Y=%7.1f Z=%7.1f  centre X=%7.1f Y=%7.1f Z=%7.1f  slots=%d"
                     % (n, 2 * e.x, 2 * e.y, 2 * e.z, o.x, o.y, o.z, len(mesh.static_materials)))
    for line in lines:
        unreal.log(line)
    unreal.log("Forum kit: %d pieces asked, %d imported" % (len(PIECES), sum(1 for l in lines if "NOT IMPORTED" not in l)))


def compose():
    import forum_layout as layout

    geo = unreal.GeometryScript_AssetUtils
    materials = unreal.GeometryScript_Materials
    queries = unreal.GeometryScript_MeshQueries
    edits = unreal.GeometryScript_MeshEdits

    def values(result, kind):
        return [v for v in (result if isinstance(result, tuple) else (result,)) if isinstance(v, kind)]

    def ints(result):
        return [v for v in (result if isinstance(result, tuple) else (result,)) if isinstance(v, int) and not isinstance(v, bool)]

    variant = next((a.split("=", 1)[1].upper() for a in sys.argv[1:] if a.startswith("--variant=")), None)
    items = layout.VARIANTS[variant]() if variant else layout.LAYOUT

    slots = []  # global material slots: [name, material interface]
    pieces = {}  # name -> (dynamic mesh with global material ids)
    def slot_index(slot_name, interface):
        for i, (existing, _) in enumerate(slots):
            if existing == slot_name:
                return i
        slots.append((slot_name, interface))
        return len(slots) - 1

    for name in sorted({p[0] for p in items}):
        if name.startswith("@"):
            continue  # built below
        mesh = unreal.load_asset("%s/SM_Kit_%s" % (FOLDER, name.replace("-", "_")))
        if not mesh:
            raise RuntimeError("Missing piece " + name)
        dynamic = unreal.DynamicMesh()
        requested = unreal.GeometryScriptMeshReadLOD()
        requested.set_editor_property("lod_type", unreal.GeometryScriptLODType.SOURCE_MODEL)
        requested.set_editor_property("lod_index", 0)
        result = geo.copy_mesh_from_static_mesh(mesh, dynamic, unreal.GeometryScriptCopyMeshFromAssetOptions(), requested)
        outcome = values(result, unreal.GeometryScriptOutcomePins)
        if outcome and outcome[0] != unreal.GeometryScriptOutcomePins.SUCCESS:
            raise RuntimeError("Could not read " + name)
        mapping = []
        for static_material in mesh.static_materials:
            slot_name = str(static_material.material_slot_name)
            for i, (existing, _) in enumerate(slots):
                if existing == slot_name:
                    mapping.append(i)
                    break
            else:
                slots.append((slot_name, static_material.material_interface))
                mapping.append(len(slots) - 1)
        for tid in range(queries.get_num_triangle_i_ds(dynamic)):
            current = ints(materials.get_triangle_material_id(dynamic, tid))
            old = current[0] if current else 0
            materials.set_triangle_material_id(dynamic, tid, mapping[old if old < len(mapping) else 0])
        pieces[name] = dynamic

    if any(p[0] == "@pyramid" for p in items):
        # The pyramid roof (forum_layout.pyramid_unit): flat faces, tiled UVs, the kit's roof material.
        roof_mesh = unreal.load_asset("%s/SM_Kit_roof" % FOLDER)
        roof_slot = slot_index(str(roof_mesh.static_materials[0].material_slot_name), roof_mesh.static_materials[0].material_interface)
        vertices, normals, uvs, triangles = layout.pyramid_unit()
        buffers = unreal.GeometryScriptSimpleMeshBuffers()
        buffers.set_editor_property("vertices", [unreal.Vector(*v) for v in vertices])
        buffers.set_editor_property("normals", [unreal.Vector(*n) for n in normals])
        buffers.set_editor_property("uv0", [unreal.Vector2D(*uv) for uv in uvs])
        buffers.set_editor_property("triangles", [unreal.IntVector(*t) for t in triangles])
        pyramid = unreal.DynamicMesh()
        edits.append_buffers_to_mesh(pyramid, buffers, roof_slot)
        pieces["@pyramid"] = pyramid

    target = unreal.DynamicMesh()
    step = layout.MODULE * layout.SCALE
    placements = {}
    for name, x, y, z, yaw, scale in items:
        placements.setdefault(name, []).append(unreal.Transform(
            unreal.Vector(x * step, y * step, z * step), unreal.Rotator(roll=0.0, pitch=0.0, yaw=yaw),
            unreal.Vector(layout.SCALE * scale[0], layout.SCALE * scale[1], layout.SCALE * scale[2])))
    for name, transforms in placements.items():
        try:
            edits.append_mesh_transformed(target, pieces[name], transforms, unreal.Transform())
        except Exception:  # noqa: BLE001 (binding signature: show it, then fail)
            unreal.log("Forum compose: append_mesh_transformed doc: %s" % edits.append_mesh_transformed.__doc__)
            raise

    path = "/Game/Buildings/Forum/SM_Forum"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.EditorAssetLibrary.delete_asset(path)
    options = unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
    options.enable_collision = False
    static_mesh, outcome = unreal.GeometryScript_NewAssetUtils.create_new_static_mesh_asset_from_mesh(target, path, options)
    if outcome != unreal.GeometryScriptOutcomePins.SUCCESS:
        raise RuntimeError("Could not create " + path)
    static_materials = []
    for slot_name, interface in slots:
        sm = unreal.StaticMaterial()
        sm.material_interface = interface
        sm.material_slot_name = slot_name
        static_materials.append(sm)
    static_mesh.set_editor_property("static_materials", static_materials)
    body = static_mesh.get_editor_property("body_setup")
    if body:
        body.set_editor_property("collision_trace_flag", unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)  # the selection trace (D262)
    unreal.EditorAssetLibrary.save_loaded_asset(static_mesh)
    bounds = static_mesh.get_bounding_box()
    unreal.log("Forum compose: %s, %d pieces placed, %d triangles, %d slots %s, bounds min %s max %s"
               % (path + (" variant " + variant if variant else ""), len(items), static_mesh.get_num_triangles(0), len(slots), [s[0] for s in slots], bounds.min, bounds.max))


if COMPOSE:
    compose()
