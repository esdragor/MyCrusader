"""Compares two folders of map layouts (Saved\\MapLayouts\\<Map>_<Size>.json, written by every generation: common.export_layout).

Use: before a regeneration of the maps, copy Saved\\MapLayouts to Saved\\MapLayouts_before; after it, run

    python C:\\Users\\baudo\\Documents\\HeroesOfLiberty-Haldir\\Tools\\Tests\\layout_diff.py <before_dir> <after_dir>

What must NOT move when only the glades of the groves (and the sacred sites) change: the places (slots), the relays, the altar and
every source that is not a tree (mines, berries, rocks...). What may move: the trees of the groves. Each file prints one line;
exit code 0 = no forbidden change, 1 = a forbidden change or a missing file. New keys of the after file (for example "sites") are
listed, not compared. --selftest builds two small folders and checks that the tool accepts a tree change and refuses a moved mine.
"""
import json
import os
import sys
import tempfile
from collections import Counter

TREE_TYPES = ("Tree",)


def load(path):
    with open(path, "r", encoding="utf-8") as handle:
        return json.load(handle)


def non_tree(sources):
    return Counter(tuple(s) for s in sources if s[0] not in TREE_TYPES)


def trees(sources):
    return Counter(tuple(s) for s in sources if s[0] in TREE_TYPES)


def compare_file(before, after):
    problems = []
    notes = []
    for key in ("map", "half", "slots", "relays"):
        if before.get(key) != after.get(key):
            problems.append("%s differs" % key)
    nb, na = non_tree(before["sources"]), non_tree(after["sources"])
    if nb != na:
        problems.append("%d non-tree sources removed, %d added" % (sum((nb - na).values()), sum((na - nb).values())))
    tb, ta = trees(before["sources"]), trees(after["sources"])
    notes.append("trees %d -> %d (%d removed, %d added)" % (sum(tb.values()), sum(ta.values()), sum((tb - ta).values()), sum((ta - tb).values())))
    for key in after:
        if key not in before and key not in ("map", "half", "slots", "relays", "sources"):
            value = after[key]
            notes.append("new key %s (%s)" % (key, len(value) if hasattr(value, "__len__") else value))
    for key in before:
        if key not in after:
            problems.append("key %s missing after" % key)
    return problems, notes


def compare_dirs(before_dir, after_dir):
    names = sorted(n for n in os.listdir(before_dir) if n.endswith(".json"))
    failed = 0
    for name in names:
        after_path = os.path.join(after_dir, name)
        if not os.path.exists(after_path):
            print("FAIL %s : missing after" % name)
            failed += 1
            continue
        problems, notes = compare_file(load(os.path.join(before_dir, name)), load(after_path))
        if problems:
            failed += 1
        print("%s %s : %s" % ("FAIL" if problems else "ok  ", name, "; ".join(problems + notes)))
    for name in sorted(n for n in os.listdir(after_dir) if n.endswith(".json") and n not in names):
        print("note %s : only after" % name)
    print("layout_diff : %s (%d files, %d failed)" % ("FAIL" if failed else "PASS", len(names), failed))
    return 1 if failed else 0


def selftest():
    base = {
        "map": "Test", "half": 1000.0, "slots": [[0.0, 0.0]], "relays": [[10.0, 10.0, 1200.0]],
        "sources": [["Tree", 1, 1], ["Tree", 2, 2], ["GoldVein", 50, 50]],
    }
    with tempfile.TemporaryDirectory() as root:
        dirs = {}
        for label in ("before", "tree_moved", "mine_moved", "same"):
            dirs[label] = os.path.join(root, label)
            os.makedirs(dirs[label])
        dumps = {
            "before": base,
            "same": base,
            "tree_moved": dict(base, sources=[["Tree", 1, 1], ["Tree", 3, 3], ["GoldVein", 50, 50]], sites=[[0, 0, 1200]]),
            "mine_moved": dict(base, sources=[["Tree", 1, 1], ["Tree", 2, 2], ["GoldVein", 51, 50]]),
        }
        for label, data in dumps.items():
            with open(os.path.join(dirs[label], "Test_Small.json"), "w", encoding="utf-8") as handle:
                json.dump(data, handle)
        expected = {"same": 0, "tree_moved": 0, "mine_moved": 1}
        for label, want in expected.items():
            got = compare_dirs(dirs["before"], dirs[label])
            print("selftest %s : expected %d, got %d -> %s" % (label, want, got, "ok" if got == want else "WRONG"))
            if got != want:
                return 1
    print("layout_diff selftest : PASS")
    return 0


if __name__ == "__main__":
    if len(sys.argv) == 2 and sys.argv[1] == "--selftest":
        sys.exit(selftest())
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(2)
    sys.exit(compare_dirs(sys.argv[1], sys.argv[2]))
