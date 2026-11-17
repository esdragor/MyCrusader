"""ELROND-1116p / ARWEN-1117c (Auto_Journey_File.md, "CooldownEnd / EffectEnd / EffectEnded"): the time calculations of the necrotic rage.
HOLUnitAbilityRules.h (C++, the one source) says: cool-down end = StartMs + RageCooldownMs, effect end = StartMs + RageDurationMs, the
effect is over when NowMs >= effect end OR Health <= RageStopHp. HOLDamnedWarriorTests.cpp does not call these three functions (it only
checks the CSV numbers), and the script test Test_UnitAbilityRageTimesReadLibrarySmoke (Script/Tests/HOLS1eLibraries_Test.as) carries the
expected values as literals (45000, 15000, ...). This test, with no engine and no build, reads the constants and the three bodies of the
.h, computes what they give, and requires every literal of the script test to be what the .h computes.
  python Tools/Tests/unit_ability_times_test.py [--root <repository>]   prints one line per check and PASS / FAIL; exit code 0 / 1
"""
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
HEADER = os.path.join("Source", "HeroesOfLiberty", "Simulation", "HOLUnitAbilityRules.h")
SCRIPT_TEST = os.path.join("Script", "Tests", "HOLS1eLibraries_Test.as")


def constants(text):
    """{name: int} of the 'constexpr int32 <name> = <n>;' lines."""
    return {name: int(value) for name, value in re.findall(r"constexpr\s+int32\s+(\w+)\s*=\s*(-?\d+)\s*;", text)}


def model(header):
    """(cooldown_end, effect_end, ended) computed from the .h: the bodies are matched on their exact shape, anything else is a problem."""
    values, problems = constants(header), []

    def body(name):
        match = re.search(r"%s\s*\(([^)]*)\)\s*\{\s*return\s+([^;]+);" % name, header)
        if not match:
            problems.append("%s: body not found in the .h" % name)
        return match.group(2).strip() if match else ""

    cooldown_body = body(r"GetRageCooldownEndMs")
    effect_body = body(r"GetRageEffectEndMs")
    ended_body = body(r"IsRageEffectEnded")
    cooldown_match = re.fullmatch(r"StartMs\s*\+\s*(Rage\w+)", cooldown_body)
    effect_match = re.fullmatch(r"StartMs\s*\+\s*(Rage\w+)", effect_body)
    ended_match = re.fullmatch(r"NowMs\s*>=\s*GetRageEffectEndMs\(StartMs\)\s*\|\|\s*Health\s*<=\s*static_cast<float>\((Rage\w+)\)", ended_body)
    if not cooldown_match:
        problems.append("GetRageCooldownEndMs: unexpected body '%s'" % cooldown_body)
    if not effect_match:
        problems.append("GetRageEffectEndMs: unexpected body '%s'" % effect_body)
    if not ended_match:
        problems.append("IsRageEffectEnded: unexpected body '%s'" % ended_body)
    if problems:
        return None, problems
    cooldown, duration, stop = values.get(cooldown_match.group(1)), values.get(effect_match.group(1)), values.get(ended_match.group(1))
    if None in (cooldown, duration, stop):
        return None, ["a constant used by the three bodies is not a constexpr int32 of the .h"]
    if cooldown_match.group(1) != "RageCooldownMs" or effect_match.group(1) != "RageDurationMs" or ended_match.group(1) != "RageStopHp":
        return None, ["the bodies must use RageCooldownMs, RageDurationMs and RageStopHp (got %s, %s, %s)" %
                      (cooldown_match.group(1), effect_match.group(1), ended_match.group(1))]
    return (lambda start: start + cooldown, lambda start: start + duration, lambda start, now, health: now >= start + duration or health <= stop), []


def script_cases(text):
    """The assertions of Test_UnitAbilityRageTimesReadLibrarySmoke: [(kind, expected, args)], kind in cooldown / effect / ended."""
    match = re.search(r"void\s+Test_UnitAbilityRageTimesReadLibrarySmoke\s*\([^)]*\)\s*\{(.*?)\n\}", text, re.S)
    if not match:
        return []
    block, cases = match.group(1), []
    for expected, start in re.findall(r"AssertEquals\(\s*(\d+)\s*,\s*HOLUnitAbilityRead::GetRageCooldownEndMs\(\s*(\d+)\s*\)", block):
        cases.append(("cooldown", int(expected), (int(start),)))
    for expected, start in re.findall(r"AssertEquals\(\s*(\d+)\s*,\s*HOLUnitAbilityRead::GetRageEffectEndMs\(\s*(\d+)\s*\)", block):
        cases.append(("effect", int(expected), (int(start),)))
    for verdict, start, now, health in re.findall(
            r"Assert(True|False)\(\s*HOLUnitAbilityRead::IsRageEffectEnded\(\s*(\d+)\s*,\s*(\d+)\s*,\s*([\d.]+)\s*\)", block):
        cases.append(("ended", verdict == "True", (int(start), int(now), float(health))))
    return cases


def problems_of(header, script):
    """What disagrees (a list of texts, empty = fine), and the number of cases checked."""
    functions, problems = model(header)
    cases = script_cases(script)
    if problems:
        return problems, 0
    if len(cases) < 6:
        return ["the script test has %d parseable assertions, 6 expected" % len(cases)], len(cases)
    cooldown, effect, ended = functions
    for kind, expected, args in cases:
        got = {"cooldown": cooldown, "effect": effect, "ended": ended}[kind](*args)
        if got != expected:
            problems.append("%s%s: the .h gives %s, the script test expects %s" % (kind, args, got, expected))
    return problems, len(cases)


def main(argv):
    root = argv[argv.index("--root") + 1] if "--root" in argv else ROOT
    with open(os.path.join(root, HEADER), encoding="utf-8") as file:
        header = file.read()
    with open(os.path.join(root, SCRIPT_TEST), encoding="utf-8") as file:
        script = file.read()

    problems, count = problems_of(header, script)
    if problems:
        for problem in problems:
            print(problem + "  <-- FAIL")
        print("unit_ability_times_test : FAIL (%d)" % len(problems))
        return 1
    # The check must bite: a cool-down of 41000 ms, a stop floor of 19 hp, an effect end that adds the cool-down are each refused.
    bites = {
        "RageCooldownMs 41000": header.replace("RageCooldownMs = 40000;", "RageCooldownMs = 41000;"),
        "RageStopHp 19": header.replace("RageStopHp = 20;", "RageStopHp = 19;"),
        "effect end uses the cool-down": re.sub(r"(GetRageEffectEndMs\(const int32 StartMs\)\s*\{\s*return\s+StartMs\s*\+\s*)RageDurationMs", r"\1RageCooldownMs", header),
    }
    not_bitten = [what for what, text in bites.items() if text == header or not problems_of(text, script)[0]]
    if not_bitten:
        print("self-test: not refused: %s  <-- FAIL" % ", ".join(not_bitten))
        return 1
    for problem in problems:
        print(problem + "  <-- FAIL")
    print("%d script assertions recomputed from HOLUnitAbilityRules.h" % count)
    print("unit_ability_times_test : %s" % ("PASS" if not problems else "FAIL (%d)" % len(problems)))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
