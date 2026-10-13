"""Contact sheet of the end-to-end captures (D246): <prefix>_<screen>.png -> <prefix>.png, two columns at half size.
  python make_sheet.py <screenshot folder> <prefix> <screen> [<screen>...]
Needs Pillow; skipped with a message without it.
"""
import os
import sys

try:
    from PIL import Image
except ImportError:
    print("make_sheet: Pillow not installed, no contact sheet")
    sys.exit(0)

folder, prefix, screens = sys.argv[1], sys.argv[2], sys.argv[3:]
width, height = 800, 450
images = [os.path.join(folder, "%s_%s.png" % (prefix, screen)) for screen in screens]
images = [path for path in images if os.path.exists(path)]
rows = (len(images) + 1) // 2
sheet = Image.new("RGB", (width * 2, height * max(rows, 1)))
for index, path in enumerate(images):
    picture = Image.open(path).convert("RGB").resize((width, height), Image.LANCZOS)
    sheet.paste(picture, ((index % 2) * width, (index // 2) * height))
out = os.path.join(folder, prefix + ".png")
sheet.save(out)
print("make_sheet: %s (%d captures)" % (out, len(images)))
