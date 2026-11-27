r"""Editor step of Tools/Art/apply_art_send.ps1 (AR8-a, D929): imports the fixed model of an artist's send and writes its DA_Art.

The plan is a JSON file named by the environment variable HOL_ART_SEND_PLAN (written by apply_art_send.ps1 after its refusals):
  model (absolute path of the .glb / .gltf, already copied under ArtSource), family, type, faction ("" = all), folder
  (/Game/Art/<Family>/<Type>), mesh (<folder>/SM_<Type>[_<Faction>]), art (<folder>/DA_Art_<Type>[_<Faction>]), yaw, scale,
  status (Final | Placeholder).
1. The model is imported (Interchange, automated, replace) into the folder; its static mesh is named SM_<Type>[_<Faction>].
2. DA_Art (created when missing: TypeName, Faction): Mesh = the static mesh, bOverrideMeshYawOffset + MeshYawOffset = yaw,
   bOverrideMeshScale + MeshScale = scale, Status. Saved, reloaded, and the reloaded values are logged.
Logs "apply_art_send: DONE ..." on success (read by apply_art_send.ps1), "apply_art_send: ERROR ..." otherwise. Run hidden:
  UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file> -unattended -nullrhi -nosound -Multiprocess
"""
import json
import os

import unreal


def log(text):
    unreal.log("apply_art_send: " + text)


def static_meshes(task, folder):
    """The static meshes made by the import (the task's objects, else the folder's static meshes)."""
    found = []
    paths = []
    try:
        paths = list(task.get_editor_property("imported_object_paths"))
    except Exception:
        paths = []
    for path in paths:
        asset = unreal.load_asset(str(path))
        if isinstance(asset, unreal.StaticMesh) and asset not in found:
            found.append(asset)
    if not found:
        for path in unreal.EditorAssetLibrary.list_assets(folder, recursive=False, include_folder=False):
            asset = unreal.load_asset(path)
            if isinstance(asset, unreal.StaticMesh):
                found.append(asset)
    return found


def object_path(package_path):
    return "%s.%s" % (package_path, package_path.rsplit("/", 1)[1])


def import_mesh(plan):
    folder, mesh_path = plan["folder"], plan["mesh"]
    mesh_name = mesh_path.rsplit("/", 1)[1]
    before = set(str(p) for p in unreal.EditorAssetLibrary.list_assets(folder, recursive=False, include_folder=False))
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", plan["model"])
    task.set_editor_property("destination_path", folder)
    task.set_editor_property("destination_name", mesh_name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    meshes = static_meshes(task, folder)
    named = [m for m in meshes if m.get_name() == mesh_name]
    if named:
        mesh = named[0]
    else:
        new = [m for m in meshes if object_path(m.get_path_name().split(".")[0]) not in before
               and m.get_path_name().split(".")[0] not in before]
        if len(new) != 1:
            raise RuntimeError("%d new static mesh(es) after the import of %s (one expected): %s"
                               % (len(new), plan["model"], [m.get_path_name() for m in meshes]))
        mesh = new[0]
        old = mesh.get_path_name().split(".")[0]
        if not unreal.EditorAssetLibrary.rename_asset(old, mesh_path):
            raise RuntimeError("could not rename %s to %s" % (old, mesh_path))
        mesh = unreal.load_asset(mesh_path)
    if mesh is None or not unreal.EditorAssetLibrary.save_loaded_asset(mesh, False):
        raise RuntimeError("could not save " + mesh_path)
    log("imported %s -> %s (%d material slot(s))" % (plan["model"], mesh.get_path_name(), len(mesh.get_editor_property("static_materials"))))
    return mesh


def load_or_create_art(plan):
    path = plan["art"]
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        asset = unreal.load_asset(path)
        if not isinstance(asset, unreal.HOLArtVisual):
            raise RuntimeError(path + " exists but is not a HOLArtVisual")
        return asset, False
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.HOLArtVisual)
    folder, name = path.rsplit("/", 1)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, folder, unreal.HOLArtVisual, factory)
    if asset is None:
        raise RuntimeError("could not create " + path)
    asset.set_editor_property("type_name", unreal.Name(plan["type"]))
    asset.set_editor_property("faction", unreal.Name(plan["faction"] or "None"))
    return asset, True


def write_art(plan, mesh):
    asset, created = load_or_create_art(plan)
    asset.set_editor_property("mesh", mesh)
    asset.set_editor_property("override_mesh_yaw_offset", True)
    asset.set_editor_property("mesh_yaw_offset", float(plan["yaw"]))
    asset.set_editor_property("override_mesh_scale", True)
    asset.set_editor_property("mesh_scale", float(plan["scale"]))
    status = unreal.HOLArtStatus.FINAL if plan["status"] == "Final" else unreal.HOLArtStatus.PLACEHOLDER
    asset.set_editor_property("status", status)
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, False):
        raise RuntimeError("could not save " + plan["art"])
    return created


def main():
    plan_file = os.environ.get("HOL_ART_SEND_PLAN", "")
    if not plan_file or not os.path.isfile(plan_file):
        raise RuntimeError("no plan (HOL_ART_SEND_PLAN=%r)" % plan_file)
    with open(plan_file, "r", encoding="utf-8") as handle:
        plan = json.load(handle)
    mesh = import_mesh(plan)
    created = write_art(plan, mesh)
    # Read back from the saved asset: what the game will load.
    art = unreal.load_asset(plan["art"])
    got = art.get_editor_property("mesh")
    got_path = got.get_path_name() if isinstance(got, unreal.Object) else str(got)
    if got_path.split(".")[0] != plan["mesh"]:
        raise RuntimeError("%s.Mesh reads back %s, not %s" % (plan["art"], got_path, plan["mesh"]))
    log("DONE art=%s (%s) mesh=%s yaw=%s(override %s) scale=%s(override %s) status=%s type=%s faction=%s" % (
        plan["art"], "created" if created else "updated", got_path,
        art.get_editor_property("mesh_yaw_offset"), art.get_editor_property("override_mesh_yaw_offset"),
        art.get_editor_property("mesh_scale"), art.get_editor_property("override_mesh_scale"),
        art.get_editor_property("status"), art.get_editor_property("type_name"), art.get_editor_property("faction")))


try:
    main()
except Exception as error:  # the .ps1 reads the log: no DONE line = failure
    unreal.log_error("apply_art_send: ERROR %s" % error)
