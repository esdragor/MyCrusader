"""The points the paths test of Les Cols needs (Tools/Tests/cols_paths.ps1), from the recipe (Tools/Editor/maps/cols.py), without the
editor: a stand-in `unreal` module is enough, the layout is plain geometry. Prints one JSON object for a size key: the slots (cm, the
world frame of the map file), for each col the two points 16 m each side of it ACROSS the range, the middle of the plaza, and the
slots of the two neighbours of slot 0 (the nearest in angle on each side).
  python Tools/Tests/cols_dump.py <Size>
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

from maps import cols, common  # noqa: E402

WALL_SIDE = 2500.0  # cm: the wall's points are this far from a range's axis, each side (the massif is about 30 m across)
ACROSS = 1600.0  # cm: the points are this far from the col's axis, each side (the gap is 16 m wide: the points are 16 m from its middle)


def main(size):
    sizes = common.read_sizes()
    places, side = sizes[size]
    recipe = common.sized_recipe("Cols", size, places, side)
    _, _, markers = cols.plan(recipe)
    slots = [recipe.slot(index)[0] for index in range(places)]
    angles = [math.degrees(math.atan2(y, x)) % 360.0 for x, y in slots]
    order = sorted(range(places), key=lambda i: (angles[i] - angles[0]) % 360.0)
    neighbours = [order[1], order[-1]] if places > 2 else [order[1]]
    passes = []
    for label, x, y in markers:
        length = math.hypot(x, y)
        tx, ty = -y / length, x / length  # across the range (the range lies along the ray from the middle through the marker)
        passes.append({"label": label, "ax": x - tx * ACROSS, "ay": y - ty * ACROSS, "bx": x + tx * ACROSS, "by": y + ty * ACROSS})
    # A point on each side of the wall halfway between the plaza and the first col of every range: the straight line crosses 50 m of
    # mountain, the way round is by the plaza or a col (the discriminating question: a neighbour's base can lie next to a col).
    walls = []
    for mountain in cols.ranges_of(recipe):
        first = [m for m in markers if m[0].startswith("ColGap%d_0" % mountain.index)][0]
        mx, my = (mountain.start[0] + first[1]) / 2.0, (mountain.start[1] + first[2]) / 2.0
        length = math.hypot(mx, my)
        tx, ty = -my / length, mx / length
        walls.append({"label": "Range%d" % mountain.index, "ax": mx - tx * WALL_SIDE, "ay": my - ty * WALL_SIDE,
                      "bx": mx + tx * WALL_SIDE, "by": my + ty * WALL_SIDE})
    result = {
        "size": size, "places": places, "half": recipe.bounds_half, "slots": [{"x": x, "y": y} for x, y in slots],
        "neighbours": neighbours, "passes": passes, "walls": walls,
    }
    print(json.dumps(result))


if __name__ == "__main__":
    main(sys.argv[1])
