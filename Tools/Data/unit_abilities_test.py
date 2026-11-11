"""L4 step 4 (LegionsInProgress.md): the table of the unit abilities (Tools/Data/UnitAbilities.csv) is well formed and in step with its script copy.

Relations only, read from the real tables (no number is copied here): every ability belongs to a real unit type, names a real effect of
Effects.csv whose time is the ability's time, costs less hit points than the unit can pay between the refusal floor and the stop floor, and
recharges after it ends; the script copy Script/Units/HOLUnitAbilityTable.as is the one make_unit_abilities.py writes.

  python Tools/Data/unit_abilities_test.py      prints one line per failed check and PASS / FAIL; exit code 0 / 1
"""
import csv
import os
import subprocess
import sys

DATA = os.path.dirname(os.path.abspath(__file__))
COLUMNS = ["Name", "DisplayName", "UnitType", "Faction", "Effect", "CooldownMs", "DurationMs", "HpCostPerHit", "MinHp", "StopHp", "Description"]


def read(name):
    path = os.path.join(DATA, name)
    if not os.path.exists(path):
        return None
    with open(path, encoding="utf-8-sig", newline="") as handle:
        return list(csv.DictReader(handle))


def number(text):
    try:
        return float(text)
    except (TypeError, ValueError):
        return 0.0


def main():
    problems = []
    checks = [0]

    def check(ok, text):
        checks[0] += 1
        if not ok:
            problems.append(text)
            print("FAIL " + text)

    abilities = read("UnitAbilities.csv")
    check(abilities is not None, "Tools/Data/UnitAbilities.csv exists")
    units = {row["Name"]: row for row in read("UnitTypes.csv")}
    effects = {row["Name"]: row for row in read("Effects.csv")}
    names = []
    for row in abilities or []:
        name = row.get("Name", "?")
        names.append(name)
        check(list(row.keys()) == COLUMNS, "%s: the columns are %s" % (name, COLUMNS))
        check(bool(row.get("DisplayName")) and bool(row.get("Description")), "%s has a name and a description" % name)
        unit = units.get(row.get("UnitType"))
        check(unit is not None, "%s: UnitType %s is a row of UnitTypes.csv" % (name, row.get("UnitType")))
        effect = effects.get(row.get("Effect"))
        check(effect is not None, "%s: Effect %s is a row of Effects.csv" % (name, row.get("Effect")))
        if effect is not None:
            check(effect["Form"] == "Temporary" and number(effect["DurationMs"]) == number(row["DurationMs"]),
                  "%s: the effect is a Temporary one of the ability's own time" % name)
        check(number(row["CooldownMs"]) > number(row["DurationMs"]) > 0, "%s: the cooldown outlasts the effect, which has a time" % name)
        floor_gap = number(row["MinHp"]) - number(row["StopHp"])
        check(number(row["StopHp"]) > 0 and floor_gap > 0, "%s: the refusal floor MinHp is above the stop floor StopHp, above zero" % name)
        check(0 < number(row["HpCostPerHit"]) <= floor_gap, "%s: one blow costs hit points, not more than the gap between the floors" % name)
        if unit is not None:
            check(number(row["MinHp"]) < number(unit["MaxHealth"]), "%s: the unit can be above the refusal floor" % name)
    check(len(names) == len(set(names)), "one row per ability name")

    out = subprocess.run([sys.executable, "-I", os.path.join(DATA, "make_unit_abilities.py"), "--check"], capture_output=True, text=True)
    check(out.returncode == 0, "HOLUnitAbilityTable.as is in step with UnitAbilities.csv (%s)" % (out.stdout + out.stderr).strip()[:160])

    print("%d checks, %d problem(s)" % (checks[0], len(problems)))
    print("PASS" if not problems else "FAIL")
    return 0 if not problems else 1


if __name__ == "__main__":
    sys.exit(main())
