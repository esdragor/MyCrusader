APPELS_DECLARES: 9 (vus par moi)
HEURE_DEBUT/HEURE_FIN: 17:15 / 17:18
PREMIER_BUILD: sans C++   BUILD: aucun (D830)   LAUNCHCHECK_DEBUT: sans objet (lecture seule, aucun jeu)
TEST_ROUGE: sans objet (diagnostic des 2 rouges existants, aucun code modifié)
COMMITS: 0
TESTS: aucun lancé (lecture du code : Source\HeroesOfLiberty\Tests\HOLStartingHeroTests.cpp:591-770) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
FAIT / RESTE:
- Coupable : D831 (84f2b30, 16:43, Config\DefaultEngine.ini [SystemSettings] hol.Nomad.ForumCell=2), lancé sans quick_check. PAS D816 (97f757c, 15:19, BuildTime 0 -> 150) : quick_check 1320/1320 à 15:19 après lui, et aucun des 2 tests n'attend la fin du chantier (NomadFirstForum compte la fondation juste posée ; NomadReplicated appelle NoteSeatForumComplete à la main).
- NomadFirstForum : Lists() exige Each.Age == 1 && Each.Cell == 10 (l. 610, assertion l. 638 « the forum in the builders' list, page Palier II, cell C ») ; PressKey(Grid, 1, 10) l. 648 et ClickAge(..., 1, 10) l. 652 attendent le forum en C. Le cvar met le forum en case 2 (E) : ces assertions tombent pour le Nomade. La pose (HOLPlayerOrders::Place) et le prix 150/400 ne dépendent pas de la case.
- NomadReplicated : ForumCell() lit HOLCommandGrid::GetAgeBuilding(Grid, 1, 10) en dur (l. 717) ; « client, slot 0: cell II C builds the forum » (l. 739) et « not greyed » (l. 740) tombent.
- Le cvar est lu dans [SystemSettings] aussi par l'exécution des tests (même projet, même DefaultEngine.ini), d'où le rouge sur galadriel-tab.
- Correctif proposé (C++, donc avec la migration D830) : les tests lisent hol.Nomad.ForumCell / hol.Nomad.ForumAge (IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Nomad.ForumCell"))->GetInt()) au lieu de 10 / 1 ; aligner en même temps le défaut du cvar 10 -> 2 (HOLBuildingTypes.cpp:61). Sans C++ : seul retour arrière possible = retirer la ligne de DefaultEngine.ini, ce qui défait le réglage de l'utilisateur (non proposé).
- known_reds.txt (copie principale) : 2 lignes ajoutées (test | 2026-11-16 | cause D831).
- Ligne `[ ]` en tête de la section Celeborn de Auto_Journey_File.md.
NON VÉRIFIÉ: le message exact des FAIL dans le journal de Galadriel (pas retrouvé dans les 4 derniers .log de Saved\Logs, noms confirmés par ARWEN-1116d.md l. 8) ; d'autres tests C++ à case 10 en dur (CELEBORN-1116d cite HOLCommandGridTests, indépendants du cvar selon lui, non relus).
ANOMALIES_PROCESSUS: KnownReds.ps1 (Find-KnownRed) compare au nom de PARTIE imprimé par quick_check (« tests »), pas au nom du test C++ : les 2 lignes ajoutées documentent mais n'empêcheront pas quick_check d'annoncer « REGRESSION » pour la partie tests. Une ligne « tests | ... » masquerait toute autre régression : non ajoutée. Note mise en commentaire dans known_reds.txt.
GIT_STATUS_PRINCIPALE: M Tools/Tests/known_reds.txt, M Auto_Journey_File.md, ?? Docs/Rapports/CELEBORN-1116e.md (voulus) ; le reste déjà là au départ
