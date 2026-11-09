"""Writes the BEFORE / AFTER listening scenario of the building selection sounds: Tools/Scenarios/audio_select_gain.txt (+ .json, .look).
Q-audio-select-gain = B (2026-11-09): the old x2.5 clipped, the new sounds are limited files (+8 dB then a -1 dBFS limiter) at 100 %.
Enter = the next sound; the screen names the family (cue) and the version that just played. Per family: OLD, NEW, OLD, NEW (so the ear can alternate).
The sounds are played by hol.Audio.SelectGain (Script/Audio/HOLAudioSelectGain.as): NEW is read from the real table (HOLAudio::Plan).

  python make_audio_select_gain.py [--root project] [--scenarios dir]
"""
import argparse
import os
import shutil

NAME = "audio_select_gain"
FAMILIES = [("wood", "Bois", "Select.BuildingWood", "F, s_wood_door"),
            ("sacred", "Sacre", "Select.BuildingSacred", "H, s_sacred_book"),
            ("coins", "Marche", "Select.BuildingCoins", "handleCoins")]
LABEL = {"old": "ANCIEN x2,5 (ecrete)", "new": "NOUVEAU limite -1 dBFS, 100 %"}
ROUNDS = 2


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", default=os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")))
    parser.add_argument("--scenarios", default=None)
    args = parser.parse_args()
    scen = args.scenarios or os.path.join(args.root, "Tools", "Scenarios")
    steps = [(fam, ui, cue, src, version) for fam, ui, cue, src in FAMILIES for _ in range(ROUNDS) for version in ("old", "new")]
    out = [
        "# AVANT / APRES des sons de selection des batiments (Bois, Sacre, Marche) : %d sons, Entree = le suivant. Q-audio-select-gain = B." % len(steps),
        "# Plain, toi contre une IA, pour avoir l'ecran du jeu ; rien a cliquer. Rejouer : console (touche ²) puis hol.Audio.SelectGain <wood|sacred|coins> <old|new>.",
        "waitlog Starting hero of slot 0 spawned at",
        "waitlog Starting hero of slot 1 spawned at",
        "hol.Camera.FocusNear -900 300",
        "frames 10",
        "note AVANT / APRES : pour chaque famille (Bois, Sacre, Marche) l'ANCIEN son (x2,5, ecrete, sature) puis le NOUVEAU (limite a -1 dBFS, joue a 100 %), deux fois chacun. Entree = le son suivant.",
        "note Le nom du cue et la version qui viennent de jouer sont ecrits a chaque Entree. Reponds ensuite dans la fiche E-audio_select_gain.md : OK, ou REGLAGE: <famille, plus fort / moins fort / autre>.",
        "capture %s_start" % NAME,
        "waitkey Entree : commencer (Bois, ancien)",
    ]
    for n, (fam, ui, cue, src, version) in enumerate(steps, 1):
        out.append("hol.Audio.SelectGain %s %s" % (fam, version))
        out.append("waitlog select gain played %s %s" % (fam, version))
        if n < len(steps):
            nfam, nui, _, _, nversion = steps[n]
            nxt = "%s, %s" % (nui, LABEL[nversion])
        else:
            nxt = "fin"
        out.append("waitkey [%d/%d] %s [%s] (%s) : %s vient de jouer. Entree : %s" % (n, len(steps), ui, cue, src, LABEL[version], nxt))
    out += ["hol.Test.Sanity", "note Fin. Dis par famille si le NOUVEAU te convient (OK) ou REGLAGE: <ce qui ne va pas>."]
    os.makedirs(scen, exist_ok=True)
    with open(os.path.join(scen, NAME + ".txt"), "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(out) + "\n")
    with open(os.path.join(scen, NAME + ".look"), "w", encoding="utf-8", newline="\n") as f:
        f.write("# <nom de capture> | ce que l'utilisateur doit voir\n"
                "%s_start | L'ecran du jeu avec les notes du scenario AVANT / APRES des sons de selection des batiments, Entree = son suivant\n" % NAME)
    shutil.copyfile(os.path.join(args.root, "Tools", "Scenarios", "audio_ui.json"), os.path.join(scen, NAME + ".json"))
    print(NAME, len(steps), "sounds")


if __name__ == "__main__":
    main()
