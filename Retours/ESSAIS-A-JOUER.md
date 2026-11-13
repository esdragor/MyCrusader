# ESSAIS À JOUER (nettoyage du 2026-11-13 soir, étape 0a de l'Auto_Journey)

**Prêts à donner : 0.** Aucune fiche E-* n'a un pré-vol PASS sur une copie à jour : toutes sont `LOGIQUE SEULEMENT` ou `[NE PAS JOUER]`, et leurs sujets sont rouverts ce soir.

Version précédente (matin du 2026-11-13) : `Docs\Archive\Retours_2026-11-13_soir\ESSAIS-A-JOUER_2026-11-13_matin.md`. Essais joués à 14:20 (`resources_compare` = arbre C, `legions_roster_gesture` = OK) : reportés et archivés dans `Docs\Archive\Retours_2026-11-13\`.

## État réel au 2026-11-13 16:1x
- **Binaire de la copie principale PÉRIMÉ** : `UnrealEditor-HeroesOfLiberty.dll` du 2026-11-11 11:06, alors que main (ecff29a) porte du C++ jusqu'au 2026-11-13 13:05 (1dc24bc, fusion groupée 481bf43). Recompiler la copie jouée avant tout pré-vol.
- **Fusionnées dans main** : fog, maps, net-playtest, visual, d751-root, galadriel-tab. **Non fusionnées** : tiers (Elrond), hero-j8-close (Celeborn), heal-sleep (Arwen, sons).
- **Accord de l'utilisateur du jour** : `Content/Audio/Candidates/` exclu par `.git/info/exclude` (partagé par tous les worktrees : le pré-vol ne doit plus échouer sur ces 294 fichiers non commités) ; `GDD.md` commité (D792, c17f8b1). Les sons candidats restent seulement dans le worktree Arwen.

## Fiches encore dans Retours\ (8 essais, 9 fiches E-* avec l'index audio), aucune donnée
- **SONS, `[NE PAS JOUER]`** (branche heal-sleep, candidats seulement dans Arwen) : `E-audio_compare_buildings_stone.md`, `E-audio_compare_buildings_metal.md` (index : `E-audio_compare_buildings.md`), `E-audio_panel.md`. Reste : recompiler Arwen, puis `preflight.ps1 <nom> -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` complet PASS.
- **`[NE PAS JOUER]`** : `E-disconnect_1v1.md` (net-playtest est maintenant dans main ; reste : copie recompilée puis `preflight.ps1 disconnect_1v1 -Root <copie> -PlayerName Alice` PASS).
- **LOGIQUE SEULEMENT** :
  - `E-neutrals_minimap.md` (d751-root, dans main ; seul FAIL du pré-vol = fichiers audio non commités, maintenant exclus : pré-vol à rejouer).
  - `E-deploy.md` (tiers, non fusionnée ; scénario `waitlog D=Pack` jamais satisfait, le jeu monte bien l'engin).
  - `E-spear_arm.md` (hero-j8-close, non fusionnée ; caméra min 15 m, prise de lance non jugeable à l'œil).
  - `E-playtest_hero_army.md` (pré-vol FAIL Sanity, spin de mêlée D641 non corrigé : voir `BUG-playtest_hero_army-spin.md`).
