"""Creates the icons of the interface (E4-1, players' table):
  /Game/UI/Icons/T_Icon_Ally    a green shield, before an ally's name
  /Game/UI/Icons/T_Icon_Enemy   red crossed swords, before an enemy's name
  /Game/UI/Icons/T_Icon_HeroEmblem   the hero's portrait for now (J8a.5 loop 7): a white shield edged in gold with a gold
                                     sun (the paladin's kit), over the player's colour (the portrait button's background)
The PNGs (64x64, the emblem 128x128, transparent) are drawn here with PIL at 256 px then reduced, into Tools/Editor/Data/Icons, then
imported as interface textures (UI group, no mipmaps, UserInterface2D compression, sRGB).
Two steps (the editor's Python has no PIL):
  python Tools/Editor/make_ui_icons.py            draws the PNGs (kept in git)
  UnrealEditor.exe <project> -nosplash -unattended -ExecCmds="py <this file>"   imports them (the editor quits)
"""
import math
import os


BIG = 256
HERE = os.path.dirname(os.path.abspath(__file__))
PNG_FOLDER = os.path.join(HERE, "Data", "Icons")
FOLDER = "/Game/UI/Icons"

GREEN = (70, 200, 90, 255)
GREEN_DARK = (16, 70, 28, 255)
GREEN_LIGHT = (150, 235, 160, 255)
RED = (230, 60, 50, 255)
RED_DARK = (80, 10, 8, 255)
# The emblem: a dark edge around it reads on every player's colour (yellow included), the gold rim and sun inside.
EMBLEM_EDGE = (28, 20, 10, 255)
GOLD = (232, 182, 48, 255)
GOLD_DARK = (150, 100, 20, 255)
WHITE = (244, 240, 228, 255)
# Each icon's size in the PNG (and the texture).
SIZES = {"T_Icon_Ally": 64, "T_Icon_Enemy": 64, "T_Icon_HeroEmblem": 128}


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


def sun_points(centre, inner, outer, rays):
    """A sun's star: `rays` sharp rays from radius `inner` to `outer` around `centre`, one pointing up."""
    points = []
    for step in range(rays * 2):
        angle = math.pi * step / rays - math.pi / 2
        radius = outer if step % 2 == 0 else inner
        points.append((centre[0] + radius * math.cos(angle), centre[1] + radius * math.sin(angle)))
    return points


def draw_hero_emblem():
    """The paladin's shield: dark edge, gold rim, white field, a gold sun of 12 rays (dark gold edged) a bit above the
    middle, the room around it left transparent for the player's colour."""
    from PIL import Image, ImageDraw
    image = Image.new("RGBA", (BIG, BIG), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    draw.polygon(shield_points(0), fill=EMBLEM_EDGE)
    draw.polygon(shield_points(8), fill=GOLD)
    draw.polygon(shield_points(22), fill=WHITE)
    centre = (128, 112)
    draw.polygon(sun_points(centre, 38, 72, 12), fill=GOLD_DARK)
    draw.polygon(sun_points(centre, 32, 62, 12), fill=GOLD)
    draw.ellipse((centre[0] - 32, centre[1] - 32, centre[0] + 32, centre[1] + 32), fill=GOLD_DARK)
    draw.ellipse((centre[0] - 26, centre[1] - 26, centre[0] + 26, centre[1] + 26), fill=GOLD)
    return image


def write_pngs():
    from PIL import Image
    os.makedirs(PNG_FOLDER, exist_ok=True)
    paths = {}
    for name, image in (("T_Icon_Ally", draw_ally()), ("T_Icon_Enemy", draw_enemy()),
                        ("T_Icon_HeroEmblem", draw_hero_emblem())):
        path = os.path.join(PNG_FOLDER, name + ".png")
        image.resize((SIZES[name], SIZES[name]), Image.LANCZOS).save(path)
        paths[name] = path
    return paths


def png_paths():
    return {name: os.path.join(PNG_FOLDER, name + ".png") for name in SIZES}


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
