"""Creates the input assets of a match (D255, J1), with Enhanced Input:
  /Game/Input/IA_Camera_Left, _Right, _Up, _Down   move the RTS camera (arrows; ZQSD belongs to the command grid, D30)
  /Game/Input/IA_Camera_Zoom                       zoom (mouse wheel)
  /Game/Input/IA_Camera_Drag                       drag the ground (middle mouse button)
  /Game/Input/IMC_Game                             their keys, added by the match controller (AHOLPlayerController)

Re-running the script refreshes the mappings of IMC_Game and keeps the actions.
Run (the editor quits when done):
  UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file> -unattended -nop4 -nosplash
"""
import unreal

FOLDER = "/Game/Input"
BOOLEAN = unreal.InputActionValueType.BOOLEAN
AXIS1D = unreal.InputActionValueType.AXIS1D
ACTIONS = {
    "IA_Camera_Left": (BOOLEAN, ["Left"]),
    "IA_Camera_Right": (BOOLEAN, ["Right"]),
    "IA_Camera_Up": (BOOLEAN, ["Up"]),
    "IA_Camera_Down": (BOOLEAN, ["Down"]),
    "IA_Camera_Zoom": (AXIS1D, ["MouseWheelAxis"]),
    "IA_Camera_Drag": (BOOLEAN, ["MiddleMouseButton"]),
}

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()


def load_or_create(name, asset_class, factory):
    path = FOLDER + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    asset = asset_tools.create_asset(name, FOLDER, asset_class, factory)
    if asset is None:
        raise RuntimeError("Could not create " + path)
    return asset


def save(asset):
    # Changes made from Python do not always mark the package dirty: saved anyway.
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError("Could not save " + asset.get_path_name())


def make_key(name):
    key = unreal.Key()
    key.set_editor_property("key_name", name)
    return key


def main():
    actions = {}
    for name, (value_type, _) in ACTIONS.items():
        action = load_or_create(name, unreal.InputAction, unreal.InputAction_Factory())
        action.set_editor_property("value_type", value_type)
        save(action)
        actions[name] = action

    context = load_or_create("IMC_Game", unreal.InputMappingContext, unreal.InputMappingContext_Factory())
    context.unmap_all()
    for name, (_, keys) in ACTIONS.items():
        for key_name in keys:
            context.map_key(actions[name], make_key(key_name))
    save(context)

    unreal.log("Game input assets in %s: %s" % (FOLDER, ", ".join(
        "%s -> %s" % (name, "/".join(keys)) for name, (_, keys) in ACTIONS.items())))


main()
