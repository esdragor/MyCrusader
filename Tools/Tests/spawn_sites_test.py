"""Sacred sites laid by the map script (SacredSitesInProgress.md S4, D697), without the editor: a RECORDING stand-in `unreal` (not the harmless one of maps_stub.py)
keeps every spawned actor with the properties set on it, so that the test reads the EFFECT on the real chain (common.make_map on the Plain map, its own
sites(recipe)): one AHOLSiteLayout actor holding the map's sites, and the "sites" key of the layout file the map painter reads (Saved/MapLayouts).
  python Tools/Tests/spawn_sites_test.py        prints one line per check and PASS / FAIL; exit code 0 / 1
"""
import json
import math
import os
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import maps_stub  # noqa: E402

maps_stub.install()
import unreal  # noqa: E402  (the stand-in)


class Placed:
    """A struct or an actor of the editor: properties kept by name, read back as set."""

    def __init__(self, label=None, cls=None, location=None):
        self.props, self.label, self.cls, self.at = {}, label, cls, location

    def set_editor_property(self, name, value):
        self.props[name] = value

    def get_editor_property(self, name):
        return self.props.get(name, maps_stub.Any())

    def set_actor_label(self, label):
        self.label = label

    def __getattr__(self, name):  # any other call of the editor (scale, folder, ...) is harmless
        return maps_stub.Any()


class Actors:
    """The editor's actor subsystem: every spawn is recorded."""

    spawned = []

    def spawn_actor_from_class(self, cls, location=None, rotation=None):
        actor = Placed(cls=cls, location=location)
        Actors.spawned.append(actor)
        return actor


class Subsystem(maps_stub.Any):
    """Any other editor subsystem (the level: new_level is true, the navmesh waits are harmless)."""

    def new_level(self, *args, **kwargs):
        return True


def install_recorder(saved_dir):
    unreal.get_editor_subsystem = lambda cls: Actors() if str(cls) == "EditorActorSubsystem" else Subsystem()
    unreal.EditorActorSubsystem = "EditorActorSubsystem"
    unreal.load_class = lambda outer, path: path if path.startswith("/Script/HeroesOfLiberty.HOL") else None
    unreal.Paths = type("Paths", (), {"convert_relative_path_to_full": staticmethod(lambda path: path),
                                      "project_dir": staticmethod(lambda: maps_stub.ROOT + os.sep),
                                      "project_saved_dir": staticmethod(lambda: saved_dir)})
    for name in ("HOLPlacedSite", "HOLPlacedRelay", "HOLPlacedSource", "HOLPlacedVolcano", "HOLPlacedLavaFlow", "HOLPlacedZone"):
        setattr(unreal, name, Placed)


def site_actors():
    return [a for a in Actors.spawned if a.cls == "/Script/HeroesOfLiberty.HOLSiteLayout"]


def main():
    from maps import common  # noqa: E402
    from maps import plain  # noqa: E402

    failed = []

    def check(name, ok, detail=""):
        print("%s: %s%s" % (name, "ok" if ok else "FAIL", "" if ok or not detail else " (" + detail + ")"))
        if not ok:
            failed.append(name)

    with tempfile.TemporaryDirectory() as saved:
        install_recorder(saved)
        sizes = common.read_sizes()
        places, side = sizes["Small"]
        recipe = common.sized_recipe("Plain", "Small", places, side)
        wanted = plain.sites(recipe)
        check("the Plain map has sites to lay", 2 <= len(wanted) <= 3, "%d sites" % len(wanted))

        # 1. spawn_sites: one layout actor, the sites as the map gave them.
        Actors.spawned.clear()
        spawn_sites = getattr(common, "spawn_sites", None)
        check("common.spawn_sites exists", spawn_sites is not None)
        if spawn_sites is not None:
            spawn_sites(wanted)
            actors = site_actors()
            check("spawn_sites spawns one HOLSiteLayout actor", len(actors) == 1, "%d actors" % len(actors))
            if len(actors) == 1:
                placed = actors[0].props.get("sites", [])
                got = [(s.props["location"].x, s.props["location"].y, s.props["radius"]) for s in placed]
                same = len(got) == len(wanted) and all(math.dist(g[:2], w[:2]) < 0.5 and abs(g[2] - w[2]) < 0.5 for g, w in zip(got, wanted))
                check("the layout holds the map's sites (position and radius)", same, "%s against %s" % (got, wanted))

        # 2. export_layout: the "sites" key of the painter's file.
        try:
            common.export_layout(recipe, [], [], wanted)
            with open(os.path.join(saved, "MapLayouts", "Plain_Small.json"), encoding="utf-8") as handle:
                data = json.load(handle)
            check("export_layout writes the sites", data.get("sites") == [list(s) for s in wanted], "%s" % data.get("sites"))
        except TypeError as error:  # the red state: export_layout does not take the sites yet
            check("export_layout writes the sites", False, str(error))
        common.export_layout(recipe, [], [])
        with open(os.path.join(saved, "MapLayouts", "Plain_Small.json"), encoding="utf-8") as handle:
            data = json.load(handle)
        check("export_layout without sites writes an empty list", data.get("sites") == [], "%s" % data.get("sites"))

        # 3. The real chain: make_map lays the layout actor of the map's own sites and says it in its summary.
        Actors.spawned.clear()
        summaries = []
        printed = []
        original_wait = common.wait_for_navmesh
        common.wait_for_navmesh = lambda recipe, nav, level, summary, on_saved: summaries.append(summary)
        class NoRidges:
            """The Plain module as it is (its relays, sources, volcano, sites), except the ridges of rocks, which need the editor's vector maths (a stand-in has none)."""

            def __getattr__(self, name):
                return getattr(plain, name)

            @staticmethod
            def obstacles(recipe):
                return 0

        try:
            common.make_map(recipe, NoRidges(), lambda *args: None)
        finally:
            common.wait_for_navmesh = original_wait
        actors = site_actors()
        check("make_map lays one HOLSiteLayout", len(actors) == 1, "%d actors" % len(actors))
        if len(actors) == 1:
            placed = actors[0].props.get("sites", [])
            check("make_map lays the sites of sites(recipe)", len(placed) == len(wanted), "%d against %d" % (len(placed), len(wanted)))
        check("make_map's summary counts the sites", bool(summaries) and ", %d sites" % len(wanted) in summaries[0], summaries[0] if summaries else "no summary")
        with open(os.path.join(saved, "MapLayouts", "Plain_Small.json"), encoding="utf-8") as handle:
            data = json.load(handle)
        check("make_map exports the sites for the painter", len(data.get("sites", [])) == len(wanted))
        del printed

    print("spawn_sites_test : %s" % ("PASS" if not failed else "FAIL (%s)" % ", ".join(failed)))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
