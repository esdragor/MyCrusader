# ESSAIS À JOUER (nettoyage du 2026-11-13 soir, étape 0a de l'Auto_Journey)

**Prêts à donner : 0** au nettoyage de 16:1x (aucune fiche E-* n'avait un pré-vol PASS sur une copie à jour). **Mise à jour 18:0x (ARWEN-1113d)** : 2 écoutes de sons passées `[À ESSAYER]` (Pierre, Forge ; voir « SONS » ci-dessous) : l'écran est vu, le son ne peut être jugé que par toi.

Version précédente (matin du 2026-11-13) : `Docs\Archive\Retours_2026-11-13_soir\ESSAIS-A-JOUER_2026-11-13_matin.md`. Essais joués à 14:20 (`resources_compare` = arbre C, `legions_roster_gesture` = OK) : reportés et archivés dans `Docs\Archive\Retours_2026-11-13\`.

## État réel au 2026-11-13 16:1x
- **Binaire de la copie principale PÉRIMÉ** : `UnrealEditor-HeroesOfLiberty.dll` du 2026-11-11 11:06, alors que main (ecff29a) porte du C++ jusqu'au 2026-11-13 13:05 (1dc24bc, fusion groupée 481bf43). Recompiler la copie jouée avant tout pré-vol.
- **Fusionnées dans main** : fog, maps, net-playtest, visual, d751-root, galadriel-tab. **Non fusionnées** : tiers (Elrond), hero-j8-close (Celeborn), heal-sleep (Arwen, sons).
- **Accord de l'utilisateur du jour** : `Content/Audio/Candidates/` exclu par `.git/info/exclude` (partagé par tous les worktrees : le pré-vol ne doit plus échouer sur ces 294 fichiers non commités) ; `GDD.md` commité (D792, c17f8b1). Les sons candidats restent seulement dans le worktree Arwen.

## Fiches encore dans Retours\ (8 essais, 9 fiches E-* avec l'index audio), aucune donnée
- **SONS, `[À ESSAYER]` depuis 2026-11-13 18:0x (ARWEN-1113d)** : `E-audio_compare_buildings_stone.md` (Pierre) et `E-audio_compare_buildings_metal.md` (Forge) (index : `E-audio_compare_buildings.md`) : pré-vol PASS 17:55 depuis Arwen (d751-root, binaire frais 17:48), panneau VU À L'ÉCRAN, SON non entendu (l'agent ne peut pas entendre : c'est ton écoute qui tranche), 2 min chacun, Entrée = son suivant, réponse `OK` / `BUG: ...` / `REGLAGE: <lettre>`. Jouer avec `-Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` (candidats seulement sur ce disque, `Content\Audio\Candidates\soft\`).
  - `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings_stone -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
  - `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings_metal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
- **SONS, `[NE PAS JOUER]`** : `E-audio_panel.md` (inchangé ; pas de pré-vol ce soir).
- **`[NE PAS JOUER]`** : `E-disconnect_1v1.md` (net-playtest est maintenant dans main ; reste : copie recompilée puis `preflight.ps1 disconnect_1v1 -Root <copie> -PlayerName Alice` PASS).
- **LOGIQUE SEULEMENT** :
  - `E-neutrals_minimap.md` (d751-root, dans main ; seul FAIL du pré-vol = fichiers audio non commités, maintenant exclus : pré-vol à rejouer).
  - `E-deploy.md` (tiers, non fusionnée ; scénario `waitlog D=Pack` jamais satisfait, le jeu monte bien l'engin).
  - `E-spear_arm.md` (hero-j8-close, non fusionnée ; caméra min 15 m, prise de lance non jugeable à l'œil).
  - `E-playtest_hero_army.md` (pré-vol FAIL Sanity, spin de mêlée D641 non corrigé : voir `BUG-playtest_hero_army-spin.md`).
