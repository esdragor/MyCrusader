"""Orphan values: registry values (Tools/Data/ValueSheet.csv) that NOTHING in the game reads (Auto_Journey_File point 4, 2026-11-11).

make_value_sheet.py already checks that every registry row finds its value and that every marked file has a row. This tool checks the
other way round: a row that the sheet reads fine, but whose constant (code row) or column (table row) has no consumer left in the game, so
tuning it changes nothing.

    python Tools/Data/value_consumers.py --list      print every dead value (kind, name, file), exit 0
    python Tools/Data/value_consumers.py --check     exit 1 on a dead value that is not in Tools/Data/orphan_values_known.txt,
                                                     and on a known entry that is not dead any more (quick_check part "values")

How a consumer is found (no compiler, plain text, so it is approximate on purpose):
  code row   : the name is the first identifier of the text the Locator matches (keywords and "Scope::" prefixes skipped). It is dead when
               that name appears nowhere else in Source/ and Script/ (headers, sources, scripts; test files do not count).
  table row  : "Key:Column" rows only. The column name is dead when no word of Source/ and Script/ is that name.
  not looked at: gdd / ignore rows, "pas codé" rows, whole-table ("*") rows, rows whose file or value is missing (make_value_sheet reports them).
A name used only through a macro-built or concatenated string is a false "dead": put its id in orphan_values_known.txt with the reason.
"""
import argparse
import importlib.util
import os
import re
import sys
from dataclasses import dataclass
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONSUMER_DIRS = ["Source", "Script"]
CONSUMER_EXT = (".h", ".cpp", ".inl", ".as")
SKIP_DIRS = {"Intermediate", "Saved", "__pycache__", "Tests"}
KEYWORDS = {
    "return", "case", "const", "constexpr", "static", "inline", "int", "int32", "int64", "uint8", "uint32", "float", "double", "bool",
    "void", "auto", "unsigned", "signed", "struct", "class", "enum", "namespace", "if", "else", "for", "while", "switch", "default",
    "true", "false", "nullptr", "FString", "FName", "TArray", "TMap", "uint16", "int16", "uint64", "char", "string", "array", "dictionary",
}
WORD = re.compile(r"[A-Za-z_]\w*")


def load(name):
    spec = importlib.util.spec_from_file_location(name, HERE / (name + ".py"))
    mod = importlib.util.module_from_spec(spec)
    sys.modules[name] = mod
    spec.loader.exec_module(mod)
    return mod


@dataclass
class Dead:
    id: str
    kind: str
    name: str
    file: str


def consumer_files(root):
    for base in CONSUMER_DIRS:
        top = Path(root) / base
        if not top.is_dir():
            continue
        for dp, dn, fn in os.walk(top):
            dn[:] = sorted(d for d in dn if d not in SKIP_DIRS)
            for name in sorted(fn):
                if not name.endswith(CONSUMER_EXT):
                    continue
                if name.endswith("_Test.as") or name.endswith("Tests.cpp") or name.endswith("Test.cpp"):
                    continue
                yield Path(dp) / name


def word_counts(root, read_text):
    counts = {}
    for p in consumer_files(root):
        try:
            text = read_text(p)
        except OSError:
            continue
        for w in WORD.findall(text):
            counts[w] = counts.get(w, 0) + 1
    return counts


def defined_name(matched):
    """First identifier of the matched text that is not a keyword and not a scope ("Scope::Name" keeps Name)."""
    for m in WORD.finditer(matched):
        if m.group(0) in KEYWORDS:
            continue
        if matched[m.end():m.end() + 2] == "::":
            continue
        return m.group(0)
    return ""


def find_dead(root, registry):
    mvs = load("make_value_sheet")
    root = Path(root)
    rows = mvs.load_registry(registry)
    counts = word_counts(root, mvs.read_text)
    dead = []
    for r in rows:
        if r.source not in ("code", "table") or r.state == "pas codé" or not r.file:
            continue
        path = root / r.file
        if not path.is_file():
            continue
        if r.source == "table":
            key, sep, col = r.locator.partition(":")
            if not sep or not col or col == "*":
                continue
            if counts.get(col, 0) == 0:
                dead.append(Dead(r.id, "column", col, r.file))
            continue
        try:
            m = re.search(r.locator, mvs.read_text(path), re.M)
        except re.error:
            continue
        if not m:
            continue
        name = defined_name(m.group(0))
        if not name:
            continue
        own = sum(1 for w in WORD.findall(m.group(0)) if w == name)
        if counts.get(name, 0) - own <= 0:
            dead.append(Dead(r.id, "constant", name, r.file))
    return dead


CONSTANT = re.compile(r"^\s*(?:static\s+|inline\s+|constexpr\s+|const\s+)+[\w:<>]+\s+(\w+)\s*(?:\[[^\]]*\])?\s*=\s*[-+0-9{.]", re.M)


@dataclass
class Uncovered:
    name: str
    file: str
    line: int


def find_uncovered(root, registry):
    """The inverse, report only: numeric constants of a file the registry cites (a code row) that no row names (neither as the
    constant a Locator matches nor inside a Locator). Files the registry does not cite are make_value_sheet's orphan check."""
    mvs = load("make_value_sheet")
    root = Path(root)
    rows = [r for r in mvs.load_registry(registry) if r.source == "code" and r.file and (root / r.file).is_file()]
    out = []
    for f in sorted({r.file for r in rows}):
        text = mvs.read_text(root / f)
        covered = set()
        for r in (x for x in rows if x.file == f):
            covered.update(WORD.findall(r.locator))
            try:
                m = re.search(r.locator, text, re.M)
            except re.error:
                continue
            if m:
                covered.add(defined_name(m.group(0)))
        for m in CONSTANT.finditer(text):
            if m.group(1) not in covered:
                out.append(Uncovered(m.group(1), f, text.count("\n", 0, m.start(1)) + 1))
    return out


def read_known(path):
    known = {}
    p = Path(path)
    if not p.is_file():
        return known
    for line in p.read_text(encoding="utf-8-sig").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        known[line.split()[0]] = line
    return known


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--root", default=str(HERE.parents[1]))
    ap.add_argument("--registry")
    ap.add_argument("--known")
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--list", action="store_true")
    ap.add_argument("--uncovered", action="store_true")
    a = ap.parse_args(argv)
    root = Path(a.root)
    registry = Path(a.registry) if a.registry else root / "Tools" / "Data" / "ValueSheet.csv"
    known_path = Path(a.known) if a.known else root / "Tools" / "Data" / "orphan_values_known.txt"
    if a.uncovered:
        found = find_uncovered(root, registry)
        for u in found:
            print(f"UNCOVERED {u.name} ({u.file}:{u.line})")
        print(f"{len(found)} constants of cited files have no registry row (report only).")
        return 0
    dead = find_dead(root, registry)
    known = read_known(known_path)
    ids = {d.id for d in dead}
    fresh = [d for d in dead if d.id not in known]
    stale = sorted(k for k in known if k not in ids)
    if a.list or not a.check:
        for d in dead:
            print(f"DEAD {d.id} ({d.kind} {d.name} in {d.file})" + ("" if d.id not in known else "  [known]"))
    else:
        for d in fresh:
            print(f"DEAD {d.id} ({d.kind} {d.name} in {d.file}): no consumer in Source/ or Script/, remove it from the registry or "
                  f"list it in {known_path.name} with the reason")
    if a.check:
        for k in stale:
            print(f"STALE {k}: listed in {known_path.name} but it has a consumer now (or no row), remove the line")
    print(f"{len(dead)} dead values, {len(dead) - len(fresh)} known, {len(fresh)} new, {len(stale)} stale known entries.")
    return 1 if a.check and (fresh or stale) else 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    sys.exit(main())
