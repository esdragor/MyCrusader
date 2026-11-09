"""Makes the LIMITED building selection sounds (Q-audio-select-gain = B, 2026-11-09).

The user asked for "volume x2 to x3" on the elected building selection sounds; played at x2.5 (+8 dB) they clipped (Wood and Sacred peak at
-4 dBFS, the Market's handleCoins at 0 dBFS). Answer B: bake the gain INTO the file with a limiter and play it at 100 %:

  ffmpeg: volume=+8dB, alimiter (ceiling -1 dBFS = 0.891, attack 1 ms, release 100 ms)

  Candidates\\soft\\s_wood_door.wav     -> Candidates\\soft\\s_wood_door_lim.wav      (Wood = F)
  Candidates\\soft\\s_sacred_book.wav   -> Candidates\\soft\\s_sacred_book_lim.wav    (Sacred = H)
  Kenney\\rpg-audio\\handleCoins.ogg    -> Candidates\\soft\\s_coins_select_lim.wav   (Market)

The sources stay outside the repository (C:\\Users\\baudo\\Documents\\MyCrusader_Audio, Docs/assets_audio.csv). Then import the waves as assets
with Tools/Editor/import_audio_elected.py (editor) and check with `check_audio_headroom.py --strict`.

  python make_limited_selects.py [--audio <MyCrusader_Audio>]
"""
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import check_audio_headroom as head  # noqa: E402

GAIN_DB = 20.0 * __import__("math").log10(2.5)   # x2.5 = +8 dB, the gain the user asked for
CHAIN = "volume=%.2fdB,alimiter=limit=0.891:attack=1:release=100:level=disabled" % GAIN_DB
# (source pack, source stem, output stem)
JOBS = [("soft", "s_wood_door", "s_wood_door_lim"),
        ("soft", "s_sacred_book", "s_sacred_book_lim"),
        ("rpg-audio", "handleCoins", "s_coins_select_lim")]


def main():
    args = sys.argv[1:]
    audio = args[args.index("--audio") + 1] if "--audio" in args else head.AUDIO
    out_dir = os.path.join(audio, "Candidates", "soft")
    for pack, stem, out_stem in JOBS:
        src = head.find_file(audio, pack, stem)
        if not src:
            print("FAIL : source not found: %s/%s under %s" % (pack, stem, audio))
            return 1
        dst = os.path.join(out_dir, out_stem + ".wav")
        run = subprocess.run([head.FFMPEG, "-hide_banner", "-y", "-i", src, "-af", CHAIN, "-c:a", "pcm_s16le", dst], capture_output=True, text=True)
        if run.returncode != 0 or not os.path.exists(dst):
            print("FAIL : ffmpeg for %s: %s" % (out_stem, run.stderr[-300:]))
            return 1
        before = head.volumedetect(src)
        after = head.volumedetect(dst)
        print("  %-20s peak %.1f -> %.1f dBFS, mean %.1f -> %.1f dB (+%.1f dB real)" % (out_stem, before[0], after[0], before[1], after[1], after[1] - before[1]))
    print("make_limited_selects: %d files in %s" % (len(JOBS), out_dir))
    return 0


if __name__ == "__main__":
    sys.exit(main())
