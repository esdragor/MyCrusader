#!/usr/bin/env python3
"""Q-melee-stall = A: the Sanity.Stall audit (hol.Test.SanityStall, 0.5 s) must tolerate the end of a FinalLeg walk.

Reads scenario logs (Saved_Tests\\Logs\\scenario_*_run?.log). A `SanityStall step N: ... stalls over X s ... phase 3 ...` line is a FALSE
POSITIVE when the soldier is on the FinalLeg walk (phase 3) and X is under the walk's own wait (FinalLegStuckSteps = 40 steps = 2 s) + margin 0.2 s:
the game ends that wait by itself and the soldier then fights. The test is RED (exit 1) while such lines exist, GREEN when none is left
(a stall on a FinalLeg walk that is told with X >= 2.2 is a real one and is allowed to show).

Usage: python sanity_stall_check.py <log> [<log> ...]   (shell globs are expanded here too)
"""
import glob
import re
import sys

LIMIT = 2.2  # FinalLegStuckSteps (2 s) + margin 0.2 s

LINE = re.compile(r"SanityStall step (\d+): unit #(\d+) .*?stalls over ([0-9.]+) s .*?phase (\d+)")


def main(args):
    paths = []
    for arg in args:
        paths.extend(sorted(glob.glob(arg)) or [arg])
    if not paths:
        print("usage: sanity_stall_check.py <log> ...")
        return 2
    bad = []
    seen = 0
    for path in paths:
        try:
            text = open(path, encoding="utf-8", errors="replace").read()
        except OSError as err:
            print("cannot read %s: %s" % (path, err))
            return 2
        for match in LINE.finditer(text):
            seen += 1
            step, unit, limit, phase = int(match.group(1)), int(match.group(2)), float(match.group(3)), int(match.group(4))
            if phase == 3 and limit < LIMIT - 1e-9:
                bad.append((path, step, unit, limit))
    print("sanity_stall_check: %d logs, %d SanityStall lines, %d on a FinalLeg walk under %.1f s" % (len(paths), seen, len(bad), LIMIT))
    for path, step, unit, limit in bad:
        print("  FALSE POSITIVE %s step %d unit #%d (told at %.1f s, phase 3 = FinalLeg)" % (path, step, unit, limit))
    print("sanity_stall_check: %s" % ("RED" if bad else "GREEN"))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
