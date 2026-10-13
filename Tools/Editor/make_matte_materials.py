"""Creates the matte materials of the game (D265: no specular, full roughness, nothing metallic):
  /Game/Rendering/M_MatteColor    one colour, parameter "Color" (buildings, D261: written per piece by AHOLBuilding)
  /Game/Rendering/M_FlatColor     one colour, unlit: exactly the colour on every face (the owner's colour, D261)
  /Game/Rendering/M_MatteGround   grey ground with a line every 10 m, aligned on the world (the camera's movement shows)
Only the missing ones are made: delete a material's .uasset to make it again. Editing one in place crashes the editor
(DeleteAllMaterialExpressions, "!IsRooted()"): AHOLBuilding references them from C++, so they load with the editor.
Run (the editor quits when done):
  UnrealEditor.exe <project> -ExecutePythonScript=<this file>
"""
import unreal

FOLDER = "/Game/Rendering"
GRID_CELL = 1000.0  # cm
GRID_LINE = 0.015  # fraction of a cell
GROUND_COLOR = unreal.LinearColor(0.16, 0.16, 0.15, 1.0)
LINE_COLOR = unreal.LinearColor(0.09, 0.09, 0.09, 1.0)

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
assets = unreal.EditorAssetLibrary
lib = unreal.MaterialEditingLibrary


def new_material(name):
    """A new material, or None if it already exists (kept as it is)."""
    path = FOLDER + "/" + name
    if assets.does_asset_exist(path):
        unreal.log("Material %s exists, kept (delete it to make it again)" % path)
        return None
    material = asset_tools.create_asset(name, FOLDER, unreal.Material, unreal.MaterialFactoryNew())
    if material is None:
        raise RuntimeError("Could not create " + path)
    return material


def expression(material, expression_class, x, y, **properties):
    node = lib.create_material_expression(material, expression_class, x, y)
    for key, value in properties.items():
        node.set_editor_property(key, value)
    return node


def make_matte(material):
    """No specular highlight, full roughness (D265); metallic stays 0."""
    specular = expression(material, unreal.MaterialExpressionConstant, -300, 200, r=0.0)
    lib.connect_material_property(specular, "", unreal.MaterialProperty.MP_SPECULAR)
    roughness = expression(material, unreal.MaterialExpressionConstant, -300, 300, r=1.0)
    lib.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)


def finish(material):
    lib.recompile_material(material)
    assets.save_loaded_asset(material)
    unreal.log("Material %s made" % material.get_path_name())


def make_color():
    material = new_material("M_MatteColor")
    if material is None:
        return
    color = expression(material, unreal.MaterialExpressionVectorParameter, -300, 0, parameter_name="Color",
                       default_value=unreal.LinearColor(0.5, 0.5, 0.5, 1.0))
    lib.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
    make_matte(material)
    finish(material)


def make_flat():
    """Unlit: the parameter "Color" is the colour shown, with no darker side (the owner's colour, D261)."""
    material = new_material("M_FlatColor")
    if material is None:
        return
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    color = expression(material, unreal.MaterialExpressionVectorParameter, -300, 0, parameter_name="Color",
                       default_value=unreal.LinearColor(0.5, 0.5, 0.5, 1.0))
    lib.connect_material_property(color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    finish(material)


def make_ground():
    material = new_material("M_MatteGround")
    if material is None:
        return
    # Distance to the middle of the cell, per axis: a line where it nears half a cell.
    position = expression(material, unreal.MaterialExpressionWorldPosition, -1600, 0)
    plane = expression(material, unreal.MaterialExpressionComponentMask, -1450, 0, r=True, g=True, b=False, a=False)
    cells = expression(material, unreal.MaterialExpressionDivide, -1300, 0, const_b=GRID_CELL)
    frac = expression(material, unreal.MaterialExpressionFrac, -1150, 0)
    centred = expression(material, unreal.MaterialExpressionSubtract, -1000, 0, const_b=0.5)
    distance = expression(material, unreal.MaterialExpressionAbs, -850, 0)
    edge = expression(material, unreal.MaterialExpressionSubtract, -700, 0, const_b=0.5 - GRID_LINE)
    sharp = expression(material, unreal.MaterialExpressionMultiply, -550, 0, const_b=1.0 / GRID_LINE)
    line = expression(material, unreal.MaterialExpressionSaturate, -400, 0)
    line_x = expression(material, unreal.MaterialExpressionComponentMask, -250, -60, r=True, g=False, b=False, a=False)
    line_y = expression(material, unreal.MaterialExpressionComponentMask, -250, 60, r=False, g=True, b=False, a=False)
    either = expression(material, unreal.MaterialExpressionMax, -100, 0)
    ground = expression(material, unreal.MaterialExpressionConstant3Vector, -100, -200, constant=GROUND_COLOR)
    lines = expression(material, unreal.MaterialExpressionConstant3Vector, -100, -320, constant=LINE_COLOR)
    mix = expression(material, unreal.MaterialExpressionLinearInterpolate, 100, -100)

    chain = [position, plane, cells, frac, centred, distance, edge, sharp, line]
    for source, target in zip(chain, chain[1:]):
        if not lib.connect_material_expressions(source, "", target, ""):
            raise RuntimeError("Could not connect the ground grid")
    for source, target, pin in ((line, line_x, ""), (line, line_y, ""), (line_x, either, "A"), (line_y, either, "B"),
                                (ground, mix, "A"), (lines, mix, "B"), (either, mix, "Alpha")):
        if not lib.connect_material_expressions(source, "", target, pin):
            raise RuntimeError("Could not connect the ground grid")
    lib.connect_material_property(mix, "", unreal.MaterialProperty.MP_BASE_COLOR)
    make_matte(material)
    finish(material)


make_color()
make_flat()
make_ground()
