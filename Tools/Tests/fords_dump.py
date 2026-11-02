"""The points the paths test of Les Gues needs (Tools/Tests/fords_paths.ps1), from the recipe (Tools/Editor/maps/fords.py), without the
editor: a stand-in `unreal` module is enough, the layout is plain geometry. Prints one JSON object for a size key: the slots (cm, the
world frame of the map file), for each ford and bridge the two points ACROSS cm each side of it across the water, the middle, one pair of points
across the water far from every gap (the discriminating question: the straight line crosses the water, the way round is by a gap), and the
slots of the neighbours of slot 0.
  python Tools/Tests/fords_dump.py <Size>
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

from maps import common, fords  # noqa: E402

ACROSS = 2200.0  # cm: the points are this far from a gap's axis, each side (the water is 18 m across: 4 m of ground each side of it)
WALL_SIDE = 1500.0  # cm: the water's points are this far from its axis, each side


def main(size):
    sizes = common.read_sizes()
    places, side = sizes[size]
    recipe = common.sized_recipe("Fords", size, places, side)
    _, gaps = fords.plan(recipe)
    slots = [recipe.slot(index)[0] for index in range(places)]
    angles = [math.degrees(math.atan2(y, x)) % 360.0 for x, y in slots]
    order = sorted(range(places), key=lambda i: (angles[i] - angles[0]) % 360.0)
    neighbours = [order[1], order[-1]] if places > 2 else [order[1]]
    passes = []
    for number, (is_bridge, x, y, yaw, width) in enumerate(gaps):
        dx, dy = math.cos(math.radians(yaw)), math.sin(math.radians(yaw))
        passes.append({"label": "%s%d" % ("bridge" if is_bridge else "ford", number), "ax": x - dx * ACROSS, "ay": y - dy * ACROSS,
                       "bx": x + dx * ACROSS, "by": y + dy * ACROSS})
    # Water far from every gap: a river's piece halfway between the bridge and a ford, a ring's one halfway between slot 0's ford and the next bridge.
    if places == 2:
        mid = 0.5 * fords.RIVER_FORD_AT * recipe.bounds_half
        walls = [{"label": "River", "ax": -WALL_SIDE, "ay": mid, "bx": WALL_SIDE, "by": mid}]
    else:
        spacing = 360.0 / places
        angle = math.radians(angles[0] + (fords.FORD_OFFSET + 1.0) / 2.0 * spacing)
        radius = fords.ring_radius(recipe)
        dx, dy = math.cos(angle), math.sin(angle)
        walls = [{"label": "Ring", "ax": (radius - WALL_SIDE) * dx, "ay": (radius - WALL_SIDE) * dy, "bx": (radius + WALL_SIDE) * dx, "by": (radius + WALL_SIDE) * dy}]
    print(json.dumps({"size": size, "places": places, "half": recipe.bounds_half, "slots": [{"x": x, "y": y} for x, y in slots],
                      "neighbours": neighbours, "passes": passes, "walls": walls}))


if __name__ == "__main__":
    main(sys.argv[1])
