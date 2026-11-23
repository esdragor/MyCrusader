# RECETTE-1123pb (2026-11-23, fin 16:38) : recette indépendante sur Haldir (integ-58, main 86a2e15 fusionné)

Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, binaire compilé 16:30. Aucun code de jeu touché.

## wall_line_long : PASS
- Préflight PASS x3 (le 1er avant retouche ; le 2e FAIL « fichiers non commités » après retouche, puis PASS après commit 096b1a8).
- Retouche de scénario : `# branches: main` (.txt et .look) ; `frames 30` -> `frames 70` après `hol.Mouse.Drag` (à 30 images la capture « during » ne montrait que 4 boîtes) ; `.look` recalé sur le viewport MESURÉ 1066x600 (le .look disait 1280x720 / 7 à 8 boîtes / 97 px).
- during (rejeu final) : 7 boîtes vertes plates + la dernière encadrée au curseur = 8, pas ~92 px, aucune rouge, Bois 20200 (0). Le Paysan est sélectionné, grille âge I visible, touche D = Palissade.
- after : 9 fondations bleu-gris alignées (la dernière coupée par le bord droit), plus de boîte verte, Bois 20155 = 20200 - 9x5. Journal : `Placement: Palisade wall line (-17711, -1375) to (-17711, 1982)` (3357 cm / 400 = 8,4 cases -> 9), `RpcOrder: place wall line received by the server`.
- Anomalie : le point d'appui varie d'un rejeu à l'autre (y -2378 puis -1375) car la caméra n'est pas au même endroit au moment de l'appui ; résultat identique (9 fondations).
- Fiche : Retours\E-wall_line_long.md, branches: main, VU À L'ÉCRAN, GESTE réel, commande Haldir sans -Hidden. Gate Essay : `=> aucun FAIL`.

## ai_garrison : PASS (avec réserve de comptage)
- Préflight PASS (37 s). Journal : `Garrison order: slot 1, 3 unit(s) queued (AI)`, `AI slot 1 step 149: 1 garrison order(s)`, `Garrison: slot 1 puts 3 of 3 units in TowerWood (3 places, 0 inside before)`.
- before : la tour seule au centre, aucun défenseur ; Forum de l'IA en haut à droite avec 5 à 6 silhouettes rouges à son pied (pas à la tour).
- during : gros plan, 2 à 3 silhouettes rouges serrées qui se chevauchent au pied de la tour ; le « 4 silhouettes » du `.look` n'est PAS lisible (corrigé en « 2 à 4 » dans ai_garrison.look, commit bde5deb). Capture 12 images avant l'ordre de garnison.
- after : UNE seule silhouette rouge dehors ; la disparition est cohérente avec le journal (3 sur 3 entrés, la tour a 3 places), mais le nombre exact d'archers dehors avant n'est pas lisible : la preuve est journal + une silhouette restante.
- Fiche : Retours\E-ai_garrison.md, branches: main, VU À L'ÉCRAN, GESTE interne, commande Haldir sans -Hidden. Gate Essay : `=> aucun FAIL`.

## faction_click_cast (essai 3) : NON JOUÉ
Budget d'appels (25) atteint avant, pas d'observation sur le gain d'Honneur au soin.

## Défauts du jeu
Aucun vu. Aucune ligne `[ ]` proposée.

## Commits (integ-58, Agent: RECETTE-1123pb)
096b1a8 (wall_line_long .txt/.look), bde5deb (ai_garrison .look).

## NON VÉRIFIÉ
Pose réelle à la souris matérielle en fenêtre visible (hol.Mouse.Drag injecté, viewport caché) ; construction des fondations par les Paysans ; ligne diagonale / ligne bloquée ; compte exact des Archers sur ai_garrison_during ; sons ; faction_click_cast.

## Dernier geste
`stray_processes.ps1 -Stop` : aucun processus détaché.
