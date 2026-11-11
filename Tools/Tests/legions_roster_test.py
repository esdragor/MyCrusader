"""L2 (LegionsInProgress.md, LQ-1 / LQ-2, D677): the Legions' roster in the tables, without the editor and without a build.

The Legions play the common roster (Spearman, ManAtArms, Archer, Crossbowman, LightCavalry, Ram, Mangonel) in reskin, with an own cost
per unit that is always lower than the Dawn's (LQ-1, D66), an own displayed name, and without the Dawn-only things (Monk, Chapel, Sanctuary,
Peasant, Halberdier). Read here the way the game reads them: Tools/Data/FactionOverrides.csv (Name, Faction, Type, Field, Value; Field = a
column of UnitTypes.csv, Value = its text, HOLFactionRules::ParseOverrides) over the rows of UnitTypes.csv / BuildingTypes.csv, whose Faction
column ("Common", or keys separated by ";") filters who owns a type (HOLFactionRules::Allows). The Barracks grid is its Produces list
filtered by that column (HOLFactionRules::FilterProduces).

  python Tools/Tests/legions_roster_test.py        prints one line per check and PASS / FAIL; exit code 0 / 1

What this does NOT show (C++, an open line of Auto_Journey_File.md): that the surcharges are applied at the birth of a unit and that the
production pays them; that stays the job of HeroesOfLiberty.Match.Factions.RealChain on the real chain.
"""
import csv
import os
import re
import sys

DATA = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "Data")
ROSTER = ["Spearman", "ManAtArms", "Archer", "Crossbowman", "LightCavalry", "Ram", "Mangonel"]
DAWN_ONLY_UNITS = ["Peasant", "Monk", "Halberdier", "Arquebusier"]
DAWN_ONLY_BUILDINGS = ["Chapel", "Sanctuary"]
LEGIONS_HOUSES = {"Barracks": ["Spearman", "ManAtArms", "DamnedWarrior"], "ArcheryRange": ["Archer", "Crossbowman"], "Stable": ["LightCavalry"],
                  "SiegeWorkshop": ["Ram", "Mangonel"]}
RESOURCES = ["Food", "Wood", "Stone", "Gold"]


def read(name):
    with open(os.path.join(DATA, name), encoding="utf-8-sig", newline="") as handle:
        return list(csv.DictReader(handle))


def parse_cost(text):
    """'(Food=60,Wood=20,Stone=0,Gold=0)' -> {'Food': 60.0, ...}; a missing resource is 0."""
    found = {key: float(value) for key, value in re.findall(r"(\w+)=([-\d.]+)", text)}
    return {key: found.get(key, 0.0) for key in RESOURCES}


def allows(faction_cell, faction):
    """HOLFactionRules::Allows: empty or Common = everybody; else the faction is in the ';' list."""
    cell = (faction_cell or "").strip()
    return cell == "" or cell == "Common" or faction in [part.strip() for part in cell.split(";")]


def produces(text):
    """'(""Spearman"",""ManAtArms"")' (the CSV cell, once unquoted) -> ['Spearman', 'ManAtArms']."""
    return re.findall(r'"?([A-Za-z]\w*)"?', text.replace('""', '"'))


def main():
    units = {row["Name"]: row for row in read("UnitTypes.csv")}
    buildings = {row["Name"]: row for row in read("BuildingTypes.csv")}
    overrides = read("FactionOverrides.csv")
    roles = read("Factions.csv")
    problems = []
    checks = [0]

    def check(ok, text):
        checks[0] += 1
        if not ok:
            problems.append(text)
            print("FAIL " + text)

    legions = {}
    for row in overrides:
        if row["Faction"] == "Legions":
            legions.setdefault(row["Type"], {})[row["Field"]] = row
    seen = set()
    for row in overrides:
        key = (row["Faction"], row["Type"], row["Field"])
        check(key not in seen, "override repeated: %s" % (key,))
        seen.add(key)
        table = units if row["Type"] in units else buildings
        check(row["Type"] in table, "override %s names the unknown type %s" % (row["Name"], row["Type"]))
        if row["Type"] in table:
            check(row["Field"] in table[row["Type"]], "override %s: %s is not a column of the type table" % (row["Name"], row["Field"]))

    # 1. Each unit of the roster: an own name, an own cost, lower than the Dawn's (the Dawn's is the table's own row).
    for unit in ROSTER:
        check(unit in units, "%s is in UnitTypes.csv" % unit)
        if unit not in units:
            continue
        own = legions.get(unit, {})
        name = own.get("DisplayName")
        check(name is not None and name["Value"].strip() != "", "%s: the Legions have an own DisplayName" % unit)
        if name is not None:
            check(name["Value"] != units[unit]["DisplayName"], "%s: the Legions' name differs from the Dawn's (%s)" % (unit, units[unit]["DisplayName"]))
        cost = own.get("Cost")
        check(cost is not None, "%s: the Legions have an own Cost" % unit)
        if cost is not None:
            mine = parse_cost(cost["Value"])
            theirs = parse_cost(units[unit]["Cost"])
            check(all(mine[r] <= theirs[r] for r in RESOURCES), "%s: no resource dearer than the Dawn's (%s vs %s)" % (unit, mine, theirs))
            check(sum(mine.values()) < sum(theirs.values()), "%s: the Legions' cost is lower than the Dawn's in total (%s vs %s)" % (unit, mine, theirs))
            check(sum(mine.values()) > 0, "%s: the Legions' cost is not free (only the Zombie is)" % unit)
        build_time = own.get("BuildTime")
        if build_time is not None:
            check(float(build_time["Value"]) <= float(units[unit]["BuildTime"]), "%s: the Legions' BuildTime is not longer than the Dawn's" % unit)
        check(allows(units[unit]["Faction"], "Legions"), "%s is open to the Legions (Faction column)" % unit)
        check(allows(units[unit]["Faction"], "Dawn"), "%s stays open to the Dawn" % unit)

    # 2. The Dawn-only things are not the Legions'; the Zombie is theirs and not the Dawn's.
    for unit in DAWN_ONLY_UNITS:
        if unit in units:
            check(not allows(units[unit]["Faction"], "Legions"), "%s is not open to the Legions" % unit)
    for building in DAWN_ONLY_BUILDINGS:
        if building in buildings:
            check(not allows(buildings[building]["Faction"], "Legions"), "%s is not open to the Legions" % building)
    check("Zombie" in units and allows(units["Zombie"]["Faction"], "Legions") and not allows(units["Zombie"]["Faction"], "Dawn"),
          "the Zombie is the Legions' and not the Dawn's")
    check(any(r["Faction"] == "Legions" and r["Role"] == "Worker" and r["Type"] == "Zombie" for r in roles), "the Legions' Worker is the Zombie")

    # 3. The grids of the military buildings, as the Legions see them: the roster's units, never a Dawn-only one.
    for house, expected in LEGIONS_HOUSES.items():
        check(house in buildings, "%s is in BuildingTypes.csv" % house)
        if house not in buildings:
            continue
        check(allows(buildings[house]["Faction"], "Legions"), "%s is open to the Legions" % house)
        grid = [u for u in produces(buildings[house]["Produces"]) if u in units and allows(units[u]["Faction"], "Legions")]
        check(grid == expected, "%s grid for the Legions is %s, got %s" % (house, expected, grid))
        check(not any(u in DAWN_ONLY_UNITS for u in grid), "%s grid for the Legions holds no Dawn-only unit" % house)
    centre = buildings.get("MainCentre")
    if centre is not None:
        grid = [u for u in produces(centre["Produces"]) if u in units and allows(units[u]["Faction"], "Legions")]
        check(grid == ["Zombie"], "the main centre's grid for the Legions is the Zombie alone, got %s" % grid)

    print("%d checks, %d problem(s)" % (checks[0], len(problems)))
    print("PASS" if not problems else "FAIL")
    return 0 if not problems else 1


if __name__ == "__main__":
    sys.exit(main())
