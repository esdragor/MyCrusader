# DIAG-1121sh : integ-47, erreurs de compilation Angelscript

HEURE_DEBUT/HEURE_FIN: 21:22 / 21:25 (arret dur 21:38) ; appels ~9
BUILD: PASS (pas de nouveau C++) ; TESTS: `quick_check -Parts tests` = 2062/2062 PASS (avant : 0/0 FAIL, module non compile)

## Cause
Les 5 erreurs du journal (`Saved_Tests\Logs\quick_tests.log`) sont une seule cause, le piege 3 du brief :
`Function RightClickOnUnit / ManualOrder is not BlueprintCallable or ScriptCallable and cannot be called from here`.
- `HOLFollowBridgeRequests_Test.as:24`
- `HOLFollowBridge_Test.as:23, 42, 43, 60, 72`
Ces methodes sont les surcharges `BlueprintOverride` de `UHOLFollowBridgeScript` (`BP_RightClickOnUnit`, `BP_ManualOrder` de `HOLFollowBridge.h`) : un test script ne peut pas les appeler (meme precedent que `HOLGameMenuRulesBridge_Test.as`, 2026-11-17).
Le `SpawnActor(AHOLPlayerController)` de Requests_Test n'a pas ete atteint (le compilateur s'arrete avant) : non verifie, sans precedent.

## Correctif (commit ad9fc7e sur integ-47, anglais)
- `Script\Orders\HOLFollowBridge_Test.as` : les 3 tests appelant les surcharges sont retires ; un test de classe (`Test_FollowBridgeClassExists`, `Get()` non nul + nom de classe `HOLFollowBridgeScript`) est garde et passe. La regle (ApplyRightClick, OnManualOrder) garde ses vecteurs dans `HOLFollowOrders_Test.as`.
- `Script\Orders\HOLFollowBridgeRequests_Test.as` : supprime (3 tests, tous passaient par `Bridge.RightClickOnUnit`).
- Aucune regle ni code C++ touche.

## Ligne de file a ajouter (Glorfindel)
« Reecrire en C++ comme HOLSignalGroundMarkTests.cpp : HOLFollowBridgeTests.cpp = (a) clic droit allie -> NativeRightClickOnUnit demarre le Follow (livre des unites 1 et 2 dans UHOLFollowBookHolder), ennemi / unite de la selection = pas de Follow ; (b) NativeManualOrder([1]) met fin au Follow de l'unite 1 et laisse l'unite 2 ; (c) RequestMoveUnitIdsTo / RequestAttackMoveUnits / RequestMoveUnitsInLineOf d'un AHOLPlayerController (spawn en C++) appellent NativeManualOrder. Les fonctions Native* sont non-UFUNCTION : tests C++ seulement. »

## Champs
TEST_ROUGE: sans objet (retrait de tests non compilables) ; TESTS_TAUTOLOGIQUES: 0 ; COMMITS: 1 (ad9fc7e), merge_main_fait: non (integ-47 = main c9ac21c + verts)
DIAGNOSTICS_AS: 0 erreur (module compile : 2062 tests lances)
BUILDS_DEMANDES: 0 ; TESTS_SCRIPT_AJOUTES: 1 (Test_FollowBridgeClassExists, remplace 3) ; TYPES_A_EXPOSER: aucun
NON VERIFIE: le test de classe n'est vert que si le nom `HOLFollowBridgeScript` est bien celui du journal (vu vert, 2062/2062) ; SpawnActor d'un contrôleur en script jamais essaye.
PROCESSUS_DETACHES: aucun lance par moi (quick_check termine) ; ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: non touche (aucun fichier ecrit hors ce rapport)
