r"""Imports the icon booth PNG (Saved/ArtIcons, made by the art_icon_all scenario, AR5-d) as interface textures (AR5-e):

  Saved\ArtIcons\T_Unit_<Type>.png      ->  /Game/UI/Icons/Units/T_Unit_<Type>          (read by Script/UI/HOLGarrisonFrameRules.as)
  Saved\ArtIcons\T_Building_<Type>.png  ->  /Game/UI/Icons/Buildings/T_Building_<Type>
  Saved\ArtIcons\T_Source_<Type>.png    ->  /Game/UI/Icons/Sources/T_Source_<Type>

Interface settings: no mips, compression UserInterface2D, sRGB, group UI. Idempotent (replace_existing). Run in the editor:
  UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file> -unattended -Multiprocess
Without the editor (plain python) it prints "<png> -> <asset>" for each PNG; `--plan <name> ...` prints the destination of given names
(Tools/Tests/import_art_icons_test.ps1).
"""
import os
import sys

PROJECT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
SOURCE = os.environ.get("HOL_ART_ICONS", os.path.join(PROJECT, "Saved", "ArtIcons"))


def destination(name):
    # Bouchon (rouge AR5-e) : aucun chemin calcule.
    return None


def pngs():
    if not os.path.isdir(SOURCE):
        return []
    return sorted(f for f in os.listdir(SOURCE) if f.lower().endswith(".png"))


def plan(names):
    lines = []
    for name in names:
        asset = destination(name)
        lines.append("%s -> %s" % (name, asset if asset else "NONE"))
    return lines


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--plan":
        for line in plan(sys.argv[2:]):
            print(line)
        sys.exit(0)
    for line in plan(pngs()):
        print(line)
