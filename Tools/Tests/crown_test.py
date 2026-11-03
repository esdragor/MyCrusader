"""C6 (La Couronne, D596 = CQ6b = A): every base is a keep behind a ring of cliffs with ONE ramp, without the editor (stand-in `unreal`, maps_stub.py).
For every size of Tools/Data/MapSizes.csv, from the recipe's rocks (Tools/Editor/maps/crown.py) rasterised at 50 cm round each place:
  - one ramp marker per place, on the ray from the place to the middle, RING_RADIUS from it;
  - the ramp is free of rock (a formation of 12 m passes: nothing within 6 m of its middle);
  - the ring is CLOSED but at the ramp: a flood fill from the place cannot leave the ring with the ramp plugged (and can with it open);
  - the rings of two places do not touch, and none goes more than 10 m beyond the bounds (on Small the map's edge closes a ring that does not fit).
  python Tools/Tests/crown_test.py [Size ...]     prints one line per size and PASS / FAIL; exit code 0 / 1
"""
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import maps_stub  # noqa: E402

maps_stub.install()
from maps import common, crown  # noqa: E402

CELL = 50.0
FREE_RADIUS = 600.0
PLUG_RADIUS = 1800.0  # a ramp is plugged by a disc this wide (the ring is cut at both its ends)


def footprints(rocks, centre, reach):
    """The rocks near centre as (x, y, a, b, cos, sin)."""
    out = []
    for x, y, sx, sy, _, yaw in rocks:
        a, b = 50.0 * sx, 50.0 * sy
        if math.dist((x, y), centre) <= reach + max(a, b):
            out.append((x, y, a, b, math.cos(math.radians(yaw)), math.sin(math.radians(yaw))))
    return out


def blocked(point, shapes):
    for x, y, a, b, cos, sin in shapes:
        dx, dy = point[0] - x, point[1] - y
        if (dx * cos + dy * sin) ** 2 / (a * a) + (-dx * sin + dy * cos) ** 2 / (b * b) <= 1.0:
            return True
    return False


def raster(shapes, origin):
    """The set of cells (i, j) of CELL cm, from origin, whose centre is inside a rock."""
    cells = set()
    for x, y, a, b, cos, sin in shapes:
        reach = max(a, b)
        for i in range(int((x - reach - origin[0]) / CELL), int((x + reach - origin[0]) / CELL) + 1):
            for j in range(int((y - reach - origin[1]) / CELL), int((y + reach - origin[1]) / CELL) + 1):
                dx, dy = origin[0] + (i + 0.5) * CELL - x, origin[1] + (j + 0.5) * CELL - y
                if (dx * cos + dy * sin) ** 2 / (a * a) + (-dx * sin + dy * cos) ** 2 / (b * b) <= 1.0:
                    cells.add((i, j))
    return cells


def escapes(centre, reach, shapes, plug, half):
    """Whether a flood fill from the centre of a ring reaches a cell beyond the ring's reach (plug: (x, y, radius) or None)."""
    cells = int(2 * reach / CELL) + 1
    origin = (centre[0] - reach, centre[1] - reach)
    rock = raster(shapes, origin)

    def at(i, j):
        return (origin[0] + (i + 0.5) * CELL, origin[1] + (j + 0.5) * CELL)

    def wall(i, j):
        point = at(i, j)
        if plug is not None and math.dist(point, plug[:2]) <= plug[2]:
            return True  # the ramp is closed by a disc of rock
        return abs(point[0]) > half or abs(point[1]) > half or (i, j) in rock  # the edge of the map closes the ring too (Small: the ring is cut by it)

    start = (cells // 2, cells // 2)
    seen = {start}
    stack = [start]
    while stack:
        i, j = stack.pop()
        if math.dist(at(i, j), centre) > reach - CELL:
            return True
        for di, dj in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            n = (i + di, j + dj)
            if 0 <= n[0] < cells and 0 <= n[1] < cells and n not in seen and not wall(*n):
                seen.add(n)
                stack.append(n)
    return False


def main(keys):
    sizes = common.read_sizes()
    failed = 0
    for key in keys or list(sizes):
        places, side = sizes[key]
        recipe = common.sized_recipe("Crown", key, places, side)
        rocks, volumes, markers = crown.plan(recipe)
        problems = []
        if len(markers) != places:
            problems.append("%d ramp markers for %d places" % (len(markers), places))
        reach = crown.RING_RADIUS + 1500.0
        closed = 0
        for index, (label, gx, gy) in enumerate(markers):
            slot = recipe.slot(index)[0]
            toward = math.atan2(-slot[1], -slot[0])
            want = (slot[0] + crown.RING_RADIUS * math.cos(toward), slot[1] + crown.RING_RADIUS * math.sin(toward))
            if math.dist((gx, gy), want) > 50.0:
                problems.append("ramp %d is not on the ray to the middle" % index)
            shapes = footprints(rocks, slot, reach)
            if blocked((gx, gy), shapes) or any(math.dist((x, y), (gx, gy)) < FREE_RADIUS - max(a, b) for x, y, a, b, _, _ in shapes):
                problems.append("ramp %d is not free of rock" % index)
            if not escapes(slot, reach, shapes, None, recipe.bounds_half):
                problems.append("ring %d has no way out with the ramp open" % index)
            if escapes(slot, reach, shapes, (gx, gy, PLUG_RADIUS), recipe.bounds_half):
                problems.append("ring %d leaks besides its ramp" % index)
            else:
                closed += 1
            if any(abs(x) > recipe.bounds_half + 1000.0 or abs(y) > recipe.bounds_half + 1000.0 for x, y, *_ in shapes):
                problems.append("ring %d goes more than 10 m beyond the bounds" % index)
        slots = [recipe.slot(index)[0] for index in range(places)]
        nearest = min(math.dist(a, b) for i, a in enumerate(slots) for b in slots[i + 1:])
        if nearest <= 2 * (crown.RING_RADIUS + 1000.0):
            problems.append("two rings touch (places %.0f m apart)" % (nearest / 100))
        print("%s: %d places, %d rocks, %d rings closed but at the ramp, rings %.0f m apart%s" % (
            key, places, len(rocks), closed, nearest / 100, "" if not problems else "  <-- FAIL: " + "; ".join(problems)))
        failed += 1 if problems else 0
    print("crown_test : %s" % ("PASS" if not failed else "FAIL (%d sizes)" % failed))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
