"""B2 (ExplorationXPInProgress.md, Retours/Q-exploration-xp.md): the exploration XP rule lives twice, in script (Script/Heroes/
HOLExplorationXPRules.as, tested alone) and in C++ (Source/HeroesOfLiberty/Simulation/HOLExplorationXPRules.h/.cpp, read by the server's fog).
Their numbers must agree: the struct defaults (CellsPerTranche, XPPerTranche, XPPerPlace) and every preset of HOLExplorationXP::Preset
(0 = C both, 1 = A surface only, 2 = B places only). Both sources are read as text, no engine.
  python Tools/Tests/exploration_xp_accord_test.py [--root <repository>]   prints one line per preset and PASS / FAIL; exit code 0 / 1
"""
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
FIELDS = ("CellsPerTranche", "XPPerTranche", "XPPerPlace")
SCRIPT = os.path.join("Script", "Heroes", "HOLExplorationXPRules.as")
HEADER = os.path.join("Source", "HeroesOfLiberty", "Simulation", "HOLExplorationXPRules.h")
SOURCE = os.path.join("Source", "HeroesOfLiberty", "Simulation", "HOLExplorationXPRules.cpp")


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


def defaults(text, struct_name):
    """{field: value} of the struct's '<int type> <field> = <n>;' members."""
    match = re.search(r"struct\s+%s\b" % struct_name, text)
    if not match:
        raise ValueError("no struct %s" % struct_name)
    body = braced(text, match.end())
    return {name: int(value) for name, value in re.findall(r"\bint(?:32)?\s+(\w+)\s*=\s*(-?\d+)\s*;", body) if name in FIELDS}


def presets(text, base):
    """{"0": .., "1": .., "2": ..} the settings each preset index returns (the final else gives 0 and any other index)."""
    match = re.search(r"\bPreset\s*\(\s*(?:const\s+)?int(?:32)?\s+Index\s*\)\s*\{", text)
    if not match:
        raise ValueError("no Preset(int Index) definition")
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


def read_numbers(script, header, source):
    """(script numbers, C++ numbers): {"default": .., "0": .., "1": .., "2": ..} each."""
    script_base = defaults(script, "FHOLExplorationXPSettings")
    code_base = defaults(header, "FSettings")
    script_numbers = {"default": script_base}
    script_numbers.update(presets(script, script_base))
    code_numbers = {"default": code_base}
    code_numbers.update(presets(source, code_base))
    return script_numbers, code_numbers


def problems_of(script_numbers, code_numbers):
    """What disagrees (a list of texts, empty = fine)."""
    out = []
    for key in ("default", "0", "1", "2"):
        for field in FIELDS:
            in_script, in_code = script_numbers[key].get(field), code_numbers[key].get(field)
            if in_script is None or in_code is None:
                out.append("%s %s: missing in %s" % (key, field, "the script" if in_script is None else "the C++"))
            elif in_script != in_code:
                out.append("%s %s: script %d, C++ %d" % (key, field, in_script, in_code))
    return out


def main(argv):
    root = argv[argv.index("--root") + 1] if "--root" in argv else ROOT
    texts = []
    for relative in (SCRIPT, HEADER, SOURCE):
        with open(os.path.join(root, relative), encoding="utf-8") as file:
            texts.append(file.read())
    script, header, source = texts
    script_numbers, code_numbers = read_numbers(script, header, source)
    for key in ("default", "0", "1", "2"):
        print("%s: script %s, C++ %s" % (key, [script_numbers[key].get(f) for f in FIELDS], [code_numbers[key].get(f) for f in FIELDS]))

    # The check must bite: a script preset B at 41 XP, a C++ without the B branch, and a header default tranche of 8 are each refused.
    bites = {
        "script B at 41": read_numbers(re.sub(r"(Index == 2\)\s*\{[^}]*XPPerPlace\s*=\s*)\d+", r"\g<1>41", script), header, source),
        "C++ without B": read_numbers(script, header, re.sub(r"else if \(Index == 2\)\s*\{[^}]*\}", "", source)),
        "header tranche 8": read_numbers(script, re.sub(r"CellsPerTranche\s*=\s*\d+", "CellsPerTranche = 8", header), source),
    }
    not_bitten = [what for what, (bad_script, bad_code) in bites.items() if not problems_of(bad_script, bad_code)]
    if not_bitten:
        print("self-test: not refused: %s  <-- FAIL" % ", ".join(not_bitten))
        return 1
    problems = problems_of(script_numbers, code_numbers)
    for problem in problems:
        print(problem + "  <-- FAIL")
    print("exploration_xp_accord_test : %s" % ("PASS" if not problems else "FAIL (%d)" % len(problems)))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
