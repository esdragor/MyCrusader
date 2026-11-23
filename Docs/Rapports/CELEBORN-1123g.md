APPELS_DECLARES: ~17 (arret a 20)
HEURE_DEBUT/HEURE_FIN: 10:33 / 10:44
PREMIER_BUILD: sans C++   BUILD: sans objet   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scenario)   TEST_ROUGE_SEPARE: non (pas de code)
COMMITS: 1 sur integ-41 (4176e88, 10:43), merge_main_fait: non (non demande)
TESTS: preflight tower_wood 3 passages : check PASS (61-70 s) ; preflight global FAIL(1) = "2 uncommitted file(s)" avant commit (tower_wood.look), corrige par le commit ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: tower_wood : LOGIQUE SEULEMENT, AUCUNE fleche vue. Pas de fiche E.
  Reglage 1 (Zoom 7, Focus 5600, waitlog "lands on") : ennemis (3 taches rouges) tout en haut, sous le bandeau de notes (y 0-40), tour hors cadre en bas ; during/2/3/after identiques, aucune fleche.
  Reglage 2 (Zoom 3, Focus 5000) : la tour bleue remplit le bas de l'ecran (derriere le panneau de commandes), ennemis toujours en haut a y 0-40 (rouge net dans after, barre de vie non lisible), entre les deux de l'herbe et un arbre : aucune fleche.
  PNG ouverts : before (tour mi-cachee par la grille de commandes, arbres, aucun ennemi), during, during2, during3, after (x2 reglages).
CAUSE (regle 7 de "Qualite des essais") :
  - Les ennemis restent a la portee maximale de la tour (30 m) : la tour tire des qu'ils entrent, a 30 m, et UnitSpeed 0.2 les ralentit au point qu'ils bougent a peine pendant 90 images. Le trajet de la fleche (30 m) est plus haut que la bande utile de l'ecran (entre bandeau de notes y~160 et grille de commandes y~440 = ~280 px) : a Zoom 3 (~600 px pour 30 m) la tour est cachee par la grille et les ennemis par le bandeau ; a Zoom 7 plus encore.
  - La ligne "releases an arrow at" (HOLCombatProcessor.cpp:3323) n'existe que pour les unites (chemin 3291) ; les fleches de tour (HOLCombatProcessor.cpp:953 et :2751) n'ont pas ce journal : le waitlog "releases an arrow at" a fait un timeout (step 44) ; seul "lands on" (:1665) est disponible, donc la capture "during" tombe a l'impact, pas au lancer.
  - Pas vérifie : la fleche est-elle dessinee en -RenderOffScreen et sa taille (non testable tant qu'elle n'est pas dans le cadre) ; a verifier en dessinant : Source (affichage des fleches, ArrowDisplay) .
FAIT: tower_wood.txt (UnitSpeed 0.2, ordre Move puis Focus 5000 Zoom 3, waitlog lands on, captures during/during2/during3/after) et .look mis a jour, commit 4176e88.
RESTE (suite proposee) : (1) laisser les ennemis marcher jusqu'a ~12-15 m de la tour (UnitSpeed 0.4, Move cible 6 m, capture ~ +60 images), Focus a mi-distance (rel ~4700), Zoom 0-1 pour que tour+ennemis tiennent dans la bande 160-440 px ; (2) a defaut une camera haute plus penchee ou placer les Lanciers a 15 m par SpawnHere pres de la tour (rel 5500) pour que le trajet soit court ; (3) lot C++ eventuel : ajouter un log "releases an arrow" aux chemins :953/:2751 pour un waitlog sur le tir.
NON VERIFIE: fleche en vol sur PNG ; dessin de la fleche hors -RenderOffScreen ; PV entames ; geste reel ; son.
QUESTIONS: aucune
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: 1 appel perdu (python heredoc interactif lance par erreur, bloque 120 s en tache de fond b30abshkm, sans effet) ; premier Edit (Write) au 5e appel ; une ligne -Append au lieu de remplacer dans EtapeDefini/Auto_Journey_File (lignes neuves a la fin).
GIT_STATUS_PRINCIPALE: docs seuls modifies (EtapeDefini, Auto_Journey_File + deja modifies) : oui
