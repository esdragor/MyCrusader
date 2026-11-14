"""Animated GIF of the Arquebusier's way A with the NEW gestures (arquebus_poses.py), the candidate's LETTER written on the
screen (Retours/Q-arquebusier-modele.md, answer of 2026-11-14: "il n'y a pas de lettres", "les GIF montrent les animations
de l'Arbaletrier"). No Unreal, nothing baked.
Letters (arquebus_sheet.py's sheet): C arquebus 110 cm + bandolier at rest, E walk, G the shot + small smoke, J the fall;
I' replaces the sheet's I (the crossbow's stirrup reload, wrong for a firearm) by the ramrod reload.
What is NEW here and what is not is written on every phase: the walk with the gun on the shoulder, the shot's recoil, the
grounded gun and the whole reload are new; the raise and aim (G, first part) and the death (J) are still the
Crossbowman's.
Run: python Tools/Editor/arquebus_new_gif.py [--zoom Z]   -> Saved/AnimPreview/arquebus_new_A.gif and _keys.png
"""
import math
import os
import sys

sys.dont_write_bytecode = True  # no __pycache__ in Tools/Editor
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import anim_rig  # noqa: E402
import arquebus_gif as ag  # noqa: E402
import arquebus_poses as ap  # noqa: E402
import peasant_props as pp  # noqa: E402
import preview_animation as pa  # noqa: E402
from PIL import Image, ImageDraw  # noqa: E402

GIF_MS = 66          # per source frame (30 frames/s -> half speed)
SMOKE_FRAMES = 26    # the smoke stays ~0.9 s: grows, rises, thins out
FALL_HOLD_MS = 1500
IDLE_FRAMES = 45
TITLE = "VOIE A : C arquebuse 110 cm + bandouliere, G petite fumee"

T = ap.P
# (start share of the attack loop, label, kind): kind "new" green, "old" orange (crossbow's gesture kept)
ATTACK_PHASES = [
    (0.0, "G  MISE EN JOUE : epaule, joue sur la crosse (poses de l'Arbaletrier, gardees)", "old"),
    (ap.ARQ_SHOT - 0.015, "G  TIR : recul violent + fumee (NEUF)", "new"),
    (0.19, "G  on baisse l'arme, crosse a terre, canon en haut (NEUF)", "new"),
    (ap.T_GROUND, "I'  RECHARGE 1/6 : charge de poudre prise a la bandouliere (NEUF)", "new"),
    (ap.T_POUR - 0.01, "I'  RECHARGE 2/6 : versee dans le canon (NEUF)", "new"),
    (ap.T_POUCH - 0.01, "I'  RECHARGE 3/6 : la balle dans le canon (NEUF)", "new"),
    (ap.T_BALL + 0.01, "I'  RECHARGE 4/6 : on tire la baguette (NEUF)", "new"),
    (ap.T_TURN, "I'  RECHARGE 5/6 : on bourre, deux fois (NEUF)", "new"),
    (ap.T_OUT, "I'  RECHARGE 6/6 : on range la baguette (NEUF)", "new"),
    (ap.T_REGAIN1, "on releve l'arme au port (NEUF)", "new"),
    (0.985, "retour en garde", "old"),
]
COLORS = {"new": (30, 110, 30), "old": (190, 110, 20)}


def puff(canvas, shared, attack_ctx, smoke, shot_frame, age, ox, oy, view):
    """The shot's smoke at its age (frames): grows 0.6 -> 1.5, rises 0.9 cm a frame, darkens toward the background."""
    sk = shared.sk
    project, centre, to_camera = view
    canvas.clip = (ox, oy, ox + pa.CELL_W, oy + pa.CELL_H)

    def screen(q):
        x, y, n = project(q)
        return (ox + pa.CELL_W / 2 + (x - centre) * pa.PX_PER_CM, oy + pa.CELL_H - 12 - y * pa.PX_PER_CM, n)

    tris = pp.posed_triangles(smoke, sk, attack_ctx["worlds"][shot_frame], shared.prop_tris[smoke["name"]])
    pts = [p for tri, _ in tris for p in tri]
    c = tuple(sum(p[i] for p in pts) / len(pts) for i in range(3))
    s = 0.6 + 0.9 * min(1.0, age / 14.0)
    rise = anim_rig.scale(sk.up, 0.9 * age)
    fade = min(1.0, age / float(SMOKE_FRAMES)) ** 1.5
    for tri, color in tris:
        moved = [anim_rig.add(anim_rig.add(c, anim_rig.scale(anim_rig.sub(p, c), s)), rise) for p in tri]
        normal = anim_rig.normalize(anim_rig.cross(anim_rig.sub(moved[1], moved[0]), anim_rig.sub(moved[2], moved[0])))
        light = 0.7 + 0.3 * abs(anim_rig.dot(normal, to_camera))
        bg = pa.BACKGROUND
        mixed = tuple(int((cc * light) * (1 - 0.6 * fade) + b * 0.6 * fade) for cc, b in zip(color, bg))
        canvas.triangle(*(screen(q) for q in moved), tuple(min(255, m) for m in mixed))
    canvas.clip = (0, 0, canvas.w, canvas.h)


def build(zoom, out_dir):
    shared = pa.Shared(False)
    ap.register(shared)
    pa.set_zoom(zoom)
    sk = shared.sk
    views = ag.all_views(sk)
    idle = pa.prepare(shared, "arq_new_idle")
    walk = pa.prepare(shared, "arq_new_walk")
    attack = pa.prepare(shared, "arq_new_attack")
    fall = pa.prepare(shared, "arqb_fall")
    smoke = shared.props["smoke_small"]
    n_attack = len(attack["rig"].times) - 1
    shot_frame = int(round(ap.ARQ_SHOT * n_attack))

    seq = []  # (ctx, frame, letter, label, kind, smoke age, ms)
    n_idle = min(IDLE_FRAMES, len(idle["rig"].times) - 1)
    seq += [(idle, k, "C", "C  REPOS : arquebuse sur l'epaule, bandouliere (NEUF)", "new", None, GIF_MS)
            for k in range(n_idle)]
    n_walk = len(walk["rig"].times) - 1
    seq += [(walk, k % n_walk, "E", "E  MARCHE : arme sur l'epaule (NEUF)", "new", None, GIF_MS)
            for k in range(2 * n_walk)]
    for k in range(n_attack):
        share = k / float(n_attack)
        label, kind = [(lab, w) for start, lab, w in ATTACK_PHASES if share >= start - 1e-9][-1]
        age = k - shot_frame if 0 <= k - shot_frame < SMOKE_FRAMES else None
        seq.append((attack, k, "G", label, kind, age, GIF_MS))
    n_fall = len(fall["rig"].times) - 1
    seq += [(fall, k, "J", "J  MORT : chute de l'Arbaletrier (gardee, a refaire si tu veux)", "old", None,
             FALL_HOLD_MS if k == n_fall else GIF_MS) for k in range(n_fall + 1)]

    header = 58
    width, cell_h = pa.CELL_W * len(views), pa.CELL_H + pa.LABEL_H
    big, small, huge = ag.font(15), ag.font(12), ag.font(30)
    frames, durations, keys = [], [], {}
    for i, (c, k, letter, label, kind, age, ms) in enumerate(seq):
        canvas = pa.Canvas(width, cell_h)
        for col, view in enumerate(views):
            if age is not None:
                puff(canvas, shared, attack, smoke, shot_frame, age, col * pa.CELL_W, 0, view)
            pa.draw_cell(canvas, shared, c, k, col * pa.CELL_W, 0, view, "%.2fs" % c["rig"].times[k])
            if col:
                canvas.line((col * pa.CELL_W, 0), (col * pa.CELL_W, cell_h - 1), (210, 210, 205), 0.4)
        body = Image.frombytes("RGB", (width, cell_h), bytes(canvas.pixels))
        img = Image.new("RGB", (width, cell_h + header), pa.BACKGROUND)
        img.paste(body, (0, header))
        draw = ImageDraw.Draw(img)
        draw.text((6, 3), TITLE, fill=(20, 20, 20), font=big)
        draw.text((6, 22), label, fill=COLORS[kind], font=big)
        for col, name in enumerate(ag.VIEW_NAMES):
            draw.text((col * pa.CELL_W + 6, 42), name, fill=(70, 70, 70), font=small)
        draw.text((width - 230, 3), "ralenti x2  image %d/%d" % (i + 1, len(seq)), fill=(110, 110, 110), font=small)
        draw.text((width - 50, 2), letter, fill=COLORS[kind], font=huge)  # the candidate's letter, big
        frames.append(img)
        durations.append(ms)
        for name, test in (("repos", i == 0), ("marche", c is walk and k == n_walk // 4),
                           ("tir", c is attack and k == shot_frame + 3),
                           ("poudre", c is attack and k == int(0.395 * n_attack)),
                           ("baguette", c is attack and k == int(0.585 * n_attack)),
                           ("bourrage", c is attack and k == int(0.69 * n_attack)),
                           ("mort", c is fall and k == n_fall)):
            if test and name not in keys:
                keys[name] = img
    mosaic = Image.new("RGB", (width, (cell_h + header) * len(keys)))
    for j, img in enumerate(keys.values()):
        mosaic.paste(img, (0, j * (cell_h + header)))
    palette = mosaic.quantize(colors=96, method=Image.Quantize.MEDIANCUT)
    gif = [f.quantize(palette=palette, dither=Image.Dither.NONE) for f in frames]
    out = os.path.join(out_dir, "arquebus_new_A.gif")
    gif[0].save(out, save_all=True, append_images=gif[1:], duration=durations, loop=0, optimize=False, disposal=1)
    mosaic.save(os.path.join(out_dir, "arquebus_new_A_keys.png"))
    print("GIF : %s (%d images, %.1f s par boucle, %.2f Mo) ; images cles : %s" % (
        out, len(gif), sum(durations) / 1000.0, os.path.getsize(out) / 1e6, ", ".join(keys)))


def main():
    args = sys.argv[1:]
    zoom = float(args[args.index("--zoom") + 1]) if "--zoom" in args else 1.0
    os.makedirs(pa.OUT_DIR, exist_ok=True)
    build(zoom, pa.OUT_DIR)


if __name__ == "__main__":
    main()
