"""Shared writer of the art assets' fields (AR0-d2 3b, D925): an editor script that makes a model writes it into
/Game/Art/<Units|Buildings|Sources>/<Type>/DA_Art_<Type> (UHOLArtVisual), never into a column of a type CSV.

set_art_field(family, type_name, field, value): loads (or creates, Status Placeholder) the type's DA_Art, sets the field
(Mesh / SkinnedMesh / AnimationProvider = the loaded asset; Projectile / MuzzleSmoke = a soft object path), saves it and
logs "art_visual_io: <asset>.<field> = <value>". SKINNED_TYPES: the types drawn by each skinned mesh (their
SkinnedMesh and AnimationProvider). Used by make_arrow_mesh.py and make_peasant_mesh.py.
Test (no editor): Tools/Tests/art_visual_io_test.ps1.
"""
import unreal

ART_ROOT = "/Game/Art"
ASSET_FIELDS = ("Mesh", "SkinnedMesh", "AnimationProvider")  # a soft object pointer: the loaded asset
PATH_FIELDS = ("Projectile", "MuzzleSmoke")  # a soft object path
# The types drawn by each skinned mesh of make_peasant_mesh.py (the art at the migration, D925); a new type that shares
# a mesh is added here.
SKINNED_TYPES = {
    "SKM_Peasant": ("Peasant", "Zombie"),
    "SKM_Spearman": ("Spearman", "Halberdier", "TempleSergeant"),
    "SKM_ManAtArms": ("ManAtArms", "DamnedWarrior", "VirtuousKnight"),
    "SKM_Archer": ("Archer",),
    "SKM_King": ("King",),
    "SKM_Crossbowman": ("Crossbowman", "PaviseCrossbowman"),
    "SKM_Arquebusier": ("Arquebusier",),
    "SKM_Hero": ("Hero",),
}


def snake(name):
    """The Python name of a property: SkinnedMesh -> skinned_mesh."""
    out = ""
    for index, char in enumerate(name):
        if char.isupper() and index > 0:
            out += "_"
        out += char.lower()
    return out


def art_path(family, type_name):
    return "%s/%s/%s/DA_Art_%s" % (ART_ROOT, family, type_name, type_name)


def load_or_create(family, type_name):
    path = art_path(family, type_name)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        asset = unreal.load_asset(path)
        if asset is None or not isinstance(asset, unreal.HOLArtVisual):
            raise RuntimeError(path + " exists but is not a HOLArtVisual")
        return asset, path
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.HOLArtVisual)
    folder, name = path.rsplit("/", 1)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, folder, unreal.HOLArtVisual, factory)
    if asset is None:
        raise RuntimeError("Could not create " + path)
    asset.set_editor_property("type_name", unreal.Name(type_name))
    asset.set_editor_property("status", unreal.HOLArtStatus.PLACEHOLDER)
    return asset, path


def set_art_field(family, type_name, field, value):
    """Sets DA_Art_<type_name>.<field> to value (an asset path) and saves the asset."""
    asset, path = load_or_create(family, type_name)
    if field in ASSET_FIELDS:
        loaded = unreal.load_asset(value)
        if loaded is None:
            raise RuntimeError("%s: %s %s does not load" % (path, field, value))
        asset.set_editor_property(snake(field), loaded)
    elif field in PATH_FIELDS:
        soft = unreal.SoftObjectPath()
        soft.import_text(value)
        if str(soft.export_text()).strip('"') != value:
            raise RuntimeError("%s: %s %s read back as %s" % (path, field, value, soft.export_text()))
        asset.set_editor_property(snake(field), soft)
    else:
        raise RuntimeError("art_visual_io: unknown field " + field)
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, False):
        raise RuntimeError("Could not save " + path)
    unreal.log("art_visual_io: %s.%s = %s (saved)" % (path, field, value))

