"""Makes a match map file from its recipe (D256, J1; TC2; the chantier Cartes, C0.3): the shared pieces of
maps/common.py (ground, light and sky, bounds, navmesh, start slots, each player's base) and the map's own obstacles and
sources from its module under Tools/Editor/maps, named by Tools/Data/Maps.csv (Name, DisplayName, Recipe: Plain -> plain).
Each map is made once per named size (TC-Q3 = A, Tools/Data/MapSizes.csv) as /Game/Maps/<Map>_<Size> (Small, Medium,
Normal, Large, Huge); Plain alone also has a fixed test map, /Game/Maps/Plain, as it has always been (the scenarios and
quick_check count on its slots, D494). The main centres are not in the map: the game mode puts one on each occupied slot
when a match starts (D259). The recipe itself (what is placed and where) is described in maps/common.py and maps/plain.py.

Needs the HeroesOfLiberty module compiled. Run in a full editor session (the editor quits when done):
  UnrealEditor.exe <project> -nosplash -unattended -ExecCmds="py <this file> [<Map>] [<Size>... | all]"
No argument: /Game/Maps/Plain, the test map. <Map> alone: that map's test map (Plain only: the other maps have none).
<Map> all, or <Map> <Size>...: the sized files /Game/Maps/<Map>_<Size> (Sizes: the keys of Tools/Data/MapSizes.csv, Small
Medium Normal Large Huge). Without a map name, the sizes (or all) are Plain's, as before: "all" = "Plain all".
seed=N anywhere in the arguments (C1c): the layout seed of the maps made (default 1, the layout of the shipped files): a map that draws
its shapes and placements from it (the Steppe) comes out as another variant; the file name stays <Map>_<Size> (Plain ignores it).
A map file already there is never overwritten: delete Content/Maps/<map>.umap first.
"""
import importlib
import os
import sys

import unreal

# The recipes are a package beside this file; the editor runs it by path (py <file>), not from its folder.
try:
    EDITOR_TOOLS = os.path.dirname(os.path.abspath(__file__))
except NameError:  # no __file__ from the editor's py command: the project's Tools/Editor
    EDITOR_TOOLS = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()), "Tools", "Editor")
sys.path.insert(0, EDITOR_TOOLS)

from maps import common

TEST_MAP = "Plain"  # The only map with a fixed test map (D494).


def split_seed(arguments):
    """The arguments without seed=N, and N (default 1)."""
    seeds = [argument for argument in arguments if argument.lower().startswith("seed=")]
    if len(seeds) > 1 or any(not seed[5:].lstrip("-").isdigit() for seed in seeds):
        raise RuntimeError("Give seed=N once, N an integer: %s" % seeds)
    return [argument for argument in arguments if argument not in seeds], int(seeds[0][5:]) if seeds else 1


def recipes_of(arguments):
    """The map named first (default Plain) and its sizes: none = its test map, "all" or the keys of MapSizes.csv; seed=N: the layout seed."""
    arguments, layout_seed = split_seed(arguments)
    maps = common.read_maps()
    if arguments and arguments[0] in maps:
        map_name, size_arguments = arguments[0], arguments[1:]
    else:
        map_name, size_arguments = TEST_MAP, arguments
    if not size_arguments:
        if map_name != TEST_MAP:
            raise RuntimeError("%s has no test map (only %s has one, D494): give a size (%s) or all" % (
                map_name, TEST_MAP, ", ".join(common.read_sizes())))
        recipes = [common.test_recipe(map_name, layout_seed)]
    else:
        sizes = common.read_sizes()
        keys = list(sizes) if [argument.lower() for argument in size_arguments] == ["all"] else size_arguments
        unknown = [key for key in keys if key not in sizes]
        if unknown:
            raise RuntimeError("Unknown maps or sizes %s (Tools/Data/Maps.csv: %s; Tools/Data/MapSizes.csv: %s)" % (
                unknown, ", ".join(maps), ", ".join(sizes)))
        recipes = [common.sized_recipe(map_name, key, *sizes[key], layout_seed=layout_seed) for key in keys]
    return importlib.import_module("maps." + maps[map_name]), recipes


def main():
    module, recipes = recipes_of(sys.argv[1:])
    present = [common.map_file(recipe) for recipe in recipes if os.path.exists(common.map_file(recipe))]
    if present:
        raise RuntimeError("Already there, delete first: %s" % ", ".join(present))
    queue = list(recipes)

    def next_map():
        if not queue:
            unreal.SystemLibrary.quit_editor()
            return
        try:
            common.make_map(queue.pop(0), module, next_map)
        except Exception as error:
            unreal.log_error("Match map: %s" % error)
            unreal.SystemLibrary.quit_editor()

    next_map()


try:
    main()
except Exception:
    unreal.SystemLibrary.quit_editor()
    raise
