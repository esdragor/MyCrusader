"""The VSCO 2 CE candidates for the interface sounds (AudioInProgress.md, user's REGLAGE of 2026-11-05: the Kenney
"interface-sounds" pack sounds like a spaceship console, the interface must sound medieval fantasy).

  python make_ui_candidates.py [cache_dir] [out_dir]

Fetches a few plucked strings, bells, a drum and wood clacks of VSCO 2 Community Edition (Versilian Studios, CC0 1.0,
https://github.com/sgossner/VSCO-2-CE, LICENSE kept next to the samples) into a cache OUTSIDE the repository, cuts each to a
short sound (silence before the attack removed, fade out, peak at -2 dB) and writes 16-bit mono WAV files
`<out_dir>\\<token>.wav` (default out_dir C:\\Users\\baudo\\Documents\\MyCrusader_Audio\\Candidates\\vsco).
The tokens (v_...) are the ones Script/Audio/HOLAudioCandidates.as names; Tools/Audio/check_audio_table.ps1 -Candidates
verifies that each of them exists. Nothing is chosen here: the user picks by ear.
"""
import os
import sys
import urllib.parse
import urllib.request
import wave

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import vsco_samples as vsco  # fetch(), read_wav(), RATE

AUDIO = r"C:\Users\baudo\Documents\MyCrusader_Audio"
HARP = "Strings/Harp"
PIZZ = "Strings/Solo Violin/Pizz"
CELLO = "Strings/Cello Section/pizzT"
BASS = "Strings/Solo Contrabass/Pizz"
GLOCK = "Percussion/Glock"
NEPAL = "Miscellania Raw/Misc 2/NepaleseBells"
WOOD = "VSCO 1 Percussion/varWood"
GONG = "VSCO 1 Percussion/varMetal/Gong"
TRI = "VSCO 1 Percussion/varMetal/triangle/1"
MALLET = "VSCO 1 Percussion/drums/other/ethnic/giant/mallet"

# token -> (folder, file, seconds kept). A token with a list of notes is an arpeggio (each note 0.11 s after the one before).
SINGLES = {
    "v_harp_A4": (HARP, "KSHarp_A4_mf.wav", 1.2), "v_harp_B3": (HARP, "KSHarp_B3_mf.wav", 1.2),
    "v_harp_C5": (HARP, "KSHarp_C5_mf.wav", 1.0), "v_harp_D4": (HARP, "KSHarp_D4_mf.wav", 1.2),
    "v_harp_D6": (HARP, "KSHarp_D6_mf.wav", 0.7), "v_harp_E5": (HARP, "KSHarp_E5_mf.wav", 1.0),
    "v_harp_F6": (HARP, "KSHarp_F6_mf.wav", 0.6), "v_harp_G5": (HARP, "KSHarp_G5_mf.wav", 0.9),
    "v_harp_A6": (HARP, "KSHarp_A6_mf.wav", 0.5),
    "v_vln_pizz_A4": (PIZZ, "LLVln_Pizz_A4_f_RR1.wav", 0.5), "v_vln_pizz_A5": (PIZZ, "LLVln_Pizz_A5_f_RR1.wav", 0.5),
    "v_vln_pizz_A6": (PIZZ, "LLVln_Pizz_A6_f_RR1.wav", 0.4), "v_vln_pizz_C5": (PIZZ, "LLVln_Pizz_C5_f_RR1.wav", 0.5),
    "v_vln_pizz_E5": (PIZZ, "LLVln_Pizz_E5_f_RR1.wav", 0.5),
    "v_cello_pizz_B3": (CELLO, "pizzT_B3_v2_RR1.wav", 0.7), "v_cello_pizz_D4": (CELLO, "pizzT_D4_v2_RR1.wav", 0.6),
    "v_bass_pizz_E1": (BASS, "BKCtbss_Pizz_E1_v3_rr1.wav", 0.9), "v_bass_pizz_E0": (BASS, "BKCtbss_Pizz_E0_v3_rr1.wav", 1.2),
    "v_glock_G4": (GLOCK, "glock_medium_G4.wav", 1.0), "v_glock_C5": (GLOCK, "glock_medium_C5.wav", 1.0),
    "v_glock_G5": (GLOCK, "glock_medium_G5.wav", 0.9), "v_glock_C6": (GLOCK, "glock_medium_C6.wav", 0.8),
    "v_glock_G6": (GLOCK, "glock_medium_G6.wav", 0.6), "v_glock_C7": (GLOCK, "glock_medium_C7.wav", 0.4),
    "v_nbell_s01": (NEPAL, "fx1_short_r01_main.wav", 1.2), "v_nbell_s02": (NEPAL, "fx1_short_r04_main.wav", 0.9),
    "v_nbell_s03": (NEPAL, "fx1_short_r07_main.wav", 0.9), "v_nbell_s04": (NEPAL, "fx1_short_r11_main.wav", 1.0),
    "v_nbell_s05": (NEPAL, "fx2_short_r01_main.wav", 1.4), "v_nbell_s06": (NEPAL, "fx2_short_r05_main.wav", 1.2),
    "v_nbell_s07": (NEPAL, "fx3_short_r01_main.wav", 1.0), "v_nbell_s08": (NEPAL, "fx3_short_r02_main.wav", 1.2),
    "v_nbell_med": (NEPAL, "fx_med_main.wav", 1.8),
    "v_triangle_hit": (TRI, "triangle1_hit_mp.wav", 1.2), "v_triangle_muted": (TRI, "triangle1_hit_mp_muted1.wav", 0.3),
    "v_claves_mf": (WOOD, "claves_mf.wav", 0.25), "v_claves_mp": (WOOD, "claves_mp.wav", 0.25),
    "v_slapstick_quiet1": (WOOD, "slapstick_quiet1.wav", 0.25), "v_slapstick_quiet2": (WOOD, "slapstick_quiet2.wav", 0.3),
    "v_tambourine_down": (WOOD, "tambourine_Down.wav", 0.5),
    "v_mallet_hit_mf": (MALLET, "EthnicLargeMallet_hit_mf_1.wav", 0.9), "v_mallet_hit_ff": (MALLET, "EthnicLargeMallet_hit_ff_1.wav", 1.1),
    "v_mallet_hit_ff2": (MALLET, "EthnicLargeMallet_hit_ff_2.wav", 1.1),
    "v_gong_smack_light": (GONG, "gong_smack_light1.wav", 1.6), "v_gong_smack_low": (GONG, "gong_smack_low_1.wav", 2.2),
}
ARPEGGIOS = {
    "v_harp_arp_up": ["v_harp_C5", "v_harp_E5", "v_harp_G5"],
    "v_harp_arp_down": ["v_harp_G5", "v_harp_E5", "v_harp_C5"],
    "v_harp_arp_low": ["v_harp_D4", "v_harp_A4", "v_harp_D6"],
    "v_glock_arp_up": ["v_glock_C5", "v_glock_G5", "v_glock_C6"],
    "v_vln_pizz_arp": ["v_vln_pizz_C5", "v_vln_pizz_E5", "v_vln_pizz_A5"],
}
RATE = vsco.RATE


def cut(data, seconds):
    data = data[:int(seconds * RATE)].copy()
    fade = min(len(data), int(0.06 * RATE))
    data[len(data) - fade:] *= np.linspace(1.0, 0.0, fade)
    peak = np.abs(data).max()
    return data * (10 ** (-2 / 20) / peak if peak > 0 else 1.0)


def write(path, data):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with wave.open(path, "wb") as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes((np.clip(data, -1, 1) * 32767).astype("<i2").tobytes())


def main():
    cache = sys.argv[1] if len(sys.argv) > 1 else os.path.join(AUDIO, "VSCO")
    out_dir = sys.argv[2] if len(sys.argv) > 2 else os.path.join(AUDIO, "Candidates", "vsco")
    license_path = os.path.join(cache, "LICENSE")
    if not os.path.exists(license_path):
        os.makedirs(cache, exist_ok=True)
        with urllib.request.urlopen(vsco.RAW + "LICENSE") as response, open(license_path, "wb") as out:
            out.write(response.read())
    made = {}
    for token, (folder, name, seconds) in SINGLES.items():
        made[token] = cut(vsco.read_wav(vsco.fetch(cache, folder, name)), seconds)
        write(os.path.join(out_dir, token + ".wav"), made[token])
    for token, notes in ARPEGGIOS.items():
        step = int(0.11 * RATE)
        mix = np.zeros(step * (len(notes) - 1) + max(len(made[n]) for n in notes))
        for index, note in enumerate(notes):
            mix[index * step:index * step + len(made[note])] += made[note]
        write(os.path.join(out_dir, token + ".wav"), cut(mix, 1.6))
    print("VSCO candidates: %d files in %s" % (len(SINGLES) + len(ARPEGGIOS), out_dir))


if __name__ == "__main__":
    main()
