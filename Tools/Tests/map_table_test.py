"""E3-13 (Auto_Journey_File.md), the "Map.Csv" part of HOLMapRulesTests.cpp ported to python: Tools/Data/Maps.csv (read by the map scripts,
mirrored by the manager) says what the C++ table (FHOLMapRules, Source/HeroesOfLiberty/Map/HOLMapRules.cpp, read as text) says: the header, one
line per map in the table's order, the key, the display name (French source text), the recipe in lower case, and the Budget column
(standard / centre / dense / none) against bBudget, bGoldAtCentre, bDenseWood. The "Map.Rules" part waits for the script socle.
  python Tools/Tests/map_table_test.py [--csv <Maps.csv>] [--cpp <HOLMapRules.cpp>]      prints PASS / FAIL (n); exit code 0 / 1
"""
import argparse
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
HEADER = "Name,DisplayName,Recipe,Budget"
ENTRY = re.compile(r'\{\s*TEXT\("([^"]*)"\)\s*,\s*NSLOCTEXT\("HOLMaps"\s*,\s*"[^"]*"\s*,\s*"([^"]*)"\)\s*((?:,\s*(?:true|false)\s*)*)\}')


def code_table(path):
    """The table of MakeMaps(): [(key, display name, bBudget, bGoldAtCentre, bDenseWood)] in the code's order."""
    with open(path, encoding="utf-8") as handle:
        text = handle.read()
    table = []
    for key, name, flags in ENTRY.findall(text):
        values = [flag.strip() == "true" for flag in flags.split(",") if flag.strip()]
        values += [False] * (3 - len(values))
        table.append((key, name, values[0], values[1], values[2]))
    return table


def check(csv_path, cpp_path):
    """The list of the problems found (empty = the table and the file agree)."""
    problems = []
    table = code_table(cpp_path)
    if not table:
        return ["no map found in " + cpp_path]
    with open(csv_path, encoding="utf-8") as handle:
        lines = [line.strip() for line in handle.read().splitlines() if line.strip()]
    if not lines or lines[0] != HEADER:
        problems.append("header %r, expected %r" % (lines[0] if lines else "", HEADER))
    if len(lines) != 1 + len(table):
        problems.append("%d lines, expected header + %d maps" % (len(lines), len(table)))
    for index, line in enumerate(lines[1:], start=1):
        cells = line.split(",")
        if len(cells) != 4:
            problems.append("line %d has %d cells, expected 4" % (index, len(cells)))
            continue
        key, name, recipe, budget = cells
        if index > len(table):
            problems.append("line %d: %s is not in the code's table" % (index, key))
            continue
        code_key, code_name, code_budget, code_centre, code_dense = table[index - 1]
        if key.lower() != code_key.lower():
            problems.append("line %d: %s, the code's table has %s there (order)" % (index, key, code_key))
            continue
        if name != code_name:
            problems.append("%s display name %r, code %r" % (key, name, code_name))
        if not recipe or recipe != recipe.lower():
            problems.append("%s recipe %r is empty or not lower case" % (key, recipe))
        if budget not in ("standard", "centre", "dense", "none"):
            problems.append("%s budget %r is not standard, centre, dense or none" % (key, budget))
        if code_budget != (budget in ("standard", "centre", "dense")):
            problems.append("%s budget %s, code bBudget %s" % (key, budget, code_budget))
        if code_dense != (budget == "dense"):
            problems.append("%s budget %s, code bDenseWood %s" % (key, budget, code_dense))
        if code_centre != (budget == "centre"):
            problems.append("%s budget %s, code bGoldAtCentre %s" % (key, budget, code_centre))
    return problems


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--csv", default=os.path.join(ROOT, "Tools", "Data", "Maps.csv"))
    parser.add_argument("--cpp", default=os.path.join(ROOT, "Source", "HeroesOfLiberty", "Map", "HOLMapRules.cpp"))
    args = parser.parse_args()
    problems = check(args.csv, args.cpp)
    for problem in problems:
        print("  " + problem)
    print("FAIL (%d)" % len(problems) if problems else "PASS (%d maps)" % len(code_table(args.cpp)))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())