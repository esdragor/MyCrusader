"""Measures the SOFT candidates ("s_" stems of Script/Audio/HOLAudioCandidates.as) against the rule of the user (2026-11-07, "Sons : pas
usants": heard very often and for a long time, so low, dull, short, soft; no trumpet, horn, little bell, nothing high-pitched or shrill).
The ear decides what is liked; this tool only refuses what is certainly tiring, by measuring the WAV files themselves:

  duration       <= 0.80 s   (short)
  centroid       <= 800 Hz   (low: the spectral centroid of the whole clip)
  energy > 3 kHz <= 1 %      (nothing shrill: share of the spectral energy above 3 kHz)
  peak           between -12 and -2 dB (audible, not clipping)
  audible        >= -18 dB above 200 Hz, loudest 100 ms (Building.Stone and Building.Metal only, answer of 2026-11-14)

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
             "Alert.Diplomacy", "Building.Wood", "Building.Stone", "Building.Sacred", "Building.Metal", "Building.Coins",
             "Building.Barracks", "Building.Range", "Building.Stable"]
MAX_SECONDS = 0.80
MAX_CENTROID = 800.0
MAX_HIGH_SHARE = 0.01
MIN_PEAK_DB, MAX_PEAK_DB = -12.0, -2.0
# Audible on small speakers (user, 2026-11-14, Stone and Metal: "les sons sont difficiles a entendre, volume trop faible"). Measured on the
# file: the sound high-passed at 200 Hz (laptop speakers and most headsets give little below), RMS of its loudest 100 ms window, in dB.
# The round 2 of Stone sat at -21 to -46 dB (energy below 100 Hz), the three elected building sounds the user hears well at -12 to -19.
# A floor, not a choice: the ear still decides. Only for the cues re-done after that answer.
AUDIBLE_CUES = ["Building.Stone", "Building.Metal", "Building.Barracks", "Building.Range", "Building.Stable"]
# D826 (2026-11-16): the Range and Stable lists are round 6, a SWEEP of ONE elected sound (the round 5 G as reference + clean variants,
# low-pass none to 1.5 kHz, make_round6_candidates.py): 3 to 10 stems, and the ear chooses how soft: duration up to 2 s (the sound is not cut),
# centroid and share above 3 kHz printed but not refused. Peak and audibility still checked.
CLEAN_CUES = ["Building.Range", "Building.Stable"]
CLEAN_MAX_SECONDS = 2.0
AUDIBLE_FROM_HZ = 200.0
MIN_AUDIBLE_DB = -18.0


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


def audible_db(path):
    """RMS (dB) of the loudest 100 ms of the sound once everything below AUDIBLE_FROM_HZ is removed."""
    rate, data = read(path)
    spectrum = np.fft.rfft(data)
    spectrum[np.fft.rfftfreq(len(data), 1.0 / rate) < AUDIBLE_FROM_HZ] = 0
    heard = np.fft.irfft(spectrum, len(data))
    window = int(0.1 * rate)
    power = np.convolve(heard ** 2, np.ones(window) / window, mode="valid") if len(heard) > window else np.array([np.mean(heard ** 2)])
    loudest = float(power.max())
    return 10 * np.log10(loudest) if loudest > 0 else -200.0


def problems_of(values, clean=False):
    seconds, centroid, high, peak = values
    found = []
    if clean:
        if seconds > CLEAN_MAX_SECONDS:
            found.append("%.2f s is too long even for a clean sweep (max %.2f)" % (seconds, CLEAN_MAX_SECONDS))
        if not (MIN_PEAK_DB <= peak <= MAX_PEAK_DB):
            found.append("peak %.1f dB outside %.0f..%.0f" % (peak, MIN_PEAK_DB, MAX_PEAK_DB))
        return found
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
        if cue in CLEAN_CUES and 3 <= len(by_cue.get(cue, [])) <= 10:
            continue
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
        clean = all(cue in CLEAN_CUES for cue, listed in by_cue.items() if stem in listed)
        for text in problems_of(values, clean):
            print("  FAIL : %s: %s" % (stem, text))
            failures += 1
    for cue in AUDIBLE_CUES:
        for stem in by_cue.get(cue, []):
            path = os.path.join(args.dir, stem + ".wav")
            if os.path.exists(path):
                level = audible_db(path)
                if level < MIN_AUDIBLE_DB:
                    print("  FAIL : %s (%s): %.1f dB above %.0f Hz, too faint (min %.0f)" % (stem, cue, level, AUDIBLE_FROM_HZ, MIN_AUDIBLE_DB))
                    failures += 1
                else:
                    print("  audible: %-24s %.1f dB above %.0f Hz" % (stem, level, AUDIBLE_FROM_HZ))
    if args.selftest:
        # the audibility floor must refuse the sub-bass thud the user could not hear (round 2 of Stone), and accept an elected sound
        for faint, must_fail in (("s_stone2_thud_low", True), ("s_wood_door_lim", False)):
            path = os.path.join(args.dir, faint + ".wav")
            if not os.path.exists(path):
                print("  FAIL : selftest sound %s is missing" % path)
                failures += 1
            elif (audible_db(path) < MIN_AUDIBLE_DB) != must_fail:
                print("  FAIL : selftest: %s at %.1f dB is %s" % (faint, audible_db(path), "accepted" if must_fail else "refused"))
                failures += 1
            else:
                print("  selftest ok : %s at %.1f dB is %s" % (faint, audible_db(path), "refused" if must_fail else "accepted"))
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
