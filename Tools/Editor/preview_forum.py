"""Offline preview of the Forum's pieces and layout (VisualInProgress.md, V2): no Unreal, numpy and Pillow only.

Reads Saved/ForumPieces.json (dump_forum_pieces.py: the kit pieces' triangles, Unreal space, cm, one material slot a
triangle) and draws flat-shaded views as the RTS camera sees them (from above at 55 degrees; the piece's slots tinted
stone, wood, ...). Unreal is left-handed (X forward, Y right, Z up): the view is drawn so that right on the screen is
+Y when looking along +X, as the game would show it. Not the real materials: for the shapes and the sides pieces face.
Run:
  python Tools/Editor/preview_forum.py --sheet [--azimuth A] [--out <png>]    every piece in a grid, named
  python Tools/Editor/preview_forum.py --layout [--azimuth A] [--out <png>]   the Forum from forum_layout.LAYOUT
"""
import argparse
import json
import math
import os
import sys

import numpy as np
from PIL import Image, ImageDraw

sys.dont_write_bytecode = True
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import forum_layout  # noqa: E402

SAVED = os.path.join(os.path.dirname(os.path.dirname(HERE)), "Saved")
PITCH = math.radians(55.0)
SUN = np.array([0.4, -0.5, 0.75])
SUN = SUN / np.linalg.norm(SUN)
TINTS = [(158, 152, 144), (140, 98, 58), (120, 130, 150), (150, 90, 90), (110, 140, 100)]
BACKGROUND = (58, 60, 66)
BRIGHTEN = 1.9  # the kit's textures are dark on average: lifted for the eye, not a game value


def texture_colours():
    """material name -> mean colour of its texture, from the kit's .mtl files (the slot names are the FBX material names)."""
    root = os.path.join(os.path.dirname(forum_layout.KIT), "OBJ format")
    colours = {}
    cache = {}
    for file in os.listdir(root):
        if not file.endswith(".mtl"):
            continue
        name = None
        for line in open(os.path.join(root, file)):
            parts = line.split()
            if len(parts) >= 2 and parts[0] == "newmtl":
                name = parts[1]
            elif len(parts) >= 2 and parts[0] == "map_Kd" and name:
                texture = os.path.join(root, parts[1])
                if texture not in cache and os.path.isfile(texture):
                    pixels = np.asarray(Image.open(texture).convert("RGB"), dtype=float).reshape(-1, 3)
                    cache[texture] = tuple(pixels.mean(axis=0))
                if texture in cache:
                    colours[name] = cache[texture]
    return colours


def load():
    with open(os.path.join(SAVED, "ForumPieces.json")) as handle:
        data = json.load(handle)
    colours = texture_colours()
    pieces, slots, palettes = {}, {}, {}
    for name, piece in data.items():
        tris = piece["tris"]
        pieces[name] = np.array([t[:3] for t in tris], dtype=float).reshape(-1, 3, 3)
        slots[name] = np.array([t[3] for t in tris], dtype=int)
        palettes[name] = [colours.get(slot, TINTS[i % len(TINTS)]) for i, slot in enumerate(piece["slots"])]
    vertices, _, _, triangles = forum_layout.pyramid_unit()
    pieces["@pyramid"] = np.array([[vertices[i], vertices[j], vertices[k]] for i, j, k in triangles], dtype=float)
    slots["@pyramid"] = np.zeros(len(triangles), dtype=int)
    palettes["@pyramid"] = [palettes["roof"][0] if "roof" in palettes else TINTS[2]]
    return pieces, slots, palettes


def camera(azimuth_deg):
    a = math.radians(azimuth_deg)
    v = np.array([math.cos(PITCH) * math.cos(a), math.cos(PITCH) * math.sin(a), math.sin(PITCH)])  # towards the camera
    right = np.array([math.sin(a), -math.cos(a), 0.0])
    up = np.array([0.0, 0.0, 1.0]) - v * v[2]
    return v, right, up / np.linalg.norm(up)


def render(items, azimuth, size, scale, centre):
    """items: list of (triangles (n,3,3) already placed, colour (n,3) base colour of each triangle); returns an image."""
    v, right, up = camera(azimuth)
    image = Image.new("RGB", size, BACKGROUND)
    draw = ImageDraw.Draw(image)
    tris = np.concatenate([t for t, _ in items])
    base = np.concatenate([c for _, c in items])
    a, b, c = tris[:, 0], tris[:, 1], tris[:, 2]
    normal = -np.cross(b - a, c - a)  # Unreal's winding: (b - a) x (c - a) points into the solid
    length = np.linalg.norm(normal, axis=1)
    keep = length > 1e-9
    normal[keep] = normal[keep] / length[keep][:, None]
    facing = normal @ v
    depth = tris.mean(axis=1) @ v
    order = np.argsort(depth)
    cx, cy = size[0] / 2, size[1] / 2
    for i in order:
        if not keep[i] or facing[i] <= 0.0:
            continue
        light = 0.55 + 0.45 * max(0.0, float(normal[i] @ SUN))
        colour = tuple(int(min(255, ch * light * BRIGHTEN)) for ch in base[i])
        points = [(cx + (p - centre) @ right * scale, cy - (p - centre) @ up * scale) for p in tris[i]]
        draw.polygon(points, fill=colour, outline=tuple(int(ch * 0.55) for ch in colour))
    return image


def colours_of(slots, palette):
    return np.array([palette[i % len(palette)] for i in slots], dtype=float).reshape(-1, 3)


def sheet(pieces, slots, palettes, azimuth, out):
    names = list(pieces)
    columns = 7
    cell = 190
    rows = (len(names) + columns - 1) // columns
    image = Image.new("RGB", (columns * cell, rows * cell), BACKGROUND)
    draw = ImageDraw.Draw(image)
    for k, name in enumerate(names):
        tris = pieces[name]
        centre = tris.reshape(-1, 3).mean(axis=0)
        extent = np.abs(tris.reshape(-1, 3) - centre).max()
        tile = render([(tris, colours_of(slots[name], palettes[name]))], azimuth, (cell, cell), (cell * 0.36) / max(extent, 1.0), centre)
        x, y = (k % columns) * cell, (k // columns) * cell
        image.paste(tile, (x, y))
        draw.text((x + 4, y + 3), name, fill=(255, 255, 255))
    image.save(out)


def place(pieces, slots, palettes, layout_items):
    items = []
    s = forum_layout.SCALE
    for name, x, y, z, yaw, scale in layout_items:
        tris = pieces[name] * (s * np.array(scale))
        a = math.radians(yaw)
        rot = np.array([[math.cos(a), -math.sin(a), 0.0], [math.sin(a), math.cos(a), 0.0], [0.0, 0.0, 1.0]])
        tris = tris @ rot.T + np.array([x, y, z]) * forum_layout.MODULE * s
        items.append((tris, colours_of(slots[name], palettes[name])))
    return items


def layout(pieces, slots, palettes, azimuth, out, variant=None):
    items = place(pieces, slots, palettes, forum_layout.VARIANTS[variant]() if variant else forum_layout.LAYOUT)
    render(items, azimuth, (1000, 800), 0.42, np.array([0.0, 0.0, 400.0])).save(out)


def variants(pieces, slots, palettes, azimuth, out, letters=None):
    """The given variants (default: all), named, in a grid of five a row. Seen from azimuth, from a little way above."""
    tile = (520, 420)
    names = [n.upper() for n in letters] if letters else sorted(forum_layout.VARIANTS)
    rows = (len(names) + 4) // 5
    image = Image.new("RGB", (5 * tile[0], rows * tile[1]), BACKGROUND)
    draw = ImageDraw.Draw(image)
    for k, name in enumerate(names):
        items = place(pieces, slots, palettes, forum_layout.VARIANTS[name]())
        image.paste(render(items, azimuth, tile, 0.2, np.array([0.0, 0.0, 330.0])), ((k % 5) * tile[0], (k // 5) * tile[1]))
        draw.text(((k % 5) * tile[0] + 8, (k // 5) * tile[1] + 6), name, fill=(255, 255, 255))
    image.save(out)


def yaws(pieces, slots, palettes, names, azimuth, out):
    """Each named piece four times, turned 0, 90, 180 and 270 degrees: which way its door or window faces."""
    cell = 260
    image = Image.new("RGB", (4 * cell, len(names) * cell), BACKGROUND)
    draw = ImageDraw.Draw(image)
    for row, name in enumerate(names):
        for column, yaw in enumerate((0.0, 90.0, 180.0, 270.0)):
            a = math.radians(yaw)
            rot = np.array([[math.cos(a), -math.sin(a), 0.0], [math.sin(a), math.cos(a), 0.0], [0.0, 0.0, 1.0]])
            tris = pieces[name] @ rot.T
            tile = render([(tris, colours_of(slots[name], palettes[name]))], azimuth, (cell, cell), cell * 0.0034 * 1.3, np.array([0.0, 0.0, 50.0]))
            image.paste(tile, (column * cell, row * cell))
            draw.text((column * cell + 4, row * cell + 3), "%s yaw %d" % (name, yaw), fill=(255, 255, 255))
    image.save(out)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--yaws", default=None, help="comma-separated pieces: each turned 0/90/180/270")
    parser.add_argument("--sheet", action="store_true")
    parser.add_argument("--layout", action="store_true")
    parser.add_argument("--variants", action="store_true", help="the variants in one grid")
    parser.add_argument("--letters", default=None, help="with --variants: only these (K,L,M...)")
    parser.add_argument("--variant", default=None, help="with --layout: this variant (default forum_layout.CHOSEN)")
    parser.add_argument("--azimuth", type=float, default=180.0)
    parser.add_argument("--out", default=None)
    args = parser.parse_args()
    pieces, slots, palettes = load()
    if args.variants:
        out = args.out or os.path.join(SAVED, "forum_variants_preview.png")
        variants(pieces, slots, palettes, args.azimuth, out, args.letters.split(",") if args.letters else None)
    elif args.yaws:
        out = args.out or os.path.join(SAVED, "forum_yaws.png")
        yaws(pieces, slots, palettes, args.yaws.split(","), args.azimuth, out)
    elif args.sheet:
        out = args.out or os.path.join(SAVED, "forum_pieces_sheet.png")
        sheet(pieces, slots, palettes, args.azimuth, out)
    else:
        out = args.out or os.path.join(SAVED, "forum_layout.png")
        layout(pieces, slots, palettes, args.azimuth, out, args.variant)
    print(out)


if __name__ == "__main__":
    main()
