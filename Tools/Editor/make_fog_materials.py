"""Creates the material the fog is drawn with (F7, D562, D381): /Game/Rendering/M_FogPost, a post process that multiplies the scene
colour by the fog texture (UHOLFogRenderSubsystem: one byte a cell, 0 unexplored, ~0.45 explored, 1 in sight) at the world position
of each pixel, rebuilt from the scene depth and the camera the C++ side sends (CamPos, CamFwd, CamRight, CamUp, CamTan).
Parameters: FogRect (x, y: the grid's lower corner in cm; z: its side in cm), FogTexture.
Only the missing ones are made: delete a material's .uasset to make it again.
Run (the editor quits when done):
  UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file> -unattended -nop4 -nosplash
"""
import unreal

FOLDER = "/Game/Rendering"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
assets = unreal.EditorAssetLibrary
lib = unreal.MaterialEditingLibrary


def new_material(name):
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


def connect(source, target, pin="", source_pin=""):
    if not lib.connect_material_expressions(source, source_pin, target, pin):
        raise RuntimeError("Could not connect %s to %s (%s)" % (source.get_name(), target.get_name(), pin))


def fog_value(material, world_xy):
    """The texture's value (0 to 1) under a world position (X, Y)."""
    rect = expression(material, unreal.MaterialExpressionVectorParameter, -1500, 300, parameter_name="FogRect",
                      default_value=unreal.LinearColor(-44000.0, -44000.0, 88000.0, 0.0))
    origin = expression(material, unreal.MaterialExpressionComponentMask, -1300, 250, r=True, g=True, b=False, a=False)
    connect(rect, origin)
    side = expression(material, unreal.MaterialExpressionComponentMask, -1300, 400, r=False, g=False, b=True, a=False)
    connect(rect, side)
    from_corner = expression(material, unreal.MaterialExpressionSubtract, -1100, 100)
    connect(world_xy, from_corner, "A")
    connect(origin, from_corner, "B")
    uv = expression(material, unreal.MaterialExpressionDivide, -900, 100)
    connect(from_corner, uv, "A")
    connect(side, uv, "B")
    default_texture = unreal.load_asset("/Engine/EngineResources/WhiteSquareTexture")
    texture = expression(material, unreal.MaterialExpressionTextureSampleParameter2D, -700, 100, parameter_name="FogTexture",
                         texture=default_texture)
    connect(uv, texture, "UVs")
    return expression(material, unreal.MaterialExpressionComponentMask, -450, 100, r=True, g=False, b=False, a=False), texture


WORLD_FROM_DEPTH = """float2 ndc = float2(UV.x * 2.0 - 1.0, 1.0 - UV.y * 2.0);
float3 dir = Fwd.xyz + Right.xyz * (ndc.x * Tan.x) + Up.xyz * (ndc.y * Tan.y);
return Pos.xyz + dir * Depth;"""


def custom_input(name):
    item = unreal.CustomInput()
    item.set_editor_property("input_name", name)
    return item


def make_post():
    material = new_material("M_FogPost")
    if material is None:
        return
    material.set_editor_property("material_domain", unreal.MaterialDomain.MD_POST_PROCESS)
    material.set_editor_property("blendable_location", unreal.BlendableLocation.BL_SCENE_COLOR_AFTER_TONEMAPPING)
    scene = expression(material, unreal.MaterialExpressionSceneTexture, -1900, -200, scene_texture_id=unreal.SceneTextureId.PPI_POST_PROCESS_INPUT0)
    depth = expression(material, unreal.MaterialExpressionSceneTexture, -1900, 100, scene_texture_id=unreal.SceneTextureId.PPI_SCENE_DEPTH)
    depth_value = expression(material, unreal.MaterialExpressionComponentMask, -1700, 100, r=True, g=False, b=False, a=False)
    connect(depth, depth_value, "", "Color")
    uv = expression(material, unreal.MaterialExpressionTextureCoordinate, -1900, 300)
    params = {}
    for index, name in enumerate(("CamPos", "CamFwd", "CamRight", "CamUp", "CamTan")):
        params[name] = expression(material, unreal.MaterialExpressionVectorParameter, -1900, 450 + 130 * index, parameter_name=name,
                                  default_value=unreal.LinearColor(0.0, 0.0, 0.0, 0.0))
    world = expression(material, unreal.MaterialExpressionCustom, -1500, 300, code=WORLD_FROM_DEPTH, description="WorldFromDepth",
                       output_type=unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    world.set_editor_property("inputs", [custom_input(n) for n in ("UV", "Depth", "Pos", "Fwd", "Right", "Up", "Tan")])
    connect(uv, world, "UV")
    connect(depth_value, world, "Depth")
    for pin, name in (("Pos", "CamPos"), ("Fwd", "CamFwd"), ("Right", "CamRight"), ("Up", "CamUp"), ("Tan", "CamTan")):
        connect(params[name], world, pin)
    xy = expression(material, unreal.MaterialExpressionComponentMask, -1300, 300, r=True, g=True, b=False, a=False)
    connect(world, xy)
    value, texture = fog_value(material, xy)
    connect(texture, value)
    color = expression(material, unreal.MaterialExpressionMultiply, -200, -100)
    connect(scene, color, "A", "Color")
    connect(value, color, "B")
    lib.connect_material_property(color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    lib.recompile_material(material)
    assets.save_loaded_asset(material)
    unreal.log("Material %s made" % material.get_path_name())


make_post()
