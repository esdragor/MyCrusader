# ELROND-1121c : TowerWood F8, partie sans C++ (2026-11-21)

Impact joueur : la Tour de guet en bois existe dans la table des bâtiments avec 3 places de garnison ; elle ne tire pas encore (C++ de F8) et ne se pose pas encore tant que la grille/placement n'est pas testé.

APPELS_DECLARES: 18 (arrêt à 25 vus ; seuil de 80 jugé sur tool_uses de la session principale)
HEURE_DEBUT/HEURE_FIN: 10:58 / 11:02 (arrêt dur 11:18)
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (aucun C++ touché)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 84add13 (Script\Tests\HOLTowerWood_Test.as, 3 tests) ; rouge NON VU à l'exécution (aucun lancement dans cet agent) ; assertit l'effet : oui (la table connaît TowerWood, GarrisonSlots calculé par HOLGarrison::SlotsOfRow vaut 3, AdmittedInto admet autant d'occupants que les places de la ligne)
COMMITS: 3 sur integ-25 (84add13 test seul, 482e26b CSV + uasset régénéré, ff7f583 journal), dernier à 11:01, merge_main_fait: non (travail sans C++, DLL non concernée ; la session principale synchronise)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0 (SlotsOfRow lit la ligne de table, AdmittedInto est la vraie règle ; le 3 est la valeur D793 du GDD)
ESSAIS: aucun
QUESTIONS: C:\Users\baudo\Documents\MyCrusader\Retours\Q-towerwood.md (A/B/C/Autre sur les chiffres ; défaut A = ce qui est dans le CSV, une ligne à changer puis regen_tables.ps1)
FAIT / RESTE:
- Fait : test script rouge (commit seul) ; ligne `TowerWood` ajoutée à la FIN de `Tools\Data\BuildingTypes.csv` (Name TowerWood, « Tour de guet en bois », 800x800, hauteur 800, 800 PV, armure tir 40, 100 bois, 20 s, case 7, palier 0, Military, non praticable, BlockAndTower, vision 4800, Common, GarrisonSlots 3) ; `regen_tables.ps1` : DryRun puis réel, exit 0 ; seul `Content\Buildings\BuildingTypes.uasset` a changé (UnitTypes inchangé) ; diff du CSV = 1 ligne `+`, aucune ligne `-` (L18-a) ; la chaîne TowerWood est bien dans l'uasset.
- Valeurs hors GDD (PV, taille, coût, temps, case, vision) : proposées, question posée, rien n'est tranché. Le GDD ne donne que : palier 0, 3 places (D793), non praticable, bois craint mêlée et siège (D692).
- CONFLIT PRÉVU : HALDIR-1121c ajoute SecondaryCentre au MÊME BuildingTypes.csv sur une autre branche. Ma ligne est tout à la fin du fichier (comme demandé) : conflit de fin de fichier, à résoudre en gardant les deux lignes. Le `BuildingTypes.uasset` (binaire) sera en conflit certain : à régénérer depuis le CSV fusionné (regen_tables.ps1), jamais « theirs ». La case 7 du palier 0 est peut-être convoitée par la Palissade ou le Centre secondaire (une cellule du CSV).
- Reste (C++, lot groupé) : colonnes d'attaque de bâtiment (type, dégâts, cadence, portée, flèches de base et par occupant), processeur de tir à côté de HOLCombatProcessor (réutiliser HOLArrow), règle « bois : pas le tir » (D692), acquisition de cible, maillage provisoire (cube par défaut), texte HOLBuildingTypeNames / nom en/ja, scénario tower_wood. Débloque en script : F7 4c/4d (HOLAIGarrisonRules, une tour à places existe).
NON VÉRIFIÉ: rien n'a été lancé (ni quick_check ni test script) : le rouge de HOLTowerWood_Test.as et son vert ne sont pas vus ; un test C++ existant qui compte les types constructibles du palier 0, ou qui suppose la case 7 libre, ou GarrisonSlots 0 partout (HOLCommandGridTests, HOLBuildingTests, HOLGarrisonOrderTests ajoute sa propre ligne de tour) peut passer au rouge ; le diff de l'uasset est binaire (relu via le CSV et la présence de la chaîne) ; la signature `FHOLAITypeFigures` / `AssertFalse` des tests est supposée d'après HOLForge_Test.as.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Elrond
ANOMALIES_PROCESSUS: un UnrealEditor-Cmd (pid 16444) tournait déjà au lancement de la régénération (autre agent) ; regen_tables.ps1 n'attend pas de créneau lui-même (aucune attente dans le script), il est passé sans collision (exit 0). A signaler à l'outil si un second éditeur sur le MÊME worktree est possible.
GIT_STATUS_PRINCIPALE: modifs de docs seulement (EtapeDefini.md, Auto_Journey_File.md, Retours\Q-towerwood.md, ce rapport) en plus de celles déjà présentes : oui

Commande de tests (session principale, un éditeur à la fois, build d'abord si le binaire est périmé) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\quick_check.ps1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond -Parts tests` (rouge attendu avant 482e26b : 3 tests HOLTowerWood_Test ; vert sur 482e26b ; puis `-Parts cpp` pour les tests de grille/garnison).
