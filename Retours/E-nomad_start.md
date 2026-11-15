# [À ESSAYER] E-nomad_start : une partie Nomade commence sans aucun bâtiment ; tes Paysans naissent en groupes DISPERSÉS sur TOUTE la carte, ton Roi avec l'un, ton héros avec un autre ; ton premier Forum se construit dès le départ (Paysans, Z = page Palier II, puis C) pour 150 bois + 400 pierre

REGLAGE de l'utilisateur 2026-11-14 : D797, codé (maps 65953a3) ; défaut d'affichage de la grille (page II estompée) corrigé (maps ef6fc88) ; rejoué 2026-11-14 13:23 (HALDIR-1114g)

STATUT : VU À L'ÉCRAN (D797) : (a) dispersion, (b) case C « Centre principal » nette sur la page Palier II, (c) fondation payée 150/400, vus sur les PNG ; GESTE interne (commandes `hol.Select.*`, pas de vraie touche Z ni de vrai clic : à juger par toi) ; `preflight` FAIL (1) = seulement `?? Content/Maps/Crown.umap` non suivi dans le worktree (motif connu, sans rapport avec l'essai)
GESTE: interne      (commandes `hol.Select.Type Peasant`, `hol.Select.Grid 1` = touche Z, `hol.Select.Grid 10` = touche C, `hol.Select.LeftClickGround` ; pas de vraie touche ni de vrai clic de l'agent)
SON: non entendu      (`-nosound` ; l'essai ne porte pas sur un son)
branches: maps

Rejoué 2026-11-14 13:23 (agent HALDIR-1114g, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, branche maps ef6fc88 ; build PASS 13:20 ; `quick_check -Parts tests` 1266/1266 PASS) : `play.ps1 nomad_start -Hidden -Root <worktree>` : le jeu se ferme seul après 24,2 s (quit du scénario) ; `preflight.ps1 nomad_start -Root <worktree>` : check PASS (28 s, 3 PNG), aucun avertissement du journal, aucun chevauchement du HUD, sanity OK ; seul FAIL : « user's launch: 1 uncommitted file(s) ... ?? Content/Maps/Crown.umap ».
Correctif (ef6fc88) : le panneau d'une page de la grille n'est estompé (opacité 0,55) que si aucune de ses cases n'est constructible au palier atteint (`HOLCommandGrid::GetAgePanelOpacity`, lit `IsAgeOpen`) ; les cases verrouillées restent grises une à une ; le médaillon garde son aspect (question Q-nomad-forum-palier, non tranchée).
Journal exact (Saved\Logs\HeroesOfLiberty.log) : « Nomad slot 0: no main centre, its units dispersed around (-14800, 0) », « Regicide: king of slot 0 spawned at (-4114, -8653) », group 0 (-3664, -8653), group 1 (17392, 17958), group 2 (-17959, -4229), group 3 (8060, -15795), group 4 (-5364, 15262), « Starting hero of slot 0 spawned at (16942, 18258) » ; « Grid: age 1 chosen » ; « Construction: slot 0 placed MainCentre foundation at (-14800, -3600), paid F 0 W 150 S 400 G 0 (F 200 W 200 S 200 G 200) ».
PNG OUVERTS et vus :
- nomad_start_wide (7000 cm) : aucun bâtiment ; un Paysan bleu en bas à gauche, une unité rouge de l'IA à gauche ; minicarte : points bleus en plusieurs coins éloignés (haut droite, gauche, bas). (a) VU.
- nomad_start_grid : « Paysan (5) », « 5 inactifs » ; page I pleine ; page II (Z) maintenant PLEINE (plus estompée) : C « Centre principal » en texte clair, net ; Marché, Atelier de siège, Chapelle, Écurie, Forge en texte gris (verrouillés) ; pages III et IV restent estompées. (b) VU.
- nomad_start_forum (3000 cm) : fondation du Forum (fil de fer bleu clair) à gauche d'une forêt, trait d'ordre blanc vers elle ; barre : Bois 200, Pierre 200 ; « 5 en marche » ; C « Centre principal » grisé après paiement (pierre 200 < 400, attendu). (c) VU.
NON VÉRIFIÉ :
- Vraie touche Z puis C (seulement les commandes `hol.Select.Grid`) ; l'infobulle de la case C.
- Fondation posée à (-14800, -3600) pour un clic visé à (-14800, -2500) (11 m, non cherché) ; pose acceptée dans le brouillard.
- Unités de l'IA (Nomade aussi) près de ton départ : voulu ou non, non tranché.
- Les médaillons I et II ont le même aspect sur la capture (médaillon non touché, Q-nomad-forum-palier).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 nomad_start -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Historique :
- 2026-11-13 22:30 (HALDIR-1113m) puis recette 3 23:05 : départ Nomade sans bâtiment, Roi + 5 Paysans + héros en rangée au point du départ, VU À L'ÉCRAN.
- Réponse de l'utilisateur 2026-11-14 : « BUG: je ne pouvais pas construire de forum, je n'avais pas l'option ; il faut que je puisse le construire exceptionnellement alors que normalement c'est palier 1 ; et en Nomade tout le monde devrait être dispersé, pas au même endroit. »
- 2026-11-14 11:23 (HALDIR-1114b) : point 1 corrigé, case C et paiement 150/400 vus, fondation cachée par un buisson.
- 2026-11-14 12:01 (HALDIR-1114d) : départ dispersé dans un disque de 60 m (d2c3ab6), forum page I case C, fondation visible, VU À L'ÉCRAN.
- 2026-11-14 12:43 (HALDIR-1114e) : D797 codé (65953a3) : carte entière, forum page II case C ; question Q-nomad-forum-palier [À RÉPONDRE].
- 2026-11-14 13:12 (HALDIR-1114f) : rejeu ; (a) (c) vus, (b) défaut d'affichage (page II estompée).
- 2026-11-14 13:23 (HALDIR-1114g) : page II pleine (ef6fc88) ; (a) (b) (c) vus, VU À L'ÉCRAN.

## Recette 2026-11-15 (build fusionné 753fd00)
RECETTE PASS (RECETTE-1115a, intégrateur indépendant, 2026-11-15 11:19 ; worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, branche fog = main 753fd00, binaire de 11:08).
- `preflight.ps1 nomad_start -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` : PASS, 0 FAIL, 0 WARN (check PASS 21 s ; journal sans avertissement ; HUD sans chevauchement ; sanity OK ; lancement joueur OK : le FAIL « Crown.umap non suivi » de Haldir n'existe pas sur Faramir). `session_gate.ps1 -Phase Essay -Essay nomad_start` : aucun FAIL (le worktree contient le tip de maps).
- Journal (Saved_Tests\Logs\scenario_nomad_start.log) : « Nomad slot 0: no main centre, its units dispersed around (-14800, 0) », roi (-4114, -8653), groupes 0 à 4 de 1 Paysan chacun à (-3664, -8653), (17392, 17958), (-17959, -4229), (8060, -15795), (-5364, 15262), héros (16942, 18258), « Building report: 0 buildings », « Grid: age 1 chosen », « Construction: slot 0 placed MainCentre foundation at (-14800, -3600), paid F 0 W 150 S 400 G 0 (F 200 W 200 S 200 G 200) » : mêmes lignes et mêmes positions que le rejeu Haldir du 11-14.
- PNG OUVERTS et vus : nomad_start_wide (aucun bâtiment bleu ; un Paysan bleu en bas à gauche ; minicarte : petits points bleus éloignés, coin haut droit, gauche, bas) ; nomad_start_grid (« Paysan (5) », « 5 inactifs », page II : C « Centre principal » en clair, Marché / Atelier de siège / Chapelle / Écurie / Forge gris, pages III et IV estompées) ; nomad_start_forum (fondation en fil de fer bleu clair, Bois 200, Pierre 200, « 5 en marche », C grisé après paiement).
- Réserves cherchées pour faire échouer, non bloquantes : (1) la fondation est à GAUCHE du centre du cadre (posée à (-14800, -3600) pour un clic visé à (-14800, -2500)), collée à la lisière de la forêt, alors que le `.look` dit « au centre du cadre » ; (2) le panneau du scénario affiche « en préparation… » sur les 3 captures (la note n'arrive qu'à la fin) ; (3) les points bleus de la minicarte sont très petits (1 à 2 px sur la capture 1920x1080) : la dispersion se lit mieux au journal qu'à l'œil ; (4) une unité rouge de l'IA près du départ, « Population 5 / 0 » : non jugés.
- Inchangé : GESTE interne (`hol.Select.*`), SON non entendu (`-nosound`).
- Commande de la fiche (Haldir) gardée : binaire de Haldir non périmé (Get-StaleBinaryProblem vide à 11:18, dll 10:10, HEAD fffe788). NON REJOUÉ depuis Haldir : cette recette ne porte que sur le build de Faramir ; Haldir ne contient pas 753fd00.
