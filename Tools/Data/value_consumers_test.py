"""Test of Tools/Data/value_consumers.py (the "orphan values" check, EtapeDefini / Auto_Journey_File point 4).

Red first: a mini repository is built in a temporary directory with a value that NOTHING reads (a constant defined once and never used, a
table column no code names) next to values that ARE read. The tool must flag exactly the dead ones, from the real registry format
(Tools/Data/ValueSheet.csv columns), and `--check` must fail on a dead value that is not in the known list and pass when it is.

    python -I Tools/Data/value_consumers_test.py        exit 0 = green, the last line says how many cases passed
"""
import importlib.util
import io
import sys
import tempfile
from contextlib import redirect_stdout
from pathlib import Path

HERE = Path(__file__).resolve().parent
COLUMNS = ["Id", "Label", "Group", "Question", "Source", "File", "Locator", "State", "Decision", "GddRef", "LastSeen", "Note"]


def load(name):
    spec = importlib.util.spec_from_file_location(name, HERE / (name + ".py"))
    mod = importlib.util.module_from_spec(spec)
    sys.modules[name] = mod
    spec.loader.exec_module(mod)
    return mod


def write(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8", newline="\n")


def build_repo(root):
    write(root / "Source" / "Game" / "Rules.h",
          "namespace R {\n"
          "constexpr int UsedCap = 10; // provisoire\n"
          "constexpr int DeadCap = 20; // provisoire\n"
          "constexpr float UsedRatio = 0.5f; // provisoire\n"
          "}\n")
    write(root / "Source" / "Game" / "User.cpp", "int Cap() { return R::UsedCap + 1; }\nfloat Ratio() { return R::UsedRatio; }\n")
    write(root / "Script" / "Uses.as", "int Twice() { return 2; }\n")
    write(root / "Source" / "Game" / "Loader.cpp", 'void Load() { Read("Hp"); }\n')
    write(root / "Tools" / "Data" / "Units.csv", "Id,Hp,Ghost\nPeasant,10,3\n")
    rows = [
        ["used", "Used cap", "G", "1", "code", "Source/Game/Rules.h", r"UsedCap\s*=\s*(\d+)", "provisoire", "", "", "", ""],
        ["dead", "Dead cap", "G", "1", "code", "Source/Game/Rules.h", r"DeadCap\s*=\s*(\d+)", "provisoire", "", "", "", ""],
        ["used.ratio", "Used ratio", "G", "1", "code", "Source/Game/Rules.h", r"UsedRatio\s*=\s*([0-9.]+)f", "provisoire", "", "", "", ""],
        ["tbl.hp", "Hp", "G", "1", "table", "Tools/Data/Units.csv", "Peasant:Hp", "provisoire", "", "", "", ""],
        ["tbl.ghost", "Ghost", "G", "1", "table", "Tools/Data/Units.csv", "Peasant:Ghost", "provisoire", "", "", "", ""],
        ["tbl.size", "Size", "G", "1", "table", "Tools/Data/Units.csv", "*", "provisoire", "", "", "", ""],
        ["later", "Not coded", "G", "1", "gdd", "", "", "pas codé", "", "", "", ""],
    ]
    out = io.StringIO()
    import csv
    w = csv.writer(out, lineterminator="\n")
    w.writerow(COLUMNS)
    w.writerows(rows)
    write(root / "Tools" / "Data" / "ValueSheet.csv", out.getvalue())
    return root / "Tools" / "Data" / "ValueSheet.csv"


def main():
    vc = load("value_consumers")
    passed, failed = 0, []

    def check(name, ok):
        nonlocal passed
        if ok:
            passed += 1
        else:
            failed.append(name)

    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        registry = build_repo(root)
        dead = {d.id: d for d in vc.find_dead(root, registry)}
        check("dead constant is flagged", "dead" in dead)
        check("dead table column is flagged", "tbl.ghost" in dead)
        check("used constants are not flagged", "used" not in dead and "used.ratio" not in dead)
        check("read table column is not flagged", "tbl.hp" not in dead)
        check("whole-table and gdd rows are not flagged", "tbl.size" not in dead and "later" not in dead)
        check("exactly two dead values", sorted(dead) == ["dead", "tbl.ghost"])
        check("the constant name is the defined one, not a namespace", dead["dead"].name == "DeadCap" if "dead" in dead else False)

        known = root / "known.txt"
        write(known, "# known dead values\ndead\n")
        buf = io.StringIO()
        with redirect_stdout(buf):
            code_new = vc.main(["--root", str(root), "--check", "--known", str(root / "absent.txt")])
        check("--check fails on dead values nobody listed", code_new == 1 and "DEAD tbl.ghost" in buf.getvalue())
        buf = io.StringIO()
        with redirect_stdout(buf):
            code_partial = vc.main(["--root", str(root), "--check", "--known", str(known)])
        check("--check still fails on the one not in the known list", code_partial == 1 and "DEAD dead" not in buf.getvalue())
        write(known, "dead\ntbl.ghost\n")
        buf = io.StringIO()
        with redirect_stdout(buf):
            code_ok = vc.main(["--root", str(root), "--check", "--known", str(known)])
        check("--check passes when every dead value is known", code_ok == 0)
        write(known, "dead\ntbl.ghost\nused\n")
        buf = io.StringIO()
        with redirect_stdout(buf):
            code_stale = vc.main(["--root", str(root), "--check", "--known", str(known)])
        check("--check flags a known entry that is no longer dead", code_stale == 1 and "STALE used" in buf.getvalue())

        # the value gets a consumer: it must stop being flagged
        write(root / "Script" / "Uses.as", "int Twice() { return R::DeadCap * 2; }\n")
        check("a consumer in a script clears the flag", "dead" not in {d.id for d in vc.find_dead(root, registry)})

    total = passed + len(failed)
    for name in failed:
        print("FAIL:", name)
    print(f"value_consumers_test: {passed}/{total} cases passed")
    return 0 if not failed else 1


if __name__ == "__main__":
    sys.exit(main())
