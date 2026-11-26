"""Creates /Game/Units/UnitTypes, the unit type table, from Tools/Data/UnitTypes.csv (D221, D227).

The CSV is the readable source: the table is imported from it, never typed in the editor. Re-running the
script replaces every row, so the CSV stays the single place to balance the types.
AR0-d2 (D925): the CSV carries no visual column any more (mesh, animation, props, projectile, smoke): the art assets
DA_Art_<Type> under /Game/Art are their only source. A visual column back in the CSV is refused, not imported.
Run (the editor quits when done):
  UnrealEditor.exe <project> -ExecutePythonScript=<this file>
"""
import os

import unreal

CSV_PATH = os.path.join(unreal.Paths.project_dir(), "Tools", "Data", "UnitTypes.csv")
TARGET_PATH = "/Game/Units/UnitTypes"
ROW_STRUCT_PATH = "/Script/HeroesOfLiberty.HOLUnitTypeRow"
# AR0-d2: the visual columns removed from the CSV and from FHOLUnitTypeRow (the art assets hold them).
VISUAL_COLUMNS = {
    "Mesh", "MeshYawOffset", "MeshScale", "SkinnedMesh", "AnimationProvider", "IdleSequence", "WalkSequence",
    "WalkPlayRate", "GatherSequences", "GatherPlayRate", "CarryWalkSequences", "CarryWalkPlayRate",
    "CarryIdleSequences", "GatherProps", "CarryProps", "BuildSequence", "BuildPlayRate", "BuildProp",
    "AttackSequence", "AttackProp", "FallSequence", "Projectile", "MuzzleSmoke",
}


def main():
    if not os.path.isfile(CSV_PATH):
        raise RuntimeError("Missing " + CSV_PATH)

    row_struct = unreal.load_object(None, ROW_STRUCT_PATH)
    if row_struct is None:
        raise RuntimeError("Missing " + ROW_STRUCT_PATH + " (is the HeroesOfLiberty module compiled?)")

    # An existing table is refilled in place: deleting it can leave the loaded object behind, and the creation that
    # follows then fails. Filling from the CSV replaces every row, so the CSV stays the only source.
    table = None
    if unreal.EditorAssetLibrary.does_asset_exist(TARGET_PATH):
        table = unreal.load_asset(TARGET_PATH)
        if table is None or table.get_editor_property("row_struct") != row_struct:
            raise RuntimeError(TARGET_PATH + " exists with another row struct: delete it first")
    else:
        factory = unreal.DataTableFactory()
        factory.set_editor_property("struct", row_struct)
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        table = asset_tools.create_asset("UnitTypes", "/Game/Units", unreal.DataTable, factory)
        if table is None:
            raise RuntimeError("Could not create " + TARGET_PATH)

    with open(CSV_PATH, "r", encoding="utf-8") as csv_file:
        csv_text = csv_file.read()
    header = set(csv_text.splitlines()[0].split(",")) if csv_text else set()
    visual = sorted(header & VISUAL_COLUMNS)
    if visual:
        raise RuntimeError(CSV_PATH + ": visual columns belong to the art assets (DA_Art_<Type>): " + ", ".join(visual))
    if not unreal.DataTableFunctionLibrary.fill_data_table_from_csv_string(table, csv_text):
        raise RuntimeError("Could not import " + CSV_PATH + " into " + TARGET_PATH)

    unreal.EditorAssetLibrary.save_loaded_asset(table)
    rows = [str(row_name) for row_name in table.get_row_names()]
    unreal.log("Unit type table created at %s (row struct %s, rows: %s)"
               % (TARGET_PATH, row_struct.get_name(), ", ".join(sorted(rows))))


main()
