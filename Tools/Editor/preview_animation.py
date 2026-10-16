"""Offline contact sheet of a scripted gesture (no Unreal, no numpy): stick figure and tool, frame by frame.

Rows: front view (facing him, his right on the left of the image), side view (from his right, forward to the right),
RTS view (from above at 55 degrees, front-right). Columns: frames spread over the loop, time in seconds under each
cell. Red = right limbs, blue = left limbs, brown = handle (or the carried load), dark = axe blade or pick head; grey
dots = path of the tool's head (or of the right hand) over the whole loop; green = the scenery of the gesture's
"scene": trunk or rock placed where the tool's head is at the hit (fastest-stopping frame, marked by a dash), bush and
pouch where the gesture puts them; the carried load of a loaded walk ("prop": logs, sack) or the box between the
hands ("scene": "load") in brown.
Run:
  python Tools/Editor/preview_animation.py [gesture] [--frames N] [--zoom Z] [--out <png>]
  python Tools/Editor/preview_animation.py --compare g1,g2,g3 [--at 0.25] [--zoom Z] [--out <png>]
      (several gestures side by side at the same point of their loops; --at is the share of the loop, 0-1)
(default output: Saved/AnimPreview/<gesture>_contact_sheet.png, outside git).
The skeleton (and the source walk of a loaded walk) come from Tools/Editor/Data/, written by
make_peasant_animations.py.
"""
import math
import os
import struct
import sys
import zlib

sys.dont_write_bytecode = True  # no __pycache__ in Tools/Editor
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import anim_rig  # noqa: E402
import peasant_poses  # noqa: E402

DATA_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Data")
SKELETON_JSON = os.path.join(DATA_DIR, "UE4MannequinSkeleton.json")

CELL_W, CELL_H, LABEL_H = 200, 230, 14
PX_PER_CM = 0.8
BACKGROUND = (246, 244, 238)
COLORS = {"c": (60, 60, 60), "r": (205, 45, 45), "l": (45, 85, 205), "handle": (130, 80, 30), "blade": (30, 30, 40),
          "trace": (175, 175, 175), "ground": (150, 150, 140), "trunk": (70, 140, 60), "text": (40, 40, 40)}
SEGMENTS = [
    ("c", ["pelvis", "spine_01", "spine_02", "spine_03", "neck_01", "head"]),
    ("r", ["spine_03", "clavicle_r", "upperarm_r", "lowerarm_r", "hand_r", "middle_01_r"]),
    ("l", ["spine_03", "clavicle_l", "upperarm_l", "lowerarm_l", "hand_l", "middle_01_l"]),
    ("r", ["pelvis", "thigh_r", "calf_r", "foot_r", "ball_r"]),
    ("l", ["pelvis", "thigh_l", "calf_l", "foot_l", "ball_l"]),
]

# 3x5 digit font (and '.', 's') for the time labels.
FONT = {"0": "111101101101111", "1": "010110010010111", "2": "111001111100111", "3": "111001111001111",
        "4": "101101111001001", "5": "111100111001111", "6": "111100111101111", "7": "111001001001001",
        "8": "111101111101111", "9": "111101111001111", ".": "000000000000010", "s": "000011110011110",
        "-": "000000111000000"}


class Canvas:
    def __init__(self, width, height):
        self.w, self.h = width, height
        self.pixels = bytearray(BACKGROUND * (width * height))

    def dot(self, x, y, color, radius=1.0):
        r = int(math.ceil(radius))
        for dy in range(-r, r + 1):
            for dx in range(-r, r + 1):
                if dx * dx + dy * dy <= radius * radius + 0.25:
                    px, py = int(round(x)) + dx, int(round(y)) + dy
                    if 0 <= px < self.w and 0 <= py < self.h:
                        i = (py * self.w + px) * 3
                        self.pixels[i:i + 3] = bytes(color)

    def line(self, a, b, color, radius=1.5):
        steps = int(max(abs(b[0] - a[0]), abs(b[1] - a[1]))) + 1
        for k in range(steps + 1):
            s = k / steps
            self.dot(a[0] + (b[0] - a[0]) * s, a[1] + (b[1] - a[1]) * s, color, radius)

    def text(self, x, y, string, color, size=2):
        for ch in string:
            glyph = FONT.get(ch)
            if glyph:
                for i, bit in enumerate(glyph):
                    if bit == "1":
                        for dy in range(size):
                            for dx in range(size):
                                px, py = x + (i % 3) * size + dx, y + (i // 3) * size + dy
                                if 0 <= px < self.w and 0 <= py < self.h:
                                    j = (py * self.w + px) * 3
                                    self.pixels[j:j + 3] = bytes(color)
            x += 4 * size

    def save(self, path):
        raw = b"".join(b"\x00" + bytes(self.pixels[y * self.w * 3:(y + 1) * self.w * 3]) for y in range(self.h))

        def chunk(kind, data):
            return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)

        png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", self.w, self.h, 8, 2, 0, 0, 0))
        png += chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b"")
        with open(path, "wb") as f:
            f.write(png)


def views(sk):
    """Projections (body-term point -> screen x, y up, depth) for the three rows; and each row's horizontal centre."""
    def body(p):
        return (anim_rig.dot(p, sk.right), anim_rig.dot(p, sk.forward), anim_rig.dot(p, sk.up))

    def front(p):
        r, f, u = body(p)
        return (-r, u)

    def side(p):
        r, f, u = body(p)
        return (f, u)

    yaw, pitch = math.radians(35.0), math.radians(55.0)

    def rts(p):
        r, f, u = body(p)
        # Camera in front-right of him, looking down: screen x across the view, y mixes height and depth.
        x = -r * math.cos(yaw) + f * math.sin(yaw)
        depth = -r * math.sin(yaw) - f * math.cos(yaw)
        return (x, u * math.cos(pitch) + depth * math.sin(pitch) * 0.6 + 40.0)

    return [(front, 0.0), (side, 20.0), (rts, 0.0)]


def tool_edge(world, sk, axe, edge_local):
    """Direction the tool's striking edge points: fixed in the right hand (the tool is held rigidly), across the
    handle."""
    edge = anim_rig.qrot(sk.rot("hand_r", world), edge_local)
    d = axe["dir"]
    return anim_rig.normalize(anim_rig.sub(edge, anim_rig.scale(d, anim_rig.dot(edge, d))))


def strike_edge_local(worlds, poses, sk, hit):
    """The edge, in the right hand's frame, that leads the head's motion into the hit."""
    velocity = anim_rig.sub(poses[hit][1]["head"], poses[hit - 1][1]["head"])
    return anim_rig.qrot(anim_rig.qconj(sk.rot("hand_r", worlds[hit])), velocity)


def tool_lines(world, sk, axe, kind, edge_local):
    """Head of the tool: an axe blade on the striking side, or a two-sided pick across the handle."""
    d, top = axe["dir"], axe["head"]
    edge = tool_edge(world, sk, axe, edge_local)
    if kind == "pick":
        tip_front = anim_rig.sub(anim_rig.add(top, anim_rig.scale(edge, 24.0)), anim_rig.scale(d, 6.0))
        tip_back = anim_rig.sub(anim_rig.add(top, anim_rig.scale(edge, -22.0)), anim_rig.scale(d, 5.0))
        return [(top, tip_front), (top, tip_back)]
    low = anim_rig.sub(top, anim_rig.scale(d, 14.0))
    tip_top = anim_rig.add(top, anim_rig.scale(edge, 12.0))
    tip_low = anim_rig.add(low, anim_rig.scale(edge, 12.0))
    return [(top, tip_top), (tip_top, tip_low), (tip_low, low), (low, top)]


def box_lines(sk, centre, half):
    """Edges of a box aligned with the body axes; centre and half sizes in (right, forward, up) cm."""
    corners = {}
    for i in (-1, 1):
        for j in (-1, 1):
            for k in (-1, 1):
                corner = (centre[0] + i * half[0], centre[1] + j * half[1], centre[2] + k * half[2])
                corners[(i, j, k)] = sk.body(corner)
    lines = []
    for (i, j, k), p in corners.items():
        for other in ((-i, j, k), (i, -j, k), (i, j, -k)):
            if other > (i, j, k):
                lines.append((p, corners[other]))
    return lines


def ellipsoid_lines(sk, centre, radii, steps=28):
    """Three body-plane ellipses of an ellipsoid (the bush)."""
    lines = []
    for a, b in ((0, 1), (0, 2), (1, 2)):
        points = []
        for k in range(steps + 1):
            angle = 2 * math.pi * k / steps
            v = list(centre)
            v[a] += radii[a] * math.cos(angle)
            v[b] += radii[b] * math.sin(angle)
            points.append(sk.body(v))
        lines += list(zip(points, points[1:]))
    return lines


def body_terms(sk, p):
    return (anim_rig.dot(p, sk.right), anim_rig.dot(p, sk.forward), anim_rig.dot(p, sk.up))


def prepare(sk, gesture_name):
    """Everything a cell of this gesture needs: solved frames, hit, scenery."""
    gesture = peasant_poses.GESTURES[gesture_name]
    rig = anim_rig.Rig(sk, gesture, anim_rig.load_base(gesture, DATA_DIR))
    poses = [rig.pose(t) for t in rig.times]
    worlds = [sk.fk(local) for local, _ in poses]
    ctx = {"gesture": gesture, "rig": rig, "poses": poses, "worlds": worlds,
           "traces": [extra["trace"] for _, extra in poses],
           "reach": max(max(rig.grip_errors(world, extra).values()) for world, (_, extra) in zip(worlds, poses)),
           "has_tool": "head" in poses[0][1], "kind": gesture.get("tool", {}).get("kind", "axe"),
           "scene": gesture.get("scene", {"kind": "trunk"}), "hit": None, "summary": "", "scenery": [],
           "edge_local": None}
    traces, scene = ctx["traces"], ctx["scene"]
    if ctx["has_tool"]:
        # The hit: the frame where the tool's head stops hardest (largest drop of speed).
        speeds = [anim_rig.length(anim_rig.sub(traces[i + 1], traces[i])) * rig.fps for i in range(len(traces) - 1)]
        hit = max(range(1, len(speeds)), key=lambda i: speeds[i - 1] - speeds[i])
        ctx["hit"] = hit
        ctx["edge_local"] = strike_edge_local(worlds, poses, sk, hit)
        r, f, u = body_terms(sk, traces[hit])
        ctx["summary"] = ", frappe a %.2f s, tete de l'outil a la frappe : droite %.0f, avant %.0f, haut %.0f cm" % (
            rig.times[hit], r, f, u)
    hit = ctx["hit"]
    if scene["kind"] == "trunk" and hit is not None:
        r, f, u = body_terms(sk, traces[hit])
        ctx["scenery"].append(((sk.body((r, f, 0.0)), sk.body((r, f, 200.0))), 2.5))
    elif scene["kind"] == "rock" and hit is not None:
        # The rock's top is where the striking tip of the head is at the hit.
        tip = tool_lines(worlds[hit], sk, poses[hit][1], ctx["kind"], ctx["edge_local"])[0][1]
        r, f, u = body_terms(sk, tip)
        height = max(10.0, u)
        ctx["scenery"] += [(line, 1.6) for line in box_lines(sk, (r, f + 10.0, height / 2), (24.0, 18.0, height / 2))]
        ctx["summary"] += ", pointe a la frappe : avant %.0f, haut %.0f cm" % (f, u)
    elif scene["kind"] == "bush":
        ctx["scenery"] += [(line, 1.2) for line in ellipsoid_lines(sk, scene["centre"], scene["radii"])]
        if "pouch" in scene:
            ctx["scenery"] += [(line, 1.2) for line in box_lines(sk, scene["pouch"], (6.0, 8.0, 8.0))]
    return ctx


def prop_lines(sk, world, prop):
    """The carried load ("prop": logs or sack), placed in the reference posture and carried by the chest."""
    chest = sk.index["spine_03"]
    motion = anim_rig.qmul(world[chest][1], anim_rig.qconj(sk.ref_world[chest][1]))

    def place(p):
        return anim_rig.add(world[chest][0], anim_rig.qrot(motion, anim_rig.sub(p, sk.ref_world[chest][0])))

    lines = []
    if prop["kind"] == "logs":
        centre, axis = sk.body(prop["centre"]), anim_rig.normalize(sk.body(prop["dir"]))
        u = anim_rig.normalize(anim_rig.cross(axis, sk.up))
        v = anim_rig.cross(u, axis)
        half, r = prop["length"] / 2, prop.get("radius", 5.0)
        offsets = ((-r, -r), (r, -r), (-r, r), (r, r))[:prop.get("count", 4)]
        for a, b in offsets:
            mid = anim_rig.add(centre, anim_rig.add(anim_rig.scale(u, a), anim_rig.scale(v, b)))
            lines.append(((place(anim_rig.sub(mid, anim_rig.scale(axis, half))),
                           place(anim_rig.add(mid, anim_rig.scale(axis, half)))), 2.6))
    elif prop["kind"] == "sack":
        lines += [((place(a), place(b)), 1.6) for a, b in ellipsoid_lines(sk, prop["centre"], prop["radii"])]
    return lines


def draw_cell(canvas, sk, ctx, frame, ox, oy, project, centre, label):
    def screen(p):
        x, y = project(p)
        return (ox + CELL_W / 2 + (x - centre) * PX_PER_CM, oy + CELL_H - 12 - y * PX_PER_CM)

    # Ground, scenery (trunk, rock, bush and pouch) and the path of the tool's head or right hand.
    canvas.line(screen(anim_rig.scale(sk.right, 100.0)), screen(anim_rig.scale(sk.right, -100.0)),
                COLORS["ground"], 0.5)
    canvas.line(screen(anim_rig.scale(sk.forward, 120.0)), screen(anim_rig.scale(sk.forward, -80.0)),
                COLORS["ground"], 0.5)
    for (a, b), width in ctx["scenery"]:
        canvas.line(screen(a), screen(b), COLORS["trunk"], width)
    for point in ctx["traces"]:
        canvas.dot(*screen(point), COLORS["trace"], 1.0)

    world = ctx["worlds"][frame]
    extra = ctx["poses"][frame][1]
    scene = ctx["scene"]
    if scene["kind"] == "load":
        mid = body_terms(sk, anim_rig.scale(anim_rig.add(extra["grip_l"], extra["grip_r"]), 0.5))
        half = scene.get("half", (14.0, 11.0, 10.0))
        for a, b in box_lines(sk, (mid[0], mid[1] - 2.0, mid[2] + 4.0), half):
            canvas.line(screen(a), screen(b), COLORS["handle"], 1.2)
    prop = ctx["gesture"].get("prop")
    if prop:
        for (a, b), width in prop_lines(sk, world, prop):
            canvas.line(screen(a), screen(b), COLORS["handle"], width)
    for color, chain in SEGMENTS:
        for a, b in zip(chain, chain[1:]):
            canvas.line(screen(sk.pos(a, world)), screen(sk.pos(b, world)), COLORS[color], 1.6)
    # Head: a ring around the head bone, lifted to the skull's centre.
    hx, hy = screen(anim_rig.add(sk.pos("head", world), (0.0, 0.0, 9.0)))
    for k in range(24):
        a = 2 * math.pi * k / 24
        canvas.dot(hx + 9 * math.cos(a), hy + 9 * math.sin(a), COLORS["c"], 1.0)
    if ctx["has_tool"]:
        kind = ctx["kind"]
        canvas.line(screen(extra["knob"]), screen(extra["head"]), COLORS["handle"], 1.8)
        for a, b in tool_lines(world, sk, extra, kind, ctx["edge_local"]):
            canvas.line(screen(a), screen(b), COLORS["blade"], 1.6 if kind == "pick" else 1.4)
    canvas.text(ox + 6, oy + CELL_H + 1, label, COLORS["text"])
    if frame == ctx["hit"]:
        canvas.text(ox + CELL_W - 30, oy + CELL_H + 1, "-", COLORS["trunk"])


def set_zoom(zoom):
    global CELL_W, CELL_H, PX_PER_CM
    CELL_W, CELL_H, PX_PER_CM = int(CELL_W * zoom), int(CELL_H * zoom), PX_PER_CM * zoom


def render(gesture_name, frames, out_path, zoom=1.0):
    set_zoom(zoom)
    sk = anim_rig.Skeleton.load(SKELETON_JSON)
    ctx = prepare(sk, gesture_name)
    rig = ctx["rig"]
    picks = [int(round(i * (len(rig.times) - 1) / frames)) for i in range(frames)]
    canvas = Canvas(CELL_W * frames, (CELL_H + LABEL_H) * 3)
    for row, (project, centre) in enumerate(views(sk)):
        oy = row * (CELL_H + LABEL_H)
        for col, frame in enumerate(picks):
            draw_cell(canvas, sk, ctx, frame, col * CELL_W, oy, project, centre, "%.2fs" % rig.times[frame])
        for col in range(1, frames):
            canvas.line((col * CELL_W, oy), (col * CELL_W, oy + CELL_H + LABEL_H - 1), (210, 210, 205), 0.4)
    canvas.save(out_path)
    print("Planche : %s (%d images, boucle de %.3f s a %.2f i/s%s ; mains hors de leur cible au plus de %.1f cm)"
          % (out_path, frames, rig.length, rig.fps, ctx["summary"], ctx["reach"]))


def compare(gesture_names, fraction, out_path, zoom=1.0):
    """Several gestures side by side at the same point of their loops (`fraction` of the loop, 0-1)."""
    set_zoom(zoom)
    sk = anim_rig.Skeleton.load(SKELETON_JSON)
    contexts = [prepare(sk, name) for name in gesture_names]
    canvas = Canvas(CELL_W * len(contexts), (CELL_H + LABEL_H) * 3)
    for row, (project, centre) in enumerate(views(sk)):
        oy = row * (CELL_H + LABEL_H)
        for col, ctx in enumerate(contexts):
            frame = int(round(fraction * (len(ctx["rig"].times) - 1)))
            draw_cell(canvas, sk, ctx, frame, col * CELL_W, oy, project, centre, "%.2fs" % ctx["rig"].times[frame])
        for col in range(1, len(contexts)):
            canvas.line((col * CELL_W, oy), (col * CELL_W, oy + CELL_H + LABEL_H - 1), (210, 210, 205), 0.4)
    canvas.save(out_path)
    print("Comparaison : %s (%s, a %.0f %% de la boucle)" % (out_path, ", ".join(gesture_names), fraction * 100))


def main():
    args = sys.argv[1:]
    gesture = "chop_wood"
    frames = 12
    out = None
    zoom = 1.0
    together = None
    fraction = 0.25
    i = 0
    while i < len(args):
        if args[i] == "--frames":
            frames = int(args[i + 1])
            i += 1
        elif args[i] == "--zoom":
            zoom = float(args[i + 1])
            i += 1
        elif args[i] == "--out":
            out = args[i + 1]
            i += 1
        elif args[i] == "--compare":
            together = args[i + 1].split(",")
            i += 1
        elif args[i] == "--at":
            fraction = float(args[i + 1])
            i += 1
        else:
            gesture = args[i]
        i += 1
    if out is None:
        folder = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "Saved", "AnimPreview")
        os.makedirs(folder, exist_ok=True)
        name = "_".join(together) + "_compare" if together else gesture + "_contact_sheet"
        out = os.path.normpath(os.path.join(folder, name + ".png"))
    if together:
        compare(together, fraction, out, zoom)
    else:
        render(gesture, frames, out, zoom)


if __name__ == "__main__":
    main()
