"""Imports the Kenney CC0 sounds named by the table Script/Audio/HOLAudioTable.as (AudioInProgress.md A2) as sound waves:

  C:\\Users\\baudo\\Documents\\MyCrusader_Audio\\Kenney\\<pack>\\...\\<file>.ogg  ->  /Game/Audio/Kenney/<pack>/SW_<file>

One asset per file named in an `Add(Cues, ..., "<pack>", "<file>,<file>", ...)` line, in the sound class SC_Effects (the
volume sliders of the options). The packs stay outside the repository (Docs/assets_audio.csv); the assets are in it.
Run in the editor: UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file> [-unattended]
A silent cue (empty pack and files, e.g. Ui.Hover) names no file. Only the sounds not yet imported are imported (the .uasset exists:
skipped, so the committed assets stay byte for byte), unless HOL_KENNEY_FORCE=1.
Environment: HOL_KENNEY_ROOT overrides the folder of the packs. Tools/Audio/check_audio_table.ps1 verifies the names first.
"""
import os
import re

PROJECT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
TABLE = os.path.join(PROJECT, "Script", "Audio", "HOLAudioTable.as")
ROOT = os.environ.get("HOL_KENNEY_ROOT", r"C:\Users\baudo\Documents\MyCrusader_Audio\Kenney")
FOLDER = "/Game/Audio/Kenney"
LINE = re.compile(r'^\s*Add\(Cues,.*?"(?P<name>[A-Za-z]+\.[A-Za-z]+)",\s*"(?P<pack>[a-z-]*)",\s*"(?P<stems>[A-Za-z0-9_,]*)"')


def wanted():
    """(pack, stem) pairs of the table, once each, in the table's order."""
    seen = []
    with open(TABLE, encoding="utf-8") as table:
        for line in table:
            match = LINE.match(line)
            if not match:
                continue
            for stem in match.group("stems").split(","):
                if not stem:
                    continue
                if (match.group("pack"), stem) not in seen:
                    seen.append((match.group("pack"), stem))
    return seen


def find(pack, stem):
    for folder, _, files in os.walk(os.path.join(ROOT, pack)):
        if stem + ".ogg" in files:
            return os.path.join(folder, stem + ".ogg")
    raise RuntimeError("%s.ogg not found in pack %s under %s" % (stem, pack, ROOT))


def import_all(unreal):
    effects = unreal.load_asset("/Game/Audio/SC_Effects")
    count = 0
    skipped = 0
    for pack, stem in wanted():
        if not os.environ.get("HOL_KENNEY_FORCE") and os.path.exists(os.path.join(PROJECT, "Content", "Audio", "Kenney", pack, "SW_" + stem + ".uasset")):
            skipped += 1
            continue
        source = find(pack, stem)
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", source)
        task.set_editor_property("destination_path", FOLDER + "/" + pack)
        task.set_editor_property("destination_name", "SW_" + stem)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", False)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        asset = "%s/%s/SW_%s" % (FOLDER, pack, stem)
        sound = unreal.load_asset(asset)
        if not isinstance(sound, unreal.SoundWave):
            raise RuntimeError("Could not import %s as a sound wave (got %s)" % (source, sound))
        sound.set_editor_property("sound_class_object", effects)
        unreal.EditorAssetLibrary.save_loaded_asset(sound)
        count += 1
    unreal.log("Kenney sounds: %d sound waves imported (%d already there) under %s" % (count, skipped, FOLDER))


if __name__ == "__main__":
    try:
        import unreal
    except ImportError:
        for pack, stem in wanted():
            print("%s/%s -> %s" % (pack, stem, find(pack, stem)))
    else:
        import_all(unreal)
