# [À ESSAYER] E-secondary_centre : poser un Centre secondaire (touche Z, Z, clic), le voir se bâtir, puis le sélectionner seul

STATUT : VU À L'ÉCRAN (page II de la grille, construction, centre fini sélectionné seul) ; l'infobulle de la case Z n'est PAS visible sur la capture (voir NON VÉRIFIÉ)
GESTE: réel
SON: non entendu
branches: wall-line
recette: RECETTE-1121sc PASS sur wall-line (main 204f769 + scenario) ; reserves traitees par HALDIR-1121sj (voir Mise a jour 1121sj) : sc_pendant recadre apres le debut reel du chantier ; Population 5/175 = plafond de partie atteint par la triche du scenario (pas un defaut) ; rapport Docs\Rapports\RECETTE-1121sc.md

Pré-vol (2026-11-21 20:28, agent HALDIR-1121sh, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, main 204f769) : `preflight.ps1 secondary_centre -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir` PASS (43 s, 5 PNG).
PNG OUVERTS et vus :
- sc_grille_age2 : 5 Paysans sélectionnés, page II de la grille allumée (badge II doré) ; case Z = « Centre secondaire » (avec A Marché, Q Atelier de siège, S Chapelle, D Écurie, W Forge). Ressources 5200 partout.
- sc_grille_infobulle : identique à sc_grille_age2, AUCUNE infobulle visible près de la case Z (la souris est sur GridCell01 mais rien ne s'affiche en capture).
- sc_pendant (rejoué 1121sj, après le début réel du chantier) : fondation du Centre secondaire, 5 Paysans au pied du bâtiment, panneau « Paysan (5) / 5 en construction » ; Population 5 / 10 ; bois 5050, pierre 4800. Pas de barre de construction visible (le panneau des Paysans donne l'état).
- sc_apres : Centre secondaire FINI, SEUL sélectionné (« Centre secondaire », Vous, 2400 / 2400 PV), grille : seule la case A Paysan est allumée ; message « Bâtiment achevé » ; Population 5 / 20 (10 du forum + 10 du centre secondaire).
- sc_forum : Centre principal sélectionné : même case A Paysan, mais en plus D Voie A du palier 1, F Voie B du palier 1 et W Cloche d'alarme améliorée ; « Vies 3 ».
Journal : « Grid: age 1 chosen » (touche Z), « Placement: SecondaryCentre (cost F 0 W 150 S 400 G 0) » (touche Z), « Construction: slot 0 placed SecondaryCentre foundation at (-16800, -6000) » (clic), 5 bâtisseurs, « finished ... after 190 steps ... speed 6 » (hol.Build.Speed 6), « Selection: SecondaryCentre of slot 0 (own) ».
NON VÉRIFIÉ : l'infobulle de la case Z (nom, 150 bois, 400 pierre) n'apparaît pas en capture ; la barre de construction en cours ; le son ; le modèle SM_Forum est provisoire (Placeholders.txt) ; la grille du centre secondaire n'est PAS identique à celle du forum (seul A Paysan, pas de voies de palier ni de cloche) : à trancher si voulu.

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle.
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 secondary_centre -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
Durée : environ 3 min (Entrée à chaque étape). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

## Mise à jour HALDIR-1121si (20:35)
- Infobulle de la case Z : cause = le scénario survolait GridCell01 (case de la grille de commandes) au lieu de AgeCell101 (case Z de la page II du panneau des bâtisseurs) ; ce n'était pas un défaut de données. Corrigé dans le scénario (commit sur wall-line : `hol.Mouse.Hover AgeCell101 40`, puis waitlog "Mouse hover AgeCell101" et "tooltip shown"). PNG sc_grille_infobulle ouvert : « Centre secondaire, bois 150, pierre 400, +10 population, Dépôt : nourriture, bois, pierre, or, Produit : Paysan ». Préflight secondary_centre PASS (40 s), sanity ok.
- Grille du Centre secondaire (A Paysan seul) : le GDD (D384, D879) ne tranche pas D/F/W ; question Retours\Q-secondary-centre-grid.md [À RÉPONDRE], valeur actuelle = Paysan seul.
- Correction de la note de l'étape 2 : le scénario déplace le curseur avant la pose pour que le centre tombe où le clic de sélection regarde (curseur garé sur GridCell01).

## Mise à jour HALDIR-1121sj (20:43)
- Population 5 / 175 : PAS UN DÉFAUT. Plafond d'un joueur = somme des PopulationProvided des bâtiments finis (HOLProductionSubsystem.cpp:584, C++) + triche, bornée à 175 par HOLProduction::GetPopulationCap (HOLProductionRules.cpp:5, MaxPopulation = 175 en HOLProductionRules.h:61) : GDD D03 (population maximale 175 par joueur), D83. Le scénario faisait hol.Cheat.Population 200 : forum 10 + 200 = 210, borné à 175, donc le +10 ne se voyait pas. Triche retirée : Population 5 / 10 pendant le chantier, 5 / 20 une fois fini (PNG vus). Aucun code touché, pas de test rouge.
- sc_pendant : le scénario attend « Construction: slot 0 first work on SecondaryCentre » (HOLBuildProcessor.cpp:942), ramène hol.Build.Speed à 1, attend 2 s, capture, puis remet 20 pour finir. Le PNG montre les 5 Paysans au pied de la fondation, « 5 en construction ». Commit c2cdee6 sur wall-line. Préflight PASS (sanity ok).
- Rejouer : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 secondary_centre -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
