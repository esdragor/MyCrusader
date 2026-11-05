"""Prepares the ground detail textures (chantier Cartes, sol CC0 etape 1) from the CC0 albedos in Tools/Data/GroundSources
(Poly Haven, registry: Docs/AssetsExternes.md). Run with the system Python (Pillow and numpy), not the editor's:
  python Tools/prepare_ground_details.py
For each <name>_diff_1k.jpg it writes <name>_detail.png (RGB, 1024 x 1024, tileable as the source): the albedo in LINEAR light divided
by its own mean per channel, then halved, so that the texture is a pure variation around 0.5 (the material multiplies it by 2 and by the map's
ColorA: the map keeps the average colour it chose, the texture only adds the grass's mottling). Import it as a linear texture (no sRGB):
Tools/Editor/make_ground_materials.py does.
"""
import glob
import os

import numpy as np
from PIL import Image

CHROMA = 0.5  # 0 = brightness only, 1 = every channel by its own mean
HERE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Data", "GroundSources")


def to_linear(c):
    return np.where(c <= 0.04045, c / 12.92, ((c + 0.055) / 1.055) ** 2.4)


for path in sorted(glob.glob(os.path.join(HERE, "*_diff_1k.jpg"))):
    name = os.path.basename(path)[: -len("_diff_1k.jpg")]
    rgb = np.asarray(Image.open(path).convert("RGB"), dtype=np.float64) / 255.0
    linear = to_linear(rgb)
    mean = linear.reshape(-1, 3).mean(axis=0)
    ratio = linear / mean
    grey = ratio @ np.array([0.2126, 0.7152, 0.0722]) / (np.array([0.2126, 0.7152, 0.0722]) @ np.ones(3))  # brightness variation, mean 1
    ratio = grey[..., None] + CHROMA * (ratio - grey[..., None])  # the colour variation, kept only in part (a blue channel near 0 is noise)
    detail = np.clip(ratio / 2.0, 0.0, 1.0)  # 0.5 = the mean; the top (x2) clips only above twice the mean
    clipped = float((ratio > 2.0).mean())
    out = os.path.join(HERE, name + "_detail.png")
    Image.fromarray((detail * 255.0 + 0.5).astype(np.uint8)).save(out)
    print("%s: mean linear %s, std per channel %s, clipped above x2: %.3f%%" % (name, np.round(mean, 3), np.round(ratio.reshape(-1, 3).std(axis=0), 3), clipped * 100))
