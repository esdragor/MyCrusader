"""The recorded instruments of the hero's fanfares (make_hero_fanfare.py): a few of VSCO 2 Community Edition's samples
(Versilian Studios, CC0 1.0, https://github.com/sgossner/VSCO-2-CE), fetched once into a cache outside the repository.

  load(cache) -> {instrument: [Sample]}   downloads what is missing, reads each sample, finds its real pitch

The bank names octaves its own way per instrument, so a sample's pitch is measured (autocorrelation), not read from
its name; drums and cymbals keep pitch 0. Samples are returned mono, float, at RATE, their silence before the attack cut.
"""
import os
import re
import urllib.parse
import urllib.request
import wave

import numpy as np

RATE = 48000
RAW = "https://raw.githubusercontent.com/sgossner/VSCO-2-CE/master/"

# Instrument -> (folder, file pattern): the loudest layer of each note, one round robin.
WANTED = {
    "trumpet": ("Brass/Trumpet/sus", r"Sum_SHTrumpet_sus_[A-G]#?\d_v3_rr1\.wav"),
    "trumpet_stac": ("Brass/Trumpet/stac", r"Sum_SHTrumpet_stac_[A-G]#?\d_v3_rr1\.wav"),
    "horn": ("Brass/F Horn/sus", r"MOHorn_sus_[A-G]#?\d_v3_1\.wav"),
    "trombone": ("Brass/Tenor Trombone/sus", r"tenortbn_sus_[A-G]#?\d_v3_1\.wav"),
    "tuba": ("Brass/Tuba/sus", r"Tuba3_sus_[A-G]#?\d_v3_rr1_Mid\.wav"),
    "violins": ("Strings/Violin Section/susVib", r"VlnEns_susVib_[A-G]#?\d_v2\.wav"),
    "violas": ("Strings/Viola Section/susvib", r"ViolaEns_susvib_[A-G]#?\d_v2_1\.wav"),
    "cellos": ("Strings/Cello Section/susvib", r"susvib_[A-G]#?\d_v3_1\.wav"),
    "basses": ("Strings/Solo Contrabass/SusVib", r"BKCtbss_SusVib_[A-G]#?\d_v3_rr1\.wav"),
    "timpani": ("Percussion/Timpani", r"Timpani\d_Hit_v4_rr1_Sum\.wav"),
    "timpani_roll": ("Percussion/Timpani/Rolls", r"Timpani\d_Roll_v5_rr1_Sum\.wav"),
    "crash": ("Percussion", r"cymbal-crash1_ff_rr1\.wav"),
    "swell": ("Percussion", r"susCymb1-cresc-Short_v1\.wav"),
    "gong": ("Percussion", r"gongHit_f\.wav"),
}

# Files of each folder (from the repository's tree, 2026-10-29), so no API call is needed.
FILES = {
    "trumpet": ["A#3", "A2", "A4", "C3", "C5", "D#3", "D4", "F2", "F4", "G3"],
    "horn": ["A#1", "A2", "C1", "C3", "D#1", "D2", "F2", "G1"],
    "trombone": ["A#1", "C#3", "C3", "D#1", "D#3", "D2", "F1", "F2", "F3"],
    "tuba": ["A#0", "A#1", "A#2", "D#1", "D2", "F1", "F2"],
    "violins": ["A2", "A3", "B2", "B4", "C4", "D3", "D5", "E4", "F#3", "G2", "G4"],
    "violas": ["A3", "B2", "B4", "C2", "C4", "D2", "D3", "D5", "E2", "E4", "F3", "G2", "G4"],
    "cellos": ["A2", "B1", "B3", "C1", "C3", "D2", "D4", "E3", "F2", "F4", "G1", "G3"],
    "basses": ["A#0", "A1", "B2", "C#2", "C1", "D1", "E1", "E2", "F#0", "F#1", "G#1", "G#2", "G0"],
}
NAMES = {
    "trumpet": "Sum_SHTrumpet_sus_{}_v3_rr1.wav",
    "trumpet_stac": "Sum_SHTrumpet_stac_{}_v3_rr1.wav",
    "horn": "MOHorn_sus_{}_v3_1.wav",
    "trombone": "tenortbn_sus_{}_v3_1.wav",
    "tuba": "Tuba3_sus_{}_v3_rr1_Mid.wav",
    "violins": "VlnEns_susVib_{}_v2.wav",
    "violas": "ViolaEns_susvib_{}_v2_1.wav",
    "cellos": "susvib_{}_v3_1.wav",
    "basses": "BKCtbss_SusVib_{}_v3_rr1.wav",
}
SINGLE = {
    # Drums from the lowest; their pitch is left as recorded (a drum, not a note).
    "timpani": ["Timpani1_Hit_v3_rr1_Sum.wav"] + ["Timpani%d_Hit_v4_rr1_Sum.wav" % i for i in range(2, 6)],
    "timpani_roll": ["Timpani%d_Roll_v5_rr1_Sum.wav" % i for i in range(1, 5)] + ["Timpani5_Roll_v4_rr1_Sum.wav"],
    "crash": ["cymbal-crash1_ff_rr1.wav"],
    "swell": ["susCymb1-cresc-Short_v1.wav"],
    "gong": ["gongHit_f.wav"],
}


class Sample:
    def __init__(self, data, pitch):
        self.data = data
        self.pitch = pitch


def files(instrument):
    if instrument in SINGLE:
        return SINGLE[instrument]
    notes = FILES["trumpet" if instrument == "trumpet_stac" else instrument]
    return [NAMES[instrument].format(note) for note in notes]


def fetch(cache, folder, name):
    path = os.path.join(cache, folder.replace("/", os.sep), name)
    if not os.path.exists(path):
        os.makedirs(os.path.dirname(path), exist_ok=True)
        url = RAW + urllib.parse.quote(folder + "/" + name)
        with urllib.request.urlopen(url) as response, open(path + ".part", "wb") as out:
            out.write(response.read())
        os.replace(path + ".part", path)
    return path


def read_wav(path):
    with wave.open(path, "rb") as file:
        channels, width, rate = file.getnchannels(), file.getsampwidth(), file.getframerate()
        raw = file.readframes(file.getnframes())
    if width == 3:
        bytes_ = np.frombuffer(raw, dtype=np.uint8).reshape(-1, 3)
        ints = (bytes_[:, 0].astype(np.int32) | (bytes_[:, 1].astype(np.int32) << 8) | (bytes_[:, 2].astype(np.int32) << 16))
        ints = np.where(ints >= 1 << 23, ints - (1 << 24), ints)
        data = ints / float(1 << 23)
    elif width == 2:
        data = np.frombuffer(raw, dtype="<i2") / 32768.0
    elif width == 4:
        data = np.frombuffer(raw, dtype="<i4") / float(1 << 31)
    else:
        raise RuntimeError("%s: %d-byte samples" % (path, width))
    data = data.reshape(-1, channels).mean(axis=1)
    if rate != RATE:
        positions = np.arange(int(len(data) * RATE / rate)) * rate / RATE
        data = np.interp(positions, np.arange(len(data)), data)
    # Cut the silence before the attack (2 ms kept).
    level = np.abs(data)
    start = int(np.argmax(level > level.max() * 0.03))
    return data[max(0, start - int(0.002 * RATE)):].astype(np.float64)


def measure_pitch(data):
    """Fundamental of a held note (Hz), by autocorrelation of 0.3 s taken after the attack."""
    piece = data[int(0.25 * RATE):int(0.55 * RATE)]
    if len(piece) < 4096:
        piece = data[:int(0.3 * RATE)]
    piece = piece - piece.mean()
    spectrum = np.fft.rfft(piece, 2 * len(piece))
    corr = np.fft.irfft(spectrum * np.conj(spectrum))[:len(piece)]
    low, high = int(RATE / 1200.0), int(RATE / 25.0)
    lag = low + int(np.argmax(corr[low:high]))
    # Prefer the shortest lag nearly as strong (octave errors).
    for divisor in (4, 3, 2):
        short = lag // divisor
        if short >= low and corr[short - 2:short + 3].max() > 0.85 * corr[lag]:
            lag = short - 2 + int(np.argmax(corr[short - 2:short + 3]))
            break
    return RATE / lag


def load(cache):
    bank = {}
    for instrument, (folder, _) in WANTED.items():
        samples = []
        for name in files(instrument):
            data = read_wav(fetch(cache, folder, name))
            pitched = instrument not in SINGLE
            samples.append(Sample(data, measure_pitch(data) if pitched else 0.0))
        bank[instrument] = samples
    return bank


if __name__ == "__main__":
    import sys
    loaded = load(sys.argv[1] if len(sys.argv) > 1 else "vsco_cache")
    for key, samples in loaded.items():
        print("%-13s %s" % (key, ", ".join("%.1f" % s.pitch for s in samples)))
