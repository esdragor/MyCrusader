"""Tools/Data/make_unit_stats.py end to end (2026-11-09): the table of UnitTypes.csv must keep the shape the generator and the game read,
and the generator must regenerate UnitStats.md from it.

A CSV field that holds a comma (a list such as (Peasant,Profane), a struct such as (Food=1,Wood=2)) must be quoted, otherwise the row
gets one field too many and every column after it moves (make_unit_stats.py died with "could not convert string to float: 'Melee'").

  python -I Tools/Data/unit_stats_test.py [path-to-UnitTypes.csv]    shape check on that file (default: the real one), then the real run
Exit code 0 = PASS, 1 = FAIL. The end-to-end run (python make_unit_stats.py, then UnitStats.md must name every non-trial row) only runs
on the real file.
"""
import csv
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
REAL_CSV = os.path.join(HERE, "UnitTypes.csv")


def shape_problems(path):
    problems = []
    with open(path, encoding="utf-8-sig", newline="") as handle:
        rows = list(csv.reader(handle))
    header = rows[0]
    for number, row in enumerate(rows[1:], 2):
        if row and len(row) != len(header):
            problems.append(f"line {number} ({row[0]}): {len(row)} fields, header has {len(header)}")
    return problems, rows


def main():
    path = sys.argv[1] if len(sys.argv) > 1 else REAL_CSV
    problems, rows = shape_problems(path)
    for problem in problems:
        print("FAIL shape:", problem)
    print(f"shape: {len(rows) - 1} rows, {len(problems)} wrong")
    if path != REAL_CSV:
        print("FAIL" if problems else "PASS")
        return 1 if problems else 0
    done = subprocess.run([sys.executable, "-I", os.path.join(HERE, "make_unit_stats.py")], capture_output=True, text=True)
    print("make_unit_stats.py exit code", done.returncode)
    if done.returncode != 0:
        print("FAIL run:", (done.stderr.strip().splitlines() or ["?"])[-1])
        problems.append("run")
    else:
        with open(os.path.join(ROOT, "UnitStats.md"), encoding="utf-8") as handle:
            text = handle.read()
        header = rows[0]
        for row in rows[1:]:
            name = row[header.index("DisplayName")]
            if row[0] == "Zombie" and name not in text:
                print("FAIL output: UnitStats.md does not name", name)
                problems.append(name)
    print("FAIL" if problems else "PASS")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
