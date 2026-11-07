"""The SOFT candidates (AudioInProgress.md, AudioBuildingsInProgress.md; user's rule of 2026-11-07, "Sons : pas usants": the sounds are heard
very often and for a long time, so low, dull, short, soft; no trumpet, horn, little bell, nothing high-pitched; the "medieval" brass and
bell rounds were refused).

  python make_soft_candidates.py [out_dir]

Every stem "s_..." named in Script/Audio/HOLAudioCandidates.as is made here from a CC0 recording (Kenney rpg-audio / impact-sounds .ogg, or a
VSCO 2 CE candidate wav of make_ui_candidates.py, licences in Docs/assets_audio.csv) by ONE recipe: silence before the attack removed,
pitch lowered by a factor (the sound also gets longer), three low-pass filters (36 dB per octave), cut to a short length, soft fade out,
peak at -4 dB. Output: 16-bit mono 48 kHz WAV `<out_dir>\\<stem>.wav` (default MyCrusader_Audio\\Candidates\\soft). ffmpeg does the signal
work. Tools/Audio/check_audio_soft.py then measures the files against the rule. Nothing is chosen here: the user picks by ear.
"""
import os
import subprocess
import sys
import tempfile
import wave

import numpy as np

AUDIO = os.environ.get("HOL_AUDIO_ROOT", r"C:\Users\baudo\Documents\MyCrusader_Audio")
RATE = 48000
PEAK_DB = -4.0
FADE = 0.09

# stem -> (source stem, pitch factor, low-pass Hz, seconds kept). A source starting with v_ is a VSCO candidate wav, the others Kenney .ogg.
RECIPES = {
    # Alert.TrainingDone, round 3: a unit is ready. Thuds of leather, wood, stone, felt, muffled metal, a soft drum, a plucked bass.
    "s_train_leather_thud": ("handleSmallLeather", 0.7, 1100, 0.30),
    "s_train_wood_knock": ("impactWood_medium_000", 0.6, 900, 0.30),
    "s_train_plank_dull": ("impactPlank_medium_000", 0.7, 800, 0.30),
    "s_train_drum_soft": ("v_timpani_hit", 1.0, 350, 0.55),
    "s_train_bass_pluck": ("v_bass_pizz_E1", 1.0, 450, 0.55),
    "s_train_mallet_dull": ("v_mallet_hit_mf", 1.0, 450, 0.45),
    "s_train_stone_tap": ("impactMining_000", 0.7, 1000, 0.30),
    "s_train_shield_muffled": ("impactPlate_heavy_000", 0.6, 600, 0.35),
    "s_train_cloth_thump": ("impactSoft_heavy_000", 0.7, 700, 0.30),
    "s_train_book_close": ("bookClose", 0.8, 900, 0.35),
    # Building.Wood (House, LumberCamp, MiningCamp, Mill, Farm)
    "s_wood_light": ("impactWood_light_000", 0.7, 1400, 0.25),
    "s_wood_medium": ("impactWood_medium_001", 0.8, 1200, 0.30),
    "s_wood_heavy": ("impactWood_heavy_000", 0.7, 800, 0.35),
    "s_wood_plank": ("impactPlank_medium_001", 0.7, 900, 0.30),
    "s_wood_chop": ("chop", 0.6, 800, 0.30),
    "s_wood_door": ("doorOpen_1", 0.8, 900, 0.40),
    "s_wood_creak_low": ("creak1", 0.6, 700, 0.40),
    "s_wood_step": ("footstep_wood_000", 0.7, 900, 0.25),
    "s_wood_slap_low": ("v_slapstick_quiet2", 0.6, 600, 0.25),
    "s_wood_deep_knock": ("impactWood_light_002", 0.5, 800, 0.30),
    # Building.Stone (MainCentre, Barracks, ArcheryRange, Stable, SiegeWorkshop)
    "s_stone_mine_a": ("impactMining_000", 0.6, 900, 0.30),
    "s_stone_mine_b": ("impactMining_001", 0.7, 1000, 0.30),
    "s_stone_mine_c": ("impactMining_002", 0.8, 1000, 0.30),
    "s_stone_mine_d": ("impactMining_003", 0.5, 800, 0.35),
    "s_stone_punch_heavy": ("impactPunch_heavy_000", 0.6, 600, 0.35),
    "s_stone_punch_soft": ("impactPunch_medium_000", 0.6, 700, 0.30),
    "s_stone_door": ("doorClose_1", 0.6, 700, 0.40),
    "s_stone_thud": ("impactSoft_heavy_000", 0.6, 600, 0.35),
    "s_stone_drum": ("v_timpani_hit", 1.0, 250, 0.50),
    "s_stone_mallet": ("v_mallet_hit_ff", 1.0, 400, 0.50),
    # Building.Sacred (Chapel, Sanctuary): no little bell; low and warm
    "s_sacred_bass_low": ("v_bass_pizz_E0", 1.0, 350, 0.75),
    "s_sacred_cello": ("v_cello_pizz_B3", 1.0, 800, 0.60),
    "s_sacred_harp_b3": ("v_harp_B3", 1.0, 900, 0.70),
    "s_sacred_harp_d4": ("v_harp_D4", 1.0, 900, 0.70),
    "s_sacred_harp_octave": ("v_harp_D4", 0.5, 700, 0.75),
    "s_sacred_gong_low": ("v_gong_smack_low", 0.8, 300, 0.75),
    "s_sacred_drum": ("v_timpani_hit", 0.9, 300, 0.60),
    "s_sacred_book": ("bookClose", 0.7, 700, 0.40),
    "s_sacred_low_toll": ("impactBell_heavy_000", 0.5, 350, 0.75),
    "s_sacred_cloth": ("cloth2", 0.8, 800, 0.40),
    # Building.Metal (Forge): muffled metal, never a ring
    "s_metal_heavy": ("impactMetal_heavy_000", 0.6, 700, 0.40),
    "s_metal_medium": ("impactMetal_medium_000", 0.6, 800, 0.35),
    "s_metal_light_low": ("impactMetal_light_000", 0.5, 800, 0.30),
    "s_metal_plate_heavy": ("impactPlate_heavy_000", 0.5, 600, 0.40),
    "s_metal_plate_medium": ("impactPlate_medium_000", 0.7, 700, 0.35),
    "s_metal_pot": ("metalPot1", 0.6, 700, 0.40),
    "s_metal_latch": ("metalLatch", 0.6, 900, 0.30),
    "s_metal_tin": ("impactTin_medium_000", 0.5, 700, 0.35),
    "s_metal_gong_muffled": ("v_gong_smack_light", 0.7, 250, 0.60),
    "s_metal_mallet": ("v_mallet_hit_ff", 0.8, 500, 0.50),
    # Building.Coins (Market): a purse, not a jingle
    "s_coins_purse": ("handleCoins", 0.55, 700, 0.40),
    "s_coins_leather_b": ("handleSmallLeather2", 0.7, 900, 0.30),
    "s_coins_leather": ("handleSmallLeather", 0.8, 1200, 0.30),
    "s_coins_drop": ("dropLeather", 0.8, 1000, 0.35),
    "s_coins_tin_b": ("impactTin_medium_002", 0.5, 700, 0.30),
    "s_coins_counter": ("impactWood_light_001", 0.5, 700, 0.30),
    "s_coins_tin": ("impactTin_medium_001", 0.5, 900, 0.30),
    "s_coins_cloth": ("clothBelt", 0.8, 1000, 0.35),
    "s_coins_soft": ("impactSoft_medium_000", 0.7, 700, 0.30),
    "s_coins_slap": ("v_slapstick_quiet1", 0.6, 600, 0.25),
}


def source_path(name):
    if name.startswith("v_"):
        return os.path.join(AUDIO, "Candidates", "vsco", name + ".wav")
    for pack in ("rpg-audio", "impact-sounds"):
        for folder, _, files in os.walk(os.path.join(AUDIO, "Kenney", pack)):
            if name + ".ogg" in files:
                return os.path.join(folder, name + ".ogg")
    raise RuntimeError("%s.ogg not found in Kenney rpg-audio / impact-sounds under %s" % (name, AUDIO))


def render(source, pitch, lowpass, seconds, out_path):
    """ffmpeg: mono, silence before the attack cut, pitch lowered, three low-pass filters; then numpy: cut, fade out, peak."""
    chain = ("aresample=%d,silenceremove=start_periods=1:start_threshold=-50dB,asetrate=%d,aresample=%d,"
             "lowpass=f=%d,lowpass=f=%d,lowpass=f=%d") % (RATE, int(RATE * pitch), RATE, lowpass, lowpass, lowpass)
    handle, temp = tempfile.mkstemp(suffix=".wav")
    os.close(handle)
    try:
        subprocess.run(["ffmpeg", "-y", "-v", "error", "-i", source, "-af", chain, "-ac", "1", "-ar", str(RATE), "-c:a", "pcm_s16le", temp],
                       check=True)
        with wave.open(temp, "rb") as wav:
            data = np.frombuffer(wav.readframes(wav.getnframes()), dtype="<i2").astype(np.float64) / 32768.0
    finally:
        os.remove(temp)
    data = data[:int(seconds * RATE)].copy()
    ramp = min(len(data), int(0.004 * RATE))
    data[:ramp] *= np.linspace(0.0, 1.0, ramp)
    fade = min(len(data), int(FADE * RATE))
    data[len(data) - fade:] *= np.linspace(1.0, 0.0, fade)
    peak = np.abs(data).max()
    data = data * (10 ** (PEAK_DB / 20) / peak if peak > 0 else 1.0)
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with wave.open(out_path, "wb") as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes((np.clip(data, -1, 1) * 32767).astype("<i2").tobytes())


def main():
    out_dir = sys.argv[1] if len(sys.argv) > 1 else os.path.join(AUDIO, "Candidates", "soft")
    for stem, (name, pitch, lowpass, seconds) in RECIPES.items():
        render(source_path(name), pitch, lowpass, seconds, os.path.join(out_dir, stem + ".wav"))
    print("Soft candidates: %d files in %s" % (len(RECIPES), out_dir))


if __name__ == "__main__":
    main()
