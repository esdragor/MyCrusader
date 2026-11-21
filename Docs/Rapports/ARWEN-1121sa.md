APPELS_DECLARES: 10 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 19:12:27 / voir fin de rapport (Get-Date ~19:14)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (Angelscript seul, aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 990a147 (bouchon IsVisibleToCamp renvoie true + 3 tests) ; assertit l'effet : oui (vue/non vue calculee par la regle). ROUGE ATTENDU A L'EXECUTION : 1 rouge = Test_StealthCampViewEnemyBeyondRevealDistanceDoesNotSee (N+1 doit etre non vue ; le bouchon renvoie vue). Les 2 autres (Test_StealthCampViewEnemyAtRevealDistanceSees, Test_StealthCampViewAllyAlwaysSees) passent deja sur le bouchon. Rouge NON VU par l'agent (pas de quick_check).
COMMITS: 3 (990a147 bouchon+tests, d886e79 regle, journal), dernier ~19:14, merge_main_fait: non (branche = main 6b226f8, rien a fusionner)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (la valeur N vient de HOLStealth::RevealDistance() / cvar ; visibilite calculee par IsVisibleToCamp -> IsHiddenFrom)
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 0   TESTS_SCRIPT_AJOUTES: 3 (Test_StealthCampViewEnemyBeyondRevealDistanceDoesNotSee, Test_StealthCampViewEnemyAtRevealDistanceSees, Test_StealthCampViewAllyAlwaysSees)
TYPES_A_EXPOSER: aucun type pour la regle ; pour le BRANCHEMENT (suite C++) il manque une liaison : positions des unites d'un camp (UHOLVisionSubsystem, par slot de camp) lisible depuis le script, et filtre reseau UHOLFogNetSubsystem (D518/D526) appelant la regle. Non code.
FAIT: HOLStealth::IsVisibleToCamp(Zones, UnitCamp, UnitPosition, ViewerCamp, ViewerPositions, Distance) dans Script\Fog\HOLStealthRules.as : meme camp = vue, sinon !IsHiddenFrom (distance inclusive 800 par defaut ou meme zone). Camp en int (pas de type camp script verifie).
RESTE: branchement C++ vue par camp, filtre reseau, aspect (bosquet, contour), zones sur cartes (L125). Commande a lancer par la session principale : quick_check -Parts tests dans C:\Users\baudo\Documents\HeroesOfLiberty-Arwen (build non requis, script seul ; verifier session_gate Launch, DLL possiblement en retard).
NON VERIFIE: compilation Angelscript, diagnostics (DIAGNOSTICS_AS non releves), rouge/vert a l'execution. Fins de ligne : les .as ont ete reecrits en LF (git avertit LF->CRLF), sans diff de contenu parasite attendu. Placeholders.txt non touche (aucun provisoire ajoute ; forme rectangulaire de zone deja provisoire).
PROCESSUS_DETACHES: voir stray_processes ci-dessous
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: inchange hors docs voulus (EtapeDefini.md modifie par moi ; GDD.md et Docs\ deja modifies avant) : oui