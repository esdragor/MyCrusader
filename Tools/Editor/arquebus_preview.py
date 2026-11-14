"""Previews of the Arquebusier's gestures (no Unreal, needs PIL through preview_animation): a sheet of the attack, and
closeups of chosen frames (one view, large) to judge a gesture by eye.
Run: python Tools/Editor/arquebus_preview.py [gesture] [--frames N] [--zoom Z] [--picks 1,2,3]
     -> Saved/AnimPreview/<gesture>_sheet.png (default gesture: arquebus_attack)
"""
import os
import sys

sys.dont_write_bytecode = True  # no __pycache__ in Tools/Editor
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import arquebus_poses  # noqa: E402,F401
import preview_animation as pa  # noqa: E402


def closeup(ctx, shared, picks, out, view_index=1, zoom=2.6):
    """One view (0 front, 1 side, 2 RTS) of the chosen frames, large."""
    pa.set_zoom(zoom)
    view = pa.views(shared.sk)[view_index]
    canvas = pa.Canvas(pa.CELL_W * len(picks), pa.CELL_H + pa.LABEL_H)
    for col, frame in enumerate(picks):
        pa.draw_cell(canvas, shared, ctx, frame, col * pa.CELL_W, 0, view, "%.2fs f%d" % (ctx["rig"].times[frame], frame))
        if col:
            canvas.line((col * pa.CELL_W, 0), (col * pa.CELL_W, pa.CELL_H + pa.LABEL_H - 1), (210, 210, 205), 0.4)
    canvas.save(out)
    pa.set_zoom(1.0)


def main():
    args = sys.argv[1:]
    frames = int(args[args.index("--frames") + 1]) if "--frames" in args else 12
    zoom = float(args[args.index("--zoom") + 1]) if "--zoom" in args else 1.0
    picks = [int(v) for v in args[args.index("--picks") + 1].split(",")] if "--picks" in args else None
    names = [a for a in args if a in arquebus_poses.GESTURES] or ["arquebus_attack"]
    shared = pa.Shared(False)
    os.makedirs(pa.OUT_DIR, exist_ok=True)
    for name in names:
        ctx = pa.render(shared, name, frames, os.path.join(pa.OUT_DIR, name + "_sheet.png"), zoom, picks=picks)
        print("  " + pa.report(ctx))


if __name__ == "__main__":
    main()
