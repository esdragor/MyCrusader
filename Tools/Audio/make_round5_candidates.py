"""Round 5 of the Barracks, Range and Stable building sounds (D801, 2026-11-15): RECORDED free samples, not synthesis.

The synthesised round 4 was refused by ear (Barracks voices distorted, Range "spaceship shots / random notes", Stable too arcade but keep the
horse). This script cuts each candidate out of a recorded, free-licensed file, then makes it obey the soft rule of check_audio_soft.py
(<= 0.80 s, centroid <= 800 Hz, <= 1 % of the energy above 3 kHz, peak -12..-2 dB, >= -18 dB above 200 Hz): high-pass 120 Hz, a low-pass
lowered step by step until the clip is low enough, a gentle tanh drive until it is loud enough above 200 Hz, peak -2.5 dB, short fades.
Nothing is chosen here: the ear decides (the agent never hears the sounds).

Sources (downloaded once into <HOL_AUDIO_ROOT>\\Sources\\round5, outside the repository; licence of each in SOURCES below):
  python make_round5_candidates.py [out_dir] [stem_prefix]
Output: 16-bit mono 48 kHz WAV <out_dir>\\<stem>.wav (default MyCrusader_Audio\\Candidates\\soft). Needs ffmpeg on PATH (decoding).
"""
import os
import subprocess
import sys
import wave

import numpy as np

AUDIO = os.environ.get("HOL_AUDIO_ROOT", r"C:\Users\baudo\Documents\MyCrusader_Audio")
SOURCE_DIR = os.path.join(AUDIO, "Sources", "round5")
RATE = 48000
PEAK_DB = -2.5
TARGET_AUDIBLE_DB = -16.0   # margin over the -18 dB floor of check_audio_soft
MAX_CENTROID = 740.0        # margin under 800 Hz
MAX_HIGH = 0.006            # margin under 1 %

# file -> (licence, author, where it comes from)
SOURCES = {
    "yell_1yell3.wav": ("CC0 (dual OGA-BY 3.0)", "HaelDB", "https://opengameart.org/content/male-gruntyelling-sounds (1yell3.wav)"),
    "yell_2yell6.wav": ("CC0 (dual OGA-BY 3.0)", "HaelDB", "https://opengameart.org/content/male-gruntyelling-sounds (2yell6.wav)"),
    "yell_3yell1.wav": ("CC0 (dual OGA-BY 3.0)", "HaelDB", "https://opengameart.org/content/male-gruntyelling-sounds (3yell1.wav)"),
    "yell_yell5.wav": ("CC0 (dual OGA-BY 3.0)", "HaelDB", "https://opengameart.org/content/male-gruntyelling-sounds (yell5.wav)"),
    "grunt_3grunt1.wav": ("CC0 (dual OGA-BY 3.0)", "HaelDB", "https://opengameart.org/content/male-gruntyelling-sounds (3grunt1.wav)"),
    "grunt_3grunt6.wav": ("CC0 (dual OGA-BY 3.0)", "HaelDB", "https://opengameart.org/content/male-gruntyelling-sounds (3grunt6.wav)"),
    "crowd_shouting.ogg": ("CC0", "StarNinjas", "https://opengameart.org/content/crowd-shoutingspeaking-ambience"),
    "sabre_draw_fast.wav": ("CC0", "StillNorthMedia / MedicineStorm", "https://opengameart.org/content/medieval-sound-effects-weapon-textures (Sabre Draw Fast)"),
    "seax_sheath_fast.wav": ("CC0", "StillNorthMedia / MedicineStorm", "https://opengameart.org/content/medieval-sound-effects-weapon-textures (Seax Sheath Fast)"),
    "longbow_shoot.wav": ("CC0", "StillNorthMedia / MedicineStorm", "https://opengameart.org/content/medieval-sound-effects-weapon-textures (English Longbow Shoot)"),
    "longbow_set_arrow.wav": ("CC0", "StillNorthMedia / MedicineStorm", "https://opengameart.org/content/medieval-sound-effects-weapon-textures (English Longbow Set Arrow)"),
    "longbow_nock_arrow.wav": ("CC0", "StillNorthMedia / MedicineStorm", "https://opengameart.org/content/medieval-sound-effects-weapon-textures (English Longbow Nock Arrow)"),
    "crossbow_shoot.wav": ("CC0", "StillNorthMedia / MedicineStorm", "https://opengameart.org/content/medieval-sound-effects-weapon-textures (Crossbow Shoot)"),
    "crossbow_bolt_passby.wav": ("CC0", "StillNorthMedia / MedicineStorm", "https://opengameart.org/content/medieval-sound-effects-weapon-textures (Crossbow Bolt Passby)"),
    "bow_ogrebane.wav": ("CC0 (multi-licence page, CC0 among them)", "Ogrebane", "https://opengameart.org/content/battle-sound-effects (Bow.wav)"),
    "archers_shooting.flac": ("CC-BY 3.0 (credit: copyc4t)", "copyc4t", "https://opengameart.org/content/archers-shooting"),
    "crossbow_shot_hit.wav": ("CC-BY 3.0 (credit: spookymodem)", "spookymodem", "https://opengameart.org/content/crossbow-shot"),
    "rl_whinnies.flac": ("CC0 as marked by archive.org (USC Cinema / Sunset Editorial collection, uploaded by archive.org staff)", "Red Library",
                         "https://archive.org/details/Red_Library_Animals_Horses_1 (R12-55 Small Horse Whinnies)"),
    "rl_snorting.flac": ("CC0 as marked by archive.org (same collection)", "Red Library", "https://archive.org/details/Red_Library_Animals_Horses_1 (R12-57 Horses Snorting)"),
    "rl_snort_vocal.flac": ("CC0 as marked by archive.org (same collection)", "Red Library", "https://archive.org/details/Red_Library_Animals_Horses_1 (R12-60 Horses Snort and Vocalize)"),
    "rl_walk.flac": ("CC0 as marked by archive.org (same collection)", "Red Library", "https://archive.org/details/Red_Library_Animals_Horses_1 (R04-54 Horses Walk on Hard Floor)"),
    "trot.ogg": ("CC0", "EZduzziteh", "https://opengameart.org/content/horse-trotting"),
}

# stem -> (source file, which event: 0 = loudest, 1 = second loudest..., seconds kept, first low-pass in Hz)
RECIPES = {
    "s_voice5_yell_a": ("yell_1yell3.wav", 0, 0.75, 1800),
    "s_voice5_yell_b": ("yell_2yell6.wav", 0, 0.75, 1800),
    "s_voice5_yell_c": ("yell_3yell1.wav", 0, 0.75, 1800),
    "s_voice5_yell_d": ("yell_yell5.wav", 0, 0.75, 1800),
    "s_voice5_grunt_a": ("grunt_3grunt1.wav", 0, 0.60, 1800),
    "s_voice5_grunt_b": ("grunt_3grunt6.wav", 0, 0.70, 1800),
    "s_voice5_crowd_a": ("crowd_shouting.ogg", 0, 0.78, 1800),
    "s_voice5_crowd_b": ("crowd_shouting.ogg", 1, 0.78, 1800),
    "s_voice5_sword_draw": ("sabre_draw_fast.wav", 0, 0.70, 1600),
    "s_voice5_sword_sheath": ("seax_sheath_fast.wav", 0, 0.70, 1600),
    "s_shot5_longbow_a": ("longbow_shoot.wav", 0, 0.70, 1800),
    "s_shot5_longbow_b": ("longbow_shoot.wav", 1, 0.70, 1800),
    "s_shot5_longbow_set": ("longbow_set_arrow.wav", 0, 0.60, 1800),
    "s_shot5_nock": ("longbow_nock_arrow.wav", 0, 0.60, 1800),
    "s_shot5_crossbow_a": ("crossbow_shoot.wav", 0, 0.70, 1800),
    "s_shot5_crossbow_b": ("crossbow_shoot.wav", 1, 0.70, 1800),
    "s_shot5_bow": ("bow_ogrebane.wav", 0, 0.70, 1800),
    "s_shot5_volley_a": ("archers_shooting.flac", 0, 0.78, 1800),
    "s_shot5_arrow_pass": ("crossbow_bolt_passby.wav", 0, 0.70, 1800),
    "s_shot5_bolt_hit": ("crossbow_shot_hit.wav", 0, 0.75, 1800),
    "s_horse5_whinny_a": ("rl_whinnies.flac", 0, 0.78, 1800),
    "s_horse5_whinny_b": ("rl_whinnies.flac", 1, 0.78, 1800),
    "s_horse5_whinny_c": ("rl_whinnies.flac", 2, 0.78, 1800),
    "s_horse5_snort_a": ("rl_snorting.flac", 0, 0.70, 1600),
    "s_horse5_snort_b": ("rl_snorting.flac", 1, 0.70, 1600),
    "s_horse5_vocal_a": ("rl_snort_vocal.flac", 0, 0.78, 1800),
    "s_horse5_vocal_b": ("rl_snort_vocal.flac", 1, 0.78, 1800),
    "s_horse5_walk": ("rl_walk.flac", 0, 0.78, 1800),
    "s_horse5_trot_a": ("trot.ogg", 0, 0.78, 1800),
    "s_horse5_trot_b": ("trot.ogg", 1, 0.78, 1800),
}


def decode(path):
    raw = subprocess.run(["ffmpeg", "-v", "error", "-i", path, "-ac", "1", "-ar", str(RATE), "-f", "f32le", "-"],
                         check=True, capture_output=True).stdout
    return np.frombuffer(raw, dtype="<f4").astype(np.float64)


def events(data, seconds):
    """Starts of the loudest non-overlapping windows, loudest first (20 ms before the attack inside each window)."""
    hop = int(0.01 * RATE)
    length = int(seconds * RATE)
    frames = np.array([np.sqrt(np.mean(data[i:i + hop] ** 2)) for i in range(0, max(1, len(data) - hop), hop)])
    span = max(1, length // hop)
    score = np.convolve(frames ** 2, np.ones(span), mode="valid") if len(frames) > span else np.array([np.sum(frames ** 2)])
    found = []
    order = np.argsort(score)[::-1]
    for index in order:
        if all(abs(int(index) - other) >= span for other in found):
            found.append(int(index))
        if len(found) >= 4:
            break
    starts = []
    for index in found:
        window = frames[index:index + span]
        attack = index + int(np.argmax(window >= 0.3 * window.max())) if len(window) else index
        starts.append(max(0, attack * hop - int(0.02 * RATE)))
    return starts


def band(data, low, high):
    spectrum = np.fft.rfft(data)
    freqs = np.fft.rfftfreq(len(data), 1.0 / RATE)
    gain = 1.0 / np.sqrt(1.0 + (low / np.maximum(freqs, 1.0)) ** 4)       # 2nd order high-pass shape
    gain *= 1.0 / np.sqrt(1.0 + (freqs / high) ** 8)                       # 4th order low-pass shape
    return np.fft.irfft(spectrum * gain, len(data))


def fades(data):
    n = len(data)
    env = np.ones(n)
    a, r = int(0.004 * RATE), int(0.12 * RATE)
    env[:a] = np.linspace(0, 1, a)
    env[-r:] *= np.linspace(1, 0, r) ** 1.5
    return data * env


def measure(data):
    spectrum = np.abs(np.fft.rfft(data * np.hanning(len(data))))
    freqs = np.fft.rfftfreq(len(data), 1.0 / RATE)
    centroid = float((freqs * spectrum).sum() / max(1e-12, spectrum.sum()))
    high = float((spectrum[freqs > 3000] ** 2).sum() / max(1e-12, (spectrum ** 2).sum()))
    heard = np.fft.rfft(data)
    heard[freqs[:len(heard)] < 200.0] = 0
    heard = np.fft.irfft(heard, len(data))
    window = int(0.1 * RATE)
    power = np.convolve(heard ** 2, np.ones(window) / window, mode="valid")
    return centroid, high, 10 * np.log10(max(1e-20, float(power.max())))


def normalise(data):
    return data / max(1e-9, np.abs(data).max()) * 10 ** (PEAK_DB / 20)


def render(stem, recipe):
    source, which, seconds, cutoff = recipe
    data = decode(os.path.join(SOURCE_DIR, source))
    if which < 0:   # -1: the last `seconds` of a short file (a second take that the loudness search cannot separate)
        start = max(0, len(data) - int(seconds * RATE))
    else:
        starts = events(data, seconds)
        start = starts[min(which, len(starts) - 1)]
    clip = data[start:start + int(seconds * RATE)]
    clip = np.concatenate([clip, np.zeros(max(0, int(seconds * RATE) - len(clip)))])
    while True:
        # The drive adds harmonics, so the low-pass is applied again after it and the whole clip is re-measured at each step.
        filtered = band(clip, 120.0, cutoff)
        filtered /= max(1e-9, np.abs(filtered).max())
        drive = 1.0
        while True:
            shaped = normalise(fades(band(np.tanh(drive * filtered), 120.0, cutoff)))
            centroid, high, audible = measure(shaped)
            if audible >= TARGET_AUDIBLE_DB or drive >= 6.0:
                break
            drive *= 1.25
        if (centroid <= MAX_CENTROID and high <= MAX_HIGH) or cutoff <= 400:
            break
        cutoff *= 0.9
    return shaped, cutoff, drive, centroid, high, audible, start / RATE


def write(path, data):
    pcm = (np.clip(data, -1, 1) * 32767).astype("<i2")
    with wave.open(path, "wb") as target:
        target.setnchannels(1)
        target.setsampwidth(2)
        target.setframerate(RATE)
        target.writeframes(pcm.tobytes())


def main():
    out_dir = sys.argv[1] if len(sys.argv) > 1 else os.path.join(AUDIO, "Candidates", "soft")
    prefix = sys.argv[2] if len(sys.argv) > 2 else ""
    os.makedirs(out_dir, exist_ok=True)
    for stem, recipe in RECIPES.items():
        if not stem.startswith(prefix):
            continue
        data, cutoff, drive, centroid, high, audible, at = render(stem, recipe)
        write(os.path.join(out_dir, stem + ".wav"), data)
        print("%-22s from %-22s at %6.2f s  low-pass %4.0f Hz  drive %.2f  centroid %3.0f Hz  >3k %.2f %%  above 200 Hz %5.1f dB"
              % (stem, recipe[0], at, cutoff, drive, centroid, high * 100, audible))


if __name__ == "__main__":
    main()
