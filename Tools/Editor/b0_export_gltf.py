"""B0 (D928) feasibility trial, NOT a tool: exports to Saved/B0/ the source files the packaged game will import at run time
(hol.Art.RuntimeLoad): the Forum's model (DA_Art_MainCentre.Mesh, a building with its materials), the Spearman's static
model (DA_Art_Spearman.Mesh, to put visibly on the Forum), the Arquebusier's body with its skeleton (DA_Art_Arquebusier.SkinnedMesh)
and its attack animation (AttackAnimation), as .glb through the engine's GLTFExporter. One line "B0 export ..." per file.
Run by the hidden editor: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script=<this file> -nullrhi -unattended -Multiprocess.
"""
import os

import unreal

OUT = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_saved_dir()), "B0")
os.makedirs(OUT, exist_ok=True)


def field(asset_path, name):
    art = unreal.load_asset(asset_path)
    if art is None:
        unreal.log_warning("B0 export: no asset " + asset_path)
        return None
    value = art.get_editor_property(name)
    if value is None:
        return None
    if isinstance(value, unreal.Object):
        return value
    try:
        return unreal.load_asset(str(value.get_path_name() if hasattr(value, "get_path_name") else value))
    except Exception:
        return None


def export(obj, file_name):
    if obj is None:
        unreal.log_warning("B0 export: nothing for " + file_name)
        return
    path = os.path.join(OUT, file_name)
    options = unreal.GLTFExportOptions()
    result = unreal.GLTFExporter.export_to_gltf(obj, path, options, set())
    ok = result[0] if isinstance(result, tuple) else result
    size = os.path.getsize(path) if os.path.exists(path) else -1
    unreal.log("B0 export {} {} -> {} ok={} bytes={}".format(obj.get_class().get_name(), obj.get_path_name(), path, ok, size))


export(field("/Game/Art/Buildings/MainCentre/DA_Art_MainCentre", "mesh"), "forum.glb")
export(field("/Game/Art/Units/Spearman/DA_Art_Spearman", "mesh"), "spearman_static.glb")
export(field("/Game/Art/Units/Arquebusier/DA_Art_Arquebusier", "skinned_mesh"), "arquebusier_skinned.glb")
export(field("/Game/Art/Units/Arquebusier/DA_Art_Arquebusier", "attack_animation"), "arquebusier_attack.glb")
unreal.log("B0 export done " + OUT)
