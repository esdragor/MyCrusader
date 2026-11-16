"""E3-05 (Auto_Journey_File.md, migration D830/D839): port of Source/HeroesOfLiberty/Tests/HOLTalentTableTests.cpp (T2, TalentsInProgress.md).

Tools/Data/Talents.csv is the Dawn Order's talent grid (GDD 13.2, D111, D195): 8 rows (levels 2 to 9) of one talent per family, Guardian,
Captain, Champion in this order (the column of the row, the family index of the script rules, HOLTalentRules.as). Checks, as in the C++ test:
  - the header is Row,Family,Id,NameFr,Target,Effect,Decision and the file has 1 + 8 * 3 lines;
  - every line has 7 cells; line k (from 0) has row 2 + k // 3 and family Guardian / Captain / Champion by k % 3;
  - the ids are all different (24), every line has a name, a target and an effect;
  - the decision is D111 for rows 2 to 6 and D195 for rows 7 to 9.

  python Tools/Tests/talent_table_test.py [--talents <Talents.csv>]    PASS / FAIL, exit code 0 / 1
(--talents lets a temporary copy of the table be checked: the red proof of the test.)
"""
import argparse
import os
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
HEADER = "Row,Family,Id,NameFr,Target,Effect,Decision"
FIRST_ROW = 2
ROW_COUNT = 8
FAMILIES = ("Guardian", "Captain", "Champion")
CELL_COUNT = 7


def run(path):
    errors = []
    if not os.path.isfile(path):
        return ["%s is not there" % path]
    with open(path, encoding="utf-8-sig") as handle:
        lines = [line.strip() for line in handle.read().splitlines() if line.strip()]

    expected = 1 + ROW_COUNT * len(FAMILIES)
    if len(lines) != expected:
        errors.append("header and %d talents: got %d lines, expected %d" % (ROW_COUNT * len(FAMILIES), len(lines), expected))
    if (lines[0] if lines else "") != HEADER:
        errors.append("header must be " + HEADER)

    ids = set()
    for index, line in enumerate(lines[1:], start=1):
        cells = line.split(",")
        if len(cells) != CELL_COUNT:
            errors.append("line %d cells: got %d, expected %d" % (index, len(cells), CELL_COUNT))
            continue
        position = index - 1
        row = int(cells[0]) if cells[0].lstrip("-").isdigit() else None
        if row != FIRST_ROW + position // len(FAMILIES):
            errors.append("line %d row: got %s, expected %d" % (index, cells[0], FIRST_ROW + position // len(FAMILIES)))
        if cells[1] != FAMILIES[position % len(FAMILIES)]:
            errors.append("line %d family: got %s, expected %s" % (index, cells[1], FAMILIES[position % len(FAMILIES)]))
        if not cells[2] or cells[2] in ids:
            errors.append("line %d id '%s' is empty or not new" % (index, cells[2]))
        ids.add(cells[2])
        for column, what in ((3, "a name"), (4, "a target"), (5, "an effect")):
            if not cells[column]:
                errors.append("line %d has no %s" % (index, what))
        # Rows 2 to 6 come from the validated grid (D111), rows 7 to 9 from D195.
        want = "D111" if row is not None and row <= 6 else "D195"
        if cells[6] != want:
            errors.append("line %d decision: got %s, expected %s" % (index, cells[6], want))
    if len(ids) != ROW_COUNT * len(FAMILIES):
        errors.append("%d different ids expected, got %d" % (ROW_COUNT * len(FAMILIES), len(ids)))
    return errors


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--talents", default=os.path.join(ROOT, "Tools", "Data", "Talents.csv"))
    args = parser.parse_args()
    errors = run(args.talents)
    for error in errors[:20]:
        print("ERREUR " + error)
    print("%s talent_table_test: %d problem(s)" % ("FAIL" if errors else "PASS", len(errors)))
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
