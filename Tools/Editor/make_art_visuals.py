"""Creates the missing art asset of a new type (D925, AR0-d2 (4): the art assets are the only source of the visual).

For every type named in Tools/Data/UnitTypes.csv, BuildingTypes.csv and SourceTypes.csv (the Name column only: the
CSV carries no visual column any more), the asset /Game/Art/<Units|Buildings|Sources>/<Type>/DA_Art_<Type>
(UHOLArtVisual) is created EMPTY when it does not exist: the type's name, no faction, no visual field (the neutral
fallback: the provisional shape), Status Placeholder. An existing asset is never rewritten (the artists' work stays); its manifest Status is read in it (Final kept).
Tools/Data/ArtVisuals.csv, the art manifest (Type,Faction,Family,Asset,Fields,Status; check_placeholders.ps1), keeps
its lines and gains one line per created asset.
Run hidden (the editor quits when done):
  UnrealEditor-Cmd.exe <project> -ExecutePythonScript=<this file> -unattended -nullrhi -nosplash -Multiprocess
"""
import csv
import os

import unreal

DATA_DIR = os.path.join(unreal.Paths.project_dir(), "Tools", "Data")
MANIFEST_PATH = os.path.join(DATA_DIR, "ArtVisuals.csv")
MANIFEST_HEADER = ["Type", "Faction", "Family", "Asset", "Fields", "Status"]
ART_ROOT = "/Game/Art"
FAMILIES = [
    ("Units", "UnitTypes.csv"),
    ("Buildings", "BuildingTypes.csv"),
    ("Sources", "SourceTypes.csv"),
]


def type_names(file_name):
    """The type names of a table CSV (its Name column, nothing else)."""
    with open(os.path.join(DATA_DIR, file_name), "r", encoding="utf-8", newline="") as csv_file:
        return [row["Name"].strip() for row in csv.DictReader(csv_file) if row.get("Name", "").strip()]


def read_manifest():
    if not os.path.isfile(MANIFEST_PATH):
        return []
    with open(MANIFEST_PATH, "r", encoding="utf-8", newline="") as csv_file:
        rows = list(csv.reader(csv_file))
    return [row for row in rows[1:] if row]


def create_empty(family, type_name):
    """The empty DA_Art of a new type; None when it already exists (left as it is)."""
    folder = "%s/%s/%s" % (ART_ROOT, family, type_name)
    name = "DA_Art_" + type_name
    path = folder + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return None
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.HOLArtVisual)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, folder, unreal.HOLArtVisual, factory)
    if asset is None:
        raise RuntimeError("Could not create " + path)
    asset.set_editor_property("type_name", unreal.Name(type_name))
    asset.set_editor_property("faction", unreal.Name("None"))
    asset.set_editor_property("status", unreal.HOLArtStatus.PLACEHOLDER)
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, False):
        raise RuntimeError("Could not save " + path)
    return path


def asset_status(path):
    """The manifest Status of a DA_Art, read in the asset: Final when its Status is Final, else Provisoire."""
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is not None and asset.get_editor_property("status") == unreal.HOLArtStatus.FINAL:
        return "Final"
    return "Provisoire"


def main():
    manifest = read_manifest()
    listed = {(row[0], row[1], row[2]) for row in manifest if len(row) >= 3}
    created = 0
    for family, file_name in FAMILIES:
        for type_name in type_names(file_name):
            path = create_empty(family, type_name)
            if path:
                created += 1
            asset_path = "%s/%s/%s/DA_Art_%s" % (ART_ROOT, family, type_name, type_name)
            status = asset_status(asset_path)
            if (type_name, "", family) not in listed:
                manifest.append([type_name, "", family, asset_path, "", status])
                listed.add((type_name, "", family))
            else:
                # The asset is the truth: an artist's Final is kept, never set back to Provisoire.
                for row in manifest:
                    if len(row) >= 6 and (row[0], row[1], row[2]) == (type_name, "", family):
                        row[5] = status
    with open(MANIFEST_PATH, "w", encoding="utf-8", newline="") as csv_file:
        writer = csv.writer(csv_file, lineterminator="\n")
        writer.writerow(MANIFEST_HEADER)
        writer.writerows(manifest)
    unreal.log("make_art_visuals: %d empty art assets created under %s, manifest %s (%d lines)"
               % (created, ART_ROOT, MANIFEST_PATH, len(manifest)))


try:
    main()
finally:
    unreal.SystemLibrary.quit_editor()
