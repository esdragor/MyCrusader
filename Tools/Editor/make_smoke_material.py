"""Creates /Game/Units/Props/M_PowderSmoke, the translucent material of the Arquebusier's powder smoke (SM_PowderSmoke,
Tools/Editor/make_arrow_mesh.py --smoke): unlit, blended, a pale grey-white whose opacity is the per-instance custom data 0
(UHOLArrowSubsystem::GetSmokeOpacity: SmokeOpacity falling to 0 over the puff's life) times a soft edge (one minus a Fresnel
term: the centre of the ball opaque, its rim see-through), so the ball reads as a cloud, not a white ball.
Run (no window, editor closed):
  UnrealEditor-Cmd.exe <project> -run=pythonscript -script="<this file>" -unattended -nullrhi -nosplash -nosound
Kept as it is when it already exists (delete it to make it again).
"""
import unreal

FOLDER = "/Game/Units/Props"
NAME = "M_PowderSmoke"
COLOR = (0.86, 0.85, 0.81)
EDGE_EXPONENT = 2.0  # the Fresnel exponent: lower = a wider soft rim

assets = unreal.EditorAssetLibrary
lib = unreal.MaterialEditingLibrary
path = FOLDER + "/" + NAME
if assets.does_asset_exist(path):
    unreal.log("Material %s exists, kept (delete it to make it again)" % path)
else:
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(NAME, FOLDER, unreal.Material, unreal.MaterialFactoryNew())
    if material is None:
        raise RuntimeError("Could not create " + path)
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    material.set_editor_property("used_with_instanced_static_meshes", True)

    def node(expression_class, x, y, **properties):
        n = lib.create_material_expression(material, expression_class, x, y)
        for key, value in properties.items():
            n.set_editor_property(key, value)
        return n

    def connect(source, target, pin=""):
        if not lib.connect_material_expressions(source, "", target, pin):
            raise RuntimeError("Could not connect %s to %s" % (source.get_name(), target.get_name()))

    colour = node(unreal.MaterialExpressionConstant3Vector, -500, -150, constant=unreal.LinearColor(COLOR[0], COLOR[1], COLOR[2], 1.0))
    lib.connect_material_property(colour, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    opacity = node(unreal.MaterialExpressionPerInstanceCustomData, -700, 150, data_index=0, const_default_value=0.5)
    fresnel = node(unreal.MaterialExpressionFresnel, -900, 350, exponent=EDGE_EXPONENT)
    soft_edge = node(unreal.MaterialExpressionOneMinus, -700, 350)
    connect(fresnel, soft_edge)
    product = node(unreal.MaterialExpressionMultiply, -450, 250)
    connect(opacity, product, "A")
    connect(soft_edge, product, "B")
    lib.connect_material_property(product, "", unreal.MaterialProperty.MP_OPACITY)

    lib.recompile_material(material)
    assets.save_loaded_asset(material)
    unreal.log("Material %s made" % material.get_path_name())
