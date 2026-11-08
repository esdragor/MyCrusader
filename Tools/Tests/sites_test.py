"""Sacred sites' map data (SacredSitesInProgress.md S4, D697: the number and the places of the sites are set by every map, no formula): the maps' own
sites(recipe) gives [(x, y, radius)] in cm, as relays(recipe) does. Without the editor (stand-in `unreal`, maps_stub.py), on the REAL chain (the maps' own
sites(recipe), relays(recipe) and volcano(recipe)).
For every map of Tools/Data/Maps.csv and every size, the sites must be: 2 to 3 in all (the user's answer, Q-sites-nombre.md; the map's own count,
common.site_count); symmetric by a half turn (every size has an even number of places; a third site is the middle); a radius above 0; inside the bounds; outside every base's reach; clear of
every relay zone, of the altar's zone and of the lava flows; and not overlapping each other.
  python Tools/Tests/sites_test.py [Map[:Size] ...]     prints one line per map and size and PASS / FAIL; exit code 0 / 1
"""
import importlib
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import maps_stub  # noqa: E402

maps_stub.install()
from maps import common  # noqa: E402

TWIN_TOLERANCE = 100.0  # cm, as the relays' symmetry (CountUnmatchedByTurn)
MIN_SITES = 2  # the user's answer in Retours\Q-sites-nombre.md: 2 to 3 sites in all per map
MAX_SITES = 3


def problems_of(recipe, sites, relays, volcano):
    """What is wrong with these sites (a list of texts, empty = fine)."""
    out = []
    places = len(recipe.angles)
    if not sites:
        return ["no site"]
    # The user's answer (Q-sites-nombre, 2026-11-07): 2 to 3 sites IN ALL per map, not per player; the count is the map's own (common.site_count).
    if not MIN_SITES <= len(sites) <= MAX_SITES:
        out.append("%d sites in all: the answer is %d to %d per map" % (len(sites), MIN_SITES, MAX_SITES))
    if len(sites) != common.site_count(recipe):
        out.append("%d sites but the map's count is %d" % (len(sites), common.site_count(recipe)))
    # Every size has an even number of places, so the places themselves are symmetric by a half turn: the sites are too (a third site is the middle).
    if places % 2:
        out.append("%d places: a half turn does not map the places on themselves" % places)
    unmatched = sum(1 for (x, y, r) in sites
                    if not any(math.dist((-x, -y), (u, v)) <= TWIN_TOLERANCE and abs(r - s) < 1.0 for (u, v, s) in sites))
    if unmatched:
        out.append("%d sites without a twin after a half turn" % unmatched)
    slots = [recipe.slot(index)[0] for index in range(places)]
    altar_x, altar_y, altar_radius = volcano["altar"]
    for i, (x, y, r) in enumerate(sites):
        if r <= 0.0:
            out.append("radius %.0f at (%.0f, %.0f)" % (r, x, y))
        if abs(x) > recipe.bounds_half - r or abs(y) > recipe.bounds_half - r:
            out.append("a site is outside the bounds at (%.0f, %.0f)" % (x, y))
        if any(math.dist((x, y), slot) < common.BASE_REACH + r for slot in slots):
            out.append("a site zone enters a base's reach at (%.0f, %.0f)" % (x, y))
        if any(math.dist((x, y), (u, v)) < r + s for (u, v, s) in relays):
            out.append("a site zone meets a relay's at (%.0f, %.0f)" % (x, y))
        if math.dist((x, y), (altar_x, altar_y)) < r + altar_radius:
            out.append("a site zone meets the altar's at (%.0f, %.0f)" % (x, y))
        if any(common.segment_distance((x, y), flow[0], flow[-1]) < common.RELAY_LAVA_HALF_WIDTH + r for flow in volcano["flows"]):
            out.append("a site zone is on a lava flow at (%.0f, %.0f)" % (x, y))
        for (u, v, s) in sites[i + 1:]:
            if math.dist((x, y), (u, v)) < r + s:
                out.append("two site zones overlap at (%.0f, %.0f)" % (x, y))
    return out


def self_test(module, recipe):
    """The checks must bite: a good set passes, then one site moved off its twin, one moved into a base's reach and one onto a relay each fail."""
    relays, volcano, sites = module.relays(recipe), module.volcano(recipe)[0], module.sites(recipe)
    broken = {
        "asymmetric": [(sites[0][0] + 1500.0, sites[0][1], sites[0][2])] + sites[1:],
        "in a base": [(recipe.slot(0)[0][0], recipe.slot(0)[0][1], sites[0][2])] + sites[1:],
        "on a relay": [(relays[0][0], relays[0][1], sites[0][2])] + sites[1:],
    }
    if problems_of(recipe, sites, relays, volcano):
        return ["the sites of the self-test map are not clear"]
    return ["self-test: a set %s is not refused" % name for name, bad in broken.items() if not problems_of(recipe, bad, relays, volcano)]


def main(keys):
    sizes = common.read_sizes()
    maps = common.read_maps()
    if not keys:
        places, side = sizes["Normal"]
        module = importlib.import_module("maps." + maps["Plain"])
        bad = self_test(module, common.sized_recipe("Plain", "Normal", places, side))
        print("self-test: %s" % ("OK" if not bad else "; ".join(bad) + "  <-- FAIL"))
        if bad:
            return 1
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
            try:
                sites = module.sites(recipe)
            except AttributeError as error:
                sites = None
                problem = "no sites(recipe) (%s)" % error
            if sites is not None:
                problem = "; ".join(problems_of(recipe, sites, relays, module.volcano(recipe)[0]))
            print("%s %s: %s%s" % (name, key, ("%d sites" % len(sites) if sites is not None else "") + (", OK" if not problem else ", " + problem),
                                   "" if not problem else "  <-- FAIL"))
            failed += 1 if problem else 0
    print("sites_test : %s" % ("PASS" if not failed else "FAIL (%d)" % failed))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
