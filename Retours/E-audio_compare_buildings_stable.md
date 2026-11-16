# [À ESSAYER] E-audio_compare_buildings_stable : écoute le tour 6 du son de sélection de l'Écurie (l'ébrouement et la voix de cheval G du tour 5 en version propre) et dis ta lettre

STATUT : VU À L'ÉCRAN pour l'écran (l'Écurie posée, panneau) ; le SON est LOGIQUE SEULEMENT : l'agent n'entend rien, l'écoute est la tienne
GESTE: interne
SON: non entendu
branches: d751-root

Contexte (D826, 2026-11-16) : tu as élu la SORTE de son G du tour 5 (`s_horse5_vocal_b`, ébrouement + voix de cheval) mais l'enregistrement était « de mauvaise qualité ». Cause trouvée : le tour 5 coupait à 0,78 s alors que la voix dure environ 0,9 s (fin tranchée), saturait (tanh) et étouffait (passe-bas, centroïde 670 Hz). Tour 6 = le MÊME enregistrement (Red Library R12-60, archive.org, D805), même instant (11,22 s), non coupé (1,42 s, jusqu'à extinction), sans saturation, 5 variantes :
- A = `s_horse5_vocal_b` : le G du tour 5 tel quel (référence, pour comparer)
- B = `s_horse6_vocal_full` : propre, aucun passe-bas (centroïde 1650 Hz)
- C = `s_horse6_vocal_lp6k` : passe-bas 6 kHz
- D = `s_horse6_vocal_lp3k` : passe-bas 3 kHz
- E = `s_horse6_vocal_lp1k5` : passe-bas 1,5 kHz (le plus doux, centroïde 980 Hz)
Toutes durent 1,42 s, au-delà de l'ancienne règle « courts » (≤ 0,80 s) : la voix entière ou rien ; à ton oreille de dire si c'est trop long. Rien n'entre dans `HOLAudioTable.as` avant ta réponse.

Building.Stable = _

Pré-vol (2026-11-16 15:24, agent ARWEN-1116a, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, branche d751-root c86557c, main 9c9d22b) : `preflight.ps1 audio_compare_buildings_stable -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (1 PNG).
PNG OUVERTS et vus :
- audio_compare_buildings_stable_start : l'Écurie (forme provisoire bleue, BlockAndTower) au centre de l'écran, le panneau du scénario « tour 6 ... A = le G du tour 5 (reference) ... E = passe-bas 1,5 kHz ».
Le journal de la partie montre les variantes jouées (`candidate played Building.Stable A s_horse5_vocal_b`, `B s_horse6_vocal_full`...).
NON VÉRIFIÉ : aucun son entendu par l'agent (-nosound) ; une meilleure SOURCE n'a pas été cherchée (même fichier, traitement seul) ; le bruit de fond de l'enregistrement d'archive (souffle) n'est pas réduit.

assets: (hors git, seulement dans le worktree Arwen) `C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Content\Audio\Candidates\soft\SW_s_horse5_vocal_b.uasset`, `SW_s_horse6_vocal_full.uasset`, `SW_s_horse6_vocal_lp6k.uasset`, `SW_s_horse6_vocal_lp3k.uasset`, `SW_s_horse6_vocal_lp1k5.uasset` ; WAV dans `C:\Users\baudo\Documents\MyCrusader_Audio\Candidates\soft\` (refaits par `Tools\Audio\make_round6_candidates.py`).

Commande complète (règle 13, 2026-11-14) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings_stable -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 1 min (5 sons, Entrée = suivant). Réponse attendue : la lettre (B à E), `OK`, `BUG: ...` ou `REGLAGE: ...`.
Reponse: REGLAGE : B mais pas le hennissement de début, juste le pfffff de fin (utilisateur, 2026-11-16 16:5x ; D832 ; ligne [ ] en tête de la section Arwen)
## Réécoute D832 (ARWEN-1116d, 2026-11-16 17:1x) : B coupé (seulement le « pfff ») contre B entier
SON: non entendu   (-nosound côté agent ; l'écoute est la tienne)
GESTE: interne
branches: galadriel-tab
Ton REGLAGE ci-dessus appliqué : `s_horse6_snort` = le B du tour 6 (`s_horse6_vocal_full`) dont la partie VOISÉE du début (le hennissement, mesurée 0,02-0,40 s par autocorrélation) est retirée : coupe à 0,48 s, il reste l'ébrouement de la fin, 0,94 s, fondu d'entrée 15 ms, crête -2,5 dB ; même enregistrement (Red Library R12-60, CC0, D805), outil `Tools\Audio\make_round6_candidates.py --snort-only`. Test `round6_candidates_test.py` : 0 trame voisée dans le coupé (19 dans le B entier).
A = `s_horse6_snort` (B coupé), B = `s_horse6_vocal_full` (B entier, pour comparer) ; cue `Building.CutStable`. Rien n'est élu pour l'Écurie avant ta réponse (Champ de tir B et Caserne A sont élus, D832, commit 89e03a6).
Pré-vol (2026-11-16 17:12, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel, branche galadriel-tab 1c34b42) : `preflight.ps1 audio_compare_buildings_stable_cut -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel` PASS (1 PNG). PNG OUVERT et vu : audio_compare_buildings_stable_cut_start : l'Écurie (forme provisoire bleue) au centre, panneau « réécoute D832, A = le B COUPÉ (seulement l'ébrouement pfff de la fin, 0,94 s), B = le B entier ». (Le panneau cite par erreur l'exemple « hol.Audio.Candidate Building.CutStable C » : seules A et B existent.)
NON VÉRIFIÉ : aucun son entendu par l'agent ; la frontière hennissement / ébrouement est mesurée, pas écoutée (si un bout de hennissement reste ou si le début du pfff est mangé : `REGLAGE: coupe plus tôt / plus tard`).
assets: (hors git, seulement dans le worktree Galadriel) `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Content\Audio\Candidates\soft\SW_s_horse6_snort.uasset` ; WAV `C:\Users\baudo\Documents\MyCrusader_Audio\Candidates\soft\s_horse6_snort.wav`. (Les élus `SW_s_shot6_bow_full` et `SW_s_barr6_draw_a` sous `Content\Audio\Soft` SONT commités.)
Commande complète :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 audio_compare_buildings_stable_cut -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`
Durée : moins d'1 min (2 sons, Entrée = suivant). Réponse attendue : `A` (le coupé, élu pour l'Écurie), `B`, ou `REGLAGE: ...`.
Reponse D832 : _
