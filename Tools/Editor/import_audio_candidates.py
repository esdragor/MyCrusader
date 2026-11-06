r"""Imports the CANDIDATES for the interface sounds (Script/Audio/HOLAudioCandidates.as, AudioInProgress.md) as sound waves:

  Kenney rpg-audio / impact-sounds .ogg  ->  /Game/Audio/Candidates/kenney/SW_<stem>
  Candidates\vsco\<stem>.wav (made by Tools/Audio/make_ui_candidates.py)  ->  /Game/Audio/Candidates/vsco/SW_<stem>

One asset per stem named in an `Add(List, "<Cue>", "<stems>")` line, once each, in the sound class SC_Effects. Sources stay outside the
repository (C:\Users\baudo\Documents\MyCrusader_Audio, Docs/assets_audio.csv). The candidate assets are NOT committed (they are
regenerated here in ~30 s); once the user has chosen, only the chosen ones move into the table.
Run in the editor: UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file> -unattended -Multiprocess
Environment: HOL_AUDIO_ROOT overrides the folder of the sources. Tools/Audio/check_audio_candidates.ps1 verifies the names first.
"""
import os
import re

PROJECT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
CANDIDATES = os.path.join(PROJECT, "Script", "Audio", "HOLAudioCandidates.as")
ROOT = os.environ.get("HOL_AUDIO_ROOT", r"C:\Users\baudo\Documents\MyCrusader_Audio")
FOLDER = "/Game/Audio/Candidates"
LINE = re.compile(r'^\s*Add\(List,\s*"[A-Za-z]+\.[A-Za-z]+",\s*"(?P<stems>[A-Za-z0-9_,]+)"')


def wanted():
    seen = []
    with open(CANDIDATES, encoding="utf-8") as table:
        for line in table:
            match = LINE.match(line)
            if match:
                for stem in match.group("stems").split(","):
                    if stem not in seen:
                        seen.append(stem)
    return seen


def source(stem):
    """(kind, path of the file)."""
    if stem.startswith("v_"):
        return "vsco", os.path.join(ROOT, "Candidates", "vsco", stem + ".wav")
    for pack in ("rpg-audio", "impact-sounds"):
        for folder, _, files in os.walk(os.path.join(ROOT, "Kenney", pack)):
            if stem + ".ogg" in files:
                return "kenney", os.path.join(folder, stem + ".ogg")
    raise RuntimeError("%s.ogg not found in Kenney rpg-audio / impact-sounds under %s" % (stem, ROOT))


def import_all(unreal):
    effects = unreal.load_asset("/Game/Audio/SC_Effects")
    count = 0
    for stem in wanted():
        kind, path = source(stem)
        if not os.path.exists(path):
            raise RuntimeError("missing source %s" % path)
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", path)
        task.set_editor_property("destination_path", FOLDER + "/" + kind)
        task.set_editor_property("destination_name", "SW_" + stem)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", False)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        sound = unreal.load_asset("%s/%s/SW_%s" % (FOLDER, kind, stem))
        if not isinstance(sound, unreal.SoundWave):
            raise RuntimeError("Could not import %s as a sound wave (got %s)" % (path, sound))
        sound.set_editor_property("sound_class_object", effects)
        unreal.EditorAssetLibrary.save_loaded_asset(sound)
        count += 1
    unreal.log("Audio candidates: %d sound waves under %s" % (count, FOLDER))


if __name__ == "__main__":
    try:
        import unreal
    except ImportError:
        for stem in wanted():
            print("%s -> %s" % (stem, source(stem)[1]))
    else:
        import_all(unreal)
