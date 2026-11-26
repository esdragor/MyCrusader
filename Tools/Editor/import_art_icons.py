r"""Imports the icon booth PNG (Saved/ArtIcons, made by the art_icon_all scenario, AR5-d) as interface textures (AR5-e):

  Saved\ArtIcons\T_Unit_<Type>.png      ->  /Game/UI/Icons/Units/T_Unit_<Type>          (read by Script/UI/HOLGarrisonFrameRules.as)
  Saved\ArtIcons\T_Building_<Type>.png  ->  /Game/UI/Icons/Buildings/T_Building_<Type>
  Saved\ArtIcons\T_Source_<Type>.png    ->  /Game/UI/Icons/Sources/T_Source_<Type>

Interface settings: no mips, compression UserInterface2D, sRGB, group UI. Idempotent (replace_existing). Then the Icon field of the
type's DA_Art (/Game/Art/<Units|Buildings|Sources>/<Type>/DA_Art_<Type>, loaded or created by art_visual_io.load_or_create) points
to the texture. Run in the editor (empty build queue, D920 rule 13):
  UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file> -unattended -Multiprocess
Without the editor (plain python) it prints "<png> -> <asset>" for each PNG; `--plan <name> ...` prints the destination of given names
(Tools/Tests/import_art_icons_test.ps1). Environment: HOL_ART_ICONS overrides the folder of the PNG.
"""
import os
import sys

PROJECT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
SOURCE = os.environ.get("HOL_ART_ICONS", os.path.join(PROJECT, "Saved", "ArtIcons"))
ICON_ROOT = "/Game/UI/Icons"
# Prefix of the PNG name -> family folder (the same under /Game/UI/Icons and /Game/Art).
FAMILIES = {"Unit": "Units", "Building": "Buildings", "Source": "Sources"}


def split(name):
    """T_Unit_Archer.png -> ("Units", "Archer", "T_Unit_Archer"), None for any other name."""
    stem, ext = os.path.splitext(os.path.basename(name))
    if ext.lower() != ".png":
        return None
    parts = stem.split("_", 2)
    if len(parts) != 3 or parts[0] != "T" or parts[1] not in FAMILIES or not parts[2]:
        return None
    return FAMILIES[parts[1]], parts[2], stem


def destination(name):
    """The package path of the texture made from the PNG `name`, None when the name is not an icon booth PNG."""
    parsed = split(name)
    if parsed is None:
        return None
    family, _, stem = parsed
    return "%s/%s/%s" % (ICON_ROOT, family, stem)


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


def import_all(unreal):
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    import art_visual_io

    tools = unreal.AssetToolsHelpers.get_asset_tools()
    count = 0
    filled = 0
    for name in pngs():
        asset_path = destination(name)
        if asset_path is None:
            unreal.log_warning("Art icons: skipped %s (not T_<Unit|Building|Source>_<Type>.png)" % name)
            continue
        family, type_name, stem = split(name)
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", os.path.join(SOURCE, name))
        task.set_editor_property("destination_path", asset_path.rsplit("/", 1)[0])
        task.set_editor_property("destination_name", stem)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", False)
        tools.import_asset_tasks([task])
        texture = unreal.load_asset(asset_path)
        if not isinstance(texture, unreal.Texture2D):
            raise RuntimeError("Could not import %s as a texture (got %s)" % (name, texture))
        texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)  # UserInterface2D
        texture.set_editor_property("srgb", True)
        texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
        if not unreal.EditorAssetLibrary.save_loaded_asset(texture, False):
            raise RuntimeError("Could not save " + asset_path)
        count += 1
        art, art_path = art_visual_io.load_or_create(family, type_name)
        art.set_editor_property("icon", texture)
        if not unreal.EditorAssetLibrary.save_loaded_asset(art, False):
            raise RuntimeError("Could not save " + art_path)
        filled += 1
        unreal.log("Art icons: %s -> %s, %s.Icon set" % (name, asset_path, art_path))
    unreal.log("Art icons: done textures=%d icons=%d from %s" % (count, filled, SOURCE))


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--plan":
        for line in plan(sys.argv[2:]):
            print(line)
        sys.exit(0)
    try:
        import unreal
    except ImportError:
        for line in plan(pngs()):
            print(line)
    else:
        import_all(unreal)
