r"""Imports the ELECTED softened sounds (pack "soft" of Script/Audio/HOLAudioTable.as, e.g. Select.BuildingWood = s_wood_door) as sound waves:

  Candidates\soft\<stem>.wav (made by Tools/Audio/make_soft_candidates.py)  ->  /Game/Audio/Soft/SW_<stem>

One asset per stem of an `Add(Cues, ..., "soft", "<stems>", ...)` line, in the sound class SC_Effects. Unlike the candidates (never committed), these
assets are the ones the game plays: they are committed (D736: only the elected sounds). The source WAV stays outside the repository
(C:\Users\baudo\Documents\MyCrusader_Audio, Docs/assets_audio.csv). Run in the editor:
  UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file> -unattended -Multiprocess
Environment: HOL_AUDIO_ROOT overrides the folder of the sources. Without the editor (plain python) it only prints what it would import.
Check afterwards: Tools/Audio/check_audio_table.ps1 -Assets.
"""
import os
import re

PROJECT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
TABLE = os.path.join(PROJECT, "Script", "Audio", "HOLAudioTable.as")
ROOT = os.environ.get("HOL_AUDIO_ROOT", r"C:\Users\baudo\Documents\MyCrusader_Audio")
FOLDER = "/Game/Audio/Soft"
LINE = re.compile(r'^\s*Add\(Cues,[^"]*"[A-Za-z]+\.[A-Za-z]+",\s*"soft",\s*"(?P<stems>[A-Za-z0-9_,]+)"')


def wanted():
    seen = []
    with open(TABLE, encoding="utf-8") as table:
        for line in table:
            match = LINE.match(line)
            if match:
                for stem in match.group("stems").split(","):
                    if stem not in seen:
                        seen.append(stem)
    return seen


def source(stem):
    return os.path.join(ROOT, "Candidates", "soft", stem + ".wav")


def import_all(unreal):
    effects = unreal.load_asset("/Game/Audio/SC_Effects")
    count = 0
    for stem in wanted():
        path = source(stem)
        if not os.path.exists(path):
            raise RuntimeError("missing source %s" % path)
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", path)
        task.set_editor_property("destination_path", FOLDER)
        task.set_editor_property("destination_name", "SW_" + stem)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", False)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        sound = unreal.load_asset("%s/SW_%s" % (FOLDER, stem))
        if not isinstance(sound, unreal.SoundWave):
            raise RuntimeError("Could not import %s as a sound wave (got %s)" % (path, sound))
        sound.set_editor_property("sound_class_object", effects)
        unreal.EditorAssetLibrary.save_loaded_asset(sound)
        count += 1
    unreal.log("Audio elected: %d sound waves under %s" % (count, FOLDER))


if __name__ == "__main__":
    try:
        import unreal
    except ImportError:
        for stem in wanted():
            print("%s -> %s" % (stem, source(stem)))
    else:
        import_all(unreal)
