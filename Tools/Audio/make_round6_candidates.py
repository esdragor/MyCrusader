"""Round 6 of the Range and Stable selection sounds (D826, 2026-11-16): a CLEAN version of the sound the user elected in round 5.

The user kept the kind of sound of letter G (Range: s_shot5_bow, a bow shot; Stable: s_horse5_vocal_b, a snort and a horse's voice) but
heard "a poor recording". Round 5 (make_round5_candidates.py) cut every clip to 0.60-0.80 s (the horse was cut before its voice ended),
drove it through tanh (distortion) and lowered a low-pass step by step until the centroid was under 740 Hz (the bow ended near 750 Hz).
Round 6 takes the SAME event of the SAME source file and treats it gently:

  - the clip runs from the attack until the sound has died away (40 dB under its loudest 10 ms for 120 ms), at most 2 s, never padded;
  - no drive at all (a gain to a -2.5 dB peak; a transient too faint above 200 Hz has only its first click turned down by a transparent
    peak limiter), a 80 Hz high-pass against rumble, 4 ms fade in, 40 ms fade out;
  - a SWEEP of the low-pass (none, 6 kHz, 3 kHz, 1.5 kHz): the ear chooses how soft, the agent never hears the sounds.

Barracks (lot A2): the user kept NO letter of round 5 (pain cries, poor quality): ten NEW sounds without any voice (BARRACKS below),
blades, a war drum and a horn, layered by a few, treated the same clean way (high-pass 80 Hz, no low-pass, no drive, soft release).

  python make_round6_candidates.py [out_dir]
Output: 16-bit mono 48 kHz WAV <out_dir>\\<stem>.wav (default MyCrusader_Audio\\Candidates\\soft). Needs ffmpeg on PATH (decoding).
Sources and licences: SOURCES of make_round5_candidates.py (bow_ogrebane.wav CC0, rl_snort_vocal.flac CC0 as marked by archive.org, D805).
Test: round6_candidates_test.py.
"""
import os
import sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import make_round5_candidates as round5  # noqa: E402  (decode, events, band, write, SOURCE_DIR, RATE, the round 5 recipes)

RATE = round5.RATE
PEAK_DB = -2.5
HIGH_PASS = 80.0
MAX_SECONDS = 2.0
SILENCE_DB = 40.0     # the sound has died away once 40 dB under its loudest 10 ms...
SILENCE_FOR = 0.12    # ...for 120 ms

# round 5 stem elected by D826 -> prefix of its round 6 variants
ELECTED = {"s_shot5_bow": "s_shot6_bow_", "s_horse5_vocal_b": "s_horse6_vocal_"}
# suffix -> low-pass in Hz (None = no low-pass); first = the cleanest
SWEEP = [("full", None), ("lp6k", 6000.0), ("lp3k", 3000.0), ("lp1k5", 1500.0)]

# Barracks, lot A2 of D826: the user kept NO letter of round 5 ("cris de douleur, mauvaise qualite"), so ten NEW sounds, no voice at all:
# blades drawn and sheathed (StillNorthMedia CC0, SOURCES of round 5), a war drum and a horn call (VSCO 2 CE, CC0 1.0), and layers of them.
# stem -> parts; a part = (kind, file, event or folder, delay in s, seconds kept or None):
#   kind "r5": <file> of round 5 SOURCE_DIR, its event of rank <event> (0 = loudest), found like round 5;
#   kind "vsco": VSCO 2 CE sample <folder>/<file>, fetched once into MyCrusader_Audio\VSCO; a part with seconds kept gets a soft release.
TIMPANI = "Percussion/Timpani"
HORN = "Brass/F Horn/sus"
BARRACKS = {
    "s_barr6_draw_a": [("r5", "sabre_draw_fast.wav", 0, 0.0, None)],
    "s_barr6_draw_b": [("r5", "sabre_draw_fast.wav", 1, 0.0, None)],
    "s_barr6_draw_c": [("r5", "sabre_draw_fast.wav", 2, 0.0, None)],
    "s_barr6_sheath_a": [("r5", "seax_sheath_fast.wav", 0, 0.0, None)],
    "s_barr6_sheath_b": [("r5", "seax_sheath_fast.wav", 1, 0.0, None)],
    "s_barr6_two_blades": [("r5", "sabre_draw_fast.wav", 0, 0.0, None), ("r5", "sabre_draw_fast.wav", 1, 0.14, None)],
    "s_barr6_drum": [("vsco", "Timpani5_Hit_v4_rr1_Sum.wav", TIMPANI, 0.0, 1.2)],
    "s_barr6_drum_double": [("vsco", "Timpani5_Hit_v4_rr1_Sum.wav", TIMPANI, 0.0, 0.5),
                            ("vsco", "Timpani4_Hit_v4_rr1_Sum.wav", TIMPANI, 0.24, 1.1)],
    "s_barr6_horn": [("vsco", "MOHorn_sus_F2_v3_1.wav", HORN, 0.0, 0.9)],
    "s_barr6_drum_blade": [("vsco", "Timpani5_Hit_v4_rr1_Sum.wav", TIMPANI, 0.0, 1.2), ("r5", "sabre_draw_fast.wav", 0, 0.10, None)],
}
RELEASE = 0.30   # seconds of the soft release (power 4) at the end of a part kept shorter than its sample, and of every Barracks sound


def release(data, seconds=RELEASE):
    """Dies away over the last `seconds` ((1 - t) ** 4: 30 dB down well before the end), so a held note or a ringing drum is never cut."""
    n = min(len(data) // 2, int(seconds * RATE))
    out = data.copy()
    if n > 0:
        out[-n:] *= (1.0 - np.linspace(0, 1, n)) ** 4
    return out


def barracks_part(kind, name, which, keep):
    if kind == "r5":
        data = round5.decode(os.path.join(round5.SOURCE_DIR, name))
        starts = round5.events(data, 0.7)
        start = starts[min(which, len(starts) - 1)]
        return data[start:start + int(MAX_SECONDS * RATE)]
    import vsco_samples as vsco
    data = vsco.read_wav(vsco.fetch(os.path.join(round5.AUDIO, "VSCO"), which, name))
    return release(data[:int(keep * RATE)]) if keep else data[:int(MAX_SECONDS * RATE)]


def render_barracks(stem):
    parts = []
    for kind, name, which, delay, keep in BARRACKS[stem]:
        data = barracks_part(kind, name, which, keep)
        parts.append((int(delay * RATE), data / max(1e-9, np.abs(data).max()) * (1.0 if not parts else 0.8)))
    mix = np.zeros(max(offset + len(data) for offset, data in parts))
    for offset, data in parts:
        mix[offset:offset + len(data)] += data
    filtered = round5.band(mix, HIGH_PASS, RATE * 10.0)   # high-pass only, no low-pass, no drive
    clip = natural_clip(filtered, 0)
    return level(release(fades(clip)), 0.1)   # a short sheath click needs its first click turned down further to be heard


def natural_clip(data, start):
    """From start until the sound has died away (SILENCE_DB under its loudest 10 ms frame for SILENCE_FOR), at most MAX_SECONDS."""
    hop = int(0.01 * RATE)
    segment = data[start:start + int(MAX_SECONDS * RATE)]
    frames = np.array([np.sqrt(np.mean(segment[i:i + hop] ** 2)) for i in range(0, len(segment) - hop + 1, hop)])
    levels = 20 * np.log10(np.maximum(frames, 1e-9))
    loudest = int(np.argmax(levels))
    quiet = levels < levels[loudest] - SILENCE_DB
    need = int(SILENCE_FOR / 0.01)
    end = len(frames)
    for index in range(loudest, len(frames) - need + 1):
        if quiet[index:index + need].all():
            end = index + 10  # keep 100 ms of the silence (inside the 120 ms found quiet): the fade never bites a sound
            break
    return segment[:min(len(segment), end * hop)]


def fades(data):
    env = np.ones(len(data))
    a, r = int(0.004 * RATE), min(len(data) // 4, int(0.04 * RATE))
    env[:a] = np.linspace(0, 1, a)
    env[-r:] *= np.linspace(1, 0, r)
    return data * env


def render(reference, cutoff):
    source, which, seconds, _ = round5.RECIPES[reference]
    data = round5.decode(os.path.join(round5.SOURCE_DIR, source))
    starts = round5.events(data, seconds)          # the same event as round 5 (same search, same rank)
    start = starts[min(which, len(starts) - 1)]
    segment = data[start:start + int(MAX_SECONDS * RATE)]
    # filtered first, then trimmed where THIS version has died away (a low-passed body rings longer than its click)
    filtered = round5.band(segment, HIGH_PASS, RATE * 10.0 if cutoff is None else cutoff)   # far above Nyquist = high-pass only
    clip = natural_clip(filtered, 0)
    shaped = level(fades(clip))
    return shaped, start / RATE


def limit(data, ceiling):
    """Transparent peak limiter (not a drive): gain = ceiling / |x| where the sample is above it, held over +-2.5 ms and smoothed over 5 ms,
    so only the first click of a transient is turned down; the waveform is never bent like tanh."""
    window = int(0.005 * RATE)
    gain = np.minimum(1.0, ceiling / np.maximum(np.abs(data), 1e-9))
    padded = np.pad(gain, (window // 2, window - window // 2 - 1), mode="edge")
    held = np.lib.stride_tricks.sliding_window_view(padded, window).min(axis=1)
    smooth = np.convolve(np.pad(held, (window // 2, window - window // 2 - 1), mode="edge"), np.ones(window) / window, mode="valid")
    return data * np.minimum(held, smooth)


def level(data, floor=0.3):
    """Peak -2.5 dB; a transient too faint above 200 Hz (check_audio_soft floor -18 dB, target -16 dB) has its click limited step by step."""
    target = 10 ** (PEAK_DB / 20)
    out = data / max(1e-9, np.abs(data).max()) * target
    ceiling = 1.0
    while round5.measure(out)[2] < round5.TARGET_AUDIBLE_DB and ceiling > floor:
        ceiling *= 0.9
        limited = limit(data / max(1e-9, np.abs(data).max()), ceiling)
        out = limited / max(1e-9, np.abs(limited).max()) * target
    return out


def main():
    out_dir = sys.argv[1] if len(sys.argv) > 1 else os.path.join(round5.AUDIO, "Candidates", "soft")
    os.makedirs(out_dir, exist_ok=True)
    for reference, prefix in ELECTED.items():
        for suffix, cutoff in SWEEP:
            stem = prefix + suffix
            data, at = render(reference, cutoff)
            round5.write(os.path.join(out_dir, stem + ".wav"), data)
            print("%-22s from %-20s at %6.2f s  %.2f s  low-pass %s" % (stem, round5.RECIPES[reference][0], at, len(data) / RATE,
                                                                       "none" if cutoff is None else "%.0f Hz" % cutoff))
    for stem in BARRACKS:
        data = render_barracks(stem)
        round5.write(os.path.join(out_dir, stem + ".wav"), data)
        print("%-22s from %s  %.2f s" % (stem, " + ".join(part[1] for part in BARRACKS[stem]), len(data) / RATE))


if __name__ == "__main__":
    main()
