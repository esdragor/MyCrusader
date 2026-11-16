"""Test of the round 6 of the Range and Stable selection sounds (D826, 2026-11-16, AudioBuildingsInProgress.md).

The user elected the KIND of sound of letter G for both (Range: s_shot5_bow, Stable: s_horse5_vocal_b) but heard a poor recording; the
round 5 cut every clip to 0.60-0.80 s, drove it through tanh and lowered a low-pass until the centroid was under 740 Hz. Round 6 must be a
clean version of the SAME sound, a few variants to hear again. On the real chain (the list of Script/Audio/HOLAudioCandidates.as and the
WAV files that make_round6_candidates.py wrote), this test asserts the effect:

  1. each cue lists the round 5 letter G first (the reference, unchanged) then 3 to 9 round 6 variants of that same sound, nothing else;
  2. every round 6 WAV exists in <HOL_AUDIO_ROOT>\\Candidates\\soft;
  3. not cut: the sound has died away before the file ends (loudest 50 ms of the last 100 ms at least 30 dB under the loudest 50 ms);
  4. not muffled: every round 6 variant is brighter (spectral centroid) than the round 5 reference, and the brightest is at least 1.5 times
     brighter (no low-pass at or under 800 Hz);
  5. not distorted nor faint: peak between -12 and -2 dB.

Run: python round6_candidates_test.py   (exit 0 = pass)
"""
import os
import re
import sys
import wave

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
AUDIO = os.environ.get("HOL_AUDIO_ROOT", r"C:\Users\baudo\Documents\MyCrusader_Audio")
SOFT = os.path.join(AUDIO, "Candidates", "soft")
CANDIDATES = os.path.join(HERE, "..", "..", "Script", "Audio", "HOLAudioCandidates.as")
LINE = re.compile(r'^\s*Add\(List,\s*"(?P<cue>[A-Za-z]+\.[A-Za-z]+)",\s*"(?P<stems>[A-Za-z0-9_,]+)"')
# cue -> (round 5 reference elected by D826, prefix of its round 6 variants)
ELECTED = {"Building.Range": ("s_shot5_bow", "s_shot6_bow_"), "Building.Stable": ("s_horse5_vocal_b", "s_horse6_vocal_")}

failures = 0


def fail(text):
    global failures
    print("  FAIL : " + text)
    failures += 1


def read(path):
    with wave.open(path, "rb") as source:
        rate = source.getframerate()
        data = np.frombuffer(source.readframes(source.getnframes()), dtype="<i2").astype(np.float64) / 32768.0
        if source.getnchannels() > 1:
            data = data.reshape(-1, source.getnchannels()).mean(axis=1)
    return rate, data


def loudest_db(data, rate):
    window = max(1, int(0.05 * rate))
    if len(data) <= window:
        return 10 * np.log10(max(1e-20, float(np.mean(data ** 2))))
    power = np.convolve(data ** 2, np.ones(window) / window, mode="valid")
    return 10 * np.log10(max(1e-20, float(power.max())))


def centroid(data, rate):
    spectrum = np.abs(np.fft.rfft(data * np.hanning(len(data))))
    freqs = np.fft.rfftfreq(len(data), 1.0 / rate)
    return float((freqs * spectrum).sum() / max(1e-12, spectrum.sum()))


lists = {}
with open(CANDIDATES, encoding="utf-8") as table:
    for line in table:
        match = LINE.match(line)
        if match:
            lists[match.group("cue")] = match.group("stems").split(",")

for cue, (reference, prefix) in ELECTED.items():
    stems = lists.get(cue, [])
    if not stems or stems[0] != reference:
        fail("%s: first stem is %s, expected the round 5 reference %s" % (cue, stems[0] if stems else "nothing", reference))
    variants = [stem for stem in stems[1:] if stem.startswith(prefix)]
    others = [stem for stem in stems[1:] if not stem.startswith(prefix)]
    if not 3 <= len(variants) <= 9:
        fail("%s: %d round 6 variants (%s*), expected 3 to 9" % (cue, len(variants), prefix))
    if others:
        fail("%s: stems other than the elected sound: %s" % (cue, ", ".join(others)))
    reference_path = os.path.join(SOFT, reference + ".wav")
    if not os.path.exists(reference_path):
        fail("%s: the round 5 reference %s.wav is missing" % (cue, reference))
        continue
    rate, data = read(reference_path)
    reference_centroid = centroid(data, rate)
    print("  %-24s reference  %.2f s  centroid %4.0f Hz" % (reference, len(data) / rate, reference_centroid))
    brightest = 0.0
    for stem in variants:
        path = os.path.join(SOFT, stem + ".wav")
        if not os.path.exists(path):
            fail("%s: %s.wav is not in %s (run make_round6_candidates.py)" % (cue, stem, SOFT))
            continue
        rate, data = read(path)
        tail = loudest_db(data[-int(0.1 * rate):], rate)
        top = loudest_db(data, rate)
        bright = centroid(data, rate)
        peak = 20 * np.log10(max(1e-9, float(np.abs(data).max())))
        brightest = max(brightest, bright)
        print("  %-24s %.2f s  tail %5.1f dB under the loudest  centroid %4.0f Hz  peak %5.1f dB" % (stem, len(data) / rate, top - tail, bright, peak))
        if top - tail < 30.0:
            fail("%s: cut while still sounding (last 100 ms only %.1f dB under the loudest, min 30)" % (stem, top - tail))
        if bright <= reference_centroid:
            fail("%s: centroid %.0f Hz is not brighter than the round 5 reference (%.0f Hz)" % (stem, bright, reference_centroid))
        if not -12.0 <= peak <= -2.0:
            fail("%s: peak %.1f dB outside -12..-2" % (stem, peak))
    if variants and brightest < 1.5 * reference_centroid:
        fail("%s: the brightest round 6 variant (%.0f Hz) is not 1.5 times brighter than the reference (%.0f Hz)" % (cue, brightest, reference_centroid))

if failures == 0:
    print("round6_candidates_test: PASS")
    sys.exit(0)
print("round6_candidates_test: FAIL (%d)" % failures)
sys.exit(1)
