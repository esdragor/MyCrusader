"""The Knight's challenge row (D875, answer A of Retours/Q-chevalier-defi-ligne; D859 n. 10; FARAMIR-1120pd).

The challenge (8 s, zone of 8 m centred on the Virtuous Knight, cool-down 45 s) lives in ONE place (D839): the row KnightChallenge of
Tools/Data/UnitAbilities.csv, Target = Self with RadiusCm 800 (an aura: the zone is centred on the caster). Script/Factions/HOLDawnValues.as
READS that row (through the generated table Script/Units/HOLUnitAbilityTable.as) instead of holding the numbers, and the three knight.challenge_*
lines of Tools/Data/ValueSheet.csv point at the CSV (Source table). The generator note "0 for Self" became "0 except an aura".

Read the way the game reads the tables; the numbers asserted are READ from the rows, nothing is copied here but the unit and the target kind.

  python -I Tools/Tests/knight_challenge_row_test.py       PASS / FAIL, exit code 0 / 1
"""
import csv
import os
import re
import subprocess
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
DATA = os.path.join(ROOT, "Tools", "Data")
FUNCTIONS = {"KnightChallengeDurationMs": "DurationMs", "RadiusCm": "RadiusCm", "CooldownMs": "CooldownMs"}
SHEET_ROWS = {"knight.challenge_duration_ms": "DurationMs", "knight.challenge_radius_cm": "RadiusCm", "knight.challenge_cooldown_ms": "CooldownMs"}


def read(name):
    with open(os.path.join(DATA, name), encoding="utf-8-sig", newline="") as handle:
        return list(csv.DictReader(handle))


def text_of(*parts):
    with open(os.path.join(ROOT, *parts), encoding="utf-8-sig") as handle:
        return handle.read()


def main():
    problems = []

    def check(ok, text):
        if not ok:
            problems.append(text)
            print("FAIL " + text)

    rows = {row["Name"]: row for row in read("UnitAbilities.csv")}
    knight = rows.get("KnightChallenge")
    check(knight is not None, "UnitAbilities.csv has a row KnightChallenge")
    if knight is not None:
        units = {row["Name"]: row for row in read("UnitTypes.csv")}
        check(knight["UnitType"] in units, "KnightChallenge: UnitType %s is a row of UnitTypes.csv" % knight["UnitType"])
        check(knight["UnitType"] == "VirtuousKnight" and knight["Faction"] == "Dawn", "KnightChallenge belongs to the Dawn's VirtuousKnight")
        check(knight["Target"] == "Self", "KnightChallenge: Target is Self (the zone is centred on the Knight)")
        check(int(knight["RadiusCm"]) > 0, "KnightChallenge: an aura has a zone radius (RadiusCm > 0)")
        check(int(knight["RangeCm"]) == 0, "KnightChallenge: nothing is thrown (RangeCm is 0)")
        check(int(knight["CooldownMs"]) > int(knight["DurationMs"]) > 0, "KnightChallenge: the cool-down outlasts the challenge")

    # The generated table is in step with the CSV and exposes the id.
    out = subprocess.run([sys.executable, "-I", os.path.join(DATA, "make_unit_abilities.py"), "--check"], capture_output=True, text=True)
    check(out.returncode == 0, "HOLUnitAbilityTable.as is in step with UnitAbilities.csv (%s)" % (out.stdout + out.stderr).strip()[:120])
    check("const int KnightChallenge" in text_of("Script", "Units", "HOLUnitAbilityTable.as"), "the generated table has an id KnightChallenge")

    # The unit abilities test accepts the aura (a Self row with a radius) : the rule is "0 except an aura", not "0 for Self".
    generator = text_of("Tools", "Data", "make_unit_abilities.py")
    check("0 for Self" not in generator, "the generator note no longer says '0 for Self' (it says '0 except an aura')")
    check("0 except an aura" in generator, "the generator note says '0 except an aura'")
    out = subprocess.run([sys.executable, "-I", os.path.join(DATA, "unit_abilities_test.py")], capture_output=True, text=True)
    check(out.returncode == 0, "unit_abilities_test.py passes with the aura row (%s)" % out.stdout.strip().splitlines()[-1:])

    # HOLDawnValues reads the row: each knight function returns the table column, not a literal.
    dawn = text_of("Script", "Factions", "HOLDawnValues.as")
    for function, column in FUNCTIONS.items():
        body = re.search(r"int %s\(\)\s*\{(.*?)\}" % function, dawn, re.S)
        check(body is not None, "HOLDawnValues.as declares %s()" % function)
        if body is not None:
            check("HOLUnitAbilityTable::Row(HOLUnitAbilityTable::KnightChallenge)." + column in body.group(1),
                  "%s() returns the %s of the row KnightChallenge" % (function, column))
            check(not re.search(r"return\s+-?[0-9]", body.group(1)), "%s() holds no number of its own (one source, D839)" % function)

    # ValueSheet.csv points at the CSV (Source table, Locator <row>:<column>), and the value the sheet reads is the row's.
    sheet = {row["Id"]: row for row in read("ValueSheet.csv")}
    for sheet_id, column in SHEET_ROWS.items():
        line = sheet.get(sheet_id)
        check(line is not None, "ValueSheet.csv has %s" % sheet_id)
        if line is not None:
            check(line["Source"] == "table" and line["File"] == "Tools/Data/UnitAbilities.csv" and line["Locator"] == "KnightChallenge:" + column,
                  "%s reads table Tools/Data/UnitAbilities.csv KnightChallenge:%s (got %s %s %s)" % (sheet_id, column, line["Source"], line["File"], line["Locator"]))

    print("PASS" if not problems else "FAIL (%d)" % len(problems))
    return 0 if not problems else 1


if __name__ == "__main__":
    sys.exit(main())
