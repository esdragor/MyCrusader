"""E3-04 (port of the Count part of Source/HeroesOfLiberty/Tests/HOLStartingPeasantsTests.cpp, IA1.1): the 5 Peasants of every occupied slot at
the start of a real match, none in the test scenarios unless their .json asks. Without the engine, on the REAL data and the REAL source text:
  - the rule of the game: Config/DefaultGame.ini [HOLMatchSettings] StartingPeasants = 5 (and the UPROPERTY default of HOLMatchSettings.h);
  - the configuration default: FHOLMatchConfig::StartingPeasants = -1 ("the setting decides": the menu and the lobby file, which has no such field);
  - the rule's code (FHOLStartRules::GetNumStartingPeasants of HOLStartRules.cpp) and the scenarios' default (HOLTestHooks::MakeScenarioConfig, in
    HOLScenario.cpp: -1 becomes 0) are read from their source and must keep the form the numbers below assume;
  - every Tools/Scenarios/*.json (the files hol.Scenario reads): the number a scenario gets = its "startingPeasants" (field name matched without
    regard to case, as FJsonObjectConverter does) or 0 when it has none; starting_peasants.json (the trial of the rule) gets the 5 of the rule.
Not run here (stays in the C++ test until the group removal, TYPES_A_EXPOSER: FHOLMatchConfig::FromJson / HOLTestHooks::MakeScenarioConfig): the
execution of those two functions themselves; this test judges the data and the text of the rule.
  python Tools/Tests/starting_peasants_count_test.py [--ini <DefaultGame.ini>]   prints PASS / FAIL; exit code 0 / 1
"""
import glob
import json
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
SRC = os.path.join(ROOT, "Source", "HeroesOfLiberty")
INI = os.path.join(ROOT, "Config", "DefaultGame.ini")
if "--ini" in sys.argv:
    INI = sys.argv[sys.argv.index("--ini") + 1]

problems = []


def read(path):
    with open(path, encoding="utf-8-sig") as handle:
        return handle.read()


def ini_starting_peasants():
    """StartingPeasants of the [/Script/HeroesOfLiberty.HOLMatchSettings] section of DefaultGame.ini (None when absent)."""
    section = None
    for line in read(INI).splitlines():
        line = line.strip()
        if line.startswith("[") and line.endswith("]"):
            section = line[1:-1]
        elif section == "/Script/HeroesOfLiberty.HOLMatchSettings" and re.match(r"StartingPeasants\s*=", line):
            return int(line.split("=", 1)[1].strip())
    return None


def num_starting_peasants(config_value, setting):
    """FHOLStartRules::GetNumStartingPeasants, in the form its source is checked to have."""
    return config_value if config_value >= 0 else max(setting, 0)


def scenario_value(text):
    """The "startingPeasants" a scenario .json asks for (None when it does not), with the key matched without regard to case."""
    for key, value in json.loads(text).items():
        if key.lower() == "startingpeasants":
            return value
    return None


def scenario_config_value(text):
    """FHOLMatchConfig::StartingPeasants after HOLTestHooks::MakeScenarioConfig: the asked number, else 0 (a missing field is -1, made 0)."""
    asked = scenario_value(text) if text is not None else None
    value = -1 if asked is None else max(asked, -1)  # FHOLMatchConfig::Normalize: Max(StartingPeasants, -1)
    return 0 if value < 0 else value


# The rule of the game.
setting = ini_starting_peasants()
if setting != 5:
    problems.append("DefaultGame.ini StartingPeasants is %r, 5 wanted" % (setting,))
setting = 5 if setting is None else setting
if not re.search(r"int32\s+StartingPeasants\s*=\s*5\s*;", read(os.path.join(SRC, "Match", "HOLMatchSettings.h"))):
    problems.append("HOLMatchSettings.h: the default of StartingPeasants is no longer 5")

# The configuration default and the code of the rule, as the numbers below assume them.
if not re.search(r"int32\s+StartingPeasants\s*=\s*-1\s*;", read(os.path.join(SRC, "Match", "HOLMatchConfig.h"))):
    problems.append("HOLMatchConfig.h: the default of FHOLMatchConfig::StartingPeasants is no longer -1")
if not re.search(r"Config\.StartingPeasants\s*>=\s*0\s*\?\s*Config\.StartingPeasants\s*:\s*FMath::Max\(Setting,\s*0\)",
                 read(os.path.join(SRC, "Match", "HOLStartRules.cpp"))):
    problems.append("HOLStartRules.cpp: GetNumStartingPeasants is no longer 'asked number if >= 0, else the setting'")
if not re.search(r"if\s*\(Config\.StartingPeasants\s*<\s*0\)\s*\{\s*Config\.StartingPeasants\s*=\s*0\s*;",
                 read(os.path.join(SRC, "Dev", "HOLScenario.cpp"))):
    problems.append("HOLScenario.cpp: MakeScenarioConfig no longer makes an unasked number 0")

# A real match: the menu (default configuration, -1) and the lobby (a file without the field).
if num_starting_peasants(-1, setting) != 5:
    problems.append("menu: not the 5 of the rule")
lobby = '{ "version": 2, "map": "/Game/Maps/Plain.Plain", "slots": [ { "state": "Human" }, { "state": "AI" } ] }'
if scenario_value(lobby) is not None or num_starting_peasants(-1, setting) != 5:
    problems.append("lobby: not the 5 of the rule")

# Every scenario file.
files = sorted(glob.glob(os.path.join(ROOT, "Tools", "Scenarios", "*.json")))
if len(files) < 10:
    problems.append("only %d scenario .json found (10 wanted)" % len(files))
asking, silent, trial = 0, 0, False
for path in files:
    name = os.path.basename(path)
    try:
        text = read(path)
        asked = scenario_value(text)
    except (ValueError, AttributeError) as error:
        problems.append("%s: cannot be read (%s)" % (name, error))
        continue
    if asked is not None and (isinstance(asked, bool) or not isinstance(asked, int) or asked < 0):
        problems.append("%s: startingPeasants %r is not a whole number >= 0" % (name, asked))
        continue
    count = num_starting_peasants(scenario_config_value(text), setting)
    if name.lower() == "starting_peasants.json":
        trial = True
        if count != 5:
            problems.append("starting_peasants.json: %d, the 5 of the rule wanted" % count)
    elif asked is not None:
        asking += 1
        if count != asked:
            problems.append("%s: %d, the %d it asks for wanted" % (name, count, asked))
    else:
        silent += 1
        if count != 0:
            problems.append("%s: %d, no starting Peasant wanted" % (name, count))
if not trial:
    problems.append("trial scenario starting_peasants.json not found")

# A scenario without .json, and one asking 5 or 3.
if num_starting_peasants(scenario_config_value(None), setting) != 0:
    problems.append("scenario without .json: not 0")
for asked in (5, 3):
    text = '{ "map": "/Game/Maps/Plain.Plain", "startingPeasants": %d, "slots": [ { "state": "Human" } ] }' % asked
    if num_starting_peasants(scenario_config_value(text), setting) != asked:
        problems.append("scenario asking %d: not %d" % (asked, asked))

for problem in problems:
    print("FAIL: " + problem)
print("%s: %d scenarios (%d ask, %d silent), setting %d, %d problem(s)" % (
    "FAIL" if problems else "PASS", len(files), asking, silent, setting, len(problems)))
sys.exit(1 if problems else 0)
