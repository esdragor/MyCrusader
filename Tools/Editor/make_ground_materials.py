"""Creates the varied ground of the maps (chantier Cartes, C1b; M-Q5: varied ground for every map):
  /Game/Rendering/M_GroundVaried    parent: three colours (ColorA, ColorB, ColorC) and a NoiseScale (cm), blended by a world
                                    noise of two octaves so that the ground shows patches: ColorA is the main ground, ColorB
                                    fills wide soft patches, ColorC rarer ones. The same matte look as M_MatteGround (D265: no
                                    specular, full roughness) and the same faint line on the building grid's tiles (4 m, through
                                    the world's origin, D344): the line is the ground's own colour, darker, so it reads on
                                    every patch and the camera's motion still shows (D265).
  /Game/Rendering/MI_Ground_<Map>   one instance per map that wants its own ground (maps/ground.py GROUNDS; the recipes make
                                    them too when missing, so a map file never lacks its instance).
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
    # The line: the patch's colour, darker, on the tile grid.
    darker = expression(material, unreal.MaterialExpressionMultiply, 800, -500, const_b=LINE_DARKEN)
    connect(patch_c, darker, "A")
    lines = tile_line(material, -1800, 1100)
    final = expression(material, unreal.MaterialExpressionLinearInterpolate, 1000, -400)
    connect(patch_c, final, "A")
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
    make_varied()
    for name in ground.GROUNDS:
        unreal.log("Ground instance %s set" % ground.instance(name).get_path_name())
except Exception as error:
    unreal.log_error("Ground materials: %s" % error)
unreal.SystemLibrary.quit_editor()
