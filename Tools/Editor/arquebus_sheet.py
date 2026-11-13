"""Offline sheet of TEN labelled Arquebusier candidates (L82 of Auto_Journey_File, tour 1; no Unreal, no mesh baked).

The Arquebusier has today the Crossbowman's mesh and gestures (Placeholders.txt). This sheet proposes, side by side,
candidate pieces (an arquebus or a musket in the crossbow's tool frame, a bandolier of charges, the smoke of the shot)
on the Crossbowman's gestures, so the user says "aime / pas aime" per letter. Nothing here is baked nor chosen: the
pieces live only in this file (not in soldier_props.SOLDIER_MESHES), the gestures are the crossbow ones re-used.
  - tool frame = the crossbow's (soldier_props, AR1): X from the front toward the butt, Y the top of the stock, Z
    across; the hands stay where the crossbow gestures put them (left at CROSSBOW_LEFT_GRIP, right 45 cm behind),
    so the butt ends near x 95 like the crossbow's, the muzzle sticks out forward (negative x).
  - columns A-J: model at rest (A-D), walk (E-F), shot with smoke (G-H), the crossbow's stirrup reload (I) and fall
    (J) to show what the borrowed gestures give with a firearm. Three rows: front, side, RTS view (preview_animation).
Run: python Tools/Editor/arquebus_sheet.py [--zoom Z] [--out <png>]   (default Saved/AnimPreview/arquebus_sheet.png)
"""
import os
import sys

sys.dont_write_bytecode = True  # no __pycache__ in Tools/Editor
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import anim_rig  # noqa: E402
import peasant_props as pp  # noqa: E402
import preview_animation as pa  # noqa: E402
import soldier_poses  # noqa: E402
import soldier_props as sp  # noqa: E402
from PIL import Image, ImageDraw  # noqa: E402

BUTT_X = 95.0          # same butt as the crossbow's tiller (the aim keeps it at the shoulder)
SMOKE = (206, 204, 198)
FLASH = (250, 196, 70)
MATCH = (230, 110, 40)  # the lit match cord's end


def make_gun(name, length, drop, light=False):
    """A matchlock in the crossbow's tool frame: `length` muzzle to butt, `drop` how far the butt falls (cm)."""
    sides = 4 if light else 8
    muzzle = BUTT_X - length
    prims = [
        # Barrel on top, forestock under it, ramrod under the forestock.
        pp.frustum_between((muzzle, 2.6, 0.0), (62.0, 2.6, 0.0), (1.5, 1.5), (1.9, 1.9), sides, sp.IRON,
                           x_hint=pp.FLAT),
        pp.prism([(muzzle + 6.0, -1.6), (62.0, -2.2), (62.0, 1.6), (muzzle + 6.0, 1.4)], 3.6, (0.0, 0.0, -1.8),
                 pp.IDENTITY, sp.WOOD),
        pp.frustum_between((muzzle + 2.0, -2.4, 0.0), (56.0, -2.4, 0.0), (0.45, 0.45), (0.45, 0.45), 3, pp.WOOD_DARK,
                           x_hint=pp.FLAT),
        # Butt: from the lock back, falling by `drop`.
        pp.prism([(60.0, -2.4), (BUTT_X, -2.4 - drop), (BUTT_X, 1.0), (60.0, 3.2)], 4.4, (0.0, 0.0, -2.2),
                 pp.IDENTITY, pp.WOOD_DARK),
        # Lock plate on his right side, the serpentine holding the match, the trigger bar under the stock.
        pp.box((63.0, 1.0, 2.5), (9.0, 3.0, 0.8), sp.IRON),
        pp.box((60.0, 4.6, 2.5), (1.2, 4.4, 1.2), sp.BRASS),
        pp.box((70.0, -4.2, 0.0), (7.0, 1.0, 1.2), sp.IRON),
    ]
    if not light:
        prims.append(pp.box((60.0, 7.2, 2.5), (1.4, 1.4, 1.4), MATCH))
        prims.append(pp.frustum_between((muzzle - 0.5, 2.6, 0.0), (muzzle + 2.5, 2.6, 0.0), (2.2, 2.2), (2.2, 2.2),
                                        sides, sp.IRON))  # muzzle ring
    return {"id": 8, "name": name, "label": name, "bone": "ik_hand_gun", "frame": "tool", "prims": prims}


def blob(centre, radius, color, sides=8):
    """A rough ball: two cones base to base along X."""
    a = anim_rig.add(centre, (radius, 0.0, 0.0))
    b = anim_rig.add(centre, (-radius, 0.0, 0.0))
    r = (radius, radius)
    small = (radius * 0.35, radius * 0.35)
    return [pp.frustum_between(a, centre, small, r, sides, color, caps=("bottom",), x_hint=pp.FLAT),
            pp.frustum_between(centre, b, r, small, sides, color, caps=("top",), x_hint=pp.FLAT)]


def make_smoke(name, length, size):
    """The shot's flash and powder smoke in front of the muzzle (tool frame), `size` 1 = small, 2 = big."""
    muzzle = BUTT_X - length
    prims = blob((muzzle - 4.0, 2.6, 0.0), 3.5, FLASH)
    for k, (dx, dy, dz, r) in enumerate(((-14, 3, 0, 7), (-28, 6, 3, 10), (-44, 10, -2, 12), (-60, 16, 4, 14))):
        if k >= 2 + size:
            break
        prims += blob((muzzle + dx * (0.5 + 0.25 * size), 2.6 + dy, dz), r * (0.6 + 0.3 * size), SMOKE)
    return {"id": 7, "name": name, "label": name, "bone": "ik_hand_gun", "frame": "tool", "prims": prims}


# Bandolier ("twelve apostles"): body terms (forward, right, up), left shoulder over the chest to the right hip.
STRAP = [(3.0, -17.0, 153.0), (12.0, -10.0, 143.0), (15.8, -2.0, 131.0), (16.0, 7.0, 119.0), (14.5, 15.0, 107.0),
         (9.0, 21.0, 98.0)]


def make_bandolier(light=False):
    prims = []
    for a, b in zip(STRAP, STRAP[1:]):
        prims.append(pp.frustum_between(a, b, (0.5, 2.2), (0.5, 2.2), 4, sp.LEATHER, caps=(), x_hint=(1.0, 0.0, 0.0)))
    for k in range(1, len(STRAP) - 1):
        top = anim_rig.add(STRAP[k], (1.4, 0.0, -1.0))
        bottom = anim_rig.add(top, (0.4, 0.0, -7.0))
        prims.append(pp.frustum_between(bottom, top, (1.3, 1.3), (1.3, 1.3), 6, pp.WOOD_DARK))
        prims.append(pp.frustum_between(top, anim_rig.add(top, (0.0, 0.0, 1.2)), (1.4, 1.4), (0.8, 0.8), 6, sp.BRASS))
    # Powder flask at the right hip, hanging from the strap's end.
    flask = (11.0, 22.0, 88.0)
    prims.append(pp.frustum_between(flask, anim_rig.add(flask, (0.0, 0.0, 9.0)), (4.0, 2.6), (1.6, 1.4), 8,
                                    (120, 80, 40)))
    return sp.body_piece(6, "bandolier", "bandouliere", "spine_03", prims)


ARQ, MUSKET = 110.0, 150.0


def register(shared):
    pieces = [make_gun("arquebus", ARQ, 6.0), make_gun("musket", MUSKET, 10.0), make_smoke("smoke_small", ARQ, 1),
              make_smoke("smoke_big", MUSKET, 2), make_bandolier()]
    for p in pieces:
        shared.props[p["name"]] = p
        shared.prop_tris[p["name"]] = pp.prop_triangles(p)
    g = soldier_poses.GESTURES
    for tag, base in (("idle", "crossbow_idle"), ("walk", "crossbow_walk"), ("attack", "crossbow_attack"),
                      ("fall", "crossbow_fall")):
        for model, props in (("arq", ("arquebus",)), ("arqb", ("arquebus", "bandolier")),
                             ("mus", ("musket",)), ("musb", ("musket", "bandolier")),
                             ("arqs", ("arquebus", "smoke_small")), ("musbs", ("musket", "bandolier", "smoke_big"))):
            pa.ALL_GESTURES["%s_%s" % (model, tag)] = dict(g[base], name="%s_%s" % (model, tag), props=props)


# (letter, label, gesture, where: "rest" | "walk" | "shot" | fraction of the loop)
SAMPLES = [
    ("A", "arquebuse 110 cm, repos", "arq_idle", 0.0),
    ("B", "mousquet 150 cm, repos", "mus_idle", 0.0),
    ("C", "arquebuse + bandouliere", "arqb_idle", 0.0),
    ("D", "mousquet + bandouliere", "musb_idle", 0.0),
    ("E", "arquebuse, marche", "arq_walk", 0.25),
    ("F", "mousquet + band., marche", "musb_walk", 0.25),
    ("G", "arquebuse, tir + fumee", "arqs_attack", "shot"),
    ("H", "mousquet, tir + grosse fumee", "musbs_attack", "shot"),
    ("I", "recharge = etrier arbalete", "arq_attack", 0.5),
    ("J", "mort = chute arbalete", "arq_fall", 0.86),
]


def main():
    args = sys.argv[1:]
    zoom = float(args[args.index("--zoom") + 1]) if "--zoom" in args else 1.2
    out = args[args.index("--out") + 1] if "--out" in args else os.path.join(pa.OUT_DIR, "arquebus_sheet.png")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    shared = pa.Shared(False)
    register(shared)
    pa.set_zoom(zoom)
    contexts = [pa.prepare(shared, gesture) for _, _, gesture, _ in SAMPLES]
    canvas = pa.Canvas(pa.CELL_W * len(SAMPLES), (pa.CELL_H + pa.LABEL_H) * 3)
    for row, view in enumerate(pa.views(shared.sk)):
        oy = row * (pa.CELL_H + pa.LABEL_H)
        for col, ((letter, label, gesture, where), ctx) in enumerate(zip(SAMPLES, contexts)):
            n = len(ctx["rig"].times) - 1
            # The shot: the crossbow attack's release frame (CROSSBOW_SHOT of the loop; the rig's "hit" is 0 here).
            frame = int(round((soldier_poses.CROSSBOW_SHOT if where == "shot" else where) * n))
            pa.draw_cell(canvas, shared, ctx, frame, col * pa.CELL_W, oy, view, "%.2fs" % ctx["rig"].times[frame])
        for col in range(1, len(SAMPLES)):
            canvas.line((col * pa.CELL_W, oy), (col * pa.CELL_W, oy + pa.CELL_H + pa.LABEL_H - 1), (210, 210, 205), 0.4)
    canvas.save(out)
    # Header band with the letters and labels (PIL default font: ASCII only).
    img = Image.open(out).convert("RGB")
    band = 22
    sheet = Image.new("RGB", (img.width, img.height + band), pa.BACKGROUND)
    sheet.paste(img, (0, band))
    draw = ImageDraw.Draw(sheet)
    for col, (letter, label, _, _) in enumerate(SAMPLES):
        draw.text((col * pa.CELL_W + 6, 5), "%s  %s" % (letter, label), fill=(20, 20, 20))
    sheet.save(out)
    print("Planche : %s (%d echantillons, vues de face / de cote / RTS)" % (out, len(SAMPLES)))
    for (letter, _, gesture, _), ctx in zip(SAMPLES, contexts):
        print("  %s %s : %s" % (letter, gesture, pa.report(ctx)))


if __name__ == "__main__":
    main()
