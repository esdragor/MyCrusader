# GALADRIEL-1121sc : accesseur des coups reçus (alerte « attaque ailleurs »)

NON COMPILÉ (C++ à relire, RELECTURE_CPP, lot C++ groupé de la séance).

Impact joueur : l'alerte « attaque ailleurs » aura enfin de quoi lire les bâtiments du joueur touchés (nombre, lieu, instant) ; les coups sur unités ne sont pas encore couverts.

APPELS_DECLARES: ~24 (arrêt à 25 vus)
HEURE_DEBUT/HEURE_FIN: 19:33 / 19:35 (Get-Date, horloge de l'agent)
PREMIER_BUILD: sans C++ (aucun build, consigne du brief)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 9659c97 (stub qui compile : GetHitsReceivedNow rend vide / false, ObserveHitsReceived ne compte rien) + « assertit l'effet : oui » (le compte, le lieu et l'instant calculés par la règle, puis la ligne ShouldAlertAttackedElsewhere)
COMMITS: 2 (9659c97 test + stub, c6060a0 accesseur réel), dernier à 19:35, merge_main_fait: non (pas de build, DLL non vérifiée ; GetMinorEventsNow est déjà dans la branche)
TESTS: aucun lancé (pas de build). TESTS_TAUTOLOGIQUES: 0
Rouges attendus sur 9659c97 (par nom) : Test_HitsReceivedCountedOnceWithPlaceAndTime (Script\UI\HOLHitsReceived_Test.as : 1 attendu après un coup, obtenu 0), Test_HitsReceivedFeedsTheAttackedElsewhereAlert (idem). Les deux verts sur c6060a0 (trace à la main : 0 / 1 / 1 / 3, lieu du coup le plus récent, temps 9 puis 20).
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (le lot C++ de la séance) ; TESTS_SCRIPT_AJOUTES: 2 (Test_HitsReceivedCountedOnceWithPlaceAndTime, Test_HitsReceivedFeedsTheAttackedElsewhereAlert) ; TYPES_A_EXPOSER: FHOLHitsWatch (USTRUCT), UHOLScriptScreen::ObserveHitsReceived (statique), GetHitsReceivedNow ; DIAGNOSTICS_AS: non relus (pas d'éditeur)

FAIT :
- Source\HeroesOfLibertyUI\HUD\HOLScriptScreen.h/.cpp (le dossier réel, pas Source\HeroesOfLiberty\UI) : `FHOLHitsWatch` (HitsReceived, LastHitLocation, LastHitSeconds ; bPrimed C++), `GetHitsReceivedNow(int32& OutHits, FVector2D& OutLastHitLocation, double& OutLastHitSeconds) const` (BlueprintPure, même modèle que GetMinorEventsNow : bâtiments du joueur lié, `AHOLBuilding::GetLastHitTime()` déjà répliqué/local), règle statique `ObserveHitsReceived(Watch, HitPlaces, HitSeconds)` (première lecture = base, un coup compte une fois, le plus récent donne lieu et temps).
- Script\UI\HOLHitsReceived_Test.as : 2 tests.
- La règle script existante `HOLAlertRules::ShouldAlertAttackedElsewhere(Watch, bHitInView, Now, Repeat)` ne lit aucun nom d'accesseur : aucun alignement nécessaire.

RESTE :
- Branchement dans `Script\UI\HOLGameScreenScript.as` (fichier non pris) : une `RefreshAttackAlert()` appelée comme RefreshMinorAlerts, qui lit GetHitsReceivedNow, détecte un compte en hausse, calcule bHitInView par le lieu contre la vue de la caméra, appelle ShouldAlertAttackedElsewhere avec Gameplay::GetTimeSeconds (même horloge monde que LastHitTime).
- Coups sur UNITÉS : aucun enregistrement côté client (besoin C++ séparé, Mass / réplication).
- Lancer : build du lot C++, puis `quick_check -Parts tests`.

NON VÉRIFIÉ : compilation (types `TArray<double>` / `double&` liés au script supposés comme `float` script ; si le binder refuse, passer à float32), les deux tests (ni rouge ni vert vus à l'exécution), le sens de LastHitTime sur un client (ligne HOLBuilding.cpp:330 / 377, non relue).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: inchangé hors docs voulus : oui (EtapeDefini.md, Auto_Journey_File.md modifiés par moi ; le reste était déjà modifié)
