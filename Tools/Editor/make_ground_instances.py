"""Makes (or sets again) every ground instance of maps/ground.py GROUNDS, then quits the editor: run once before several editors make maps at the same time
(they would all try to create the missing instance). Editor: UnrealEditor ... -ExecCmds="py Tools/Editor/make_ground_instances.py" """
import os
import sys

import unreal

sys.path.insert(0, os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()), "Tools", "Editor"))
from maps import ground

try:
    for name in ground.GROUNDS:
        ground.instance(name)
        unreal.log("Ground instance ready: %s" % name)
finally:
    unreal.SystemLibrary.quit_editor()