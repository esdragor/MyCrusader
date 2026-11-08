"""Writes the ONE listening panel of the audio sound trials (L9-10, 2026-11-08): Tools/Scenarios/audio_panel.txt (+ .json, .look).
The user listens to everything with one command: Enter = the next sound, the panel names the section, the cue and the letter that just played.
Sections: (1) the ELECTED building sounds at the gain of the game (250 %, "loud") then at 100 % to hear the clipping (wood F, sacred H, Market),
(2) stone (stone2 candidates), (3) metal (metal2 candidates), (4) the six alerts, (5) the training alert. The cues and stems are read from
Script/Audio/HOLAudioCandidates.as by make_audio_compare.py (same reader, same hol.Audio.Candidate command).

  python make_audio_panel.py [--root project] [--scenarios dir]
"""
import argparse
import os
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import make_audio_compare as base  # noqa: E402

NAME = "audio_panel"
ALERTS = ["Alert.UnderAttack", "Alert.BuildingDone", "Alert.TechDone", "Alert.Trade", "Alert.Volcano", "Alert.Diplomacy"]
# (cue, letter index, loud) of the elected building sounds: wood F, sacred H, the Market's handleCoins.
ELECTED = [("Select.BuildingWood", 5), ("Select.BuildingSacred", 7), ("Building.CoinsElected", 0)]


def sections(all_cues):
    stems = {cue: s for cue, s in all_cues}
    stems["Building.CoinsElected"] = ["handleCoins"]
    for cue in ("Building.Stone", "Building.Metal"):
        stems[cue] = next(s for c, s in all_cues if c == cue) if any(c == cue for c, _ in all_cues) else []
    full = lambda cue, loud=False: [(cue, i, stems[cue][i], loud) for i in range(len(stems[cue]))]
    elected = [(c, i, stems[c][0 if c == "Building.CoinsElected" else i], True) for c, i in ELECTED]
    elected += [(c, i, stems[c][0 if c == "Building.CoinsElected" else i], False) for c, i in ELECTED]
    result = [
        ("Elus a 250 % puis a 100 % (Bois F, Sacre H, Marche) : ecoute si ca ECRETE (craque, sature) a 250 %", elected),
        ("Pierre [Building.Stone] (nouveaux candidats stone2)", full("Building.Stone")),
        ("Metal [Building.Metal] (nouveaux candidats metal2)", full("Building.Metal")),
    ]
    for cue in ALERTS:
        result.append(("Alerte %s" % cue, full(cue)))
    result.append(("Entrainement fini [Alert.TrainingDone]", full("Alert.TrainingDone")))
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", default=os.path.abspath(os.path.join(HERE, "..", "..")))
    parser.add_argument("--scenarios", default=None)
    args = parser.parse_args()
    scen = args.scenarios or os.path.join(args.root, "Tools", "Scenarios")
    parts = sections(base.cues(args.root))
    total = sum(len(s) for _, s in parts)
    out = [
        "# Panneau d'ecoute UNIQUE des essais sonores : %d sons en %d sections, Entree = le son suivant. Sources CC0, jamais choisies a ta place." % (total, len(parts)),
        "# Plain, toi contre une IA, pour avoir l'ecran du jeu ; rien a cliquer. Reecouter : console (touche ²) puis hol.Audio.Candidate <Cue> <lettre> [loud].",
        "waitlog Starting hero of slot 0 spawned at",
        "waitlog Starting hero of slot 1 spawned at",
        "hol.Camera.FocusNear -900 300",
        "frames 10",
        "note Panneau d'ecoute unique : %d sons en %d sections. Entree = le son suivant ; le panneau dit la section, le cue et la lettre qui viennent de jouer." % (total, len(parts)),
        "note Tu peux t'arreter a tout moment. Reponds ensuite dans la fiche E-audio_panel.md : une lettre par cue (ou OK, ou REGLAGE: ...).",
        "capture %s_start" % NAME,
        "waitkey Entree : commencer",
    ]
    flat = [(n, section, step) for n, (section, steps) in enumerate(parts, 1) for step in steps]
    position = 0
    last_section = 0
    for n, section, (cue, index, stem, loud) in flat:
        if n != last_section:
            out.append("note Section %d/%d : %s" % (n, len(parts), section))
            last_section = n
        out.append("hol.Audio.Candidate %s %s%s" % (cue, base.LETTERS[index], " loud" if loud else ""))
        out.append("waitlog candidate played %s %s" % (cue, base.LETTERS[index]))
        position += 1
        if position < len(flat):
            nn, nsection, (ncue, nindex, _, nloud) = flat[position]
            nxt = "%s %s%s" % (ncue, base.LETTERS[nindex], " (250 %)" if nloud else "") + ("  [nouvelle section %d/%d]" % (nn, len(parts)) if nn != n else "")
        else:
            nxt = "fin"
        out.append("waitkey [%d/%d, section %d/%d] %s %s (%s%s) vient de jouer. Entree : %s" % (position, total, n, len(parts), cue, base.LETTERS[index], stem, ", 250 %" if loud else "", nxt))
    out += ["hol.Test.Sanity", "note Fin du panneau. Dis par cue ta lettre preferee, ou REGLAGE: <ce qui ne va pas>."]
    os.makedirs(scen, exist_ok=True)
    with open(os.path.join(scen, NAME + ".txt"), "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(out) + "\n")
    with open(os.path.join(scen, NAME + ".look"), "w", encoding="utf-8", newline="\n") as f:
        f.write("# <nom de capture> | ce que l'utilisateur doit voir\n"
                "%s_start | L'ecran du jeu avec le panneau du scenario : notes du panneau d'ecoute unique, Entree = son suivant\n" % NAME)
    shutil.copyfile(os.path.join(args.root, "Tools", "Scenarios", "audio_ui.json"), os.path.join(scen, NAME + ".json"))
    print(NAME, total, "steps,", len(parts), "sections")


if __name__ == "__main__":
    main()
