"""One editor session for several maps: arguments Map:Size,Size ... or Map:all (driver of create_match_map.py, scratchpad only)."""
import importlib
import os
import sys

import unreal

EDITOR_TOOLS = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()), "Tools", "Editor")
sys.path.insert(0, EDITOR_TOOLS)
from maps import common


def main():
    maps = common.read_maps()
    sizes = common.read_sizes()
    queue = []
    for argument in sys.argv[1:]:
        name, _, keys = argument.partition(":")
        module = importlib.import_module("maps." + maps[name])
        for key in (list(sizes) if keys == "all" else keys.split(",")):
            queue.append((module, common.sized_recipe(name, key, *sizes[key], layout_seed=1)))
    present = [common.map_file(recipe) for _, recipe in queue if os.path.exists(common.map_file(recipe))]
    if present:
        raise RuntimeError("Already there, delete first: %s" % ", ".join(present))

    def next_map():
        if not queue:
            unreal.SystemLibrary.quit_editor()
            return
        module, recipe = queue.pop(0)
        try:
            common.make_map(recipe, module, next_map)
        except Exception as error:
            unreal.log_error("Match map: %s" % error)
            unreal.SystemLibrary.quit_editor()

    next_map()


try:
    main()
except Exception:
    unreal.SystemLibrary.quit_editor()
    raise
