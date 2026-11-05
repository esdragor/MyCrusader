"""Writes Tools/Data/FactionOverrides.csv into Source/HeroesOfLiberty/Match/HOLFactionRules.cpp (the built-in copy a packaged game reads).

Run after every edit of the CSV:  python -I C:\\...\\Tools\\Data\\make_builtin_overrides.py
The block between '// BEGIN generated overrides' and '// END generated overrides' is replaced; nothing else is touched.
The test HeroesOfLiberty.Match.Factions.Tables fails when the two differ.
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
CSV = os.path.join(HERE, "FactionOverrides.csv")
CPP = os.path.join(ROOT, "Source", "HeroesOfLiberty", "Match", "HOLFactionRules.cpp")


def main():
    with open(CSV, "r", encoding="utf-8-sig", newline="") as handle:
        lines = [line for line in handle.read().replace("\r\n", "\n").split("\n") if line.strip()]
    body = "\n".join("\t\t\t\"%s\\n\"" % line.replace("\\", "\\\\").replace("\"", "\\\"") for line in lines)
    block = "\t\t// BEGIN generated overrides\n\t\tconst TCHAR* BuiltInOverrides = TEXT(\n%s);\n\t\t// END generated overrides" % body
    with open(CPP, "rb") as handle:
        raw = handle.read()
    crlf = b"\r\n" in raw
    text = raw.decode("utf-8").replace("\r\n", "\n")
    pattern = re.compile(r"\t\t// BEGIN generated overrides\n.*?\t\t// END generated overrides", re.S)
    if not pattern.search(text):
        print("markers not found in", CPP)
        return 1
    new = pattern.sub(lambda _: block, text, count=1)
    if crlf:
        new = new.replace("\n", "\r\n")
    with open(CPP, "wb") as handle:
        handle.write(new.encode("utf-8"))
    print("BuiltInOverrides: %d lines written (%d rows)" % (len(lines), len(lines) - 1))
    return 0


sys.exit(main())
