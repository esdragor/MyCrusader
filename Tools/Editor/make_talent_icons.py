"""Draws the Paladin's talent icons (REGLAGE talents_icons_compare, 2026-11-13: real icons in the style of the abilities'
T_Icon_Aura / T_Icon_CloseRanks / T_Icon_Banner of make_ui_icons.py, not tinted cells with a name nor letter badges).
One icon per talent cell of the tree (rows 2 to 9 = keys 20..92, columns 0 Guardian, 1 Captain, 2 Champion), drawn with PIL
at 256 px (gold and white on transparent, dark edged) then reduced.

  python Tools/Editor/make_talent_icons.py           the 24 PNGs into Tools/Editor/Data/Icons/Talents (T_Talent_<key>.png)
  python Tools/Editor/make_talent_icons.py --sheet <png>
      the comparison sheet out of the game: every icon in three framings (A dark box + family border, B family disc,
      C dark box + family corner) at 64 and 32 px, labelled, for Retours/Q-talents-icons.md
Import into the game: not done yet (waits for the answer to Q-talents-icons; same import as make_ui_icons.py).
"""
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
# Same palette and shapes as make_ui_icons.py (not imported: importing it writes its PNGs, its module code runs at load).
BIG = 256
EMBLEM_EDGE = (28, 20, 10, 255)
GOLD = (232, 182, 48, 255)
GOLD_DARK = (150, 100, 20, 255)
WHITE = (244, 240, 228, 255)


def shield_points(inset):
    """A heater shield in the 256 px square, `inset` px inside the outline (make_ui_icons.shield_points)."""
    top, left, right = 22 + inset, 34 + inset, 222 - inset
    points = [(left, top), (right, top), (right, 120)]
    tip = (128, 236 - inset * 1.4)
    for step in range(1, 12):
        t = step / 12.0
        points.append((right + (tip[0] - right) * t, 120 + (tip[1] - 120) * math.sin(t * math.pi / 2)))
    points.append(tip)
    for step in range(11, 0, -1):
        t = step / 12.0
        points.append((left + (tip[0] - left) * t, 120 + (tip[1] - 120) * math.sin(t * math.pi / 2)))
    points.append((left, 120))
    return points


def sun_points(centre, inner, outer, rays):
    """A sun's star (make_ui_icons.sun_points)."""
    points = []
    for step in range(rays * 2):
        angle = math.pi * step / rays - math.pi / 2
        radius = outer if step % 2 == 0 else inner
        points.append((centre[0] + radius * math.cos(angle), centre[1] + radius * math.sin(angle)))
    return points


PNG_FOLDER = os.path.join(HERE, "Data", "Icons", "Talents")
SIZE = 64
EDGE = EMBLEM_EDGE
STEEL = (196, 204, 214, 255)
STEEL_DARK = (70, 76, 88, 255)
RED = (214, 64, 52, 255)
GREEN = (90, 190, 90, 255)
# The tree's family colours (HOLGameScreenScript.as TalentFamilyColour), 0..255.
FAMILY = [(89, 153, 255), (255, 204, 77), (255, 115, 102)]
FAMILY_NAME = ["Gardien", "Capitaine", "Champion"]
NAMES = {
    20: "Moisson benie", 21: "Boucliers leves", 22: "Lame benie",
    30: "Vigie de l'Aube", 31: "Mains du guerisseur", 32: "Endurance du croise",
    40: "Etendard du rempart", 41: "Pas de l'Aube", 42: "Cri du defi",
    50: "Remparts de lumiere", 51: "Grace abondante", 52: "Contre parfait",
    60: "Double etendard", 61: "Freres d'armes", 62: "Escorte du champion",
    70: "Banniere benie", 71: "Charge disciplinee", 72: "Fer de lance",
    80: "Rempart eternel", 81: "Ordre serre", 82: "Sentence publique",
    90: "Jugement des assiegeants", 91: "Renforts aguerris", 92: "Verdict des armes",
}


def new():
    from PIL import Image, ImageDraw
    image = Image.new("RGBA", (BIG, BIG), (0, 0, 0, 0))
    return image, ImageDraw.Draw(image)


def edged_polygon(draw, points, fill, width=8):
    draw.polygon(points, fill=EDGE)
    cx = sum(x for x, _ in points) / len(points)
    cy = sum(y for _, y in points) / len(points)
    inner = []
    for x, y in points:
        dx, dy = x - cx, y - cy
        length = max(1.0, math.hypot(dx, dy))
        inner.append((x - dx / length * width, y - dy / length * width))
    draw.polygon(inner, fill=fill)


def sun(draw, centre, scale=1.0):
    draw.polygon(sun_points(centre, 30 * scale, 60 * scale, 12), fill=GOLD_DARK)
    draw.polygon(sun_points(centre, 25 * scale, 51 * scale, 12), fill=GOLD)
    r = 22 * scale
    draw.ellipse((centre[0] - r, centre[1] - r, centre[0] + r, centre[1] + r), fill=WHITE)


def shield(draw, dx=0, dy=0, scale=1.0, field=WHITE):
    def place(points):
        return [(128 + (x - 128) * scale + dx, 128 + (y - 128) * scale + dy) for x, y in points]
    draw.polygon(place(shield_points(0)), fill=EDGE)
    draw.polygon(place(shield_points(10)), fill=GOLD)
    draw.polygon(place(shield_points(24)), fill=field)


def sword(draw, angle_deg, length=210, width=30, centre=(128, 128), blade=STEEL):
    a = math.radians(angle_deg)
    c, s = math.cos(a), math.sin(a)

    def at(x, y):
        return (centre[0] + x * c - y * s, centre[1] + x * s + y * c)
    half, w = length / 2.0, width / 2.0
    for inset, colour, guard_colour in ((0, EDGE, EDGE), (6, blade, GOLD)):
        wi = w - inset
        draw.polygon([at(-half + inset, 0), at(-half + 24, -wi), at(half - 52, -wi), at(half - 52, wi), at(-half + 24, wi)],
                     fill=colour)
        g = 12 - inset
        draw.polygon([at(half - 52 - g, -32 + inset), at(half - 52 + g, -32 + inset), at(half - 52 + g, 32 - inset),
                      at(half - 52 - g, 32 - inset)], fill=guard_colour)
        h = 9 - inset
        draw.polygon([at(half - 52, -h), at(half - inset, -h), at(half - inset, h), at(half - 52, h)], fill=guard_colour)


def tower(draw, left=70, right=186, top=70, bottom=240):
    edged_polygon(draw, [(left, top), (right, top), (right, bottom), (left, bottom)], STEEL, 8)
    step = (right - left) / 5.0
    for i in (0, 2, 4):
        x0 = left + step * i
        draw.rectangle((x0, top - 34, x0 + step, top), fill=EDGE)
        draw.rectangle((x0 + 6, top - 28, x0 + step - 6, top + 6), fill=STEEL)
    draw.rectangle((112, bottom - 60, 144, bottom - 6), fill=EDGE)


def wall(draw, top=120, bottom=232):
    edged_polygon(draw, [(20, top), (236, top), (236, bottom), (20, bottom)], STEEL, 8)
    for i in range(5):
        x0 = 20 + i * 46
        draw.rectangle((x0, top - 30, x0 + 30, top), fill=EDGE)
        draw.rectangle((x0 + 6, top - 24, x0 + 24, top + 6), fill=STEEL)
    for row, y in enumerate(range(top + 34, bottom - 6, 34)):
        draw.line((24, y, 232, y), fill=STEEL_DARK, width=4)


def banner(draw, dx=0, scale=1.0, mark="sun"):
    def p(x, y):
        return (128 + (x - 128) * scale + dx, 128 + (y - 128) * scale)
    draw.rectangle((*p(50, 26), *p(70, 244)), fill=EDGE)
    draw.rectangle((*p(55, 30), *p(65, 240)), fill=GOLD)
    draw.polygon([p(66, 40), p(226, 40), p(226, 184), p(190, 150), p(154, 184), p(66, 184)], fill=EDGE)
    draw.polygon([p(72, 48), p(218, 48), p(218, 166), p(190, 138), p(160, 166), p(72, 166)], fill=WHITE)
    centre = p(140, 102)
    if mark == "sun":
        draw.polygon(sun_points(centre, 18 * scale, 38 * scale, 12), fill=GOLD)
    elif mark == "cross":
        cross(draw, centre, 36 * scale, 12 * scale, GOLD)


def cross(draw, centre, half, thick, colour, edge=True):
    x, y = centre
    if edge:
        e = thick + 6
        draw.rectangle((x - half - 6, y - e, x + half + 6, y + e), fill=EDGE)
        draw.rectangle((x - e, y - half - 6, x + e, y + half + 6), fill=EDGE)
    draw.rectangle((x - half, y - thick, x + half, y + thick), fill=colour)
    draw.rectangle((x - thick, y - half, x + thick, y + half), fill=colour)


def helmet(draw, cx=128, cy=128, scale=1.0):
    def p(x, y):
        return (cx + x * scale, cy + y * scale)
    outline = [p(-70, 80), p(-70, -20), p(-50, -62), p(0, -84), p(50, -62), p(70, -20), p(70, 80)]
    edged_polygon(draw, outline, STEEL, 8 * scale)
    draw.rectangle((*p(-56, -8), *p(56, 6)), fill=EDGE)
    draw.rectangle((*p(-5, -8), *p(5, 60)), fill=EDGE)
    draw.rectangle((*p(-6, -84), *p(6, -20)), fill=GOLD)


def arrow_chevrons(draw, count=3, colour=GOLD):
    for i in range(count):
        x = 50 + i * 54
        pts = [(x, 60), (x + 44, 60), (x + 104, 128), (x + 44, 196), (x, 196), (x + 60, 128)]
        draw.polygon(pts, fill=EDGE)
        inner = [(x + 12, 70), (x + 40, 70), (x + 92, 128), (x + 40, 186), (x + 12, 186), (x + 64, 128)]
        draw.polygon(inner, fill=colour)


def scales(draw):
    draw.rectangle((122, 40, 134, 220), fill=EDGE)
    draw.rectangle((125, 44, 131, 216), fill=GOLD)
    draw.rectangle((80, 214, 176, 232), fill=EDGE)
    draw.rectangle((84, 218, 172, 228), fill=GOLD)
    draw.line((40, 70, 216, 70), fill=EDGE, width=14)
    draw.line((44, 70, 212, 70), fill=GOLD, width=6)
    for x in (54, 202):
        draw.line((x, 70, x - 30, 140), fill=EDGE, width=5)
        draw.line((x, 70, x + 30, 140), fill=EDGE, width=5)
        draw.chord((x - 42, 110, x + 42, 170), 0, 180, fill=EDGE)
        draw.chord((x - 34, 114, x + 34, 162), 0, 180, fill=GOLD)
    draw.ellipse((116, 28, 140, 52), fill=GOLD)


def wheat(draw, dx, angle):
    a = math.radians(angle)
    c, s = math.cos(a), math.sin(a)

    def at(x, y):
        return (128 + dx + x * c - y * s, 150 + x * s + y * c)
    draw.line([at(0, 90), at(0, -40)], fill=EDGE, width=12)
    draw.line([at(0, 88), at(0, -38)], fill=GOLD_DARK, width=5)
    for k in range(5):
        y = -40 - k * 18
        for side in (-1, 1):
            cx, cy = at(side * 12, y)
            draw.ellipse((cx - 13, cy - 18, cx + 13, cy + 18), fill=EDGE)
            draw.ellipse((cx - 8, cy - 13, cx + 8, cy + 13), fill=GOLD)


def draw_talent(key):
    image, d = new()
    if key == 20:  # Moisson benie: three wheat ears under a small sun
        sun(d, (200, 56), 0.55)
        for dx, angle in ((-50, -14), (0, 0), (50, 14)):
            wheat(d, dx, angle)
    elif key == 21:  # Boucliers leves: two shields raised up
        shield(d, -46, -10, 0.66)
        shield(d, 46, -10, 0.66)
        d.polygon([(128, 250), (96, 214), (114, 214), (114, 190), (142, 190), (142, 214), (160, 214)], fill=GOLD)
    elif key == 22:  # Lame benie: an upright sword in a sun
        sun(d, (128, 92), 1.1)
        sword(d, -90, 236, 34)
    elif key == 30:  # Vigie de l'Aube: a tower under a rising sun
        sun(d, (128, 60), 0.75)
        tower(d, 78, 178, 120, 244)
    elif key == 31:  # Mains du guerisseur: the healing gold cross, a second small one
        cross(d, (116, 140), 84, 24, GOLD)
        cross(d, (206, 56), 34, 10, WHITE)
    elif key == 32:  # Endurance du croise: a shield with a red cross
        shield(d)
        cross(d, (128, 116), 52, 14, RED, edge=False)
    elif key == 40:  # Etendard du rempart: a banner over a wall
        wall(d, 168, 240)
        banner(d, 0, 0.62)
    elif key == 41:  # Pas de l'Aube: formation moving forward, chevrons under a shield
        shield(d, -60, 0, 0.62)
        arrow_chevrons(d, 2)
    elif key == 42:  # Cri du defi: a horn with sound arcs
        d.polygon([(30, 150), (150, 90), (150, 210)], fill=EDGE)
        d.polygon([(44, 150), (142, 102), (142, 198)], fill=GOLD)
        for r in (40, 70, 100):
            d.arc((150 - r, 150 - r, 150 + r, 150 + r), -50, 50, fill=EDGE, width=16)
            d.arc((150 - r, 150 - r, 150 + r, 150 + r), -46, 46, fill=WHITE, width=8)
    elif key == 50:  # Remparts de lumiere: a wall in rays of light
        sun(d, (128, 90), 1.0)
        wall(d, 140, 236)
    elif key == 51:  # Grace abondante: two suns (a second charge)
        sun(d, (90, 150), 0.95)
        sun(d, (180, 82), 0.75)
    elif key == 52:  # Contre parfait: two blades crossing with a white spark
        sword(d, 45, 240)
        sword(d, 135, 240)
        d.polygon(sun_points((128, 128), 10, 44, 6), fill=WHITE)
    elif key == 60:  # Double etendard: two banners
        banner(d, -42, 0.72)
        banner(d, 42, 0.72)
    elif key == 61:  # Freres d'armes: two shields overlapping, one in front
        shield(d, -36, 0, 0.78, STEEL)
        shield(d, 36, 8, 0.78)
    elif key == 62:  # Escorte du champion: a helmet flanked by two small shields
        shield(d, -78, 30, 0.44, STEEL)
        shield(d, 78, 30, 0.44, STEEL)
        helmet(d, 128, 128, 1.1)
    elif key == 70:  # Banniere benie: a banner with a gold cross
        banner(d, 0, 1.0, "cross")
    elif key == 71:  # Charge disciplinee: three gold chevrons
        arrow_chevrons(d, 3)
    elif key == 72:  # Fer de lance: a spearhead
        d.polygon([(128, 14), (196, 120), (150, 150), (150, 244), (106, 244), (106, 150), (60, 120)], fill=EDGE)
        d.polygon([(128, 34), (182, 118), (142, 140), (142, 236), (114, 236), (114, 140), (74, 118)], fill=STEEL)
        d.rectangle((104, 146, 152, 166), fill=GOLD)
    elif key == 80:  # Rempart eternel: a tower in a halo ring
        d.ellipse((14, 14, 242, 242), outline=GOLD_DARK, width=18)
        d.ellipse((20, 20, 236, 236), outline=(250, 226, 140, 255), width=8)
        tower(d, 86, 170, 92, 214)
    elif key == 81:  # Ordre serre: a block of 3 x 3 small shields
        for row in range(3):
            for col in range(3):
                shield(d, (col - 1) * 74, (row - 1) * 74, 0.3)
    elif key == 82:  # Sentence publique: a gavel
        hammer = [(60, 60), (150, 30), (176, 104), (86, 134)]
        edged_polygon(d, hammer, GOLD, 9)
        d.line((128, 100, 214, 236), fill=EDGE, width=26)
        d.line((130, 104, 210, 230), fill=GOLD_DARK, width=12)
    elif key == 90:  # Jugement des assiegeants: a sword driven down into a wall
        wall(d, 160, 240)
        sword(d, 90, 220, 32, (128, 110))
    elif key == 91:  # Renforts aguerris: a helmet with a plus
        helmet(d, 112, 140, 1.05)
        cross(d, (204, 58), 34, 11, GOLD)
    elif key == 92:  # Verdict des armes: scales of judgement
        scales(d)
    return image


def write_pngs():
    from PIL import Image
    os.makedirs(PNG_FOLDER, exist_ok=True)
    paths = []
    for key in NAMES:
        path = os.path.join(PNG_FOLDER, "T_Talent_%d.png" % key)
        draw_talent(key).resize((SIZE, SIZE), Image.LANCZOS).save(path)
        paths.append(path)
    return paths


def framed(icon_big, family, variant, size):
    """The icon as the hero's block would show it: A dark box with a family border, B a family-coloured disc, C a dark box
    with a family corner."""
    from PIL import Image, ImageDraw
    box = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    d = ImageDraw.Draw(box)
    colour = FAMILY[family] + (255,)
    dark = (24, 26, 32, 255)
    border = max(2, size // 16)
    if variant == "A":
        d.rectangle((0, 0, size - 1, size - 1), fill=colour)
        d.rectangle((border, border, size - 1 - border, size - 1 - border), fill=dark)
    elif variant == "B":
        d.rectangle((0, 0, size - 1, size - 1), fill=dark)
        d.ellipse((1, 1, size - 2, size - 2), fill=tuple(int(c * 0.55) for c in FAMILY[family]) + (255,))
    else:
        d.rectangle((0, 0, size - 1, size - 1), fill=dark)
        corner = size // 3
        d.polygon([(0, 0), (corner, 0), (0, corner)], fill=colour)
    inner = size - 2 * border - 2
    small = icon_big.resize((inner, inner), Image.LANCZOS)
    box.alpha_composite(small, (border + 1, border + 1))
    return box


def write_sheet(path):
    from PIL import Image, ImageDraw
    keys = list(NAMES)
    cell_w, cell_h = 250, 96
    width = 40 + 3 * cell_w
    height = 60 + len(keys) // 3 * cell_h
    sheet = Image.new("RGBA", (width, height), (52, 54, 60, 255))
    d = ImageDraw.Draw(sheet)
    d.text((10, 8), "Icones de talents : A bordure de famille | B disque de famille | C coin de famille (64 px puis 32 px)",
           fill=(240, 240, 240, 255))
    for family in range(3):
        d.text((40 + family * cell_w, 30), FAMILY_NAME[family], fill=FAMILY[family] + (255,))
    for index, key in enumerate(keys):
        row, family = index // 3, index % 3
        x0, y0 = 40 + family * cell_w, 50 + row * cell_h
        if family == 0:
            d.text((4, y0 + 30), "R%d" % (key // 10), fill=(200, 200, 200, 255))
        big = draw_talent(key)
        x = x0
        for variant in ("A", "B", "C"):
            sheet.alpha_composite(framed(big, family, variant, 64), (x, y0))
            x += 68
        sheet.alpha_composite(framed(big, family, "A", 32), (x, y0))
        sheet.alpha_composite(framed(big, family, "B", 32), (x, y0 + 34))
        d.text((x0, y0 + 68), NAMES[key], fill=(230, 230, 230, 255))
    sheet.save(path)
    return path


if __name__ == "__main__":
    if len(sys.argv) >= 3 and sys.argv[1] == "--sheet":
        print(write_sheet(sys.argv[2]))
    else:
        for found in write_pngs():
            print(found)
