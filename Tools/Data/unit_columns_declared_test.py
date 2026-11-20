"""L18-a (SAURON-1120pa, D864 + D865): every unit-table column the Knight's passive reads is DECLARED everywhere the chain needs it.

The chain of a column: Tools/Data/UnitTypes.csv header -> a UPROPERTY of FHOLUnitTypeRow (the DataTable row) -> `Constants.<Field> = <Field>`
in FHOLUnitTypeRow::ToConstants -> a UPROPERTY of FHOLUnitTypeConstants (the Mass shared fragment the combat processor reads).
A field missing from one link is silently 0 in the game (no error): this test names it.

Also: every CSV header column is a UPROPERTY of FHOLUnitTypeRow, except the columns the importer reads another way (EXTRA_CSV_COLUMNS).

  python -I Tools/Data/unit_columns_declared_test.py [root]      Exit code 0 = PASS, 1 = FAIL.
"""
import csv
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(sys.argv[1]) if len(sys.argv) > 1 else os.path.abspath(os.path.join(HERE, "..", ".."))
HEADER = os.path.join(ROOT, "Source", "HeroesOfLiberty", "Simulation", "HOLUnitTypes.h")
CSV_PATH = os.path.join(ROOT, "Tools", "Data", "UnitTypes.csv")

# D865 (30 %, 8 m) and D864 (Nearest): the columns of the Knight's passive.
REDIRECT_COLUMNS = ["DamageRedirectPercent", "DamageRedirectRadius"]
# CSV columns that are not a UPROPERTY of the row by the same name (the row key, and columns read by other code).
EXTRA_CSV_COLUMNS = {"Name", "DisplayName", "Cost", "BuildTime", "Population", "Faction", "MinTier"}

FIELD = re.compile(r"UPROPERTY\([^\n]*\)\s*\n\s*[\w:<>,\s]+?\s+(\w+)\s*(?:=[^;]*)?;")


def struct_text(text, name):
    start = text.index("struct HEROESOFLIBERTY_API " + name)
    end = text.index("\n};", start)
    return text[start:end]


def main():
    with open(HEADER, encoding="utf-8-sig") as handle:
        text = handle.read()
    row = struct_text(text, "FHOLUnitTypeRow")
    consts = struct_text(text, "FHOLUnitTypeConstants")
    row_fields = set(FIELD.findall(row))
    const_fields = set(FIELD.findall(consts))
    with open(CSV_PATH, encoding="utf-8-sig", newline="") as handle:
        header = next(csv.reader(handle))
    to_constants = text[text.index("FHOLUnitTypeRow::ToConstants(") :]
    problems = []
    for column in header:
        if column not in row_fields and column not in EXTRA_CSV_COLUMNS:
            problems.append(f"CSV column {column} is not a UPROPERTY of FHOLUnitTypeRow")
    for field in REDIRECT_COLUMNS:
        if field not in header:
            problems.append(f"{field}: not a column of UnitTypes.csv")
        if field not in row_fields:
            problems.append(f"{field}: not a UPROPERTY of FHOLUnitTypeRow")
        if field not in const_fields:
            problems.append(f"{field}: not a UPROPERTY of FHOLUnitTypeConstants")
        if not re.search(r"Constants\.%s\s*=" % field, to_constants):
            problems.append(f"{field}: never copied by FHOLUnitTypeRow::ToConstants (the game would read 0)")
    for problem in problems:
        print("FAIL:", problem)
    print(f"columns declared: {len(header)} CSV columns, {len(row_fields)} row fields, {len(const_fields)} constants fields, {len(problems)} wrong")
    print("FAIL" if problems else "PASS")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
