"""Creates the sound classes and the mix of the volume settings (D245, front-end step 4):
  /Game/Audio/SC_Music, SC_Effects, SC_Voice   sound classes, to be given to the game's sounds
  /Game/Audio/SCM_Volumes                       sound mix whose class overrides carry the volumes of the Sound tab

The master volume needs no asset (it acts on the audio device). Re-running the script keeps existing assets.
Run (the editor quits when done):
  UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file> -unattended -nop4 -nosplash
"""
import unreal

FOLDER = "/Game/Audio"
CLASSES = ["SC_Music", "SC_Effects", "SC_Voice"]
MIX = "SCM_Volumes"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()


def load_or_create(name, asset_class, factory):
    path = FOLDER + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    asset = asset_tools.create_asset(name, FOLDER, asset_class, factory)
    if asset is None:
        raise RuntimeError("Could not create " + path)
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    return asset


def main():
    for name in CLASSES:
        load_or_create(name, unreal.SoundClass, unreal.SoundClassFactory())
    load_or_create(MIX, unreal.SoundMix, unreal.SoundMixFactory())
    unreal.log("Volume assets in %s: %s, %s" % (FOLDER, ", ".join(CLASSES), MIX))


main()
