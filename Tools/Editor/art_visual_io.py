"""Shared writer of the art assets' fields (AR0-d2 3b, D925; AR3-a/b): editor scripts write the art of a type into
/Game/Art/<Units|Buildings|Sources>/<Type>/DA_Art_<Type> (UHOLArtVisual), never into a column of a type CSV.

1. set_art_field(family, type_name, field, value): loads (or creates, Status Placeholder) the type's DA_Art, sets the
   field (Mesh / SkinnedMesh / AnimationProvider = the loaded asset; Projectile / MuzzleSmoke = a soft object path),
   saves it and logs "art_visual_io: <asset>.<field> = <value>". SKINNED_TYPES: the types drawn by each skinned mesh
   (their SkinnedMesh and AnimationProvider). Used by make_arrow_mesh.py and make_peasant_mesh.py (import: no side
   effect).
2. write_animation_references() (AR3-a): for every DA_Art under /Game/Art/Units with an AnimationProvider (TP_*), each
   state whose number the asset sets (Idle, Walk, Attack, Fall, Build, and by resource Gather / CarryWalk / CarryIdle)
   gets its reference: the UAnimSequence at that number in GetSequences() of the provider. The game then finds the same
   number again at the Mass type registration (HOLArtVisual::Apply): what it shows does not change
   (HeroesOfLiberty.Art.Migration.MigrationKeepsEveryType). A reference already set is left alone (an artist's choice);
   a number out of the list, or an animation found earlier in the list (the lookup would give another number), is
   reported and left as a number. Run as a script, hidden (the editor quits when done), build queue empty:
     UnrealEditor-Cmd.exe <project> -ExecutePythonScript=<this file> -unattended -nullrhi -nosplash -Multiprocess
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


# --- 2. Animation references (AR3-a) ---------------------------------------------------------------------------------

ART_UNITS = ART_ROOT + "/Units"
STATES = [("idle", "idle_sequence", "idle_animation"), ("walk", "walk_sequence", "walk_animation"),
          ("attack", "attack_sequence", "attack_animation"), ("fall", "fall_sequence", "fall_animation"),
          ("build", "build_sequence", "build_animation")]
RESOURCE_SETS = [("gather", "gather_sequences", "gather_animations"),
                 ("carry walk", "carry_walk_sequences", "carry_walk_animations"),
                 ("carry idle", "carry_idle_sequences", "carry_idle_animations")]
RESOURCES = ["food", "wood", "stone", "gold"]


def sequence_list(provider):
    """The UAnimSequence of each number of the provider (None where a slot is empty)."""
    return [entry.get_editor_property("sequence") for entry in provider.get_sequences()]


def reference_for(sequences, number, label, problems):
    """The animation whose first place in the list is this number; None (and a problem) otherwise."""
    if number < 0:
        return None
    if number >= len(sequences) or sequences[number] is None:
        problems.append("%s: number %d out of the %d sequences" % (label, number, len(sequences)))
        return None
    animation = sequences[number]
    first = sequences.index(animation)
    if first != number:
        problems.append("%s: %s is also number %d (kept as a number)" % (label, animation.get_name(), first))
        return None
    return animation


def is_unset(value):
    return value is None or (hasattr(value, "is_null") and value.is_null())


def write_asset_animations(asset, problems):
    """Sets the animation references of one art asset; the number of references written."""
    provider = asset.get_editor_property("animation_provider")
    if provider is None:
        return 0
    sequences = sequence_list(provider)
    type_name = str(asset.get_editor_property("type_name"))
    written = 0
    for state, number_field, reference_field in STATES:
        if not asset.get_editor_property("override_" + number_field):
            continue
        if not is_unset(asset.get_editor_property(reference_field)):
            continue
        animation = reference_for(sequences, asset.get_editor_property(number_field), type_name + " " + state, problems)
        if animation is not None:
            asset.set_editor_property(reference_field, animation)
            written += 1
    for state, numbers_field, references_field in RESOURCE_SETS:
        if not asset.get_editor_property("override_" + numbers_field):
            continue
        numbers = asset.get_editor_property(numbers_field)
        references = asset.get_editor_property(references_field)
        changed = False
        for resource in RESOURCES:
            if not is_unset(references.get_editor_property(resource)):
                continue
            label = "%s %s %s" % (type_name, state, resource)
            animation = reference_for(sequences, numbers.get_editor_property(resource), label, problems)
            if animation is not None:
                references.set_editor_property(resource, animation)
                changed = True
                written += 1
        if changed:
            asset.set_editor_property(references_field, references)
    return written


def write_animation_references():
    """Writes the animation references of every unit art asset; logs the counts."""
    problems = []
    assets = 0
    written = 0
    for path in unreal.EditorAssetLibrary.list_assets(ART_UNITS, recursive=True, include_folder=False):
        asset = unreal.EditorAssetLibrary.load_asset(path)
        if not isinstance(asset, unreal.HOLArtVisual):
            continue
        count = write_asset_animations(asset, problems)
        if count:
            if not unreal.EditorAssetLibrary.save_loaded_asset(asset, False):
                raise RuntimeError("Could not save " + path)
            unreal.log("art_visual_io: %s: %d animation references (saved)" % (path, count))
            assets += 1
            written += count
    for problem in problems:
        unreal.log_warning("art_visual_io: " + problem)
    unreal.log("art_visual_io: %d animation references written in %d unit art assets, %d left as numbers"
               % (written, assets, len(problems)))


# Run as a script (-ExecutePythonScript): write the references and quit; imported by make_*_mesh.py: nothing runs.
if __name__ == "__main__":
    try:
        write_animation_references()
    finally:
        unreal.SystemLibrary.quit_editor()
