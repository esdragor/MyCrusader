"""V4b (VolcanoInProgress.md, D607, D573): the volcano site of a map is fair, without the editor (stand-in `unreal`, maps_stub.py).
For every map of Tools/Data/Maps.csv and every size of Tools/Data/MapSizes.csv, the map's module must GIVE a site (volcano(recipe)), it must pass
common.volcano_unfair() AND each unfair variant made from it must be caught (a flow shortened, a flow bent, a flow doubled in place of another, an
altar moved, a flow out of the bounds, a site without a flow). Also printed, not failing (the lava is inert on the terrain, D379): what the flows and
the altar cross of the map's own terrain (the water of the Fords, the ranges of the Cols) and whether the altar's zone meets a relay's.
  python Tools/Tests/volcano_site_test.py [Map[:Size] ...]     prints one line per map and size and PASS / FAIL; exit code 0 / 1
"""
import importlib
import copy
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import maps_stub  # noqa: E402

maps_stub.install()
from maps import common  # noqa: E402


def mutants(site):
    """(label, site) for each way a site can be unfair; each one must be caught."""
    out = []
    short = copy.deepcopy(site)
    short["flows"][0] = [(x * 0.7, y * 0.7) for x, y in short["flows"][0]]
    out.append(("one flow 30 % shorter", short))
    bent = copy.deepcopy(site)
    x, y = bent["flows"][0][1]
    bent["flows"][0][1] = (x + 300.0, y + 500.0)
    out.append(("one flow bent by 5 m", bent))
    doubled = copy.deepcopy(site)
    doubled["flows"][1] = list(doubled["flows"][0])
    out.append(("one flow aimed twice at the same camp", doubled))
    lonely = copy.deepcopy(site)
    lonely["flows"].append([(0.0, 0.0), (1234.0, 777.0), (2468.0, 1554.0)])
    out.append(("a flow alone in its turn", lonely))
    moved = copy.deepcopy(site)
    x, y, radius = moved["altar"]
    moved["altar"] = (x + 3000.0, y + 1000.0, radius)
    out.append(("the altar 30 m off", moved))
    outside = copy.deepcopy(site)
    outside["flows"] = [[(x * 5.0, y * 5.0) for x, y in flow] for flow in outside["flows"]]
    out.append(("flows out of the bounds", outside))
    bare = copy.deepcopy(site)
    bare["flows"] = []
    out.append(("a site without a flow", bare))
    return out


def notes(name, module, recipe, site):
    """What the site meets of the map's terrain (information)."""
    out = []
    points = []
    for flow in site["flows"]:
        for (ax, ay), (bx, by) in zip(flow, flow[1:]):
            points += [(ax + (bx - ax) * step / 10.0, ay + (by - ay) * step / 10.0) for step in range(11)]
    if hasattr(module, "distance_to_water"):
        out.append("%d/%d flow points within 8 m of the water" % (sum(1 for point in points if module.distance_to_water(recipe, point) < 800.0), len(points)))
    if hasattr(module, "ranges_of") and hasattr(module, "distance_to_range"):
        ranges = module.ranges_of(recipe)
        out.append("%d/%d flow points on a massif" % (sum(1 for point in points if any(module.distance_to_range(point, mountain) < module.MOUNTAIN_HALF for mountain in ranges)), len(points)))
    if hasattr(module, "avoid_points"):
        rocks = module.avoid_points(recipe)
        out.append("%d rocks within 8 m of a flow" % sum(1 for rock in rocks if any(math.dist(rock, point) < 800.0 for point in points)))
    if site["altar"] is not None and hasattr(module, "relays"):
        x, y, radius = site["altar"]
        try:
            met = sum(1 for rx, ry, rr in module.relays(recipe) if math.dist((x, y), (rx, ry)) < radius + rr)
            out.append("altar zone meets %d relay zone(s)" % met)
        except Exception as error:  # a map whose relays need the whole recipe: say so, do not fail
            out.append("relays not read (%s)" % type(error).__name__)
    return out


def altar_on_the_bridge(module, recipe, site):
    """D673 (Q-autel-gues = A): on the Fords with 2 places the altar stands on the middle bridge, in the middle of the map, not in the river.
    None when it does, else the text of what is wrong."""
    _, gaps = module.plan(recipe)
    bridges = [gap for gap in gaps if gap[0]]
    if len(bridges) != 1:
        return "the 2-place Fords should have one bridge, found %d" % len(bridges)
    _, bx, by, _, width = bridges[0]
    x, y, _ = site["altar"]
    if abs(x - bx) > module.WATER_WIDTH / 2.0 or abs(y - by) > width / 2.0:
        return "the altar (%.0f, %.0f) is not on the bridge (%.0f, %.0f, %.0f cm wide): D673" % (x, y, bx, by, width)
    if math.hypot(x, y) > common.VOLCANO_TOLERANCE:
        return "the altar (%.0f, %.0f) is not at the middle of the map: D673" % (x, y)
    return None


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
            if not hasattr(module, "volcano"):
                print("%s %s: the module gives NO volcano site  <-- FAIL" % (name, key))
                failed += 1
                continue
            sites = module.volcano(recipe)
            problems = common.volcano_unfair(recipe, sites)
            ok = not problems and len(sites) == 1
            caught = 0
            for label, mutant in mutants(sites[0]):
                found = common.volcano_unfair(recipe, [mutant])
                if found:
                    caught += 1
                else:
                    ok = False
                    print("  %s %s: NOT CAUGHT: %s" % (name, key, label))
            flows = sites[0]["flows"]
            wrong = altar_on_the_bridge(module, recipe, sites[0]) if name == "Fords" and places == 2 else None
            if wrong:
                ok = False
                problems.append(wrong)
            if len(flows) != places:
                ok = False
                problems.append("%d flows for %d places" % (len(flows), places))
            print("%s %s: %d places, %d flows of %.0f m, altar %s, %s, %d/7 unfair variants caught%s" % (
                name, key, places, len(flows), common.math.dist(flows[0][0], flows[0][-1]) / 100, tuple(round(v) for v in sites[0]["altar"]),
                "FAIR" if not problems else "UNFAIR " + "; ".join(problems), caught, "" if ok else "  <-- FAIL"))
            note = notes(name, module, recipe, sites[0])
            if note:
                print("    " + "; ".join(note))
            failed += 0 if ok else 1
    print("volcano_site_test : %s" % ("PASS" if not failed else "FAIL (%d)" % failed))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))