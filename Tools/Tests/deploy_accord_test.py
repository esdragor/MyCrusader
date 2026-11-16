"""MIG-J-Deploy (D830, D835; Palier3InProgress.md G5): the deployable engine's rule lives twice, in script (Script/Siege/HOLDeployRules.as,
tested by Script/Tests/HOLDeployRules_Test.as) and in C++ (Source/HeroesOfLiberty/Siege/HOLDeployRules.h/.cpp, read by HOLDeployProcessor,
HOLCombatProcessor, HOLUnitOrderProcessor). The 9 tests of each side have the same names and the same cases, but every case is computed from
DefaultProfile(): equal tests prove nothing if the two profiles differ. This check reads both sources as text, no engine: the named integer
constants (Proposed*, TooClose), the fields DefaultProfile() fills from them, and the members of the three enums, in order (the tests walk
the phases by index, PhaseAt).
  python Tools/Tests/deploy_accord_test.py [--root <repository>]   prints what it compared and PASS / FAIL; exit code 0 / 1
"""
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
SCRIPT = os.path.join("Script", "Siege", "HOLDeployRules.as")
HEADER = os.path.join("Source", "HeroesOfLiberty", "Siege", "HOLDeployRules.h")
SOURCE = os.path.join("Source", "HeroesOfLiberty", "Siege", "HOLDeployRules.cpp")
ENUMS = ("Phase", "Order", "Action")


def without_comments(text):
    return re.sub(r"/\*.*?\*/", "", re.sub(r"//[^\n]*", "", text), flags=re.S)


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


def constants(text):
    """{name: value} of the 'const int X = n;' (script) or 'constexpr int32 X = n;' (C++) lines."""
    return {name: int(value) for name, value in re.findall(r"\b(?:const|constexpr)\s+int(?:32)?\s+(\w+)\s*=\s*(-?\d+)\s*;", text)}


def enum_members(text, name):
    """The members of the enum, in order (None if absent)."""
    match = re.search(r"\benum\s+(?:class\s+)?%s\b[^{;]*\{" % name, text)
    if not match:
        return None
    body = braced(text, match.start())
    return [re.match(r"\s*(\w+)", item).group(1) for item in body.split(",") if item.strip()]


def profile_fields(text):
    """{field: constant name} of the 'Profile.<field> = <constant>;' lines of DefaultProfile()."""
    match = re.search(r"\bDefaultProfile\s*\(\s*\)\s*\{", text)
    if not match:
        return None
    return dict(re.findall(r"Profile\.(\w+)\s*=\s*(\w+)\s*;", braced(text, match.start())))


def read_side(rules, *, script):
    rules = without_comments(rules)
    enums = {kind: enum_members(rules, ("EHOLDeploy" if script else "E") + kind) for kind in ENUMS}
    return {"constants": constants(rules), "profile": profile_fields(rules), "enums": enums}


def problems_of(script_side, code_side):
    out = []
    for name in sorted(set(script_side["constants"]) | set(code_side["constants"])):
        in_script, in_code = script_side["constants"].get(name), code_side["constants"].get(name)
        if in_script != in_code:
            out.append("constant %s: script %s, C++ %s" % (name, in_script, in_code))
    if not script_side["profile"] or script_side["profile"] != code_side["profile"]:
        out.append("DefaultProfile: script %s, C++ %s" % (script_side["profile"], code_side["profile"]))
    for kind in ENUMS:
        if script_side["enums"][kind] is None or script_side["enums"][kind] != code_side["enums"][kind]:
            out.append("enum %s: script %s, C++ %s" % (kind, script_side["enums"][kind], code_side["enums"][kind]))
    return out


def compare(script, header, source):
    return problems_of(read_side(script, script=True), read_side(header + "\n" + source, script=False))


def main(argv):
    root = argv[argv.index("--root") + 1] if "--root" in argv else ROOT
    texts = []
    for relative in (SCRIPT, HEADER, SOURCE):
        with open(os.path.join(root, relative), encoding="utf-8") as file:
            texts.append(file.read())
    script, header, source = texts
    side = read_side(script, script=True)
    print("constants %s" % side["constants"])
    print("DefaultProfile %s" % side["profile"])
    print("enums %s" % side["enums"])

    # The check must bite: a script minimum range of 1300, a C++ profile that forgets PackMs, and two script phases swapped are each refused.
    swapped = read_side(script, script=True)
    phases = list(swapped["enums"]["Phase"] or [])
    swapped["enums"]["Phase"] = phases[1:2] + phases[0:1] + phases[2:]
    bites = {
        "script MinRange 1300": compare(re.sub(r"(ProposedMinRangeCm\s*=\s*)\d+", r"\g<1>1300", script), header, source),
        "C++ without PackMs": compare(script, header, re.sub(r"Profile\.PackMs\s*=\s*\w+\s*;", "", source)),
        "script phases swapped": problems_of(swapped, read_side(header + "\n" + source, script=False)),
    }
    not_bitten = [what for what, problems in bites.items() if not problems]
    if not_bitten:
        print("self-test: not refused: %s  <-- FAIL" % ", ".join(not_bitten))
        return 1
    problems = compare(script, header, source)
    for problem in problems:
        print(problem + "  <-- FAIL")
    print("deploy_accord_test : %s" % ("PASS" if not problems else "FAIL (%d)" % len(problems)))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
