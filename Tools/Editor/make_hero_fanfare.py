"""Makes the hero's level-up sounds, two orchestral fanfares (J8b.5, the trial's wish: « ça doit être épique », the
synthesised ones sounding like « un jeu des années 2000 »):
  /Game/Audio/SW_HeroLevelUp   a trumpet call landing on a D major chord of brass, strings, timpani and cymbal
  /Game/Audio/SW_HeroTier      a rising call over a timpani roll and a cymbal swell, then a heroic cadence (D, Bb, C, D)
                               of the whole orchestra with crashes and a gong, when a tier is reached (levels 3 and 6)
Both of class SC_Effects, played by the owner's HUD with the level alerts.

Played from recorded instruments: VSCO 2 Community Edition (CC0, Tools/Audio/vsco_samples.py, fetched once into
<project>/Saved/Audio/VSCO), each note the nearest sample moved to its pitch, sections thickened by detuned copies,
placed left to right as on a stage, then a hall (a convolution) and a soft limiter. Provisional until the game's own
sounds come; the alerts keep their ping (FHOLForumAlert::SoundPath).
Two steps:
  python Tools/Editor/make_hero_fanfare.py            renders <project>/Saved/Audio/SW_Hero*.wav (needs numpy)
  UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file> -unattended -nop4 -nosplash
                                                      imports them (re-running replaces the assets)
"""
import os
import sys

FOLDER = "/Game/Audio"
SOUNDS = ["SW_HeroLevelUp", "SW_HeroTier", "SW_HeroCry", "SW_HeroBanner"]
PROJECT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
SAVED = os.path.join(PROJECT, "Saved", "Audio")

NAMES = {"C": 0, "C#": 1, "Db": 1, "D": 2, "D#": 3, "Eb": 3, "E": 4, "F": 5, "F#": 6, "Gb": 6, "G": 7, "G#": 8,
         "Ab": 8, "A": 9, "A#": 10, "Bb": 10, "B": 11}


def hz(note):
    """« A4 » -> 440."""
    name, octave = note[:-1], int(note[-1])
    return 440.0 * 2.0 ** ((NAMES[name] + 12 * (octave + 1) - 69) / 12.0)


def chord(start, length, velocity, parts):
    """parts: {instrument: [notes]} -> score lines."""
    return [(start, length, note, instrument, velocity) for instrument, notes in parts.items() for note in notes]


def harmony(start, length, velocity, top, brass, low, strings, basses):
    return chord(start, length, velocity, {
        "trumpet": top, "horn": brass, "trombone": low[:1], "tuba": low[1:],
        "violins": strings[-2:], "violas": strings[1:-2], "cellos": strings[:1], "basses": basses})


# Scores: (start s, length s, note, instrument, velocity); percussion notes name the drum (1 lowest to 5).
LEVEL = (
    chord(0.0, 0.13, 0.8, {"trumpet_stac": ["A4", "D4"]}) + chord(0.15, 0.13, 0.85, {"trumpet_stac": ["A4", "D4"]})
    + chord(0.3, 0.13, 0.9, {"trumpet_stac": ["A4", "D4"]}) + [(0.3, 0.0, "2", "timpani", 0.5)]
    + harmony(0.45, 1.3, 1.0, ["D5", "A4"], ["D3", "F#3", "A3", "D4"], ["D3", "D2"],
              ["D3", "A3", "F#4", "A4", "D5", "A5"], ["D2"])
    + [(0.45, 0.0, "1", "timpani", 1.0), (0.45, 0.0, "", "crash", 0.7)]
)

TIER = (
    # The call rising (A, D, F#) in thirds over a timpani roll and a cymbal swell landing on the arrival.
    chord(0.0, 0.26, 0.8, {"trumpet": ["A4", "F#4"], "horn": ["A3"]})
    + chord(0.3, 0.26, 0.9, {"trumpet": ["D5", "A4"], "horn": ["D4"]})
    + chord(0.6, 0.26, 1.0, {"trumpet": ["F#5", "D5"], "horn": ["F#3", "D4"]})
    + [(0.0, 0.9, "1", "timpani_roll", 0.8), (0.9, 0.0, "", "swell", 0.8)]
    # D: the arrival.
    + harmony(0.9, 0.7, 1.0, ["D5", "A4"], ["D3", "F#3", "A3", "D4"], ["D3", "D2"], ["D3", "A3", "F#4", "A4", "D5", "F#5"], ["D2"])
    + [(0.9, 0.0, "1", "timpani", 1.0), (0.9, 0.0, "", "crash", 0.8)]
    # Bb, then C: the climb.
    + harmony(1.6, 0.4, 0.95, ["D5", "Bb4"], ["D3", "F3", "Bb3", "D4"], ["Bb2", "Bb1"], ["Bb2", "F3", "D4", "Bb4", "D5", "F5"], ["Bb1"])
    + [(1.6, 0.0, "2", "timpani", 0.8)]
    + harmony(2.0, 0.4, 1.0, ["E5", "C5"], ["C3", "E3", "G3", "C4"], ["C3", "C2"], ["C3", "G3", "E4", "C5", "E5", "G5"], ["C2"])
    + [(2.0, 0.0, "3", "timpani", 0.85)]
    # D: the triumph, on its third.
    + harmony(2.4, 1.3, 1.0, ["F#5", "D5", "A4"], ["D3", "F#3", "A3", "D4"], ["D3", "D2"],
              ["D3", "A3", "F#4", "A4", "D5", "F#5"], ["D2"])
    + [(2.4, 0.0, "1", "timpani", 1.0), (2.58, 0.0, "2", "timpani", 0.7), (2.4, 0.0, "", "crash", 0.9),
       (2.4, 0.0, "", "gong", 0.6)]
)

# J8c.4 loop 5: the cry « Serrez les rangs » (a rallying call of low brass, a fifth leaping up, timpani and a crash) and the
# banner's planting (a bright trumpet call over a timpani thud and a gong), both played to every player.
CRY = (
    chord(0.0, 0.32, 0.95, {"horn": ["D3", "A3"], "trombone": ["D3"], "tuba": ["D2"]}) + [(0.0, 0.0, "1", "timpani", 1.0)]
    + chord(0.38, 0.7, 1.0, {"trumpet": ["A4", "D5"], "horn": ["A3", "D4"], "trombone": ["A2"], "tuba": ["A1"]})
    + [(0.38, 0.0, "1", "timpani", 1.0), (0.38, 0.0, "", "crash", 0.6)]
)
BANNER = (
    chord(0.0, 0.14, 0.8, {"trumpet_stac": ["D5"]}) + [(0.0, 0.0, "2", "timpani", 0.8)]
    + chord(0.18, 0.9, 1.0, {"trumpet": ["A4", "F#5"], "horn": ["D4"], "violins": ["D5", "A5"]})
    + [(0.18, 0.0, "1", "timpani", 0.9), (0.18, 0.0, "", "gong", 0.5)]
)

# Name -> (score, time scale, longest sound s): the tier's played twice as fast (the trial: « temps divisé par 2 »).
SCORES = {"SW_HeroLevelUp": (LEVEL, 1.0, 4.2), "SW_HeroTier": (TIER, 0.5, 3.5), "SW_HeroCry": (CRY, 1.0, 2.4),
          "SW_HeroBanner": (BANNER, 1.0, 2.4)}

# Instrument -> (loudness, pan from -1 left to 1 right, release s, detuned copies in cents for a section).
MIX = {
    "trumpet": (0.9, 0.15, 0.25, [-6.0, 7.0]),
    "trumpet_stac": (0.9, 0.15, 0.06, [-6.0, 7.0]),
    "horn": (0.75, -0.35, 0.3, [-7.0, 5.0]),
    "trombone": (0.7, 0.45, 0.3, [-5.0, 6.0]),
    "tuba": (0.65, 0.3, 0.3, []),
    "violins": (0.4, -0.55, 0.45, [-8.0, 8.0]),
    "violas": (0.35, -0.15, 0.45, [-8.0, 8.0]),
    "cellos": (0.45, 0.4, 0.45, [-6.0, 6.0]),
    "basses": (0.45, 0.6, 0.45, [-5.0]),
    "timpani": (1.0, -0.1, 0.0, []),
    "timpani_roll": (0.8, -0.1, 0.15, []),
    "crash": (0.55, 0.35, 0.0, []),
    "swell": (0.5, 0.35, 0.0, []),
    "gong": (0.6, -0.4, 0.0, []),
}
PERCUSSION = {"timpani", "timpani_roll", "crash", "swell", "gong"}


def render(name, bank, np, rate):
    score, scale, longest = SCORES[name]
    score = [(start * scale, note_length * scale, note, instrument, velocity)
             for start, note_length, note, instrument, velocity in score]
    length = longest
    out = np.zeros((int(length * rate), 2))
    rng = np.random.default_rng(len(name))

    def place(data, start, gain, pan):
        first = int(start * rate)
        if first < 0:
            data, first = data[-first:], 0
        data = data[:len(out) - first]
        out[first:first + len(data), 0] += data * gain * np.sqrt(0.5 * (1.0 - pan))
        out[first:first + len(data), 1] += data * gain * np.sqrt(0.5 * (1.0 + pan))

    for start, note_length, note, instrument, velocity in score:
        loudness, pan, release, copies = MIX[instrument]
        samples = bank[instrument]
        gain = loudness * velocity ** 1.5
        if instrument in PERCUSSION:
            sample = samples[min(int(note or 1), len(samples)) - 1].data
            if instrument == "swell":
                # Its loudest point lands on the given time.
                start -= int(np.argmax(np.abs(sample))) / rate
            if instrument == "timpani_roll":
                sample = sample[:int(note_length * rate)] * np.linspace(0.25, 1.0, int(note_length * rate)) ** 2
            place(sample, start, gain, pan)
            continue
        target = hz(note)
        sample = min(samples, key=lambda s: abs(np.log2(target / s.pitch)))
        for cents in [0.0] + copies:
            ratio = target / sample.pitch * 2.0 ** (cents / 1200.0)
            count = int((note_length + release) * rate)
            positions = np.arange(count) * ratio
            positions = positions[positions < len(sample.data) - 1]
            data = np.interp(positions, np.arange(len(sample.data)), sample.data)
            held = int(note_length * rate)
            if len(data) > held:
                fade = len(data) - held
                data[held:] *= np.linspace(1.0, 0.0, fade) ** 2
            # Copies a few ms late, to either side: a section, not one player.
            offset = 0.0 if cents == 0.0 else rng.uniform(0.008, 0.025)
            side = 0.0 if cents == 0.0 else 0.2 * np.sign(cents)
            place(data, start + offset, gain / (1.0 + 0.6 * len(copies)), np.clip(pan + side, -1.0, 1.0))
    return out


def hall(dry, np, rate, seconds=2.6, wet=0.32):
    """Convolution with a made-up hall: two decorrelated decaying noises, darker as they fade, 25 ms late."""
    rng = np.random.default_rng(11)
    count = int(seconds * rate)
    t = np.arange(count) / rate
    decay = np.exp(-6.9 * t / seconds)
    responses = []
    for _ in range(2):
        noise = rng.standard_normal(count) * decay
        # Darker with time: blend toward a smoothed copy.
        smooth = np.convolve(noise, np.ones(24) / 24.0, mode="same")
        blend = np.clip(t / seconds * 1.5, 0.0, 1.0)
        response = noise * (1.0 - blend) + smooth * blend
        response = np.concatenate([np.zeros(int(0.025 * rate)), response])
        responses.append(response / np.sqrt(np.sum(response ** 2)))
    size = 1 << int(np.ceil(np.log2(len(dry) + len(responses[0]))))
    out = np.zeros((len(dry) + int(seconds * rate), 2))
    for channel in range(2):
        conv = np.fft.irfft(np.fft.rfft(dry[:, channel], size) * np.fft.rfft(responses[channel], size), size)
        out[:len(dry), channel] += dry[:, channel]
        out[:, channel] += wet * conv[:len(out)]
    return out


def write(path, sound, np, rate, longest):
    import wave
    sound = sound[:int(longest * rate)]
    peak = np.max(np.abs(sound)) or 1.0
    # A soft limiter: louder without clipping.
    sound = np.tanh(1.8 * sound / peak) / np.tanh(1.8) * 0.89
    fade = int(0.4 * rate)
    sound[-fade:] *= np.linspace(1.0, 0.0, fade)[:, None]
    # Trim the tail once it is inaudible (-60 dB).
    level = np.max(np.abs(sound), axis=1)
    end = len(level) - int(np.argmax(level[::-1] > 0.001)) + int(0.05 * rate)
    sound = sound[:min(end, len(sound))]
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with wave.open(path, "wb") as file:
        file.setnchannels(2)
        file.setsampwidth(2)
        file.setframerate(rate)
        file.writeframes((np.clip(sound, -1.0, 1.0) * 32767).astype("<i2").tobytes())
    return len(sound) / rate


def render_all():
    import numpy as np
    sys.path.insert(0, os.path.join(PROJECT, "Tools", "Audio"))
    import vsco_samples
    bank = vsco_samples.load(os.path.join(SAVED, "VSCO"))
    # Every sample to the same loudness, its first second's.
    for samples in bank.values():
        for sample in samples:
            sample.data = sample.data / (np.sqrt(np.mean(sample.data[:vsco_samples.RATE] ** 2)) * 8.0 + 1e-9)
    for name in SOUNDS:
        path = os.path.join(SAVED, name + ".wav")
        seconds = write(path, hall(render(name, bank, np, vsco_samples.RATE), np, vsco_samples.RATE), np, vsco_samples.RATE,
                        SCORES[name][2])
        print("Wrote %s (%.2f s)" % (path, seconds))


def import_all(unreal):
    for name in SOUNDS:
        path = os.path.join(SAVED, name + ".wav")
        if not os.path.exists(path):
            raise RuntimeError("%s missing: run python %s first" % (path, os.path.abspath(__file__)))
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", path)
        task.set_editor_property("destination_path", FOLDER)
        task.set_editor_property("destination_name", name)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", False)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        asset = FOLDER + "/" + name
        sound = unreal.load_asset(asset)
        if not isinstance(sound, unreal.SoundWave):
            raise RuntimeError("Could not import %s as a sound wave (got %s)" % (path, sound))
        sound.set_editor_property("sound_class_object", unreal.load_asset(FOLDER + "/SC_Effects"))
        unreal.EditorAssetLibrary.save_loaded_asset(sound)
        unreal.log("Hero fanfare: %s from %s" % (asset, path))


try:
    import unreal
except ImportError:
    render_all()
else:
    import_all(unreal)
