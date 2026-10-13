"""Creates the font of the interface (D251, front-end finishing F4):
  /Game/UI/Fonts/NotoSansJP_Regular, NotoSansJP_Bold   font faces of Noto Sans JP (SIL Open Font License 1.1)
  /Game/UI/Fonts/F_HOL                                  composite font: Roboto of the engine + a Japanese sub-font
The font files come from the official repository notofonts/noto-cjk, at a fixed version (Sans2.004, Japanese subset),
downloaded into Intermediate/FontSources when missing; the licence is copied next to the assets.
Re-running the script re-imports the faces and remakes the composite font.
Run in a full editor session (the font import needs Slate, a commandlet crashes on it; the editor quits when done):
  UnrealEditor.exe <project> -nosplash -unattended -ExecCmds="py <this file>"
"""
import os
import shutil
import urllib.request

import unreal

FOLDER = "/Game/UI/Fonts"
SOURCE = "https://raw.githubusercontent.com/notofonts/noto-cjk/Sans2.004/"
FACES = {
    "NotoSansJP_Regular": "Sans/SubsetOTF/JP/NotoSansJP-Regular.otf",
    "NotoSansJP_Bold": "Sans/SubsetOTF/JP/NotoSansJP-Bold.otf",
}
LICENCE = "LICENSE"
BASE_FONT = "/Engine/EngineFonts/Roboto.Roboto"

PROJECT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
DOWNLOADS = os.path.join(PROJECT, "Intermediate", "FontSources")


def fetch(path):
    """The local copy of a file of the repository, downloaded once."""
    local = os.path.join(DOWNLOADS, os.path.basename(path))
    if not os.path.exists(local):
        os.makedirs(DOWNLOADS, exist_ok=True)
        unreal.log("Downloading " + SOURCE + path)
        urllib.request.urlretrieve(SOURCE + path, local)
    return local


def import_face(name, path):
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", fetch(path))
    task.set_editor_property("destination_path", FOLDER)
    task.set_editor_property("destination_name", name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    face = unreal.load_asset(FOLDER + "/" + name)
    if not isinstance(face, unreal.FontFace):
        raise RuntimeError("Could not import %s as a font face (got %s)" % (path, face))
    return face


def main():
    faces = {name: import_face(name, path) for name, path in FACES.items()}

    path = FOLDER + "/F_HOL"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        font = unreal.load_asset(path)
    else:
        font = unreal.AssetToolsHelpers.get_asset_tools().create_asset("F_HOL", FOLDER, unreal.Font, unreal.FontFactory())
    if font is None:
        raise RuntimeError("Could not create " + path)
    if not unreal.HOLFonts.make_composite_font(font, unreal.load_asset(BASE_FONT), faces["NotoSansJP_Regular"],
                                              faces["NotoSansJP_Bold"]):
        raise RuntimeError("Could not make the composite font " + path)
    if not unreal.EditorAssetLibrary.save_loaded_asset(font, only_if_is_dirty=False):
        raise RuntimeError("Could not save " + path)

    content = os.path.join(PROJECT, "Content", "UI", "Fonts")
    shutil.copyfile(fetch(LICENCE), os.path.join(content, "NotoSansJP-LICENSE.txt"))
    unreal.log("UI fonts in %s: F_HOL = Roboto + %s" % (FOLDER, ", ".join(faces)))


try:
    main()
finally:
    unreal.SystemLibrary.quit_editor()
