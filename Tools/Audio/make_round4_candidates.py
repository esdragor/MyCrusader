"""Round 4 of the building selection sounds (D798, 2026-11-14: "le reste n'est pas interessant"; sounds of ANOTHER nature, not stone or metal):
VOICES of men-at-arms for the Barracks (Building.Barracks), SHOTS of bows for the Archery range (Building.Range), a HORSE for the Stable
(Building.Stable). No CC0 recording of voices, bows or horses is in MyCrusader_Audio (Kenney, VSCO), so the ten of each are SYNTHESISED here
from nothing (numpy): additive voice through male formants, Karplus-Strong bowstring, shaped noise for whooshes, snorts and hooves.
Crude placeholders for the ear, nothing chosen here. Each file: <= 0.8 s, low-passed at 2.4 kHz, high-passed at 80 Hz, peak -2.5 dB, so it
passes Tools/Audio/check_audio_soft.py (soft rule and the -18 dB audibility floor above 200 Hz).

  python make_round4_candidates.py [out_dir] [stem_prefix]

Output: 16-bit mono 48 kHz WAV <out_dir>\\<stem>.wav (default MyCrusader_Audio\\Candidates\\soft). Deterministic (fixed seeds).
"""
import os
import sys
import wave

import numpy as np

AUDIO = os.environ.get("HOL_AUDIO_ROOT", r"C:\Users\baudo\Documents\MyCrusader_Audio")
RATE = 48000
PEAK_DB = -2.5
MAX_S = 0.78

# Male vowel formants (Hz) and the weight of each formant.
VOWELS = {"a": (730, 1090, 2440), "o": (570, 840, 2410), "e": (530, 1840, 2480), "u": (300, 870, 2240), "i": (390, 1990, 2550)}
WEIGHTS = (1.0, 0.45, 0.15)
BANDS = (90.0, 110.0, 160.0)


def t_of(seconds):
    return np.arange(int(seconds * RATE)) / RATE


def envelope(n, attack, release_power=1.5):
    env = np.ones(n)
    a = max(1, int(attack * RATE))
    env[:a] = np.linspace(0, 1, a)
    env[a:] = (1 - np.linspace(0, 1, n - a)) ** release_power
    return env


def formant_gain(freq, formants):
    gain = np.zeros_like(freq)
    for (centre, weight, band) in zip(formants, WEIGHTS, BANDS):
        gain += weight / np.sqrt(1 + ((freq - centre) / (band / 2)) ** 2)
    return gain


def track(points, n):
    """A value per sample from (fraction of the length, value) points."""
    xs = [p[0] for p in points]
    ys = [p[1] for p in points]
    return np.interp(np.linspace(0, 1, n), xs, ys)


def voice(seconds, f0_points, vowel_points, attack=0.03, breath=0.0, seed=1, aspirate=0.04):
    """Additive male voice: harmonics of f0 shaped by moving formants, a breathy 'h' at the start."""
    rng = np.random.default_rng(seed)
    n = int(seconds * RATE)
    f0 = track(f0_points, n) * (1 + 0.006 * np.sin(2 * np.pi * 5.5 * t_of(seconds)))
    formants = []
    for index in range(3):
        formants.append(track([(x, VOWELS[v][index]) for (x, v) in vowel_points], n))
    phase = 2 * np.pi * np.cumsum(f0) / RATE
    out = np.zeros(n)
    for k in range(1, 40):
        freq = k * f0
        if freq.min() > 3000:
            break
        # Tilt k^-1.0 (was k^-0.6): a darker chest voice; at -0.6 four voices sat at a centroid of 813-959 Hz (soft rule: 800).
        gain = formant_gain(freq, formants) * k ** -1.0 * (freq < 3000)
        out += gain * np.sin(k * phase + rng.uniform(0, 6.28))
    out /= max(1e-9, np.abs(out).max())
    if breath > 0:
        out = (1 - breath) * out + breath * shaped_noise(n, 250, 1100, seed + 7)
    out *= envelope(n, attack)
    if aspirate > 0:
        h = shaped_noise(int(aspirate * RATE), 500, 1800, seed + 3) * 0.35 * envelope(int(aspirate * RATE), 0.01, 0.5)
        out = np.concatenate([h, out])
    return out


def shaped_noise(n, low, high, seed):
    rng = np.random.default_rng(seed)
    spectrum = np.fft.rfft(rng.standard_normal(n))
    freqs = np.fft.rfftfreq(n, 1.0 / RATE)
    spectrum[(freqs < low) | (freqs > high)] = 0
    out = np.fft.irfft(spectrum, n)
    return out / max(1e-9, np.abs(out).max())


def moving_noise(seconds, centre_points, width, seed):
    """Noise whose band moves (a whoosh): short blocks of band noise cross-faded."""
    n = int(seconds * RATE)
    out = np.zeros(n)
    block = int(0.02 * RATE)
    centres = track(centre_points, n)
    window = np.hanning(2 * block)
    for start in range(0, n - 2 * block, block):
        c = centres[start]
        out[start:start + 2 * block] += shaped_noise(2 * block, max(60, c - width / 2), c + width / 2, seed + start) * window
    return out / max(1e-9, np.abs(out).max())


def string_pluck(seconds, freq, damping=0.996, seed=2, brightness=0.5):
    """Karplus-Strong: a bowstring's twang."""
    rng = np.random.default_rng(seed)
    n = int(seconds * RATE)
    period = int(RATE / freq)
    buf = rng.uniform(-1, 1, period)
    # A softer pluck (8-sample smoothing, was 4): at 4 the twangs sat at a centroid of 815-907 Hz (soft rule: 800).
    buf = np.convolve(buf, np.ones(8) / 8, mode="same") if brightness < 1 else buf
    out = np.zeros(n)
    for i in range(n):
        out[i] = buf[i % period]
        buf[i % period] = damping * 0.5 * (buf[i % period] + buf[(i + 1) % period])
    return out / max(1e-9, np.abs(out).max())


def thud(seconds, freq, noise_low=80, noise_high=900, seed=4, decay=25.0):
    t = t_of(seconds)
    body = np.sin(2 * np.pi * freq * t * (1 - 0.15 * t)) * np.exp(-decay * t)
    knock = shaped_noise(len(t), noise_low, noise_high, seed) * np.exp(-60 * t)
    out = 0.7 * body + 0.5 * knock
    return out / max(1e-9, np.abs(out).max())


def hoof(seed, hard=1.0):
    t = t_of(0.09)
    clop = (np.sin(2 * np.pi * 420 * t) * 0.6 + np.sin(2 * np.pi * 780 * t) * 0.3) * np.exp(-70 * t)
    out = hard * clop + 0.6 * thud(0.09, 240, 200, 1200, seed, 40)
    return out / max(1e-9, np.abs(out).max())


def place(parts, seconds):
    """[(start s, sound, gain)] mixed into one clip."""
    out = np.zeros(int(seconds * RATE))
    for (start, sound, gain) in parts:
        i = int(start * RATE)
        j = min(len(out), i + len(sound))
        out[i:j] += gain * sound[:j - i]
    return out


def snort(seed, seconds=0.32, flutter=0.0):
    n = int(seconds * RATE)
    out = shaped_noise(n, 180, 1400, seed) * envelope(n, 0.02, 2.0)
    if flutter > 0:
        out *= 1 - flutter * 0.5 * (1 + np.sin(2 * np.pi * 28 * t_of(seconds)))
    return out


def nicker(seconds, f0, pulses_hz, seed):
    v = voice(seconds, [(0, f0), (0.5, f0 * 1.08), (1, f0 * 0.9)], [(0, "o"), (1, "u")], breath=0.35, seed=seed, aspirate=0.0)
    return v * (0.55 + 0.45 * np.sin(2 * np.pi * pulses_hz * t_of(len(v) / RATE)) ** 2)


def recipes():
    r = {}
    # Building.Barracks: voices of men-at-arms (shouts, orders, barracks murmur).
    r["s_voice_hah"] = voice(0.42, [(0, 135), (1, 105)], [(0, "a"), (1, "a")], seed=11)
    r["s_voice_ho"] = voice(0.45, [(0, 118), (1, 100)], [(0, "o"), (1, "o")], seed=12)
    r["s_voice_hey"] = voice(0.40, [(0, 128), (0.4, 140), (1, 112)], [(0, "e"), (1, "e")], seed=13)
    r["s_voice_hup"] = place([(0, voice(0.22, [(0, 120), (1, 112)], [(0, "a"), (1, "a")], seed=14), 1.0),
                              (0.27, voice(0.25, [(0, 135), (1, 108)], [(0, "u"), (1, "u")], seed=15), 1.0)], 0.6)
    r["s_voice_duo"] = place([(0, voice(0.45, [(0, 130), (1, 104)], [(0, "a"), (1, "o")], seed=16), 0.6),
                              (0.015, voice(0.45, [(0, 108), (1, 92)], [(0, "a"), (1, "o")], seed=17), 0.6)], 0.5)
    r["s_voice_trio"] = place([(0, voice(0.5, [(0, 112), (1, 98)], [(0, "o"), (1, "o")], seed=18), 0.45),
                               (0.02, voice(0.5, [(0, 126), (1, 110)], [(0, "o"), (1, "o")], seed=19), 0.45),
                               (0.035, voice(0.5, [(0, 98), (1, 88)], [(0, "o"), (1, "o")], seed=20), 0.45)], 0.6)
    r["s_voice_grunt"] = voice(0.30, [(0, 98), (1, 86)], [(0, "u"), (1, "o")], attack=0.015, seed=21, aspirate=0.02)
    r["s_voice_murmur"] = place([(0, voice(0.7, [(0, 105), (0.5, 112), (1, 100)], [(0, "o"), (0.5, "a"), (1, "e")], attack=0.12, breath=0.25, seed=22, aspirate=0), 0.4),
                                 (0.08, voice(0.62, [(0, 92), (1, 98)], [(0, "e"), (0.5, "o"), (1, "u")], attack=0.12, breath=0.25, seed=23, aspirate=0), 0.4),
                                 (0.15, voice(0.55, [(0, 118), (1, 108)], [(0, "a"), (1, "o")], attack=0.12, breath=0.25, seed=24, aspirate=0), 0.35)], 0.76)
    r["s_voice_whisper"] = place([(0, voice(0.6, [(0, 100), (1, 96)], [(0, "a"), (0.5, "u"), (1, "e")], attack=0.08, breath=0.8, seed=25, aspirate=0), 0.6),
                                  (0.1, voice(0.55, [(0, 112), (1, 104)], [(0, "o"), (1, "a")], attack=0.08, breath=0.8, seed=26, aspirate=0), 0.5)], 0.72)
    r["s_voice_order"] = place([(0, voice(0.2, [(0, 110), (1, 125)], [(0, "o"), (1, "i")], seed=27), 1.0),
                                (0.22, voice(0.36, [(0, 140), (1, 104)], [(0, "a"), (1, "a")], seed=28, aspirate=0.03), 1.0)], 0.66)
    # Building.Range: shots (bowstring twang, arrow whoosh, arrow hitting the target).
    whoosh = lambda s, c0, c1, seed: moving_noise(s, [(0, c0), (1, c1)], 500, seed) * envelope(int(s * RATE), s * 0.4, 1.2)
    r["s_shot_twang_low"] = string_pluck(0.45, 95, 0.997, 31)
    r["s_shot_twang"] = string_pluck(0.40, 140, 0.995, 32)
    r["s_shot_twang_whoosh"] = place([(0, string_pluck(0.35, 120, 0.995, 33), 0.8), (0.04, whoosh(0.3, 900, 400, 34), 0.5)], 0.45)
    r["s_shot_whoosh"] = whoosh(0.35, 1000, 350, 35)
    r["s_shot_volley"] = place([(0, whoosh(0.3, 900, 350, 36), 0.5), (0.07, whoosh(0.3, 800, 300, 37), 0.5), (0.15, whoosh(0.3, 950, 400, 38), 0.5)], 0.5)
    r["s_shot_thunk"] = thud(0.3, 260, 200, 1500, 39, 22)
    r["s_shot_twang_thunk"] = place([(0, string_pluck(0.3, 125, 0.994, 40), 0.7), (0.3, thud(0.3, 150, 100, 1400, 41, 22), 0.9)], 0.62)
    r["s_shot_draw"] = place([(0, shaped_noise(int(0.35 * RATE), 150, 700, 42) * np.linspace(0, 1, int(0.35 * RATE)) ** 2 * 0.35, 1.0),
                              (0.35, string_pluck(0.35, 110, 0.995, 43), 0.9)], 0.72)
    r["s_shot_double"] = place([(0, string_pluck(0.3, 130, 0.993, 44), 0.8), (0.2, string_pluck(0.3, 115, 0.993, 45), 0.8)], 0.52)
    r["s_shot_crossbow"] = place([(0, thud(0.05, 300, 400, 2000, 46, 80), 0.6), (0.02, string_pluck(0.25, 170, 0.985, 47), 0.8),
                                  (0.06, whoosh(0.22, 800, 400, 48), 0.4)], 0.32)
    # Building.Stable: a horse (nicker, snort, lip flutter, hooves).
    r["s_horse_nicker"] = nicker(0.55, 130, 9, 51)
    r["s_horse_nicker_long"] = nicker(0.76, 115, 7, 52)
    r["s_horse_snort"] = snort(53)
    r["s_horse_flutter"] = snort(54, 0.5, 1.0)
    r["s_horse_neigh_low"] = voice(0.72, [(0, 210), (0.25, 330), (0.6, 260), (1, 190)], [(0, "e"), (0.4, "a"), (1, "o")], breath=0.3, seed=55, aspirate=0.02) * \
        (1 + 0.25 * np.sin(2 * np.pi * 9 * t_of(0.74)))[:int(0.74 * RATE)]
    r["s_horse_walk"] = place([(0, hoof(56), 0.8), (0.18, hoof(57), 0.6), (0.36, hoof(58), 0.8), (0.54, hoof(59), 0.6)], 0.66)
    r["s_horse_trot"] = place([(0, hoof(60), 0.9), (0.1, hoof(61), 0.7), (0.3, hoof(62), 0.9), (0.4, hoof(63), 0.7)], 0.52)
    r["s_horse_gallop"] = place([(0, hoof(64), 0.7), (0.07, hoof(65), 0.8), (0.14, hoof(66), 1.0), (0.38, hoof(67), 0.7), (0.45, hoof(68), 0.8), (0.52, hoof(69), 1.0)], 0.64)
    r["s_horse_stamp"] = place([(0, thud(0.25, 110, 120, 1200, 70, 18), 1.0), (0.18, snort(71, 0.3), 0.6)], 0.5)
    r["s_horse_snort_hoof"] = place([(0, snort(72, 0.3, 0.6), 0.8), (0.32, hoof(73), 0.8), (0.48, hoof(74), 0.7)], 0.6)
    return r


HOOVES = ("s_horse_walk", "s_horse_trot", "s_horse_gallop", "s_horse_snort_hoof")


def finish(sound, cutoff, drive=0.0):
    # Measured 2026-11-14: at 2.4 kHz / 80 Hz the voices and twangs sat at a centroid of 835-1376 Hz (soft rule: 800) and the hooves and the
    # thunk under the -18 dB floor above 200 Hz; so a lower low-pass (1.5 kHz, 1.2 kHz for the shots) and a high-pass at 150 Hz.
    # The hooves were still at -18.2 to -18.7 dB (short clops, the 100 ms window is half silence): a soft saturation (tanh, drive) raises
    # their body before the peak normalisation, so the peak stays at PEAK_DB (no clipping, the -2.1 dBFS limiter rule unchanged).
    sound = sound[:int(MAX_S * RATE)]
    if drive > 0:
        sound = np.tanh(drive * sound / max(1e-9, np.abs(sound).max()))
    spectrum = np.fft.rfft(sound)
    freqs = np.fft.rfftfreq(len(sound), 1.0 / RATE)
    spectrum *= 1 / np.sqrt(1 + (freqs / cutoff) ** 12)
    spectrum[freqs < 150] = 0
    sound = np.fft.irfft(spectrum, len(sound))
    fade = int(0.04 * RATE)
    sound[-fade:] *= np.linspace(1, 0, fade)
    return sound / max(1e-9, np.abs(sound).max()) * 10 ** (PEAK_DB / 20)


def write(path, sound):
    with wave.open(path, "wb") as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes((np.clip(sound, -1, 1) * 32767).astype("<i2").tobytes())


def main():
    out_dir = sys.argv[1] if len(sys.argv) > 1 else os.path.join(AUDIO, "Candidates", "soft")
    prefix = sys.argv[2] if len(sys.argv) > 2 else "s_"
    os.makedirs(out_dir, exist_ok=True)
    count = 0
    for stem, sound in recipes().items():
        if stem.startswith(prefix):
            cutoff = 1000.0 if stem.startswith("s_shot_") else 1200.0
            write(os.path.join(out_dir, stem + ".wav"), finish(sound, cutoff, 3.0 if stem in HOOVES else 0.0))
            count += 1
    print("make_round4_candidates: %d files in %s" % (count, out_dir))


if __name__ == "__main__":
    main()
