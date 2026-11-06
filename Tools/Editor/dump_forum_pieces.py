"""Writes Saved/ForumPieces.json: the triangles of every imported kit piece (make_forum_mesh.py), in Unreal's space and cm,
with each triangle's material slot, for the offline preview (preview_forum.py: no Unreal, numpy and Pillow only).

Run (no window):
  UnrealEditor-Cmd.exe <project> -run=pythonscript -script="<this file>" -unattended -nullrhi -nosplash -nosound
"""
import json
import os
import sys

import unreal

TOOLS_DIR = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()), "Tools", "Editor")
sys.dont_write_bytecode = True  # no __pycache__ in Tools/Editor
sys.path.insert(0, TOOLS_DIR)
import forum_layout as pieces  # noqa: E402

geo = unreal.GeometryScript_AssetUtils
queries = unreal.GeometryScript_MeshQueries


def values(result, kind):
    return [v for v in (result if isinstance(result, tuple) else (result,)) if isinstance(v, kind)]


def read(mesh):
    dynamic = unreal.DynamicMesh()
    requested = unreal.GeometryScriptMeshReadLOD()
    requested.set_editor_property("lod_type", unreal.GeometryScriptLODType.SOURCE_MODEL)
    requested.set_editor_property("lod_index", 0)
    result = geo.copy_mesh_from_static_mesh(mesh, dynamic, unreal.GeometryScriptCopyMeshFromAssetOptions(), requested)
    outcome = values(result, unreal.GeometryScriptOutcomePins)
    if outcome and outcome[0] != unreal.GeometryScriptOutcomePins.SUCCESS:
        raise RuntimeError("Could not read " + mesh.get_path_name())
    return dynamic


def slot_of(dynamic, tid):
    try:
        result = unreal.GeometryScript_Materials.get_triangle_material_id(dynamic, tid)
        ints = [v for v in (result if isinstance(result, tuple) else (result,)) if isinstance(v, int) and not isinstance(v, bool)]
        return ints[0] if ints else 0
    except Exception:  # noqa: BLE001 (binding differs: every triangle in slot 0)
        return 0


out = {}
for name in pieces.PIECES:
    mesh = unreal.load_asset("%s/SM_Kit_%s" % (pieces.FOLDER, name.replace("-", "_")))
    if not mesh:
        raise RuntimeError("Missing piece " + name)
    dynamic = read(mesh)
    triangles = []
    for tid in range(queries.get_num_triangle_i_ds(dynamic)):
        corners = values(queries.get_triangle_positions(dynamic, tid), unreal.Vector)
        if len(corners) < 3:
            continue
        triangles.append([[round(c.x, 2), round(c.y, 2), round(c.z, 2)] for c in corners[:3]] + [slot_of(dynamic, tid)])
    out[name] = {"tris": triangles, "slots": [str(m.material_slot_name) for m in mesh.static_materials]}
    unreal.log("Forum dump: %-28s %4d triangles, slots %s" % (name, len(triangles), out[name]["slots"]))

path = os.path.join(unreal.Paths.project_saved_dir(), "ForumPieces.json")
with open(path, "w") as handle:
    json.dump(out, handle)
unreal.log("Forum dump: %d pieces written to %s" % (len(out), path))
