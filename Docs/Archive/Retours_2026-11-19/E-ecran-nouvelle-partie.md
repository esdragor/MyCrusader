# [RÉPONDU] E-ecran-nouvelle-partie : l'écran Nouvelle partie (écrit en script) s'ouvre depuis le titre avec sa grille de 8 places, ses listes qui se grisent et ses boutons Lancer / Retour, identiques à l'écran C++ d'avant

STATUT : VU À L'ÉCRAN
GESTE: interne   (le scénario manipule la grille par commandes ; l'utilisateur fera les vrais clics au waitkey)
SON: sans objet
branches: d751-root

Pré-vol (2026-11-18 22:34, session principale, worktree d'essai C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, d751-root 675da2a, main ac48996) : `preflight.ps1 ecran_nouvelle_partie -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (25 s, 5 PNG ; journal : menu=HOLNewGameScreenScript). Premier pré-vol de l'écran script : 22:00 dans le worktree Sauron (celui des captures de la recette).
Sur ce même worktree, écran script à l'écran (2026-11-18 22:26-22:34) : `quick_check -Parts lobby` PASS (salon réseau à deux joueurs, Alice et Bob, partie normale et choix de faction ; classe HOLNewGameScreenScript chez Alice), tests 1543/1543, menu fr / en / ja / ailaunch PASS, smoke PASS.
Recette indépendante (agent RECETTE-1118c, 2026-11-18, par lecture des PNG seulement, rien lancé ; rapport C:\Users\baudo\Documents\MyCrusader\Docs\Rapports\RECETTE-1118c.md) : 5 captures APRÈS (script, 22:00) comparées une à une aux 5 captures AVANT (écran C++, 20:44), même scénario, même résolution 1066 x 600. Les 5 paires sont IDENTIQUES octet pour octet (mêmes empreintes SHA-256), donc zéro pixel d'écart. Les deux réserves de la recette 1118b (cadre de la liste Taille, étiquette « dev » et largeur des boutons du titre) étaient dues à la résolution différente (1280 x 720) et sont levées.
PNG OUVERTS et vus (APRÈS, chacun comparé à son AVANT) :
- ecran_np_ouverture : « Nouvelle partie » ; Carte Plaine, Taille Petite (2 joueurs), Brouillard Sans brouillard, Équipes ensemble Oui ; grille de 8 places : Vous (Ordre de l'Aube, Bleu, équipe 1), IA Normal 2 avec liste Normal grisée (Rouge, équipe 2), 6 places Fermé grisées ; Mode Standard, Nomade / Régicide / Catastrophe grisés, Sites sacrés Non ; Retour, Lancer (focus doré) ; identique à l'écran C++
- ecran_np_ia_niveau : Taille Moyenne (4 joueurs) ; IA Normal 2 et IA Normal 3 avec leur liste Normal grisée, pastilles Bleu / Rouge / Vert, équipes 1 / 2 / Aucune ; place 4 Fermé actif, places 5 à 8 grisées ; identique à l'écran C++
- ecran_np_couleur : places 2, 3, 4 sur IA ; pastilles Bleu, Rouge, Jaune, Turquoise toutes différentes (la place 4 a sauté à la couleur libre suivante, focus brun-doré sur sa liste) ; équipes 1 / 2 / Aucune / Aucune ; identique à l'écran C++
- ecran_np_taille_liste : liste Taille ouverte sous le champ, « — », Petite (2 joueurs) grisée, Moyenne (4 joueurs) cernée de clair, Normale, Grande, Immense en clair, cadre de liste doré ; identique à l'écran C++
- ecran_np_retour : écran titre « Heroes of Liberty » avec Jouer (focus doré), Multijoueur, Réglages, Quitter, étiquette « dev » ; identique à l'écran C++
Les notes jaunes du scénario masquent en partie la ligne du haut sur les captures 3 et 4 (même chose avant et après, donc pas une régression).
NON VÉRIFIÉ : le mode salon À L'ŒIL (le test automatique du salon passe, mais aucune capture du salon n'a été comparée à l'écran C++ : lignes des autres joueurs grisées, Prêt, Spectateurs, options du chef) ; le clavier / la manette (flèches, LB-RB, Échap, navigation entre colonnes) ; les autres tailles, cartes, factions et options (Brouillard, Équipes ensemble, Mode, Sites sacrés changés) ; Lancer jusqu'à la carte (les captures s'arrêtent avant) ; un PNG ne prouve pas quelle implémentation l'a dessiné (le script est établi par le pré-vol, non relu par la recette) ; les lignes de journal de l'écran n'ont pas été relues.

Geste (3 min) :
1. Lance la commande ci-dessous ; le scénario ouvre Nouvelle partie depuis le titre, change une taille, passe des places en IA, ouvre la liste Taille, puis revient au titre.
2. Regarde : la grille de 8 places est la même qu'avant, et les listes se grisent quand une taille ne tient pas le nombre de joueurs (Petite grisée avec 4 joueurs).
3. Une place passée en IA montre son niveau, et une couleur déjà prise saute à la suivante.
4. Lancer ouvre la carte, Retour ramène au titre.

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle, jamais « même commande avec… ».
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 ecran_nouvelle_partie -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 3 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
Reponse: OK (utilisateur, 2026-11-19 ~00:40, dans la conversation ; essai joué sur le worktree d'essai ; bascule fusionnée ensuite, voir Auto_Journey_Jour.md « Agents du 2026-11-19 »)
