"""Simulator of the map generation's room, without the editor (Retours\\Q-glade-roomerror.md; chantier Cartes, the regeneration of the maps): plays the REAL chain of
common.make_map that draws the trees (use_glade_style, lay_relays_and_sources, outside_relays, apply_edits, the map's sites, outside_sites) for a map at a size, with the recording
`unreal` stand-in of spawn_sites_test.py, and says whether the map comes out or the generator refuses (a RuntimeError: steppe.RoomError, "No room for a grove", "a territory
holds N trees ..., the budget is M"): the refusals make_maps.ps1 would meet in the editor, found here in seconds instead of an hour of editor time.
  python Tools/Tests/glade_budget.py <Map> [<Size>...] [--vs-a]     one line per size; --vs-a also runs style A (today's maps) and prints its tree count
  python Tools/Tests/glade_budget.py all [--vs-a]                   the eight maps of Tools/Data/Maps.csv, five sizes each
  python Tools/Tests/glade_budget.py <Map> --style <Letter>         another style than the coded one (A to H) for the same map
Exit code 0 when every line is ok, 1 otherwise. The picks file (Tools/Editor/maps/grove_picks.json) grows by the keys of the styles played (commit it with the code).
"""
import contextlib
import io
import importlib
import os
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import spawn_sites_test as base  # noqa: E402  (installs the stand-in `unreal` and holds the recorder)

MAPS = ["Plain", "Steppe", "Hinterland", "ElDorado", "Cols", "Forest", "Fords", "Crown"]  # Tools/Data/Maps.csv
SIZES = ["Small", "Medium", "Normal", "Large", "Huge"]  # Tools/Data/MapSizes.csv
TREE_BUDGET_PER_PLAYER = 400  # maps-tree-budget-400 (2026-11-04): the base budget; the forests and the steppe multiply it by their own factors (steppe.py), Map.Table checks the real one

_saved = tempfile.mkdtemp(prefix="glade_budget_")
base.install_recorder(_saved)
base.Actors.get_all_level_actors = lambda self: []  # drop_buried (apply_edits) asks the level for its actors: the rocks (obstacles) are not laid here, so nothing is "buried"


def _apply(common, values):
    """The glade knobs of a style (glades, scale, reach); the shape cache does not know them."""
    if values != (common.GLADE_FORCE, common.GLADE_SCALE, common.GLADE_REACH):
        common.GLADE_FORCE, common.GLADE_SCALE, common.GLADE_REACH = values
        common.SHAPES.clear()


def simulate(map_name, size, style=None, module=None):
    """One map at one size. style: None = the coded GLADE_STYLE_BY_MAP, a letter of GLADE_STYLES, or a tuple (letter, glades, scale, reach) for a style that does not exist.
    Returns {map, size, style, status 'ok' | 'RoomError', message, trees, per_player, relay_variants, seconds}."""
    from maps import common  # noqa: E402
    sizes = common.read_sizes()
    places, side = sizes[size]
    recipe = common.sized_recipe(map_name, size, places, side)
    if module is None:
        module = importlib.import_module("maps." + common.read_maps()[map_name])
    if style is None:
        letter, values = common.glade_style(map_name), None
        values = common.GLADE_STYLES[letter]
    elif isinstance(style, str):
        letter, values = style, common.GLADE_STYLES[style]
    else:
        letter, values = style[0], tuple(style[1:])
    row = {"map": map_name, "size": size, "style": letter, "status": "ok", "message": "", "trees": 0, "per_player": 0.0, "relay_variants": 0}
    started = time.time()
    log = io.StringIO()
    sites_clear = getattr(common, "SITES_CLEAR_SOURCES", False)
    try:
        _apply(common, values)
        with contextlib.redirect_stdout(log):
            relays, sources, groups = common.lay_relays_and_sources(recipe, module)
            sources = common.outside_relays(sources, relays)
            if not getattr(module, "NO_EDITS", False):
                sources = common.apply_edits(recipe, sources, relays, module)
            sites = list(module.sites(recipe)) if hasattr(module, "sites") else []
            if sites_clear and sites:
                sources = common.outside_sites(sources, sites, recipe)
        row["trees"] = sum(1 for source in sources if str(source.get_editor_property("type")) == "Tree")
        row["per_player"] = row["trees"] / float(places)
    except RuntimeError as error:  # steppe.RoomError is a RuntimeError, so are the "No room" and "budget" refusals of common
        row["status"], row["message"] = "RoomError", "%s: %s" % (type(error).__name__, str(error)[:200])
    finally:
        common.SHAPES.clear()
        common.GLADE_FORCE, common.GLADE_SCALE, common.GLADE_REACH = common.GLADE_BASE
        common.RELAY_ZONES[:] = []
    row["relay_variants"] = sum(1 for line in log.getvalue().splitlines() if "relay variant" in line and "refused" in line)
    row["seconds"] = round(time.time() - started, 1)
    return row


def simulate_map(map_name, sizes=None, style=None):
    return [simulate(map_name, size, style) for size in (sizes or SIZES)]


def describe(row, versus=None):
    text = "%s %s style %s: %s" % (row["map"], row["size"], row["style"], row["status"])
    if row["status"] == "ok":
        text += " trees %d (%.0f per player, base budget %d)" % (row["trees"], row["per_player"], TREE_BUDGET_PER_PLAYER)
    else:
        text += " (%s)" % row["message"]
    if row["relay_variants"]:
        text += " [%d relay variant(s) refused first]" % row["relay_variants"]
    if versus is not None:
        text += " | style A: %s" % (("trees %d" % versus["trees"]) if versus["status"] == "ok" else "RoomError")
    return text + " %.1f s" % row["seconds"]


def main(arguments):
    versus = "--vs-a" in arguments
    style = None
    if "--style" in arguments:
        style = arguments[arguments.index("--style") + 1]
    names = [a for a in arguments if not a.startswith("--") and a != style]
    maps = MAPS if names[:1] == ["all"] else names[:1]
    sizes = names[1:] if names[:1] != ["all"] else []
    if not maps:
        print(__doc__)
        return 1
    failed = 0
    for map_name in maps:
        for size in sizes or SIZES:
            try:
                row = simulate(map_name, size, style)
                other = simulate(map_name, size, "A") if versus and row["style"] != "A" else None
            except Exception as error:  # noqa: BLE001  (a fault of the harness, not a refusal of the generator: said, and the next size goes on)
                print("%s %s: HARNESS ERROR %s: %s" % (map_name, size, type(error).__name__, error), flush=True)
                failed += 1
                continue
            print(describe(row, other), flush=True)
            failed += row["status"] != "ok"
    print("glade_budget : %s" % ("ALL OK" if not failed else "%d REFUSED" % failed))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
