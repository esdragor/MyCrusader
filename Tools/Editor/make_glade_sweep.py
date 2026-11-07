"""Makes /Game/Maps/Crown, the size-less file of a listed map (a scenario map not in Tools/Data/Maps.csv becomes the first map of the list, HOLNewGameScreen::SetConfig; La Couronne has only sized files, so this one is free and never loaded by a match), the test map of the glade sweep (maps/glade_sweep.py): the same 300-tree forest eight times, other glades each time.
Run by Tools/make_glade_sweep.ps1 (full editor, quits when done):
  UnrealEditor.exe <project> -nosplash -unattended -Multiprocess -ExecCmds="py <this file>"
A file already there is never overwritten: delete Content/Maps/Crown.umap first (the script does it)."""
import os
import sys

import unreal

try:
    EDITOR_TOOLS = os.path.dirname(os.path.abspath(__file__))
except NameError:
    EDITOR_TOOLS = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()), "Tools", "Editor")
sys.path.insert(0, EDITOR_TOOLS)

from maps import common, glade_sweep


def main():
    # Two places at the 400 m test map's circle (the scenario needs a human and an AI slot); no base is laid, only the forests.
    recipe = common.Recipe("Plain", "%s/Crown" % common.MAPS_ROOT, [180.0, 0.0], common.SLOT_RADIUS, common.BOUNDS_HALF_SIZE, common.GROUND_SIZE)
    if os.path.exists(common.map_file(recipe)):
        raise RuntimeError("Already there, delete first: %s" % common.map_file(recipe))

    def done():
        unreal.SystemLibrary.quit_editor()

    try:
        common.make_map(recipe, glade_sweep, done)
    except Exception as error:
        unreal.log_error("Match map: %s" % error)
        unreal.SystemLibrary.quit_editor()


main()
