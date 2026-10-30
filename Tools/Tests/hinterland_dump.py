"""The points the paths test of a Hinterland needs (Tools/Tests/hinterland_paths.ps1), from the recipe (Tools/Editor/maps/hinterland.py),
without the editor: a stand-in `unreal` module is enough, the layout is plain geometry. Prints one JSON object for a size key:
slots, each pocket's veins and openings, the belt's passages (all in cm, the world frame of the map file), so that the script asks the
game's own navmesh (hol.Nav.Path) about the same places the file was made with.
  python Tools/Tests/hinterland_dump.py <Size>
"""
import json
import math
import os
import sys
import types

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))


class Any:
    """Any attribute or call of the stand-in module."""

    def __init__(self, *args, **kwargs):
        pass

    def __getattr__(self, name):
        return Any()

    def __call__(self, *args, **kwargs):
        return Any()


class Vector:
    def __init__(self, x=0.0, y=0.0, z=0.0):
        self.x, self.y, self.z = x, y, z


fake = types.ModuleType("unreal")
fake.Vector = Vector
fake.LinearColor = lambda *values: values
fake.Paths = types.SimpleNamespace(convert_relative_path_to_full=lambda path: path, project_dir=lambda: ROOT + os.sep)
fake.__getattr__ = lambda name: Any()
sys.modules["unreal"] = fake
sys.path.insert(0, os.path.join(ROOT, "Tools", "Editor"))

from maps import common, hinterland  # noqa: E402


def main(size):
    sizes = common.read_sizes()
    places, side = sizes[size]
    recipe = common.sized_recipe("Hinterland", size, places, side)
    plan = hinterland.plan(recipe)
    slots = [recipe.slot(index)[0] for index in range(places)]
    veins = {index: [] for index in range(places)}
    for kind, x, y, zone in plan.points:
        if zone == "pocket" and kind.startswith(("GoldVein", "StoneVein")):
            index = min(range(places), key=lambda i: math.dist((x, y), slots[i]))
            veins[index].append({"type": kind, "x": x, "y": y})
    result = {
        "size": size, "places": places, "half": recipe.bounds_half, "slots": [{"x": x, "y": y} for x, y in slots],
        "veins": {str(index): items for index, items in veins.items()},
        "openings": [{"slot": index, "x": point[0], "y": point[1]} for index, point in plan.openings],
        "passages": [{"x": point[0], "y": point[1]} for point in plan.passages],
    }
    print(json.dumps(result))


if __name__ == "__main__":
    main(sys.argv[1])
