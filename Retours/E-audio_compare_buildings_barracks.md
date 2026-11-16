# [À ESSAYER] E-audio_compare_buildings_barracks : écoute le tour 6 du son de sélection de la Caserne (dix sons neufs, sans aucune voix) et dis ta lettre

STATUT : VU À L'ÉCRAN pour l'écran (la Caserne posée, panneau) ; le SON est LOGIQUE SEULEMENT : l'agent n'entend rien, l'écoute est la tienne
GESTE: interne
SON: non entendu
branches: d751-root

Contexte (D826, 2026-11-16) : au tour 5 tu n'as retenu AUCUNE lettre de la Caserne (« cris de douleur, mauvaise qualité »). Tour 6 = dix sons NEUFS, sans aucune voix (ni cri, ni grognement, ni foule), traités proprement comme le Champ de tir et l'Écurie du tour 6 : non coupés (jusqu'à extinction), sans saturation, aucun passe-bas (seulement un passe-haut 80 Hz). Sources CC0 : lames StillNorthMedia (`sabre_draw_fast.wav`, `seax_sheath_fast.wav`), timbale et cor VSCO 2 CE.
- A = `s_barr6_draw_a` : une lame tirée (1)
- B = `s_barr6_draw_b` : une lame tirée (2)
- C = `s_barr6_draw_c` : une lame tirée (3)
- D = `s_barr6_sheath_a` : une lame rengainée (1)
- E = `s_barr6_sheath_b` : une lame rengainée (2)
- F = `s_barr6_two_blades` : deux lames tirées presque ensemble
- G = `s_barr6_drum` : tambour de guerre (timbale), un coup
- H = `s_barr6_drum_double` : tambour, deux coups
- I = `s_barr6_horn` : un cor, une note (0,9 s)
- J = `s_barr6_drum_blade` : tambour puis lame tirée
Les lames (A, B, C, F, J) sont très claires (centroïde vers 6 kHz, bien au-dessus de l'ancienne règle « sons pas usants » ≤ 800 Hz) : si c'est trop aigu, dis `REGLAGE: plus doux` et le tour suivant fera un balayage du passe-bas comme pour le Champ de tir. Rien n'entre dans `HOLAudioTable.as` avant ta réponse.

Building.Barracks = _

Pré-vol (2026-11-16 15:49, agent ARWEN-1116b, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, branche d751-root 734b313) : `preflight.ps1 audio_compare_buildings_barracks -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (1 PNG, 1 WARN commun hol.HUD.Report). Les 10 lettres A à J jouées dans le journal de la partie (`Scenario waits for the key: Building.Barracks J (s_barr6_drum_blade)`), aucun asset manquant.
PNG OUVERTS et vus :
- audio_compare_buildings_barracks_start : la Caserne (forme provisoire bleue) au centre de l'écran, le panneau du scénario « tour 6 : dix sons NEUFS, sans aucune voix. A B C = une lame tiree ... J = tambour puis lame ».
NON VÉRIFIÉ : aucun son entendu par l'agent (-nosound) ; la clarté des lames (choisie non filtrée, l'oreille décide) ; le cor est une note tenue coupée à 0,9 s avec une extinction douce (pas un appel de plusieurs notes).

assets: (hors git, seulement dans le worktree Arwen) `C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Content\Audio\Candidates\soft\SW_s_barr6_draw_a.uasset`, `SW_s_barr6_draw_b.uasset`, `SW_s_barr6_draw_c.uasset`, `SW_s_barr6_sheath_a.uasset`, `SW_s_barr6_sheath_b.uasset`, `SW_s_barr6_two_blades.uasset`, `SW_s_barr6_drum.uasset`, `SW_s_barr6_drum_double.uasset`, `SW_s_barr6_horn.uasset`, `SW_s_barr6_drum_blade.uasset` ; WAV dans `C:\Users\baudo\Documents\MyCrusader_Audio\Candidates\soft\` (refaits par `Tools\Audio\make_round6_candidates.py`).

Commande complète (règle 13, 2026-11-14) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings_barracks -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 1 min 30 (10 sons, Entrée = suivant). Réponse attendue : la lettre (A à J, ou plusieurs), `OK`, `BUG: ...` ou `REGLAGE: ...`.
