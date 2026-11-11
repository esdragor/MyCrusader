"""L4 step 1 (LegionsInProgress.md, Q-legions-l4 = A / A): the Damned Warrior and its necrotic rage in the tables, without the editor and
without a build. RED FIRST: the unit, the ability table and the effect row do not exist yet.

No number is written here: only RELATIONS read from the real tables (the profile A of the question: more fragile than the Man-at-arms,
more damage per second, dearer than the Legions' Man-at-arms (the "Garde noire"), a heavy profane infantryman of the Barracks, tier 1).

  python Tools/Tests/legions_warrior_test.py        prints one line per check and PASS / FAIL; exit code 0 / 1

What this does NOT show (C++, next steps of L4): that the rage is applied on the real chain (damage, cadence, hit points paid per blow,
cooldown); that stays the job of HeroesOfLiberty.Combat.DamnedWarrior.Rage.
"""
import csv
import os
import re
import sys

DATA = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "Data")
RESOURCES = ["Food", "Wood", "Stone", "Gold"]


def read(name):
    path = os.path.join(DATA, name)
    if not os.path.exists(path):
        return None
    with open(path, encoding="utf-8-sig", newline="") as handle:
        return list(csv.DictReader(handle))


def parse_cost(text):
    found = {key: float(value) for key, value in re.findall(r"(\w+)=([-\d.]+)", text or "")}
    return {key: found.get(key, 0.0) for key in RESOURCES}


def allows(faction_cell, faction):
    """HOLFactionRules::Allows: empty or Common = everybody; else the faction is in the ';' list."""
    cell = (faction_cell or "").strip()
    return cell == "" or cell == "Common" or faction in [part.strip() for part in cell.split(";")]


def produces(text):
    return re.findall(r'"?([A-Za-z]\w*)"?', (text or "").replace('""', '"'))


def categories(text):
    return [part.strip() for part in re.findall(r"[A-Za-z]\w*", text or "")]


def number(text):
    try:
        return float(text)
    except (TypeError, ValueError):
        return 0.0


def main():
    units = {row["Name"]: row for row in read("UnitTypes.csv")}
    buildings = {row["Name"]: row for row in read("BuildingTypes.csv")}
    overrides = read("FactionOverrides.csv")
    abilities = read("UnitAbilities.csv")
    effects = read("Effects.csv")
    problems = []
    checks = [0]

    def check(ok, text):
        checks[0] += 1
        if not ok:
            problems.append(text)
            print("FAIL " + text)

    # 1. The unit: the Legions' heavy profane infantryman of the Barracks.
    warrior = units.get("DamnedWarrior")
    check(warrior is not None, "DamnedWarrior is a row of UnitTypes.csv")
    maa = units["ManAtArms"]
    if warrior is not None:
        check(allows(warrior["Faction"], "Legions") and not allows(warrior["Faction"], "Dawn"), "the Damned Warrior is the Legions' and not the Dawn's")
        cats = categories(warrior["ArmorCategories"])
        for wanted in ("Profane", "HeavyInfantry", "Heavy"):
            check(wanted in cats, "the Damned Warrior is %s (ArmorCategories %s)" % (wanted, cats))
        check(number(warrior["MaxHealth"]) < number(maa["MaxHealth"]), "profile A: the Damned Warrior is more fragile than the Man-at-arms")
        dps = number(warrior["Attack"]) / max(number(warrior["AttackInterval"]), 0.001)
        maa_dps = number(maa["Attack"]) / max(number(maa["AttackInterval"]), 0.001)
        check(dps > maa_dps, "profile A: the Damned Warrior hits harder per second than the Man-at-arms (%.2f vs %.2f)" % (dps, maa_dps))
        check(int(number(warrior["MinTier"])) == 1, "the Damned Warrior opens at tier 1")
        check(number(warrior["Population"]) == 1, "the Damned Warrior takes one population")
        # dearer than the Legions' Man-at-arms (the "Garde noire": the override of ManAtArms' Cost for the Legions)
        black_guard = [r for r in overrides if r["Faction"] == "Legions" and r["Type"] == "ManAtArms" and r["Field"] == "Cost"]
        check(len(black_guard) == 1, "the Legions' Man-at-arms (Garde noire) has an own cost to compare with")
        if black_guard:
            mine = sum(parse_cost(warrior["Cost"]).values())
            theirs = sum(parse_cost(black_guard[0]["Value"]).values())
            check(mine > theirs, "the Damned Warrior costs more than the Legions' Garde noire (%s vs %s)" % (mine, theirs))

    # 2. The Barracks: the Legions' one offers it, the Dawn's does not (HOLFactionRules::FilterProduces).
    barracks = buildings["Barracks"]
    grid = produces(barracks["Produces"])
    check("DamnedWarrior" in grid, "the Barracks' Produces holds DamnedWarrior")
    if warrior is not None:
        check("DamnedWarrior" in [u for u in grid if u in units and allows(units[u]["Faction"], "Legions")], "the Legions' Barracks offers the Damned Warrior")
        check("DamnedWarrior" not in [u for u in grid if u in units and allows(units[u]["Faction"], "Dawn")], "the Dawn's Barracks does not offer the Damned Warrior")

    # 3. The ability: a row of UnitAbilities.csv (the unit abilities frame, step 4), every number above zero.
    check(abilities is not None, "Tools/Data/UnitAbilities.csv exists")
    rage = [r for r in (abilities or []) if r.get("Name") == "NecroticRage"]
    check(len(rage) == 1, "UnitAbilities.csv has one NecroticRage row")
    if rage:
        row = rage[0]
        check(row.get("UnitType") == "DamnedWarrior", "NecroticRage belongs to the DamnedWarrior")
        for column in ("CooldownMs", "DurationMs", "HpCostPerHit", "MinHp", "StopHp"):
            check(number(row.get(column)) > 0, "NecroticRage: %s is above zero" % column)
        check(number(row.get("CooldownMs")) > number(row.get("DurationMs")), "NecroticRage: the cooldown outlasts the rage")

    # 4. The effect read by the combat: more damage dealt, faster cadence.
    effect = [r for r in (effects or []) if r.get("Name") == "NecroticRage"]
    check(len(effect) == 1, "Effects.csv has one NecroticRage row")
    if effect:
        check(number(effect[0].get("Dealt")) > 0, "NecroticRage: damage dealt above zero")
        check(number(effect[0].get("Rate")) > 0, "NecroticRage: cadence above zero")

    print("%d checks, %d problem(s)" % (checks[0], len(problems)))
    print("PASS" if not problems else "FAIL")
    return 0 if not problems else 1


if __name__ == "__main__":
    sys.exit(main())
