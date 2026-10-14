"""Measures the ground speed a walk animation was authored for, to set WalkPlayRate (D234).

A walk without root motion walks in place: the planted foot slides backwards under the body at the speed the body
would move. For each sample the lowest foot is taken as planted; its horizontal speed, averaged over the cycle, is the
animation's natural speed. WalkPlayRate = unit speed / natural speed makes the stride match the unit's speed.
Run (the editor quits when done):
  UnrealEditor.exe <project> -ExecutePythonScript="<this file>"
"""
import unreal

SEQUENCE = "/Game/CloseCombat/Character/UE4Mannequin/Animations/Swordsman/Movement/A_SW_Walk_F"
FEET = ("foot_l", "foot_r")
UNIT_SPEED = 300.0  # cm/s, LightInfantry (Tools/Data/UnitTypes.csv)
SAMPLES = 240


def foot_positions(sequence, time, options):
    pose = unreal.AnimPoseExtensions.get_anim_pose_at_time(sequence, time, options)
    return [unreal.AnimPoseExtensions.get_bone_pose(pose, foot, unreal.AnimPoseSpaces.WORLD).translation for foot in FEET]


def main():
    sequence = unreal.load_asset(SEQUENCE)
    if sequence is None:
        raise RuntimeError("Missing " + SEQUENCE)
    length = sequence.get_play_length()
    options = unreal.AnimPoseEvaluationOptions()
    times = [length * i / SAMPLES for i in range(SAMPLES + 1)]
    frames = [foot_positions(sequence, t, options) for t in times]

    speeds = []
    for i in range(SAMPLES):
        planted = min(range(len(FEET)), key=lambda f: frames[i][f].z)
        a, b = frames[i][planted], frames[i + 1][planted]
        horizontal = ((b.x - a.x) ** 2 + (b.y - a.y) ** 2) ** 0.5
        speeds.append(horizontal / (times[i + 1] - times[i]))
    speeds.sort()
    # The median ignores the instants where the planted foot changes (a jump between the two feet).
    natural = speeds[len(speeds) // 2]
    unreal.log("Walk %s: %.2f s, planted foot speed median %.1f cm/s (mean %.1f), WalkPlayRate for %.0f cm/s = %.2f"
               % (sequence.get_name(), length, natural, sum(speeds) / len(speeds), UNIT_SPEED, UNIT_SPEED / natural))


main()
