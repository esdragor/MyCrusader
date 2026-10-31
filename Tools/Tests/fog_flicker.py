"""Flicker of the order lines: the mean change from one frame to the next, in the pixels of the lines.
Usage: fog_flicker.py <folder with flick_00..07.png> <max> <tag>
A line pixel = bright yellow (the move colour) in at least one of the frames, widened by 2 pixels; the change = the mean absolute
difference (0-255, all channels) of the same pixel between two consecutive frames. Also prints the line's coverage (pixels)."""
import sys
import numpy as np
from PIL import Image

folder, limit, tag = sys.argv[1], float(sys.argv[2]), sys.argv[3]
frames = [np.asarray(Image.open("%s/flick_%02d.png" % (folder, n)).convert("RGB")).astype(np.int16) for n in range(8)]
if frames[0][100:500, 300:900].mean() < 5:
    print("fog_flicker : black frames (the world was not drawn)")
    sys.exit(3)
def yellow(f):
    r, g, b = f[..., 0], f[..., 1], f[..., 2]
    return (r > 120) & (g > 90) & (r - b > 70) & (g - b > 50)
mask = np.zeros(frames[0].shape[:2], bool)
for f in frames:
    mask |= yellow(f)
def dilate(m, n):
    out = m.copy()
    for dy in range(-n, n + 1):
        for dx in range(-n, n + 1):
            out |= np.roll(np.roll(m, dy, 0), dx, 1)
    return out
mask = dilate(mask, 2)
# Only the static segment between the two steps (the units' own segment moves a little): a fixed window of the scenario's camera
# (1280 x 720 capture), x 650-1000, y 170-260.
window = np.zeros_like(mask)
window[170:260, 650:1000] = True
mask &= window
def lum(f):
    return f.mean(axis=2)
def box(a, n):
    out = np.zeros_like(a, dtype=float)
    for dy in range(-n, n + 1):
        for dx in range(-n, n + 1):
            out += np.roll(np.roll(a, dy, 0), dx, 1)
    return out / (2 * n + 1) ** 2
# The line's energy in a frame: its luminance above the local background (a 11 x 11 mean), over the window.
energy = []
for f in frames:
    l = lum(f)
    energy.append(np.maximum(0.0, l - box(l, 5))[window].sum())
churn = [np.abs(lum(frames[i + 1]) - lum(frames[i]))[mask].sum() for i in range(7)]
mean_energy = float(np.mean(energy))
# flicker = the line's pixels changing from one frame to the next, relative to what the line is made of; wobble = its total energy's swing.
flicker = float(np.mean(churn)) / max(mean_energy, 1.0)
wobble = float(np.std(energy)) / max(mean_energy, 1.0)
coverage = np.mean([yellow(f).sum() for f in frames])
ok = flicker <= limit and mask.sum() > 20
print("fog_flicker : %s %s (churn/energy %.3f, wobble %.3f, line pixels %d, mean yellow pixels %d, limit %.2f)"
      % ("PASS" if ok else "FAIL", tag, flicker, wobble, mask.sum(), coverage, limit))
sys.exit(0 if ok else 1)
