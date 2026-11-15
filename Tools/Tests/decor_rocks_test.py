"""Rocks of the maps' scenery (GDD D802, rocks_compare2 tour 2: the user liked A = faceted rocks alone, C = mostly faceted (6 of 8), D = half and half):
common.ROCK_STYLE (one line, a key of common.ROCK_STYLES) gives the share of FACETED rocks (engine cubes) among the map's boulders and stones, the rest stay
ROUND (spheres). Run on the REAL chain, without the editor: common.spawn_decor on a two-slot recipe, with a recording instanced-mesh component, for each
style of ROCK_STYLES; the share is read from the instances spawn_decor really laid (its density draw, the clumps, fill_holes), never from the constant.
  python Tools/Tests/decor_rocks_test.py        prints one line per check and PASS / FAIL; exit code 0 / 1
"""
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import maps_stub  # noqa: E402

maps_stub.install()
from maps import common  # noqa: E402

FAILS = []


def check(ok, text):
    print(("OK   " if ok else "FAIL ") + text)
    if not ok:
        FAILS.append(text)


class Recipe:
    """Two slots facing each other on a 240 m square (the Plain's size class)."""
    angles = [0.0, 180.0]
    bounds_half = 12000.0
    layout_seed = 7

    def slot(self, index):
        a = math.radians(self.angles[index])
        radial, left = (math.cos(a), math.sin(a)), (-math.sin(a), math.cos(a))
        return (8000.0 * radial[0], 8000.0 * radial[1]), radial, left


class Component:
    """A recording HierarchicalInstancedStaticMeshComponent: its name and how many instances it got."""
    made = {}

    def __init__(self):
        self.name = None

    def rename(self, name):
        self.name = name

    def add_instances(self, transforms, world_space):
        Component.made[self.name] = Component.made.get(self.name, 0) + len(transforms)

    def __getattr__(self, name):
        return maps_stub.Any()


def run(style):
    common.ROCK_STYLE = style
    Component.made = {}
    common.add_component = lambda actor, cls: Component()
    common.spawn = lambda *args, **kwargs: maps_stub.Any()
    common.matte_instance = lambda name, color: (name, color)
    common.spawn_decor(Recipe(), {"Boulder": 3.0, "Stone": 25.0}, [], clumps={"Cairn": 0.6})
    return dict(Component.made)


def main():
    styles = getattr(common, "ROCK_STYLES", None)
    check(isinstance(styles, dict) and sorted(styles) == ["A", "C", "D"], "ROCK_STYLES holds the three styles the user kept (A, C, D): %r" % (styles,))
    check(getattr(common, "ROCK_STYLE", None) in (styles or {}), "ROCK_STYLE (the setting) is one of them: %r" % (getattr(common, "ROCK_STYLE", None),))
    if FAILS:
        return 1
    default = common.ROCK_STYLE
    for kind in ("FacetBoulder", "FacetStone"):
        entry = common.DECOR_KINDS.get(kind)
        check(entry is not None and "Cube" in entry[0], "DECOR_KINDS has %s, an engine cube: %r" % (kind, entry and entry[0]))
    shares = {}
    for style in sorted(styles):
        made = run(style)
        for big, facet in (("Boulder", "FacetBoulder"), ("Stone", "FacetStone")):
            round_n, facet_n = made.get("Decor_" + big, 0), made.get("Decor_" + facet, 0)
            total = round_n + facet_n
            share = facet_n / total if total else -1.0
            shares[(style, big)] = share
            check(total >= 20 and abs(share - styles[style]) <= 0.05,
                  "style %s: %d of %d %s laid faceted (share %.3f, setting %.2f)" % (style, facet_n, total, big, share, styles[style]))
    # The setting really drives the map: a different style gives a different share (not one fixed split).
    check(shares[("A", "Boulder")] > shares[("D", "Boulder")], "style A lays more faceted boulders than style D")
    common.ROCK_STYLE = default
    print("decor_rocks_test: %s" % ("PASS" if not FAILS else "FAIL (%d)" % len(FAILS)))
    return 0 if not FAILS else 1


if __name__ == "__main__":
    sys.exit(main())
