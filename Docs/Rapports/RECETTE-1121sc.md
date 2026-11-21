# RECETTE-1121sc : recette independante de E-secondary_centre (worktree HeroesOfLiberty-Haldir, wall-line = main 204f769 + scenario)

VERDICT : PASS (fonction), avec une reserve sur la capture "pendant".

APPELS_DECLARES: 9 (arret 25 vus)
HEURE_DEBUT/HEURE_FIN: 20:36 / voir fin (arret dur 20:58)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build, aucun code)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (recette, aucun code ecrit)
COMMITS: 0, merge_main_fait: non (recette)
TESTS: preflight.ps1 secondary_centre -Root HeroesOfLiberty-Haldir : PASS (40 s), "no warning or error in the log", "sanity audit passed", 5 points OK ; TESTS_TAUTOLOGIQUES: sans objet
ESSAIS: secondary_centre : preflight PASS, 5 PNG ouverts avec Read (sc_grille_age2, sc_grille_infobulle, sc_pendant, sc_apres, sc_forum)

## Ce que chaque PNG montre (vu, pas lu dans le journal)
- sc_grille_age2 : 5 Paysans selectionnes ; page II allumee (badge II doré, touche Z) : A Marche, Z "Centre secondaire", Q Atelier de siege, S Chapelle, D Ecurie, W Forge. Pages III et IV grisees. OK. La case Z n'affiche pas son cout (identique aux autres cases : seule l'infobulle le donne).
- sc_grille_infobulle : infobulle presente a droite de la case Z : "Centre secondaire", 150 bois, 400 pierre, "+10 population", "Depot : nourriture, bois, pierre, or", "Produit : Paysan". OK. Elle recouvre en partie les cases E/S (Chapelle tronquee) : normal pour une infobulle.
- sc_pendant : fondation en fil de fer (cadre bleu pale) a l'ouest du forum, ligne de chemin depuis les Paysans ; bois 5050 (-150), pierre 4800 (-400) : cout paye a la pose. Panneau "Paysan (5) / 5 en marche" (Inactifs : 0). MAIS : les 5 Paysans sont encore a droite, loin de la fondation, en marche : AUCUN chantier partiel, AUCUNE barre de construction, personne ne batit. Le .look dit "chantier partiel avec des Paysans autour qui le batissent" : non conforme a l'image.
- sc_apres : Centre secondaire fini, SEUL selectionne (panneau "Centre secondaire / Vous / Points de vie 2400 / 2400"), grille : seule la case A Paysan est allumee ; message "Batiment acheve" ; ressources 5050 / 4800 inchangees depuis la pose. Un bosquet masque le bas-gauche du batiment (cadrage, sans gravite).
- sc_forum : Centre principal : A Paysan + D Voie A du palier 1 + F Voie B du palier 1 + W Cloche d'alarme ameliorée ; "Vies 3". Le centre secondaire n'a ni voies ni cloche ni Vies (voir Q-secondary-centre-grid.md, deja ouverte).

## Reserves (ce qu'un joueur remarquerait)
1. Pendant la construction : la capture ne prouve ni la barre de construction ni l'avancement visuel ; on ne voit que la fondation avant l'arrivee des batisseurs. A re-cadrer (capture apres arrivee des Paysans, ex. a mi-chantier) avant de dire "construction VUE".
2. Modele SM_Forum identique au Centre principal (provisoire, deja dans Placeholders.txt) : sc_apres et sc_forum se ressemblent a s'y meprendre ; seul le panneau de selection les distingue.
3. Aucune pop visible changee : "Population 5 / 175" avant et apres le centre fini, alors que l'infobulle annonce "+10 population" : le plafond reste 175 sur sc_apres (a verifier : le Centre principal donne-t-il deja une part du 175 ? Un +10 attendu donnerait 185). A VERIFIER, peut-etre un defaut reel.

NON VERIFIE : son ; barre de construction en cours ; effet reel de +10 population (point 3) ; jeu reel hors capture (aucun lancement fenetre).
PROCESSUS_DETACHES: voir sortie stray_processes en dernier geste (reponse finale)
ANOMALIES_PROCESSUS: aucune vue
GIT_STATUS_PRINCIPALE: non lance (aucune ecriture hors ce rapport et la ligne de fiche)

recette: RECETTE-1121sc PASS sur wall-line (main 204f769 + scenario)
