# [RÉPONDU] E-ecran-statistiques : l'écran Statistiques (écrit en script) s'ouvre depuis le bandeau Victoire, a ses 4 onglets, ses 3 tableaux et son graphe à 6 séries, identiques à l'écran C++ d'avant

STATUT : VU À L'ÉCRAN
GESTE: interne   (le scénario clique par commandes ; l'utilisateur fera les vrais clics au waitkey)
SON: sans objet
branches: hero-j8-close

Pré-vol (2026-11-18 21:26, session principale, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, main 1d6647d) : `preflight.ps1 ecran_statistiques -TestMap -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn` PASS (40 s, 8 PNG).
Recette indépendante (agent RECETTE-1118a, 2026-11-18, par lecture des PNG seulement, rien lancé) : 8 captures APRÈS (Celeborn, script) comparées une à une aux 8 captures AVANT (Sauron, écran C++), même scénario. AUCUNE différence d'interface vue (cadre, taille 1066 x 600, titres, onglets, en-têtes, lignes, nombres, alignements, boutons de série, axes, graduations, courbes, légende, repère et libellé, Retour). Seul le décor 3D change légèrement d'une capture à l'autre (ignoré).
PNG OUVERTS et vus (APRÈS, chacun comparé à son AVANT) :
- ecran_stats_bandeau : partie sur la base de l'IA en décombres, bandeau Victoire au centre (Continuer en surbrillance, Quitter vers le menu, Statistiques) ; identique à l'écran C++
- ecran_stats_resume : fenêtre Statistiques, onglet Résumé allumé (doré) ; colonnes Joueur, Équipe, Résultat, Score militaire, Score économique, Total ; Joueur 1 (pastille bleue) Équipe tiret, Vainqueur, 0 / 0 / 0 ; IA Normal 2 (pastille rouge) tiret, Vaincu, 0 / 0 / 0 ; Retour en bas ; identique à l'écran C++
- ecran_stats_militaire : onglet Militaire allumé ; colonnes Joueur, Tués, Perdus, Bâtiments perdus, Armée max ; Joueur 1 0 / 0 / 0 / 6 ; IA Normal 2 0 / 8 / 3 / 8 ; identique à l'écran C++
- ecran_stats_economie : onglet Économie allumé ; colonnes Joueur, Nourriture, Bois, Pierre, Or, Produits, Péons max, Population max ; Joueur 1 0 0 0 0 0 5 11 ; IA Normal 2 0 0 0 0 0 0 8 ; identique à l'écran C++
- ecran_stats_chronologie : fenêtre plus haute, onglet Chronologie allumé ; rangée de 6 boutons (Péons allumé, Armée, Population, Bâtiments, Ressources récoltées, Stock) ; graphe axe 0 à 6 / 0:00 à 0:15, courbe bleue Joueur 1 de 0 à 5, rouge IA à plat sur 0, repère vertical rouge à 0:13 « IA Normal 2 vaincu », légende Joueur 1 / IA Normal 2 ; identique à l'écran C++
- ecran_stats_serie_armee : bouton Armée allumé ; axe 0 à 10 ; bleue jusqu'à 6, rouge jusqu'à 8 (les deux s'arrêtent à 0:10), même repère et légende ; identique à l'écran C++
- ecran_stats_serie_stock : bouton Stock allumé ; axe 0 à 25000 par 5000 ; bleue de 1000 environ à 20500, rouge presque à plat vers 800 ; même repère et légende ; identique à l'écran C++
- ecran_stats_retour : écran Statistiques fermé, plus de tableau ni de graphe, bandeau Victoire revenu avec la note du scénario (4 points, « prêt ») ; identique à l'écran C++
Remarque sur le .look (même chose avant et après, donc pas une régression) : pour serie_armee il dit « la courbe de l'IA tombe à la fin » ; sur les deux versions la courbe rouge monte à 8 puis s'arrête à 0:10 sans retomber (pas d'échantillon après la mort). À corriger dans le .look, pas dans l'écran.
Autre remarque, identique avant/après : le bouton Statistiques du HUD est partiellement caché derrière Diplomatie sur la capture Résumé (« S » visible) ; il apparaît normalement sur les captures suivantes.
NON VÉRIFIÉ : les nombres valent presque tous 0 dans cette partie courte, donc le groupement des milliers et les grands nombres ne sont pas vus ; un spectateur (non joué) ; l'ouverture depuis le bouton Statistiques du HUD en cours de partie (la capture montre seulement l'ouverture depuis le bandeau Victoire) ; les séries Population, Bâtiments et Ressources récoltées (non capturées) ; la touche Échap et la manette ; le fond 3D (non comparé). Les lignes « Stats screen: » du journal scenario_ecran_statistiques.log n'ont pas été relues.

Geste (2 min) :
1. Lance la commande ci-dessous ; le scénario ouvre l'écran Statistiques par le bandeau Victoire et parcourt les onglets et séries tout seul.
2. Regarde : les 3 tableaux (Résumé, Militaire, Économie) ont les mêmes colonnes et les mêmes nombres que d'habitude.
3. Chronologie : clique les 6 séries, le graphe change à chaque clic et reste identique à l'ancien (axes, courbes, légende, repère « vaincu »).
4. Retour et Échap ferment l'écran sans perdre la partie (bandeau Victoire encore là).

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle, jamais « même commande avec… ».
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Scenarios\play.ps1 ecran_statistiques -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`
Durée : environ 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
Reponse: OK (utilisateur, 2026-11-19 ~00:40, dans la conversation ; essai joué sur le worktree d'essai ; bascule fusionnée ensuite, voir Auto_Journey_Jour.md « Agents du 2026-11-19 »)
