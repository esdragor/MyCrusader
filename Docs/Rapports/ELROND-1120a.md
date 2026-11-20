NON COMPILÉ

# ELROND-1120a : D861 (b) engin monté + déplacement

APPELS_DECLARES: ~27 (dépassé de 2 : les 5 derniers réservés ont été entamés par deux Edit refusés « not read »)
HEURE_DEBUT/HEURE_FIN: 09:03 / ~09:09 (Get-Date)
PREMIER_BUILD: sans build (NON COMPILÉ, demandé)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: non lancé (aucun build dans l'agent)
TEST_ROUGE: d9d930b (test + champ stub bWalkHeld non lu) ; assertit l'effet : oui (ordre de marche ouvert, vitesse de consigne > 0 une fois Packed + 2 pas). Rouge NON VU (rien lancé).
COMMITS: 2 (d9d930b test, 78be07e correctif), dernier 09:07, merge_main_fait: non (branche = main 901e391)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lot C++ du jour : `Tools\build.ps1 -LiveCoding=0 -Wait` sur C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, puis test `HeroesOfLiberty.Siege.Deploy` : PressWhileWalking, PlayerButton, InMatch, Processor, DeployedAutoFire doivent rester verts + MoveWhileDeployedKeepsOrder)
TESTS_SCRIPT_AJOUTES: 0 (le test est C++ : simulation Mass, D838)
TYPES_A_EXPOSER: aucun

## CAUSE CORRIGÉE (le rapport ETAPE0-1120-A disait « StuckSteps clôt l'ordre » : insuffisant)
- `HOLDeployProcessor.cpp:86-90` met `DesiredSpeed` à 0 tant que l'engin n'est pas Packed.
- `HOLUnitOrderProcessor.cpp:3853` ne ré-arme la vitesse d'une marche QUE si `DesiredSpeed > 0` : le 0 posé pendant `Packing` survit donc à `Packed`, l'ordre reste ouvert mais l'engin ne bouge plus. Cause réelle pour tout but à plus de 500 cm.
- `StuckSteps` (`:4008`) ne compte que si le but est à <= `StuckCountDistance` (500 cm) et ne s'éloigne pas : il ne clôt l'ordre que pour un but proche (le test prend 400 cm pour couvrir aussi ce cas). Hypothèse partielle, pas la cause générale.
- Il n'y avait pas à « garder l'ordre dans le fragment » : l'ordre (`FHOLMoveOrderFragment`) n'est pas perdu ; ce qui se perd est la vitesse.

## FAIT
- `HOLDeployProcessor.h` : `uint8 bWalkHeld` dans `FHOLDeployFragment`.
- `HOLDeployProcessor.cpp` : à la coupure de vitesse, si une marche est ouverte (`Orders[It].Phase != None`) -> `bWalkHeld = 1` ; à Packed sans marche -> remis à 0.
- `HOLUnitOrderProcessor.cpp` : (1) `:3853` ré-arme la vitesse quand `bWalkHeld` et phase Packed (recherche du fragment seulement pour une marche à vitesse 0) ; (2) `:4008` ne compte pas les pas d'immobilité pendant la retenue.
- Test `HeroesOfLiberty.Siege.Deploy.MoveWhileDeployedKeepsOrder` (HOLDeployButtonTests.cpp) ; include `MassNavigationFragments.h` ajouté.

## RESTE
- Cause (a) (attaque engin démonté) : NON TRAITÉ, tests `AttackWhilePackedDeploysThenFires` / `AtTheCombatReachButBeyondDeployRange` non écrits (plafond d'appels). HOLCombatProcessor.cpp non touché.

## NON VÉRIFIÉ
- Compilation (includes `FMassMoveTargetFragment::DesiredSpeed.Get()` dans le test, `EHOLMovePhase` visible de HOLDeployProcessor.cpp : déjà utilisé là).
- Dans le harnais sans mouvement, la marche d'un Mangonel peut rester en `WaitingPath` avec une vitesse 0 : alors le test ne vérifie que l'ordre ouvert (assertion de vitesse conditionnelle à `SpeedPacked > 0`) ; le test peut donc être VERT avant correctif si seul `StuckSteps` ne se déclenche pas. Si le test est vert avant le correctif, le rouge réel est à voir par le scénario `deploy` (marche longue).
- Les fins de ligne : git signale « LF will be replaced by CRLF » sur 3 fichiers édités (eol_check.ps1 -Fix à passer si diff de fins de ligne inattendu).

PROCESSUS_DETACHES: aucun processus détaché pour C:\Users\baudo\Documents\HeroesOfLiberty-Elrond
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: modifs de docs d'autres sessions (AubeInProgress, Auto_Journey_File, DuelInProgress, EtapeDefini, Placeholders, PlaceholdersInProgress, SignalsInProgress) + mes lignes de Palier3InProgress/Auto_Journey_File/EtapeDefini ; code inchangé : oui
