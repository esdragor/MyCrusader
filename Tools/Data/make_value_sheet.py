"""Writes Valeurs.md and Valeurs.csv (repo root): the sheet of the values "to tune in test", read FOR REAL from the tables and the code.

PT0 of PlaytestInProgress.md (D727). Nothing is copied by hand: Tools/Data/ValueSheet.csv (the registry) says WHERE each value lives
(a table cell, a regular expression on a constant in the code, or nothing yet), its STATE and which question of GDD 17.1 it serves;
this tool reads the real value and writes the sheet.

    python Tools/Data/make_value_sheet.py                      write Valeurs.md and Valeurs.csv, print the checks
    python Tools/Data/make_value_sheet.py --check              checks only (no file written), exit 1 on a failure (quick_check part "values")
    python Tools/Data/make_value_sheet.py --accept <id> --note "..." [--state validé]
                                                               the user answered REGLAGE: / validated it: remember the value seen now

Registry columns: Id, Label, Group, Question (1..10, ";" for several, empty = none), Source, File, Locator, State, Decision, GddRef,
LastSeen, Note.
  Source  table : File = a CSV of Tools/Data, Locator = "<first-column key>:<column>" or "*" (the whole table: its size).
          code  : File = a source file, Locator = a regular expression with ONE group (the value), searched in the file.
          gdd   : nothing to read (a value of the GDD that no code carries yet).
          ignore: the file carries a marker ("provisional", ...) that is NOT a value to tune (a placeholder shape, a wording...): the
                  Note says why. It only tells the orphan check that the file was looked at.
  State   pas codé / provisoire / réglé à l'essai / validé.

Checks: (a) every table or code row finds its value; (b) drift: a "réglé à l'essai" or "validé" value differs from LastSeen (only a
"validé" drift fails the check); (c) orphans: a file of Source, Script or Tools/Data with a marker and no registry row; (d) GDD lines
"à régler en test" whose decision (or section) is in no row (approximate, for human review); (e) "pas codé" rows whose file exists now.
"""
import argparse
import csv
import datetime
import io
import os
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

STATES = ["pas codé", "provisoire", "réglé à l'essai", "validé"]
SOURCES = ["table", "code", "gdd", "ignore"]
COLUMNS = ["Id", "Label", "Group", "Question", "Source", "File", "Locator", "State", "Decision", "GddRef", "LastSeen", "Note"]

QUESTIONS = {
    1: "Le héros est-il important sans être obligatoire ?",
    2: "La mort est-elle suffisamment punitive ?",
    3: "Le délai de résurrection ouvre-t-il une fenêtre intéressante ?",
    4: "Le duel est-il une vraie décision ou une obligation ?",
    5: "La progression marche-t-elle sans système d'âges ?",
    6: "L'économie offre-t-elle assez de décisions ?",
    7: "Les combats d'armée sont-ils intéressants sans le héros ?",
    8: "Le volcan est-il une opportunité plutôt qu'une frustration ?",
    9: "Le snowball est-il puissant sans être irréversible ?",
    10: "Chaque faction a-t-elle une façon différente de jouer ?",
}

MARKER = re.compile(r"(?i)(to be tuned|to tune|tuned in test|indicative|à régler|a regler|provisoire|provisional|\[Q0?\d+\])")
SCAN_DIRS = ["Source", "Script", "Tools/Data"]
SCAN_EXT = (".h", ".cpp", ".as", ".py", ".csv")
SKIP_DIRS = {"Tests", "Intermediate", "Saved", "__pycache__"}
SKIP_NAMES = {"ValueSheet.csv", "make_value_sheet.py", "value_sheet_test.py", "value_consumers.py", "value_consumers_test.py"}
GDD_MARKER = re.compile(r"(?i)à régler en test")


@dataclass
class Item:
    id: str
    text: str


@dataclass
class Drift:
    id: str
    state: str
    seen: str
    now: str


@dataclass
class Gap:
    line: int
    section: str
    text: str


@dataclass
class Report:
    rows: list = field(default_factory=list)
    values: dict = field(default_factory=dict)
    errors: list = field(default_factory=list)
    drifts: list = field(default_factory=list)
    orphans: list = field(default_factory=list)
    gdd_gaps: list = field(default_factory=list)
    ready: list = field(default_factory=list)

    def failed(self):
        return bool(self.errors or self.orphans or any(d.state == "validé" for d in self.drifts))


@dataclass
class Row:
    id: str
    label: str
    group: str
    question: str
    source: str
    file: str
    locator: str
    state: str
    decision: str
    gddref: str
    lastseen: str
    note: str

    def questions(self):
        return [int(q) for q in re.findall(r"\d+", self.question)]


def load_registry(path):
    with open(path, encoding="utf-8-sig", newline="") as f:
        reader = csv.DictReader(f)
        rows = []
        for r in reader:
            rows.append(Row(*[(r.get(c) or "").strip() for c in COLUMNS]))
    return rows


# ---------- reading the real values ----------

def read_text(path):
    return Path(path).read_text(encoding="utf-8-sig", errors="replace")


def read_table(root, file, locator):
    path = Path(root) / file
    with open(path, encoding="utf-8-sig", newline="") as f:
        rows = list(csv.reader(f))
    if not rows:
        raise KeyError("table vide")
    header, body = rows[0], rows[1:]
    if locator == "*":
        return f"{len(body)} lignes, {len(header)} colonnes"
    key, _, col = locator.partition(":")
    if col not in header:
        raise KeyError(f"colonne {col!r} absente")
    for r in body:
        if r and r[0] == key:
            idx = header.index(col)
            return r[idx] if idx < len(r) else ""
    raise KeyError(f"ligne {key!r} absente")


def read_code(root, file, locator):
    text = read_text(Path(root) / file)
    m = re.search(locator, text, re.M)
    if not m:
        raise KeyError("l'expression ne trouve rien")
    value = (m.group(1) if m.groups() else m.group(0)).strip()
    return value[:-1] if re.fullmatch(r"-?\d+\.", value) else value  # 1200.f in C++ is 1200


def read_value(root, row):
    """(value, error text or None, file exists)."""
    if row.source in ("gdd", "ignore"):
        return "", None, True
    path = Path(root) / row.file
    if not row.file or not path.is_file():
        return "", f"la valeur a changé de place : fichier {row.file or '(vide)'} absent", False
    try:
        if row.source == "table":
            return read_table(root, row.file, row.locator), None, True
        return read_code(root, row.file, row.locator), None, True
    except (KeyError, re.error) as e:
        return "", f"la valeur a changé de place : {e} ({row.file} {row.locator})", True


# ---------- orphans and GDD ----------

def scan_markers(root):
    found = []
    for base in SCAN_DIRS:
        top = Path(root) / base
        if not top.is_dir():
            continue
        for dp, dn, fn in os.walk(top):
            dn[:] = sorted(d for d in dn if d not in SKIP_DIRS)
            for name in sorted(fn):
                if not name.endswith(SCAN_EXT) or name in SKIP_NAMES:
                    continue
                if name.endswith("_Test.as") or name.endswith("Tests.cpp") or name.endswith("Test.cpp"):
                    continue
                p = Path(dp) / name
                try:
                    if MARKER.search(read_text(p)):
                        found.append(p.relative_to(root).as_posix())
                except OSError:
                    pass
    return sorted(found)


def registry_keys(rows):
    ds, secs = set(), set()
    for r in rows:
        for text in (r.decision, r.gddref):
            for a, b in re.findall(r"D(\d+)\s*[-–]\s*D?(\d+)", text):
                a, b = int(a), int(b)
                if a <= b and b - a <= 2000:
                    ds.update(range(a, b + 1))
            ds.update(int(x) for x in re.findall(r"D(\d+)", text))
            secs.update(re.findall(r"§\s*(\d+(?:\.\d+)*)", text))
    return ds, secs


def section_covered(sec, secs):
    return any(sec == s or sec.startswith(s + ".") for s in secs)


def gdd_gaps(gdd_path, rows):
    if not Path(gdd_path).is_file():
        return []
    ds, secs = registry_keys(rows)
    gaps = []
    section, heading_ds = "", set()
    for n, line in enumerate(read_text(gdd_path).splitlines(), 1):
        if line.startswith("#"):
            heading_ds = {int(x) for x in re.findall(r"\bD(\d+)", line)}
            m = re.match(r"#+\s+(\d+(?:\.\d+)*)\.?(\s|$)", line)
            if m:
                section = m.group(1)
            continue
        if not GDD_MARKER.search(line):
            continue
        line_ds = {int(x) for x in re.findall(r"\bD(\d+)", line)}
        if (line_ds | heading_ds) & ds:
            continue
        if section and section_covered(section, secs):
            continue
        gaps.append(Gap(n, section, line.strip()))
    return gaps


# ---------- the checks ----------

def run_checks(root, registry, gdd):
    root = Path(root)
    rep = Report()
    rows = load_registry(registry)
    rep.rows = rows
    seen_ids = set()
    for r in rows:
        if not r.id or r.id in seen_ids:
            rep.errors.append(Item(r.id, "identifiant vide ou en double"))
        seen_ids.add(r.id)
        if r.state not in STATES:
            rep.errors.append(Item(r.id, f"état inconnu : {r.state!r}"))
        if r.source not in SOURCES:
            rep.errors.append(Item(r.id, f"source inconnue : {r.source!r}"))
            continue
        value, err, exists = read_value(root, r)
        if r.source == "ignore" and r.file and not (root / r.file).is_file():
            rep.errors.append(Item(r.id, f"fichier cité absent : {r.file}"))
        rep.values[r.id] = value
        if r.state == "pas codé":
            if r.source in ("table", "code") and exists:
                rep.ready.append(r)
        elif err:
            rep.errors.append(Item(r.id, err))
        if r.state in ("réglé à l'essai", "validé") and not err and r.lastseen != value:
            rep.drifts.append(Drift(r.id, r.state, r.lastseen, value))
    cited = {r.file for r in rows if r.file}
    rep.orphans = [f for f in scan_markers(root) if f not in cited]
    rep.gdd_gaps = gdd_gaps(gdd, rows)
    return rep


# ---------- the sheet ----------

def esc(text):
    return str(text).replace("|", "\\|").replace("\n", " ")


def where(r):
    if r.source == "table":
        return f"{Path(r.file).name} {r.locator}"
    if r.source == "code":
        return Path(r.file).name
    return "GDD" if r.source == "gdd" else ""


def sorted_rows(rows):
    return sorted(rows, key=lambda r: (r.group, r.id))


def table_for(rows, rep):
    out = ["| Id | Réglage | Valeur | État | Décision | Où | Note |", "|---|---|---|---|---|---|---|"]
    for r in sorted_rows(rows):
        v = rep.values.get(r.id, "")
        shown = v if v != "" else "—"
        drift = next((d for d in rep.drifts if d.id == r.id), None)
        note = r.note
        if drift:
            note = f"**dérive : vu {drift.seen or '?'}, lu {drift.now}.** {note}".strip()
        out.append(f"| `{r.id}` | {esc(r.label)} | {esc(shown)} | {r.state} | {esc(r.decision)} | {esc(where(r))} | {esc(note)} |")
    return out


def render_md(rep):
    rows = [r for r in rep.rows if r.source != "ignore"]
    ignored = [r for r in rep.rows if r.source == "ignore"]
    counts = {s: sum(1 for r in rows if r.state == s) for s in STATES}
    L = ["# Fiche de valeurs du prototype", "",
         "Générée par `Tools/Data/make_value_sheet.py` depuis `Tools/Data/ValueSheet.csv` (le registre) : **les valeurs sont lues "
         "dans les tables et le code, jamais recopiées**. Ne pas éditer ce fichier : changer la valeur (ou le registre) puis "
         "relancer `python Tools/Data/make_value_sheet.py`. Réglage fait à l'essai (`REGLAGE:`) : "
         "`python Tools/Data/make_value_sheet.py --accept <id> --note \"...\"`. "
         "Les statistiques des unités sont dans `UnitStats.md` (outil voisin).", "",
         "États : *pas codé* (le GDD la nomme, aucun code), *provisoire* (codée, jamais jouée), *réglé à l'essai* (changée après un essai), "
         "*validé* (l'utilisateur l'a confirmée).", "",
         "**" + str(len(rows)) + " réglages** : " + ", ".join(f"{counts[s]} {s}" for s in STATES) + ".", ""]
    for q in range(1, 11):
        sel = [r for r in rows if q in r.questions()]
        L += [f"## Question {q} : {QUESTIONS[q]}", ""]
        L += table_for(sel, rep) if sel else ["Aucun réglage rattaché pour l'instant."]
        L.append("")
    L += ["## Autres valeurs, par chantier", ""]
    groups = sorted({r.group for r in rows if not r.questions()})
    if not groups:
        L += ["Aucune.", ""]
    for g in groups:
        L += [f"### {g}", ""] + table_for([r for r in rows if not r.questions() and r.group == g], rep) + [""]
    if ignored:
        L += ["## Fichiers marqués « provisoire » sans valeur à régler", "",
              "| Id | Fichier | Pourquoi |", "|---|---|---|"]
        for r in sorted_rows(ignored):
            L.append(f"| `{r.id}` | {esc(r.file)} | {esc(r.note)} |")
        L.append("")
    L += ["## Contrôles du jour", "", f"- Valeurs introuvables : {len(rep.errors)}",
          f"- Dérives (réglé / validé) : {len(rep.drifts)}",
          f"- Fichiers marqués absents du registre : {len(rep.orphans)}",
          f"- Lignes du GDD « à régler en test » sans réglage au registre : {len(rep.gdd_gaps)}",
          f"- Lignes « pas codé » dont le fichier existe maintenant : {len(rep.ready)}", ""]
    return "\n".join(L).rstrip("\n") + "\n"


def render_csv(rep):
    out = io.StringIO()
    w = csv.writer(out, lineterminator="\n")
    w.writerow(["Id", "Label", "Group", "Question", "Source", "State", "Value", "Where", "Locator", "Decision", "GddRef", "LastSeen"])
    for r in sorted(rep.rows, key=lambda r: (r.group, r.id)):
        w.writerow([r.id, r.label, r.group, r.question, r.source, r.state, rep.values.get(r.id, ""), r.file, r.locator, r.decision,
                    r.gddref, r.lastseen])
    return out.getvalue()


def sheet_is_stale(root, rep, md_path):
    p = Path(md_path)
    return (not p.is_file()) or p.read_text(encoding="utf-8") != render_md(rep)


def write_text(path, text):
    tmp = Path(str(path) + ".tmp")
    tmp.write_text(text, encoding="utf-8", newline="\n")
    os.replace(tmp, path)


# ---------- the command line ----------

def print_report(rep, check):
    def p(s):
        print(s)
    for e in rep.errors:
        p(f"ERREUR {e.id}: {e.text}")
    for d in rep.drifts:
        p(f"DERIVE {d.id} ({d.state}): vu {d.seen or '?'}, lu {d.now}  -> --accept {d.id} --note \"...\"")
    for o in rep.orphans:
        p(f"ORPHELIN {o}: marqueur « provisoire / à régler » sans ligne au registre")
    for r in rep.ready:
        p(f"PRET {r.id}: le fichier {r.file} existe maintenant, passer la ligne en « provisoire »")
    for g in rep.gdd_gaps:
        p(f"GDD {g.line} (§ {g.section or '?'}): {g.text[:100]}")
    p(f"{len(rep.rows)} lignes, {len(rep.errors)} erreurs, {len(rep.drifts)} dérives, {len(rep.orphans)} orphelins, "
      f"{len(rep.gdd_gaps)} lignes du GDD sans réglage, {len(rep.ready)} prêtes.")


def accept(args, root, registry):
    rows = load_registry(registry)
    target = next((r for r in rows if r.id == args.accept), None)
    if target is None:
        print(f"ERREUR: id inconnu {args.accept!r}")
        return 2
    value, err, _ = read_value(root, target)
    if err or target.source not in ("table", "code"):
        print(f"ERREUR: {args.accept}: {err or 'pas de valeur à retenir pour cette source'}")
        return 2
    target.lastseen = value
    target.state = args.state
    day = args.date or datetime.date.today().isoformat()
    target.note = (target.note + " | " if target.note else "") + f"{day}: {args.note or 'accepté'} (valeur {value})"
    out = io.StringIO()
    w = csv.writer(out, lineterminator="\n")
    w.writerow(COLUMNS)
    for r in rows:
        w.writerow([getattr(r, c.lower()) for c in COLUMNS])
    write_text(registry, out.getvalue())
    print(f"{args.accept}: valeur retenue {value}, état {args.state}")
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--root", default=str(ROOT))
    ap.add_argument("--registry")
    ap.add_argument("--gdd")
    ap.add_argument("--out-md")
    ap.add_argument("--out-csv")
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--accept")
    ap.add_argument("--note", default="")
    ap.add_argument("--state", default="réglé à l'essai", choices=STATES)
    ap.add_argument("--date")
    ap.add_argument("--quiet", action="store_true")
    a = ap.parse_args(argv)
    root = Path(a.root)
    registry = Path(a.registry) if a.registry else root / "Tools" / "Data" / "ValueSheet.csv"
    gdd = Path(a.gdd) if a.gdd else root / "GDD.md"
    md_path = Path(a.out_md) if a.out_md else root / "Valeurs.md"
    csv_path = Path(a.out_csv) if a.out_csv else root / "Valeurs.csv"
    if a.accept:
        return accept(a, root, registry)
    rep = run_checks(root, registry, gdd)
    if not a.check:
        write_text(md_path, render_md(rep))
        write_text(csv_path, render_csv(rep))
    elif sheet_is_stale(root, rep, md_path):
        print("AVERTISSEMENT: Valeurs.md est périmé, relancer python Tools/Data/make_value_sheet.py")
    if not a.quiet:
        print_report(rep, a.check)
    return 1 if rep.failed() else 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    sys.exit(main())
