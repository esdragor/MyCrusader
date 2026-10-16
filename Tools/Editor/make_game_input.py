"""Creates the input assets of a match (D255 in J1, D262 in J2, D30 grid in J3, D288 in J4, D30 "." in J5b, F1 F2 F3 and Tab in J6a.5), with Enhanced Input:
  /Game/Input/IA_Camera_Left, _Right, _Up, _Down   move the RTS camera (arrows; ZQSD belongs to the command grid, D30)
  /Game/Input/IA_Camera_Zoom                       zoom (mouse wheel)
  /Game/Input/IA_Camera_Drag                       drag the ground (middle mouse button)
  /Game/Input/IA_Select                            select what is under the cursor, or nothing; dragged, a box (left click, D262, D288)
  /Game/Input/IA_SelectAdd                         held, a click or a box adds or removes units (Shift, D288)
  /Game/Input/IA_Order                             send the selected units, or the rally point, to the ground (right click, D288)
  /Game/Input/IA_Deselect                          select nothing (Escape, D262)
  /Game/Input/IA_NextIdle                          select the next idle worker, the camera above it (".", D30, J5b)
  /Game/Input/IA_MilitaryBuildings                 select all the player's military buildings; again, the camera to the next one (F1, D30, J6a.5)
  /Game/Input/IA_EconomicBuildings                 the same for its economic buildings (F2)
  /Game/Input/IA_TechnologicalBuildings            the same for its technological buildings (F3); F4 (the hero) comes with the heroes
  /Game/Input/IA_NextSubgroup                      the next subgroup of a group of buildings (Tab, the user's 2nd trial of J6a.5)
  /Game/Input/IA_Grid_00 to IA_Grid_11             the cells of the command grid, A Z E R / Q S D F / W X C V (D30, J3)
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
    "IA_Select": (BOOLEAN, ["LeftMouseButton"]),
    "IA_SelectAdd": (BOOLEAN, ["LeftShift", "RightShift"]),
    "IA_Order": (BOOLEAN, ["RightMouseButton"]),
    "IA_Deselect": (BOOLEAN, ["Escape"]),
    # The engine names a character key after its character on the player's layout: "." on QWERTY (Period); on AZERTY
    # the key of ";" and "." (Semicolon, with Shift or not, as the grid's keys follow AZERTY); the keypad's "." (Decimal).
    "IA_NextIdle": (BOOLEAN, ["Period", "Semicolon", "Decimal"]),
    # F1 to F4 are the game's (D30): the engine's debug view modes on them are removed in DefaultInput.ini.
    "IA_MilitaryBuildings": (BOOLEAN, ["F1"]),
    "IA_EconomicBuildings": (BOOLEAN, ["F2"]),
    "IA_TechnologicalBuildings": (BOOLEAN, ["F3"]),
    # A group of buildings: All, then each type, each main centre alone (AoE4's way); Tab is bound to nothing else.
    "IA_NextSubgroup": (BOOLEAN, ["Tab"]),
}
# Command grid (D30): one key per cell, row by row, as the keys sit on an AZERTY keyboard.
GRID_KEYS = ["A", "Z", "E", "R", "Q", "S", "D", "F", "W", "X", "C", "V"]
for cell, key in enumerate(GRID_KEYS):
    ACTIONS["IA_Grid_%02d" % cell] = (BOOLEAN, [key])

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
