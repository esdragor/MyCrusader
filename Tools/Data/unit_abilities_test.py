"""L4 step 4 (LegionsInProgress.md): the table of the unit abilities (Tools/Data/UnitAbilities.csv) is well formed and in step with its script copy.

Relations only, read from the real tables (no number is copied here): every ability belongs to a real unit type, names a real effect of
Effects.csv whose time is the ability's time, costs less hit points than the unit can pay between the refusal floor and the stop floor, and
recharges after it ends; the script copy Script/Units/HOLUnitAbilityTable.as is the one make_unit_abilities.py writes.

  python Tools/Data/unit_abilities_test.py      prints one line per failed check and PASS / FAIL; exit code 0 / 1
"""
import csv
import os
import re
import subprocess
import sys

DATA = os.path.dirname(os.path.abspath(__file__))
COLUMNS = ["Name", "DisplayName", "UnitType", "Faction", "Effect", "CooldownMs", "DurationMs", "HpCostPerHit", "MinHp", "StopHp", "Description", "Target", "RadiusCm", "RangeCm"]


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
        check(number(row["CooldownMs"]) > number(row["DurationMs"]) > 0, "%s: the cooldown outlasts the effect, which has a time" % name)
        check(row["Target"] in ("Self", "Ground"), "%s: Target is Self or Ground" % name)
        # The effect and hit-point checks are those of a self ability (the rage: a morale effect on the caster, blows that cost hit points).
        # An aimed ability (the Light Seal, D859 step 1c) lays a zone: its slow is simulation C++ (the lot), no morale row yet, so its Effect may be
        # empty (and then its three hit-point columns are 0); if it names an effect, that effect must be a real Temporary row of its own time.
        if row["Target"] == "Self":
            effect = effects.get(row.get("Effect"))
            check(effect is not None, "%s: Effect %s is a row of Effects.csv" % (name, row.get("Effect")))
            if effect is not None:
                check(effect["Form"] == "Temporary" and number(effect["DurationMs"]) == number(row["DurationMs"]),
                      "%s: the effect is a Temporary one of the ability's own time" % name)
            floor_gap = number(row["MinHp"]) - number(row["StopHp"])
            check(number(row["StopHp"]) > 0 and floor_gap > 0, "%s: the refusal floor MinHp is above the stop floor StopHp, above zero" % name)
            check(0 < number(row["HpCostPerHit"]) <= floor_gap, "%s: one blow costs hit points, not more than the gap between the floors" % name)
        else:
            effect = effects.get(row.get("Effect")) if row.get("Effect") else None
            check(not row.get("Effect") or effect is not None, "%s: a named Effect %s is a row of Effects.csv" % (name, row.get("Effect")))
            if effect is not None:
                check(effect["Form"] == "Temporary" and number(effect["DurationMs"]) == number(row["DurationMs"]),
                      "%s: the effect is a Temporary one of the ability's own time" % name)
            check(number(row["HpCostPerHit"]) == 0 and number(row["MinHp"]) == 0 and number(row["StopHp"]) == 0, "%s: an aimed ability has no hit-point cost" % name)
        if row["Target"] == "Ground":
            check(number(row["RadiusCm"]) > 0 and number(row["RangeCm"]) > 0, "%s: an aimed ability has a zone radius and a throw range" % name)
        else:
            check(number(row["RadiusCm"]) == 0 and number(row["RangeCm"]) == 0, "%s: a self ability has no zone and no range" % name)
        if unit is not None:
            check(number(row["MinHp"]) < number(unit["MaxHealth"]), "%s: the unit can be above the refusal floor" % name)
    check(len(names) == len(set(names)), "one row per ability name")

    out = subprocess.run([sys.executable, "-I", os.path.join(DATA, "make_unit_abilities.py"), "--check"], capture_output=True, text=True)
    check(out.returncode == 0, "HOLUnitAbilityTable.as is in step with UnitAbilities.csv (%s)" % (out.stdout + out.stderr).strip()[:160])

    # Guard (GLORFINDEL-1119n4): the generated struct keeps every field the rows assign and has a field for every CSV column. A generator edit once
    # replaced `int StopHp` by the three ground columns: the table no longer compiled and only the script run showed it.
    table = os.path.join(os.path.dirname(os.path.dirname(DATA)), "Script", "Units", "HOLUnitAbilityTable.as")
    with open(table, encoding="utf-8") as handle:
        source = handle.read()
    struct = re.search(r"struct FHOLUnitAbilityRow\s*\{(.*?)\n\t\}", source, re.S)
    check(struct is not None, "the table declares struct FHOLUnitAbilityRow")
    declared = set(re.findall(r"^\s*(?:int|float|bool|FString|FName|FText)\s+(\w+)\s*(?:=[^;]*)?;", struct.group(1), re.M)) if struct else set()
    assigned = set(re.findall(r"\bR\.(\w+)\s*=", source))
    for field in sorted(assigned - declared):
        check(False, "R.%s is assigned in Row() but FHOLUnitAbilityRow does not declare %s" % (field, field))
    check(bool(assigned) and assigned <= declared, "every field assigned in Row() is declared in the struct (%d fields)" % len(assigned))
    text_columns = {"Name", "DisplayName", "Description"}  # Name is the Key field; the two texts are the Name() / Description() functions, not struct fields
    for column in COLUMNS:
        if column not in text_columns:
            check(column in declared, "CSV column %s has a field in FHOLUnitAbilityRow" % column)
            check(column in assigned, "CSV column %s is copied into its field by Row()" % column)
    check("Key" in declared and "Key" in assigned, "the Name column is the Key field")

    print("%d checks, %d problem(s)" % (checks[0], len(problems)))
    print("PASS" if not problems else "FAIL")
    return 0 if not problems else 1


if __name__ == "__main__":
    sys.exit(main())
