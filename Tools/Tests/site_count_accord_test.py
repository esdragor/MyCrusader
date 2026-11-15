"""S7 (SacredSitesInProgress.md, D699): the code's count of sacred sites per map (FHOLMapInfo::NumSites of HOLMapRules.cpp/.h, which greys the box
"Sites sacres" of the new game screen at 0, before the map is loaded) must be the count the map's file lays: len(sites(recipe)) of the map's own
module (Tools/Editor/maps, common.site_count: SITE_COUNTS, else SITE_COUNT_DEFAULT), for every map of Tools/Data/Maps.csv and every size. Without
the editor (stand-in `unreal`, maps_stub.py), on the REAL chain (the maps' own sites(recipe)); the C++ table is read from its source.
  python Tools/Tests/site_count_accord_test.py      prints one line per map and PASS / FAIL; exit code 0 / 1
"""
import importlib
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import maps_stub  # noqa: E402

maps_stub.install()
from maps import common  # noqa: E402

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
MAP_DIR = os.path.join(ROOT, "Source", "HeroesOfLiberty", "Map")
NUM_SITES_FIELD = 5  # FHOLMapInfo: Key, DisplayName, bBudget, bGoldAtCentre, bDenseWood, NumSites


def split_args(text):
    """The top-level arguments of a brace initializer (commas inside parentheses and string literals do not split)."""
    args, depth, quoted, current = [], 0, False, ""
    for index, char in enumerate(text):
        if char == '"' and (index == 0 or text[index - 1] != "\\"):
            quoted = not quoted
        elif not quoted and char == "(":
            depth += 1
        elif not quoted and char == ")":
            depth -= 1
        elif not quoted and depth == 0 and char == ",":
            args.append(current.strip())
            current = ""
            continue
        current += char
    if current.strip():
        args.append(current.strip())
    return args


def code_counts(header, source):
    """{map key: NumSites} from the C++ table (MakeMaps in HOLMapRules.cpp), the default of the header for a line that does not give it."""
    default = re.search(r"int32\s+NumSites\s*=\s*(\d+)\s*;", header)
    if not default:
        raise ValueError("no 'int32 NumSites = <n>;' in HOLMapRules.h")
    counts = {}
    for line in re.findall(r"\{\s*(TEXT\(\"\w+\"\).*?)\}\s*,", source):
        args = split_args(line)
        key = re.match(r'TEXT\("(\w+)"\)', args[0]).group(1)
        counts[key] = int(args[NUM_SITES_FIELD]) if len(args) > NUM_SITES_FIELD else int(default.group(1))
    return counts


def problems_of(counts, laid):
    """What disagrees (a list of texts, empty = fine). laid = {map: {size: number of sites the map's file lays}}."""
    out = []
    for name in sorted(set(counts) ^ set(laid)):
        out.append("%s is in %s only" % (name, "the C++ table" if name in counts else "Tools/Data/Maps.csv"))
    for name in sorted(set(counts) & set(laid)):
        for size, number in laid[name].items():
            if number != counts[name]:
                out.append("%s %s: the file lays %d sites, NumSites says %d" % (name, size, number, counts[name]))
    return out


def main():
    with open(os.path.join(MAP_DIR, "HOLMapRules.h"), encoding="utf-8") as file:
        header = file.read()
    with open(os.path.join(MAP_DIR, "HOLMapRules.cpp"), encoding="utf-8") as file:
        source = file.read()
    counts = code_counts(header, source)
    sizes = common.read_sizes()
    laid = {}
    for name, recipe_name in common.read_maps().items():
        module = importlib.import_module("maps." + recipe_name)
        laid[name] = {key: len(module.sites(common.sized_recipe(name, key, places, side))) for key, (places, side) in sizes.items()}
        print("%s: code %s, file %s" % (name, counts.get(name, "-"), sorted(set(laid[name].values()))))

    # The check must bite: a table that gives Plain 3 sites, one without Plain, and a header default of 0 are each refused.
    bites = {
        "Plain at 3": code_counts(header, source.replace('{ TEXT("Plain"), NSLOCTEXT("HOLMaps", "Map_Plain", "Plaine"), true },',
                                                          '{ TEXT("Plain"), NSLOCTEXT("HOLMaps", "Map_Plain", "Plaine"), true, false, false, 3 },')),
        "Plain missing": {key: value for key, value in counts.items() if key != "Plain"},
        "default 0": code_counts(re.sub(r"int32\s+NumSites\s*=\s*\d+", "int32 NumSites = 0", header), source),
    }
    not_bitten = [what for what, bad in bites.items() if not problems_of(bad, laid)]
    if not_bitten:
        print("self-test: not refused: %s  <-- FAIL" % ", ".join(not_bitten))
        return 1
    problems = problems_of(counts, laid)
    for problem in problems:
        print(problem + "  <-- FAIL")
    print("site_count_accord_test : %s" % ("PASS" if not problems else "FAIL (%d)" % len(problems)))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
