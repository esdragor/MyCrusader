"""The groves' dart throwing stays fast and gives the same trees (MapsInProgress.md, Haldir-5, 2026-11-08).

Hinterland's pockets hold groves of 210 trees. common.grove_attempt used to check every candidate tree against ALL the trees already placed
(free(): O(n) per try), up to 52 500 tries per attempt, up to 48 attempts per grove: Hinterland:Small relays(recipe) (which lays the whole plan) took
minutes in plain Python (the same before the sites' commits: measured on c158640). Two checks, on the real chain, without the editor (stand-in `unreal`):
  1. SAME TREES: grove_attempt gives, for fixed (seed, count, attempt), exactly the trees it gave with the all-pairs check (hashes taken BEFORE the fix);
     the maps in the repository were generated with those trees.
  2. FAST: Hinterland Small, relays(recipe) and sites(recipe), end to end in GROVE_BUDGET seconds.
  python Tools/Tests/grove_speed_test.py        prints one line per check and PASS / FAIL; exit code 0 / 1
"""
import hashlib
import importlib
import os
import sys
import threading
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import maps_stub  # noqa: E402

maps_stub.install()
from maps import common  # noqa: E402

GROVE_BUDGET = 40.0  # seconds for relays + sites of Hinterland:Small with the picks file (Haldir-6: about 3 s; without it, about a minute)

# (seed, count, attempt, compact) -> sha1[:16] of the trees' offsets, taken with the all-pairs free() (c158640)
SAME_TREES = {
    (3, 40, 0, False): "7da2976af6fe5f0f",
    (5, 25, 2, False): "7f25c17986204451",
    (1234, 14, 0, False): "a0f0776e42322eca",
    (8, 100, 1, False): "70b3a75b8bd4ba60",
    (1234, 210, 0, False): "d6b39fac5f55293a",
    (77, 210, 33, True): "0fcb6962792f4c10",
    (1234, 60, 1, False): "6eef6648406c333a",  # these three: no room found (None)
    (99, 60, 40, True): "6eef6648406c333a",
    (3, 40, 35, True): "6eef6648406c333a",
}


def digest(trees):
    text = "None" if trees is None else ";".join("%.6f,%.6f" % tree for tree in trees)
    return hashlib.sha1(text.encode()).hexdigest()[:16]


def same_trees():
    failed = 0
    for (seed, count, attempt, compact), wanted in SAME_TREES.items():
        got = digest(common.grove_attempt(common.Rng(seed, 11, attempt), count, compact))
        if got != wanted:
            failed += 1
            print("same trees %s: %s, wanted %s  <-- FAIL" % ((seed, count, attempt, compact), got, wanted))
    print("same trees: %d of %d identical  %s" % (len(SAME_TREES) - failed, len(SAME_TREES), "PASS" if not failed else "FAIL"))
    return failed


def picks_same():
    """The picks file only skips the search: grove_local with the chosen attempt given gives the trees the full search gives."""
    failed = 0
    keys = [(3, 40, 500.0), (5, 25, 330.0), (1234, 14, 850.0), (8, 100, 1500.0), (21, 14, 329.0)]
    saved = common.save_grove_picks
    common.save_grove_picks = lambda: None  # the test keys do not go in the file
    for seed, count, bound in keys:
        attempt, cold = common.grove_search(seed, count, bound)
        pick_key = "%d:%d:%r" % (seed, count, bound)
        common.SHAPES.clear()
        common.grove_picks()[pick_key] = attempt
        warm = common.grove_local(seed, count, bound)
        del common.grove_picks()[pick_key]
        common.SHAPES.clear()
        searched = common.grove_local(seed, count, bound)  # no pick: the search itself
        del common.grove_picks()[pick_key]
        if digest(warm) != digest(cold) or digest(searched) != digest(cold):
            failed += 1
            print("picks %s: attempt %d gives other trees  <-- FAIL" % ((seed, count, bound), attempt))
    common.save_grove_picks = saved
    common.SHAPES.clear()
    print("picks same: %d of %d identical  %s" % (len(keys) - failed, len(keys), "PASS" if not failed else "FAIL"))
    return failed


def hinterland_fast(passes=3):
    """Hinterland Small relays+sites, PASSES times (the worst counts: the machine is shared with other agents), the shapes forgotten each time."""
    maps = common.read_maps()
    places, side = common.read_sizes()["Small"]
    recipe = common.sized_recipe("Hinterland", "Small", places, side)
    module = importlib.import_module("maps." + maps["Hinterland"])
    worst = 0.0
    for number in range(passes):
        common.SHAPES.clear()
        result = {}

        def work():
            result["relays"] = module.relays(recipe)
            result["sites"] = module.sites(recipe)

        start = time.time()
        thread = threading.Thread(target=work, daemon=True)
        thread.start()
        thread.join(GROVE_BUDGET)
        if thread.is_alive():
            print("Hinterland Small relays+sites: pass %d not done in %.0f s (picks file missing or stale? run again to fill it)  <-- FAIL" % (number + 1, GROVE_BUDGET))
            return 1
        seconds = time.time() - start
        worst = max(worst, seconds)
        if not (len(result["relays"]) > 0 and len(result["sites"]) > 0):
            print("Hinterland Small relays+sites: nothing laid  <-- FAIL")
            return 1
    ok = worst <= GROVE_BUDGET
    print("Hinterland Small relays+sites: worst of %d passes %.1f s (budget %.0f; HOL_GROVE_PICKS=off gives the cold time, about a minute)  %s" % (passes, worst, GROVE_BUDGET, "PASS" if ok else "FAIL"))
    return 0 if ok else 1


if __name__ == "__main__":
    failed = same_trees() + picks_same() + hinterland_fast()
    print("grove_speed_test: %s" % ("PASS" if not failed else "FAIL"))
    sys.stdout.flush()
    os._exit(1 if failed else 0)
