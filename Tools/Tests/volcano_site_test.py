"""V4b (VolcanoInProgress.md, D607, D573): the volcano site of a map is fair, without the editor (stand-in `unreal`, maps_stub.py).
For every size of Tools/Data/MapSizes.csv, the Steppe's site must pass common.volcano_unfair() AND each unfair variant made from it must be caught
(a flow shortened, a flow bent, a flow doubled in place of another, an altar moved, a flow out of the bounds, a site without a flow).
  python Tools/Tests/volcano_site_test.py [Size ...]     prints one line per size and PASS / FAIL; exit code 0 / 1
"""
import copy
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import maps_stub  # noqa: E402

maps_stub.install()
from maps import common, steppe  # noqa: E402


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


def main(keys):
    sizes = common.read_sizes()
    failed = 0
    for key in keys or list(sizes):
        places, side = sizes[key]
        recipe = common.sized_recipe("Steppe", key, places, side)
        sites = steppe.volcano(recipe)
        problems = common.volcano_unfair(recipe, sites)
        ok = not problems
        caught = 0
        for label, mutant in mutants(sites[0]):
            found = common.volcano_unfair(recipe, [mutant])
            if found:
                caught += 1
            else:
                ok = False
                print("  %s: NOT CAUGHT: %s" % (key, label))
        flows = sites[0]["flows"]
        print("%s: %d places, %d flows of %.0f m, altar %s, %s, %d/7 unfair variants caught%s" % (
            key, places, len(flows), common.math.dist(flows[0][0], flows[0][-1]) / 100, tuple(round(v) for v in sites[0]["altar"]),
            "FAIR" if not problems else "UNFAIR " + "; ".join(problems), caught, "" if ok else "  <-- FAIL"))
        failed += 0 if ok else 1
    print("volcano_site_test : %s" % ("PASS" if not failed else "FAIL (%d sizes)" % failed))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))