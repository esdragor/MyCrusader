"""The varied ground of the maps (chantier Cartes, C1b; M-Q5: varied ground for every map). The parent material
/Game/Rendering/M_GroundVaried is made by Tools/Editor/make_ground_materials.py (three colours blended by a world noise,
the faint tile line of the building grid kept; since 2026-11-03 also a CC0 detail texture, see DETAILS below); each map that wants its own ground has an instance of it, set here, and names
it in its module (steppe.GROUND = "MI_Ground_Steppe"). A module without GROUND keeps the grey M_MatteGround (none now: every map but the Steppe and the fixed test map has its own). The
recipes make the instance when missing, so a map file never lacks it; the script sets them again (tuning a colour: edit
GROUNDS here, run the script, remake the maps only if you want the file to carry it - the instance is an asset, the map only
points to it).
"""
import unreal

FOLDER = "/Game/Rendering"
PARENT = "M_GroundVaried"
DEFAULT = FOLDER + "/M_MatteGround"


def color(r, g, b):
    return unreal.LinearColor(r, g, b, 1.0)


# name -> (ColorA the main ground, ColorB the wide patches, ColorC the rarer ones, NoiseScale in cm: the size of a patch, BFrom and
# BWidth, CFrom and CWidth: where the noise turns the ground into B / C and how crisp the edge is, see make_ground_materials.py).
# M-Q8 (the user, with a reference minimap): patches of a CLEARLY different colour, sandy / orange and dark mud, as irregular blobs (linear
# values; M_MatteGround is a grey of 0.16).
# M-Q10 (the user, 2026-10-30): no patches of sand or mud, a uniform grass: BFrom / CFrom 2.0, out of the noise's reach (0 to 1).
# The Steppe: yellow-green grass, orange sand, dark mud.
GROUNDS = {
    "MI_Ground_Steppe": (color(0.115, 0.145, 0.055), color(0.300, 0.200, 0.085), color(0.060, 0.045, 0.030), 2200.0, 2.0, 0.04, 2.0, 0.04),
    # The Hinterland (C2): a dark moss green, ochre sand and stony earth, grey-brown mud; smaller patches than the Steppe.
    "MI_Ground_Hinterland": (color(0.080, 0.115, 0.050), color(0.260, 0.170, 0.080), color(0.070, 0.062, 0.055), 2000.0, 2.0, 0.04, 2.0, 0.04),
    # 2026-11-04 (the user: "the ground of the Plain is grey; every map but the Steppe gets a real ground"): one textured ground per map, a uniform
    # colour with no patches (BFrom / CFrom out of the noise's reach), each its own tone so that the maps tell apart.
    "MI_Ground_Plain": (color(0.090, 0.165, 0.050), color(0.220, 0.200, 0.080), color(0.070, 0.060, 0.040), 2400.0, 2.0, 0.04, 2.0, 0.04),
    "MI_Ground_Forest": (color(0.045, 0.090, 0.035), color(0.150, 0.110, 0.050), color(0.040, 0.035, 0.030), 2400.0, 2.0, 0.04, 2.0, 0.04),
    "MI_Ground_Cols": (color(0.130, 0.120, 0.060), color(0.260, 0.200, 0.100), color(0.080, 0.070, 0.050), 2400.0, 2.0, 0.04, 2.0, 0.04),
    "MI_Ground_Fords": (color(0.100, 0.160, 0.070), color(0.220, 0.200, 0.090), color(0.070, 0.060, 0.040), 2400.0, 2.0, 0.04, 2.0, 0.04),
    "MI_Ground_ElDorado": (color(0.070, 0.130, 0.060), color(0.200, 0.170, 0.070), color(0.050, 0.050, 0.035), 2400.0, 2.0, 0.04, 2.0, 0.04),
    "MI_Ground_Crown": (color(0.110, 0.130, 0.090), color(0.230, 0.200, 0.110), color(0.070, 0.065, 0.055), 2400.0, 2.0, 0.04, 2.0, 0.04),
}


# Sol CC0 (etape 1, 2026-11-03): the detail texture of each instance (made by Tools/prepare_ground_details.py from the CC0 albedos of
# Poly Haven listed in Docs/AssetsExternes.md, imported by make_ground_materials.py). name -> (source name, TextureScale in cm: the
# size of one repeat of the first sample, the second is x0.37 of it, DetailStrength: 0 = none, 1 = the texture's own contrast).
TEXTURE_FOLDER = FOLDER + "/Textures/Ground"
DETAILS = {
    "MI_Ground_Steppe": ("grass_ground", 900.0, 0.7),
    "MI_Ground_Hinterland": ("forest_ground_06", 900.0, 0.7),
    "MI_Ground_Plain": ("grass_ground", 900.0, 0.7),
    "MI_Ground_Forest": ("forest_ground_06", 900.0, 0.7),
    "MI_Ground_Cols": ("sparse_grass", 900.0, 0.7),
    "MI_Ground_Fords": ("sparse_grass", 900.0, 0.7),
    "MI_Ground_ElDorado": ("grass_ground", 900.0, 0.7),
    "MI_Ground_Crown": ("sparse_grass", 900.0, 0.7),
}
# Every source texture to import (the alternatives are kept so that a map can be switched by DETAILS alone).
TEXTURES = ("grass_ground", "sparse_grass", "forest_ground_06")


def texture_name(source):
    return "T_Ground_%s_Detail" % "".join(part.capitalize() for part in source.split("_"))


def instance(name):
    """The instance of M_GroundVaried named name (a key of GROUNDS), made when missing, its parameters set (again)."""
    colors_and_scale = GROUNDS[name]
    path = FOLDER + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        material = unreal.load_asset(path)
    else:
        factory = unreal.MaterialInstanceConstantFactoryNew()
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, FOLDER, unreal.MaterialInstanceConstant, factory)
        if material is None:
            raise RuntimeError("Could not create " + path)
    parent = unreal.load_asset(FOLDER + "/" + PARENT)
    if parent is None:
        raise RuntimeError("Missing %s/%s: run Tools/Editor/make_ground_materials.py first" % (FOLDER, PARENT))
    lib = unreal.MaterialEditingLibrary
    lib.set_material_instance_parent(material, parent)
    for parameter, value in zip(("ColorA", "ColorB", "ColorC"), colors_and_scale[:3]):
        lib.set_material_instance_vector_parameter_value(material, parameter, value)
    for parameter, value in zip(("NoiseScale", "BFrom", "BWidth", "CFrom", "CWidth"), colors_and_scale[3:]):
        lib.set_material_instance_scalar_parameter_value(material, parameter, value)
    if name in DETAILS:
        source, texture_scale, strength = DETAILS[name]
        texture = unreal.load_asset(TEXTURE_FOLDER + "/" + texture_name(source))
        if texture is None:
            raise RuntimeError("Missing texture %s: run Tools/Editor/make_ground_materials.py first" % texture_name(source))
        lib.set_material_instance_texture_parameter_value(material, "GroundTexture", texture)
        lib.set_material_instance_scalar_parameter_value(material, "TextureScale", texture_scale)
        lib.set_material_instance_scalar_parameter_value(material, "DetailStrength", strength)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material
