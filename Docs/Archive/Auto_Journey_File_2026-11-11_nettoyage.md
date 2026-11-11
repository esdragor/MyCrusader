# Auto_Journey_File.md : nettoyage du 2026-11-11
Lignes déplacées telles quelles (numéro = ligne avant nettoyage), vérifiées faites dans git log main / EtapeDefini.md / *InProgress.md ; puis, plus bas, texte AVANT correction des lignes modifiées.

## Lignes archivées
[l.22 ; D751 : cause racine corrigée (d751-root, aiwave PASS x3), dans main]
- [ ] (IA, D751) l'IA doit marcher en groupe a la vitesse du plus lent pour ses vagues (escorte etiree sur 70 m, tetes a 300 cm/s contre 180-220) : ouvrir le chantier, test rouge sur la marche de la vague.
[l.24 ; audio_select_gain : OK joué par l utilisateur le 2026-11-09 17:00]
- [ ] (sans C++, Arwen, ~25 min) Audio : scénario `audio_select_gain` (avant/après limiteur, Q-audio-select-gain = B, 558fd57 sur d751-root) : preflight PASS, PNG ouverts, fiche `Retours\E-audio_select_gain.md` (GESTE, SON, branches: d751-root) ; `AudioInProgress.md`.
[l.29 ; L8-2 : audit Sanity commité 2c213e5, dans main, utilisé par les mesures]
- [ ] (C++, test) L8-2 : audit `hol.Test.Sanity` « cible à portée, aucune attaque depuis N s » ; c'est le test rouge de BUG-melee-stuck-attack.
[l.34 ; D769 : validé, 6e40600, fusionné 2b5b556 ; D769c 83c8142]
- [ ] (C++, agent neuf, Celeborn, D769 du 2026-11-10) Neutres : un soldat Neutre sans cible riposte contre l'agresseur d'un camarade frappé à moins de R cm (hol.Combat.NeutralAssistRadius, départ ~600), jamais d'aggro ; test rouge d'abord ; scénario diplomacy_qground_dump (l'homme d'armes rouge le plus bas doit frapper dès le début) ; NeutralsInProgress.md, Retours\BUG-melee-stuck-attack.md.
[l.35 ; melee_stuck_old : mesuré 3/8 le 2026-11-10 17:08 (EtapeDefini)]
- [ ] (sans C++, Celeborn, ~25 min) Mêlée : `repeat_scenario melee_stuck_old -Runs 8` au défaut final (KeepStuckCount 0 ; watch 8/8 et melee_stuck 7/8 faits à la fusion du 2026-11-09 13:1x) ; `NeutralsInProgress.md` ; critère informatif >= 5/8.
[l.36 ; PlaceBoxedIn 1 + KeepStuckCount 1 : 6/8, pas meilleur (EtapeDefini, 2026-11-10 17:08)]
- [ ] (sans C++, Celeborn, ~25 min) Mêlée : mesurer PlaceBoxedIn 1 + KeepStuckCount 1 combinés (jamais mesurés, Celeborn-24) sur melee_stuck, N=8 ; `NeutralsInProgress.md` ; témoin : scénario `melee_stuck_on_ksc` 8/8.
[l.37 ; playtest_hero_army 0/4 et neutrals rejoués le 2026-11-10 (EtapeDefini, Celeborn-28)]
- [ ] (sans C++, Celeborn, ~20 min) D641 : rejouer `playtest_hero_army` et `neutrals` sur main fusionné (Sanity spin) et noter dans `Retours\BUG-melee-spin-sanity.md` et `BUG-playtest_hero_army-spin.md` lesquels passent ; 1er test : preflight playtest_hero_army.
[l.38 ; A/B N=16 : 14/16 contre 10/16, RearKeepBothFull 1 rejeté (EtapeDefini, 2026-11-10 15:05)]
- [ ] (2026-11-10, Celeborn, sans C++, ~30 min) Melee : A/B N=16 (ab_matrix.ps1) melee_stuck vs melee_stuck_on_bf (RearKeepBothFull 1) avant de changer le defaut ; reporter les chiffres de playtest_hero_army 0/4 et neutrals 2/4 dans Retours\BUG-melee-spin-sanity.md et BUG-playtest_hero_army-spin.md.
[l.72 ; hol.Decor.ClearNear : code cff1220 dans main, build PASS, tests verts (VisualInProgress)]
- [ ] (C++, Sauron, EN TETE, reponse utilisateur 2026-11-09 17:07 Q-neutrals-trees-decor = A) commande de test C++ qui retire les arbres (decor de la carte, pas des sources) autour d'un point, pour que le camp du scenario `neutrals` soit vu dans une clairiere ; puis refaire `neutrals` (preflight + PNG ouverts) et E-neutrals ; journal ScenariosInProgress.md, fiche Retours\Q-neutrals-trees-decor.md.
[l.85 ; ligne [x]]
- [x] (docs, session principale, 2026-11-10) D753 à D767 écrits au GDD § 22 (Q1 à Q14 + Q15 clairières en D767) ; prochain numéro D768 ; fiches de `Retours\` déjà purgées.

## Texte avant correction des lignes modifiées
[l.2]
Nettoyage du 2026-11-10 : lignes faites ou périmées déplacées telles quelles dans `Docs\Archive\Auto_Journey_File_2026-11-10_nettoyage.md` (précédent : `Docs\Archive\Auto_Journey_File_2026-11-07.md`, qui garde le texte long de l'ancienne ligne Celeborn-18). Deuxième passage du 2026-11-10 : même archive, section « Deuxième passage ».
[l.10]
1. Mêlée (Celeborn) : A/B N=16 melee_stuck contre melee_stuck_on_bf avant de changer le défaut ; puis les lignes mêlée ouvertes.
[l.13]
4. GDD : FAIT le 2026-11-10 (D753-D767 au § 22, prochain D768) ; reste à coder : Palier 3 G2+G3+G4, ForgeArmorCategories B, SITES_CLEAR_SOURCES True, L4 et rage, valeurs orphelins, check_placeholders, fusion de visual.
[l.18]
9. Build de rattrapage des worktrees : FAIT le 2026-11-10 (12:49-13:45, 7 builds PASS, Elrond déjà à jour, tous sur main a6022e1) ; à refaire seulement si du code est fusionné dans main avant l'essai.
[l.31]
- [ ] (C++) D641 : soldats qui tournent en cercle en mêlée (playtest_hero_army, neutrals, diplomacy_qground) ; rejouer sur main fusionné ; mêmes causes que les lignes ci-dessus.
[l.41]
- (état du 2026-11-09 : Paliers, Talents, Relais, perf S2-S4 : voir `EtapeDefini.md`).
[l.81]
- [ ] (fait en partie : 232d6eb dans main (qground) ; ClickUnit durci par 798d1d3 et 4c1db9f sur net-playtest, pas encore dans main) (sans C++) neutrals_qground et diplomacy_qground_watch : meme risque pan_exposure que fog_trees (clic a 125 images de la camera) ; corriger la camera avant le clic ; ClickUnit -1 enemy fragile (legions_roster_gesture(_dawn)).
