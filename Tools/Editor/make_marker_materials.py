"""Creates the materials of the ground markers of the selection (J4, D288), unlit and masked, drawn on the engine's
1 m plane (/Engine/BasicShapes/Plane) by instanced meshes (AHOLSelectionMarkers):
  /Game/Rendering/M_GroundRing    a ring touching the plane's edges (selected units, the order marker)
  /Game/Rendering/M_GroundFrame   a square frame along the plane's edges (the selected building)
Per-instance custom data: 0-2 the colour (linear), 3 the stroke as a share of the half-size (0 to 1).
Only the missing ones are made: delete a material's .uasset to make it again.
Run (the editor quits when done):
  UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file> -unattended -nop4 -nosplash
"""
import unreal

FOLDER = "/Game/Rendering"
SHARPNESS = 400.0  # how fast the mask goes from 0 to 1 at the stroke's edges

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


def connect(source, target, pin=""):
    if not lib.connect_material_expressions(source, "", target, pin):
        raise RuntimeError("Could not connect %s to %s" % (source.get_name(), target.get_name()))


def make_marker(name, distance_to_edge):
    """Unlit, masked: the colour where the distance to the centre (0 at the centre, 1 at the edge) is within the stroke."""
    material = new_material(name)
    if material is None:
        return
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_MASKED)
    material.set_editor_property("used_with_instanced_static_meshes", True)

    color = expression(material, unreal.MaterialExpressionPerInstanceCustomData3Vector, -400, -200, data_index=0,
                       const_default_value=unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    lib.connect_material_property(color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    distance = distance_to_edge(material)
    stroke = expression(material, unreal.MaterialExpressionPerInstanceCustomData, -1000, 300, data_index=3, const_default_value=0.15)
    inner = expression(material, unreal.MaterialExpressionOneMinus, -850, 300)
    connect(stroke, inner)
    # Past the inner edge...
    past_inner = expression(material, unreal.MaterialExpressionSubtract, -700, 150)
    connect(distance, past_inner, "A")
    connect(inner, past_inner, "B")
    sharp_inner = expression(material, unreal.MaterialExpressionMultiply, -550, 150, const_b=SHARPNESS)
    connect(past_inner, sharp_inner, "A")
    in_inner = expression(material, unreal.MaterialExpressionSaturate, -400, 150)
    connect(sharp_inner, in_inner)
    # ...and before the outer one.
    before_outer = expression(material, unreal.MaterialExpressionOneMinus, -700, 400)
    connect(distance, before_outer)
    sharp_outer = expression(material, unreal.MaterialExpressionMultiply, -550, 400, const_b=SHARPNESS)
    connect(before_outer, sharp_outer, "A")
    in_outer = expression(material, unreal.MaterialExpressionSaturate, -400, 400)
    connect(sharp_outer, in_outer)
    mask = expression(material, unreal.MaterialExpressionMultiply, -250, 250)
    connect(in_inner, mask, "A")
    connect(in_outer, mask, "B")
    lib.connect_material_property(mask, "", unreal.MaterialProperty.MP_OPACITY_MASK)

    lib.recompile_material(material)
    assets.save_loaded_asset(material)
    unreal.log("Material %s made" % material.get_path_name())


def ring_distance(material):
    """Distance from the plane's centre, 1 at the middle of an edge."""
    uv = expression(material, unreal.MaterialExpressionTextureCoordinate, -1600, 0)
    centre = expression(material, unreal.MaterialExpressionConstant2Vector, -1600, 100, r=0.5, g=0.5)
    distance = expression(material, unreal.MaterialExpressionDistance, -1450, 0)
    connect(uv, distance, "A")
    connect(centre, distance, "B")
    return expression_times_two(material, distance)


def frame_distance(material):
    """The larger of the two distances to the centre lines, 1 on every edge."""
    uv = expression(material, unreal.MaterialExpressionTextureCoordinate, -1600, 0)
    centred = expression(material, unreal.MaterialExpressionSubtract, -1450, 0, const_b=0.5)
    connect(uv, centred, "A")
    away = expression(material, unreal.MaterialExpressionAbs, -1300, 0)
    connect(centred, away)
    along_u = expression(material, unreal.MaterialExpressionComponentMask, -1150, -60, r=True, g=False, b=False, a=False)
    along_v = expression(material, unreal.MaterialExpressionComponentMask, -1150, 60, r=False, g=True, b=False, a=False)
    connect(away, along_u)
    connect(away, along_v)
    larger = expression(material, unreal.MaterialExpressionMax, -1000, 0)
    connect(along_u, larger, "A")
    connect(along_v, larger, "B")
    return expression_times_two(material, larger)


def expression_times_two(material, source):
    doubled = expression(material, unreal.MaterialExpressionMultiply, -1000, 0, const_b=2.0)
    connect(source, doubled, "A")
    return doubled


make_marker("M_GroundRing", ring_distance)
make_marker("M_GroundFrame", frame_distance)
