"""Creates the icons of the interface (E4-1, players' table):
  /Game/UI/Icons/T_Icon_Ally    a green shield, before an ally's name
  /Game/UI/Icons/T_Icon_Enemy   red crossed swords, before an enemy's name
The PNGs (64x64, transparent) are drawn here with PIL at 256 px then reduced, into Tools/Editor/Data/Icons, then
imported as interface textures (UI group, no mipmaps, UserInterface2D compression, sRGB).
Two steps (the editor's Python has no PIL):
  python Tools/Editor/make_ui_icons.py            draws the PNGs (kept in git)
  UnrealEditor.exe <project> -nosplash -unattended -ExecCmds="py <this file>"   imports them (the editor quits)
"""
import math
import os


SIZE = 64
BIG = 256
HERE = os.path.dirname(os.path.abspath(__file__))
PNG_FOLDER = os.path.join(HERE, "Data", "Icons")
FOLDER = "/Game/UI/Icons"

GREEN = (70, 200, 90, 255)
GREEN_DARK = (16, 70, 28, 255)
GREEN_LIGHT = (150, 235, 160, 255)
RED = (230, 60, 50, 255)
RED_DARK = (80, 10, 8, 255)


def shield_points(inset):
    """A heater shield in the 256 px square, `inset` px inside the outline."""
    top, left, right = 22 + inset, 34 + inset, 222 - inset
    points = [(left, top), (right, top), (right, 120)]
    # The lower sides curve down to the point.
    tip = (128, 236 - inset * 1.4)
    for step in range(1, 12):
        t = step / 12.0
        points.append((right + (tip[0] - right) * t, 120 + (tip[1] - 120) * math.sin(t * math.pi / 2)))
    points.append(tip)
    for step in range(11, 0, -1):
        t = step / 12.0
        points.append((left + (tip[0] - left) * t, 120 + (tip[1] - 120) * math.sin(t * math.pi / 2)))
    points.append((left, 120))
    return points


def draw_ally():
    from PIL import Image, ImageDraw
    image = Image.new("RGBA", (BIG, BIG), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    draw.polygon(shield_points(0), fill=GREEN_DARK)
    draw.polygon(shield_points(18), fill=GREEN)
    # A light band down the middle, readable at 16 px as the shield's ridge.
    draw.polygon([(118, 40), (138, 40), (138, 205), (128, 215), (118, 205)], fill=GREEN_LIGHT)
    return image


def sword_points(angle, width, length, inset):
    """One sword's blade, guard and grip as polygons, rotated by `angle` around the centre."""
    centre = 128.0
    c, s = math.cos(angle), math.sin(angle)

    def at(x, y):
        # x along the sword (tip at -length/2), y across it
        return (centre + x * c - y * s, centre + x * s + y * c)

    half = length / 2.0
    w = width / 2.0 - inset
    blade = [at(-half + inset * 1.6, 0), at(-half + 26, -w), at(half - 58, -w), at(half - 58, w), at(-half + 26, w)]
    g = 14 - inset
    guard = [at(half - 58 - g, -34 + inset), at(half - 58 + g, -34 + inset), at(half - 58 + g, 34 - inset),
             at(half - 58 - g, 34 - inset)]
    h = 10 - inset
    grip = [at(half - 58, -h), at(half - inset, -h), at(half - inset, h), at(half - 58, h)]
    return [blade, guard, grip]


def draw_enemy():
    from PIL import Image, ImageDraw
    image = Image.new("RGBA", (BIG, BIG), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    swords = [math.radians(45), math.radians(135)]
    # Both outlines first, then both fills: the crossing reads as one shape.
    for inset, colour in ((0, RED_DARK), (11, RED)):
        for angle in swords:
            for part in sword_points(angle, 58, 300, inset):
                draw.polygon(part, fill=colour)
    return image


def write_pngs():
    from PIL import Image
    os.makedirs(PNG_FOLDER, exist_ok=True)
    paths = {}
    for name, image in (("T_Icon_Ally", draw_ally()), ("T_Icon_Enemy", draw_enemy())):
        path = os.path.join(PNG_FOLDER, name + ".png")
        image.resize((SIZE, SIZE), Image.LANCZOS).save(path)
        paths[name] = path
    return paths


def png_paths():
    return {name: os.path.join(PNG_FOLDER, name + ".png") for name in ("T_Icon_Ally", "T_Icon_Enemy")}


def import_textures(paths):
    import unreal

    for name, path in paths.items():
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", path)
        task.set_editor_property("destination_path", FOLDER)
        task.set_editor_property("destination_name", name)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", False)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        asset = FOLDER + "/" + name
        texture = unreal.load_asset(asset)
        if not isinstance(texture, unreal.Texture2D):
            raise RuntimeError("Could not import %s as a texture (got %s)" % (path, texture))
        texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
        texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
        texture.set_editor_property("srgb", True)
        texture.set_editor_property("never_stream", True)
        if not unreal.EditorAssetLibrary.save_loaded_asset(texture, only_if_is_dirty=False):
            raise RuntimeError("Could not save " + asset)
        unreal.log("UI icon %s from %s" % (asset, path))


try:
    import unreal  # noqa: F401
    IN_EDITOR = True
except ImportError:
    IN_EDITOR = False

if IN_EDITOR:
    try:
        import_textures(png_paths())
    finally:
        unreal.SystemLibrary.quit_editor()
else:
    for found in write_pngs().values():
        print(found)
