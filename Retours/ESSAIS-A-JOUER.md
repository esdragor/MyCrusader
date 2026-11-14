# ESSAIS À JOUER (nettoyage du 2026-11-13 soir, étape 0a de l'Auto_Journey)

**Prêts à donner : 0** au nettoyage de 16:1x (aucune fiche E-* n'avait un pré-vol PASS sur une copie à jour). **Mise à jour 18:0x (ARWEN-1113d)** : 2 écoutes de sons passées `[À ESSAYER]` (Pierre, Forge ; voir « SONS » ci-dessous) : l'écran est vu, le son ne peut être jugé que par toi.

Version précédente (matin du 2026-11-13) : `Docs\Archive\Retours_2026-11-13_soir\ESSAIS-A-JOUER_2026-11-13_matin.md`. Essais joués à 14:20 (`resources_compare` = arbre C, `legions_roster_gesture` = OK) : reportés et archivés dans `Docs\Archive\Retours_2026-11-13\`.

## État réel au 2026-11-13 21:4x
- **Binaire de la copie principale PÉRIMÉ** : `UnrealEditor-HeroesOfLiberty.dll` du 2026-11-11 11:06, alors que main (15bbd25) porte le C++ des fusions groupées du soir. Recompiler la copie jouée avant tout pré-vol (ou jouer depuis un worktree à jour de main, comme Faramir).
- **Fusionnées dans main** : fusion groupée n°1 19:29 (dc67a16 : fog, maps, d751-root, visual, galadriel-tab, net-playtest, hero-j8-close), n°2 21:23 (15bbd25 : maps, net-playtest, tiers) ; heal-sleep aussi dans main. Les mentions « non fusionnée » de tiers / hero-j8-close plus bas sont donc périmées (E-deploy, E-spear_arm : statut à revoir par la recette). **Non fusionnée** : d751-root actuel (« Faction choice », autre session). RIEN POUSSÉ.
- **Fusion groupée du 2026-11-14 12:41 (main = 0ae4a56)** : d751-root (tour 3 Pierre/Forge : les WAV candidats restent sur le disque d'Arwen, jouer depuis Arwen), visual (rocks_compare), galadriel-tab (panneau du scénario et des triches traduits), maps (Nomade : premier forum, départ dispersé ; couvert des forêts retiré), hero-j8-close (prise B, chute du Lancier), tiers (garnison) sont DANS main. net-playtest fusionnée ensuite (main dd2a053, docs 154cf2b). RIEN POUSSÉ.
- **Accord de l'utilisateur du jour** : `Content/Audio/Candidates/` exclu par `.git/info/exclude` (partagé par tous les worktrees : le pré-vol ne doit plus échouer sur ces 294 fichiers non commités) ; `GDD.md` commité (D792, c17f8b1). Les sons candidats sont copiés dans le worktree qui joue l'essai (Faramir depuis 19:42, copie d'Arwen).

## Fiches encore dans Retours\ (8 essais, 9 fiches E-* avec l'index audio), aucune donnée
- **Mise à jour 19:4x (FARAMIR, recette réparée sur main dc67a16 + scénario fafdee9)** : `E-legions_warrior.md` pré-vol PASS, 3 PNG vus, VU À L'ÉCRAN : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 legions_warrior -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`.
- **SONS Pierre / Forge** : 3e tour répondu le 2026-11-14 (D798 : Atelier de siège Stone A, Forum Metal B, Forge Stone G) ; fiches archivées dans `Docs\Archive\Retours_2026-11-14_b\` ; tour 4 (voix, tirs, cheval) en préparation par ARWEN-1114d.
- **`[À ESSAYER]` VU À L'ÉCRAN avec réserve** : `E-neutrals_minimap.md` (correctif 12 px dans main ; carré ~8 px effectifs sur la capture : un REGLAGE de taille est possible).
- **SONS, `[NE PAS JOUER]`** : `E-audio_panel.md` (inchangé ; pas de pré-vol ce soir).
- **`[NE PAS JOUER]`** : `E-disconnect_1v1.md` (net-playtest est maintenant dans main ; reste : copie recompilée puis `preflight.ps1 disconnect_1v1 -Root <copie> -PlayerName Alice` PASS).
- **LOGIQUE SEULEMENT** :
  - `E-deploy.md` (tiers, non fusionnée ; scénario `waitlog D=Pack` jamais satisfait, le jeu monte bien l'engin).
  - `E-spear_arm.md` : jouée le 2026-11-14 : OK, archivée dans `Docs\Archive\Retours_2026-11-14_b\`.
  - `E-playtest_hero_army.md` (pré-vol FAIL Sanity, spin de mêlée D641 non corrigé : voir `BUG-playtest_hero_army-spin.md`).

## Mise à jour du 2026-11-14 (essais joués par l'utilisateur)
- OK, fiches archivées dans `Docs\Archive\Retours_2026-11-14\` : legions_warrior, neutrals_minimap, fog_orders, map_eldorado, talents, income_tooltip.
- BUG : nomad_start (forum non constructible au départ Nomade ; départ pas dispersé) ; audio Pierre et Forge (aucun candidat bon, volume trop faible).
- NON JOUES : market et talents_icons_compare (éditeur de Galadriel ne démarre pas : binaire périmé) : à relancer depuis Faramir.
- Mise à jour suivante : market et talents_icons_compare rejoués depuis Faramir : OK (fiches archivées). Restent seulement nomad_start et audio Pierre / Forge (en BUG, à refaire).

## Mise à jour du 2026-11-14 12:5x (réponses de l'utilisateur rangées, D797-D800)
- Archivées dans `Docs\Archive\Retours_2026-11-14_b\` : E-neutrals (OK), E-spear_arm (OK), E-rocks_compare + Q-rocks_compare (B et D : D800), Q-nomad-net (A : D799), Q-frames-refus (B : défaut Warn gardé), sons E-audio_compare_buildings + _stone + _metal (3e tour : D798).
- **À jouer maintenant : 1** : `E-rocks_compare2.md` `[À ESSAYER]` (VU À L'ÉCRAN, GESTE interne, branches visual), avec sa question `Q-rocks_compare2.md` : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 rocks_compare2 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` (réserve : pré-vol « lancement joueur » FAIL tant que Sauron n'est pas recompilé).
- **Pas prêts** : `E-nomad_start.md` (réglage D797 : forum au palier 2, en cours HALDIR-1114e ; question `Q-nomad-forum-palier.md`) ; sons tour 4 (voix, tirs, cheval) en préparation par ARWEN-1114d.
- Fusion groupée du jour : 12:41 main 0ae4a56, puis net-playtest (main dd2a053, docs 154cf2b). RIEN POUSSÉ.

## Clôture du 2026-11-14 13:4x (main f539041, 3 fusions groupées, RIEN POUSSÉ)
- Répondus et archivés dans `Docs\Archive\Retours_2026-11-14_c\` : rocks_compare2 + Q-rocks_compare2 (D802 : A, C, D aimés), Q-nomad-forum-palier (D803 : A), sons tour 4 barracks / range / stable (D801 : rejetés, tour 5 à refaire avec des sons naturels).
- **À jouer maintenant : 1** : `E-nomad_start.md` `[À ESSAYER]` (VU À L'ÉCRAN, GESTE interne, SON non entendu ; worktree Haldir, maps ef6fc88 dans main) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 nomad_start -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
- Les mentions « À jouer maintenant : rocks_compare2 » et « Pas prêts : nomad_start » plus haut sont périmées.
