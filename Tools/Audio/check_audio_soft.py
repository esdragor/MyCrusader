"""Measures the SOFT candidates ("s_" stems of Script/Audio/HOLAudioCandidates.as) against the rule of the user (2026-11-07, "Sons : pas
usants": heard very often and for a long time, so low, dull, short, soft; no trumpet, horn, little bell, nothing high-pitched or shrill).
The ear decides what is liked; this tool only refuses what is certainly tiring, by measuring the WAV files themselves:

  duration       <= 0.80 s   (short)
  centroid       <= 800 Hz   (low: the spectral centroid of the whole clip)
  energy > 3 kHz <= 1 %      (nothing shrill: share of the spectral energy above 3 kHz)
  peak           between -12 and -2 dB (audible, not clipping)

  python check_audio_soft.py [--dir <folder of the wavs>] [--file <HOLAudioCandidates.as>] [--selftest]

Prints one line per sound (stem, duration, centroid, share above 3 kHz, peak) and one line per problem, then "check_audio_soft: PASS"
or "FAIL (n)"; exit 0 = pass. --selftest also measures two bright sounds that MUST fail (a trumpet call and a glockenspiel C6 of the VSCO
candidates), so the tool is known to tell tiring from soft; a bright sound that passes is itself a failure of the tool.
"""
import argparse
import os
import re
import sys
import wave

import numpy as np

AUDIO = r"C:\Users\baudo\Documents\MyCrusader_Audio"
HERE = os.path.dirname(os.path.abspath(__file__))
LINE = re.compile(r'^\s*Add\(List,\s*"(?P<cue>[A-Za-z]+\.[A-Za-z]+)",\s*"(?P<stems>[A-Za-z0-9_,]+)"')
# The cues whose ten candidates MUST all be soft (s_ stems): the training alert, the six other alerts, the five building families.
SOFT_CUES = ["Alert.TrainingDone", "Alert.UnderAttack", "Alert.BuildingDone", "Alert.TechDone", "Alert.Trade", "Alert.Volcano",
             "Alert.Diplomacy", "Building.Wood", "Building.Stone", "Building.Sacred", "Building.Metal", "Building.Coins"]
MAX_SECONDS = 0.80
MAX_CENTROID = 800.0
MAX_HIGH_SHARE = 0.01
MIN_PEAK_DB, MAX_PEAK_DB = -12.0, -2.0


def read(path):
    with wave.open(path, "rb") as source:
        rate = source.getframerate()
        raw = np.frombuffer(source.readframes(source.getnframes()), dtype="<i2").astype(np.float64) / 32768.0
        if source.getnchannels() > 1:
            raw = raw.reshape(-1, source.getnchannels()).mean(axis=1)
    return rate, raw


def measure(path):
    """(seconds, centroid in Hz, share of the energy above 3 kHz, peak in dB)."""
    rate, data = read(path)
    seconds = len(data) / rate
    spectrum = np.abs(np.fft.rfft(data * np.hanning(len(data))))
    freqs = np.fft.rfftfreq(len(data), 1.0 / rate)
    total = spectrum.sum()
    energy = (spectrum ** 2).sum()
    centroid = float((freqs * spectrum).sum() / total) if total > 0 else 0.0
    high = float((spectrum[freqs > 3000] ** 2).sum() / energy) if energy > 0 else 0.0
    peak = float(np.abs(data).max())
    return seconds, centroid, high, (20 * np.log10(peak) if peak > 0 else -200.0)


def problems_of(values):
    seconds, centroid, high, peak = values
    found = []
    if seconds > MAX_SECONDS:
        found.append("%.2f s is not short (max %.2f)" % (seconds, MAX_SECONDS))
    if centroid > MAX_CENTROID:
        found.append("centroid %.0f Hz is not low (max %.0f)" % (centroid, MAX_CENTROID))
    if high > MAX_HIGH_SHARE:
        found.append("%.1f %% of the energy above 3 kHz is shrill (max %.0f %%)" % (high * 100, MAX_HIGH_SHARE * 100))
    if not (MIN_PEAK_DB <= peak <= MAX_PEAK_DB):
        found.append("peak %.1f dB outside %.0f..%.0f" % (peak, MIN_PEAK_DB, MAX_PEAK_DB))
    return found


def soft_stems(candidates_file):
    """(every s_ stem, {cue: [its s_ stems]})."""
    stems = []
    by_cue = {}
    with open(candidates_file, encoding="utf-8") as source:
        for line in source:
            match = LINE.match(line)
            if match:
                for stem in match.group("stems").split(","):
                    if stem.startswith("s_"):
                        by_cue.setdefault(match.group("cue"), []).append(stem)
                        if stem not in stems:
                            stems.append(stem)
    return stems, by_cue


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dir", default=os.path.join(AUDIO, "Candidates", "soft"))
    parser.add_argument("--file", default=os.path.join(HERE, "..", "..", "Script", "Audio", "HOLAudioCandidates.as"))
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    failures = 0
    stems, by_cue = soft_stems(args.file)
    for cue in SOFT_CUES:
        if len(by_cue.get(cue, [])) != 10:
            print("  FAIL : %s has %d soft (s_) candidates, expected 10" % (cue, len(by_cue.get(cue, []))))
            failures += 1
    if not stems:
        print("  FAIL : no s_ stem in %s (the soft candidates are not named yet)" % args.file)
        failures += 1
    for stem in stems:
        path = os.path.join(args.dir, stem + ".wav")
        if not os.path.exists(path):
            print("  FAIL : %s.wav is not in %s (run make_soft_candidates.py)" % (stem, args.dir))
            failures += 1
            continue
        values = measure(path)
        print("  %-26s %.2f s  centroid %4.0f Hz  >3k %4.1f %%  peak %5.1f dB" % (stem, values[0], values[1], values[2] * 100, values[3]))
        for text in problems_of(values):
            print("  FAIL : %s: %s" % (stem, text))
            failures += 1
    if args.selftest:
        for bright in ("v_trumpet_call", "v_glock_C6"):
            path = os.path.join(AUDIO, "Candidates", "vsco", bright + ".wav")
            if not os.path.exists(path):
                print("  FAIL : selftest sound %s is missing" % path)
                failures += 1
            elif problems_of(measure(path)):
                print("  selftest ok : %s is refused (%s)" % (bright, "; ".join(problems_of(measure(path)))))
            else:
                print("  FAIL : selftest: the bright sound %s was NOT refused" % bright)
                failures += 1
    print("  %d soft sounds measured" % len(stems))
    if failures == 0:
        print("check_audio_soft: PASS")
        return 0
    print("check_audio_soft: FAIL (%d)" % failures)
    return 1


if __name__ == "__main__":
    sys.exit(main())
