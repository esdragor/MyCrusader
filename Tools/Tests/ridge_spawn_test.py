"""No scenario of a sized map spawns a unit ON the ridge of 3 boulders (SAURON-1116d, the deploy BUG: the Mangonel was born ON the ridge).
The ridge (common.RIDGE_AHEAD, common.RIDGE_BOULDERS, D286 / J4) lies RIDGE_AHEAD (30 m) in front of every centre, across the way to the middle.
`hol.Camera.FocusNear <Dx> <Dy>` puts the camera at (Dx, Dy) cm in the frame of the player's main centre (Dx toward the middle) and
`hol.Units.SpawnHere` spawns at the camera's focus point: a scenario with a "mapSize" in its .json that spawns after a FocusNear whose point
falls on a boulder (footprint = the boulder's flattened sphere plus a margin for the unit) is a FAIL.
The footprint is CALCULATED from the REAL constants of Tools/Editor/maps/common.py (the boulder spawn_ridges lays: scale.x along the ridge, scale.y across it).
  python Tools/Tests/ridge_spawn_test.py       prints one line per scenario checked and PASS / FAIL; exit code 0 / 1
"""
import glob
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import maps_stub  # noqa: E402

maps_stub.install()
from maps import common  # noqa: E402

SCENARIOS = os.path.join(os.path.dirname(HERE), "Scenarios")
UNIT_MARGIN = 100.0  # cm of clearance around a boulder for a unit born next to it
FAILS = []


def check(ok, text):
    print(("OK   " if ok else "FAIL ") + text)
    if not ok:
        FAILS.append(text)


def on_ridge(dx, dy, radius=0.0):
    """True when the disc of `radius` cm around (dx, dy), in the centre's frame, touches one of the ridge's boulders (+ UNIT_MARGIN).
    A boulder is a 1 m sphere scaled (x along the ridge, y across it, as spawn_ridges yaws it) at (RIDGE_AHEAD, offset)."""
    for offset, scale in common.RIDGE_BOULDERS:
        half_across = scale.y * 50.0 + UNIT_MARGIN + radius
        half_along = scale.x * 50.0 + UNIT_MARGIN + radius
        if abs(dx - common.RIDGE_AHEAD) <= half_across and abs(dy - offset) <= half_along:
            return True
    return False


def spawn_points(path):
    """(line number, Dx, Dy) of every hol.Units.SpawnHere of the script, with the last hol.Camera.FocusNear (not 'enemy') before it
    (None when no FocusNear came before: the camera sits on the centre itself, Dx 0 Dy 0)."""
    focus = (0.0, 0.0)
    found = []
    with open(path, encoding="utf-8") as handle:
        for number, line in enumerate(handle, 1):
            words = line.split()
            if not words or line.lstrip().startswith("#"):
                continue
            if words[0] == "hol.Camera.FocusNear" and len(words) >= 3 and "enemy" not in words[3:]:
                focus = (float(words[1]), float(words[2]))
            elif words[0] == "hol.Units.SpawnHere":
                # Arguments: Count Type Slot Radius (cm); the radius is the scatter around the focus.
                found.append((number, focus[0], focus[1], float(words[4]) if len(words) >= 5 else 0.0))
    return found


def ridge_maps():
    """Lower-case names of the maps whose builder lays the ridge (a Tools/Editor/maps/<name>.py that calls common.spawn_ridges, e.g. plain, steppe)."""
    names = set()
    for path in glob.glob(os.path.join(os.path.dirname(HERE), "Editor", "maps", "*.py")):
        if os.path.basename(path) in ("common.py", "__init__.py"):
            continue
        with open(path, encoding="utf-8") as handle:
            if "common.spawn_ridges(" in handle.read():
                names.add(os.path.basename(path)[:-3])
    return names


def sized(path, maps):
    """True when the scenario's .json has a "mapSize" and its map (/Game/Maps/<Name>.<Name>) is one of `maps` (a map without the ridge, as ElDorado, is out)."""
    try:
        with open(path[:-4] + ".json", encoding="utf-8") as handle:
            data = json.load(handle)
    except (OSError, ValueError):
        return False
    name = re.sub(r"^.*/", "", str(data.get("map", ""))).split(".")[0].lower()
    return "mapSize" in data and name in maps


def main():
    # The check is real: the footprint says "on" at the ridge's heart and "off" far from it (guards a constants change that would blind the rule).
    check(on_ridge(common.RIDGE_AHEAD, 0.0), "the ridge's centre (%d, 0) is on the ridge" % common.RIDGE_AHEAD)
    check(not on_ridge(0.0, 0.0), "the centre itself (0, 0) is not on the ridge")
    maps = ridge_maps()
    check("plain" in maps and "steppe" in maps, "the ridge maps found: %s" % ", ".join(sorted(maps)))
    for path in sorted(glob.glob(os.path.join(SCENARIOS, "*.txt"))):
        if not sized(path, maps):
            continue
        for number, dx, dy, radius in spawn_points(path):
            name = os.path.basename(path)
            check(not on_ridge(dx, dy, radius), "%s:%d SpawnHere (radius %g) at the focus (%g, %g) is off the ridge" % (name, number, radius, dx, dy))
    print("PASS" if not FAILS else "FAIL (%d)" % len(FAILS))
    return 0 if not FAILS else 1


if __name__ == "__main__":
    sys.exit(main())
