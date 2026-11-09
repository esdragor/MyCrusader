"""The glade style of each map (Retours\\Q-glade-style.md; the user's answer of 2026-11-09 to E-glade-sweep: "a different style of glades per map"), without the editor.
common.GLADE_STYLES = the eight styles of the sweep (letter -> (glades, scale, reach), the same numbers as maps/glade_sweep.VARIANTS), common.GLADE_STYLE_BY_MAP =
the style of every map by its recipe.map_name (one line per map to change), common.use_glade_style(map_name) = what make_map calls first: it sets GLADE_FORCE / GLADE_SCALE /
GLADE_REACH from the table. Read on the real chain (grove_points, the function the maps call): a map with a style other than A gets other trees than style A, going back to
style A gives the A trees again (no cache of one style leaks into another), and the grove picks file (grove_signature) stays valid across styles.
  python Tools/Tests/glade_style_test.py        prints one line per check and PASS / FAIL; exit code 0 / 1
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import maps_stub  # noqa: E402

maps_stub.install()


def main():
    from maps import common  # noqa: E402
    from maps import glade_sweep  # noqa: E402

    failed = []

    def check(name, ok, detail=""):
        print("%s: %s%s" % (name, "ok" if ok else "FAIL", "" if ok or not detail else " (" + detail + ")"))
        if not ok:
            failed.append(name)

    styles = getattr(common, "GLADE_STYLES", None)
    by_map = getattr(common, "GLADE_STYLE_BY_MAP", None)
    use = getattr(common, "use_glade_style", None)
    check("common.GLADE_STYLES, GLADE_STYLE_BY_MAP and use_glade_style exist", styles is not None and by_map is not None and use is not None)
    if styles is None or by_map is None or use is None:
        print("glade_style_test : FAIL (%s)" % ", ".join(failed))
        return 1

    check("the eight letters A to H", sorted(styles) == list("ABCDEFGH"), "%s" % sorted(styles))
    check("the styles are the sweep's variants (one source of numbers)",
          all(tuple(styles[letter]) == (glades, scale, reach) for letter, glades, scale, reach in glade_sweep.VARIANTS))
    check("style A is today's maps (no glade forced, scale 1, reach 0.55)", tuple(styles.get("A", ())) == (0, 1.0, 0.55))

    names = []
    for key in ("Steppe", "Plain", "Forest", "Cols", "Fords", "ElDorado", "Crown", "Hinterland"):
        try:
            places, side = common.read_sizes()["Small"]
            names.append((key, common.sized_recipe(key, "Small", places, side).map_name))
        except Exception as error:  # noqa: BLE001  (the name of the map is unknown to the recipes)
            check("recipe of %s" % key, False, str(error)[:80])
    print("map names: %s" % ", ".join("%s=%s" % pair for pair in names))
    check("every map has a style of the table", all(by_map.get(name) in styles for _, name in names), "%s" % {name: by_map.get(name) for _, name in names})

    def forest():
        return common.grove_points(7, (0.0, 0.0), 300, reach=3500.0)

    baseline = forest()  # the module as loaded: style A everywhere
    signature = common.grove_signature()
    styled = [name for _, name in names if by_map.get(name, "A") != "A"]
    check("at least two maps have a style other than A (a style that varies from map to map)", len(styled) >= 2, "%d maps" % len(styled))
    if styled:
        use(styled[0])
        common.SHAPES.clear()
        other = forest()
        check("a map with style %s has other trees than style A" % by_map.get(styled[0]), other != baseline)
        check("the picks file stays valid across styles (same signature)", common.grove_signature() == signature)
        use("no such map")  # a name the table does not know = style A
        check("an unknown map falls back to style A (force 0, scale 1, reach 0.55)", (common.GLADE_FORCE, common.GLADE_SCALE, common.GLADE_REACH) == (0, 1.0, 0.55))
        check("going back to style A gives the A trees again (no leak between styles)", forest() == baseline)
        use(styled[0])
        check("the styled trees come back for the styled map (not the cache of A)", forest() == other)
    use("no such map")

    print("glade_style_test : %s" % ("PASS" if not failed else "FAIL (%s)" % ", ".join(failed)))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
