"""B2 / D809 (GDD.md, Retours/Q-exploration-xp.md, archived in Docs/Archive/Retours_2026-11-15): the exploration XP rule lives only in C++
(Source/HeroesOfLiberty/Simulation/HOLExplorationXPRules.h/.cpp, D836 point 1: the script copy is gone). No C++ test asserts the preset
numbers (HOLExplorationXPTests.cpp reads Preset(GetPresetIndex()) itself), so this test compares them, read as text, with the numbers the
user decided: every preset of HOLExplorationXP::Preset (0 = C both, 1 = A surface only, 2 = B places only), the struct defaults (= C) and
the console variable's default (hol.HeroXP.ExplorationPreset = 0 = C). No engine.
  python Tools/Tests/exploration_xp_presets_test.py [--root <repository>]   one line per preset and PASS / FAIL; exit code 0 / 1
"""
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
FIELDS = ("CellsPerTranche", "XPPerTranche", "XPPerPlace")
HEADER = os.path.join("Source", "HeroesOfLiberty", "Simulation", "HOLExplorationXPRules.h")
SOURCE = os.path.join("Source", "HeroesOfLiberty", "Simulation", "HOLExplorationXPRules.cpp")
# The decided numbers. C (default, D809 Q1 C): tranche 3 XP + place 25 XP. A (Q-exploration-xp option A): tranche 6 XP, no place.
# B (option B): places only, 40 XP each. The tranche is a 16 x 16 fog cells square (64 m) in all three.
DECIDED = {
    "0": ("C", {"CellsPerTranche": 16, "XPPerTranche": 3, "XPPerPlace": 25}),
    "1": ("A", {"CellsPerTranche": 16, "XPPerTranche": 6, "XPPerPlace": 0}),
    "2": ("B", {"CellsPerTranche": 16, "XPPerTranche": 0, "XPPerPlace": 40}),
}


def braced(text, start):
    """The text between the brace at or after start and its matching closing brace."""
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[opening + 1:index]
    raise ValueError("unbalanced braces")


def assignments(text):
    """{field: value} of the 'Settings.<field> = <n>;' lines of a block."""
    return {name: int(value) for name, value in re.findall(r"Settings\.(\w+)\s*=\s*(-?\d+)\s*;", text)}


def defaults(text):
    """{field: value} of the FSettings struct's '<int type> <field> = <n>;' members."""
    match = re.search(r"struct\s+FSettings\b", text)
    if not match:
        raise ValueError("no struct FSettings")
    body = braced(text, match.end())
    return {name: int(value) for name, value in re.findall(r"\bint(?:32)?\s+(\w+)\s*=\s*(-?\d+)\s*;", body) if name in FIELDS}


def presets(text, base):
    """{"0": .., "1": .., "2": ..} the settings each preset index returns (the final else gives 0 and any other index)."""
    match = re.search(r"\bPreset\s*\(\s*(?:const\s+)?int(?:32)?\s+Index\s*\)\s*\{", text)
    if not match:
        raise ValueError("no Preset(int32 Index) definition")
    body = braced(text, match.start())
    branches, common = {}, body
    for found in re.finditer(r"if\s*\(\s*Index\s*==\s*(\d+)\s*\)\s*\{", body):
        block = braced(body, found.start())
        branches[found.group(1)] = block
        common = common.replace(block, "", 1)
    final_else = list(re.finditer(r"else\s*\{", body))
    if final_else:
        block = braced(body, final_else[-1].start())
        branches.setdefault("0", block)
        common = common.replace(block, "", 1)
    shared = dict(base)
    shared.update(assignments(common))
    out = {}
    for index in ("0", "1", "2"):
        settings = dict(shared)
        settings.update(assignments(branches.get(index, branches.get("0", ""))))
        out[index] = settings
    return out


def cvar_default(text):
    """The default value of hol.HeroXP.ExplorationPreset (the line after its name)."""
    match = re.search(r'TEXT\("hol\.HeroXP\.ExplorationPreset"\)\s*,\s*(-?\d+)\s*,', text)
    if not match:
        raise ValueError("no hol.HeroXP.ExplorationPreset console variable")
    return int(match.group(1))


def main(argv):
    root = ROOT
    if "--root" in argv:
        root = os.path.abspath(argv[argv.index("--root") + 1])
    failures = []
    try:
        with open(os.path.join(root, HEADER), encoding="utf-8") as handle:
            header = handle.read()
        with open(os.path.join(root, SOURCE), encoding="utf-8") as handle:
            source = handle.read()
        base = defaults(header)
        found = presets(source, base)
        cvar = cvar_default(source)
    except (OSError, ValueError) as error:
        print("FAIL exploration_xp_presets_test: %s" % error)
        return 1
    if base != DECIDED["0"][1]:
        failures.append("struct defaults %s, decided (C) %s" % (base, DECIDED["0"][1]))
    if cvar != 0:
        failures.append("hol.HeroXP.ExplorationPreset default %d, decided 0 (C, D809 Q1 C)" % cvar)
    for index, (name, expected) in DECIDED.items():
        got = {field: found[index].get(field) for field in FIELDS}
        line = "preset %s (%s): %s" % (index, name, got)
        if got != expected:
            failures.append("%s, decided %s" % (line, expected))
        else:
            print("ok " + line)
    for failure in failures:
        print("ERREUR " + failure)
    print("exploration_xp_presets_test: %s (A, B, C and the default C)" % ("FAIL" if failures else "PASS"))
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))