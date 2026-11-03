"""The points the paths test of La Couronne needs (Tools/Tests/crown_paths.ps1), from the recipe (Tools/Editor/maps/crown.py), without the editor
(stand-in `unreal`, maps_stub.py). Prints one JSON object for a size key: the slots (cm, world frame), for place 0 the two points 16 m each side of its ramp
ALONG the ray to the middle (inside and outside the ring), the two points 15 m each side of the ring on the far side from the ramp, the neighbour slots.
  python Tools/Tests/crown_dump.py <Size>
"""
import json
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import maps_stub  # noqa: E402

maps_stub.install()
from maps import common, crown  # noqa: E402

ACROSS = 1600.0  # cm: the ramp's pair of points, each side of the ring's line
BACK = 1500.0  # cm: the back wall's pair of points, each side of the ring's line


def main(size):
    places, side = common.read_sizes()[size]
    recipe = common.sized_recipe("Crown", size, places, side)
    slots = [recipe.slot(index)[0] for index in range(places)]
    sx, sy = slots[0]
    toward = math.atan2(-sy, -sx)
    ux, uy = math.cos(toward), math.sin(toward)
    angles = [math.degrees(math.atan2(y, x)) % 360.0 for x, y in slots]
    order = sorted(range(places), key=lambda i: (angles[i] - angles[0]) % 360.0)
    neighbours = [order[1], order[-1]] if places > 2 else [order[1]]
    ring = crown.RING_RADIUS
    ramp = {"ax": sx + ux * (ring - ACROSS), "ay": sy + uy * (ring - ACROSS), "bx": sx + ux * (ring + ACROSS), "by": sy + uy * (ring + ACROSS)}
    back = {"ax": sx - ux * (ring - BACK), "ay": sy - uy * (ring - BACK), "bx": sx - ux * (ring + BACK), "by": sy - uy * (ring + BACK)}
    print(json.dumps({"size": size, "places": places, "slots": [{"x": x, "y": y} for x, y in slots], "neighbours": neighbours, "ramp": ramp, "back": back}))


if __name__ == "__main__":
    main(sys.argv[1])
