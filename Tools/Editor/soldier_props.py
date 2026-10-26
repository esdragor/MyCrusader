"""Weapons of the soldiers (J7a.3): the Spearman's spear, the Man-at-arms' sword and shield.

Pure Python, same primitives and conventions as peasant_props.py (read its docstring): every piece is skinned 100 % to
ONE bone; light versions (light=True) for the mesh's last LOD, the in-game look (D326). One mesh per type (user's
choice, 2026-10-18): SKM_Spearman = mannequin + spear, SKM_ManAtArms = mannequin + sword + shield; the weapons are
ALWAYS shown (no per-instance choice), so their ids only name them (SOLDIER_MESHES lists each mesh's pieces).

  - spear (id 1): held upright in the right hand; tool frame on ik_hand_gun (origin at the butt, X up the shaft toward
    the point, Y the blade's side); the gestures key the bone (anim_rig "tool", one hand, fixed "edge").
  - sword (id 2): in the right hand, same tool frame (origin at the pommel, X along the blade, Y an edge).
  - shield (id 3): heater shield on the left forearm, skinned to hand_l ("bone" frame of peasant_props): its frame
    follows the left hand, which the gestures place with its palm and fingers (free hand).
Archer (J7c, built in J7c.1a; SKM_Archer = mannequin + bow + quiver + arrow):
  - bow (id 4): 160 cm longbow in the LEFT fist, skinned to hand_l like the shield; frame: origin at the grip, X up
    the bow (the hand's pinky-to-index axis: the upper limb), Y toward the string (the archer's side), Z = X x Y.
    The string is straight (braced): one rigid bone cannot bend it to the drawing hand.
  - arrow (id 5): 78 cm, the right hand's tool on ik_hand_gun (origin at the nock, X toward the point); the attack
    keys point it at the bow's grip. Shown only from the nock to the release (see soldier_poses ARCHER_ATTACK
    "hidden"): the visible arrow is the client's projectile after that.
  - quiver (id 6): on the back (spine_03, "body" frame), opening above the right shoulder, five fletchings out.
Crossbowman (AR1, 2026-10-26; SKM_Crossbowman = mannequin + crossbow + bolt + bolt_quiver). Ids only have to differ
inside one mesh and stay in 1-8 (peasant_props.prop_bit: the vertex mask is one byte): the crossbow takes the free 8,
the loaded bolt and the bolt quiver reuse the Archer's 5 and 6 (never in the same mesh).
  - crossbow (id 8): stirrup crossbow, the hands' TOOL on ik_hand_gun ("tool" frame, keyed by the gestures, child of
    the ik root: during the stirrup reload it stays point down under the foot while both hands are free on the string).
    Frame: origin at the stirrup's front end (the "knob": on the ground at the reload), X along the tiller toward the
    butt, Y the top of the stock (the bolt's groove, the "edge": up when it lies flat), Z = X x Y (the prod's axis,
    across). Two-handed tool of anim_rig: the left hand at CROSSBOW_LEFT_GRIP behind the prod ("knob_offset"), the right
    hand CROSSBOW_HAND_SPACING further back, at the wrist of the stock (the trigger). Stirrup x 0-13, steel prod
    across at x ~13 (drawn: its tips 7 cm toward the butt), tiller x 10-95 (85 cm), the string straight from each tip
    to the nut (spanned: the crossbow is loaded at rest, in the walk and at the aim; one rigid bone cannot move it),
    nut at CROSSBOW_NUT, trigger lever under the stock.
  - bolt (id 5): the LOADED bolt in the groove (nut to 35 cm forward), skinned to ik_hand_r ("bone" frame): a child
    of ik_hand_gun never keyed by the baker (only ik_hand_gun is), so it follows the crossbow exactly; its frame is
    the crossbow's (local = ik_hand_r's reference inverse x ik_hand_gun's reference). The gestures can hide it after
    the shot (preview "hidden"; the baker will have to scale ik_hand_r to 0 there, AR3): scaling ik_hand_gun
    ("tool_hidden") would hide the crossbow too. The bolt IN THE HAND (from the hip to the groove) has no cheap bone
    (see CrossbowInProgress.md, AR1): left out; the bolt in flight is the client's SM_Bolt (AR-Q5).
  - bolt_quiver (id 6): on the right hip, skinned to the pelvis ("body" frame: the thigh would swing it in the walk),
    opening up, four fletched bolt ends out, a band in the player's colour.

Hero, the Paladin-Commander (J8a.0c, 2026-10-26, provisional; dressed as a paladin, user's choice): SKM_Hero =
mannequin + longsword + sun_shield + tabard + mantle + pauldrons + helm; it plays the Man-at-arms' gestures as they are
(maa_*), MeshScale 1.8 in game instead of 1.5; arms and legs in the player's colour. Ids reuse others' (never in the
same mesh). The J8a.0c pieces are "body" pieces anchored at the body origin (their points are body terms).
  - longsword (id 2): the sword's tool frame and grip (SWORD_GRIP), 112 cm, gold crossguard and pommel.
  - sun_shield (id 3): the shield's frame and outline, a gold sun (disc and eight rays) on the player's colour.
  - tabard (id 1): white panels front and back, belt to the collar, gold rims, a gold sun on the chest and the back;
    on spine_02 (the belt and the chest both follow it best), ~0.7 cm off the skin; never below the belt.
  - mantle (id 4): short white panel on the upper back (mid-back to the trapezius), gold rims; on spine_03.
  - pauldron_l / pauldron_r (ids 5, 6): big rounded gold domes over the shoulder joints, out and up; on the
    clavicles (an upper-arm pauldron would swing into the neck when the arm rises to strike).
  - helm (id 7): closed great helm (steel, eye slit, gold cross and band) with a gold sun crest; on head.

PLAYER COLOUR: primitives with "player": True are drawn in PLAYER_PREVIEW here; the mesh builder must write them like
the body (vertex alpha 0: the material then shows the player's colour) - the shield's face, the spear's pennant.
Run alone to print the table:  python Tools/Editor/soldier_props.py
"""
import math
import os
import sys

sys.dont_write_bytecode = True  # no __pycache__ in Tools/Editor
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import anim_rig  # noqa: E402
import peasant_props as pp  # noqa: E402

PLAYER_PREVIEW = (52, 96, 200)  # stands for the player's colour on the previews
WOOD = pp.WOOD_HANDLE
LEATHER = pp.LEATHER
IRON, STEEL = pp.IRON, pp.STEEL
BRASS = (176, 140, 60)

# Spear: 240 cm from the butt to the point (taller than the man: reads at the RTS camera), held at SPEAR_GRIP.
SPEAR_LENGTH = 240.0
SPEAR_GRIP = 95.0          # butt to the right hand's grip (the gestures' "knob_offset")
SPEAR_RADIUS = 1.7
SPEAR_HEAD = 30.0          # length of the blade (leaf), ending at SPEAR_LENGTH
SPEAR_HEAD_WIDTH = 7.0

# Sword: 92 cm, pommel to point; grip 14 cm.
SWORD_LENGTH = 92.0
SWORD_GRIP = 8.0           # pommel to the right hand's grip
SWORD_GUARD = 16.0         # pommel to the crossguard
SWORD_WIDTH = 4.6

# Heater shield, on the left forearm (hand_l): 60 cm tall, 46 wide.
SHIELD_TOP, SHIELD_BOTTOM, SHIELD_HALF_WIDTH = 26.0, -34.0, 23.0
SHIELD_OUT = 5.5           # grip point (in the fist) to the board's back, toward the back of the hand
SHIELD_TOWARD_ELBOW = 9.0  # grip point to the board's centre, toward the elbow


def player(prim):
    prim["player"] = True
    return prim


def make_spear(light=False):
    sides = 4 if light else 6
    x0 = SPEAR_LENGTH - SPEAR_HEAD
    prims = [
        pp.frustum_between((0.0, 0.0, 0.0), (x0 + 2.0, 0.0, 0.0), (SPEAR_RADIUS, SPEAR_RADIUS),
                           (SPEAR_RADIUS * 0.9, SPEAR_RADIUS * 0.9), sides, WOOD, x_hint=pp.FLAT),
        # Iron socket under the head, then the leaf blade, flat across Z, widest at a third.
        pp.frustum_between((x0 - 6.0, 0.0, 0.0), (x0 + 1.0, 0.0, 0.0), (2.1, 2.1), (1.6, 1.6), sides, IRON,
                           x_hint=pp.FLAT),
        pp.prism([(x0, -1.2), (x0 + 10.0, -SPEAR_HEAD_WIDTH / 2), (SPEAR_LENGTH, 0.0),
                  (x0 + 10.0, SPEAR_HEAD_WIDTH / 2), (x0, 1.2)], 1.2, (0.0, 0.0, -0.6), pp.IDENTITY, STEEL),
        # Pennant in the player's colour, below the head, out on the blade's side.
        player(pp.prism([(x0 - 30.0, 0.0), (x0 - 23.0, 24.0), (x0 - 10.0, 0.0)], 0.6, (0.0, 0.0, -0.3), pp.IDENTITY,
                        PLAYER_PREVIEW)),
    ]
    if not light:
        # Iron butt cap.
        prims.append(pp.frustum_between((0.0, 0.0, 0.0), (5.0, 0.0, 0.0), (1.6, 1.6), (2.0, 2.0), 6, IRON,
                                        x_hint=pp.FLAT))
    return {"id": 1, "name": "spear", "label": "lance", "bone": "ik_hand_gun", "frame": "tool", "prims": prims}


def make_sword(light=False):
    sides = 4 if light else 6
    tip = SWORD_LENGTH
    prims = [
        # Leather grip, crossguard across the edges, blade tapering to the point.
        pp.frustum_between((1.5, 0.0, 0.0), (SWORD_GUARD, 0.0, 0.0), (1.5, 1.3), (1.3, 1.2), sides, LEATHER,
                           x_hint=pp.FLAT),
        pp.box((SWORD_GUARD + 0.8, 0.0, 0.0), (1.6, 18.0, 2.0), BRASS),
        pp.prism([(SWORD_GUARD + 1.6, -SWORD_WIDTH / 2), (tip - 12.0, -SWORD_WIDTH / 2 + 0.6), (tip, 0.0),
                  (tip - 12.0, SWORD_WIDTH / 2 - 0.6), (SWORD_GUARD + 1.6, SWORD_WIDTH / 2)], 0.9, (0.0, 0.0, -0.45),
                 pp.IDENTITY, STEEL),
    ]
    if not light:
        prims.append(pp.box((0.8, 0.0, 0.0), (3.2, 3.6, 3.0), BRASS))  # pommel
    return {"id": 2, "name": "sword", "label": "epee", "bone": "ik_hand_gun", "frame": "tool", "prims": prims}


def hand_axes(sk, side="l"):
    """The hand's reference axes as anim_rig.Rig reads them: fingers (wrist to middle knuckle), across (pinky to
    index), palm (the side the fingers curl to), and the grip point (in the closed fingers), all component space."""
    hand = "hand_" + side
    knuckle = sk.pos("middle_01_" + side)
    fingers = anim_rig.sub(knuckle, sk.pos(hand))
    across = anim_rig.sub(sk.pos("index_01_" + side), sk.pos("pinky_01_" + side))
    m1, m2, m3 = (sk.pos("middle_%02d_%s" % (k, side)) for k in (1, 2, 3))
    along = anim_rig.normalize(anim_rig.sub(m2, m1))
    curl = anim_rig.sub(anim_rig.sub(m3, m2), anim_rig.scale(along, anim_rig.dot(along, anim_rig.sub(m3, m2))))
    palm = anim_rig.normalize(anim_rig.cross(fingers, across))
    if anim_rig.dot(palm, curl) < 0.0:
        palm = anim_rig.scale(palm, -1.0)
    grip = anim_rig.add(anim_rig.lerp(sk.pos(hand), knuckle, 0.85), anim_rig.scale(palm, 2.5))
    return anim_rig.normalize(fingers), anim_rig.normalize(across), palm, grip


def shield_local(sk):
    """The shield's frame in hand_l's reference frame: X out of the back of the hand (the shield's face), Z toward the
    index (up when the thumb is up), Y = Z x X; origin at the board's back, in front of the forearm."""
    fingers, across, palm, grip = hand_axes(sk, "l")
    x = anim_rig.scale(palm, -1.0)
    z = anim_rig.normalize(anim_rig.sub(across, anim_rig.scale(x, anim_rig.dot(across, x))))
    y = anim_rig.cross(z, x)
    origin = anim_rig.add(anim_rig.add(grip, anim_rig.scale(x, SHIELD_OUT)), anim_rig.scale(fingers, -SHIELD_TOWARD_ELBOW))
    bt, bq = sk.ref_world[sk.index["hand_l"]]
    rotation = anim_rig.qfrom_matrix((x, y, z))
    return (anim_rig.qrot(anim_rig.qconj(bq), anim_rig.sub(origin, bt)),
            anim_rig.qnormalize(anim_rig.qmul(anim_rig.qconj(bq), rotation)))


def heater_outline(scale_by=1.0, steps=4):
    """Convex heater outline in (up, side) cm, counter-clockwise: flat top, straight sides, curved to the point."""
    top, bottom, half = SHIELD_TOP * scale_by, SHIELD_BOTTOM * scale_by, SHIELD_HALF_WIDTH * scale_by
    right = [(top, half), (top * 0.1, half)]
    # Arc from the side's lower end to the point.
    for k in range(1, steps):
        a = k / steps * math.pi / 2
        right.append((top * 0.1 + (bottom - top * 0.1) * math.sin(a), half * math.cos(a)))
    pts = right + [(bottom, 0.0)] + [(u, -s) for u, s in reversed(right)]
    # Counter-clockwise in the prism's (x, y): x = up, y = -side (see make_shield).
    return [(u, -s) for u, s in pts][::-1]


def make_shield(sk, light=False):
    # The prism extrudes along its local Z: turned so it extrudes along the frame's X (out), its local X up (frame Z).
    q = pp.rotation_z_to((1.0, 0.0, 0.0), (0.0, 0.0, 1.0))
    steps = 2 if light else 4
    prims = [
        # Iron rim: the board's outline, a little larger, behind the face...
        pp.prism(heater_outline(1.0, steps), 1.4, (0.0, 0.0, 0.0), q, IRON),
        # ...and the face in the player's colour, in front of it.
        player(pp.prism(heater_outline(0.94, steps), 0.8, (1.4, 0.0, 0.0), q, PLAYER_PREVIEW)),
    ]
    if not light:
        prims.append(pp.box((2.6, 0.0, 2.0), (1.6, 7.0, 7.0), IRON))  # boss
    return {"id": 3, "name": "shield", "label": "bouclier", "bone": "hand_l", "frame": "bone",
            "local": shield_local(sk), "prims": prims}


# Archer (J7c): longbow, arrow, quiver.
BOW_LENGTH = 160.0         # tip to tip along the bow
BOW_BRACE = 15.0           # grip to the string (brace height), toward the archer
ARROW_LENGTH = 78.0        # nock to the point
ARROW_GRIP = 2.0           # nock to the right hand's grip point (the gestures' "knob_offset")
QUIVER_ANCHOR = (-11.0, -28.0, 104.0)   # bottom of the quiver, body terms (reference pose), behind his left hip
QUIVER_TOP = (25.0, 0.0, 56.0)          # its opening relative to the anchor: above and behind the right shoulder
FEATHER = (226, 222, 210)


def bow_local(sk):
    """The bow's frame in hand_l's reference frame: X along the bow, up when the thumb is up (the hand's pinky-to-index
    axis), Y toward the string (back along the forearm, toward the archer), Z = X x Y; origin at the grip point."""
    fingers, across, palm, grip = hand_axes(sk, "l")
    x = anim_rig.normalize(across)
    y = anim_rig.scale(fingers, -1.0)
    y = anim_rig.normalize(anim_rig.sub(y, anim_rig.scale(x, anim_rig.dot(y, x))))
    z = anim_rig.cross(x, y)
    bt, bq = sk.ref_world[sk.index["hand_l"]]
    rotation = anim_rig.qfrom_matrix((x, y, z))
    return (anim_rig.qrot(anim_rig.qconj(bq), anim_rig.sub(grip, bt)),
            anim_rig.qnormalize(anim_rig.qmul(anim_rig.qconj(bq), rotation)))


def make_bow(sk, light=False):
    sides = 4 if light else 6
    half = BOW_LENGTH / 2.0
    tip_y = BOW_BRACE - 1.5
    # The braced limbs: thickest at the grip, bending toward the string to the tips (limb points in (x, y)).
    limb = [(0.0, 0.0, 1.9), (half * 0.45, 3.5, 1.5), (half * 0.8, 9.0, 1.1), (half, tip_y, 0.6)]
    if light:
        limb = [limb[0], limb[2], limb[3]]
    prims = [pp.frustum_between((-6.0, 0.0, 0.0), (6.0, 0.0, 0.0), (2.4, 2.4), (2.4, 2.4), sides, LEATHER,
                                x_hint=(0.0, 1.0, 0.0))]  # leather grip
    for sign in (1.0, -1.0):
        for (xa, ya, ra), (xb, yb, rb) in zip(limb, limb[1:]):
            prims.append(pp.frustum_between((sign * xa, ya, 0.0), (sign * xb, yb, 0.0), (ra, ra * 1.2),
                                            (rb, rb * 1.2), sides, pp.WOOD_DARK, x_hint=(0.0, 1.0, 0.0)))
    # The string, straight between the tips.
    prims.append(pp.frustum_between((half, tip_y + 0.4, 0.0), (-half, tip_y + 0.4, 0.0), (0.35, 0.35), (0.35, 0.35),
                                    3, FEATHER, caps=()))
    return {"id": 4, "name": "bow", "label": "arc", "bone": "hand_l", "frame": "bone", "local": bow_local(sk),
            "prims": prims}


def fletching(x0, x1, color, radius=0.5):
    """Three vanes around the shaft (tool frame, X along it) from x0 to x1."""
    vanes = []
    for k in range(3):
        a = 2.0 * math.pi * k / 3.0
        out = (0.0, math.cos(a), math.sin(a))
        q = pp.rotation_xz((1.0, 0.0, 0.0), anim_rig.cross((1.0, 0.0, 0.0), out))
        vanes.append(pp.prism([(x0, radius), (x1 - 2.0, radius + 2.2), (x1, radius)], 0.3,
                              anim_rig.scale(anim_rig.cross((1.0, 0.0, 0.0), out), -0.15), q, color))
    return vanes


def make_arrow(light=False):
    prims = [pp.frustum_between((0.0, 0.0, 0.0), (ARROW_LENGTH - 5.0, 0.0, 0.0), (0.5, 0.5), (0.5, 0.5),
                                3 if light else 4, WOOD, x_hint=pp.FLAT),
             pp.prism([(ARROW_LENGTH - 6.0, -1.2), (ARROW_LENGTH, 0.0), (ARROW_LENGTH - 6.0, 1.2)], 0.5,
                      (0.0, 0.0, -0.25), pp.IDENTITY, IRON)]
    prims += fletching(2.0, 14.0, FEATHER)[:1 if light else 3]
    return {"id": 5, "name": "arrow", "label": "fleche", "bone": "ik_hand_gun", "frame": "tool", "prims": prims}


def make_quiver(light=False):
    sides = 6 if light else 8
    b = pp.body_to_prop
    top = b(QUIVER_TOP)
    axis = anim_rig.normalize(top)
    prims = [pp.frustum_between((0.0, 0.0, 0.0), top, (4.5, 5.5), (5.5, 6.5), sides, LEATHER, caps=("bottom",),
                                x_hint=(1.0, 0.0, 0.0)),
             player(pp.frustum_between(anim_rig.scale(top, 0.82), anim_rig.scale(top, 0.9), (5.6, 6.6), (5.7, 6.7),
                                       sides, PLAYER_PREVIEW, caps=(), x_hint=(1.0, 0.0, 0.0)))]  # band in his colour
    # Arrows standing out of the opening: shafts and fletchings.
    for k, (dr, df) in enumerate(((0.0, 0.0), (2.5, 1.5), (-2.5, 1.0), (1.5, -2.0), (-1.5, -2.0))[:3 if light else 5]):
        start = anim_rig.add(top, b((dr, df, -4.0)))
        end = anim_rig.add(anim_rig.add(top, b((dr, df, 0.0))), anim_rig.scale(axis, 10.0 + k % 2 * 2.0))
        prims.append(pp.box(end, (2.0, 2.0, 8.0), FEATHER, pp.rotation_z_to(axis, (1.0, 0.0, 0.0))))
        prims.append(pp.frustum_between(start, end, (0.5, 0.5), (0.5, 0.5), 3, WOOD, caps=()))
    return {"id": 6, "name": "quiver", "label": "carquois", "bone": "spine_03", "frame": "body",
            "anchor": QUIVER_ANCHOR, "prims": prims}


# Crown of the King (V5.3, provisional; user's trial of 2026-10-25: big, floating above his head, all gold): a band
# closed on top (no inside to show) between two darker rims, five merlons, about 1 m above his head in game. Skinned to
# the pelvis ("body" frame): it follows his walk and his fight and falls with him (the root would leave it floating 30 s
# above his body, HOLCorpse::HoldSeconds). 3 times the first one: 85 cm across in the mesh, about 1.3 m in game at
# MeshScale 1.5, so it reads at the RTS camera's starting height.
CROWN_ANCHOR = (0.0, 0.0, 245.0)  # bottom of the band, body terms (reference pose): 63 cm above the head in the mesh
CROWN_RADIUS, CROWN_HEIGHT, CROWN_MERLON = 39.0, 24.0, 18.0
GOLD = (222, 178, 52)


def make_crown(light=False):
    sides = 8 if light else 12
    r, h = CROWN_RADIUS, CROWN_HEIGHT
    prims = [pp.frustum_between((0.0, 0.0, 0.0), (0.0, 0.0, h), (r, r), (r, r), sides, GOLD, caps=("top",)),
             pp.frustum_between((0.0, 0.0, -1.5), (0.0, 0.0, 4.5), (r + 2.4, r + 2.4), (r + 2.4, r + 2.4), sides, BRASS,
                                caps=("bottom",)),
             pp.frustum_between((0.0, 0.0, h - 4.5), (0.0, 0.0, h + 0.9), (r + 1.8, r + 1.8), (r + 1.8, r + 1.8),
                                sides, BRASS, caps=("top",))]
    for k in range(5):
        a = 2.0 * math.pi * k / 5.0
        centre = (math.cos(a) * (r - 3.0), math.sin(a) * (r - 3.0), h + CROWN_MERLON / 2.0)
        prims.append(pp.box(centre, (13.5, 13.5, CROWN_MERLON), GOLD,
                            pp.rotation_z_to((0.0, 0.0, 1.0), (math.cos(a), math.sin(a), 0.0))))
    return {"id": 7, "name": "crown", "label": "couronne", "bone": "pelvis", "frame": "body", "anchor": CROWN_ANCHOR,
            "prims": prims}


# Crossbowman (AR1): stirrup crossbow, loaded bolt, bolt quiver on the right hip (see the module's docstring).
CROSSBOW_LENGTH = 95.0       # stirrup's front end to the butt (the tool's "handle_length")
CROSSBOW_TILLER = (10.0, 95.0)   # the tiller (stock) along X: 85 cm
CROSSBOW_PROD_X = 13.0       # the prod's centre on the tiller
CROSSBOW_PROD_HALF = 35.0    # 70 cm across
CROSSBOW_PROD_DRAW = 7.0     # the drawn tips' setback toward the butt
CROSSBOW_NUT = 34.0          # the nut, where the spanned string rests
CROSSBOW_LEFT_GRIP = 22.0    # stirrup's end to the left hand (the gestures' "knob_offset")
CROSSBOW_HAND_SPACING = 45.0  # left hand to the right hand (the trigger)
CROSSBOW_TOP = 2.5           # the tiller's top (Y) in front of the butt: the groove
BOLT_LENGTH = 35.0
# AR3 (user's choice at the AR2 previews, 2026-10-26): the prod thickened from ~1.7 cm to ~3 cm seen from above (its
# width along the tiller: 3.6 cm at the centre, 2.25 at the tips; 1.7 cm did not read at the RTS camera) and its steel
# lighter than the weapons' STEEL, so the bar reads against the wood and the ground.
CROSSBOW_PROD_STEEL = (204, 208, 214)
BOLT_QUIVER_ANCHOR = (28.0, -2.0, 70.0)  # bottom of the bolt quiver, body terms (reference pose): his right hip
BOLT_QUIVER_TOP = (2.0, -4.0, 30.0)      # its opening relative to the anchor: at the belt, leaning back a little


def make_crossbow(light=False):
    sides = 4 if light else 6
    x0, x1 = CROSSBOW_TILLER
    top = CROSSBOW_TOP
    prims = [
        # Tiller: a beam in front, the deeper butt behind (prisms in (x, y), 4 cm thick across Z).
        pp.prism([(x0, -top), (70.0, -top), (70.0, top), (x0, top)], 4.0, (0.0, 0.0, -2.0), pp.IDENTITY, WOOD),
        pp.prism([(66.0, -3.0), (x1, -8.0), (x1, top), (66.0, top)], 4.4, (0.0, 0.0, -2.2), pp.IDENTITY, pp.WOOD_DARK),
        # Iron stirrup in front of the prod: two side bars and the foot's bar, in the prod's plane (X, Z).
        pp.box((0.8, 0.0, 0.0), (1.6, 1.6, 15.0), IRON),
        pp.box((7.0, 0.0, 6.6), (13.0, 1.4, 1.4), IRON, pp.rotation_xz((13.0, 0.0, -1.5), (0.0, 0.0, 1.0))),
        pp.box((7.0, 0.0, -6.6), (13.0, 1.4, 1.4), IRON, pp.rotation_xz((13.0, 0.0, 1.5), (0.0, 0.0, 1.0))),
    ]
    # Steel prod across Z, its drawn tips set back toward the butt (points along it: (x, z, half thickness)).
    px, half, draw = CROSSBOW_PROD_X, CROSSBOW_PROD_HALF, CROSSBOW_PROD_DRAW
    limb = [(px, 0.0, 2.0), (px + draw * 0.3, half * 0.55, 1.7), (px + draw, half, 1.25)]
    if light:
        limb = [limb[0], limb[2]]
    for sign in (1.0, -1.0):
        for (xa, za, ra), (xb, zb, rb) in zip(limb, limb[1:]):
            prims.append(pp.frustum_between((xa, 0.0, sign * za), (xb, 0.0, sign * zb), (ra * 0.9, ra),
                                            (rb * 0.9, rb), sides, CROSSBOW_PROD_STEEL, x_hint=(1.0, 0.0, 0.0)))
    # The string, straight from each tip to the nut (spanned), on the groove.
    tip = px + draw
    for sign in (1.0, -1.0):
        prims.append(pp.frustum_between((tip, 0.3, sign * half), (CROSSBOW_NUT, top + 0.3, 0.0), (0.35, 0.35),
                                        (0.35, 0.35), 3, FEATHER, caps=()))
    if not light:
        # Nut (across the groove) and the long trigger lever under the stock.
        prims.append(pp.frustum_between((CROSSBOW_NUT, top - 0.6, -2.4), (CROSSBOW_NUT, top - 0.6, 2.4), (1.6, 1.6),
                                        (1.6, 1.6), 6, FEATHER))
        prims.append(pp.box((CROSSBOW_NUT + 14.0, -top - 1.2, 0.0), (24.0, 1.2, 1.4), IRON,
                            pp.rotation_xz((24.0, -2.0, 0.0), (0.0, 0.0, 1.0))))
    return {"id": 8, "name": "crossbow", "label": "arbalete", "bone": "ik_hand_gun", "frame": "tool", "prims": prims}


def bolt_prims(x0, light, along=1.0):
    """A bolt from its nock at x0 along +X (along=1) or -X: square head, two vanes in the (X, Z) plane."""
    def at(d, y=0.0, z=0.0):
        return (x0 + along * d, y, z)
    prims = [pp.frustum_between(at(0.0), at(BOLT_LENGTH - 4.0), (0.7, 0.7), (0.7, 0.7), 3 if light else 4, WOOD,
                                x_hint=pp.FLAT),
             pp.frustum_between(at(BOLT_LENGTH - 4.5), at(BOLT_LENGTH), (1.2, 1.2), (0.0, 0.0), 4, IRON,
                                x_hint=pp.FLAT)]
    if not light:
        for sign in (1.0, -1.0):
            prims.append(pp.box(at(4.0, 0.0, sign * 1.4), (6.0, 0.3, 2.2), FEATHER))
    return prims


def bolt_local(sk):
    """The crossbow's frame (ik_hand_gun's, identity) in ik_hand_r's reference frame: ik_hand_r is a child of
    ik_hand_gun never keyed, so a piece bound this way follows the crossbow exactly."""
    gt, gq = sk.ref_world[sk.index["ik_hand_gun"]]
    rt, rq = sk.ref_world[sk.index["ik_hand_r"]]
    return (anim_rig.qrot(anim_rig.qconj(rq), anim_rig.sub(gt, rt)), anim_rig.qnormalize(anim_rig.qmul(anim_rig.qconj(rq), gq)))


def make_bolt(sk, light=False):
    """The loaded bolt in the groove: nock at the nut, head forward (toward the stirrup)."""
    prims = bolt_prims(CROSSBOW_NUT, light, -1.0)
    for prim in prims:  # lift it onto the groove
        prim["t"] = anim_rig.add(prim["t"], (0.0, CROSSBOW_TOP + 0.8, 0.0))
    return {"id": 5, "name": "bolt", "label": "carreau", "bone": "ik_hand_r", "frame": "bone", "local": bolt_local(sk),
            "prims": prims}


def make_bolt_quiver(light=False):
    sides = 6 if light else 8
    b = pp.body_to_prop
    top = b(BOLT_QUIVER_TOP)
    axis = anim_rig.normalize(top)
    # Flat against the hip: wider forward (local X toward forward) than out to the side.
    prims = [pp.frustum_between((0.0, 0.0, 0.0), top, (5.0, 3.6), (6.0, 4.2), sides, LEATHER, caps=("bottom",),
                                x_hint=(1.0, 0.0, 0.0)),
             player(pp.frustum_between(anim_rig.scale(top, 0.8), anim_rig.scale(top, 0.9), (6.1, 4.3), (6.2, 4.4),
                                       sides, PLAYER_PREVIEW, caps=(), x_hint=(1.0, 0.0, 0.0)))]  # band in his colour
    # Bolt ends standing out of the opening: shafts and two vanes each.
    for k, (dr, df) in enumerate(((0.0, 2.5), (0.0, -2.5), (1.5, 0.0), (-1.5, 0.5))[:2 if light else 4]):
        start = anim_rig.add(top, b((dr, df, -4.0)))
        end = anim_rig.add(anim_rig.add(top, b((dr, df, 0.0))), anim_rig.scale(axis, 6.0 + k % 2 * 1.5))
        prims.append(pp.box(end, (2.4, 0.4, 5.0), FEATHER, pp.rotation_z_to(axis, (1.0, 0.0, 0.0))))
        prims.append(pp.frustum_between(start, end, (0.6, 0.6), (0.6, 0.6), 3, WOOD, caps=()))
    return {"id": 6, "name": "bolt_quiver", "label": "carquois de carreaux", "bone": "pelvis", "frame": "body",
            "anchor": BOLT_QUIVER_ANCHOR, "prims": prims}


# Hero, the Paladin-Commander (J8a.0, provisional; J8a-Q3): the Man-at-arms' gestures, MeshScale 1.8 in game (x1.2),
# long sword, shield with a gold sun; J8a.0c: dressed as a paladin (tabard, pauldrons, mantle, helm), every piece glued
# to a body bone. Mesh: 1 cm in the mesh = 1.8 cm in game.
LONGSWORD_LENGTH = 112.0   # pommel to point (the gestures keep the sword's grip, SWORD_GRIP)
LONGSWORD_GUARD = 21.0     # pommel to the crossguard: a hand-and-a-half grip
LONGSWORD_WIDTH = 5.2
WHITE = (236, 234, 224)
SUN_CENTRE = 1.0           # the shield's sun: its centre above the board's centre (cm, frame Z)
SUN_RADIUS, SUN_RAY = 7.5, 17.0
# J8a.0c pieces: "body" frame with the anchor at the body origin, so their points are body terms (forward, right, up).
# Panels: inner-face paths (forward, up), reference pose, kept ~0.7 cm off the skin.
TABARD_BONE = "spine_02"
TABARD_FRONT = [(14.4, 104.0), (14.4, 144.0), (6.5, 155.0)]   # belt to the chest, then up to the collar
TABARD_FRONT_HALF = [14.0, 15.0, 9.0]
TABARD_BACK = [(-13.8, 106.0), (-16.6, 123.0), (-22.7, 141.0)]  # belt to the shoulder blades (the mantle above)
TABARD_BACK_HALF = [13.0, 15.0, 16.0]
PANEL_THICK, RIM = 1.0, 1.4
MANTLE_BONE = "spine_03"
MANTLE_PATH = [(-23.4, 131.0), (-23.4, 148.0), (-16.6, 161.0)]  # mid-back up over the trapezius
MANTLE_HALF = [18.5, 17.5, 11.0]
PAULDRON_BONE = "clavicle_%s"
PAULDRON_CENTRE = (17.7, -9.7, 149.5)   # right side, body terms (right, forward, up): the shoulder joint
PAULDRON_AXIS = (0.55, 0.0, 0.835)      # out and up from the joint (right, forward, up)
PAULDRON_RADII = (14.0, 12.5)           # forward, across the axis
PAULDRON_HEIGHT = 14.5                  # joint to the top of the dome along the axis
HELM_BONE = "head"
HELM_CENTRE = (-1.5, 0.0)               # forward, right
HELM_LOW, HELM_HIGH, HELM_TOP = 160.0, 178.0, 186.0
HELM_RADII = (12.8, 10.6)               # forward, right
CREST_AT, CREST_RADIUS, CREST_RAY = 195.0, 6.0, 12.0


def ccw(points):
    """The polygon counter-clockwise (the prism's convention)."""
    area = sum(a[0] * b[1] - b[0] * a[1] for a, b in zip(points, points[1:] + points[:1]))
    return points if area > 0.0 else points[::-1]


def make_longsword(light=False):
    sides = 4 if light else 6
    tip, guard = LONGSWORD_LENGTH, LONGSWORD_GUARD
    w = LONGSWORD_WIDTH
    prims = [
        pp.frustum_between((1.5, 0.0, 0.0), (guard, 0.0, 0.0), (1.6, 1.4), (1.4, 1.3), sides, LEATHER, x_hint=pp.FLAT),
        pp.box((guard + 0.9, 0.0, 0.0), (1.8, 24.0, 2.2), GOLD),  # wide gold crossguard
        pp.prism([(guard + 1.8, -w / 2), (tip - 14.0, -w / 2 + 0.7), (tip, 0.0), (tip - 14.0, w / 2 - 0.7),
                  (guard + 1.8, w / 2)], 1.0, (0.0, 0.0, -0.5), pp.IDENTITY, STEEL),
    ]
    if not light:
        prims.append(pp.box((0.8, 0.0, 0.0), (3.6, 4.4, 3.6), GOLD))  # pommel
    return {"id": 2, "name": "longsword", "label": "epee longue", "bone": "ik_hand_gun", "frame": "tool",
            "prims": prims}


def make_sun_shield(sk, light=False):
    """The Man-at-arms' heater shield (same frame and outline), a gold sun on the player's colour."""
    q = pp.rotation_z_to((1.0, 0.0, 0.0), (0.0, 0.0, 1.0))  # prism (u, v) -> frame (Z = u, Y = -v), extruded along X
    steps = 2 if light else 4
    face = 1.4 + 0.8
    prims = [pp.prism(heater_outline(1.0, steps), 1.4, (0.0, 0.0, 0.0), q, IRON),
             player(pp.prism(heater_outline(0.94, steps), 0.8, (1.4, 0.0, 0.0), q, PLAYER_PREVIEW)),
             # The sun's disc (also the boss) and its rays, standing out of the face.
             pp.frustum_between((face - 0.2, 0.0, SUN_CENTRE), (face + 1.2, 0.0, SUN_CENTRE),
                                (SUN_RADIUS, SUN_RADIUS), (SUN_RADIUS * 0.85, SUN_RADIUS * 0.85), 8 if light else 12,
                                GOLD, x_hint=(0.0, 0.0, 1.0))]
    rays = 8
    for k in range(0, rays, 2 if light else 1):  # light: the four long rays
        a = 2.0 * math.pi * (k + 0.5) / rays
        up, side = math.cos(a), math.sin(a)
        length = SUN_RAY if k % 2 == 0 else SUN_RAY * 0.75
        half = 2.6

        def at(r, s):  # (radial, sideways) to the prism's (u, v)
            return (SUN_CENTRE + up * r - side * s, -(side * r + up * s))
        prims.append(pp.prism(ccw([at(SUN_RADIUS - 1.0, -half), at(length, 0.0), at(SUN_RADIUS - 1.0, half)]), 0.6,
                              (face - 0.1, 0.0, 0.0), q, GOLD))
    return {"id": 3, "name": "sun_shield", "label": "bouclier au soleil", "bone": "hand_l", "frame": "bone",
            "local": shield_local(sk), "prims": prims}


def body_piece(ident, name, label, bone, prims):
    """A piece in the "body" frame anchored at the body origin: its points are body terms (forward, right, up)."""
    return {"id": ident, "name": name, "label": label, "bone": bone, "frame": "body", "anchor": (0.0, 0.0, 0.0),
            "prims": prims}


def sun_prims(centre, normal, up, radius, ray, light, depth=0.8):
    """A gold sun (disc and eight rays) standing out of a face along `normal`; `up` = the rays' first axis."""
    out = anim_rig.add(centre, anim_rig.scale(normal, depth))
    prims = [pp.frustum_between(centre, out, (radius, radius), (radius * 0.85, radius * 0.85), 8 if light else 12,
                                GOLD, x_hint=up)]
    q = pp.rotation_xz(up, normal)  # local X = up, Z = normal
    for k in range(0, 8, 2 if light else 1):
        a = 2.0 * math.pi * (k + 0.5) / 8
        d, s = (math.cos(a), math.sin(a)), (-math.sin(a), math.cos(a))
        length = ray if k % 2 == 0 else ray * 0.75
        half, base = radius * 0.35, radius - 1.0
        tri = [(d[0] * base + s[0] * half, d[1] * base + s[1] * half), (d[0] * length, d[1] * length),
               (d[0] * base - s[0] * half, d[1] * base - s[1] * half)]
        prims.append(pp.prism(ccw(tri), depth * 0.7, centre, q, GOLD))
    return prims


def panel(path, halves, outward, light, colour=WHITE, rim=GOLD):
    """A cloth panel along a path of inner-face points (forward, up), PANEL_THICK thick outward (+1 forward, -1 back);
    a gold rim along its sides and free ends, a little lower than the cloth."""
    prims = []
    last = len(path) - 2
    for k, ((f0, u0), (f1, u1)) in enumerate(zip(path, path[1:])):
        seg = anim_rig.normalize((f1 - f0, 0.0, u1 - u0))
        normal = (seg[2], 0.0, -seg[0])
        if normal[0] * outward < 0.0:
            normal = anim_rig.scale(normal, -1.0)
        q = pp.rotation_xz(seg, normal)
        length = math.hypot(f1 - f0, u1 - u0)
        a, b = (-0.5 if k > 0 else 0.0), length + (0.5 if k < last else 0.0)  # overlap at the joints
        w0, w1 = halves[k], halves[k + 1]
        prims.append(pp.prism(ccw([(a, -w0), (b, -w1), (b, w1), (a, w0)]), PANEL_THICK, (f0, 0.0, u0), q, colour))
        if rim and not light:
            ra, rb = a - (RIM if k == 0 else 0.0), b + (RIM if k == last else 0.0)
            prims.append(pp.prism(ccw([(ra, -w0 - RIM), (rb, -w1 - RIM), (rb, w1 + RIM), (ra, w0 + RIM)]),
                                  PANEL_THICK - 0.35, (f0, 0.0, u0), q, rim))
    return prims


def panel_point(path, k, at, outward):
    """A point on the outer face of a panel's segment k, `at` of its length, and the face's normal."""
    (f0, u0), (f1, u1) = path[k], path[k + 1]
    seg = anim_rig.normalize((f1 - f0, 0.0, u1 - u0))
    normal = (seg[2], 0.0, -seg[0])
    if normal[0] * outward < 0.0:
        normal = anim_rig.scale(normal, -1.0)
    return anim_rig.add((f0 + (f1 - f0) * at, 0.0, u0 + (u1 - u0) * at), anim_rig.scale(normal, PANEL_THICK)), \
        normal, seg


def make_tabard(light=False):
    """White tabard, front and back, belt to the collar; a gold sun on the chest and on the back."""
    prims = panel(TABARD_FRONT, TABARD_FRONT_HALF, 1.0, light) + panel(TABARD_BACK, TABARD_BACK_HALF, -1.0, light)
    centre, normal, seg = panel_point(TABARD_FRONT, 0, 0.6, 1.0)
    prims += sun_prims(centre, normal, seg, 6.0, 12.5, light)
    centre, normal, seg = panel_point(TABARD_BACK, 0, 0.75, -1.0)
    prims += sun_prims(centre, normal, seg, 5.0, 10.5, light)
    return body_piece(1, "tabard", "tabard", TABARD_BONE, prims)


def make_mantle(light=False):
    """Short white mantle on the upper back, between the pauldrons; gold hem."""
    return body_piece(4, "mantle", "pelerine", MANTLE_BONE, panel(MANTLE_PATH, MANTLE_HALF, -1.0, light))


def make_pauldron(side, light=False):
    """Big rounded gold pauldron: a dome over the shoulder joint, out and up."""
    sign = 1.0 if side == "r" else -1.0
    r, f, u = PAULDRON_CENTRE
    centre = (f, sign * r, u)  # (forward, right, up)
    axis = anim_rig.normalize((PAULDRON_AXIS[1], sign * PAULDRON_AXIS[0], PAULDRON_AXIS[2]))
    rf, ra = PAULDRON_RADII

    def at(s):
        return anim_rig.add(centre, anim_rig.scale(axis, s))

    def radii(s):
        k = math.sqrt(max(0.0, 1.0 - (max(s, 0.0) / PAULDRON_HEIGHT) ** 2))
        return (max(rf * k, 0.0), max(ra * k, 0.0))
    steps = [-3.0, 0.0, 8.0, PAULDRON_HEIGHT] if light else [-3.0, 0.0, 5.0, 9.0, 12.0, PAULDRON_HEIGHT]
    sides = 8 if light else 12
    prims = [pp.frustum_between(at(s0), at(s1), radii(s0), radii(s1), sides, GOLD, x_hint=(1.0, 0.0, 0.0),
                                caps=("bottom", "top") if s1 < PAULDRON_HEIGHT else ("bottom",))
             for s0, s1 in zip(steps, steps[1:])]
    if not light:  # a darker lip at its edge
        prims.append(pp.frustum_between(at(-3.6), at(-1.6), (rf + 0.8, ra + 0.8), (rf + 0.8, ra + 0.8), sides,
                                        pp.GOLD_DARK, x_hint=(1.0, 0.0, 0.0)))
    return body_piece(5 if side == "l" else 6, "pauldron_" + side,
                      "epauliere " + ("gauche" if side == "l" else "droite"), PAULDRON_BONE % side, prims)


def make_helm(light=False):
    """Closed great helm, eye slit and gold cross on the face, gold sun crest on top."""
    f, r = HELM_CENTRE
    rf, rr = HELM_RADII
    sides = 10 if light else 16
    prims = [pp.frustum_between((f, r, HELM_LOW), (f, r, HELM_HIGH), (rf, rr), (rf, rr), sides, STEEL),
             pp.frustum_between((f, r, HELM_HIGH), (f, r, HELM_TOP), (rf, rr), (rf * 0.55, rr * 0.55), sides, STEEL),
             pp.frustum_between((f, r, HELM_LOW - 0.6), (f, r, HELM_LOW + 2.4), (rf + 0.6, rr + 0.6),
                                (rf + 0.6, rr + 0.6), sides, GOLD),             # gold band at the bottom
             pp.box((f + rf - 0.2, r, 171.0), (1.6, 15.0, 1.8), IRON)]         # eye slit
    if not light:  # gold cross on the face
        prims.append(pp.box((f + rf - 0.1, r, 165.0), (1.4, 3.0, 12.0), GOLD))
        prims.append(pp.box((f + rf - 0.1, r, 174.5), (1.4, 15.0, 2.4), GOLD))
    # The crest: a stem and a gold sun facing forward.
    prims.append(pp.box((f, r, (HELM_TOP + CREST_AT) / 2.0 - 1.0), (2.0, 2.0, CREST_AT - HELM_TOP + 1.0), GOLD))
    prims += sun_prims((f - 0.75, r, CREST_AT), (1.0, 0.0, 0.0), (0.0, 0.0, 1.0), CREST_RADIUS, CREST_RAY, light,
                       depth=1.5)
    return body_piece(7, "helm", "heaume", HELM_BONE, prims)


def build_props(sk, light=False):
    props = [make_spear(light), make_sword(light), make_shield(sk, light), make_bow(sk, light), make_arrow(light),
             make_quiver(light), make_crown(light), make_crossbow(light), make_bolt(sk, light),
             make_bolt_quiver(light), make_longsword(light), make_sun_shield(sk, light), make_tabard(light),
             make_mantle(light), make_pauldron("l", light), make_pauldron("r", light), make_helm(light)]
    return {p["name"]: p for p in props}


# The soldiers' meshes (J7a.3) and their pieces, always shown.
SOLDIER_MESHES = {"SKM_Spearman": ("spear",), "SKM_ManAtArms": ("sword", "shield"),
                  # J7c.1a: the arrow is always in the mesh; the gestures hide it (ik_hand_gun scaled to nothing,
                  # soldier_poses "tool_hidden") outside the draw.
                  "SKM_Archer": ("bow", "arrow", "quiver"),
                  # V5.3: the King, the Man-at-arms with a crown.
                  "SKM_King": ("sword", "shield", "crown"),
                  # AR1: the Crossbowman; the loaded bolt is hidden by the gestures after the shot (ik_hand_r).
                  "SKM_Crossbowman": ("crossbow", "bolt", "bolt_quiver"),
                  # J8a.0c: the Hero, a tall Man-at-arms dressed as a paladin (MeshScale 1.8), the Man-at-arms' gestures.
                  "SKM_Hero": ("longsword", "sun_shield", "tabard", "mantle", "pauldron_l", "pauldron_r",
                               "helm")}


def main():
    sk = anim_rig.Skeleton.load(os.path.join(os.path.dirname(os.path.abspath(__file__)), "Data",
                                             "UE4MannequinSkeleton.json"))
    for light in (False, True):
        print("Pieces%s :" % (" legeres" if light else ""))
        for prop in sorted(build_props(sk, light).values(), key=lambda p: p["id"]):
            size = pp.extents(prop)
            tris = pp.prop_triangles(prop)
            verts = len(pp.part_buffers({"key": prop["name"], "mask": 0, "owner": prop, "prims": prop["prims"]},
                                        sk)["vertices"])
            print("  %-3d %-7s os %-12s %-26s %4d triangles %4d sommets" % (
                prop["id"], prop["name"], prop["bone"], "%.0f x %.0f x %.0f cm" % size, len(tris), verts))


if __name__ == "__main__":
    main()
