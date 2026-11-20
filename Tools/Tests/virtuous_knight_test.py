"""The Dawn's Virtuous Knight row (D864, D865; SAURON-1120pb, answer A of Retours/Q-chevalier-ligne): a heavy infantryman of the Dawn's Barracks
(placeholder stats copied from the Man-at-arms), tier 1, who redirects 30 % of the damage taken by allies within 800 cm (8 m).

Read the way the game reads the tables: Tools/Data/UnitTypes.csv (the row, its Faction and MinTier keys, the two redirect columns) and
Tools/Data/BuildingTypes.csv (the Barracks' Produces list; the Dawn's grid is the Produces entries whose Faction key allows the Dawn,
HOLFactionRules::Allows). The values asserted are READ from the rows, the Man-at-arms being the reference the knight is copied from.

  python -I Tools/Tests/virtuous_knight_test.py       PASS / FAIL, exit code 0 / 1
"""
import csv
import os
import sys

DATA = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "Data")
COPIED_FROM_MAN_AT_ARMS = ["MaxHealth", "Speed", "Radius", "Attack", "AttackInterval", "AttackRange", "MeleeArmor", "RangedArmor", "SiegeArmor",
                           "HeroArmor", "Cost", "BuildTime", "Population", "ArmorCategories", "MinTier", "SightRadius", "Faction"]


def read(name):
    with open(os.path.join(DATA, name), encoding="utf-8-sig", newline="") as handle:
        return list(csv.DictReader(handle))


def produces(text):
    return [part.strip().strip('"') for part in text.strip().strip("()").replace('""', '"').split(",") if part.strip()]


def allows(faction_key, faction):
    keys = [key.strip() for key in faction_key.split(";")]
    return "Common" in keys or faction in keys


def main():
    problems = []
    checks = [0]

    def check(condition, message):
        checks[0] += 1
        if not condition:
            problems.append(message)

    units = {row["Name"]: row for row in read("UnitTypes.csv")}
    buildings = {row["Name"]: row for row in read("BuildingTypes.csv")}
    knight = units.get("VirtuousKnight")
    check(knight is not None, "UnitTypes.csv has a VirtuousKnight row")
    if knight is not None:
        check(int(float(knight["DamageRedirectPercent"])) == 30, "the knight redirects 30 percent, got %s" % knight["DamageRedirectPercent"])
        check(int(float(knight["DamageRedirectRadius"])) == 800, "the knight's redirect radius is 800 cm, got %s" % knight["DamageRedirectRadius"])
        check(allows(knight["Faction"], "Dawn") and not allows(knight["Faction"], "Legions"), "the knight is the Dawn's alone, got %s" % knight["Faction"])
        check(knight["MinTier"] == "1", "the knight opens at tier 1, got %s" % knight["MinTier"])
        reference = units.get("ManAtArms")
        check(reference is not None, "the Man-at-arms row (the stats reference) is there")
        if reference is not None:
            for column in COPIED_FROM_MAN_AT_ARMS:
                if column == "Faction":
                    continue
                check(knight[column] == reference[column], "placeholder stat %s copied from the Man-at-arms: %s vs %s" % (column, knight[column], reference[column]))
        others = [name for name, row in units.items() if name != "VirtuousKnight" and (int(float(row["DamageRedirectPercent"] or 0)) != 0)]
        check(others == [], "no other unit redirects damage, got %s" % others)
    barracks = buildings.get("Barracks")
    check(barracks is not None, "BuildingTypes.csv has the Barracks")
    if barracks is not None:
        grid = produces(barracks["Produces"])
        check("VirtuousKnight" in grid, "the Barracks' Produces holds the VirtuousKnight, got %s" % grid)
        dawn = [u for u in grid if u in units and allows(units[u]["Faction"], "Dawn")]
        legions = [u for u in grid if u in units and allows(units[u]["Faction"], "Legions")]
        check(dawn == ["Halberdier", "ManAtArms", "VirtuousKnight"], "the Dawn's Barracks grid is [Halberdier, ManAtArms, VirtuousKnight], got %s" % dawn)
        check("VirtuousKnight" not in legions, "the Legions' Barracks grid does not offer the knight, got %s" % legions)

    print("%d checks, %d problem(s)" % (checks[0], len(problems)))
    for problem in problems:
        print("FAIL: " + problem)
    print("PASS" if not problems else "FAIL")
    return 0 if not problems else 1


if __name__ == "__main__":
    sys.exit(main())
