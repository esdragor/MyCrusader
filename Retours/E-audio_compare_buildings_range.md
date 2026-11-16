# [À ESSAYER] E-audio_compare_buildings_range : écoute le tour 6 du son de sélection du Champ de tir (le tir d'arc G du tour 5 en version propre) et dis ta lettre

STATUT : VU À L'ÉCRAN pour l'écran (le Champ de tir posé, panneau) ; le SON est LOGIQUE SEULEMENT : l'agent n'entend rien, l'écoute est la tienne
GESTE: interne
SON: non entendu
branches: d751-root

Contexte (D826, 2026-11-16) : tu as élu la SORTE de son G du tour 5 (`s_shot5_bow`, tir d'arc) mais l'enregistrement était « de mauvaise qualité ». Cause trouvée : le tour 5 saturait le son (tanh) et l'étouffait par un passe-bas abaissé jusqu'à 750 Hz de centroïde. Tour 6 = le MÊME enregistrement (bow_ogrebane.wav, CC0), même instant, non coupé (0,40 s, jusqu'à extinction), sans saturation, 5 variantes :
- A = `s_shot5_bow` : le G du tour 5 tel quel (référence, pour comparer)
- B = `s_shot6_bow_full` : propre, aucun passe-bas (le plus clair, centroïde 3700 Hz)
- C = `s_shot6_bow_lp6k` : passe-bas 6 kHz
- D = `s_shot6_bow_lp3k` : passe-bas 3 kHz
- E = `s_shot6_bow_lp1k5` : passe-bas 1,5 kHz (le plus doux, centroïde 880 Hz)
B, C et D dépassent l'ancienne règle « sons pas usants » (centroïde ≤ 800 Hz) : c'est à ton oreille de dire jusqu'où adoucir. Rien n'entre dans `HOLAudioTable.as` avant ta réponse.

Building.Range = _

Pré-vol (2026-11-16 15:24, agent ARWEN-1116a, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, branche d751-root c86557c, main 9c9d22b) : `preflight.ps1 audio_compare_buildings_range -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (1 PNG).
PNG OUVERTS et vus :
- audio_compare_buildings_range_start : le Champ de tir (cibles bleues) au centre de l'écran, le panneau du scénario « tour 6 ... A = le G du tour 5 (reference) ... E = passe-bas 1,5 kHz ».
Le journal de la partie montre les 5 variantes jouées (`candidate played Building.Range A` ... `E`).
NON VÉRIFIÉ : aucun son entendu par l'agent (-nosound) ; une meilleure SOURCE n'a pas été cherchée (même fichier, traitement seul) : si toutes les lettres sonnent encore sales, dis-le et le tour 7 cherchera un autre enregistrement.

assets: (hors git, seulement dans le worktree Arwen) `C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Content\Audio\Candidates\soft\SW_s_shot5_bow.uasset`, `SW_s_shot6_bow_full.uasset`, `SW_s_shot6_bow_lp6k.uasset`, `SW_s_shot6_bow_lp3k.uasset`, `SW_s_shot6_bow_lp1k5.uasset` ; WAV dans `C:\Users\baudo\Documents\MyCrusader_Audio\Candidates\soft\` (refaits par `Tools\Audio\make_round6_candidates.py`).

Commande complète (règle 13, 2026-11-14) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings_range -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 1 min (5 sons, Entrée = suivant). Réponse attendue : la lettre (B à E), `OK`, `BUG: ...` ou `REGLAGE: ...`.
