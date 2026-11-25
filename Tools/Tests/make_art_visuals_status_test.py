"""make_art_visuals.py keeps the Status of each existing DA_Art (read in the asset) in Tools/Data/ArtVisuals.csv.

An artist who sets a DA_Art to Final must not see the manifest go back to Provisoire the next time the script runs (a
new type added to a table). Runs WITHOUT the editor: a fake `unreal` module stands in for the asset library, the script
runs on a temporary project (its Tools/Data CSVs), and the written manifest is read back.
  python -I Tools/Tests/make_art_visuals_status_test.py      (exit 0 = PASS)
"""
import csv
import os
import runpy
import sys
import tempfile
import types

SCRIPT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "Editor", "make_art_visuals.py")
HEADER = ["Type", "Faction", "Family", "Asset", "Fields", "Status"]


class FakeAsset:
    def __init__(self, status):
        self.props = {"status": status}

    def get_editor_property(self, name):
        return self.props[name]

    def set_editor_property(self, name, value):
        self.props[name] = value


def fake_unreal(project_dir, assets):
    """assets: DA_Art path -> FakeAsset (the existing ones); created ones are added."""
    unreal = types.ModuleType("unreal")
    unreal.HOLArtStatus = types.SimpleNamespace(PLACEHOLDER="Placeholder", FINAL="Final")
    unreal.Paths = types.SimpleNamespace(project_dir=lambda: project_dir)
    unreal.Name = lambda value: value
    unreal.HOLArtVisual = object

    class DataAssetFactory:
        def set_editor_property(self, name, value):
            pass

    class AssetTools:
        def create_asset(self, name, folder, cls, factory):
            asset = FakeAsset(unreal.HOLArtStatus.PLACEHOLDER)
            assets[folder + "/" + name] = asset
            return asset

    unreal.DataAssetFactory = DataAssetFactory
    unreal.AssetToolsHelpers = types.SimpleNamespace(get_asset_tools=lambda: AssetTools())
    unreal.EditorAssetLibrary = types.SimpleNamespace(
        does_asset_exist=lambda path: path in assets,
        load_asset=lambda path: assets.get(path),
        save_loaded_asset=lambda asset, only_dirty: True,
    )
    unreal.SystemLibrary = types.SimpleNamespace(quit_editor=lambda: None)
    unreal.log = lambda text: None
    return unreal


def write_csv(path, rows):
    with open(path, "w", encoding="utf-8", newline="") as csv_file:
        csv.writer(csv_file, lineterminator="\n").writerows(rows)


def main():
    problems = []
    with tempfile.TemporaryDirectory() as project_dir:
        data = os.path.join(project_dir, "Tools", "Data")
        os.makedirs(data)
        write_csv(os.path.join(data, "UnitTypes.csv"), [["Name"], ["Archer"]])
        write_csv(os.path.join(data, "BuildingTypes.csv"), [["Name"], ["House"], ["Barn"], ["NewTower"]])
        write_csv(os.path.join(data, "SourceTypes.csv"), [["Name"]])
        house = "/Game/Art/Buildings/House/DA_Art_House"
        barn = "/Game/Art/Buildings/Barn/DA_Art_Barn"
        archer = "/Game/Art/Units/Archer/DA_Art_Archer"
        # House: an artist set it Final, its manifest line still says Provisoire. Barn: Final, no manifest line yet.
        # Archer: still a placeholder.
        write_csv(os.path.join(data, "ArtVisuals.csv"), [
            HEADER,
            ["House", "", "Buildings", house, "Mesh", "Provisoire"],
            ["Archer", "", "Units", archer, "", "Provisoire"],
        ])
        assets = {house: FakeAsset("Final"), barn: FakeAsset("Final"), archer: FakeAsset("Placeholder")}
        sys.modules["unreal"] = fake_unreal(project_dir, assets)
        runpy.run_path(SCRIPT, run_name="make_art_visuals")
        with open(os.path.join(data, "ArtVisuals.csv"), "r", encoding="utf-8", newline="") as csv_file:
            rows = {row["Type"]: row for row in csv.DictReader(csv_file)}
        expected = {"House": "Final", "Barn": "Final", "Archer": "Provisoire", "NewTower": "Provisoire"}
        for type_name, status in expected.items():
            got = rows.get(type_name, {}).get("Status")
            if got != status:
                problems.append("%s: Status %r, expected %r (read in its DA_Art)" % (type_name, got, status))
        if rows.get("House", {}).get("Fields") != "Mesh":
            problems.append("House: its Fields %r lost" % rows.get("House", {}).get("Fields"))
        if "/Game/Art/Buildings/NewTower/DA_Art_NewTower" not in assets:
            problems.append("NewTower: its empty DA_Art was not created")
    for problem in problems:
        print("FAIL " + problem)
    print("make_art_visuals_status_test : %s (%d problems)" % ("PASS" if not problems else "FAIL", len(problems)))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
