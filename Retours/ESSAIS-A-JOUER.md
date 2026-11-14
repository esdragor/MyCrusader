# ESSAIS À JOUER (nettoyage du 2026-11-13 soir, étape 0a de l'Auto_Journey)

**Prêts à donner : 0** au nettoyage de 16:1x (aucune fiche E-* n'avait un pré-vol PASS sur une copie à jour). **Mise à jour 18:0x (ARWEN-1113d)** : 2 écoutes de sons passées `[À ESSAYER]` (Pierre, Forge ; voir « SONS » ci-dessous) : l'écran est vu, le son ne peut être jugé que par toi.

Version précédente (matin du 2026-11-13) : `Docs\Archive\Retours_2026-11-13_soir\ESSAIS-A-JOUER_2026-11-13_matin.md`. Essais joués à 14:20 (`resources_compare` = arbre C, `legions_roster_gesture` = OK) : reportés et archivés dans `Docs\Archive\Retours_2026-11-13\`.

## État réel au 2026-11-13 21:4x
- **Binaire de la copie principale PÉRIMÉ** : `UnrealEditor-HeroesOfLiberty.dll` du 2026-11-11 11:06, alors que main (15bbd25) porte le C++ des fusions groupées du soir. Recompiler la copie jouée avant tout pré-vol (ou jouer depuis un worktree à jour de main, comme Faramir).
- **Fusionnées dans main** : fusion groupée n°1 19:29 (dc67a16 : fog, maps, d751-root, visual, galadriel-tab, net-playtest, hero-j8-close), n°2 21:23 (15bbd25 : maps, net-playtest, tiers) ; heal-sleep aussi dans main. Les mentions « non fusionnée » de tiers / hero-j8-close plus bas sont donc périmées (E-deploy, E-spear_arm : statut à revoir par la recette). **Non fusionnée** : d751-root actuel (« Faction choice », autre session). RIEN POUSSÉ.
- **Accord de l'utilisateur du jour** : `Content/Audio/Candidates/` exclu par `.git/info/exclude` (partagé par tous les worktrees : le pré-vol ne doit plus échouer sur ces 294 fichiers non commités) ; `GDD.md` commité (D792, c17f8b1). Les sons candidats sont copiés dans le worktree qui joue l'essai (Faramir depuis 19:42, copie d'Arwen).

## Fiches encore dans Retours\ (8 essais, 9 fiches E-* avec l'index audio), aucune donnée
- **Mise à jour 19:4x (FARAMIR, recette réparée sur main dc67a16 + scénario fafdee9)** : `E-legions_warrior.md` pré-vol PASS, 3 PNG vus, VU À L'ÉCRAN : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 legions_warrior -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`.
- **SONS, `[À ESSAYER]`** : `E-audio_compare_buildings_stone.md` (Pierre) et `E-audio_compare_buildings_metal.md` (Forge) (index : `E-audio_compare_buildings.md`) : pré-vol PASS 19:44 depuis Faramir (main dc67a16, candidats copiés dans ce worktree), panneau VU À L'ÉCRAN, SON non entendu (l'agent ne peut pas entendre : c'est ton écoute qui tranche), 2 min chacun, Entrée = son suivant, réponse `OK` / `BUG: ...` / `REGLAGE: <lettre>`.
  - `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 audio_compare_buildings_stone -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
  - `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 audio_compare_buildings_metal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
- **`[À ESSAYER]` VU À L'ÉCRAN avec réserve** : `E-neutrals_minimap.md` (correctif 12 px dans main ; carré ~8 px effectifs sur la capture : un REGLAGE de taille est possible).
- **SONS, `[NE PAS JOUER]`** : `E-audio_panel.md` (inchangé ; pas de pré-vol ce soir).
- **`[NE PAS JOUER]`** : `E-disconnect_1v1.md` (net-playtest est maintenant dans main ; reste : copie recompilée puis `preflight.ps1 disconnect_1v1 -Root <copie> -PlayerName Alice` PASS).
- **LOGIQUE SEULEMENT** :
  - `E-deploy.md` (tiers, non fusionnée ; scénario `waitlog D=Pack` jamais satisfait, le jeu monte bien l'engin).
  - `E-spear_arm.md` (hero-j8-close, non fusionnée ; caméra min 15 m, prise de lance non jugeable à l'œil).
  - `E-playtest_hero_army.md` (pré-vol FAIL Sanity, spin de mêlée D641 non corrigé : voir `BUG-playtest_hero_army-spin.md`).
