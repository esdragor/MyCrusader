"""Headroom of the sounds played above 100 % volume (AudioBuildingsInProgress.md, 2026-11-09).

The table (Script/Audio/HOLAudioTable.as) plays the elected building selection sounds at BuildingSelectGainPercent (250 = x2.5, the
user's "volume x2 to x3"). A sound whose peak is P dBFS rings at P + 20*log10(gain/100) dBFS: above 0 the mixer CLIPS (distortion).
This tool MEASURES that (ffmpeg volumedetect on the real files, the gain read from the real table) and prints, per cue, the peak, the
level after the gain and what a limiter at -1 dBFS would give instead. It does not decide anything: exit 0 and a WARN line when a cue
would clip (the user's answer to Q-audio-select-gain picks the fix), exit 1 only when a file or the table cannot be read.

  python check_audio_headroom.py [--root <worktree>] [--audio <MyCrusader_Audio>]
  python check_audio_headroom.py --selftest      (a -4 dB tone must clip at 250 and not at 100: proves the measure)
"""
import math
import os
import re
import subprocess
import sys
import tempfile

AUDIO = os.environ.get("HOL_AUDIO_ROOT", r"C:\Users\baudo\Documents\MyCrusader_Audio")
FFMPEG = os.environ.get("FFMPEG", "ffmpeg")


def volumedetect(path, pre=""):
    """(max_volume dB, mean_volume dB) of a file, after the optional ffmpeg filter chain `pre`."""
    chain = (pre + "," if pre else "") + "volumedetect"
    out = subprocess.run([FFMPEG, "-hide_banner", "-i", path, "-af", chain, "-f", "null", "-"],
                         capture_output=True, text=True).stderr
    mx = re.search(r"max_volume: (-?[\d.]+) dB", out)
    mean = re.search(r"mean_volume: (-?[\d.]+) dB", out)
    if not mx or not mean:
        raise RuntimeError("volumedetect gave nothing for " + path)
    return float(mx.group(1)), float(mean.group(1))


def gain_db(percent):
    return 20.0 * math.log10(percent / 100.0)


def elected_cues(table_text):
    """The cues played above 100 %: [(name, pack, stem, percent)]. The gain constant is resolved by name."""
    consts = dict((m.group(1), int(m.group(2))) for m in re.finditer(r"const int (\w+)\s*=\s*(\d+);", table_text))
    cues = []
    for m in re.finditer(r'Add\(Cues, HOLAudioEvent::\w+, "([\w.]+)", "([\w-]*)", "([\w,]*)", (\w+),', table_text):
        name, pack, stems, vol = m.groups()
        percent = int(vol) if vol.isdigit() else consts.get(vol, 100)
        if percent > 100 and pack and stems:
            for stem in stems.split(","):
                cues.append((name, pack, stem, percent))
    return cues


def selection_cues(table_text):
    """Every building selection cue of the table, whatever its gain: [(name, pack, stem, percent)]."""
    consts = dict((m.group(1), int(m.group(2))) for m in re.finditer(r"const int (\w+)\s*=\s*(\d+);", table_text))
    cues = []
    for m in re.finditer(r'Add\(Cues, HOLAudioEvent::\w+, "(Select\.Building\w+)", "([\w-]*)", "([\w,]*)", (\w+),', table_text):
        name, pack, stems, vol = m.groups()
        percent = int(vol) if vol.isdigit() else consts.get(vol, 100)
        for stem in stems.split(","):
            if pack and stem:
                cues.append((name, pack, stem, percent))
    return cues


# The ceiling of the limited files (Q-audio-select-gain = B, 2026-11-09): -1 dBFS, a tenth of a dB of tolerance for the meter.
CEILING_DB = -0.9


def strict(root, audio):
    """Q-audio-select-gain = B: no building selection sound may ring above -1 dBFS AT THE GAIN OF THE TABLE. Exit 1 when one does."""
    table = os.path.join(root, "Script", "Audio", "HOLAudioTable.as")
    cues = selection_cues(open(table, encoding="utf-8").read())
    if not cues:
        print("FAIL : no Select.Building cue found in the table (the pattern or the table changed)")
        return 1
    bad = 0
    for name, pack, stem, percent in cues:
        path = find_file(audio, pack, stem)
        if not path:
            print("FAIL : file not found for %s: %s/%s under %s" % (name, pack, stem, audio))
            return 1
        peak, mean = volumedetect(path)
        after = peak + gain_db(percent)
        ok = after <= CEILING_DB
        bad += 0 if ok else 1
        print("  %-24s %-20s x%.2f: file peak %.1f -> rings at %+.1f dBFS, mean %.1f dB (%s)"
              % (name, stem, percent / 100.0, peak, after, mean + gain_db(percent), "ok" if ok else "ABOVE -1 dBFS"))
    print("check_audio_headroom --strict: %s (%d sounds, %d above the -1 dBFS ceiling)" % ("PASS" if not bad else "FAIL", len(cues), bad))
    return 1 if bad else 0


def find_file(audio, pack, stem):
    if pack == "soft":
        candidates = [os.path.join(audio, "Candidates", "soft", stem + ".wav")]
    else:
        candidates = []
        base = os.path.join(audio, "VSCO" if pack == "vsco" else os.path.join("Kenney", pack))
        for dirpath, _, files in os.walk(base):
            for f in files:
                if os.path.splitext(f)[0] == stem and f.lower().endswith((".ogg", ".wav")):
                    candidates.append(os.path.join(dirpath, f))
    for c in candidates:
        if os.path.exists(c):
            return c
    return None


def report(root, audio):
    table = os.path.join(root, "Script", "Audio", "HOLAudioTable.as")
    if not os.path.exists(table):
        print("FAIL : no table at " + table)
        return 1
    cues = elected_cues(open(table, encoding="utf-8").read())
    if not cues:
        print("no cue above 100 % in the table: nothing can clip by gain (the elected sounds are limited files at 100 %, Q-audio-select-gain = B)")
        print("check_audio_headroom: PASS")
        return 0
    clip = 0
    for name, pack, stem, percent in cues:
        path = find_file(audio, pack, stem)
        if not path:
            print("FAIL : file not found for %s: %s/%s under %s" % (name, pack, stem, audio))
            return 1
        peak, mean = volumedetect(path)
        g = gain_db(percent)
        after = peak + g
        # What a limiter would do with the same gain: ceiling -1 dBFS (0.891), fast attack, 100 ms release.
        lpeak, lmean = volumedetect(path, "volume=%.2fdB,alimiter=limit=0.891:attack=1:release=100:level=disabled" % g)
        verdict = "CLIPS by %.1f dB" % after if after > 0 else "ok"
        if after > 0:
            clip += 1
        print("  %-24s %-16s x%.2f (+%.1f dB): peak %.1f -> %+.1f dBFS (%s); mean %.1f -> %.1f dB; with a -1 dBFS limiter: peak %.1f, mean %.1f"
              % (name, stem, percent / 100.0, g, peak, after, verdict, mean, mean + g, lpeak, lmean))
    print("  %d sounds, %d would clip at the table's gain" % (len(cues), clip))
    if clip:
        print("check_audio_headroom: WARN (%d clip; the fix is the user's choice, Q-audio-select-gain)" % clip)
    else:
        print("check_audio_headroom: PASS")
    return 0


def selftest():
    d = tempfile.mkdtemp(prefix="headroom_")
    wav = os.path.join(d, "tone.wav")
    # A 440 Hz tone whose peak is -4 dB, like every soft_candidate (make_soft_candidates.py peaks at -4 dB).
    subprocess.run([FFMPEG, "-hide_banner", "-y", "-f", "lavfi", "-i", "sine=frequency=440:duration=0.5", "-af", "volume=14dB", wav],   # sine=... is -18 dBFS; +14 dB makes about -4
                   capture_output=True)
    peak, _ = volumedetect(wav)
    ok = abs(peak - (-4.0)) < 0.6
    print("  tone peak %.1f dB (expected about -4): %s" % (peak, "ok" if ok else "BAD"))
    at100 = peak + gain_db(100)
    at250 = peak + gain_db(250)
    ok = ok and at100 <= 0 and at250 > 0
    print("  at 100 %%: %+.1f dBFS (must not clip); at 250 %%: %+.1f dBFS (must clip): %s" % (at100, at250, "ok" if (at100 <= 0 and at250 > 0) else "BAD"))
    # The table parser on a cut-down table: a constant gain, a literal 80, a silent cue.
    text = ('const int G = 250;\n'
            'Add(Cues, HOLAudioEvent::A, "Select.A", "soft", "s_a", G, 97, 103, 60, 1);\n'
            'Add(Cues, HOLAudioEvent::B, "Alert.B", "rpg-audio", "b", 80, 100, 100, 0, 2);\n'
            'Add(Cues, HOLAudioEvent::C, "Ui.C", "", "", 150, 100, 100, 0, 0);\n')
    found = elected_cues(text)
    ok = ok and found == [("Select.A", "soft", "s_a", 250)]
    print("  parser on a cut-down table: %s -> %s" % (found, "ok" if found == [("Select.A", "soft", "s_a", 250)] else "BAD"))
    print("check_audio_headroom selftest: " + ("PASS" if ok else "FAIL"))
    return 0 if ok else 1


if __name__ == "__main__":
    args = sys.argv[1:]
    if "--selftest" in args:
        sys.exit(selftest())
    root = args[args.index("--root") + 1] if "--root" in args else os.getcwd()
    audio = args[args.index("--audio") + 1] if "--audio" in args else AUDIO
    if "--strict" in args:
        sys.exit(strict(root, audio))
    sys.exit(report(root, audio))
