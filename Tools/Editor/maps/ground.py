"""The varied ground of the maps (chantier Cartes, C1b; M-Q5: varied ground for every map). The parent material
/Game/Rendering/M_GroundVaried is made by Tools/Editor/make_ground_materials.py (three colours blended by a world noise,
the faint tile line of the building grid kept); each map that wants its own ground has an instance of it, set here, and names
it in its module (steppe.GROUND = "MI_Ground_Steppe"). A module without GROUND keeps the grey M_MatteGround (Plain). The
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


# name -> (ColorA the main ground, ColorB the wide patches, ColorC the rarer ones, NoiseScale in cm: the size of a patch).
# The Steppe: yellow-green grass, dry straw, ochre earth; muted (linear values; M_MatteGround is a grey of 0.16).
GROUNDS = {
    "MI_Ground_Steppe": (color(0.115, 0.145, 0.055), color(0.190, 0.170, 0.090), color(0.165, 0.120, 0.070), 4000.0),
}


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
    lib.set_material_instance_scalar_parameter_value(material, "NoiseScale", colors_and_scale[3])
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material
