"""Bakes the Peasant's scripted gestures (peasant_poses.py) into AnimSequences on the UE4 mannequin skeleton (J5c.1).

Steps, all in one run:
  1. Dumps the reference skeleton (names, parents, local reference transforms) to
     Tools/Editor/Data/UE4MannequinSkeleton.json, the input of the offline rig and preview, and logs its axis
     conventions.
  2. For each gesture, solves every frame with anim_rig.py (the same code as the offline preview) and writes the
     local transform of the animated bones into /Game/Units/Mannequin/Animations/<name>, created or updated in place
     (remove_all_bone_tracks: a referenced asset is never deleted). No track on `root` (in place), none on ik_* bones.
  3. Reads the sequence back (get_anim_pose_at_time, raw and compressed) and compares component-space positions with
     the offline forward kinematics; checks root at the origin, first frame = last frame, length.
A gesture with a "base" (the loaded walk) first samples that source animation: local transforms of the pelvis and legs
at each of its keys, written to Tools/Editor/Data/<source name>.json for the rig and the offline preview; the baked
sequence keeps the source's length and frame rate, its feet are compared with the source's, and its play rate for the
Peasant is measured as in measure_walk_speed.py.
Run (headless, editor closed, from PowerShell; log lines start with "HOLAnim"):
  UnrealEditor-Cmd.exe <project> -run=pythonscript -script="<this file>" -unattended -nop4 -nosplash
  add "--dump-only" inside the -script value to only refresh the skeleton and source JSON files.
"""
import json
import os
import sys

import unreal

TOOLS_DIR = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()), "Tools", "Editor")
sys.dont_write_bytecode = True  # no __pycache__ in Tools/Editor
sys.path.insert(0, TOOLS_DIR)
import anim_rig  # noqa: E402
import peasant_poses  # noqa: E402

SKELETON = "/Game/CloseCombat/Character/UE4Mannequin/Meshes/UE4_Mannequin_Skeleton"
# Any sequence on that skeleton: FindBonePathToRoot reads the hierarchy through it.
HIERARCHY_SEQUENCE = "/Game/CloseCombat/Character/UE4Mannequin/Animations/Swordsman/Movement/A_SW_Idle"
PREVIEW_MESH = "/Game/Units/Mannequin/SKM_Mannequin_Classic"
FOLDER = "/Game/Units/Mannequin/Animations"
SKELETON_JSON = os.path.join(TOOLS_DIR, "Data", "UE4MannequinSkeleton.json")
DATA_DIR = os.path.join(TOOLS_DIR, "Data")
# Bones taken from a gesture's source animation (the walk under a loaded walk).
BASE_BONES = ("pelvis", "thigh_l", "calf_l", "foot_l", "ball_l", "thigh_r", "calf_r", "foot_r", "ball_r")
FEET = ("foot_l", "foot_r")
# The Peasant's speed and mesh scale (Tools/Data/UnitTypes.csv), for its loaded-walk play rate. The project's rule
# (D234, ImplementationInProgress.md) is WalkPlayRate = unit speed / planted-foot speed, without the mesh scale
# (Peasant on A_SW_Walk_F: 600 / 246 = 2.44); the value with the mesh scale is logged beside it.
PEASANT_SPEED = 600.0
PEASANT_MESH_SCALE = 1.5
# The walk the soldiers and the Peasant use today (WalkPlayRate 2.44 for the Peasant), measured for comparison.
REFERENCE_WALK = "/Game/CloseCombat/Character/UE4Mannequin/Animations/Swordsman/Movement/A_SW_Walk_F"
# Sequences renamed since they were made (old name, new name), renamed in place before the bake so nothing is
# deleted or made twice: the loaded walk became the food carry when wood and stone got their own (2026-10-16).
RENAMES = [("A_Peasant_CarryWalk", "A_Peasant_CarryFood")]
WATCHED = ("pelvis", "head", "hand_l", "hand_r", "lowerarm_l", "lowerarm_r", "foot_l", "foot_r", "calf_l", "calf_r",
           "middle_03_l", "index_03_r")

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
assets = unreal.EditorAssetLibrary
poses = unreal.AnimPoseExtensions
LOCAL, WORLD = unreal.AnimPoseSpaces.LOCAL, unreal.AnimPoseSpaces.WORLD


def log(text):
    unreal.log("HOLAnim: " + text)


def load(path):
    asset = unreal.load_asset(path)
    if asset is None:
        raise RuntimeError("Missing " + path)
    return asset


def vec(v):
    return (v.x, v.y, v.z)


def quat(q):
    return (q.x, q.y, q.z, q.w)


def dump_skeleton(skeleton):
    """Reference skeleton to JSON, parent-first; parents through AnimationLibrary.find_bone_path_to_root."""
    ref = poses.get_reference_pose(skeleton)
    names = [str(n) for n in poses.get_bone_names(ref)]
    hierarchy = load(HIERARCHY_SEQUENCE)
    parents = {}
    for name in names:
        path = [str(n) for n in unreal.AnimationLibrary.find_bone_path_to_root(hierarchy, name)]
        parents[name] = path[1] if len(path) > 1 else ""
    ordered, placed = [], set()
    while len(ordered) < len(names):
        for name in names:
            if name not in placed and (not parents[name] or parents[name] in placed):
                ordered.append(name)
                placed.add(name)
    bones = []
    worst_scale = 0.0
    for name in ordered:
        local = poses.get_ref_bone_pose(ref, name, LOCAL)
        world = poses.get_ref_bone_pose(ref, name, WORLD)
        worst_scale = max(worst_scale, max(abs(c - 1.0) for c in vec(local.scale3d)))
        bones.append({"name": name, "parent": parents[name], "local_t": vec(local.translation),
                      "local_q": quat(local.rotation), "world_t": vec(world.translation),
                      "world_q": quat(world.rotation)})
    os.makedirs(os.path.dirname(SKELETON_JSON), exist_ok=True)
    with open(SKELETON_JSON, "w", encoding="utf-8") as f:
        json.dump({"skeleton": SKELETON, "bones": bones}, f, indent=1)
    log("squelette : %d os ecrits dans %s (parents par AnimationLibrary.find_bone_path_to_root, ecart d'echelle max "
        "%.4f)" % (len(bones), SKELETON_JSON, worst_scale))
    sk = anim_rig.Skeleton.load(SKELETON_JSON)
    # The offline FK must give back Unreal's component-space reference pose.
    fk_error = max(anim_rig.length(anim_rig.sub(sk.ref_world[i][0], tuple(b["world_t"]))) for i, b in enumerate(bones))
    log("FK hors ligne de la pose de reference : ecart max %.4f cm" % fk_error)
    return sk


def log_axes(sk):
    """Which local axis runs along each bone (toward its child), in the reference pose."""
    log("droite du personnage = %s (hand_r en x = %.1f), avant = +Y, haut = +Z"
        % ("+X" if sk.right[0] > 0 else "-X", sk.pos("hand_r")[0]))
    pairs = [("pelvis", "spine_01"), ("spine_01", "spine_02"), ("spine_03", "neck_01"), ("neck_01", "head"),
             ("clavicle_l", "upperarm_l"), ("clavicle_r", "upperarm_r"), ("upperarm_l", "lowerarm_l"),
             ("upperarm_r", "lowerarm_r"), ("lowerarm_l", "hand_l"), ("lowerarm_r", "hand_r"),
             ("hand_l", "middle_01_l"), ("hand_r", "middle_01_r"), ("thigh_l", "calf_l"), ("thigh_r", "calf_r"),
             ("calf_l", "foot_l"), ("calf_r", "foot_r"), ("foot_l", "ball_l"), ("foot_r", "ball_r")]
    for bone, child in pairs:
        along = anim_rig.qrot(anim_rig.qconj(sk.rot(bone)), anim_rig.sub(sk.pos(child), sk.pos(bone)))
        axis = max(range(3), key=lambda k: abs(along[k]))
        x_world = anim_rig.qrot(sk.rot(bone), (1.0, 0.0, 0.0))
        log("%-11s -> %-11s : axe local %s%s (%.1f cm) ; X local en composant = (%.2f, %.2f, %.2f)"
            % (bone, child, "+" if along[axis] > 0 else "-", "XYZ"[axis], abs(along[axis]), *x_world))


def natural_speed(sequence, samples=240):
    """Ground speed a walk without root motion was made for: median speed of the lowest (planted) foot."""
    options = unreal.AnimPoseEvaluationOptions()
    length = sequence.get_play_length()
    times = [length * i / samples for i in range(samples + 1)]
    feet = []
    for t in times:
        pose = poses.get_anim_pose_at_time(sequence, t, options)
        feet.append([vec(poses.get_bone_pose(pose, foot, WORLD).translation) for foot in FEET])
    speeds = []
    for i in range(samples):
        planted = min(range(len(FEET)), key=lambda f: feet[i][f][2])
        a, b = feet[i][planted], feet[i + 1][planted]
        speeds.append(((b[0] - a[0]) ** 2 + (b[1] - a[1]) ** 2) ** 0.5 / (times[i + 1] - times[i]))
    speeds.sort()
    return speeds[len(speeds) // 2]


def play_rate(speed):
    """Play rate of a walk for the Peasant, project rule first, then with the mesh scale."""
    return "%.2f (avec MeshScale %.1f : %.2f)" % (PEASANT_SPEED / speed, PEASANT_MESH_SCALE,
                                                  PEASANT_SPEED / (PEASANT_MESH_SCALE * speed))


def dump_base(path):
    """Samples the pelvis and legs of a source animation at each of its keys, to Data/<name>.json."""
    source = load(path)
    length = source.get_play_length()
    frames = unreal.AnimationLibrary.get_num_frames(source)
    fps = frames / length
    times = [length * i / frames for i in range(frames + 1)]
    options = unreal.AnimPoseEvaluationOptions()
    bones = {name: [] for name in BASE_BONES}
    root_travel = pelvis_first = pelvis_last = None
    for k, t in enumerate(times):
        pose = poses.get_anim_pose_at_time(source, t, options)
        for name in BASE_BONES:
            local = poses.get_bone_pose(pose, name, LOCAL)
            bones[name].append(list(vec(local.translation)) + list(quat(local.rotation)))
        root = vec(poses.get_bone_pose(pose, "root", WORLD).translation)
        root_travel = max(root_travel or 0.0, anim_rig.length(root))
        pelvis = vec(poses.get_bone_pose(pose, "pelvis", WORLD).translation)
        pelvis_first = pelvis_first or pelvis
        pelvis_last = pelvis
    # Loop: the last key must equal the first; the source's own gap is logged, then closed.
    gap = max(max(abs(a - b) for a, b in zip(bones[n][0], bones[n][-1])) for n in BASE_BONES)
    for name in BASE_BONES:
        bones[name][-1] = list(bones[name][0])
    speed = natural_speed(source)
    data = {"source": path, "length": length, "fps": fps, "frames": frames, "times": times, "bones": bones,
            "natural_speed": speed}
    out = os.path.join(DATA_DIR, path.rsplit("/", 1)[-1] + ".json")
    with open(out, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=1)
    log("source %s : %.4f s, %d images (%.2f i/s), mouvement de racine %s, racine a %.2f cm de l'origine au plus, "
        "bassin deplace de %.2f cm entre premiere et derniere image, ecart de boucle de la source %.5f (ferme), "
        "pied d'appui %.1f cm/s -> vitesse de lecture du Paysan %s ; ecrit dans %s"
        % (path, length, frames, fps, source.get_editor_property("enable_root_motion"), root_travel,
           anim_rig.length(anim_rig.sub(pelvis_first, pelvis_last)), gap, speed, play_rate(speed), out))


def frame_rate(fps):
    whole = int(round(fps))
    return unreal.FrameRate(whole, 1) if abs(fps - whole) < 1e-3 else unreal.FrameRate(int(round(fps * 1000)), 1000)


def bake(sk, gesture):
    name = gesture["name"]
    path = FOLDER + "/" + name
    base = anim_rig.load_base(gesture, DATA_DIR)
    rig = anim_rig.Rig(sk, gesture, base)
    fps = rig.fps
    times = rig.times
    frames = len(times) - 1
    bones = list(anim_rig.ANIMATED_BONES)
    bones += [b for b in (base["bones"] if base else ()) if b not in bones]
    locals_by_frame = []
    worst_grip = 0.0
    for t in times:
        local, axe = rig.pose(t)
        locals_by_frame.append(local)
        worst_grip = max(worst_grip, max(rig.grip_errors(sk.fk(local), axe).values()))

    sequence = load(path) if assets.does_asset_exist(path) else None
    if sequence is None:
        factory = unreal.AnimSequenceFactory()
        factory.set_editor_property("target_skeleton", load(SKELETON))
        factory.set_editor_property("preview_skeletal_mesh", load(PREVIEW_MESH))
        sequence = asset_tools.create_asset(name, FOLDER, unreal.AnimSequence, factory)
        if sequence is None:
            raise RuntimeError("Could not create " + path)
        log("%s cree" % path)
    controller = sequence.get_editor_property("controller")
    controller.open_bracket("Bake " + name, False)
    controller.remove_all_bone_tracks(False)
    controller.set_frame_rate(frame_rate(fps), False)
    controller.set_number_of_frames(unreal.FrameNumber(frames), False)
    for bone in bones:
        i = sk.index[bone]
        positions = [unreal.Vector(*local[i][0]) for local in locals_by_frame]
        rotations = [unreal.Quat(*local[i][1]) for local in locals_by_frame]
        scales = [unreal.Vector(1.0, 1.0, 1.0)] * len(locals_by_frame)
        controller.add_bone_curve(bone, False)
        if not controller.set_bone_track_keys(bone, positions, rotations, scales, False):
            raise RuntimeError("Could not key " + bone)
    controller.close_bracket(False)
    sequence.set_editor_property("enable_root_motion", False)
    assets.save_loaded_asset(sequence)
    log("%s : %d images a %.2f i/s (%.3f s), %d pistes, ecart max des mains a leur cible %.2f cm"
        % (path, frames, fps, rig.length, len(bones), worst_grip))
    return sequence, rig, times, locals_by_frame


def verify(sk, sequence, rig, times, locals_by_frame):
    length = sequence.get_play_length()
    tracks = [str(n) for n in unreal.AnimationLibrary.get_animation_track_names(sequence)]
    log("relecture : duree %.4f s (attendue %.4f), %d images, pistes : %s"
        % (length, rig.length, unreal.AnimationLibrary.get_num_frames(sequence), ", ".join(tracks)))
    if "root" in tracks or any(t.startswith("ik_") for t in tracks):
        log("ERREUR : piste sur root ou sur un os ik_*")
    for label, eval_type in (("brute", unreal.AnimDataEvalType.RAW), ("compressee", unreal.AnimDataEvalType.COMPRESSED)):
        options = unreal.AnimPoseEvaluationOptions()
        options.set_editor_property("evaluation_type", eval_type)
        worst, worst_at, root_worst = 0.0, "", 0.0
        first = last = None
        for k, t in enumerate(times):
            pose = poses.get_anim_pose_at_time(sequence, t, options)
            offline = sk.fk(locals_by_frame[k])
            here = {}
            for bone in WATCHED:
                p = vec(poses.get_bone_pose(pose, bone, WORLD).translation)
                here[bone] = p
                error = anim_rig.length(anim_rig.sub(p, offline[sk.index[bone]][0]))
                if error > worst:
                    worst, worst_at = error, "%s a %.3f s" % (bone, t)
            root = poses.get_bone_pose(pose, "root", WORLD)
            root_worst = max(root_worst, anim_rig.length(vec(root.translation)))
            if k == 0:
                first = here
            last = here
        loop = max(anim_rig.length(anim_rig.sub(first[b], last[b])) for b in WATCHED)
        log("relecture %s : ecart max FK hors ligne / Unreal %.3f cm (%s), racine a %.3f cm de l'origine au plus, "
            "premiere / derniere image ecart %.3f cm" % (label, worst, worst_at, root_worst, loop))
    if rig.base is not None:
        # Same feet as the source walk, at the same times (no new foot sliding), and the same play rate.
        source = load(rig.base["source"])
        options = unreal.AnimPoseEvaluationOptions()
        feet_worst = 0.0
        for t in times[:-1]:
            ours = poses.get_anim_pose_at_time(sequence, t, options)
            theirs = poses.get_anim_pose_at_time(source, t, options)
            for foot in FEET + ("ball_l", "ball_r"):
                a = vec(poses.get_bone_pose(ours, foot, WORLD).translation)
                b = vec(poses.get_bone_pose(theirs, foot, WORLD).translation)
                feet_worst = max(feet_worst, anim_rig.length(anim_rig.sub(a, b)))
        speed = natural_speed(sequence)
        log("pieds face a la source %s : ecart max %.3f cm (hors derniere image, refermee) ; pied d'appui %.1f cm/s "
            "-> vitesse de lecture du Paysan %s (source %s)"
            % (rig.base["source"].rsplit("/", 1)[-1], feet_worst, speed, play_rate(speed),
               play_rate(rig.base["natural_speed"])))


def rename_old():
    for old, new in RENAMES:
        old_path, new_path = FOLDER + "/" + old, FOLDER + "/" + new
        if not assets.does_asset_exist(old_path):
            continue
        if assets.does_asset_exist(new_path):
            log("ERREUR : %s et %s existent tous deux, renommage saute" % (old_path, new_path))
            continue
        referencers = assets.find_package_referencers_for_asset(old_path)
        renamed = assets.rename_asset(old_path, new_path)
        log("renommage %s -> %s : %s (%d referenceurs avant) ; ancien chemin encore present : %s"
            % (old_path, new_path, renamed, len(referencers), assets.does_asset_exist(old_path)))


def main():
    rename_old()
    skeleton = load(SKELETON)
    sk = dump_skeleton(skeleton)
    log_axes(sk)
    reference = natural_speed(load(REFERENCE_WALK))
    log("marche de reference %s : pied d'appui %.1f cm/s -> vitesse de lecture du Paysan %s"
        % (REFERENCE_WALK.rsplit("/", 1)[-1], reference, play_rate(reference)))
    for source in sorted({g["base"] for g in peasant_poses.GESTURES.values() if "base" in g}):
        dump_base(source)
    if "--dump-only" in sys.argv[1:]:
        return
    for gesture in peasant_poses.GESTURES.values():
        verify(sk, *bake(sk, gesture))
    log("termine")


main()
