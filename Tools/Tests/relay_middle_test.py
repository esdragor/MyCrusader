"""D647 (Q-volcan-autel-relais = B): on a map with a volcano the relay of the middle stands at least 24 m from the middle of the map, so that the relay and
the crater's altar are two stakes. Without the editor (stand-in `unreal`, maps_stub.py), on the REAL chain (the maps' own relays(recipe) and volcano(recipe)).
For every map of Tools/Data/Maps.csv and every size: the relays are the places' ring then the 5 fixed ones, the first fixed one is the middle relay; it must
stand RELAY_MIN from the middle, its zone must not meet the altar's zone, the 5 zones must not overlap, nothing may enter a base's reach. Also printed, not
failing: whether the middle relay's zone meets a lava flow (kept off when the map has room), and the pairs' symmetry by a turn of 180 degrees.
  python Tools/Tests/relay_middle_test.py [Map[:Size] ...]     prints one line per map and size and PASS / FAIL; exit code 0 / 1
"""
import importlib
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import maps_stub  # noqa: E402

maps_stub.install()
from maps import common  # noqa: E402

RELAY_MIN = 2400.0  # D647, cm
FIXED = 5


def problems_of(recipe, relays, site):
    """What is wrong with these relays (a list of texts, empty = fine)."""
    out = []
    places = len(recipe.angles)
    if len(relays) != places + FIXED:
        return ["%d relays, %d expected" % (len(relays), places + FIXED)]
    fixed = relays[places:]
    middle = fixed[0]
    if math.hypot(middle[0], middle[1]) < RELAY_MIN - 1.0:
        out.append("middle relay %.0f cm from the middle (24 m wanted)" % math.hypot(middle[0], middle[1]))
    ax, ay, ar = site["altar"]
    for x, y, r in relays:
        if math.dist((x, y), (ax, ay)) < r + ar - 1.0:
            out.append("a relay zone meets the altar's at (%.0f, %.0f)" % (x, y))
    for i, (x, y, r) in enumerate(relays):
        for (u, v, s) in relays[i + 1:]:
            if math.dist((x, y), (u, v)) < r + s:
                out.append("two relay zones overlap at (%.0f, %.0f)" % (x, y))
    slots = [recipe.slot(index)[0] for index in range(places)]
    for x, y, r in relays:
        if any(math.dist((x, y), slot) < common.BASE_REACH + r for slot in slots):
            out.append("a relay zone enters a base's reach at (%.0f, %.0f)" % (x, y))
        if abs(x) > recipe.bounds_half or abs(y) > recipe.bounds_half:
            out.append("a relay is outside the bounds at (%.0f, %.0f)" % (x, y))
    pairs = fixed[1:]
    unmatched = sum(1 for (x, y, _) in pairs if not any(math.dist((x, y), (-u, -v)) <= 100.0 for (u, v, _) in pairs))
    if unmatched:
        out.append("%d fixed relays (not the middle one) without a twin after a turn of 180 degrees" % unmatched)
    return out


def main(keys):
    sizes = common.read_sizes()
    maps = common.read_maps()
    wanted = [key.split(":") for key in keys] or [[name] for name in maps]
    failed = 0
    for want in wanted:
        name = want[0]
        module = importlib.import_module("maps." + maps[name])
        for key in want[1:] or list(sizes):
            places, side = sizes[key]
            recipe = common.sized_recipe(name, key, places, side)
            try:
                relays = module.relays(recipe)
            except Exception as error:  # a recipe that needs the editor for its relays: said, counted as not read
                print("%s %s: relays not read (%s: %s)" % (name, key, type(error).__name__, error))
                continue
            site = module.volcano(recipe)[0]
            problems = problems_of(recipe, relays, site)
            middle = relays[places]
            on_flow = any(common.segment_distance(middle[:2], flow[0], flow[-1]) < common.RELAY_LAVA_HALF_WIDTH + middle[2] for flow in site["flows"])
            print("%s %s: middle relay %.0f m from the middle%s, %s%s" % (
                name, key, math.hypot(middle[0], middle[1]) / 100.0, ", ON A LAVA FLOW" if on_flow else "", "OK" if not problems else "; ".join(problems),
                "" if not problems else "  <-- FAIL"))
            failed += 1 if problems else 0
    print("relay_middle_test : %s" % ("PASS" if not failed else "FAIL (%d)" % failed))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
