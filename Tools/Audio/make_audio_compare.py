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
    # The three long ones below are over the 3 minutes of rule 18 (100, 70 and 70 sounds): kept as "tout", NOT counted, no answer sheet.
    ("audio_compare", ("Ui.",), "les 10 sons de l'interface (clic, survol, ouvrir, fermer, retour, erreur, confirmer, bascule, tic, defilement) [TOUT, non compte : plus de 3 min]", False),
    ("audio_compare_orders", ("Order.", "Select."), "les 5 ordres (deplacer, attaquer, recolter, construire, garnison) et les 2 selections (unite, batiment) [TOUT, non compte : plus de 3 min]", False),
    ("audio_compare_alerts", ("Alert.",), "les 7 alertes (attaque, batiment fini, entrainement fini, technologie, commerce, volcan, diplomatie) [TOUT, non compte : plus de 3 min]", False),
    # One scenario per alert (10 SOFT sounds, about 2 min, one answer sheet each). Alert.TrainingDone has its own scenario below.
    ("audio_compare_alerts_attack", ("Alert.UnderAttack",), "l'alerte d'ATTAQUE subie : deux coups SOURDS et graves, reconnaissable mais pas stridente (pas usante)"),
    ("audio_compare_alerts_buildingdone", ("Alert.BuildingDone",), "l'alerte de BATIMENT fini : sons GRAVES, SOURDS, COURTS et DOUX, pas usants"),
    ("audio_compare_alerts_techdone", ("Alert.TechDone",), "l'alerte de TECHNOLOGIE finie : sons GRAVES, SOURDS, COURTS et DOUX, pas usants"),
    ("audio_compare_alerts_trade", ("Alert.Trade",), "l'alerte de COMMERCE : sons GRAVES, SOURDS, COURTS et DOUX, pas usants (une bourse, pas un tintement)"),
    ("audio_compare_alerts_volcano", ("Alert.Volcano",), "l'alerte de VOLCAN (le jeu la joue 3 fois a 1 s d'ecart) : grondements GRAVES et SOURDS, pas stridents"),
    ("audio_compare_alerts_diplomacy", ("Alert.Diplomacy",), "l'alerte de DIPLOMATIE : sons GRAVES, SOURDS, COURTS et DOUX, pas usants"),
    ("audio_compare_training", ("Alert.TrainingDone",), "l'alerte d'entrainement fini, troisieme tour : sons GRAVES, SOURDS, COURTS et DOUX (bois, cuir, tambour sourd, pierre, metal etouffe), sans cor ni trompette ni clochette"),
    # "tout" (all five families, about 11 min, over the 3 min of rule 18): kept, NOT counted as an essay, no answer sheet.
    ("audio_compare_buildings", ("Building.",), "le son de SELECTION d'un batiment, une famille apres l'autre (bois, pierre, sacre, metal, pieces) : sons GRAVES, SOURDS, COURTS et DOUX, pas usants [TOUT, non compte : 11 min]", False),
    # One scenario per family of buildings (10 sounds, about 2 min each) with its own answer sheet.
    ("audio_compare_buildings_wood", ("Building.Wood",), "le son de SELECTION des batiments en BOIS (Maison, camps, Moulin, Ferme) : sons GRAVES, SOURDS, COURTS et DOUX, pas usants"),
    ("audio_compare_buildings_stone", ("Building.Stone",), "le son de SELECTION des batiments en PIERRE (Centre principal, Caserne, Champ de tir, Ecurie, Atelier de siege) : sons GRAVES, SOURDS, COURTS et DOUX, pas usants"),
    ("audio_compare_buildings_sacred", ("Building.Sacred",), "le son de SELECTION des batiments SACRES (Chapelle, Sanctuaire) : sons GRAVES, SOURDS, COURTS et DOUX, pas usants"),
    ("audio_compare_buildings_metal", ("Building.Metal",), "le son de SELECTION du batiment de METAL (Forge) : sons GRAVES, SOURDS, COURTS et DOUX, pas usants"),
    ("audio_compare_buildings_coins", ("Building.Coins",), "le son de SELECTION du batiment de PIECES (Marche) : sons GRAVES, SOURDS, COURTS et DOUX, pas usants"),
    # Round 5 (D801) then round 6 (D826): one scenario per building (make_round6_candidates.py), the building itself on screen.
    ("audio_compare_buildings_barracks", ("Building.Barracks",), "le son de SELECTION de la CASERNE, tour 6 : dix sons NEUFS, AUCUNE voix (ni cri, ni grognement, ni foule), propres (non coupes, sans saturation)"),
    ("audio_compare_buildings_range", ("Building.Range",), "le son de SELECTION du CHAMP DE TIR, tour 6 : le tir d'arc G du tour 5 en version propre"),
    ("audio_compare_buildings_stable", ("Building.Stable",), "le son de SELECTION de l'ECURIE, tour 6 : l'ebrouement et la voix de cheval G du tour 5 en version propre"),
    # D832 REGLAGE (2026-11-16): Stable B without the neigh, only the snort; its answer goes in E-audio_compare_buildings_stable.md (no sheet).
    ("audio_compare_buildings_stable_cut", ("Building.CutStable",), "le son de SELECTION de l'ECURIE, reecoute D832 : le B du tour 6 COUPE (seulement l'ebrouement pfff de la fin, sans le hennissement du debut) contre le B entier [non compte : sa fiche est E-audio_compare_buildings_stable.md]", False),
]
# The scenario of ONE building places that building, finished and owned by the player, behind the centre, and puts the camera on it:
# the screen shows the building whose sound plays, not the Forum. name -> (class for hol.Cheat.SpawnBuilding, "<who> que tu ecoutes, ...").
BUILDING = {
    "audio_compare_buildings_barracks": ("Barracks", "la Caserne que tu ecoutes, finie et a toi, posee"),
    "audio_compare_buildings_range": ("ArcheryRange", "le Champ de tir que tu ecoutes, fini(e) et a toi, pose(e)"),
    "audio_compare_buildings_stable": ("Stable", "l'Ecurie que tu ecoutes, fini(e) et a toi, pose(e)"),
    "audio_compare_buildings_stable_cut": ("Stable", "l'Ecurie que tu ecoutes, fini(e) et a toi, pose(e)"),
}
BUILDING_AT = (-3800, 4600)
# Texts of a scenario that differ from the general ones (round 6, D826): count (title, "" = none), source (second line),
# intro / answer / end (notes), look (what the capture must show). A missing key keeps the general text.
CLEAN6_SOURCE = "D826 : la sorte de son G du tour 5 est elue, il faut une version propre ; A = le G du tour 5 tel quel (reference), B a E = le meme enregistrement, non coupe, sans saturation, du plus clair (B, aucun filtre) au plus doux (E, passe-bas 1,5 kHz). Jamais choisi a ta place."
CLEAN6_LETTERS = "5 sons, 1 cue : A = le G du tour 5 (reference, mauvaise qualite), B = propre sans filtre, C = passe-bas 6 kHz, D = passe-bas 3 kHz, E = passe-bas 1,5 kHz (le plus doux)."
CLEAN6_ANSWER = "Reponds : ta lettre preferee (B a E), ou REGLAGE: <ce qui ne va pas> (encore sale, trop aigu, trop long...)."
CLEAN6_END = "Fin de l'ecoute. Dis ta lettre preferee (B a E), ou REGLAGE: <ce qui ne va pas>."
TEXT = {
    "audio_compare_buildings_barracks": {
        "count": "",
        "source": "D826 : aucune lettre du tour 5 retenue (cris de douleur, mauvaise qualite) ; sources CC0 (StillNorthMedia lames, VSCO 2 CE timbale et cor), jamais choisies a ta place.",
        "intro": "Ecoute le son de SELECTION de la CASERNE, tour 6 : dix sons NEUFS, sans aucune voix. A B C = une lame tiree, D E = une lame rengainee, F = deux lames tirees ensemble, G = tambour de guerre (un coup), H = tambour (deux coups), I = un cor (une note), J = tambour puis lame.",
        "answer": "Reponds : ta lettre preferee (ou plusieurs), ou REGLAGE: <ce qui ne va pas> (trop aigu, trop long...), ou aucune, dix autres.",
        "end": "Fin de l'ecoute. Dis ta lettre preferee (A a J), ou REGLAGE: <ce qui ne va pas>.",
        "look": "OBJET : la Caserne au centre de l'ecran (posee par hol.Cheat.SpawnBuilding, pas le Forum) ; le panneau du scenario dit d'ecouter le son de SELECTION de la CASERNE, tour 6 : dix sons neufs sans aucune voix, A a J (lames, tambour, cor), Entree = variante suivante",
    },
    "audio_compare_buildings_range": {
        "count": "cinq variantes du MEME son",
        "source": CLEAN6_SOURCE,
        "intro": "Ecoute le son de SELECTION du CHAMP DE TIR, tour 6 : le tir d'arc G du tour 5 en version propre. " + CLEAN6_LETTERS,
        "answer": CLEAN6_ANSWER,
        "end": CLEAN6_END,
        "look": "OBJET : le Champ de tir (cibles) au centre de l'ecran (pose par hol.Cheat.SpawnBuilding, pas le Forum) ; le panneau du scenario dit d'ecouter le son de SELECTION du CHAMP DE TIR, tour 6 : le tir d'arc G du tour 5 en version propre, 5 variantes A a E, Entree = variante suivante",
    },
    "audio_compare_buildings_stable": {
        "count": "cinq variantes du MEME son",
        "source": CLEAN6_SOURCE,
        "intro": "Ecoute le son de SELECTION de l'ECURIE, tour 6 : l'ebrouement et la voix de cheval G du tour 5 en version propre. " + CLEAN6_LETTERS,
        "answer": CLEAN6_ANSWER,
        "end": CLEAN6_END,
        "look": "OBJET : l'Ecurie (forme provisoire BlockAndTower) au centre de l'ecran (pose par hol.Cheat.SpawnBuilding, pas le Forum) ; le panneau du scenario dit d'ecouter le son de SELECTION de l'ECURIE, tour 6 : l'ebrouement et la voix de cheval G du tour 5 en version propre, 5 variantes A a E, Entree = variante suivante",
    },
    "audio_compare_buildings_stable_cut": {
        "count": "deux variantes",
        "source": "D832 : tu as dit B, mais sans le hennissement du debut, juste le pfffff de fin ; A = le B coupe (le hennissement, la partie voisee, est retire ; meme enregistrement, meme licence), B = le B entier pour comparer. Jamais choisi a ta place.",
        "intro": "Ecoute le son de SELECTION de l'ECURIE, reecoute D832. 2 sons, 1 cue : A = le B du tour 6 COUPE (seulement l'ebrouement pfff de la fin, 0,94 s), B = le B entier (hennissement puis ebrouement, 1,42 s).",
        "answer": "Reponds : A (le coupe), B (l'entier), ou REGLAGE: <ce qui ne va pas> (coupe trop tot, trop tard, encore un bout de hennissement...).",
        "end": "Fin de l'ecoute. Dis A, B, ou REGLAGE: <ce qui ne va pas>.",
        "look": "OBJET : l'Ecurie (forme provisoire BlockAndTower) au centre de l'ecran (pose par hol.Cheat.SpawnBuilding, pas le Forum) ; le panneau du scenario dit d'ecouter le son de SELECTION de l'ECURIE, reecoute D832 : A = le B coupe (seulement l'ebrouement), B = le B entier, Entree = variante suivante",
    },
}
# The line of the answer sheet for a cue whose name alone says little (family cues of the buildings): "<label> = _".
SHEET_LABEL = {
    "Building.Wood": "Bois (Maison, camps, Moulin, Ferme) [Building.Wood]",
    "Building.Stone": "Pierre (Centre principal, Caserne, Champ de tir, Ecurie, Atelier de siege) [Building.Stone]",
    "Building.Sacred": "Sacre (Chapelle, Sanctuaire) [Building.Sacred]",
    "Building.Metal": "Metal (Forge) [Building.Metal]",
    "Building.Coins": "Pieces (Marche) [Building.Coins]",
    "Alert.UnderAttack": "Attaque subie [Alert.UnderAttack]",
    "Alert.BuildingDone": "Batiment fini [Alert.BuildingDone]",
    "Alert.TechDone": "Technologie finie [Alert.TechDone]",
    "Alert.Trade": "Commerce [Alert.Trade]",
    "Alert.Volcano": "Volcan [Alert.Volcano]",
    "Alert.Diplomacy": "Diplomatie [Alert.Diplomacy]",
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
    # The letters, so that the answer is unambiguous: the panel shows the same "<Cue> <letter> (<file>)".
    out += ["", "Lettres (le panneau affiche le nom du cue a cote de la lettre) :"]
    out += ["%s : %s" % (cue, ", ".join("%s %s" % (LETTERS[i], stems[i]) for i in range(len(stems)))) for cue, stems in mine]
    root = project.replace("/", "\\")
    out += [
        "",
        "Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File %s\\Tools\\Scenarios\\play.ps1 %s -Root %s`" % (root, name, root),
        "Durée : environ %d min (%d sons, Entrée = variante suivante ; le panneau dit « <Cue> <lettre> (<fichier>) vient de jouer »)." % (max(1, (len(steps) * 12 + 30) // 60), len(steps)),
        "Réponse attendue : une lettre par cue, `OK`, `BUG: ...` ou `REGLAGE: ...`.",
        "",
    ]
    os.makedirs(retours, exist_ok=True)
    with open(os.path.join(retours, "E-%s.md" % name), "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(out))


def write(name, prefixes, what, all_cues, scen, retours, project, sheet=True):
    # Select.BuildingWood / Select.BuildingSacred (2026-11-08) are the ELECTED building sounds, listened to in the Building.* scenarios: not in the orders one.
    mine = [c for c in all_cues if c[0].startswith(prefixes) and c[0] not in ("Select.BuildingWood", "Select.BuildingSacred")]
    steps = [(cue, index, stems[index]) for cue, stems in mine for index in range(len(stems))]
    custom = TEXT.get(name, {})
    count = custom.get("count", "dix variantes chacune")
    out = [
        "# Ecoute : %s%s, Entree = variante suivante" % (what, ", " + count if count else ""),
        "# Plain, toi contre une IA, pour avoir l'ecran du jeu ; rien a cliquer : le panneau dit quelle variante vient de jouer. "
        + custom.get("source", "Sources CC0 (Kenney rpg-audio et impact-sounds, VSCO 2 CE), jamais choisies a ta place."),
        "waitlog Starting hero of slot 0 spawned at",
        "waitlog Starting hero of slot 1 spawned at",
    ]
    if name in BUILDING:
        building, who = BUILDING[name]
        out += [
            "# %s derriere le centre, camera dessus : l'ecran montre le batiment du son, pas le Forum." % who,
            "hol.Cheat.SpawnBuilding %s 0 %d %d" % (building, BUILDING_AT[0], BUILDING_AT[1]),
            "waitlog Cheat: spawned %s of slot 0" % building,
            "hol.Camera.FocusNear %d %d" % BUILDING_AT,
        ]
    else:
        out.append("hol.Camera.FocusNear -900 300")
    out += [
        "frames 10",
        "note " + custom.get("intro", "Ecoute %s. %d sons, %d %s, dix variantes A a J chacune." % (what, len(steps), len(mine), "cue" if len(mine) == 1 else "cues")),
        "note Entree = la variante suivante (le panneau dit le cue et la lettre qui viennent de jouer). Pour reecouter : console (touche ²) puis hol.Audio.Candidate <Cue> <lettre>, par exemple hol.Audio.Candidate %s C." % steps[0][0],
        "note " + custom.get("answer", ("Reponds : ta lettre preferee (ou plusieurs)" if len(mine) == 1 else "Reponds par cue : sa lettre preferee (ou plusieurs)") + ", ou « aucune, dix autres » ; tu peux dire ce que tu aimes pour les dix suivantes."),
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
        "note " + custom.get("end", "Fin de l'ecoute. Dis par cue ta lettre preferee, ou REGLAGE: <ce qui ne va pas>."),
    ]
    os.makedirs(scen, exist_ok=True)
    with open(os.path.join(scen, name + ".txt"), "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(out) + "\n")
    with open(os.path.join(scen, name + ".look"), "w", encoding="utf-8", newline="\n") as f:
        f.write("# <nom de capture> | ce que l'utilisateur doit voir\n"
                "%s_start | %s\n" % (name, custom.get("look", "L'ecran du jeu avec le panneau du scenario : les notes disent d'ecouter %s, Entree = variante suivante" % what)))
    shutil.copyfile(os.path.join(project, "Tools", "Scenarios", "audio_ui.json"), os.path.join(scen, name + ".json"))
    if retours and sheet:
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
            print(group[0], write(group[0], group[1], group[2], everything, scen, args.retours, args.root, group[3] if len(group) > 3 else True), "steps")
