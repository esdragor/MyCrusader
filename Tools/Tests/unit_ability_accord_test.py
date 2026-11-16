"""MIG-J-UnitAbility (D830/D835, 2026-11-16): the necrotic rage lives twice. The script reads it from the generated tables
(Script/Units/HOLUnitAbilityTable.as, row NecroticRage, from Tools/Data/UnitAbilities.csv; Script/Morale/HOLMoraleTable.as, row
NecroticRage, from Tools/Data/Effects.csv); the combat processor (Mass, it cannot call the script) reads the constants of
Source/HeroesOfLiberty/Simulation/HOLUnitAbilityRules.h, a copy ("change both places"). Their numbers must agree, field by field.
Both sides are read as text, no engine, no build.
  python Tools/Tests/unit_ability_accord_test.py [--root <repository>]   prints one line per field and PASS / FAIL; exit code 0 / 1
"""
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
ABILITY_TABLE = os.path.join("Script", "Units", "HOLUnitAbilityTable.as")
MORALE_TABLE = os.path.join("Script", "Morale", "HOLMoraleTable.as")
HEADER = os.path.join("Source", "HeroesOfLiberty", "Simulation", "HOLUnitAbilityRules.h")

# C++ constant (HOLUnitAbility::Rage<name>) -> (script table, field of the row NecroticRage).
PAIRS = (
    ("DurationMs", "ability", "DurationMs"),
    ("CooldownMs", "ability", "CooldownMs"),
    ("HpCostPerHit", "ability", "HpCostPerHit"),
    ("MinHp", "ability", "MinHp"),
    ("StopHp", "ability", "StopHp"),
    ("UnitType", "ability", "UnitType"),
    ("DamagePercent", "morale", "Dealt"),
    ("RatePercent", "morale", "Rate"),
    ("DurationMs", "morale", "DurationMs"),
)


def script_row(text, key):
    """{field: value} of the 'R.<field> = <value>;' lines of the generated row whose R.Key is key (values as text, quotes removed)."""
    match = re.search(r'R\.Key\s*=\s*"%s"\s*;' % re.escape(key), text)
    if not match:
        return {}
    end = text.find("return R;", match.end())
    block = text[match.start():end if end >= 0 else len(text)]
    return {name: value.strip().strip('"') for name, value in re.findall(r"R\.(\w+)\s*=\s*([^;]+);", block)}


def code_constants(text):
    """{name: value} of the HOLUnitAbility::Rage<name> constants (numbers as text) and of RageUnitType."""
    found = {name: value for name, value in re.findall(r"constexpr\s+int32\s+Rage(\w+)\s*=\s*(-?\d+)\s*;", text)}
    unit = re.search(r'RageUnitType\s*\(\s*TEXT\(\s*"(\w+)"\s*\)\s*\)', text)
    if unit:
        found["UnitType"] = unit.group(1)
    return found


def problems_of(ability, morale, code):
    """What disagrees (a list of texts, empty = fine)."""
    tables = {"ability": ability, "morale": morale}
    out = []
    for constant, table, field in PAIRS:
        in_script, in_code = tables[table].get(field), code.get(constant)
        if in_script is None or in_code is None:
            out.append("%s.%s / Rage%s: missing in %s" % (table, field, constant, "the script" if in_script is None else "the C++"))
        elif in_script != in_code:
            out.append("%s.%s / Rage%s: script %s, C++ %s" % (table, field, constant, in_script, in_code))
    return out


def main(argv):
    root = argv[argv.index("--root") + 1] if "--root" in argv else ROOT
    texts = []
    for relative in (ABILITY_TABLE, MORALE_TABLE, HEADER):
        with open(os.path.join(root, relative), encoding="utf-8") as file:
            texts.append(file.read())
    ability_text, morale_text, header = texts
    ability, morale, code = script_row(ability_text, "NecroticRage"), script_row(morale_text, "NecroticRage"), code_constants(header)
    tables = {"ability": ability, "morale": morale}
    for constant, table, field in PAIRS:
        print("%s.%s = %s ; C++ Rage%s = %s" % (table, field, tables[table].get(field), constant, code.get(constant)))

    # The check must bite: a script StopHp of 21, a C++ without the rate, a morale damage of 41 are each refused.
    bites = {
        "script StopHp 21": (script_row(re.sub(r"R\.StopHp\s*=\s*\d+", "R.StopHp = 21", ability_text), "NecroticRage"), morale, code),
        "C++ without the rate": (ability, morale, code_constants(re.sub(r"constexpr\s+int32\s+RageRatePercent[^;]*;", "", header))),
        "morale Dealt 41": (ability, script_row(morale_text.replace("R.Dealt = 40;", "R.Dealt = 41;"), "NecroticRage") if "R.Dealt = 40;" in morale_text
                            else dict(morale, Dealt="41"), code),
    }
    not_bitten = [what for what, args in bites.items() if not problems_of(*args)]
    if not_bitten:
        print("self-test: not refused: %s  <-- FAIL" % ", ".join(not_bitten))
        return 1
    problems = problems_of(ability, morale, code)
    for problem in problems:
        print(problem + "  <-- FAIL")
    print("unit_ability_accord_test : %s" % ("PASS" if not problems else "FAIL (%d)" % len(problems)))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
