"""Creates /Game/Maps/MainMenu: an empty map whose game mode override is the menu's (D243, front-end step 3).

The interface draws its own background, so the map holds nothing. Needs the HeroesOfLibertyUI module compiled.
Run in a full editor session (the editor quits when done):
  UnrealEditor.exe <project> -ExecCmds="py <this file>"
"""
import os

import unreal

MAP_PATH = "/Game/Maps/MainMenu"
GAME_MODE_PATH = "/Script/HeroesOfLibertyUI.HOLMenuGameMode"


def main():
    # Checked on disk: the asset registry cache can still list a deleted map at startup.
    if os.path.exists(os.path.join(unreal.Paths.project_content_dir(), "Maps", "MainMenu.umap")):
        raise RuntimeError(MAP_PATH + " already exists: delete Content/Maps/MainMenu.umap first")

    game_mode = unreal.load_class(None, GAME_MODE_PATH)
    if game_mode is None:
        raise RuntimeError("Missing " + GAME_MODE_PATH + " (is the HeroesOfLibertyUI module compiled?)")

    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not level_subsystem.new_level(MAP_PATH, False):
        raise RuntimeError("Could not create " + MAP_PATH)

    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    world.get_world_settings().set_editor_property("default_game_mode", game_mode)
    if not level_subsystem.save_current_level():
        raise RuntimeError("Could not save " + MAP_PATH)
    unreal.log("MainMenu map created at %s (game mode %s)" % (MAP_PATH, GAME_MODE_PATH))


try:
    main()
finally:
    unreal.SystemLibrary.quit_editor()
