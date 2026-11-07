"""Writes the scenarios that let the user LISTEN to the candidates of the sound cues (AudioInProgress.md, method of the ten samples):
Tools/Scenarios/audio_compare.txt (the 10 cues of the interface), audio_compare_orders.txt (5 orders + 2 selections),
audio_compare_alerts.txt (7 alerts) and audio_compare_training.txt (the training alert alone, third round: soft sounds), each with
its .json (the audio_ui match) and .look. For each cue the ten variants A to J play one after the other: Enter = the next variant;
the panel text ALWAYS names the cue next to the letter ("Alert.TrainingDone C (v_horn_call) vient de jouer. Entree : Alert.TrainingDone D").
hol.Audio.Candidate <Cue> <A-J> in the console plays one again. The cues and stems are read from Script/Audio/HOLAudioCandidates.as.

L8-4: with --retours <dir> the pre-written answer sheet Retours/E-<scenario>.md is written too, one line `<Cue> = _` per cue the scenario
plays (the user replaces the _ by the letter(s) he prefers), with the honest status lines (GESTE: interne, SON: non entendu).

  python make_audio_compare.py [--only name,name] [--scenarios dir] [--retours dir] [--root project]
"""
import argparse
import os
import re
import shutil

HERE = os.path.dirname(os.path.abspath(__file__))
LINE = re.compile(r'^\s*Add\(List,\s*"(?P<cue>[A-Za-z]+\.[A-Za-z]+)",\s*"(?P<stems>[A-Za-z0-9_,]+)"')
LETTERS = "ABCDEFGHIJ"
GROUPS = [
    ("audio_compare", ("Ui.",), "les 10 sons de l'interface (clic, survol, ouvrir, fermer, retour, erreur, confirmer, bascule, tic, defilement)"),
    ("audio_compare_orders", ("Order.", "Select."), "les 5 ordres (deplacer, attaquer, recolter, construire, garnison) et les 2 selections (unite, batiment)"),
    ("audio_compare_alerts", ("Alert.",), "les 7 alertes (attaque, batiment fini, entrainement fini, technologie, commerce, volcan, diplomatie)"),
    ("audio_compare_training", ("Alert.TrainingDone",), "l'alerte d'entrainement fini, troisieme tour : sons GRAVES, SOURDS, COURTS et DOUX (bois, cuir, tambour sourd, pierre, metal etouffe), sans cor ni trompette ni clochette"),
    ("audio_compare_buildings", ("Building.",), "le son de SELECTION d'un batiment, une famille apres l'autre (bois, pierre, sacre, metal, pieces) : sons GRAVES, SOURDS, COURTS et DOUX, pas usants"),
]
# The line of the answer sheet for a cue whose name alone says little (family cues of the buildings): "<label> = _".
SHEET_LABEL = {
    "Building.Wood": "Bois (Maison, camps, Moulin, Ferme) [Building.Wood]",
    "Building.Stone": "Pierre (Centre principal, Caserne, Champ de tir, Ecurie, Atelier de siege) [Building.Stone]",
    "Building.Sacred": "Sacre (Chapelle, Sanctuaire) [Building.Sacred]",
    "Building.Metal": "Metal (Forge) [Building.Metal]",
    "Building.Coins": "Pieces (Marche) [Building.Coins]",
}


def cues(project):
    result = []
    with open(os.path.join(project, "Script", "Audio", "HOLAudioCandidates.as"), encoding="utf-8") as source:
        for line in source:
            match = LINE.match(line)
            if match:
                result.append((match.group("cue"), match.group("stems").split(",")))
    return result


def label(step):
    return "%s %s" % (step[0], LETTERS[step[1]])


def write_answer_sheet(retours, name, what, mine, steps, project):
    out = [
        "# [À ESSAYER] E-%s : écoute %s ; tu réponds par cue, la lettre que tu préfères" % (name, what),
        "",
        "STATUT : LOGIQUE SEULEMENT   (aucune oreille n'a encore écouté : l'agent a lancé les scénarios sans son)",
        "GESTE: interne      (les sons sont joués par la commande hol.Audio.Candidate du scénario, pas par un vrai clic ; seule la touche Entrée est la tienne)",
        "SON: non entendu   (-nosound côté agent ; passe à « entendu » quand quelqu'un a écouté)",
        "branches: heal-sleep   (le scénario et les candidats viennent du worktree Arwen ; candidats JAMAIS commités, décision B de l'utilisateur)",
        "",
        "Réponses : remplace le _ par la lettre préférée (A à J), plusieurs lettres possibles, ou « aucune, dix autres » et dis ce que tu aimes.",
    ]
    out += ["%s = _" % SHEET_LABEL.get(cue, cue) for cue, _ in mine]
    root = project.replace("/", "\\")
    out += [
        "",
        "Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File %s\\Tools\\Scenarios\\play.ps1 %s -Root %s`" % (root, name, root),
        "Durée : environ %d min (%d sons, Entrée = variante suivante ; le panneau dit « <Cue> <lettre> (<fichier>) vient de jouer »)." % (max(1, len(steps) * 12 // 60 + 1), len(steps)),
        "Réponse attendue : une lettre par cue, `OK`, `BUG: ...` ou `REGLAGE: ...`.",
        "",
    ]
    os.makedirs(retours, exist_ok=True)
    with open(os.path.join(retours, "E-%s.md" % name), "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(out))


def write(name, prefixes, what, all_cues, scen, retours, project):
    mine = [c for c in all_cues if c[0].startswith(prefixes)]
    steps = [(cue, index, stems[index]) for cue, stems in mine for index in range(len(stems))]
    out = [
        "# Ecoute : %s, dix variantes chacune, Entree = variante suivante" % what,
        "# Plain, toi contre une IA, pour avoir l'ecran du jeu ; rien a cliquer : le panneau dit quelle variante vient de jouer. Sources CC0 (Kenney rpg-audio et impact-sounds, VSCO 2 CE), jamais choisies a ta place.",
        "waitlog Starting hero of slot 0 spawned at",
        "waitlog Starting hero of slot 1 spawned at",
        "hol.Camera.FocusNear -900 300",
        "frames 10",
        "note Ecoute %s. %d sons, %d cues, dix variantes A a J chacune." % (what, len(steps), len(mine)),
        "note Entree = la variante suivante (le panneau dit le cue et la lettre qui viennent de jouer). Pour reecouter : console (touche ²) puis hol.Audio.Candidate <Cue> <lettre>, par exemple hol.Audio.Candidate %s C." % steps[0][0],
        "note Reponds par cue : sa lettre preferee (ou plusieurs), ou « aucune, dix autres » ; tu peux dire ce que tu aimes pour les dix suivantes.",
        "capture %s_start" % name,
        "waitkey Entree : commencer par %s" % label(steps[0]),
    ]
    for position, step in enumerate(steps):
        out.append("hol.Audio.Candidate %s %s" % (step[0], LETTERS[step[1]]))
        out.append("waitlog candidate played %s %s" % (step[0], LETTERS[step[1]]))
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
    os.makedirs(scen, exist_ok=True)
    with open(os.path.join(scen, name + ".txt"), "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(out) + "\n")
    with open(os.path.join(scen, name + ".look"), "w", encoding="utf-8", newline="\n") as f:
        f.write("# <nom de capture> | ce que l'utilisateur doit voir\n"
                "%s_start | L'ecran du jeu avec le panneau du scenario : les notes disent d'ecouter %s, Entree = variante suivante\n"                % (name, what))
    shutil.copyfile(os.path.join(project, "Tools", "Scenarios", "audio_ui.json"), os.path.join(scen, name + ".json"))
    if retours:
        write_answer_sheet(retours, name, what, mine, steps, project)
    return len(steps)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", default=os.path.abspath(os.path.join(HERE, "..", "..")))
    parser.add_argument("--scenarios", default=None)
    parser.add_argument("--retours", default=None)
    parser.add_argument("--only", default=None)
    args = parser.parse_args()
    scen = args.scenarios or os.path.join(args.root, "Tools", "Scenarios")
    only = args.only.split(",") if args.only else None
    everything = cues(args.root)
    for group in GROUPS:
        if only is None or group[0] in only:
            print(group[0], write(group[0], group[1], group[2], everything, scen, args.retours, args.root), "steps")
