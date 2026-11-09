"""The glade style of each map against the room of its forests (Retours\\Q-glade-style.md, Q-glade-roomerror.md), without the editor: Tools/Tests/glade_budget.py plays the
map's REAL chain (lay_relays_and_sources, outside_relays, apply_edits, sites, outside_sites: the lines of common.make_map that draw the trees) for a map at a size with a glade
style and says whether it comes out or the room runs out (steppe.RoomError, the "no room / cannot hold" refusals of the generator, the ones make_maps.ps1 would meet in the editor).
  python Tools/Tests/glade_budget_test.py          cheap checks (the detector works on the real chain; Plain Small and Steppe Small come out); PASS / FAIL, exit code 0 / 1
  python Tools/Tests/glade_budget_test.py --full   also the 7 maps x 5 sizes with the coded GLADE_STYLE_BY_MAP: every one must come out (minutes; one process per map is quicker:
                                                   python Tools/Tests/glade_budget.py <Map> prints that map's five sizes)
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))


def main():
    failed = []

    def check(name, ok, detail=""):
        print("%s: %s%s" % (name, "ok" if ok else "FAIL", "" if ok or not detail else " (" + detail + ")"))
        if not ok:
            failed.append(name)

    try:
        import glade_budget  # noqa: E402
    except ImportError as error:
        check("Tools/Tests/glade_budget.py exists (the simulator)", False, str(error))
        print("glade_budget_test : FAIL (%s)" % ", ".join(failed))
        return 1

    plain = glade_budget.simulate("Plain", "Small")
    check("Plain Small comes out with its coded style", plain["status"] == "ok", "%s" % plain)
    check("the simulator counts the trees of the real chain (Plain Small: some, per player)", plain.get("trees", 0) > 100 and plain.get("per_player", 0) > 50, "%s" % plain)
    steppe = glade_budget.simulate("Steppe", "Small")
    check("Steppe Small comes out with style G", steppe["status"] == "ok" and steppe["style"] == "G", "%s" % steppe)

    # The detector: a refusal of the generator (here steppe.RoomError, raised by the map module's own sources(), the way the real Steppe raises it) is reported as a refusal
    # with its message, not raised and not "ok".
    from maps import plain  # noqa: E402
    from maps import steppe  # noqa: E402

    class Refusing:
        def __getattr__(self, name):
            return getattr(plain, name)

        @staticmethod
        def sources(recipe):
            raise steppe.RoomError("Steppe: territory of slot 0 cannot hold 99999 trees in 3 groves")

    refused = glade_budget.simulate("Plain", "Small", module=Refusing())
    check("a RoomError of the map's sources is reported as a refusal with its message", refused["status"] == "RoomError" and "cannot hold" in refused.get("message", ""), "%s" % refused)
    again = glade_budget.simulate("Plain", "Small")
    check("a refused run leaves no trace in the next one (same trees as before)", again.get("trees") == plain.get("trees"), "%s against %s" % (again.get("trees"), plain.get("trees")))

    if "--full" in sys.argv:
        for name in glade_budget.MAPS:
            for row in glade_budget.simulate_map(name):
                check("%s %s with style %s" % (name, row["size"], row["style"]), row["status"] == "ok", row.get("message", ""))

    print("glade_budget_test : %s" % ("PASS" if not failed else "FAIL (%s)" % ", ".join(failed)))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
