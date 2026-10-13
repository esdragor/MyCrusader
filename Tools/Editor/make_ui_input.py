"""Creates the interface input assets of Common UI (D241, front-end step 3), with Enhanced Input:
  /Game/UI/Input/IA_UI_Confirm   confirm the focused button (keyboard, gamepad; the mouse clicks buttons directly)
  /Game/UI/Input/IA_UI_Back      back out of a screen
  /Game/UI/Input/IMC_UI          their keys, added by the menu player controller

Re-running the script refreshes the mappings of IMC_UI and keeps the actions.
Run (the editor quits when done):
  UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file> -unattended -nop4 -nosplash
"""
import unreal

FOLDER = "/Game/UI/Input"
MAPPINGS = {
    "IA_UI_Confirm": ["Enter", "SpaceBar", "Gamepad_FaceButton_Bottom"],
    "IA_UI_Back": ["Escape", "Gamepad_FaceButton_Right"],
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


def make_key(name):
    key = unreal.Key()
    key.set_editor_property("key_name", name)
    return key


def main():
    actions = {}
    for name in MAPPINGS:
        action = load_or_create(name, unreal.InputAction, unreal.InputAction_Factory())
        action.set_editor_property("value_type", unreal.InputActionValueType.BOOLEAN)
        unreal.EditorAssetLibrary.save_loaded_asset(action)
        actions[name] = action

    context = load_or_create("IMC_UI", unreal.InputMappingContext, unreal.InputMappingContext_Factory())
    context.unmap_all()
    for name, keys in MAPPINGS.items():
        for key_name in keys:
            context.map_key(actions[name], make_key(key_name))
    unreal.EditorAssetLibrary.save_loaded_asset(context)

    unreal.log("UI input assets in %s: %s" % (FOLDER, ", ".join(
        "%s -> %s" % (name, "/".join(keys)) for name, keys in MAPPINGS.items())))


main()
