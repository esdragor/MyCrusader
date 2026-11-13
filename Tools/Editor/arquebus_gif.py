"""Animated GIFs of the Arquebusier's ways A and B (Retours/Q-arquebusier-modele.md; no Unreal, no mesh baked).

The user saw the still sheet (arquebus_sheet.py) and asked for a GIF: the poses MOVING, slow, from several angles.
One GIF per way, the pieces of arquebus_sheet.py on the Crossbowman's gestures (re-used as they are):
  - way A: arquebus 110 cm + bandolier (C) + small smoke (G); way B: musket 150 cm + bandolier (D) + big smoke (H).
  - sequence: rest (idle loop), walk (two loops), raise and shot (the crossbow's own recoil kick, the smoke stays
    where the shot left it for half a second), lowering and the crossbow's STIRRUP reload (the gun points at the
    ground: wrong for a firearm, said in the label), fall (held at the end).
  - four views side by side: front, side, RTS (preview_animation's 3/4 view from above) and the game's camera
    (from behind him, 55 degrees down).
  - played at half speed (source 30 frames/s, each shown GIF_MS ms).
Way C (all new gestures: gun on the shoulder, fork rest, ramrod, fall dropping the gun) has no gesture to animate
yet: no GIF for it.
Run: python Tools/Editor/arquebus_gif.py [A|B ...] [--zoom Z]   -> Saved/AnimPreview/arquebus_way_<X>.gif
     plus arquebus_way_<X>_keys.png (the key frames stacked, to look at without a GIF viewer).
"""
import math
import os
import sys

sys.dont_write_bytecode = True  # no __pycache__ in Tools/Editor
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import anim_rig  # noqa: E402
import arquebus_sheet as aq  # noqa: E402
import peasant_props as pp  # noqa: E402
import preview_animation as pa  # noqa: E402
import soldier_poses  # noqa: E402
from PIL import Image, ImageDraw, ImageFont  # noqa: E402

GIF_MS = 66          # per source frame (30 frames/s -> half speed)
SMOKE_FRAMES = 15    # the smoke stays half a second (source time) where the shot left it
FALL_HOLD_MS = 1500  # the last frame of the fall, held
IDLE_FRAMES = 45     # the rest: the idle loop's first 1.5 s only
GAME_PITCH = 55.0    # the game's camera: behind him, looking down 55 degrees

WAYS = {
    "A": ("VOIE A - arquebuse courte 110 cm + bandouliere (C) + petite fumee (G)", "arqb", "smoke_small"),
    "B": ("VOIE B - mousquet long 150 cm + bandouliere (D) + grosse fumee (H)", "musb", "smoke_big"),
}

# Phases of the crossbow attack loop (shares of the loop, soldier_poses.CROSSBOW_ATTACK keys).
SHOT = soldier_poses.CROSSBOW_SHOT
ATTACK_PHASES = [
    (0.0, "VISEE (geste arbalete : epaule, joue sur la crosse)", None),
    (SHOT, "TIR + RECUL (recul de l'arbalete) + FUMEE", None),
    (0.225, "ABAISSE L'ARME vers l'etrier (geste arbalete)", "warn"),
    (0.41, "RECHARGE = ETRIER D'ARBALETE : FAUX pour une arme a feu (canon pointe au sol) -> geste neuf",
     "warn"),
    (0.94, "retour en garde", None),
]


def game_view(sk):
    """Behind him, looking down GAME_PITCH degrees: screen x = his right, up mixes height and forward."""
    p = math.radians(GAME_PITCH)

    def game(q):
        r, f, u = pa.body_terms(sk, q)
        return (r, f * math.sin(p) + u * math.cos(p) + 60.0, -f * math.cos(p) + u * math.sin(p))

    return (game, 0.0, anim_rig.normalize(sk.body((0.0, -math.cos(p), math.sin(p)))))


def all_views(sk):
    return pa.views(sk) + [game_view(sk)]


VIEW_NAMES = ["FACE", "PROFIL", "VUE RTS (3/4 haut)", "CAMERA JEU (dos, 55 deg)"]


def draw_smoke(canvas, shared, ctx, smoke, shot_frame, ox, oy, view):
    """The smoke at the muzzle of the shot frame (it stays in the air while the gun moves on), depth-written first."""
    sk = shared.sk
    project, centre, to_camera = view
    canvas.clip = (ox, oy, ox + pa.CELL_W, oy + pa.CELL_H)

    def screen(q):
        x, y, n = project(q)
        return (ox + pa.CELL_W / 2 + (x - centre) * pa.PX_PER_CM, oy + pa.CELL_H - 12 - y * pa.PX_PER_CM, n)

    for tri, color in pp.posed_triangles(smoke, sk, ctx["worlds"][shot_frame], shared.prop_tris[smoke["name"]]):
        normal = anim_rig.normalize(anim_rig.cross(anim_rig.sub(tri[1], tri[0]), anim_rig.sub(tri[2], tri[0])))
        light = 0.7 + 0.3 * abs(anim_rig.dot(normal, to_camera))
        canvas.triangle(*(screen(q) for q in tri), tuple(min(255, int(c * light)) for c in color))
    canvas.clip = (0, 0, canvas.w, canvas.h)


def font(size):
    try:
        return ImageFont.load_default(size=size)
    except (TypeError, OSError):
        return ImageFont.load_default()


def build(way, zoom, out_dir):
    title, model, smoke_name = WAYS[way]
    shared = pa.Shared(False)
    aq.register(shared)
    pa.set_zoom(zoom)
    sk = shared.sk
    views = all_views(sk)
    ctx = {tag: pa.prepare(shared, "%s_%s" % (model, tag)) for tag in ("idle", "walk", "fall")}
    attack = attack_plain = pa.prepare(shared, "%s_attack" % model)  # the smoke is drawn apart (draw_smoke)
    smoke = shared.props[smoke_name]
    n_attack = len(attack["rig"].times) - 1
    shot_frame = int(round(SHOT * n_attack))

    # (context, frame, phase label, warn, smoke age or None, duration ms)
    seq = []
    n_idle = min(IDLE_FRAMES, len(ctx["idle"]["rig"].times) - 1)  # the idle loop is ~11 s: its start only
    seq += [(ctx["idle"], k, "REPOS (geste arbalete)", None, None, GIF_MS) for k in range(n_idle)]
    n_walk = len(ctx["walk"]["rig"].times) - 1
    seq += [(ctx["walk"], k % n_walk, "MARCHE (geste arbalete, arme tenue devant)", None, None, GIF_MS)
            for k in range(2 * n_walk)]
    for k in range(n_attack):
        share = k / float(n_attack)
        label, warn = [(lab, w) for start, lab, w in ATTACK_PHASES if share >= start - 1e-9][-1]
        age = k - shot_frame if 0 <= k - shot_frame < SMOKE_FRAMES else None
        seq.append((attack_plain, k, label, warn, age, GIF_MS))
    n_fall = len(ctx["fall"]["rig"].times) - 1
    seq += [(ctx["fall"], k, "MORT (chute de l'Arbaletrier, l'arme reste en main)", None, None,
             FALL_HOLD_MS if k == n_fall else GIF_MS) for k in range(n_fall + 1)]

    header = 58
    width, cell_h = pa.CELL_W * len(views), pa.CELL_H + pa.LABEL_H
    big, small = font(15), font(12)
    frames, durations = [], []
    keys = {}
    for i, (c, k, label, warn, age, ms) in enumerate(seq):
        canvas = pa.Canvas(width, cell_h)
        for col, view in enumerate(views):
            if age is not None:
                draw_smoke(canvas, shared, attack, smoke, shot_frame, col * pa.CELL_W, 0, view)
            pa.draw_cell(canvas, shared, c, k, col * pa.CELL_W, 0, view, "%.2fs" % c["rig"].times[k])
            if col:
                canvas.line((col * pa.CELL_W, 0), (col * pa.CELL_W, cell_h - 1), (210, 210, 205), 0.4)
        body = Image.frombytes("RGB", (width, cell_h), bytes(canvas.pixels))
        img = Image.new("RGB", (width, cell_h + header), pa.BACKGROUND)
        img.paste(body, (0, header))
        draw = ImageDraw.Draw(img)
        draw.text((6, 3), title, fill=(20, 20, 20), font=big)
        draw.text((6, 22), label, fill=(190, 30, 30) if warn else (30, 90, 30), font=big)
        for col, name in enumerate(VIEW_NAMES):
            draw.text((col * pa.CELL_W + 6, 42), name, fill=(70, 70, 70), font=small)
        draw.text((width - 150, 3), "ralenti x2  image %d/%d" % (i + 1, len(seq)), fill=(110, 110, 110), font=small)
        frames.append(img)
        durations.append(ms)
        for name, test in (("repos", i == 0), ("marche", c is ctx["walk"] and k == n_walk // 4),
                           ("tir", c is attack_plain and k == shot_frame + 2),
                           ("recharge", c is attack_plain and k == int(0.57 * n_attack)),
                           ("mort", c is ctx["fall"] and k == n_fall)):
            if test and name not in keys:
                keys[name] = img
    # One palette for the whole GIF (no flicker): taken from the key frames.
    mosaic = Image.new("RGB", (width, (cell_h + header) * len(keys)))
    for j, img in enumerate(keys.values()):
        mosaic.paste(img, (0, j * (cell_h + header)))
    palette = mosaic.quantize(colors=96, method=Image.Quantize.MEDIANCUT)
    gif = [f.quantize(palette=palette, dither=Image.Dither.NONE) for f in frames]
    out = os.path.join(out_dir, "arquebus_way_%s.gif" % way)
    gif[0].save(out, save_all=True, append_images=gif[1:], duration=durations, loop=0, optimize=False, disposal=1)
    mosaic.save(os.path.join(out_dir, "arquebus_way_%s_keys.png" % way))
    print("GIF : %s (%d images, %.1f s par boucle, %.2f Mo) ; images cles : %s" % (
        out, len(gif), sum(durations) / 1000.0, os.path.getsize(out) / 1e6, ", ".join(keys)))
    return out


def main():
    args = sys.argv[1:]
    zoom = float(args[args.index("--zoom") + 1]) if "--zoom" in args else 1.0
    ways = [a for a in args if a in WAYS] or sorted(WAYS)
    os.makedirs(pa.OUT_DIR, exist_ok=True)
    for way in ways:
        build(way, zoom, pa.OUT_DIR)


if __name__ == "__main__":
    main()
