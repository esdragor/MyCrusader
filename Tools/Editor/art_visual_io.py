"""AR3-a: writes the animations by reference of the unit art assets from their current numbers (no visible change).

For every DA_Art under /Game/Art/Units with an AnimationProvider (TP_*), each state whose number the asset sets
(Idle, Walk, Attack, Fall, Build, and by resource Gather / CarryWalk / CarryIdle) gets its reference: the UAnimSequence
at that number in GetSequences() of the provider. The game then finds the same number again at the Mass type
registration (HOLArtVisual::Apply): what it shows does not change (HeroesOfLiberty.Art.Migration.MigrationKeepsEveryType).
A reference already set is left alone (an artist's choice); a number out of the list, or an animation found earlier in
the list (the lookup would give another number), is reported and left as a number.
Run hidden (the editor quits when done), build queue empty:
  UnrealEditor-Cmd.exe <project> -ExecutePythonScript=<this file> -unattended -nullrhi -nosplash -Multiprocess
"""
import unreal

ART_UNITS = "/Game/Art/Units"
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


def write_asset(asset, problems):
    """Sets the references of one art asset; the number of references written."""
    provider = asset.get_editor_property("animation_provider")
    if provider is None:
        return 0
    sequences = sequence_list(provider)
    type_name = str(asset.get_editor_property("type_name"))
    written = 0
    for state, number_field, reference_field in STATES:
        if not asset.get_editor_property("b_override_" + number_field):
            continue
        if not is_unset(asset.get_editor_property(reference_field)):
            continue
        animation = reference_for(sequences, asset.get_editor_property(number_field), type_name + " " + state, problems)
        if animation is not None:
            asset.set_editor_property(reference_field, animation)
            written += 1
    for state, numbers_field, references_field in RESOURCE_SETS:
        if not asset.get_editor_property("b_override_" + numbers_field):
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


def main():
    problems = []
    assets = 0
    written = 0
    for path in unreal.EditorAssetLibrary.list_assets(ART_UNITS, recursive=True, include_folder=False):
        asset = unreal.EditorAssetLibrary.load_asset(path)
        if not isinstance(asset, unreal.HOLArtVisual):
            continue
        count = write_asset(asset, problems)
        if count:
            if not unreal.EditorAssetLibrary.save_loaded_asset(asset, False):
                raise RuntimeError("Could not save " + path)
            assets += 1
            written += count
    for problem in problems:
        unreal.log_warning("art_visual_io: " + problem)
    unreal.log("art_visual_io: %d animation references written in %d unit art assets, %d left as numbers"
               % (written, assets, len(problems)))


try:
    main()
finally:
    unreal.SystemLibrary.quit_editor()
