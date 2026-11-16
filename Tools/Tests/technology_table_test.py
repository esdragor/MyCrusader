"""L17-03 (Auto_Journey_File.md, migration D830/D839): port of the TABLE tests of Source/HeroesOfLiberty/Tests/HOLTechnologyTableTests.cpp
(HeroesOfLiberty.Technologies.Table and HeroesOfLiberty.Technologies.Playable) to a check of Tools/Data/Technologies.csv.

The expected content below is the GDD's grid 11.1 (names, tier, building, faction), not a copy of the CSV: the test fails when a
technology of the grid is missing, wrongly tiered or put in another building. Checks, as in the C++ tests:
  1. the grid: as many rows as lines of the grid (26), each at its tier, building and faction (the Dawn's own four, Common for the rest);
  2. every row: a name to show, tier 1 or 2, hero level 0, a building of BuildingTypes.csv (or Academy, planned), a positive cost
     with no negative amount, a research time > 0, known statistics, a positive percent;
  3. Implemented = every statistic of the row is applied by code (CodedStats);
  4. the chain: a level II asks for its level I (lower tier, same building, same statistics, stronger); a row with a Requires finds it;
  5. Playable (IsPlayable = building is a type AND Implemented): exactly the 17 expected, the four Soon ones are not, and the
     building of each playable one researches in a queue of 5;
  6. the check itself: a made-up row set (a technology missing, a wrong tier, a wrong flag) is refused.

  python Tools/Tests/technology_table_test.py [--techs <Technologies.csv>] [--buildings <BuildingTypes.csv>]    PASS / FAIL, exit 0 / 1
(--techs lets a temporary copy of the table be checked: the red proof of the test.)

What stays in C++ (read by UDataTable / Mass, not a table check): HeroesOfLiberty.Technologies.Rules (HOLTechnology::Check and its
order of reasons, MakeCost, TryEnqueue / Cancel / Advance, GetBonusPercent, ScaleByPercent, GetAttackStat, GetArmorStat, FHOLTechnologyState).
"""
import argparse
import csv
import io
import os
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
DATA = os.path.join(ROOT, "Tools", "Data")

# (name, tier, building): GDD 11.1.
GRID = [
    ("MeleeAttackI", 1, "Forge"), ("MeleeAttackII", 2, "Forge"), ("RangedAttackI", 1, "Forge"), ("RangedAttackII", 2, "Forge"),
    ("InfantryArmorI", 1, "Forge"), ("InfantryArmorII", 2, "Forge"), ("CavalryArmorI", 1, "Forge"), ("CavalryArmorII", 2, "Forge"),
    ("DoubleAxesI", 1, "LumberCamp"), ("DoubleAxesII", 2, "LumberCamp"), ("MinerPicksI", 1, "MiningCamp"), ("MinerPicksII", 2, "MiningCamp"),
    ("PloughI", 1, "Farm"), ("PloughII", 2, "Farm"), ("WheelbarrowI", 1, "Farm"), ("WheelbarrowII", 2, "Farm"),
    ("ImprovedAlarmBell", 2, "MainCentre"),
    ("Masonry", 2, "Academy"), ("Veterans", 2, "Academy"), ("Logistics", 2, "Academy"),
    ("ReinforcedRam", 1, "SiegeWorkshop"), ("HeavyMunitions", 2, "SiegeWorkshop"),
    ("Liturgy", 1, "Sanctuary"), ("TemperedHalberds", 1, "Barracks"), ("BlessingOfArms", 2, "Sanctuary"), ("VowOfPoverty", 2, "Chapel"),
]
DAWN_OWN = {"Liturgy", "TemperedHalberds", "BlessingOfArms", "VowOfPoverty"}
PLANNED_BUILDINGS = {"Academy"}
CODED_STATS = {"MeleeAttack", "RangedAttack", "InfantryArmor", "CavalryArmor", "GatherWood", "GatherStone", "GatherGold", "GatherFood",
               "WorkerSpeed", "WorkerCarry", "MonkCostDiscount"}
KNOWN_STATS = CODED_STATS | {"GarrisonSpeed", "WallTowerHealth", "CoreUnitHealth", "ArmyMarchSpeed", "RamHealth", "MangonelDamage",
                             "HonourGain", "HalberdierBonusVsHeavyInfantry", "KnightAndHalberdierBonusVsProfane"}
EXPECTED_PLAYABLE = {
    "DoubleAxesI", "DoubleAxesII", "MinerPicksI", "MinerPicksII", "PloughI", "PloughII", "WheelbarrowI", "WheelbarrowII", "VowOfPoverty",
    "MeleeAttackI", "MeleeAttackII", "RangedAttackI", "RangedAttackII", "InfantryArmorI", "InfantryArmorII", "CavalryArmorI", "CavalryArmorII",
}
SOON = ("ImprovedAlarmBell", "TemperedHalberds", "ReinforcedRam", "HeavyMunitions")
HEADER = ["Name", "DisplayName", "Category", "Faction", "Building", "MinTier", "MinHeroLevel", "Cost", "ResearchTime", "Requires",
          "Stats", "Percent", "Implemented"]


def read_rows(text):
    """(rows, problems): rows = [dict] by header; problems for a wrong header, a short line or a duplicate name."""
    reader = csv.reader(io.StringIO(text))
    lines = [line for line in reader if line and any(cell.strip() for cell in line)]
    if not lines or lines[0] != HEADER:
        return [], ["the header must be " + ",".join(HEADER)]
    rows, problems, seen = [], [], set()
    for number, cells in enumerate(lines[1:], start=2):
        if len(cells) != len(HEADER):
            problems.append("line %d: %d cells expected, got %d" % (number, len(HEADER), len(cells)))
            continue
        row = dict(zip(HEADER, cells))
        if not row["Name"] or row["Name"] in seen:
            problems.append("line %d: empty or duplicate name '%s'" % (number, row["Name"]))
            continue
        seen.add(row["Name"])
        rows.append(row)
    return rows, problems


def parse_struct(cell):
    """'(Food=150,Wood=0,Stone=0,Gold=100)' -> {'Food': 150, ...}; a malformed cell gives {}."""
    result = {}
    for part in cell.strip().strip("()").split(","):
        key, _, value = part.partition("=")
        try:
            result[key.strip()] = int(value)
        except ValueError:
            return {}
    return result


def parse_list(cell):
    """'(A,B)' or '(A)' -> ['A', 'B']."""
    return [item.strip() for item in cell.strip().strip("()").split(",") if item.strip()]


def building_names(text):
    """(names, max_queue): the buildings of BuildingTypes.csv and their MaxQueue."""
    reader = csv.reader(io.StringIO(text))
    lines = [line for line in reader if line and any(cell.strip() for cell in line)]
    header = lines[0]
    queue_index = header.index("MaxQueue")
    names, queues = set(), {}
    for cells in lines[1:]:
        names.add(cells[0])
        try:
            queues[cells[0]] = int(cells[queue_index])
        except (ValueError, IndexError):
            queues[cells[0]] = -1
    return names, queues


def check(techs_text, buildings_text):
    """The list of problems of Technologies.csv against the grid 11.1 and BuildingTypes.csv ([] = good)."""
    rows, problems = read_rows(techs_text)
    if problems and not rows:
        return problems
    by_name = {row["Name"]: row for row in rows}
    buildings, queues = building_names(buildings_text)

    # 1. The grid: every line there, at its tier, building and faction; nothing else.
    if len(rows) != len(GRID):
        problems.append("as many rows as the grid 11.1 has lines: %d expected, got %d" % (len(GRID), len(rows)))
    for name, tier, building in GRID:
        row = by_name.get(name)
        if row is None:
            problems.append("%s of the grid 11.1 is not in the table" % name)
            continue
        if row["MinTier"] != str(tier):
            problems.append("%s: tier %d expected, got %s" % (name, tier, row["MinTier"]))
        if row["Building"] != building:
            problems.append("%s: building %s expected, got %s" % (name, building, row["Building"]))
        faction = "Dawn" if name in DAWN_OWN else "Common"
        if row["Faction"] != faction:
            problems.append("%s: faction %s expected, got %s" % (name, faction, row["Faction"]))

    # 2-4. Every row.
    for row in rows:
        name = row["Name"]
        if not row["DisplayName"].strip():
            problems.append("%s: has no name to show" % name)
        if row["MinTier"] not in ("1", "2"):
            problems.append("%s: tier 1 or 2 expected, got %s" % (name, row["MinTier"]))
        if row["MinHeroLevel"] != "0":
            problems.append("%s: the tier alone decides (T-Q1 = A, D583): hero level 0, got %s" % (name, row["MinHeroLevel"]))
        if row["Building"] not in buildings and row["Building"] not in PLANNED_BUILDINGS:
            problems.append("%s: building %s is neither in BuildingTypes.csv nor a planned one" % (name, row["Building"]))
        cost = parse_struct(row["Cost"])
        if sorted(cost) != ["Food", "Gold", "Stone", "Wood"]:
            problems.append("%s: Cost cell malformed: %s" % (name, row["Cost"]))
        else:
            if any(value < 0 for value in cost.values()):
                problems.append("%s: costs a negative amount" % name)
            if sum(cost.values()) <= 0:
                problems.append("%s: costs nothing" % name)
        try:
            if float(row["ResearchTime"]) <= 0:
                problems.append("%s: takes no time" % name)
        except ValueError:
            problems.append("%s: ResearchTime is not a number: %s" % (name, row["ResearchTime"]))
        stats = parse_list(row["Stats"])
        if not stats:
            problems.append("%s: changes no statistic" % name)
        for stat in stats:
            if stat not in KNOWN_STATS:
                problems.append("%s: statistic %s is not one the effects code knows" % (name, stat))
        try:
            percent = int(row["Percent"])
        except ValueError:
            percent = 0
        if percent <= 0:
            problems.append("%s: percent must be positive, got %s" % (name, row["Percent"]))
        # 3. Implemented = all statistics applied by code (D601).
        coded = bool(stats) and all(stat in CODED_STATS for stat in stats)
        if row["Implemented"] not in ("True", "False") or (row["Implemented"] == "True") != coded:
            problems.append("%s: Implemented (%s) must tell whether its statistics are applied by code (%s)" % (name, row["Implemented"], coded))
        # 4. The chain.
        if name.endswith("II") and row["Requires"] != name[:-1]:
            problems.append("%s: asks for %s, got '%s'" % (name, name[:-1], row["Requires"]))
        if row["Requires"]:
            before = by_name.get(row["Requires"])
            if before is None:
                problems.append("%s: the technology it requires (%s) is not in the table" % (name, row["Requires"]))
            else:
                if not before["MinTier"] < row["MinTier"]:
                    problems.append("%s: asks a lower tier than itself" % name)
                if before["Building"] != row["Building"]:
                    problems.append("%s: not researched in the same building as %s" % (name, row["Requires"]))
                if not (percent > int(before["Percent"] or 0)):
                    problems.append("%s: not stronger than %s" % (name, row["Requires"]))
                if parse_list(before["Stats"]) != stats:
                    problems.append("%s: not the same statistics as %s" % (name, row["Requires"]))

    # 5. Playable: building is a type AND Implemented.
    playable = {row["Name"] for row in rows if row["Building"] in buildings and row["Implemented"] == "True"}
    if len(playable) != 17:
        problems.append("17 technologies are playable, got %d" % len(playable))
    for name in sorted(EXPECTED_PLAYABLE - playable):
        problems.append("%s is expected to be playable" % name)
    for name in sorted(playable - EXPECTED_PLAYABLE):
        problems.append("%s is playable but not expected" % name)
    for name in SOON:
        row = by_name.get(name)
        if row is None:
            problems.append("%s (Soon) is not in the table" % name)
            continue
        if row["Building"] not in buildings:
            problems.append("%s: its building must exist (greyed Soon, not missing)" % name)
        if name in playable:
            problems.append("%s must not be playable: no effect coded (D601)" % name)
    for name in sorted(EXPECTED_PLAYABLE):
        row = by_name.get(name)
        if row is not None and queues.get(row["Building"]) != 5:
            problems.append("%s: %s must research in a queue of 5 (T-Q6 = A)" % (name, row["Building"]))
    return problems


def run(techs_path, buildings_path):
    errors = []
    for path in (techs_path, buildings_path):
        if not os.path.isfile(path):
            return ["%s is not there" % path]
    with open(techs_path, encoding="utf-8-sig", newline="") as handle:
        techs_text = handle.read()
    with open(buildings_path, encoding="utf-8-sig", newline="") as handle:
        buildings_text = handle.read()

    # The real files agree.
    errors.extend("Technologies.csv: " + problem for problem in check(techs_text, buildings_text))

    # 6. The check itself, on the same table made wrong (proof that each fault is refused).
    lines = techs_text.splitlines()
    without_plough = "\n".join(line for line in lines if not line.startswith("PloughII,"))
    if not any("PloughII" in problem for problem in check(without_plough, buildings_text)):
        errors.append("a table with a missing technology is refused")
    wrong_tier = techs_text.replace("PloughII,Charrue II,Economy,Common,Farm,2,", "PloughII,Charrue II,Economy,Common,Farm,1,")
    if wrong_tier == techs_text or not any("PloughII: tier" in problem for problem in check(wrong_tier, buildings_text)):
        errors.append("a table with a wrong tier is refused")
    wrong_flag = techs_text.replace("(MonkCostDiscount),20,True", "(MonkCostDiscount),20,False")
    if wrong_flag == techs_text or not any("VowOfPoverty" in problem for problem in check(wrong_flag, buildings_text)):
        errors.append("a table with a wrong Implemented flag is refused")
    return errors


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--techs", default=os.path.join(DATA, "Technologies.csv"))
    parser.add_argument("--buildings", default=os.path.join(DATA, "BuildingTypes.csv"))
    args = parser.parse_args()
    errors = run(args.techs, args.buildings)
    for error in errors:
        print("ERREUR " + error)
    print("%s technology_table_test: %d problem(s)" % ("FAIL" if errors else "PASS", len(errors)))
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
