"""E3-01 (Auto_Journey_File.md, migration D830/D839): port of Source/HeroesOfLiberty/Tests/HOLCampTableTests.cpp (NQ3 = C, D613).

Tools/Data/Camps.csv is the content of each neutral camp as a list of unit types. A type that Tools/Data/UnitTypes.csv does not
know must fail (a camp could not be built from it). Checks, as in the C++ test:
  1. the real files agree: every member of every camp is a unit type (and the camps file parses without problem);
  2. D613: the Brigands camp is there, made of Spearman, ManAtArms, Archer and Crossbowman, none of them Peasant, King or Hero;
  3. the check itself: the same table plus a made-up type is refused, naming the type; the known type of that camp is not blamed;
  4. the shape: a missing members cell, a duplicate name and an empty member are three problems, only the good first B is kept.

  python Tools/Tests/camp_table_test.py [--camps <Camps.csv>] [--units <UnitTypes.csv>]    PASS / FAIL, exit code 0 / 1
(--camps lets a temporary copy of the table be checked: the red proof of the test.)
"""
import argparse
import os
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
DATA = os.path.join(ROOT, "Tools", "Data")
HEADER = "Name,DisplayName,Members"


def data_lines(text):
    """The non-empty, trimmed lines of a CSV text (Windows or Unix line ends)."""
    return [line.strip() for line in text.splitlines() if line.strip()]


def parse(text):
    """(camps, problems): camps = [(name, display, [members])]; the same rules as HOLCampTable::Parse."""
    lines = data_lines(text)
    if not lines or lines[0] != HEADER:
        return [], ["the header must be " + HEADER]
    camps, problems, seen = [], [], set()
    for index, line in enumerate(lines[1:], start=2):
        cells = line.split(",")
        if len(cells) != 3 or not cells[0]:
            problems.append("line %d: 3 cells and a name expected, got '%s'" % (index, line))
            continue
        members = [member.strip() for member in cells[2].split(";")]
        if any(not member for member in members):
            problems.append("line %d: camp %s has an empty member" % (index, cells[0]))
            continue
        if cells[0] in seen:
            problems.append("line %d: camp %s is listed twice" % (index, cells[0]))
            continue
        seen.add(cells[0])
        camps.append((cells[0], cells[1], members))
    return camps, problems


def type_names(units_text):
    """The unit type names of UnitTypes.csv: the first cell of every line after the header."""
    names = set()
    for line in data_lines(units_text)[1:]:
        comma = line.find(",")
        if comma > 0:
            names.add(line[:comma])
    return names


def check(camps_text, units_text):
    """(camps, problems): the parse problems plus one line per member that UnitTypes.csv does not know."""
    camps, problems = parse(camps_text)
    known = type_names(units_text)
    for name, _display, members in camps:
        for member in members:
            if member not in known:
                problems.append("camp %s: type %s is not in UnitTypes.csv" % (name, member))
    return camps, problems


def run(camps_path, units_path):
    errors = []

    def expect(condition, message):
        if not condition:
            errors.append(message)

    for path in (camps_path, units_path):
        if not os.path.isfile(path):
            return ["%s is not there" % path]
    with open(camps_path, encoding="utf-8-sig") as handle:
        camps_text = handle.read()
    with open(units_path, encoding="utf-8-sig") as handle:
        units_text = handle.read()

    # 1. The real files agree.
    camps, problems = check(camps_text, units_text)
    errors.extend("Camps.csv: " + problem for problem in problems)

    # 2. D613: the prototype's brigands.
    brigands = next((members for name, _d, members in camps if name == "Brigands"), None)
    expect(brigands is not None, "the Brigands camp is in the table")
    if brigands is not None:
        for kind in ("Spearman", "ManAtArms", "Archer", "Crossbowman"):
            expect(kind in brigands, "the Brigands have a %s" % kind)
        for member in brigands:
            expect(member not in ("Peasant", "King", "Hero"), "the Brigands do not field a %s" % member)

    # 3. The check itself, on the same table with a made-up type (proof that an unknown type is refused and named).
    _wrong, wrong_problems = check(camps_text.rstrip() + "\nWrongCamp,Wrong camp,Spearman;Dragonling\n", units_text)
    expect(any("Dragonling" in problem for problem in wrong_problems), "a camp with a type unknown to UnitTypes.csv is refused")
    expect(not any("type Spearman" in problem for problem in wrong_problems), "the known type of that same camp is not blamed")

    # 4. The shape: one problem per bad line, only the good first B is kept.
    shaped, shape_problems = parse("Name,DisplayName,Members\nA,Camp A\nB,Camp B,Spearman\nB,Camp B again,Archer\nC,Camp C,Spearman;;Archer\n")
    expect(len(shape_problems) == 3, "one problem per bad line (missing cell, duplicate name, empty member): got %d" % len(shape_problems))
    expect(len(shaped) == 1, "only the good first B is kept: got %d" % len(shaped))
    return errors


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--camps", default=os.path.join(DATA, "Camps.csv"))
    parser.add_argument("--units", default=os.path.join(DATA, "UnitTypes.csv"))
    args = parser.parse_args()
    errors = run(args.camps, args.units)
    for error in errors:
        print("ERREUR " + error)
    print("%s camp_table_test: %d problem(s)" % ("FAIL" if errors else "PASS", len(errors)))
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
