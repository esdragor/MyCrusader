# ARWEN-1123pe (2026-11-23, worktree HeroesOfLiberty-Arwen, branche integ-37)

NON COMPILE (aucun build, aucun quick_check : la session principale les lance). merge main non fait (consigne).

Impact joueur : au relache d'une ligne de palissade, les Paysans selectionnes partent construire les fondations (avant : fondations posees, personne ne construisait).

APPELS_DECLARES: ~17
HEURE_DEBUT/HEURE_FIN: 17:06 / ~17:10
PREMIER_BUILD: sans build   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build par l'agent)
TEST_ROUGE: d455b04 (Test_WallLineBuildersGetOrder + Test_WallLineNoSelectionNoBuilders, bouchon BuildersForCell / QueueForCell) ; assertit l'effet : en partie (la regle pure : 3 cellules x 2 Paysans = 6 ordres, file des cellules 1 et 2) ; le RPC lui-meme (siege + monde) n'est pas atteignable d'un test script. Rouge NON VU a l'execution.   TEST_ROUGE_SEPARE: oui
COMMITS: 2 (d455b04 test + bouchon, 8ffb822 correctif), merge_main_fait: non
TESTS: aucun lance. TESTS_TAUTOLOGIQUES: 0 (le nombre de cellules vient de HOLWallLine::PlanFromRequest, les ordres de BuildersForCell)
ESSAIS: aucun (wall_line_long a rejouer par l'integrateur apres build : regle 19 d'agent_rules)
QUESTIONS: aucune ecrite (voir ci-dessous la regle N/M, valeur par defaut codee)

## Fait
- d455b04 : `Script\Buildings\HOLWallLineRules.as` `HOLWallLine::BuildersForCell(CellIndex, UnitIds)` et `QueueForCell(CellIndex, bQueue)` en BOUCHON (aucun Paysan, drapeau inchange = comportement d'avant) ; `Script\Tests\HOLWallLineRules_Test.as` : `Test_WallLineBuildersGetOrder` (ligne de 3 cellules par PlanFromRequest, 2 Paysans : 6 ordres attendus, file false/true/true sans Shift, true avec Shift) et `Test_WallLineNoSelectionNoBuilders` (vert des le bouchon : sans selection, aucun constructeur).
- 8ffb822 : `HOLWallLineRules.as` : BuildersForCell renvoie tous les Paysans, QueueForCell = `bQueue || CellIndex > 0`. `Script\Net\HOLRpcPlayer.as` `ServerPlaceWallLine(FName TypeName, FVector_NetQuantize Start, FVector_NetQuantize End, TArray<uint32> UnitIds, bool bQueue)` : `NoBuilders` remplace par `HOLRpc::OrderIds(HOLWallLine::BuildersForCell(Index, UnitIds))` et `HOLWallLine::QueueForCell(Index, bQueue)` pour chaque cellule. `Source\HeroesOfLiberty\Player\HOLPlayerController.cpp` : `RequestPlaceWallLine` lit `GetCommandUnits()`, `NativeManualOrder(Builders)` + journal "Placement: N units sent to build the line" + `OnOrderGiven.Broadcast(Start)` seulement si non vide (comme le batiment seul :3406-3414) ; declaration (:90) et route `RoutePlaceWallLine` (+ `TConstArrayView<uint32> UnitIds`, `&Ids` en 9e argument de CallFull, qui range par TYPE : tableau, 2 vecteurs, nom, bool). `.h` inchange (la signature de RequestPlaceWallLine ne change pas).

## Regle de repartition N fondations / M Paysans (NON TRANCHEE par le GDD : GDD:3348 dit seulement "plusieurs Paysans en parallele")
Valeur codee par defaut, a confirmer : chaque Paysan recoit chaque cellule ; la 1re cellule suit Shift, les suivantes sont en file (le meme enchainement que 3 batiments poses avec Shift). Effet en jeu : les M Paysans construisent la 1re fondation ensemble, puis la 2e, etc. (pas 1 Paysan par fondation). Alternative non codee : repartition 1 Paysan par fondation (parallele pur). Aucun reglage cvar ajoute ; une ligne a changer : `HOLWallLine::BuildersForCell` / `QueueForCell` dans `HOLWallLineRules.as`.

## A lancer par la session principale
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\build.ps1 -LiveCoding=0 -Wait` (C++ : signature de route) puis `quick_check.ps1 -Parts tests` DU WORKTREE. Rouge a voir sur d455b04 (checkout du 1er commit), vert sur 8ffb822.
BUILDS_DEMANDES: 1   TESTS_SCRIPT_AJOUTES: 2 (Test_WallLineBuildersGetOrder, Test_WallLineNoSelectionNoBuilders)   TYPES_A_EXPOSER: aucun

## NON VERIFIE
Rien compile ni execute. Le comportement reel (les Paysans marchent vraiment, les fondations de la ligne sont toutes construites, la file) n'est pas prouve : le test ne couvre que la regle pure. Que `HOLRpcBridge::OrderPlaceBuilding` / `HOLPlayerOrders::Place` traite bien `bQueue` vrai pour la 2e fondation comme une mise en file des Paysans (corps de UHOLPlayerOrdersBoard::AskPlace non lu). Que le script charge (diagnostics AS non relus : DIAGNOSTICS_AS non releve). Un test Mass/C++ de bout en bout (ligne + 2 Paysans, ordres lus dans la file des unites) reste a ecrire.
Pas de refus a 0 Paysan ajoute (une ligne sans selection reste permise, comme un batiment seul sans selection) ; `HOLRpc::PlaceBuildingRefusal` pour 0 constructeur non relu.

## Anomalies
Le worktree contient des fichiers non suivis `Docs/Integrate_integ-*` (ils existaient avant moi, non touches). IN_USE.txt du worktree non mis a jour (pas de CPP: oui, aucun build).
PROCESSUS_DETACHES: aucun lance par l'agent. ANOMALIES_PROCESSUS: aucune.
