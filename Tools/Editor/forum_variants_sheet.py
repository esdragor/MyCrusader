"""One picture of the Forum's variants, as the game showed them (Tools/Scenarios/forum_variants.ps1): each <letter>.png of the
folder, cropped round the Forum (the camera of forum_variant.txt puts it there), named, in a grid of four a row. Pillow only.
  python Tools/Editor/forum_variants_sheet.py <folder> <out.png>
"""
import os
import sys

from PIL import Image, ImageDraw

CROP = (60, 40, 760, 520)   # on the 1280 x 720 capture: the whole Forum and its court, the HUD bar and the minimap left out


def main():
    folder, out = sys.argv[1], sys.argv[2]
    names = sorted(f for f in os.listdir(folder) if f.lower().endswith(".png") and len(f) == 5)
    width, height = CROP[2] - CROP[0], CROP[3] - CROP[1]
    columns = 4
    rows = (len(names) + columns - 1) // columns
    sheet = Image.new("RGB", (columns * width, rows * height), (40, 40, 44))
    draw = ImageDraw.Draw(sheet)
    for k, name in enumerate(names):
        tile = Image.open(os.path.join(folder, name)).convert("RGB").crop(CROP)
        x, y = (k % columns) * width, (k // columns) * height
        sheet.paste(tile, (x, y))
        draw.rectangle((x, y, x + 34, y + 30), fill=(0, 0, 0))
        draw.text((x + 10, y + 8), name[0], fill=(255, 255, 255))
    sheet.save(out)
    print(out, len(names), "variants")


if __name__ == "__main__":
    main()
