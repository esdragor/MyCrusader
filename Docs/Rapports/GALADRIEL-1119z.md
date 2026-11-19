# GALADRIEL-1119z : lot R2a (portes d'ordre de ApplyOrders + annulation), worktree HeroesOfLiberty-Galadriel, branche galadriel-tab

Impact joueur : aucun visible (mêmes refus, mêmes lignes de journal, même ordre des effets) ; l'ordre des portes d'un ordre de production et la décision d'annulation sont désormais décidés par le script.

La recette R2-PLAN-1119 s'est révélée juste sur le découpage : étapes 1, 2 et 3 faites. NON COMPILÉ, aucun build, aucun quick_check, aucun jeu lancé.

## Commits (branche galadriel-tab, anglais, sans Co-Authored-By, pas de push, pas de merge)
- 88bcc44 Script tests : `Script\Tests\HOLProductionBoard_Test.as` (4 tests : `Test_ProductionBoard_OrderGates`, `_OrderGateOrder`, `_RallyGates`, `_Cancel`)
- 281c49c Script : `Script\Buildings\HOLProductionBoard.as` (enums `EHOLOrderGateVerdict`, `EHOLProductionCancelVerdict`, namespace `HOLProductionBoard`, classe `UHOLProductionBoardScript`)
- (3e commit, voir `git log`) C++ : `Source\HeroesOfLiberty\Buildings\HOLProductionBoard.h/.cpp` NEUFS + `HOLProductionSubsystem.cpp` (include, portes G1-G5, annulation)

## Forme
- Pont (raison D839 : liaison à exposer) : `UHOLProductionBoard` (UWorldSubsystem, Abstract), calquée sur `HOLConstructionBoard`. `AskOrderGate(World, bHasState, bOwner, bHasType, bFinished, bEarlyRally, bForumDown, bRally)` -> 0 passe, 1 sans état, 2 pas son bâtiment, 3 sans type, 4 non fini, 5 forum en ruine ; `AskCancel(World, bValidIndex)` -> 0 annule, 1 ignore ; `AskStockAfterCancel(World, Stock, Cost)`. `_Implementation` = -1 (repli C++) ; sans carte, `AskStockAfterCancel` rend Stock + Cost.
- `ApplyOrders` : les faits sont lus en C++ (`bOwner`, `Type`, `bEarlyRally`, `bFinished`, `bForumDown`, gardés par `bOwner` donc aucun déréférencement de bâtiment nul) puis `AskOrderGate` ; si -1, la même chaîne de portes est calculée en C++ (jumeau, retiré plus tard). Les 4 lignes de journal (texte, niveau Warning pour G1) sont inchangées ; G3 reste silencieux.
- Annulation : `AskCancel` ; 0 + index valide = `AskStockAfterCancel` puis `Queue.RemoveAt` ; 1 = `continue` silencieux ; -1 = `HOLProduction::Cancel` comme avant. Le journal `cancelled` et les effets 483-484 (`SetResources`, `NotifyProductionChanged`) sont inchangés.
- Règle non recopiée : le coût rendu passe par `HOLProductionRead::TryEnqueue` / `Cancel` (règle C++) sur une file brouillon d'un objet ; seul l'ordre des portes est neuf.
- NON touchés : `AdvanceQueues`, `GetCost`, `GetBirthLocation`, naissance, population, `AskQueue` (R2b), issue du Rally (R3), `HOLProduction::TryEnqueue/Cancel`.

## Rouge à voir (valeur à fausser)
Dans `HOLProductionBoard.as` `OrderGateVerdict` : passer `if (!bOwner)` à `if (bOwner)` (casse `OrderGates` et `OrderGateOrder`), ou `if (!bFinished && !bEarlyRally)` à `if (!bFinished)` (casse `RallyGates` : un point de ralliement d'un bâtiment non fini serait refusé). Rouge côté C++ : retirer `bEarlyRally` de la chaîne de repli ne se voit qu'en jeu (`rpc` ou `quick_check` scénario Rally `:1542`).

## Preuves à rejouer (par la session principale)
1. Un build (lot C++ de la séance) : `Tools\build.ps1 -LiveCoding=0 -Wait` dans le worktree Galadriel, vérifier `Get-StaleBinaryProblem` avant.
2. `quick_check` : les 4 tests `Test_ProductionBoard_*` (rouge puis vert) ; total des tests = ancien + 4, jamais en baisse.
3. Scénarios de production : `quick_check.ps1` `:763` (Peasant done), `:790` (`its forum is down`, G5), `:1369`, `:1542` (Rally) ; `rpc_orders_p1p2_run.ps1`, `rpc_orders_p3p4_run.ps1`, `rpc_routes_run.ps1`.
4. Tests C++ d'accord inchangés : `Production.PopulationCheckedAtEnd`, `Production.UnitTier`, `HOLDisconnectTests.cpp:196/:467`.
5. Annulation en jeu : mettre en file puis annuler (ligne `cancelled`, stock rendu), annuler un index hors file (aucune ligne, stock inchangé).

## Risques de compilation (non compilé)
- Script : `n"Item"` (FName littéral) et les appels `UPARAM(ref)` de `HOLProductionRead::TryEnqueue/Cancel` dans `StockAfterCancel` ; la valeur de retour de `TryEnqueue` est ignorée.
- Un seul sous-classe script d'`UHOLProductionBoard` doit exister ; noms d'enums uniques (`EHOLCancelVerdict` de la construction n'est pas réutilisé).

## Champs
BUILDS_DEMANDES: 0 (le lot C++ R2a attend le build de la séance) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 4 (Test_ProductionBoard_OrderGates, _OrderGateOrder, _RallyGates, _Cancel) ; LIGNES_CPP_SUPPRIMEES: 0 (jumeau de repli gardé) ; LIGNES_MIGREES_PAR_LOT: ~35 (G1-G5 + annulation) ; TYPES_A_EXPOSER: aucun ; TEST_ROUGE: pas vu (non compilé, à voir au build) ; EDITS_REFUSES: 0 ; PROCESSUS_DETACHES: aucun lancé.

## Anomalies
Aucune sur la recette. EtapeDefini.md et Auto_Journey_File.md non touchés (consigne).
