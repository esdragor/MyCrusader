"""Creates the material the fog is drawn with (F7, D562, D381): /Game/Rendering/M_FogPost, a post process that multiplies the scene
colour by the fog texture, read as a cubic B-spline so the edges are round (UHOLFogRenderSubsystem: one byte a cell, 0 unexplored, ~0.45 explored, 1 in sight) at the world position
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


# F7.3 (the user's trial: "the edges are very rough"): the grid has one cell for 4 m, so a bilinear read shows the cells' staircase
# under a short blur. A cubic B-spline read (four bilinear taps, Sigg and Hadwiger) rounds the staircase into smooth curves.
SMOOTH_FOG = """float w; float h; Tex.GetDimensions(w, h);
float2 size = float2(w, h);
float2 coord = UV * size - 0.5;
float2 base = floor(coord);
float2 f = coord - base;
float2 f2 = f * f;
float2 f3 = f2 * f;
float2 w0 = (1.0 - 3.0 * f + 3.0 * f2 - f3) / 6.0;
float2 w1 = (4.0 - 6.0 * f2 + 3.0 * f3) / 6.0;
float2 w2 = (1.0 + 3.0 * f + 3.0 * f2 - 3.0 * f3) / 6.0;
float2 w3 = f3 / 6.0;
float2 g0 = w0 + w1;
float2 g1 = w2 + w3;
float2 p0 = (base - 1.0 + w1 / g0 + 0.5) / size;
float2 p1 = (base + 1.0 + w3 / g1 + 0.5) / size;
float a = Tex.SampleLevel(TexSampler, float2(p0.x, p0.y), 0).r;
float b = Tex.SampleLevel(TexSampler, float2(p1.x, p0.y), 0).r;
float c = Tex.SampleLevel(TexSampler, float2(p0.x, p1.y), 0).r;
float d = Tex.SampleLevel(TexSampler, float2(p1.x, p1.y), 0).r;
return saturate((a * g0.x + b * g1.x) * g0.y + (c * g0.x + d * g1.x) * g1.y);"""


def fog_value(material, world_xy):
    """The texture's value (0 to 1) under a world position (X, Y), read smoothly."""
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
    texture = expression(material, unreal.MaterialExpressionTextureObjectParameter, -900, 300, parameter_name="FogTexture",
                         texture=default_texture)
    smooth = expression(material, unreal.MaterialExpressionCustom, -600, 100, code=SMOOTH_FOG, description="SmoothFog",
                        output_type=unreal.CustomMaterialOutputType.CMOT_FLOAT1)
    smooth.set_editor_property("inputs", [custom_input("UV"), custom_input("Tex")])
    connect(uv, smooth, "UV")
    connect(texture, smooth, "Tex")
    return smooth


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
    value = fog_value(material, xy)
    # What the player must see through the fog (the order lines and their points) writes the custom stencil: no fog there.
    stencil = expression(material, unreal.MaterialExpressionSceneTexture, -1900, -450, scene_texture_id=unreal.SceneTextureId.PPI_CUSTOM_STENCIL)
    stencil_value = expression(material, unreal.MaterialExpressionComponentMask, -1700, -450, r=True, g=False, b=False, a=False)
    connect(stencil, stencil_value, "", "Color")
    marked = expression(material, unreal.MaterialExpressionCustom, -1400, -450, code="return step(0.5, S);", description="Marked",
                        output_type=unreal.CustomMaterialOutputType.CMOT_FLOAT1)
    marked.set_editor_property("inputs", [custom_input("S")])
    connect(stencil_value, marked, "S")
    clear = expression(material, unreal.MaterialExpressionLinearInterpolate, -400, 0)
    connect(value, clear, "A")
    connect(expression(material, unreal.MaterialExpressionConstant, -600, 100, r=1.0), clear, "B")
    connect(marked, clear, "Alpha")
    color = expression(material, unreal.MaterialExpressionMultiply, -200, -100)
    connect(scene, color, "A", "Color")
    connect(clear, color, "B")
    lib.connect_material_property(color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    lib.recompile_material(material)
    assets.save_loaded_asset(material)
    unreal.log("Material %s made" % material.get_path_name())


make_post()
