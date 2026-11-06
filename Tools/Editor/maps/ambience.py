"""The light of each map (chantier Cartes, ambiance lumineuse, 2026-11-03). Inside D258 and D265: NO shadow in the game, matte materials, no
reflection, NO post-processing (no volume, no grading, no bloom, fixed exposure, no ambient occlusion). A map's ambience is only the
colour, the strength and the angle of its sun, the tint and the strength of its sky light, and a faint height fog: lights without shadows
add no shadow pass, the fog is the engine's own (drawn with the scene, before the fog of war's post process M_FogPost). Nothing else is
touched (apply() sets an allow-list of properties, tests/ambience_test.py checks it).

AMBIENCES maps a map's name (a line of Tools/Data/Maps.csv) to its ambience, or to None: the engine's own defaults, left alone (Plain, the
reference; also any map without an entry, so that a new map behaves as before). An ambience is a dict:
  sun_color      (r, g, b) the sun light's colour, 0 to 1 as seen (sRGB, the engine's FColor), white = (1, 1, 1)
  sun_intensity  lux (the engine's default is 10)
  sun_pitch      degrees, negative = looking down (the default is -50): never steeper than -70, so that the faces of a volume stay lit differently (D265)
  sun_yaw        degrees, kept within -30..80 of the default 30: seen from the game's camera the faces turned away from the sun (yaw 200, tried 2026-11-03) went near black
  sky_intensity  the sky light's strength (the default is 1)
  sky_tint       (r, g, b) the sky light's colour, as sun_color
  fog_color      (r, g, b) the fog's inscattering luminance, linear (the engine's default is (0.447, 0.638, 1.0), a blue)
  fog_density    the height fog's density (the engine's default is 0.02)
  fog_start      cm: the distance from the camera where the fog starts
BOUNDS are the moderate limits the test holds every ambience to (priority: the team colours, the selection circles, the life bars, the
foundation and the resources stay readable; no sunset that turns the blue into violet).
"""
import unreal

DEFAULT_PITCH = -50.0
DEFAULT_YAW = 30.0

# name -> ambience (see above) or None = the engine's defaults, untouched.
AMBIENCES = {
    # Plain: the reference, as before this chantier.
    "Plain": None,
    # Steppe: the engine's defaults, like Plain (2026-11-03: the user did NOT validate the golden ambience, which came out greener than
    # golden; only the Hinterland's is validated). The values tried, kept to try again:
    #   dict(sun_color=(1.0, 0.95, 0.86), sun_intensity=10.5, sun_pitch=-52.0, sun_yaw=30.0, sky_intensity=1.0, sky_tint=(1.0, 0.98, 0.92),
    #        fog_color=(0.70, 0.64, 0.42), fog_density=0.008, fog_start=2500.0)
    "Steppe": None,
    # Hinterland: a cooler, green-grey light, lower and weaker; a slightly denser haze.
    "Hinterland": dict(sun_color=(0.90, 0.96, 0.94), sun_intensity=8.5, sun_pitch=-45.0, sun_yaw=55.0,
                       sky_intensity=0.95, sky_tint=(0.90, 0.97, 0.94),
                       fog_color=(0.46, 0.58, 0.52), fog_density=0.014, fog_start=1500.0),
    # Forest: darker and green; a soft haze.
    "Forest": dict(sun_color=(0.88, 0.96, 0.86), sun_intensity=7.5, sun_pitch=-42.0, sun_yaw=15.0,
                   sky_intensity=0.9, sky_tint=(0.88, 0.96, 0.88),
                   fog_color=(0.42, 0.58, 0.42), fog_density=0.016, fog_start=1500.0),
    # El Dorado: a warm, dry light, high; a very light sandy haze.
    "ElDorado": dict(sun_color=(1.0, 0.93, 0.84), sun_intensity=11.0, sun_pitch=-58.0, sun_yaw=-10.0,
                     sky_intensity=1.0, sky_tint=(1.0, 0.96, 0.88),
                     fog_color=(0.78, 0.64, 0.42), fog_density=0.006, fog_start=3000.0),
    # Cols: cold and clear, a blue sky; a pale blue haze.
    "Cols": dict(sun_color=(0.92, 0.96, 1.0), sun_intensity=10.5, sun_pitch=-48.0, sun_yaw=60.0,
                 sky_intensity=1.1, sky_tint=(0.90, 0.96, 1.0),
                 fog_color=(0.62, 0.74, 0.94), fog_density=0.010, fog_start=2000.0),
    # Fords: neutral and clear.
    "Fords": dict(sun_color=(1.0, 1.0, 0.97), sun_intensity=10.0, sun_pitch=-50.0, sun_yaw=45.0,
                  sky_intensity=1.0, sky_tint=(0.97, 1.0, 1.0),
                  fog_color=(0.58, 0.70, 0.88), fog_density=0.012, fog_start=2000.0),
    # Crown: neutral, slightly golden.
    "Crown": dict(sun_color=(1.0, 0.97, 0.90), sun_intensity=10.0, sun_pitch=-50.0, sun_yaw=20.0,
                  sky_intensity=1.0, sky_tint=(1.0, 0.98, 0.94),
                  fog_color=(0.68, 0.64, 0.50), fog_density=0.010, fog_start=2000.0),
}

# The moderate limits (the test): name -> (low, high) for a number, and for a colour (low, high, widest ratio between its channels).
BOUNDS = {
    "sun_color": (0.84, 1.0, 1.20),
    "sky_tint": (0.84, 1.0, 1.20),
    "sun_intensity": (7.0, 12.0),
    "sun_pitch": (-70.0, -35.0),
    "sun_yaw": (-30.0, 80.0),
    "sky_intensity": (0.85, 1.3),
    "fog_color": (0.30, 1.0, 2.0),
    "fog_density": (0.004, 0.025),
    "fog_start": (0.0, 4000.0),
}
KEYS = tuple(BOUNDS)

# The only properties apply() sets (the test refuses any other one, and any shadow switched on).
SUN_PROPERTIES = ("light_color", "intensity", "cast_shadows")
SKY_PROPERTIES = ("light_color", "intensity", "cast_shadows")
FOG_PROPERTIES = ("fog_inscattering_luminance", "fog_density", "start_distance")


def for_map(name):
    """The ambience of a map, None for the engine's defaults (Plain, and every map without an entry)."""
    return AMBIENCES.get(name)


def rotation(name):
    """(pitch, yaw) of the map's sun."""
    ambience = for_map(name)
    return (DEFAULT_PITCH, DEFAULT_YAW) if ambience is None else (ambience["sun_pitch"], ambience["sun_yaw"])


def as_color(rgb):
    return unreal.Color(round(rgb[0] * 255), round(rgb[1] * 255), round(rgb[2] * 255), 255)


def apply(name, sun, sky, fog):
    """Sets the map's ambience on the sun's and the sky's light components and on the height fog's component (the sun's rotation is
    rotation(name), given at its spawn). Shadows stay off, always. A map without an ambience keeps the engine's defaults."""
    sun.set_editor_property("cast_shadows", False)
    sky.set_editor_property("cast_shadows", False)
    ambience = for_map(name)
    if ambience is None:
        return
    sun.set_editor_property("light_color", as_color(ambience["sun_color"]))
    sun.set_editor_property("intensity", ambience["sun_intensity"])
    sky.set_editor_property("light_color", as_color(ambience["sky_tint"]))
    sky.set_editor_property("intensity", ambience["sky_intensity"])
    red, green, blue = ambience["fog_color"]
    fog.set_editor_property("fog_inscattering_luminance", unreal.LinearColor(red, green, blue, 1.0))
    fog.set_editor_property("fog_density", ambience["fog_density"])
    fog.set_editor_property("start_distance", ambience["fog_start"])
