"""B1 (BiomeInProgress.md): the light limits of the biomes' script rule (Script/Biomes/HOLBiomeRules.as, HOLBiome::LightsInBounds) are the
SAME as the limits Tools/Editor/maps/ambience.py holds every baked ambience to (BOUNDS), converted to the script's whole-number units
(colours and the sky's strength in per mille, the sun's strength in tenths of lux, the haze's density in 0.0001, angles and distances as they
are). Until now this equivalence was read by hand; this reads both and compares. The editor is not needed (stand-in `unreal`, maps_stub.py),
and nothing is written into Tools/Editor/maps.
  python Tools/Tests/biome_bounds_check.py      exit 0 and "PASS" if they agree, 1 and the differences if not
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import maps_stub  # noqa: E402

ROOT = maps_stub.install()
from maps import ambience  # noqa: E402

RULES = os.path.join(ROOT, "Script", "Biomes", "HOLBiomeRules.as")


def permille(value):
    return int(round(value * 1000))


def percent(value):
    # the widest ratio between a colour's channels, in percent (1.20 -> 120)
    return int(round(value * 100))


def expected_from_python():
    b = ambience.BOUNDS
    return {
        "sun_color": tuple(permille(v) for v in b["sun_color"][:2]) + (percent(b["sun_color"][2]),),
        "sky_tint": tuple(permille(v) for v in b["sky_tint"][:2]) + (percent(b["sky_tint"][2]),),
        "fog_color": tuple(permille(v) for v in b["fog_color"][:2]) + (percent(b["fog_color"][2]),),
        "sun_intensity": (int(round(b["sun_intensity"][0] * 10)), int(round(b["sun_intensity"][1] * 10))),
        "sun_pitch": (int(b["sun_pitch"][0]), int(b["sun_pitch"][1])),
        "sun_yaw": (int(b["sun_yaw"][0]), int(b["sun_yaw"][1])),
        "sky_intensity": (permille(b["sky_intensity"][0]), permille(b["sky_intensity"][1])),
        "fog_density": (int(round(b["fog_density"][0] * 10000)), int(round(b["fog_density"][1] * 10000))),
        "fog_start": (int(b["fog_start"][0]), int(b["fog_start"][1])),
    }


def found_in_script():
    text = open(RULES, encoding="utf-8").read()
    body = text[text.index("bool LightsInBounds"):]
    found = {}
    for key, name in (("sun_color", "Sun"), ("sky_tint", "Sky"), ("fog_color", "Fog")):
        m = re.search(r"ChannelsInBounds\(B\.%sR, B\.%sG, B\.%sB, (\d+), (\d+), (\d+)\)" % (name, name, name), body)
        found[key] = tuple(int(g) for g in m.groups()) if m else None
    for key, name in (("sun_intensity", "SunIntensityTenths"), ("sun_pitch", "SunPitch"), ("sun_yaw", "SunYaw"),
                      ("sky_intensity", "SkyIntensityPermille"), ("fog_density", "FogDensity"), ("fog_start", "FogStartCm")):
        m = re.search(r"B\.%s >= (-?\d+) && B\.%s <= (-?\d+)" % (name, name), body)
        found[key] = tuple(int(g) for g in m.groups()) if m else None
    return found


def main():
    expected = expected_from_python()
    found = found_in_script()
    bad = [(k, expected[k], found.get(k)) for k in expected if expected[k] != found.get(k)]
    if bad:
        for key, want, got in bad:
            print("DIFFERS %s: ambience.py says %s, HOLBiomeRules.as says %s" % (key, want, got))
        print("FAIL")
        return 1
    print("%d limits compared, all equal" % len(expected))
    print("PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
