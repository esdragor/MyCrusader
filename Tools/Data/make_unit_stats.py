"""Writes UnitStats.md (repo root) from Tools/Data/UnitTypes.csv: the stats of every unit type, readable.

Run it again whenever the CSV changes:  python Tools/Data/make_unit_stats.py
The combat constants below copy Source/HeroesOfLiberty/Simulation/HOLCombatRules.h; keep them in step.
"""
import csv
import math
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CSV_PATH = ROOT / "Tools" / "Data" / "UnitTypes.csv"
OUT_PATH = ROOT / "UnitStats.md"

# HOLCombatRules.h
RANGED_REACH = 200.0  # cm: above it, the hits are shots (an arrow in flight)
ARROW_SPEED = 3750.0  # cm/s, hol.Combat.ArrowSpeed by default (D345 trial: x1.5)
ATTACK_MOVE_RADIUS = 1600.0  # cm
MAX_ATTACKERS = 6
DYING_STEPS = 20
STEP_HZ = 20

# Trial rows, identical in play to another type: listed, left out of the matchups.
TRIAL_ROWS = {"LightInfantryNanite"}

NAMES = {
    "LightInfantry": "Infanterie légère",
    "LightInfantryNanite": "Infanterie légère (essai Nanite)",
    "Archer": "Archer",
    "Peasant": "Paysan",
    "Spearman": "Piquier",
    "ManAtArms": "Homme d'armes",
}
CATEGORIES = {
    "LightInfantry": "Infanterie légère",
    "HeavyInfantry": "Infanterie lourde",
    "Ranged": "Distance",
    "Cavalry": "Cavalerie",
    "Siege": "Siège",
    "Peasant": "Paysan",
    "GunpowderInfantry": "Infanterie à poudre",
    "Heavy": "Lourd",
}
ATTACK_TYPES = {"Melee": "Mêlée", "Ranged": "Distance", "Siege": "Siège", "Hero": "Héros"}
ARMOR_OF_TYPE = {"Melee": "MeleeArmor", "Ranged": "RangedArmor", "Siege": "SiegeArmor", "Hero": "HeroArmor"}

BUILDING_RANGED_ARMOR = 40  # Tools/Data/BuildingTypes.csv RangedArmor (D345)


def read_tile_cm():
    """TileSize (cm) of Config/DefaultGame.ini (D311, D344: 400 = 4 m); 400 if missing."""
    ini = ROOT / "Config" / "DefaultGame.ini"
    m = re.search(r"^TileSize=([0-9.]+)", ini.read_text(encoding="utf-8"), re.M) if ini.exists() else None
    return float(m.group(1)) if m else 400.0


TILE_CM = read_tile_cm()

RESOURCES = [("Food", "Nourriture"), ("Wood", "Bois"), ("Stone", "Pierre"), ("Gold", "Or")]


def parse_struct(text):
    """'(Food=30,Wood=50,Stone=0,Gold=0)' -> {'Food': 30.0, ...}"""
    return {k: float(v) for k, v in re.findall(r"(\w+)=(-?[\d.]+)", text)}


def parse_list(text):
    """'(Ranged,GunpowderInfantry)' -> ['Ranged', 'GunpowderInfantry']"""
    return re.findall(r"\w+", text)


def parse_bonuses(text):
    """'((Category=LightInfantry,Damage=5),(Category=Cavalry,Damage=17))' -> [('LightInfantry', 5.0), ...]"""
    return [(c, float(d)) for c, d in re.findall(r"Category=(\w+),\s*Damage=(-?[\d.]+)", text)]


def num(x):
    """Short number: 3.0 -> '3', 1.875 -> '1,875' (French decimal comma)."""
    s = f"{x:.3f}".rstrip("0").rstrip(".")
    return s.replace(".", ",")


def metres(cm):
    """Distance in tiles first (the unit of the table, D344), metres after: '0,29 case (1,16 m)'."""
    tiles = round(cm / TILE_CM, 2)
    return f"{num(tiles)} case{'s' if tiles >= 2 else ''} ({num(round(cm / 100.0, 2))} m)"


def speed(cm_per_s):
    tiles = round(cm_per_s / TILE_CM, 2)
    return f"{num(tiles)} case{'s' if tiles >= 2 else ''}/s ({num(round(cm_per_s / 100.0, 2))} m/s)"


def cost_text(cost):
    parts = [f"{num(cost[k])} {fr}" for k, fr in RESOURCES if cost.get(k, 0) > 0]
    return " + ".join(parts) if parts else "—"


def is_ranged(u):
    return u["AttackRange"] > RANGED_REACH


def damage(att, dfn):
    """HOLCombat::GetDamage (D345): attack + every bonus aimed at one of the target's categories - its armour of the
    attack's type, 1 at least."""
    d = att["Attack"]
    for cat, bonus in att["Bonuses"]:
        if bonus > 0 and cat in dfn["ArmorCategories"]:
            d += bonus
    d -= dfn[ARMOR_OF_TYPE[att["AttackType"]]]
    return max(d, 1)


def time_to_kill(att, dfn):
    """Seconds from the start of the attack (in reach) to the killing hit: wind-up, then one interval per hit.
    Leaves out the arrow's flight and the 50 ms steps (the game's cadence is exact on average)."""
    hits = math.ceil(dfn["MaxHealth"] / damage(att, dfn))
    windup = att["AttackInterval"] * min(max(att["AttackWindUpPercent"], 1), 100) / 100.0
    return hits, windup + (hits - 1) * att["AttackInterval"]


def load():
    units = []
    with open(CSV_PATH, encoding="utf-8") as f:
        for row in csv.DictReader(f):
            u = dict(row)
            for k in ("Radius", "Speed", "MaxHealth", "AvoidanceDistance", "BuildTime", "Population",
                      "CarryCapacity", "Attack", "AttackInterval", "AttackWindUpPercent", "AttackRange",
                      "MeleeArmor", "RangedArmor", "SiegeArmor", "HeroArmor", "AggressionRadius",
                      "BuildingAttack", "BuildingAttackInterval", "BuildingAttackRange"):
                u[k] = float(row[k])
            u["ArmorCategories"] = parse_list(row["ArmorCategories"])
            u["Bonuses"] = parse_bonuses(row["Bonuses"])
            u["Cost"] = parse_struct(row["Cost"])
            u["GatherRates"] = parse_struct(row["GatherRates"])
            units.append(u)
    return units


def table(header, rows):
    out = ["| " + " | ".join(header) + " |", "|" + "|".join(["---"] * len(header)) + "|"]
    out += ["| " + " | ".join(r) + " |" for r in rows]
    return "\n".join(out)


def main():
    units = load()
    fighters = [u for u in units if u["Name"] not in TRIAL_ROWS]
    name = lambda u: NAMES.get(u["Name"], u["DisplayName"])
    L = []
    L.append("# Stats des unités")
    L.append("")
    L.append("Produit par `python Tools/Data/make_unit_stats.py` à partir de `Tools/Data/UnitTypes.csv` (la source de "
             "l'asset `/Game/Units/UnitTypes`, D227) et des constantes de `HOLCombatRules.h`. Ne pas modifier à la main : "
             "changer le CSV, puis relancer le script. Valeurs provisoires, à équilibrer. Distances et vitesses en cases "
             f"(une case = {num(TILE_CM / 100)} m, D344), en mètres entre parenthèses.")
    L.append("")

    L.append("## Combat")
    L.append("")
    rows = []
    for u in units:
        bonus = ", ".join(f"+{num(b)} contre {CATEGORIES[c]}" for c, b in u["Bonuses"] if b > 0) or "—"
        aggro = metres(u["AggressionRadius"]) if u["AggressionRadius"] > 0 else "aucun (seulement sur ordre)"
        rows.append([
            name(u), num(u["MaxHealth"]), num(u["Attack"]), bonus,
            num(u["AttackInterval"]) + " s", num(round(60.0 / u["AttackInterval"], 1)),
            num(round(u["Attack"] / u["AttackInterval"], 2)),
            f"{num(u['AttackWindUpPercent'])} % ({num(round(u['AttackInterval'] * u['AttackWindUpPercent'] / 100, 3))} s)",
            ATTACK_TYPES[u["AttackType"]] + (" (tir)" if is_ranged(u) else ""), metres(u["AttackRange"]),
            num(u["MeleeArmor"]), num(u["RangedArmor"]), num(u["SiegeArmor"]), num(u["HeroArmor"]),
            ", ".join(CATEGORIES[c] for c in u["ArmorCategories"]), aggro,
        ])
    L.append(table(["Unité", "PV", "Attaque", "Bonus", "Cadence", "Coups / min", "Dégâts / s (sans bonus ni armure)",
                    "Élan du 1er coup", "Type d'attaque", "Portée", "Armure mêlée", "Armure distance", "Armure siège",
                    "Armure héros", "Catégories d'armure", "Rayon d'agressivité"], rows))
    L.append("")
    L.append("- Dégâts d'un coup = attaque + chaque bonus visant une des catégories de la cible (ils s'additionnent) − "
             "armure de la cible du type de l'attaque (mêlée, distance, siège ou héros), 1 au moins.")
    L.append("- Cadence : temps entre deux coups. Élan : part de la cadence qu'attend le premier coup sur une cible.")
    L.append(f"- Portée : entre les bords des deux corps. Au-delà de {metres(RANGED_REACH)}, les coups sont des tirs "
             f"(flèche à {num(ARROW_SPEED / 100)} m/s, qui touche toujours) ; l'armure retirée vient du type d'attaque.")
    L.append(f"- Rayon d'agressivité : riposte et prise de cible automatiques, de bord à bord. Sur attaque-déplacement, "
             f"une unité qui n'en a pas cherche à {metres(ATTACK_MOVE_RADIUS)}.")
    L.append(f"- Au plus {MAX_ATTACKERS} attaquants de mêlée vont sur une même cible hors de portée ; les autres en "
             f"prennent une moins entourée.")
    L.append(f"- Une unité à 0 PV reste {DYING_STEPS} pas ({num(DYING_STEPS / STEP_HZ)} s à {STEP_HZ} Hz) pour sa chute.")
    L.append("")

    L.append("## Attaque contre bâtiment")
    L.append("")
    rows = []
    for u in fighters:
        if u["BuildingAttack"] <= 0:
            rows.append([name(u), "—", "—", "—", "—", "—", "—"])
            continue
        hit = max(u["BuildingAttack"] - (BUILDING_RANGED_ARMOR if u["BuildingAttackType"] == "Ranged" else 0), 1)
        rows.append([name(u), num(u["BuildingAttack"]), ATTACK_TYPES[u["BuildingAttackType"]], num(hit),
                     num(u["BuildingAttackInterval"]) + " s", num(round(hit / u["BuildingAttackInterval"], 2)),
                     metres(u["BuildingAttackRange"])])
    L.append(table(["Unité", "Attaque", "Type", "Dégâts par coup (armure distance des bâtiments retirée)", "Cadence",
                    "Dégâts / s", "Portée (bord du corps au bâtiment)"], rows))
    L.append("")
    L.append(f"- Contre un bâtiment, l'unité se sert toujours de cette attaque (D345). Les bâtiments ont "
             f"{BUILDING_RANGED_ARMOR} d'armure distance, rien contre les autres types ; 1 dégât au moins.")
    L.append("- Cibles : sur ordre (clic droit), en attaque-déplacement quand aucune unité ennemie n'est à portée de "
             "recherche, au repos dans le rayon d'agressivité sans unité ennemie (le Paysan seulement sur ordre) ; une unité "
             "ennemie qui arrive fait lâcher le bâtiment.")
    L.append("")

    L.append("## Déplacement et corps")
    L.append("")
    rows = [[name(u), speed(u["Speed"]), metres(u["Radius"]), metres(u["AvoidanceDistance"])] for u in units]
    L.append(table(["Unité", "Vitesse", "Rayon du corps", "Distance d'évitement"], rows))
    L.append("")

    L.append("## Production et économie")
    L.append("")
    rows = []
    for u in units:
        rates = u["GatherRates"]
        gather = (", ".join(f"{fr} {num(rates[k])}/s" for k, fr in RESOURCES if rates.get(k, 0) > 0)
                  if any(v > 0 for v in rates.values()) else "—")
        carry = num(u["CarryCapacity"]) if u["CarryCapacity"] > 0 else "—"
        rows.append([name(u), cost_text(u["Cost"]), num(u["BuildTime"]) + " s", num(u["Population"]), gather, carry])
    L.append(table(["Unité", "Coût", "Temps de production", "Population", "Récolte", "Charge portée"], rows))
    L.append("")

    L.append("## Affrontements")
    L.append("")
    L.append("Ligne = l'attaquant, colonne = la cible. Chaque case : dégâts d'un coup · coups pour tuer · temps pour tuer "
             "une cible seule et immobile (élan du premier coup, puis un coup par cadence ; sans le vol de la flèche).")
    L.append("")
    rows = []
    for a in fighters:
        cells = [f"**{name(a)}**"]
        for d in fighters:
            hits, t = time_to_kill(a, d)
            cells.append(f"{num(damage(a, d))} · {hits} · {num(round(t, 1))} s")
        rows.append(cells)
    L.append(table(["Attaquant \\ Cible"] + [name(d) for d in fighters], rows))
    L.append("")
    L.append("Les lignes d'essai (" + ", ".join(NAMES.get(n, n) for n in sorted(TRIAL_ROWS)) +
             ") sont laissées de côté : elles se battent comme le type qu'elles copient.")
    L.append("")

    OUT_PATH.write_text("\n".join(L), encoding="utf-8")
    print(f"Wrote {OUT_PATH}")


if __name__ == "__main__":
    main()
