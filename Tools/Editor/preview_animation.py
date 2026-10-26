"""Offline contact sheet of a scripted gesture (no Unreal, no numpy): body, stick figure and props, frame by frame.

Rows: front view (facing him, his right on the left of the image), side view (from his right, forward to the right),
RTS view (from above at 55 degrees, front-right). Columns: frames spread over the loop, time in seconds under each
cell. Grey dots = the Peasant's mesh (Data/SKM_Mannequin_Body.json, each vertex carried rigidly by the bone that
weighs most on it); red = right limbs, blue = left limbs; the PROPS of the gesture ("props", peasant_props.py) are
drawn filled, on their bones, exactly as the mesh will show them (axe and pickaxe on the tool bone ik_hand_gun, loads
on spine_03), hidden where they are behind something. Grey dots on the ground = path of the tool's head (or of the
right hand) over the loop; green = the scenery of the gesture's "scene": trunk or rock placed where the tool's head
is at the hit (fastest-stopping frame, marked by a dash), bush and pouch where the gesture puts them, beam (the
builder's work: a block whose top is where the hammer's face is at the hit, the impact marked by a cross).
The output line gives the hit, how far the hands miss their targets, for a tool how far each hand on it is from
the handle carried by the tool bone (the tool in the hands if ~0), for each prop the body points inside it
(clearance: the hands holding props left out; a one-handed tool's free hand counts), the feet's drift (planted if
~0, gestures without a base) and, for the beam, the hammer's face at each hit and how deep any part of the hammer or
the left hand goes into the beam.
Run:
  python Tools/Editor/preview_animation.py [gesture] [--frames N | --picks f1,f2,...] [--zoom Z] [--out <png>]
      (--picks: the frame indices to show, e.g. one strike in detail)
  python Tools/Editor/preview_animation.py --compare g1,g2,g3 [--at 0.25] [--zoom Z] [--out <png>]
      (several gestures side by side at the same point of their loops; --at is the share of the loop, 0-1)
  python Tools/Editor/preview_animation.py --all
      (every gesture: <name>_contact_sheet.png (12 frames) and <name>_zoom.png (6 frames, zoom 2), plus the
      comparisons A_Peasant_Carry_comparaison.png and A_Peasant_IdleCarry_comparaison.png)
  add --light to any of them to draw the light props (peasant_props.build_props(light=True), the mesh's last LOD).
  add --mesh M (or M1,M2,... with --compare, one per gesture) to show a soldier mesh's pieces
      (soldier_props.SOLDIER_MESHES) instead of the gesture's "props", e.g. the Hero with the Man-at-arms' gestures;
      with --compare, --scales s1,s2,... draws each column at that MeshScale relative to 1.5 (the game's sizes).
(default output: Saved/AnimPreview/<gesture>_contact_sheet.png, outside git).
The skeleton, the source walk or idle and the body come from Tools/Editor/Data/, written by
make_peasant_animations.py.
"""
import json
import math
import os
import struct
import sys
import zlib

sys.dont_write_bytecode = True  # no __pycache__ in Tools/Editor
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import anim_rig  # noqa: E402
import peasant_poses  # noqa: E402
import soldier_poses  # noqa: E402
import soldier_props  # noqa: E402

# Every gesture the preview knows: the Peasant's and the soldiers' (J7a.3).
ALL_GESTURES = dict(peasant_poses.GESTURES, **soldier_poses.GESTURES)
ALL_GESTURES.update(getattr(soldier_poses, "PROPOSED", {}))  # proposals under review, not baked yet
import peasant_props  # noqa: E402

DATA_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Data")
SKELETON_JSON = os.path.join(DATA_DIR, "UE4MannequinSkeleton.json")
BODY_JSON = os.path.join(DATA_DIR, "SKM_Mannequin_Body.json")
OUT_DIR = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "Saved", "AnimPreview"))

CELL_W, CELL_H, LABEL_H = 200, 230, 14
PX_PER_CM = 0.8
BACKGROUND = (246, 244, 238)
COLORS = {"c": (60, 60, 60), "r": (205, 45, 45), "l": (45, 85, 205), "body": (214, 209, 198),
          "trace": (175, 175, 175), "ground": (150, 150, 140), "trunk": (70, 140, 60), "text": (40, 40, 40)}
SEGMENTS = [
    ("c", ["pelvis", "spine_01", "spine_02", "spine_03", "neck_01", "head"]),
    ("r", ["spine_03", "clavicle_r", "upperarm_r", "lowerarm_r", "hand_r", "middle_01_r"]),
    ("l", ["spine_03", "clavicle_l", "upperarm_l", "lowerarm_l", "hand_l", "middle_01_l"]),
    ("r", ["pelvis", "thigh_r", "calf_r", "foot_r", "ball_r"]),
    ("l", ["pelvis", "thigh_l", "calf_l", "foot_l", "ball_l"]),
]
# Bones whose vertices may touch a prop: the hands hold it.
HOLDING = ("hand_", "index_", "middle_", "ring_", "pinky_", "thumb_")
BODY_STEP = 1  # every vertex drawn
SURFACE_BIAS = 1.5  # cm: a prop lying on the skin (a strap) is not hidden by the vertices around it

# 3x5 digit font (and '.', 's') for the time labels.
FONT = {"0": "111101101101111", "1": "010110010010111", "2": "111001111100111", "3": "111001111001111",
        "4": "101101111001001", "5": "111100111001111", "6": "111100111101111", "7": "111001001001001",
        "8": "111101111101111", "9": "111101111001111", ".": "000000000000010", "s": "000011110011110",
        "-": "000000111000000"}


class Canvas:
    """RGB image with a depth buffer ("nearness": larger = closer to the camera) and a clip rectangle."""

    def __init__(self, width, height):
        self.w, self.h = width, height
        self.pixels = bytearray(BACKGROUND * (width * height))
        self.near = [-1e9] * (width * height)
        self.clip = (0, 0, width, height)

    def _put(self, px, py, color, near=None, write=False, tolerance=0.0):
        x0, y0, x1, y1 = self.clip
        if x0 <= px < x1 and y0 <= py < y1:
            i = py * self.w + px
            if near is not None and near + tolerance < self.near[i]:
                return
            if write:
                self.near[i] = near
            self.pixels[i * 3:i * 3 + 3] = bytes(color)

    def dot(self, x, y, color, radius=1.0, near=None, tolerance=0.0):
        r = int(math.ceil(radius))
        for dy in range(-r, r + 1):
            for dx in range(-r, r + 1):
                if dx * dx + dy * dy <= radius * radius + 0.25:
                    self._put(int(round(x)) + dx, int(round(y)) + dy, color, near, False, tolerance)

    def line(self, a, b, color, radius=1.5, near=None, tolerance=0.0):
        """Line between screen points; `near` = (nearness at a, at b) to hide it behind the props."""
        steps = int(max(abs(b[0] - a[0]), abs(b[1] - a[1]))) + 1
        for k in range(steps + 1):
            s = k / steps
            n = None if near is None else near[0] + (near[1] - near[0]) * s
            self.dot(a[0] + (b[0] - a[0]) * s, a[1] + (b[1] - a[1]) * s, color, radius, n, tolerance)

    def splat(self, x, y, color, half, near):
        """Small square of the body, drawn where it is in front of the props (it does not hide the stick figure)."""
        x0, y0, x1, y1 = self.clip
        rgb = bytes(color)
        for py in range(max(y0, int(y - half)), min(y1, int(y + half) + 1)):
            row = py * self.w
            for px in range(max(x0, int(x - half)), min(x1, int(x + half) + 1)):
                i = row + px
                if near >= self.near[i]:
                    self.pixels[i * 3:i * 3 + 3] = rgb

    def triangle(self, a, b, c, color):
        """Filled triangle of screen points (x, y, nearness), depth-tested and written."""
        x0, y0, x1, y1 = self.clip
        minx, maxx = max(x0, int(math.floor(min(a[0], b[0], c[0])))), min(x1 - 1, int(math.ceil(max(a[0], b[0], c[0]))))
        miny, maxy = max(y0, int(math.floor(min(a[1], b[1], c[1])))), min(y1 - 1, int(math.ceil(max(a[1], b[1], c[1]))))
        area = (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0])
        if abs(area) < 1e-9 or minx > maxx or miny > maxy:
            return
        rgb = bytes(color)
        for py in range(miny, maxy + 1):
            y = py + 0.5
            for px in range(minx, maxx + 1):
                x = px + 0.5
                w0 = ((b[0] - x) * (c[1] - y) - (b[1] - y) * (c[0] - x)) / area
                w1 = ((c[0] - x) * (a[1] - y) - (c[1] - y) * (a[0] - x)) / area
                w2 = 1.0 - w0 - w1
                if w0 < -1e-6 or w1 < -1e-6 or w2 < -1e-6:
                    continue
                n = w0 * a[2] + w1 * b[2] + w2 * c[2]
                i = py * self.w + px
                if n >= self.near[i]:
                    self.near[i] = n
                    self.pixels[i * 3:i * 3 + 3] = rgb

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
    """Projections (component point -> screen x, y up, nearness) for the three rows, each row's horizontal centre and
    the direction toward its camera (for the shading)."""
    def body(p):
        return (anim_rig.dot(p, sk.right), anim_rig.dot(p, sk.forward), anim_rig.dot(p, sk.up))

    def front(p):
        r, f, u = body(p)
        return (-r, u, f)

    def side(p):
        r, f, u = body(p)
        return (f, u, r)

    yaw, pitch = math.radians(35.0), math.radians(55.0)

    def rts(p):
        r, f, u = body(p)
        # Camera in front-right of him, looking down: screen x across the view, y mixes height and depth.
        x = -r * math.cos(yaw) + f * math.sin(yaw)
        depth = -r * math.sin(yaw) - f * math.cos(yaw)
        return (x, u * math.cos(pitch) + depth * math.sin(pitch) * 0.6 + 40.0,
                -depth * math.cos(pitch) + u * math.sin(pitch))

    to_rts = anim_rig.normalize(sk.body((math.sin(yaw) * math.cos(pitch), math.cos(yaw) * math.cos(pitch),
                                         math.sin(pitch))))
    return [(front, 0.0, sk.forward), (side, 20.0, sk.right), (rts, 0.0, to_rts)]


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


# ---------------------------------------------------------------------------------------------------------------------
# Shared data: skeleton, props, body cloud


class Shared:
    def __init__(self, light=False):
        self.sk = anim_rig.Skeleton.load(SKELETON_JSON)
        self.props = peasant_props.build_props(self.sk, light)
        self.props.update(soldier_props.build_props(self.sk, light))
        self.prop_tris = {name: peasant_props.prop_triangles(p) for name, p in self.props.items()}
        self.body = []
        self.body_side = []  # "l" / "r" for the points of the hands, else None
        if os.path.exists(BODY_JSON):
            with open(BODY_JSON, "r", encoding="utf-8") as f:
                points = json.load(f)["points"]
            for x, y, z, bone in points:
                i = self.sk.index[bone]
                t, q = self.sk.ref_world[i]
                self.body.append((i, anim_rig.qrot(anim_rig.qconj(q), anim_rig.sub((x, y, z), t)),
                                  bone.startswith(HOLDING)))
                self.body_side.append(bone[-1] if bone.startswith(HOLDING) else None)

    def posed_body(self, world, step=1):
        out = []
        for k in range(0, len(self.body), step):
            i, local, holding = self.body[k]
            t, q = world[i]
            out.append((anim_rig.add(t, anim_rig.qrot(q, local)), holding))
        return out


def inside_depth(prim, p):
    """How deep point p (prop frame) is inside a primitive (cm), or None if outside."""
    q = anim_rig.qrot(anim_rig.qconj(prim["q"]), anim_rig.sub(p, prim["t"]))
    shape = prim["shape"]
    if shape == "box":
        margins = [s * 0.5 - abs(c) for s, c in zip(prim["size"], q)]
        return min(margins) if min(margins) > 0.0 else None
    if shape == "frustum":
        h = prim["height"]
        if not 0.0 < q[2] < h:
            return None
        s = q[2] / h
        rx = prim["bottom"][0] + (prim["top"][0] - prim["bottom"][0]) * s
        ry = prim["bottom"][1] + (prim["top"][1] - prim["bottom"][1]) * s
        if rx < 1e-6 or ry < 1e-6:
            return None
        e = math.sqrt((q[0] / rx) ** 2 + (q[1] / ry) ** 2)
        return min(q[2], h - q[2], (1.0 - e) * min(rx, ry)) if e < 1.0 else None
    if shape == "prism":
        if not 0.0 < q[2] < prim["height"]:
            return None
        pts = prim["points"]
        best = min(q[2], prim["height"] - q[2])
        for a, b in zip(pts, pts[1:] + pts[:1]):
            ex, ey = b[0] - a[0], b[1] - a[1]
            d = (ex * (q[1] - a[1]) - ey * (q[0] - a[0])) / math.hypot(ex, ey)
            if d <= 0.0:
                return None
            best = min(best, d)
        return best
    return None


def clearance(shared, prop, world, points, free=()):
    """Body points (not the hands holding it; the hands of the sides in `free` count) inside the prop in a pose:
    (count, deepest cm). `points` from Shared.posed_body with step 1 (aligned with Shared.body_side)."""
    t, q = peasant_props.prop_frame(prop, shared.sk, world)
    lo = [min(c) - 1.0 for c in zip(*[p for tri, _ in shared.prop_tris[prop["name"]] for p in tri])]
    hi = [max(c) + 1.0 for c in zip(*[p for tri, _ in shared.prop_tris[prop["name"]] for p in tri])]
    count, deepest = 0, 0.0
    for k, (p, holding) in enumerate(points):
        if holding and shared.body_side[k] not in free:
            continue
        local = anim_rig.qrot(anim_rig.qconj(q), anim_rig.sub(p, t))
        if any(c < a or c > b for c, a, b in zip(local, lo, hi)):
            continue
        depth = max((inside_depth(prim, local) or 0.0) for prim in prop["prims"])
        if depth > 0.0:
            count += 1
            deepest = max(deepest, depth)
    return count, deepest


def prepare(shared, gesture_name, mesh=None):
    """Everything a cell of this gesture needs: solved frames, hit, scenery, props; and the checks. `mesh`: show that
    soldier mesh's pieces instead of the gesture's props."""
    sk = shared.sk
    gesture = ALL_GESTURES[gesture_name]
    rig = anim_rig.Rig(sk, gesture, anim_rig.load_base(gesture, DATA_DIR))
    poses = [rig.pose(t) for t in rig.times]
    worlds = [sk.fk(local) for local, _ in poses]
    ctx = {"name": gesture_name, "gesture": gesture, "rig": rig, "poses": poses, "worlds": worlds,
           "traces": [extra["trace"] for _, extra in poses],
           "reach": max(max(rig.grip_errors(world, extra).values()) for world, (_, extra) in zip(worlds, poses)),
           "has_tool": rig.has_tool, "scene": gesture.get("scene", {"kind": "trunk" if rig.has_tool else "none"}),
           "hit": None, "summary": "", "scenery": [],
           "props": [shared.props[name] for name in (soldier_props.SOLDIER_MESHES[mesh] if mesh
                                                     else gesture.get("props", ()))], "checks": []}
    traces, scene = ctx["traces"], ctx["scene"]
    if ctx["has_tool"]:
        hit = rig.strike()[0]
        ctx["hit"] = hit
        r, f, u = body_terms(sk, traces[hit])
        ctx["summary"] = ", frappe a %.2f s, tete de l'outil a la frappe : droite %.0f, avant %.0f, haut %.0f cm" % (
            rig.times[hit], r, f, u)
        handle = [rig.handle_distances(world) for world, (_, extra) in zip(worlds, poses) if not extra.get("released")]
        ctx["checks"].append("mains a l'axe du manche porte par %s au plus : %s" % (anim_rig.TOOL_BONE, ", ".join(
            "%s %.3f cm" % ({"l": "gauche", "r": "droite"}[side], max([h[side] for h in handle] or [0.0]))
            for side in rig.tool_hands)))
    hit = ctx["hit"]
    if scene["kind"] == "trunk" and hit is not None:
        r, f, u = body_terms(sk, traces[hit])
        ctx["scenery"].append(((sk.body((r, f, 0.0)), sk.body((r, f, 200.0))), 2.5))
    elif scene["kind"] == "rock" and hit is not None:
        # The rock's top is where the pickaxe's point is at the hit.
        t, q = worlds[hit][sk.index[anim_rig.TOOL_BONE]]
        tip = anim_rig.add(t, anim_rig.qrot(q, (peasant_props.HANDLE_LENGTH - 6.0, 24.0, 0.0)))
        r, f, u = body_terms(sk, tip)
        height = max(10.0, u)
        ctx["scenery"] += [(line, 1.6) for line in box_lines(sk, (r, f + 10.0, height / 2), (24.0, 18.0, height / 2))]
        ctx["summary"] += ", pointe a la frappe : avant %.0f, haut %.0f cm" % (f, u)
    elif scene["kind"] == "beam" and hit is not None:
        beam_scene(shared, ctx)
    elif scene["kind"] == "bush":
        ctx["scenery"] += [(line, 1.2) for line in ellipsoid_lines(sk, scene["centre"], scene["radii"])]
        if "pouch" in scene:
            ctx["scenery"] += [(line, 1.2) for line in box_lines(sk, scene["pouch"], (6.0, 8.0, 8.0))]
    if rig.base is None:
        # Planted feet: how far each foot drifts from its place at the first frame.
        drift = max(anim_rig.length(anim_rig.sub(sk.pos(foot, world), sk.pos(foot, worlds[0])))
                    for world in worlds for foot in ("foot_l", "foot_r", "ball_l", "ball_r"))
        loop = max(anim_rig.length(anim_rig.sub(a[0], b[0])) for a, b in zip(worlds[0], worlds[-1]))
        ctx["checks"].append("pieds : derive max %.3f cm ; premiere / derniere image : ecart max des os %.3f cm"
                             % (drift, loop))
    # Clearance of each prop against the body (hands left out), over the loop (every frame up to 60, else spread).
    free = tuple(side for side in ("l", "r") if side not in rig.tool_hands) if rig.has_tool else ()
    if shared.body and ctx["props"]:
        step = max(1, int(math.ceil(len(worlds) / 60.0)))
        for prop in ctx["props"]:
            worst = (0, 0.0, 0.0)
            for k in range(0, len(worlds), step):
                released = ("l", "r") if poses[k][1].get("released") else ()  # a released tool: the hands count
                count, deep = clearance(shared, prop, worlds[k], shared.posed_body(worlds[k]),
                                        (free + released) if prop["frame"] == "tool" else ())
                if (count, deep) > worst[:2]:
                    worst = (count, deep, rig.times[k])
            ctx["checks"].append("%s : points du corps%s dedans au plus %d (%.2f s), %.1f cm de profondeur au plus"
                                 % (prop["name"], " (main libre comprise)" if free and prop["frame"] == "tool" else "",
                                    worst[0], worst[2], worst[1]))
    return ctx


def hammer_face(sk, world):
    """Centre of the hammer's face and its outward direction (component space) in a solved pose."""
    t, q = world[sk.index[anim_rig.TOOL_BONE]]
    centre = anim_rig.add(t, anim_rig.qrot(q, (peasant_props.HAMMER_HEAD_AT, peasant_props.HAMMER_FACE, 0.0)))
    return centre, anim_rig.qrot(q, (0.0, 1.0, 0.0))


def beam_scene(shared, ctx):
    """The builder's beam: a block from the ground to the hammer's face at the hit, "near" cm before the impact to
    "depth" cm behind its near edge, "width" cm wide (centred on him); checks every hit and the hammer and left hand
    against the block."""
    sk, scene, worlds, rig = shared.sk, ctx["scene"], ctx["worlds"], ctx["rig"]
    centre, _ = hammer_face(sk, worlds[ctx["hit"]])
    r, f, top = body_terms(sk, centre)
    near, far, half = f - scene["near"], f - scene["near"] + scene["depth"], scene["width"] / 2.0
    ctx["beam"] = (near, far, half, top)
    ctx["scenery"] += [(line, 1.6) for line in box_lines(sk, (0.0, (near + far) / 2, top / 2),
                                                          (half, (far - near) / 2, top / 2))]
    # The impact: a cross on the top.
    for d in ((6.0, 0.0), (0.0, 6.0)):
        ctx["scenery"].append(((sk.body((r - d[0], f - d[1], top)), sk.body((r + d[0], f + d[1], top))), 1.2))
    # Every hit: the frames where the face is lowest (local minima of its height).
    heights = [body_terms(sk, hammer_face(sk, w)[0])[2] for w in worlds]
    hits = [k for k in range(1, len(worlds) - 1) if heights[k] <= heights[k - 1] and heights[k] < heights[k + 1]]
    parts = []
    for k in hits:
        c, out = hammer_face(sk, worlds[k])
        cr, cf, cu = body_terms(sk, c)
        tilt = math.degrees(math.acos(max(-1.0, min(1.0, -anim_rig.dot(out, sk.up)))))
        parts.append("%.2f s : droite %.1f, avant %.1f, haut %.1f cm, face a %.0f deg de la verticale"
                     % (rig.times[k], cr, cf, cu, tilt))
    ctx["checks"].append("marteau sur la poutre (dessus a %.1f cm, de %.0f a %.0f cm devant) : %s"
                         % (top, near, far, " ; ".join(parts)))

    def into_beam(points):
        deepest = 0.0
        for p in points:
            pr, pf, pu = body_terms(sk, p)
            if -half < pr < half and near < pf < far and pu < top:
                deepest = max(deepest, min(top - pu, pf - near, far - pf, pr + half, half - pr))
        return deepest

    hammer = shared.props["hammer"]
    tris = shared.prop_tris["hammer"]
    left = [k for k, side in enumerate(shared.body_side) if side == "l"]
    worst_hammer = worst_hand = 0.0
    gap = 1e9
    for world in worlds:
        points = [p for tri, _ in peasant_props.posed_triangles(hammer, sk, world, tris) for p in tri]
        worst_hammer = max(worst_hammer, into_beam(points))
        body = shared.posed_body(world)
        hand = [body[k][0] for k in left]
        worst_hand = max(worst_hand, into_beam(hand))
        gap = min(gap, min(body_terms(sk, p)[2] for p in hand) - top)
    ctx["checks"].append("dans la poutre au plus : marteau %.2f cm, main gauche %.2f cm ; dessous de la main gauche "
                         "%.2f cm au-dessus du dessus au plus bas" % (worst_hammer, worst_hand, gap))


def draw_cell(canvas, shared, ctx, frame, ox, oy, view, label):
    sk = shared.sk
    project, centre, to_camera = view
    canvas.clip = (ox, oy, ox + CELL_W, oy + CELL_H)

    def screen(p):
        x, y, n = project(p)
        return (ox + CELL_W / 2 + (x - centre) * PX_PER_CM, oy + CELL_H - 12 - y * PX_PER_CM, n)

    def flat(p):
        return screen(p)[:2]

    # Ground, scenery (trunk, rock, bush and pouch) and the path of the tool's head or right hand: behind everything.
    canvas.line(flat(anim_rig.scale(sk.right, 100.0)), flat(anim_rig.scale(sk.right, -100.0)), COLORS["ground"], 0.5)
    canvas.line(flat(anim_rig.scale(sk.forward, 120.0)), flat(anim_rig.scale(sk.forward, -80.0)), COLORS["ground"], 0.5)
    for (a, b), width in ctx["scenery"]:
        canvas.line(flat(a), flat(b), COLORS["trunk"], width)
    for point in ctx["traces"]:
        canvas.dot(*flat(point), COLORS["trace"], 1.0)

    world = ctx["worlds"][frame]
    # Props: filled, flat-shaded toward the camera, depth-buffered.
    hidden = ctx["gesture"].get("hidden", {})  # {prop: (start s, end s)}: not drawn then (the Archer's arrow)
    for prop in ctx["props"]:
        span = hidden.get(prop["name"])
        if span and span[0] <= ctx["rig"].times[frame] <= span[1]:
            continue
        for tri, color in peasant_props.posed_triangles(prop, sk, world, shared.prop_tris[prop["name"]]):
            normal = anim_rig.normalize(anim_rig.cross(anim_rig.sub(tri[1], tri[0]), anim_rig.sub(tri[2], tri[0])))
            light = 0.55 + 0.45 * abs(anim_rig.dot(normal, to_camera))
            canvas.triangle(*(screen(p) for p in tri), tuple(min(255, int(c * light)) for c in color))
    # Body (grey splats, one per vertex): hides the props behind it, hidden by those in front.
    half = max(0.5, 0.9 * PX_PER_CM)
    for p, _ in shared.posed_body(world, BODY_STEP):
        x, y, n = screen(p)
        canvas.splat(x, y, COLORS["body"], half, n - SURFACE_BIAS)
    # Stick figure on top of the body, hidden behind the props (a little tolerance: the bones are inside the body).
    for color, chain in SEGMENTS:
        for a, b in zip(chain, chain[1:]):
            sa, sb = screen(sk.pos(a, world)), screen(sk.pos(b, world))
            canvas.line(sa[:2], sb[:2], COLORS[color], 1.4, (sa[2], sb[2]), 6.0)
    canvas.clip = (0, 0, canvas.w, canvas.h)
    canvas.text(ox + 6, oy + CELL_H + 1, label, COLORS["text"])
    if frame == ctx["hit"]:
        canvas.text(ox + CELL_W - 30, oy + CELL_H + 1, "-", COLORS["trunk"])


def set_zoom(zoom):
    global CELL_W, CELL_H, PX_PER_CM
    CELL_W, CELL_H, PX_PER_CM = int(200 * zoom), int(230 * zoom), 0.8 * zoom


def report(ctx):
    rig = ctx["rig"]
    lines = ["boucle de %.3f s a %.2f i/s%s ; mains hors de leur cible au plus de %.1f cm"
             % (rig.length, rig.fps, ctx["summary"], ctx["reach"])]
    return "\n    ".join(lines + ctx["checks"])


def render(shared, gesture_name, frames, out_path, zoom=1.0, ctx=None, picks=None, mesh=None):
    set_zoom(zoom)
    sk = shared.sk
    ctx = ctx or prepare(shared, gesture_name, mesh)
    rig = ctx["rig"]
    picks = picks or [int(round(i * (len(rig.times) - 1) / frames)) for i in range(frames)]
    frames = len(picks)
    canvas = Canvas(CELL_W * frames, (CELL_H + LABEL_H) * 3)
    for row, view in enumerate(views(sk)):
        oy = row * (CELL_H + LABEL_H)
        for col, frame in enumerate(picks):
            draw_cell(canvas, shared, ctx, frame, col * CELL_W, oy, view, "%.2fs" % rig.times[frame])
        for col in range(1, frames):
            canvas.line((col * CELL_W, oy), (col * CELL_W, oy + CELL_H + LABEL_H - 1), (210, 210, 205), 0.4)
    canvas.save(out_path)
    print("Planche : %s (%d images)" % (out_path, frames))
    return ctx


def compare(shared, gesture_names, fraction, out_path, zoom=1.0, contexts=None, meshes=None, scales=None):
    """Several gestures side by side at the same point of their loops (`fraction` of the loop, 0-1); `meshes`: one
    soldier mesh (or None) per gesture; `scales`: one MeshScale per gesture (drawn relative to 1.5)."""
    global PX_PER_CM
    set_zoom(zoom)
    sk = shared.sk
    meshes = meshes or [None] * len(gesture_names)
    contexts = contexts or [prepare(shared, name, mesh) for name, mesh in zip(gesture_names, meshes)]
    canvas = Canvas(CELL_W * len(contexts), (CELL_H + LABEL_H) * 3)
    base = PX_PER_CM
    for row, view in enumerate(views(sk)):
        oy = row * (CELL_H + LABEL_H)
        for col, ctx in enumerate(contexts):
            frame = int(round(fraction * (len(ctx["rig"].times) - 1)))
            PX_PER_CM = base * (scales[col] / 1.5 if scales else 1.0)
            draw_cell(canvas, shared, ctx, frame, col * CELL_W, oy, view, "%.2fs" % ctx["rig"].times[frame])
            PX_PER_CM = base
        for col in range(1, len(contexts)):
            canvas.line((col * CELL_W, oy), (col * CELL_W, oy + CELL_H + LABEL_H - 1), (210, 210, 205), 0.4)
    canvas.save(out_path)
    print("Comparaison : %s (%s, a %.0f %% de la boucle)" % (out_path, ", ".join(gesture_names), fraction * 100))


def render_all(shared):
    """Every gesture (full and zoomed sheets) and the two comparisons of the loaded walks and idles."""
    os.makedirs(OUT_DIR, exist_ok=True)
    contexts = {}
    for key, gesture in peasant_poses.GESTURES.items():
        ctx = prepare(shared, key)
        contexts[key] = ctx
        # A gesture may choose its frames ("preview": {"frames": N, "zoom_picks": [...]}) so the sheets show its hits.
        preview = gesture.get("preview", {})
        render(shared, key, preview.get("frames", 12), os.path.join(OUT_DIR, gesture["name"] + "_contact_sheet.png"),
               1.0, ctx)
        render(shared, key, 6, os.path.join(OUT_DIR, gesture["name"] + "_zoom.png"), 2.0, ctx,
               preview.get("zoom_picks"))
        print("  %s : %s" % (gesture["name"], report(ctx)))
    for names, label in ((("carry_food", "carry_wood", "carry_stone"), "A_Peasant_Carry_comparaison"),
                         (("idle_carry_food", "idle_carry_wood", "idle_carry_stone"),
                          "A_Peasant_IdleCarry_comparaison")):
        compare(shared, names, 0.25, os.path.join(OUT_DIR, label + ".png"), 1.6, [contexts[n] for n in names])


def main():
    args = sys.argv[1:]
    gesture = "chop_wood"
    frames = 12
    out = None
    zoom = 1.0
    together = None
    picks = None
    fraction = 0.25
    meshes = None
    scales = None
    i = 0
    light = "--light" in args
    args = [a for a in args if a != "--light"]
    shared = Shared(light)
    if "--all" in args:
        render_all(shared)
        return
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
        elif args[i] == "--picks":
            picks = [int(v) for v in args[i + 1].split(",")]
            i += 1
        elif args[i] == "--at":
            fraction = float(args[i + 1])
            i += 1
        elif args[i] == "--mesh":
            meshes = args[i + 1].split(",")
            i += 1
        elif args[i] == "--scales":
            scales = [float(v) for v in args[i + 1].split(",")]
            i += 1
        else:
            gesture = args[i]
        i += 1
    if out is None:
        os.makedirs(OUT_DIR, exist_ok=True)
        name = "_".join(together) + "_compare" if together else gesture + "_contact_sheet"
        out = os.path.join(OUT_DIR, name + ".png")
    if together:
        compare(shared, together, fraction, out, zoom, meshes=meshes, scales=scales)
    else:
        ctx = render(shared, gesture, frames, out, zoom, picks=picks, mesh=meshes[0] if meshes else None)
        print("  " + report(ctx))


if __name__ == "__main__":
    main()
