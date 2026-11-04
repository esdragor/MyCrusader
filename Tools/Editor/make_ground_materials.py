"""Creates the varied ground of the maps (chantier Cartes, C1b; M-Q5: varied ground for every map):
  /Game/Rendering/M_GroundVaried    parent: three colours (ColorA, ColorB, ColorC) and a NoiseScale (cm), blended by a world
                                    noise of two octaves so that the ground shows patches: ColorA is the main ground, ColorB
                                    fills wide soft patches, ColorC rarer ones. The same matte look as M_MatteGround (D265: no
                                    specular, full roughness) and the same faint line on the building grid's tiles (4 m, through
                                    the world's origin, D344): the line is the ground's own colour, darker, so it reads on
                                    every patch and the camera's motion still shows (D265).
                                    Sol CC0 (etape 1, 2026-11-03): the colour is then multiplied by a detail texture (a CC0 grass or earth of
                                    Poly Haven, Docs/AssetsExternes.md) projected by WORLD coordinates (no UV: the ground is a plane of
                                    any size), twice at two unrelated scales (TextureScale, then x DETAIL_RATIO: no visible repeat), each
                                    centred on 1 so that the map keeps the average colour it chose; DetailStrength (0 = off) sets the
                                    contrast. Two samples of one texture, no normal map (D258: no light effect; the budget is 144 fps).
  /Game/Rendering/MI_Ground_<Map>   one instance per map that wants its own ground (maps/ground.py GROUNDS; the recipes make
                                    them too when missing, so a map file never lacks its instance).
/Game/Rendering/Textures/Ground/T_Ground_<name>_Detail: the detail textures, imported from Tools/Data/GroundSources/<name>_detail.png
(made by Tools/prepare_ground_details.py, linear colour, mipmaps, anisotropic filtering: the project's setting) when missing or changed.
Only the missing parent is made (delete M_GroundVaried.uasset to make it again: editing a material in place crashes the
editor, see make_matte_materials.py); the instances are always set again, so tuning a colour is: edit maps/ground.py, run this.
Run (the editor quits when done):
  UnrealEditor.exe <project> -nosplash -unattended -ExecCmds="py <this file>"
"""
import os
import sys

import unreal

try:
    EDITOR_TOOLS = os.path.dirname(os.path.abspath(__file__))
except NameError:  # no __file__ from the editor's py command: the project's Tools/Editor
    EDITOR_TOOLS = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()), "Tools", "Editor")
sys.path.insert(0, EDITOR_TOOLS)

from maps import ground

lib = unreal.MaterialEditingLibrary
assets = unreal.EditorAssetLibrary

GRID_CELL = 400.0  # cm: UHOLBuildingSettings::TileSize (D344)
GRID_LINE = 0.015  # fraction of a cell, each side of the line: 12 cm wide
LINE_DARKEN = 0.62  # the line is the ground's colour times this (M_MatteGround: line 0.09 over ground 0.16)
# Patches (M-Q8, the user: ground patches of a clearly different colour, as the reference minimap shows): the noise (0 to 1, mostly 0.3 to
# 0.7) goes from ColorA to ColorB from BFrom over BWidth (a narrow width: a crisp edge, an irregular blob), a second noise from that to
# ColorC from CFrom over CWidth (rarer). The four numbers are PARAMETERS of the material (defaults here), set by each map's instance
# (maps/ground.py GROUNDS), so that tuning the patches is editing GROUNDS and running this script, not remaking the parent.
B_FROM, B_WIDTH = 0.60, 0.05
C_FROM, C_WIDTH = 0.62, 0.05
SECOND_SCALE = 1.7  # the second noise is this much wider
SECOND_OFFSET = 71234.0  # cm: and elsewhere in the world
DETAIL_RATIO = 0.37  # the second detail sample's tile is this much of the first one's (unrelated sizes: the repeats never line up)
DETAIL_OFFSET = 1234.5  # cm: and shifted
SOURCES = os.path.join(os.path.dirname(EDITOR_TOOLS), "Data", "GroundSources")


def expression(material, expression_class, x, y, **properties):
    node = lib.create_material_expression(material, expression_class, x, y)
    for key, value in properties.items():
        node.set_editor_property(key, value)
    return node


def connect(source, target, pin=""):
    if not lib.connect_material_expressions(source, "", target, pin):
        raise RuntimeError("Could not connect %s to %s.%s" % (source.get_name(), target.get_name(), pin))


def noise(material, y, scale, offset):
    """A world noise of two octaves in 0..1, at 1 / scale (a node), the world shifted by offset (cm)."""
    position = expression(material, unreal.MaterialExpressionWorldPosition, -2000, y)
    shifted = expression(material, unreal.MaterialExpressionAdd, -1850, y, const_b=offset)
    scaled = expression(material, unreal.MaterialExpressionDivide, -1700, y)
    connect(position, shifted)
    connect(shifted, scaled, "A")
    connect(scale, scaled, "B")
    node = expression(material, unreal.MaterialExpressionNoise, -1500, y, scale=1.0, quality=1, levels=2, level_scale=2.0,
                      turbulence=False, output_min=0.0, output_max=1.0)
    node.set_editor_property("noise_function", unreal.NoiseFunction.NOISEFUNCTION_GRADIENT_ALU)
    connect(scaled, node)  # the node's first input is the position
    return node


def ramp(material, source, low, width, x, y):
    """smoothstep(low, low + width, source): saturate((source - low) / width), then 3t^2 - 2t^3: a transition of the given width.
    low and width are nodes (the instance's parameters)."""
    sub = expression(material, unreal.MaterialExpressionSubtract, x, y)
    mul = expression(material, unreal.MaterialExpressionDivide, x + 150, y)
    connect(low, sub, "B")
    connect(width, mul, "B")
    sat = expression(material, unreal.MaterialExpressionSaturate, x + 300, y)
    square = expression(material, unreal.MaterialExpressionMultiply, x + 450, y - 60)
    double = expression(material, unreal.MaterialExpressionMultiply, x + 450, y + 60, const_b=2.0)
    triple = expression(material, unreal.MaterialExpressionSubtract, x + 600, y + 60, const_a=3.0)
    result = expression(material, unreal.MaterialExpressionMultiply, x + 750, y)
    connect(source, sub)
    connect(sub, mul)
    connect(mul, sat)
    connect(sat, square, "A")
    connect(sat, square, "B")
    connect(sat, double)
    connect(double, triple, "B")
    connect(square, result, "A")
    connect(triple, result, "B")
    return result


def tile_line(material, x, y):
    """1 on the tile grid's lines, 0 elsewhere (as M_MatteGround: a line every tile, through the world's origin)."""
    position = expression(material, unreal.MaterialExpressionWorldPosition, x, y)
    plane = expression(material, unreal.MaterialExpressionComponentMask, x + 150, y, r=True, g=True, b=False, a=False)
    cells = expression(material, unreal.MaterialExpressionDivide, x + 300, y, const_b=GRID_CELL)
    frac = expression(material, unreal.MaterialExpressionFrac, x + 450, y)
    centred = expression(material, unreal.MaterialExpressionSubtract, x + 600, y, const_b=0.5)
    distance = expression(material, unreal.MaterialExpressionAbs, x + 750, y)
    edge = expression(material, unreal.MaterialExpressionSubtract, x + 900, y, const_b=0.5 - GRID_LINE)
    sharp = expression(material, unreal.MaterialExpressionMultiply, x + 1050, y, const_b=1.0 / GRID_LINE)
    line = expression(material, unreal.MaterialExpressionSaturate, x + 1200, y)
    line_x = expression(material, unreal.MaterialExpressionComponentMask, x + 1350, y - 60, r=True, g=False, b=False, a=False)
    line_y = expression(material, unreal.MaterialExpressionComponentMask, x + 1350, y + 60, r=False, g=True, b=False, a=False)
    either = expression(material, unreal.MaterialExpressionMax, x + 1500, y)
    chain = [position, plane, cells, frac, centred, distance, edge, sharp, line]
    for source, target in zip(chain, chain[1:]):
        connect(source, target)
    connect(line, line_x)
    connect(line, line_y)
    connect(line_x, either, "A")
    connect(line_y, either, "B")
    return either


def import_textures():
    """Imports (again when the file changed) the detail PNGs of Tools/Data/GroundSources as linear textures with mipmaps."""
    for name in ground.TEXTURES:
        source = os.path.join(SOURCES, name + "_detail.png")
        if not os.path.isfile(source):
            raise RuntimeError("Missing %s (run Tools/prepare_ground_details.py)" % source)
        folder = ground.TEXTURE_FOLDER
        asset = ground.texture_name(name)
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", source)
        task.set_editor_property("destination_path", folder)
        task.set_editor_property("destination_name", asset)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("automated", True)
        task.set_editor_property("save", False)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        texture = unreal.load_asset(folder + "/" + asset)
        if texture is None:
            raise RuntimeError("Could not import " + source)
        texture.set_editor_property("srgb", False)  # a variation around 0.5 in linear light, not a colour
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DEFAULT)
        texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP)
        texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_WORLD)
        texture.set_editor_property("filter", unreal.TextureFilter.TF_DEFAULT)  # trilinear, anisotropy by the project's r.MaxAnisotropy
        texture.set_editor_property("address_x", unreal.TextureAddress.TA_WRAP)
        texture.set_editor_property("address_y", unreal.TextureAddress.TA_WRAP)
        texture.set_editor_property("never_stream", False)
        assets.save_loaded_asset(texture)
        unreal.log("Texture %s imported from %s" % (texture.get_path_name(), source))


def detail(material, base, x, y):
    """base times the two detail samples (each lerp(1, 2 * sample, strength), the mean 1): returns the node. The sampler is a
    parameter (GroundTexture) so that each instance names its own texture."""
    default = unreal.load_asset(ground.TEXTURE_FOLDER + "/" + ground.texture_name(next(iter(ground.TEXTURES))))
    texture_scale = expression(material, unreal.MaterialExpressionScalarParameter, x, y + 400, parameter_name="TextureScale", default_value=800.0)
    strength = expression(material, unreal.MaterialExpressionScalarParameter, x, y + 500, parameter_name="DetailStrength", default_value=1.0)
    position = expression(material, unreal.MaterialExpressionWorldPosition, x, y)
    plane = expression(material, unreal.MaterialExpressionComponentMask, x + 150, y, r=True, g=True, b=False, a=False)
    connect(position, plane)
    result = base
    for index, (ratio, offset) in enumerate(((1.0, 0.0), (DETAIL_RATIO, DETAIL_OFFSET))):
        row = y + index * 200
        shifted = expression(material, unreal.MaterialExpressionAdd, x + 300, row, const_b=offset)
        divide = expression(material, unreal.MaterialExpressionDivide, x + 450, row)
        scaled = expression(material, unreal.MaterialExpressionMultiply, x + 450, row + 80, const_b=ratio)
        connect(plane, shifted)
        connect(shifted, divide, "A")
        connect(texture_scale, scaled, "A")
        connect(scaled, divide, "B")
        sample = expression(material, unreal.MaterialExpressionTextureSampleParameter2D, x + 650, row, parameter_name="GroundTexture",
                            texture=default, sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
        connect(divide, sample, "UVs")
        doubled = expression(material, unreal.MaterialExpressionMultiply, x + 850, row, const_b=2.0)
        connect(sample, doubled, "A")
        mixed = expression(material, unreal.MaterialExpressionLinearInterpolate, x + 1000, row, const_a=1.0)
        connect(doubled, mixed, "B")
        connect(strength, mixed, "Alpha")
        times = expression(material, unreal.MaterialExpressionMultiply, x + 1150, row)
        connect(result, times, "A")
        connect(mixed, times, "B")
        result = times
    return result


def make_varied():
    path = ground.FOLDER + "/" + ground.PARENT
    if assets.does_asset_exist(path):
        unreal.log("Material %s exists, kept (delete it to make it again)" % path)
        return
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(ground.PARENT, ground.FOLDER, unreal.Material, unreal.MaterialFactoryNew())
    if material is None:
        raise RuntimeError("Could not create " + path)
    color_a = expression(material, unreal.MaterialExpressionVectorParameter, 0, -500, parameter_name="ColorA",
                         default_value=unreal.LinearColor(0.16, 0.16, 0.15, 1.0))
    color_b = expression(material, unreal.MaterialExpressionVectorParameter, 0, -380, parameter_name="ColorB",
                         default_value=unreal.LinearColor(0.2, 0.18, 0.12, 1.0))
    color_c = expression(material, unreal.MaterialExpressionVectorParameter, 0, -260, parameter_name="ColorC",
                         default_value=unreal.LinearColor(0.12, 0.11, 0.1, 1.0))
    scale = expression(material, unreal.MaterialExpressionScalarParameter, -2200, 250, parameter_name="NoiseScale", default_value=3000.0)
    wider = expression(material, unreal.MaterialExpressionMultiply, -2050, 650, const_b=SECOND_SCALE)
    connect(scale, wider, "A")
    first = noise(material, 0, scale, 0.0)
    second = noise(material, 500, wider, SECOND_OFFSET)
    b_from = expression(material, unreal.MaterialExpressionScalarParameter, -1500, -300, parameter_name="BFrom", default_value=B_FROM)
    b_width = expression(material, unreal.MaterialExpressionScalarParameter, -1500, -200, parameter_name="BWidth", default_value=B_WIDTH)
    c_from = expression(material, unreal.MaterialExpressionScalarParameter, -1500, 800, parameter_name="CFrom", default_value=C_FROM)
    c_width = expression(material, unreal.MaterialExpressionScalarParameter, -1500, 900, parameter_name="CWidth", default_value=C_WIDTH)
    to_b = ramp(material, first, b_from, b_width, -1200, 0)
    to_c = ramp(material, second, c_from, c_width, -1200, 500)
    patch_b = expression(material, unreal.MaterialExpressionLinearInterpolate, 400, -400)
    connect(color_a, patch_b, "A")
    connect(color_b, patch_b, "B")
    connect(to_b, patch_b, "Alpha")
    patch_c = expression(material, unreal.MaterialExpressionLinearInterpolate, 600, -400)
    connect(patch_b, patch_c, "A")
    connect(color_c, patch_c, "B")
    connect(to_c, patch_c, "Alpha")
    # The detail texture (two samples by world coordinates), then the line: the ground's colour, darker, on the tile grid.
    textured = detail(material, patch_c, 700, -1400)
    darker = expression(material, unreal.MaterialExpressionMultiply, 800, -500, const_b=LINE_DARKEN)
    connect(textured, darker, "A")
    lines = tile_line(material, -1800, 1100)
    final = expression(material, unreal.MaterialExpressionLinearInterpolate, 1000, -400)
    connect(textured, final, "A")
    connect(darker, final, "B")
    connect(lines, final, "Alpha")
    lib.connect_material_property(final, "", unreal.MaterialProperty.MP_BASE_COLOR)
    # Matte (D265): no specular, full roughness.
    specular = expression(material, unreal.MaterialExpressionConstant, 1000, 200, r=0.0)
    lib.connect_material_property(specular, "", unreal.MaterialProperty.MP_SPECULAR)
    roughness = expression(material, unreal.MaterialExpressionConstant, 1000, 300, r=1.0)
    lib.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    lib.recompile_material(material)
    assets.save_loaded_asset(material)
    unreal.log("Material %s made" % material.get_path_name())


try:
    import_textures()
    make_varied()
    for name in ground.GROUNDS:
        unreal.log("Ground instance %s set" % ground.instance(name).get_path_name())
except Exception as error:
    unreal.log_error("Ground materials: %s" % error)
unreal.SystemLibrary.quit_editor()
