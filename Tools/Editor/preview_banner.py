"""Offline previews of the Banner of the Dawn (banner_prims.py), no Unreal: what the user validates in J8c.4 loop 4.

A small z-buffer on the very triangles SM_Banner is built from (make_banner_mesh.py), flat-shaded, two-sided.
Panels: face on (the cloth toward the camera), side (the cloth edge on), the RTS view (from above at 55 degrees,
no turn: the camera never rotates), and the same RTS view at the size it has on screen (a hero ~40 px tall) enlarged 3 times beside the
true-size crop. A grey figure 2.2 m tall (the hero) stands beside the pole for scale.
Run:  python Tools/Editor/preview_banner.py [--out <png>]      (default Saved/AnimPreview/Banner_preview.png)
"""
import math
import os
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import numpy as np  # noqa: E402
from PIL import Image, ImageDraw  # noqa: E402

import banner_prims  # noqa: E402
import peasant_props as pp  # noqa: E402

HERO_HEIGHT = 220.0
GROUND = (86, 108, 72)
SKY = (176, 196, 214)
LIGHT = np.array((-0.3, -0.4, 0.86))
LIGHT = LIGHT / np.linalg.norm(LIGHT)
FIGURE_COLOR = (96, 102, 120)


def figure():
    """A plain figure of the hero's height, to the pole's right (+Y), standing 1.4 m off it."""
    centre_y = 140.0
    half = HERO_HEIGHT * 0.5
    prims = [pp.box((0.0, centre_y, HERO_HEIGHT * 0.25), (26.0, 40.0, HERO_HEIGHT * 0.5), FIGURE_COLOR),
             pp.box((0.0, centre_y, HERO_HEIGHT * 0.5 + 40.0), (34.0, 50.0, 80.0), FIGURE_COLOR),
             pp.box((0.0, centre_y, HERO_HEIGHT - 17.0), (24.0, 24.0, 34.0), FIGURE_COLOR)]
    del half
    return [tri for prim in prims for tri in pp.primitive_triangles(prim)]


def render(triangles, yaw, pitch, scale, size, origin_shift=(0.0, 0.0)):
    """Image (size = (w, h) px) of the triangles seen from yaw / pitch degrees at `scale` px per cm; the ground's origin
    (0, 0, 0) is at the image's centre horizontally and 12 % above its bottom, shifted by origin_shift (cm on screen)."""
    w, h = size
    yaw_r, pitch_r = math.radians(yaw), math.radians(pitch)
    d = np.array((math.cos(pitch_r) * math.cos(yaw_r), math.cos(pitch_r) * math.sin(yaw_r), math.sin(pitch_r)))
    right = np.array((-math.sin(yaw_r), math.cos(yaw_r), 0.0))
    up = np.cross(right, -d)
    image = np.zeros((h, w, 3), dtype=np.float64)
    image[:] = SKY
    horizon = int(h * 0.88)
    if pitch > 0.0:
        image[:] = GROUND
    else:
        image[horizon:] = GROUND
    depth = np.full((h, w), -1e9)
    cx, cy = w * 0.5 + origin_shift[0] * scale, h * 0.88 - origin_shift[1] * scale
    for tri, color in triangles:
        p = np.array(tri, dtype=np.float64)
        n = np.cross(p[1] - p[0], p[2] - p[0])
        norm = np.linalg.norm(n)
        if norm < 1e-9:
            continue
        n /= norm
        if np.dot(n, d) < 0.0:
            n = -n
        shade = 0.62 + 0.38 * max(0.0, float(np.dot(n, LIGHT)))
        rgb = np.array(color, dtype=np.float64) * shade
        sx = cx + scale * (p @ right)
        sy = cy - scale * (p @ up)
        z = p @ d
        x0, x1 = int(max(0, math.floor(sx.min()))), int(min(w - 1, math.ceil(sx.max())))
        y0, y1 = int(max(0, math.floor(sy.min()))), int(min(h - 1, math.ceil(sy.max())))
        if x0 > x1 or y0 > y1:
            continue
        gx, gy = np.meshgrid(np.arange(x0, x1 + 1) + 0.5, np.arange(y0, y1 + 1) + 0.5)
        det = (sy[1] - sy[2]) * (sx[0] - sx[2]) + (sx[2] - sx[1]) * (sy[0] - sy[2])
        if abs(det) < 1e-9:
            continue
        l0 = ((sy[1] - sy[2]) * (gx - sx[2]) + (sx[2] - sx[1]) * (gy - sy[2])) / det
        l1 = ((sy[2] - sy[0]) * (gx - sx[2]) + (sx[0] - sx[2]) * (gy - sy[2])) / det
        l2 = 1.0 - l0 - l1
        inside = (l0 >= -1e-6) & (l1 >= -1e-6) & (l2 >= -1e-6)
        zz = l0 * z[0] + l1 * z[1] + l2 * z[2]
        window = depth[y0:y1 + 1, x0:x1 + 1]
        take = inside & (zz > window)
        window[take] = zz[take]
        image[y0:y1 + 1, x0:x1 + 1][take] = rgb
    return Image.fromarray(np.clip(image, 0, 255).astype(np.uint8))


def label(image, text):
    ImageDraw.Draw(image).text((8, 6), text, fill=(20, 20, 20))
    return image


def main():
    out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "Saved", "AnimPreview", "Banner_preview.png")
    if "--out" in sys.argv:
        out = sys.argv[sys.argv.index("--out") + 1]
    tris = banner_prims.triangles()
    both = tris + figure()
    panels = [label(render(both, 180.0, 0.0, 1.55, (420, 640)), "face on, seen from the game camera's side; hero 2.2 m"),
              label(render(both, 90.0, 0.0, 1.55, (420, 640)), "side (cloth edge on)"),
              label(render(both, 180.0, 55.0, 1.55, (420, 640), (0.0, 0.0)), "RTS view as in the game (55 deg, no turn)")]
    # At the size it has on screen: a hero about 40 px tall, enlarged 3 times (nearest) beside the true-size crop.
    small_scale = 40.0 / HERO_HEIGHT
    small = render(both, 180.0, 55.0, small_scale, (140, 180), (0.0, 0.0))
    big = small.resize((small.width * 3, small.height * 3), Image.NEAREST)
    shot = Image.new("RGB", (420 + 140 + 420, 180 * 3), SKY)
    shot.paste(big.crop((0, 0, 420, 540)), (0, 0))
    shot.paste(small, (430, 0))
    label(shot, "on screen: x3 (left), true size (right)")
    sheet = Image.new("RGB", (420 * 3 + 20, 640 + 540 + 10), (240, 240, 240))
    for k, panel in enumerate(panels):
        sheet.paste(panel, (k * 430, 0))
    sheet.paste(shot, (0, 650))
    os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)
    sheet.save(out)
    print("Wrote", os.path.abspath(out), sheet.size)


if __name__ == "__main__":
    main()
