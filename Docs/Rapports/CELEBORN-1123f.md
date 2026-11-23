APPELS_DECLARES: ~24
HEURE_DEBUT/HEURE_FIN: 10:23 / 10:32
PREMIER_BUILD: sans C++   BUILD: sans objet   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scenario d'essai)   TEST_ROUGE_SEPARE: non (pas de code)
COMMITS: 3 sur integ-41 (dernier 10:32), merge_main_fait: non (non demande)
TESTS: preflight tower_wood PASS (check 50 s) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: tower_wood : preflight PASS, PNG ouverts (before, during, during2, after), LOGIQUE SEULEMENT : tour (boite bleue) visible, 3 Lanciers rouges visibles en haut de l'ecran, AUCUNE fleche reperee en vol (waitlog 'lands on' passe donc des fleches ont touche), barres de vie non lisibles ; pas de fiche E
QUESTIONS: aucune
FAIT: tower_wood.txt/.look/.json (Tools\Scenarios, integ-41) ; corrige : coordonnees Units.Move en MONDE (tour a -10800,0 pour PlaceNear 4000 0 relatif au centre -14800), mapSize Small, UnitSpeed 0.4. RESTE : cadrage (les ennemis finissent sous le bandeau de notes en haut, fleche trop fine) : essayer Focus plus pres de la tour, capture des l'entree en portee, ou hol.Cheat.UnitSpeed 0.2.
NON VERIFIE: fleche en vol sur PNG ; PV entames visibles ; geste reel ; son.
PROCESSUS_DETACHES: voir stray_processes
ANOMALIES_PROCESSUS: premier Edit apres le 6e appel (j'ai du lire d'abord les modeles) ; aucune
GIT_STATUS_PRINCIPALE: docs seuls modifies (EtapeDefini, Auto_Journey_File + deja modifies) : oui