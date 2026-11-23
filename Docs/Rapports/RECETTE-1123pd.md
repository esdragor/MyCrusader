# RECETTE-1123pd (2026-11-23 17:31-17:38, worktree HeroesOfLiberty-Haldir, main b5299e5, binaire 17:30)

Impact joueur : au relâché d'une ligne de palissade, le Paysan sélectionné part bâtir (vérifié en jeu) ; une tour garnie de 3 occupants lâche 4 flèches par volée (vérifié au journal).

## 1. wall_line_long : CORRIGÉ EN JEU (un Paysan sélectionné ; construction instantanée du scénario)
- preflight PASS deux fois (avant et après retouche), 3 PNG ouverts : during (7 boîtes vertes, Bois 20200), after (9 fondations, Bois 20155, Paysan « En marche »), build (Paysan au bout de la ligne, 3 premières cases pleines, reste en cadres).
- Journal : « Placement: 1 units sent to build the line » ; « RpcOrder: place wall line received by the server: slot 0 type Palisade builders 1 queue false » ; ordres « Build order of slot 0: 1 units to Palisade at (-17800, -1400) » puis 8 « queued » ; 5,4 s plus tard « Construction: slot 0 first work on Palisade at (-17800, -1400), 1 builders », « finished ... instant » ; idem (-1000) et (-600) avant la fin du scénario.
- Retouche de scénario (commit 66cb4c9 sur la branche du worktree, `Agent: RECETTE-1123pd`) : `waitlog Construction: slot 0 first work on Palisade` (texte vérifié HOLBuildProcessor.cpp:942), `wait 1`, `hol.Build.Report`, `capture wall_line_long_build` + ligne du `.look`. Preflight rejoué APRÈS le commit : PASS.
- Fiche : Retours\E-wall_line_long.md ; gate Essay : « => aucun FAIL ».
- NON VÉRIFIÉ : barre d'avancement / hauteur (hol.Build.Instant 1 du scénario) ; plusieurs Paysans sélectionnés (règle de répartition d'ARWEN-1123pe, non tranchée par le GDD) ; Maj ; réseau.

## 2. ai_garrison : CORRIGÉ EN JEU (nombres lus au journal, `hol.Combat.LogBuildingShots 1` ajouté au scénario dans le même commit 66cb4c9 ; preflight PASS 28 s ; PNG du scénario non rouverts, pas de fiche demandée)
- Avant l'entrée : « Combat: building shot step 115: #66537 TowerWood looses arrow 1 of 1 at #16 ManAtArms » (M = 1).
- « Garrison: slot 1 puts 3 of 3 units in TowerWood (3 places, 0 inside before) » à 17:35:47.
- Après : volées aux pas 147, 180, 212 (écart 32-33 pas = 1,6 s à 20 Hz), chacune « arrow 1 of 4 » à « arrow 4 of 4 » au même pas : M = 4 = 1 + 3 occupants, trois volées de suite.
- NON VÉRIFIÉ : l'aspect à l'écran des 4 flèches (même point de départ, donc superposées) ; le rouge sur 27f7476 ; les tests C++ BuildingShot ; d'autres bâtiments armés.

## Anomalies
- Un `python3 - <<EOF` vide lancé par erreur par moi s'est figé en REPL (comme FARAMIR-1123pg) ; arrêté par TaskStop, sans effet sur le dépôt.
- Les lignes `frames 40` / `frames 70` de wall_line_long donnent un WARN de check.ps1 (préexistant, non touché).

APPELS: ~21. PROCESSUS_DETACHES: voir stray_processes ci-dessous. Aucun code de jeu modifié (3 fichiers de scénario seulement).
