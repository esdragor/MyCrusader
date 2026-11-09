"""Sources inside a sacred site's disc (Retours\\Q-sites-arbres-zone.md, the user's answer of 2026-11-09 = B): common.SITES_CLEAR_SOURCES = True makes make_map
leave out of the map's sources every one whose centre is inside a site's disc (as drop_reserved does for the relays, the camps and the altar), before the
layout actor and the painter's file are written. Run on the REAL chain (common.make_map on the Plain map, its own sites(recipe) and sources(recipe), with
the recording `unreal` stand-in of spawn_sites_test.py); the module is wrapped only to add three trees by the sites (one at a site's centre, one just
inside its edge, one just outside it) so that the effect is read even where the real forest does not reach a site.
  python Tools/Tests/sites_clear_test.py        prints one line per check and PASS / FAIL; exit code 0 / 1
"""
import json
import math
import os
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import spawn_sites_test as base  # noqa: E402  (installs the stand-in `unreal` and holds the recorder)


def run_make_map(common, recipe, module, saved):
    """make_map on the module; returns (the sources of the SourceLayout actor, the sources of the painter's file as (type, x, y))."""
    base.Actors.spawned.clear()
    original_wait = common.wait_for_navmesh
    common.wait_for_navmesh = lambda recipe, nav, level, summary, on_saved: None
    try:
        common.make_map(recipe, module, lambda *args: None)
    finally:
        common.wait_for_navmesh = original_wait
    layouts = [a for a in base.Actors.spawned if a.cls == "/Script/HeroesOfLiberty.HOLSourceLayout"]
    laid = layouts[0].props.get("sources", []) if len(layouts) == 1 else None
    with open(os.path.join(saved, "MapLayouts", "Plain_Small.json"), encoding="utf-8") as handle:
        exported = [tuple(row) for row in json.load(handle)["sources"]]
    return laid, exported


def main():
    from maps import common  # noqa: E402
    from maps import plain  # noqa: E402

    failed = []

    def check(name, ok, detail=""):
        print("%s: %s%s" % (name, "ok" if ok else "FAIL", "" if ok or not detail else " (" + detail + ")"))
        if not ok:
            failed.append(name)

    with tempfile.TemporaryDirectory() as saved:
        base.install_recorder(saved)
        places, side = common.read_sizes()["Small"]
        recipe = common.sized_recipe("Plain", "Small", places, side)
        sites = list(plain.sites(recipe))
        check("the Plain map has sites to clear", 2 <= len(sites) <= 3, "%d sites" % len(sites))
        radius = sites[0][2]
        planted = [(sites[0][0], sites[0][1]), (sites[0][0] + radius - 20.0, sites[0][1]), (sites[0][0] + radius + 60.0, sites[0][1])]

        class WithTreesBySites:
            """The Plain module as it is (relays, sources, sites), except the ridges of rocks (they need the editor's vector maths) and three trees planted by a site."""

            def __getattr__(self, name):
                return getattr(plain, name)

            @staticmethod
            def obstacles(recipe):
                return 0

            @staticmethod
            def sources(recipe):
                placed, groups = plain.sources(recipe)
                return list(placed) + [common.placed_source("Tree", x, y) for x, y in planted], groups

        def inside(rows):
            return [row for row in rows if any(math.dist((row[1], row[2]), (x, y)) < r for x, y, r in sites)]

        def as_rows(sources):
            rows = []
            for source in sources:
                at = source.get_editor_property("location")
                rows.append((str(source.get_editor_property("type")), at.x, at.y))
            return rows

        check("common.SITES_CLEAR_SOURCES exists", hasattr(common, "SITES_CLEAR_SOURCES"))

        # The knob off (the old behaviour, answer A): nothing is left out, so the planted trees are still in the disc.
        common.SITES_CLEAR_SOURCES = False
        laid_off, exported_off = run_make_map(common, recipe, WithTreesBySites(), saved)
        rows_off = as_rows(laid_off) if laid_off is not None else []
        check("knob off: the layout actor holds sources", bool(rows_off))
        check("knob off: the planted trees by the site are kept", len(inside(rows_off)) >= 2, "%d in the discs" % len(inside(rows_off)))

        # The knob on (answer B).
        common.SITES_CLEAR_SOURCES = True
        laid_on, exported_on = run_make_map(common, recipe, WithTreesBySites(), saved)
        rows_on = as_rows(laid_on) if laid_on is not None else []
        check("knob on: the layout actor holds sources", bool(rows_on))
        check("knob on: no source of the layout actor is inside a site's disc", not inside(rows_on), "%d still inside" % len(inside(rows_on)))
        check("knob on: no source of the painter's file is inside a site's disc", not inside(exported_on), "%d still inside" % len(inside(exported_on)))
        check("knob on: exactly the sources inside the discs are gone", len(rows_off) - len(rows_on) == len(inside(rows_off)),
              "%d off, %d on, %d inside" % (len(rows_off), len(rows_on), len(inside(rows_off))))
        check("knob on: the tree just outside the disc stays",
              any(row[0] == "Tree" and abs(row[1] - planted[2][0]) < 0.5 and abs(row[2] - planted[2][1]) < 0.5 for row in rows_on))
        check("knob on: the painter's file agrees with the layout actor", len(exported_on) == len(rows_on), "%d against %d" % (len(exported_on), len(rows_on)))

    print("sites_clear_test : %s" % ("PASS" if not failed else "FAIL (%s)" % ", ".join(failed)))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
