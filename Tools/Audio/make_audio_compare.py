"""Writes the three scenarios that let the user LISTEN to the candidates of every interface cue (AudioInProgress.md, method of the
ten samples): Tools/Scenarios/audio_compare.txt (the 10 cues of the interface), audio_compare_orders.txt (5 orders + 2 selections),
audio_compare_alerts.txt (7 alerts), each with its .json (the audio_ui match) and .look. For each cue the ten variants A to J play one
after the other: Enter = the next variant (the panel says which one just played and which comes next); hol.Audio.Candidate <Cue> <A-J>
in the console plays one again. The cues and stems are read from Script/Audio/HOLAudioCandidates.as. Run: python make_audio_compare.py
"""
import os
import re
import shutil

HERE = os.path.dirname(os.path.abspath(__file__))
PROJECT = os.path.abspath(os.path.join(HERE, "..", ".."))
SCEN = os.path.join(PROJECT, "Tools", "Scenarios")
LINE = re.compile(r'^\s*Add\(List,\s*"(?P<cue>[A-Za-z]+\.[A-Za-z]+)",\s*"(?P<stems>[A-Za-z0-9_,]+)"')
GROUPS = [
    ("audio_compare", ("Ui.",), "les 10 sons de l'interface (clic, survol, ouvrir, fermer, retour, erreur, confirmer, bascule, tic, defilement)"),
    ("audio_compare_orders", ("Order.", "Select."), "les 5 ordres (deplacer, attaquer, recolter, construire, garnison) et les 2 selections (unite, batiment)"),
    ("audio_compare_alerts", ("Alert.",), "les 7 alertes (attaque, batiment fini, entrainement fini, technologie, commerce, volcan, diplomatie)"),
]


def cues():
    result = []
    with open(os.path.join(PROJECT, "Script", "Audio", "HOLAudioCandidates.as"), encoding="utf-8") as source:
        for line in source:
            match = LINE.match(line)
            if match:
                result.append((match.group("cue"), match.group("stems").split(",")))
    return result


def write(name, prefixes, what, all_cues):
    mine = [c for c in all_cues if c[0].startswith(prefixes)]
    steps = [(cue, index, stems[index]) for cue, stems in mine for index in range(len(stems))]
    label = lambda s: "%s %s" % (s[0], "ABCDEFGHIJ"[s[1]])
    out = [
        "# Ecoute : %s, dix variantes MEDIEVAL FANTASY chacune (bois, cuir, parchemin, metal sourd, cloche, pierre, cordes), Entree = variante suivante" % what,
        "# Plain, toi contre une IA, pour avoir l'ecran du jeu ; rien a cliquer : le panneau dit quelle variante vient de jouer. Sources CC0 (Kenney rpg-audio et impact-sounds, VSCO 2 CE), jamais choisies a ta place.",
        "waitlog Starting hero of slot 0 spawned at",
        "waitlog Starting hero of slot 1 spawned at",
        "hol.Camera.FocusNear -900 300",
        "frames 10",
        "note Ecoute %s. %d sons, %d cues, dix variantes A a J chacune." % (what, len(steps), len(mine)),
        "note Entree = la variante suivante (le panneau dit laquelle vient de jouer). Pour reecouter : console (touche ²) puis hol.Audio.Candidate <Cue> <lettre>, par exemple hol.Audio.Candidate Ui.Open C.",
        "note Reponds par cue : sa lettre preferee (ou plusieurs), ou « aucune, dix autres » ; tu peux dire ce que tu aimes (bois, cloche, harpe...) pour les dix suivantes.",
        "capture %s_start" % name,
        "waitkey Entree : commencer par %s" % label(steps[0]),
    ]
    for position, step in enumerate(steps):
        out.append("hol.Audio.Candidate %s %s" % (step[0], "ABCDEFGHIJ"[step[1]]))
        out.append("waitlog candidate played %s %s" % (step[0], "ABCDEFGHIJ"[step[1]]))
        if position + 1 < len(steps):
            nxt = steps[position + 1]
            text = "%s (%s) vient de jouer. Entree : %s%s" % (label(step), step[2], label(nxt), "  [nouveau cue]" if nxt[0] != step[0] else "")
        else:
            text = "%s (%s) vient de jouer : c'etait la derniere. Entree : fin" % (label(step), step[2])
        out.append("waitkey " + text)
    out += [
        "hol.Test.Sanity",
        "note Fin de l'ecoute. Dis par cue ta lettre preferee, ou REGLAGE: <ce qui ne va pas>.",
    ]
    with open(os.path.join(SCEN, name + ".txt"), "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(out) + "\n")
    with open(os.path.join(SCEN, name + ".look"), "w", encoding="utf-8", newline="\n") as f:
        f.write("# <nom de capture> | ce que l'utilisateur doit voir\n%s_start | L'ecran du jeu avec le panneau du scenario : les notes disent d'ecouter %s, Entree = variante suivante\n" % (name, what))
    shutil.copyfile(os.path.join(SCEN, "audio_ui.json"), os.path.join(SCEN, name + ".json")) if name != "audio_ui" else None
    return len(steps)


if __name__ == "__main__":
    everything = cues()
    for group in GROUPS:
        print(group[0], write(group[0], group[1], group[2], everything), "steps")
