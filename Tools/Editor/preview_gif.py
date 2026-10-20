import os
import sys
import tempfile
sys.dont_write_bytecode = True
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "Tools", "Editor"))
import preview_animation as pa  # noqa: E402
from PIL import Image  # noqa: E402

shared = pa.Shared(False)
tmp = tempfile.mkdtemp(dir=os.path.dirname(os.path.abspath(__file__)))
for name in sys.argv[1:]:
    ctx = pa.prepare(shared, name)
    n = len(ctx["rig"].times) - 1  # last frame = first
    frames = []
    for k in range(n):
        path = os.path.join(tmp, "%s_%03d.png" % (name, k))
        pa.render(shared, name, 1, path, 1.5, ctx, [k])
        img = Image.open(path).convert("RGB")
        w, h = img.size
        img = img.crop((0, h // 3, w, h))  # side and RTS views
        frames.append(img.convert("P", palette=Image.ADAPTIVE))
    out = os.path.join(ROOT, "Saved", "AnimPreview", name + ".gif")
    frames[0].save(out, save_all=True, append_images=frames[1:], duration=int(1000 / ctx["rig"].fps), loop=0)
    print(out)
