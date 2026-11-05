"""Tests of Tools/Data/make_value_sheet.py (PT0 of PlaytestInProgress.md, D727).

Run:  python Tools/Data/value_sheet_test.py
Each case builds a small repository in a temporary folder (a table, a script, a GDD, a registry) and checks the tool on it;
the last class runs the tool on the REAL repository (the registry of the day must pass without error).
"""
import contextlib
import csv
import io
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))

import make_value_sheet as mvs  # noqa: E402  (the module under test)

def qmain(argv):
    """mvs.main with its report kept off the console."""
    with contextlib.redirect_stdout(io.StringIO()):
        return mvs.main(argv)


HEADER = "Id,Label,Group,Question,Source,File,Locator,State,Decision,GddRef,LastSeen,Note"

UNITS = "Name,Health,Speed\nSpear,100,5\nHero,500,6\n"
RULES = "namespace R\n{\n\tS.Foo = 42; // indicative, to tune in test\n\tS.Bar = 7;\n}\n"
GDD = """# GDD

## 5. Économie

### 5.3 Population

- La maison ajoute ~10 (valeurs indicatives, à régler en test).
- Le plafond est de 175 (D40) ; la taille est à régler en test.

### 7.2 Combat

#### Moral *(décision D74)*

- Les effets : à régler en test.

## 22. Décisions

| D99 | 2026-11-01 | Test | une valeur à régler en test |
"""


def make_repo(tmp, registry_rows, extra=None, gdd=GDD):
    root = Path(tmp)
    (root / "Tools" / "Data").mkdir(parents=True)
    (root / "Script").mkdir()
    (root / "Tools" / "Data" / "Units.csv").write_text(UNITS, encoding="utf-8")
    (root / "Script" / "Rules.as").write_text(RULES, encoding="utf-8")
    (root / "GDD.md").write_text(gdd, encoding="utf-8")
    for name, text in (extra or {}).items():
        path = root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")
    reg = root / "Tools" / "Data" / "ValueSheet.csv"
    reg.write_text(HEADER + "\n" + "\n".join(registry_rows) + "\n", encoding="utf-8")
    return root


def row(id_, source="code", file="Script/Rules.as", locator=r"S\.Foo\s*=\s*(\d+)", state="provisoire", decision="",
        gddref="", last="", question="1", group="g", label=""):
    out = io.StringIO()
    csv.writer(out, lineterminator="").writerow([id_, label or id_, group, question, source, file, locator, state, decision,
                                                 gddref, last, ""])
    return out.getvalue()


def run(root):
    return mvs.run_checks(Path(root), Path(root) / "Tools" / "Data" / "ValueSheet.csv", Path(root) / "GDD.md")


class ReadValues(unittest.TestCase):
    def test_code_regex_finds_the_value(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a")])
            rep = run(root)
            self.assertEqual(rep.values["a"], "42")
            self.assertEqual(rep.errors, [])

    def test_float_written_1200_dot_is_read_as_1200(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a", locator=r"R = (\d+\.)")], {"Script/Rules.as": "const R = 1200.f;" + chr(10)})
            self.assertEqual(run(root).values["a"], "1200")

    def test_table_cell(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("h", "table", "Tools/Data/Units.csv", "Hero:Health")])
            self.assertEqual(run(root).values["h"], "500")

    def test_whole_table_is_counted(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("u", "table", "Tools/Data/Units.csv", "*")])
            self.assertEqual(run(root).values["u"], "2 lignes, 3 colonnes")

    def test_regex_that_finds_nothing_is_an_error(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a", locator=r"S\.Missing\s*=\s*(\d+)")])
            rep = run(root)
            self.assertEqual([e.id for e in rep.errors], ["a"])
            self.assertIn("changé de place", rep.errors[0].text)

    def test_missing_file_and_missing_cell_are_errors(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("f", file="Script/Nope.as"), row("c", "table", "Tools/Data/Units.csv", "Ghost:Health"),
                                 row("d", "table", "Tools/Data/Units.csv", "Hero:Mana")])
            self.assertEqual(sorted(e.id for e in run(root).errors), ["c", "d", "f"])

    def test_bad_state_source_and_duplicate_are_errors(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a"), row("a"), row("b", state="maybe"), row("c", source="web")])
            ids = sorted(e.id for e in run(root).errors)
            self.assertEqual(ids, ["a", "b", "c"])


class Drift(unittest.TestCase):
    def test_validated_value_that_moved_is_a_drift_and_fails_the_check(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a", state="validé", last="40")])
            rep = run(root)
            self.assertEqual([d.id for d in rep.drifts], ["a"])
            self.assertTrue(rep.failed())

    def test_tuned_value_that_moved_is_listed_but_does_not_fail(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a", state="réglé à l'essai", last="40")])
            rep = run(root)
            self.assertEqual([d.id for d in rep.drifts], ["a"])
            self.assertFalse(rep.failed())

    def test_same_value_no_drift_and_provisional_never_drifts(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a", state="validé", last="42"), row("b", locator=r"S\.Bar\s*=\s*(\d+)", last="1")])
            self.assertEqual(run(root).drifts, [])

    def test_accept_updates_the_value_seen_the_state_and_the_note(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a")])
            reg = root / "Tools" / "Data" / "ValueSheet.csv"
            rc = qmain(["--root", str(root), "--accept", "a", "--note", "trop long", "--date", "2026-11-05"])
            self.assertEqual(rc, 0)
            r = list(csv.DictReader(reg.open(encoding="utf-8")))[0]
            self.assertEqual((r["LastSeen"], r["State"]), ("42", "réglé à l'essai"))
            self.assertIn("2026-11-05", r["Note"])
            self.assertIn("trop long", r["Note"])
            self.assertEqual(run(root).drifts, [])

    def test_accept_unknown_id_fails(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a")])
            self.assertNotEqual(qmain(["--root", str(root), "--accept", "zz", "--note", "x"]), 0)


class Orphans(unittest.TestCase):
    OTHER = "// Values are indicative, to tune.\nint X = 3;\n"

    def test_file_with_marker_not_in_registry_is_an_orphan(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a")], {"Script/Other.as": self.OTHER})
            rep = run(root)
            self.assertEqual(rep.orphans, ["Script/Other.as"])
            self.assertTrue(rep.failed())

    def test_ignore_row_cites_the_file(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a"), row("x", "ignore", "Script/Other.as", "", "pas codé")], {"Script/Other.as": self.OTHER})
            self.assertEqual(run(root).orphans, [])

    def test_tests_are_not_scanned(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a")], {"Script/Tests/T.as": self.OTHER, "Script/Rules_Test.as": self.OTHER,
                                             "Source/X/Tests/FooTests.cpp": self.OTHER})
            self.assertEqual(run(root).orphans, [])

    def test_file_without_marker_is_fine(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a")], {"Script/Plain.as": "int X = 3;\n"})
            self.assertEqual(run(root).orphans, [])

    def test_french_and_english_markers(self):
        for text in ("// à régler en test", "// provisoire", "// to be tuned in play", "// PROVISIONAL", "// [Q04]"):
            with tempfile.TemporaryDirectory() as t:
                root = make_repo(t, [row("a")], {"Script/Other.as": text + "\n"})
                self.assertEqual(run(root).orphans, ["Script/Other.as"], text)


class GddCoverage(unittest.TestCase):
    def test_uncovered_lines_are_listed(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a")])
            gaps = run(root).gdd_gaps
            self.assertEqual(len(gaps), 4)

    def test_a_decision_in_the_registry_covers_its_lines(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a", decision="D99"), row("p", "gdd", "", "", "pas codé", decision="D40")])
            texts = [g.text for g in run(root).gdd_gaps]
            self.assertFalse(any("D99" in x for x in texts))
            self.assertFalse(any("D40" in x for x in texts))

    def test_a_section_in_the_registry_covers_its_lines(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("p", "gdd", "", "", "pas codé", gddref="§ 5.3")])
            gaps = run(root).gdd_gaps
            self.assertEqual([g.section for g in gaps], ["7.2", "22"])

    def test_decision_range_and_heading_decision(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("p", "gdd", "", "", "pas codé", decision="D70-D80, D98-D100")])
            gaps = run(root).gdd_gaps
            self.assertFalse(any("D99" in g.text for g in gaps))
            self.assertFalse(any(g.section == "7.2" for g in gaps))  # heading carries D74

    def test_gdd_missing_is_not_a_crash(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a")])
            (root / "GDD.md").unlink()
            self.assertEqual(run(root).gdd_gaps, [])


class Ready(unittest.TestCase):
    def test_not_coded_row_whose_file_exists_is_ready(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("n", "code", "Script/Rules.as", r"S\.Foo\s*=\s*(\d+)", "pas codé")])
            self.assertEqual([x.id for x in run(root).ready], ["n"])

    def test_not_coded_row_whose_file_is_absent_is_not_an_error(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("n", "code", "Script/Future.as", r"X=(\d+)", "pas codé")])
            rep = run(root)
            self.assertEqual(rep.errors, [])
            self.assertEqual(rep.ready, [])
            self.assertEqual(rep.values["n"], "")


class Output(unittest.TestCase):
    def test_two_runs_and_a_shuffled_registry_give_the_same_sheet(self):
        rows = [row("b", question="2"), row("a", question="1"), row("c", "gdd", "", "", "pas codé", question="", group="z")]
        with tempfile.TemporaryDirectory() as t1, tempfile.TemporaryDirectory() as t2:
            r1 = make_repo(t1, rows)
            r2 = make_repo(t2, list(reversed(rows)))
            self.assertEqual(qmain(["--root", str(r1)]), 0)
            self.assertEqual(qmain(["--root", str(r1), "--out-md", str(r1 / "second.md")]), 0)
            self.assertEqual(qmain(["--root", str(r2)]), 0)
            a = (r1 / "Valeurs.md").read_text(encoding="utf-8")
            self.assertEqual(a, (r1 / "second.md").read_text(encoding="utf-8"))
            self.assertEqual(a, (r2 / "Valeurs.md").read_text(encoding="utf-8"))
            self.assertEqual((r1 / "Valeurs.csv").read_text(encoding="utf-8"), (r2 / "Valeurs.csv").read_text(encoding="utf-8"))

    def test_sheet_has_a_section_per_question_and_the_real_value(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a", question="3", label="Delai de retour")])
            qmain(["--root", str(root)])
            md = (root / "Valeurs.md").read_text(encoding="utf-8")
            self.assertEqual(md.count("\n## Question "), 10)
            self.assertIn("`a`", md)
            self.assertIn("| 42 |", md)
            self.assertIn("Delai de retour", md)

    def test_check_writes_nothing_and_exit_code_follows_failures(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a")])
            self.assertEqual(qmain(["--root", str(root), "--check"]), 0)
            self.assertFalse((root / "Valeurs.md").exists())
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a", locator=r"Nope(\d+)")])
            self.assertEqual(qmain(["--root", str(root), "--check"]), 1)

    def test_stale_sheet_is_a_warning_only(self):
        with tempfile.TemporaryDirectory() as t:
            root = make_repo(t, [row("a")])
            qmain(["--root", str(root)])
            (root / "Script" / "Rules.as").write_text(RULES.replace("42", "43"), encoding="utf-8")
            rep = run(root)
            self.assertTrue(mvs.sheet_is_stale(root, rep, root / "Valeurs.md"))
            self.assertEqual(qmain(["--root", str(root), "--check"]), 0)


class RealRepository(unittest.TestCase):
    def test_the_registry_of_the_day_passes(self):
        reg = ROOT / "Tools" / "Data" / "ValueSheet.csv"
        self.assertTrue(reg.exists(), "Tools/Data/ValueSheet.csv is missing")
        rep = mvs.run_checks(ROOT, reg, ROOT / "GDD.md")
        self.assertEqual([f"{e.id}: {e.text}" for e in rep.errors], [])
        self.assertEqual(rep.orphans, [])
        self.assertEqual([d.id for d in rep.drifts if d.state == "validé"], [])
        self.assertGreater(len(rep.values), 50)

    def test_real_gdd_leftovers_are_justified(self):
        rep = mvs.run_checks(ROOT, ROOT / "Tools" / "Data" / "ValueSheet.csv", ROOT / "GDD.md")
        self.assertEqual([f"GDD:{g.line} {g.text[:60]}" for g in rep.gdd_gaps], [])


if __name__ == "__main__":
    unittest.main(verbosity=1)
