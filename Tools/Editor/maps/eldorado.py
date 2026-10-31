"""El Dorado (chantier Cartes, C3; GDD 8.5, D496): no gold in the bases, a cluster of gold veins in the middle of the map, ringed
with marsh. NOT YET REGISTERED in Tools/Data/Maps.csv (2026-10-31): the recipe is prepared and measured offline, waiting for the
user's answers to CQ2 (Auto_Journey_Retours.md) and for the marsh (C3b-C3d: speed hook, zones in the level, drawing).

The map is the Steppe's (steppe.sources) with every gold vein taken out of the territories and put in the middle; the bases, the wood,
the berries and the stone are the Steppe's. The middle holds the gold of the Steppe's budget (common.mineral_mix: 10 000 per player,
15 000 on Huge) as one petal of veins per place, the same petal turned onto every place: the cluster is as far from every place as from
any other, and every place has the same share of it (the fairness of the table, 0.0 cm). The petals are packed by a greedy scan from the
centre outwards (the biggest vein first), CLUSTER_GAP between the edges of two veins.

Settings (the user's answer to CQ2b picks one; the default is the recommendation):
  HOL_ELDORADO_GOLD  "all" (default: every vein of the territories goes to the middle), "half" (every other vein of the mix, the value
                     halved), "keep_near" (the nearest veins of each base stay at home, the rest goes to the middle).
  HOL_MARSH_WIDTH    cm of marsh round the cluster (default 3000).
The marsh is a list of round zones (marsh_zones): the rule is in Script/Terrain/HOLTerrainRules.as; the level will carry them (C3c).
"""
import math
import os

from maps import common, steppe

GOLD_MODE = os.environ.get("HOL_ELDORADO_GOLD", "all")
MARSH_WIDTH = float(os.environ.get("HOL_MARSH_WIDTH", "3000"))
CLUSTER_GAP = 500.0  # cm between the edges of two veins of the cluster (a Peasant: 60 cm radius, 2 m free to walk round)
HOME_RADIUS = 13000.0  # cm: a gold vein closer than this to a place is one of its near veins (the near layer is 110 to 125 m)
RADIAL_STEP = 100.0
ANGULAR_STEPS = 48
RADIUS = {"Small": 150.0, "Medium": 220.0, "Standard": 300.0, "Big": 420.0}


def is_gold(source):
    return str(source.get_editor_property("type")).startswith("GoldVein")


def tier_of(source):
    name = str(source.get_editor_property("type"))[len("GoldVein"):]
    return name if name else "Standard"


def centre_tiers(recipe):
    """The tiers of the veins of one place's petal in the middle, biggest first."""
    tiers = sorted(common.mineral_mix(recipe)["Gold"], key=lambda tier: -common.MINERAL_AMOUNTS[tier])
    if GOLD_MODE == "half":
        return tiers[::2]
    if GOLD_MODE == "keep_near":
        for tier in common.mineral_near(recipe)["Gold"]:
            tiers.remove(tier)
    return tiers


def petal(recipe):
    """[(tier, x, y)] of one petal in the sector of place 0 (angle 0), packed from the centre outwards; the other places' petals are
    this one turned by 360 / places."""
    places = len(recipe.angles)
    sector = 2.0 * math.pi / places
    placed = []

    def free(x, y, radius):
        # a vein against its own copies in the other sectors
        for k in range(1, places):
            tx, ty = common.turn((x, y), k * sector)
            if math.hypot(x - tx, y - ty) < 2.0 * radius + CLUSTER_GAP:
                return False
        for k in range(places):
            for _, ox, oy, other in placed:
                tx, ty = common.turn((ox, oy), k * sector)
                if math.hypot(x - tx, y - ty) < radius + other + CLUSTER_GAP:
                    return False
        return True

    for tier in centre_tiers(recipe):
        radius = RADIUS[tier]
        found = None
        distance = 0.0 if not placed and places == 1 else radius + CLUSTER_GAP
        while found is None:
            # every position of the sector is tried at this distance; free() also checks the veins of the other places' petals
            for step in range(ANGULAR_STEPS):
                angle = (-0.5 + (step + 0.5) / ANGULAR_STEPS) * sector
                x, y = distance * math.cos(angle), distance * math.sin(angle)
                if free(x, y, radius):
                    found = (tier, x, y, radius)
                    break
            distance += RADIAL_STEP
            if distance > 40000.0:
                raise RuntimeError("El Dorado: no room for the cluster")
        placed.append(found)
    return placed


def cluster(recipe):
    """[(type name, x, y)] of the gold in the middle: every place's petal, the map's centre at (0, 0)."""
    places = len(recipe.angles)
    sector = 2.0 * math.pi / places
    out = []
    for k in range(places):
        for tier, x, y, _ in petal(recipe):
            tx, ty = common.turn((x, y), k * sector)
            out.append((common.mineral_type("Gold", tier), tx, ty))
    return out


def cluster_radius(recipe):
    return max(math.hypot(x, y) + RADIUS[type_name_tier(name)] for name, x, y in cluster(recipe))


def type_name_tier(name):
    tier = name[len("GoldVein"):]
    return tier if tier else "Standard"


def marsh_zones(recipe):
    """[(x, y, radius)] round zones that make a ring of marsh round the cluster, MARSH_WIDTH wide from CLUSTER_GAP beyond the veins."""
    inner = cluster_radius(recipe) + 2.0 * CLUSTER_GAP
    width = MARSH_WIDTH
    middle = inner + 0.5 * width
    count = max(8, int(math.ceil(2.0 * math.pi * middle / (0.9 * width))))
    return [(middle * math.cos(2.0 * math.pi * i / count), middle * math.sin(2.0 * math.pi * i / count), 0.5 * width) for i in range(count)]


def sources(recipe):
    """The Steppe's sources less its gold (all, half or the far ones, GOLD_MODE) plus the cluster in the middle."""
    placed, groups = steppe.sources(recipe)
    if GOLD_MODE == "keep_near":
        slots = [recipe.slot(i)[0] for i in range(len(recipe.angles))]
        placed = [s for s in placed if not is_gold(s) or any(math.dist((s.get_editor_property("location").x, s.get_editor_property("location").y), slot) < HOME_RADIUS for slot in slots)]
    else:
        placed = [s for s in placed if not is_gold(s)]
    placed += [common.placed_source(name, x, y) for name, x, y in cluster(recipe)]
    return placed, groups
