"""Creates or updates one art asset per type from the CSV rows (D925, AR0-d1: the migration of the art in one go, D924).

For every row of Tools/Data/UnitTypes.csv, BuildingTypes.csv and SourceTypes.csv, the asset
/Game/Art/<Units|Buildings|Sources>/<Type>/DA_Art_<Type> (UHOLArtVisual) gets the type's name and every visual field the
row sets, each numeric one with its bOverride toggle, Status Placeholder (the migrated art is the placeholder art).
Re-running it rewrites the same assets from the same rows (idempotent). Then Tools/Data/ArtVisuals.csv, the art manifest
(Type,Faction,Family,Asset,Fields,Status; check_placeholders.ps1), is rewritten: one line per asset.
Test: HeroesOfLiberty.Art.Visual.MigrationKeepsEveryType (the constants of every type are the CSV's).
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

# CSV column -> (kind, toggle): "mesh" = a soft object pointer (the loaded asset), "path" = a soft object path,
# "int" / "float" = a number set with its bOverride toggle.
UNIT_FIELDS = [
    ("Mesh", "mesh", None),
    ("MeshYawOffset", "float", "bOverrideMeshYawOffset"),
    ("MeshScale", "float", "bOverrideMeshScale"),
    ("SkinnedMesh", "mesh", None),
    ("AnimationProvider", "mesh", None),
    ("IdleSequence", "int", "bOverrideIdleSequence"),
    ("WalkSequence", "int", "bOverrideWalkSequence"),
    ("WalkPlayRate", "float", "bOverrideWalkPlayRate"),
    ("GatherPlayRate", "float", "bOverrideGatherPlayRate"),
    ("CarryWalkPlayRate", "float", "bOverrideCarryWalkPlayRate"),
    ("BuildSequence", "int", "bOverrideBuildSequence"),
    ("BuildPlayRate", "float", "bOverrideBuildPlayRate"),
    ("AttackSequence", "int", "bOverrideAttackSequence"),
    ("FallSequence", "int", "bOverrideFallSequence"),
    # AR0-d2: by resource ("(Food=4,Wood=2,Stone=3,Gold=3)") and the props.
    ("GatherSequences", "sequences", "bOverrideGatherSequences"),
    ("CarryWalkSequences", "sequences", "bOverrideCarryWalkSequences"),
    ("CarryIdleSequences", "sequences", "bOverrideCarryIdleSequences"),
    ("GatherProps", "props", "bOverrideGatherProps"),
    ("CarryProps", "props", "bOverrideCarryProps"),
    ("BuildProp", "int", "bOverrideBuildProp"),
    ("AttackProp", "int", "bOverrideAttackProp"),
    ("Projectile", "path", None),
    ("MuzzleSmoke", "path", None),
]
MESH_ONLY = [("Mesh", "mesh", None)]
FAMILIES = [
    ("Units", "UnitTypes.csv", UNIT_FIELDS),
    ("Buildings", "BuildingTypes.csv", MESH_ONLY),
    ("Sources", "SourceTypes.csv", MESH_ONLY),
]


def snake(name):
    """The Python name of a property: bOverrideX -> override_x, MeshScale -> mesh_scale."""
    if name.startswith("b") and len(name) > 1 and name[1].isupper():
        name = name[1:]
    out = ""
    for index, char in enumerate(name):
        if char.isupper() and index > 0:
            out += "_"
        out += char.lower()
    return out


def set_property(asset, name, value):
    try:
        asset.set_editor_property(snake(name), value)
    except Exception:
        asset.set_editor_property(name, value)


def soft_path(text):
    """A soft object path from its text (empty = none); this Python API has no MakeSoftObjectPath."""
    path = unreal.SoftObjectPath()
    if text:
        try:
            path.import_text(text)
        except Exception:
            path = unreal.SoftObjectPath(text)
        if str(path.export_text()).strip('"') != text:
            raise RuntimeError("soft path %s read back as %s" % (text, path.export_text()))
    return path


def resource_struct(kind, text):
    """FHOLResourceSequences / FHOLResourceProps from the CSV's "(Food=4,Wood=2,Stone=3,Gold=3)"."""
    value = unreal.HOLResourceSequences() if kind == "sequences" else unreal.HOLResourceProps()
    for pair in text.strip().strip("()").split(","):
        if pair.strip():
            key, number = pair.split("=")
            value.set_editor_property(key.strip().lower(), int(float(number)))
    return value


def read_rows(file_name):
    with open(os.path.join(DATA_DIR, file_name), "r", encoding="utf-8", newline="") as csv_file:
        return [row for row in csv.DictReader(csv_file) if row.get("Name", "").strip()]


def load_or_create(family, type_name):
    folder = "%s/%s/%s" % (ART_ROOT, family, type_name)
    name = "DA_Art_" + type_name
    path = folder + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        asset = unreal.load_asset(path)
        if asset is None or not isinstance(asset, unreal.HOLArtVisual):
            raise RuntimeError(path + " exists but is not a HOLArtVisual: delete it first")
        return asset, path
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.HOLArtVisual)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, folder, unreal.HOLArtVisual, factory)
    if asset is None:
        raise RuntimeError("Could not create " + path)
    return asset, path


def fill(asset, type_name, row, fields):
    """Sets the type and every visual field the row sets; a field the row leaves empty is reset (toggle off, no asset)."""
    set_property(asset, "TypeName", unreal.Name(type_name))
    set_property(asset, "Faction", unreal.Name("None"))
    written = []
    for column, kind, toggle in fields:
        cell = (row.get(column) or "").strip()
        if kind == "mesh":
            value = None
            if cell:
                value = unreal.load_asset(cell)
                if value is None:
                    raise RuntimeError("%s: %s %s does not load" % (type_name, column, cell))
            set_property(asset, column, value)
        elif kind == "path":
            set_property(asset, column, soft_path(cell))
        elif kind in ("sequences", "props"):
            if cell:
                set_property(asset, column, resource_struct(kind, cell))
            set_property(asset, toggle, bool(cell))
        else:
            if cell:
                set_property(asset, column, int(float(cell)) if kind == "int" else float(cell))
            set_property(asset, toggle, bool(cell))
        if cell:
            written.append(column)
    set_property(asset, "Status", unreal.HOLArtStatus.PLACEHOLDER)
    return written


def main():
    manifest = []
    for family, file_name, fields in FAMILIES:
        for row in read_rows(file_name):
            type_name = row["Name"].strip()
            asset, path = load_or_create(family, type_name)
            written = fill(asset, type_name, row, fields)
            if not unreal.EditorAssetLibrary.save_loaded_asset(asset, False):
                raise RuntimeError("Could not save " + path)
            manifest.append([type_name, "", family, path, ";".join(written), "Provisoire"])
    with open(MANIFEST_PATH, "w", encoding="utf-8", newline="") as csv_file:
        writer = csv.writer(csv_file, lineterminator="\n")
        writer.writerow(MANIFEST_HEADER)
        writer.writerows(manifest)
    unreal.log("make_art_visuals: %d art assets written under %s, manifest %s" % (len(manifest), ART_ROOT, MANIFEST_PATH))


try:
    main()
finally:
    unreal.SystemLibrary.quit_editor()
