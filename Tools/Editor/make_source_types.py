"""Creates /Game/Resources/SourceTypes, the resource source type table, from Tools/Data/SourceTypes.csv (D297, like the
building types of D260).

The CSV is the readable source: the table is imported from it, never typed in the editor. Re-running the
script replaces every row, so the CSV stays the single place to balance the types.
Run (the editor quits when done):
  UnrealEditor.exe <project> -ExecutePythonScript=<this file>
"""
import os

import unreal

CSV_PATH = os.path.join(unreal.Paths.project_dir(), "Tools", "Data", "SourceTypes.csv")
TARGET_PATH = "/Game/Resources/SourceTypes"
ROW_STRUCT_PATH = "/Script/HeroesOfLiberty.HOLSourceTypeRow"


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
        table = asset_tools.create_asset("SourceTypes", "/Game/Resources", unreal.DataTable, factory)
        if table is None:
            raise RuntimeError("Could not create " + TARGET_PATH)

    with open(CSV_PATH, "r", encoding="utf-8") as csv_file:
        csv_text = csv_file.read()
    if not unreal.DataTableFunctionLibrary.fill_data_table_from_csv_string(table, csv_text):
        raise RuntimeError("Could not import " + CSV_PATH + " into " + TARGET_PATH)

    unreal.EditorAssetLibrary.save_loaded_asset(table)
    rows = [str(row_name) for row_name in table.get_row_names()]
    unreal.log("Source type table created at %s (row struct %s, rows: %s)"
               % (TARGET_PATH, row_struct.get_name(), ", ".join(sorted(rows))))


main()
