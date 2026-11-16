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
# Barracks (D826 lot A2): the user kept NO letter of round 5 ("cris de douleur, mauvaise qualite"): ten NEW sounds, none cut out of a voice
# recording (no yell, grunt nor crowd), treated as cleanly as the Range and Stable round 6 (not cut, no drive, peak -12..-2 dB, <= 2 s).
BARRACKS = "Building.Barracks"
BARRACKS_PREFIX = "s_barr6_"
VOICE_WORDS = ("yell", "grunt", "crowd", "voice", "shout", "scream")

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

sys.path.insert(0, HERE)
import make_round6_candidates as round6  # noqa: E402  (the recipes the WAV files were made from)

stems = lists.get(BARRACKS, [])
if len(stems) != 10:
    fail("%s: %d stems, expected 10 new sounds" % (BARRACKS, len(stems)))
for stem in stems:
    if not stem.startswith(BARRACKS_PREFIX) or any(word in stem for word in VOICE_WORDS):
        fail("%s: %s is not a round 6 sound without voice (%s*)" % (BARRACKS, stem, BARRACKS_PREFIX))
        continue
    recipe = getattr(round6, "BARRACKS", {}).get(stem)
    if not recipe:
        fail("%s: no recipe for %s in make_round6_candidates.BARRACKS" % (BARRACKS, stem))
        continue
    voiced = [part[1] for part in recipe if any(word in part[1].lower() for word in VOICE_WORDS)]
    if voiced:
        fail("%s: %s is cut out of a voice recording: %s" % (BARRACKS, stem, ", ".join(voiced)))
    path = os.path.join(SOFT, stem + ".wav")
    if not os.path.exists(path):
        fail("%s: %s.wav is not in %s (run make_round6_candidates.py)" % (BARRACKS, stem, SOFT))
        continue
    rate, data = read(path)
    tail = loudest_db(data[-int(0.1 * rate):], rate)
    top = loudest_db(data, rate)
    peak = 20 * np.log10(max(1e-9, float(np.abs(data).max())))
    seconds = len(data) / rate
    print("  %-24s %.2f s  tail %5.1f dB under the loudest  centroid %4.0f Hz  peak %5.1f dB" % (stem, seconds, top - tail, centroid(data, rate), peak))
    if top - tail < 30.0:
        fail("%s: cut while still sounding (last 100 ms only %.1f dB under the loudest, min 30)" % (stem, top - tail))
    if not -12.0 <= peak <= -2.0:
        fail("%s: peak %.1f dB outside -12..-2" % (stem, peak))
    if seconds > 2.0:
        fail("%s: %.2f s, longer than 2 s" % (stem, seconds))


# D832 REGLAGE (2026-11-16): Stable B without the neigh of its start, only the snort ("pfff") of its end. Building.CutStable = [the cut, B whole].
# The neigh is a VOICED sound (a pitch: strong autocorrelation between 250 and 1500 Hz), the snort a breath (noise): measured here, not in the
# generator. Effect: B whole has a voiced neigh; the cut has none left in its audible part, keeps the snort's body, is not cut at its end.
STABLE_CUT = "Building.CutStable"
STABLE_B = "s_horse6_vocal_full"


def voiced_frames(data, rate, within_db=25.0, threshold=0.7):
    """Frames of 40 ms (hop 20 ms) within `within_db` of the loudest whose normalised autocorrelation peaks >= threshold for a pitch of 250-1500 Hz."""
    size, hop = int(0.04 * rate), int(0.02 * rate)
    lo, hi = int(rate / 1500), int(rate / 250)
    levels, harms = [], []
    for start in range(0, len(data) - size + 1, hop):
        frame = data[start:start + size] - data[start:start + size].mean()
        ac = np.correlate(frame, frame, "full")[size - 1:]
        harms.append(float(ac[lo:hi].max() / max(1e-12, ac[0])))
        levels.append(10 * np.log10(max(1e-20, float(np.mean(frame ** 2)))))
    top = max(levels) if levels else 0.0
    return [i * hop / rate for i in range(len(levels)) if levels[i] >= top - within_db and harms[i] >= threshold]


stems = lists.get(STABLE_CUT, [])
if len(stems) != 2 or stems[1] != STABLE_B:
    fail("%s: %s, expected [the snort cut out of %s, %s]" % (STABLE_CUT, ",".join(stems) or "nothing", STABLE_B, STABLE_B))
else:
    whole_path, cut_path = os.path.join(SOFT, STABLE_B + ".wav"), os.path.join(SOFT, stems[0] + ".wav")
    if not os.path.exists(cut_path) or not os.path.exists(whole_path):
        fail("%s: %s.wav or %s.wav is not in %s (run make_round6_candidates.py)" % (STABLE_CUT, stems[0], STABLE_B, SOFT))
    else:
        rate, whole = read(whole_path)
        _, cut = read(cut_path)
        neigh, left = voiced_frames(whole, rate), voiced_frames(cut, rate)
        tail = loudest_db(cut[-int(0.1 * rate):], rate)
        top = loudest_db(cut, rate)
        peak = 20 * np.log10(max(1e-9, float(np.abs(cut).max())))
        print("  %-24s %.2f s  voiced frames %d (B whole: %d, %.2f-%.2f s)  tail %5.1f dB  peak %5.1f dB" % (
            stems[0], len(cut) / rate, len(left), len(neigh), min(neigh or [0]), max(neigh or [0]), top - tail, peak))
        if len(neigh) < 3:
            fail("%s: B whole shows no voiced neigh (%d frames): the measure cannot tell the neigh from the snort" % (STABLE_B, len(neigh)))
        if left:
            fail("%s: still a voiced neigh at %s s" % (stems[0], ", ".join("%.2f" % at for at in left)))
        if not 0.15 <= len(cut) / rate < len(whole) / rate - 0.2:
            fail("%s: %.2f s, expected the snort only (0.15 s to B whole minus the neigh)" % (stems[0], len(cut) / rate))
        if top - tail < 30.0:
            fail("%s: cut while still sounding (last 100 ms only %.1f dB under the loudest)" % (stems[0], top - tail))
        if not -12.0 <= peak <= -2.0:
            fail("%s: peak %.1f dB outside -12..-2" % (stems[0], peak))

if failures == 0:
    print("round6_candidates_test: PASS")
    sys.exit(0)
print("round6_candidates_test: FAIL (%d)" % failures)
sys.exit(1)
