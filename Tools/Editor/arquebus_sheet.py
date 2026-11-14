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

from arquebus_props import (ARQ, BUTT_X, FLASH, MATCH, MUSKET, SMOKE, STRAP, blob, make_bandolier, make_gun,  # noqa: E402,F401
                            make_smoke)




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
