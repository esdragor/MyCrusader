"""Ambiance lumineuse par carte (chantier Cartes, 2026-11-03), sans l'editeur (stand-in `unreal`, maps_stub.py). Echoue si :
  - une carte de Tools/Data/Maps.csv n'a pas d'entree dans ambience.AMBIENCES ;
  - une ambiance active allume une ombre (cast_shadows True) ou regle une autre propriete que la liste autorisee (soleil, ciel, brume) ;
  - une valeur sort des bornes moderees (ambience.BOUNDS : couleurs a pas plus de 20 % d'ecart entre canaux, soleil jamais plus raide que -70 degres...) ;
  - une carte sans entree ne garde pas les valeurs du moteur (apply ne regle alors que cast_shadows False) ;
  - Plain n'est pas la reference inchangee (None) ;
  - le controle lui-meme n'attrape pas ses variantes fautives (ombre allumee, valeur hors bornes, propriete en trop, entree manquante).
  python Tools/Tests/ambience_test.py        PASS / FAIL, code de sortie 0 / 1
"""
import copy
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import maps_stub  # noqa: E402

maps_stub.install()
from maps import ambience, common  # noqa: E402


class Recorder:
    """A light or fog component that notes what is set on it."""

    def __init__(self):
        self.calls = []

    def set_editor_property(self, name, value):
        self.calls.append((name, value))


def problems_of(table, names, apply=ambience.apply):
    """Every way `table` (name -> ambience or None) breaks the rules, for the maps `names`."""
    found = []
    for name in names:
        if name not in table:
            found.append("%s: no entry in AMBIENCES" % name)
            continue
        entry = table[name]
        if entry is None:
            continue
        if set(entry) != set(ambience.KEYS):
            found.append("%s: keys %s" % (name, sorted(set(entry) ^ set(ambience.KEYS))))
            continue
        for key, bound in ambience.BOUNDS.items():
            value = entry[key]
            if isinstance(value, tuple):
                low, high, ratio = bound
                if len(value) != 3 or not all(low <= channel <= high for channel in value):
                    found.append("%s.%s %s outside %s..%s" % (name, key, value, low, high))
                elif max(value) > ratio * min(value):
                    found.append("%s.%s %s: channels more than x%s apart" % (name, key, value, ratio))
            elif not bound[0] <= value <= bound[1]:
                found.append("%s.%s = %s outside %s..%s" % (name, key, value, bound[0], bound[1]))
    # What apply() does (with the table swapped in): the lights' shadows stay off, only the allowed properties are set.
    saved = ambience.AMBIENCES
    ambience.AMBIENCES = table
    try:
        for name in names:
            sun, sky, fog = Recorder(), Recorder(), Recorder()
            try:
                apply(name, sun, sky, fog)
            except KeyError as error:
                found.append("%s: apply() fails, key %s" % (name, error))
                continue
            for label, recorder, allowed in (("sun", sun, ambience.SUN_PROPERTIES), ("sky", sky, ambience.SKY_PROPERTIES), ("fog", fog, ambience.FOG_PROPERTIES)):
                for prop, value in recorder.calls:
                    if prop not in allowed:
                        found.append("%s: %s property %s is not allowed" % (name, label, prop))
                    if prop == "cast_shadows" and value:
                        found.append("%s: %s casts shadows" % (name, label))
            for label, recorder in (("sun", sun), ("sky", sky)):
                if ("cast_shadows", False) not in recorder.calls:
                    found.append("%s: %s does not switch shadows off" % (name, label))
            if table.get(name) is None and (sun.calls != [("cast_shadows", False)] or sky.calls != [("cast_shadows", False)] or fog.calls):
                found.append("%s: no ambience, yet something besides the shadows is set" % name)
    finally:
        ambience.AMBIENCES = saved
    return found


def mutants(table, name):
    """(label, table) for each way an ambience can be wrong; each one must be caught."""
    out = []
    shadow = copy.deepcopy(table)
    out.append(("shadows on", shadow, lambda n, sun, sky, fog: (ambience.apply(n, sun, sky, fog), sun.set_editor_property("cast_shadows", True))))
    for label, key, value in (("sun too strong", "sun_intensity", 30.0), ("sun too steep", "sun_pitch", -85.0), ("fog too thick", "fog_density", 0.1),
                              ("sunset sun", "sun_color", (1.0, 0.7, 0.4)), ("violet sky", "sky_tint", (0.9, 0.7, 1.0)), ("black fog", "fog_color", (0.0, 0.0, 0.0))):
        bad = copy.deepcopy(table)
        bad[name][key] = value
        out.append((label, bad, ambience.apply))
    missing = copy.deepcopy(table)
    del missing[name]
    out.append(("entry missing", missing, ambience.apply))
    short = copy.deepcopy(table)
    del short[name]["fog_start"]
    out.append(("key missing", short, ambience.apply))

    def extra(n, sun, sky, fog):
        ambience.apply(n, sun, sky, fog)
        fog.set_editor_property("volumetric_fog", True)
    out.append(("property outside the list", copy.deepcopy(table), extra))
    return out


def main():
    names = list(common.read_maps())
    failed = 0
    found = problems_of(ambience.AMBIENCES, names)
    for line in found:
        print("  " + line)
    failed += len(found)
    print("%d maps (%s): %s" % (len(names), ", ".join(names), "all fine" if not found else "%d problem(s)" % len(found)))
    if ambience.AMBIENCES.get("Plain", "missing") is not None:
        print("  Plain is not the reference (None)")
        failed += 1
    if ambience.rotation("NoSuchMap") != (ambience.DEFAULT_PITCH, ambience.DEFAULT_YAW):
        print("  a map without an entry does not keep the default sun angle")
        failed += 1
    # The check must catch each wrong variant (so that it cannot pass for nothing).
    name = next(name for name in names if ambience.AMBIENCES.get(name) is not None)
    for label, table, apply in mutants(ambience.AMBIENCES, name):
        if not problems_of(table, names, apply):
            print("  NOT CAUGHT: %s" % label)
            failed += 1
    print("ambience_test : %s" % ("PASS" if not failed else "FAIL (%d)" % failed))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
