# HALDIR-1119z : lot P2 (HOLPlayerOrders, seconde moitie) - NON COMPILE

Impact joueur : aucun attendu (memes ordres, memes refus, meme chaine bChain, meme ligne de journal) ; chemin reel = RPC controleur / IA -> `HOLPlayerOrders::*` -> pont -> planche script -> regle script -> emetteur lie. Repli C++ garde sur -1.
Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`, branche `selection`. Aucun build, aucun quick_check, aucun jeu. EtapeDefini.md et Auto_Journey_File.md non touches.

## Verification des appelants (point dur)
Grep `HOLPlayerOrders::(Gather|Build|DropOff|Place|QueueProduction|MarketTrade|SetRally|Garrison|UngarrisonAll|TakesGarrisonOrder|SendNonWorkersTo)(` sur `Source\` : appelants = `AI\HOLAIOrders.cpp` (88, 236, 244, 251, 259, 267), `Player\HOLPlayerController.cpp` (RPC 4636-4709, 5956 ; TakesGarrisonOrder 2065, 2089, 2899) et des tests C++ (AIOrders, AISlot, GarrisonOrder, Gather, StartingHero). AUCUN dans `Simulation\` ni dans un processeur Mass. Le pont garde de toute facon `IsInGameThread()` (hors fil de jeu : -1, repli C++). Branchement fait.

## Commits (branche selection, rien pousse, pas de merge)
- `a787c75` liaison : `HOLCommandScriptLibrary.h/.cpp` : `IssueGatherPlayer`, `IssueQueuedWorkPlayer`, `IssueBuildPlayer`, `IssueDropOffMovePlayer` (FPlayerOrderScope ouverte DANS la fonction C++), `IssueRallyOrder` (avec bQueue, SANS portee : le corps C++ de `SetRally` n'ouvrait ni FPlayerOrderScope ni FRallyOrderScope, on copie), `IssueProductionTrade`, `TakesGarrisonOrder`, `OwnsBuilding` (lecteurs d'etat de jeu : CVar `hol.Garrison.RightClickEnters`, proprietaire). Place / QueueProduction / Garrison / UngarrisonAll reutilisent les emetteurs existants (`IssuePlace`, `IssueProductionQueue`, `IssueGarrison`, `IssueUngarrisonAll`). Relecture : `ConsumeGathers/Builds/Moves/Productions/Constructions/Garrisons` existants (pas de nouveau Consume).
- `f62cbf8` script + tests : `Script\Commands\HOLPlayerOrders.as` (9 regles ajoutees au namespace), `HOLPlayerOrdersBoard.as` (9 `Answer*` + 9 surcharges de la planche ; correction `HOLCommandScript::GetCommands()` sans `this`, ligne 56, en premier), tests `HOLPlayerOrders_Test.as` (+6), `HOLPlayerOrdersBoard_Test.as` (+3).
- `09f7e41` branchement : `HOLPlayerOrdersBoard.h/.cpp` (9 `Ask*` statiques + 9 evenements natifs `*Answer`, defaut -1) et les 9 corps de `HOLPlayerOrders.cpp`.

## Ordres migres (9 sur 9 de la seconde moitie)
Gather, Build, DropOff, Place, QueueProduction, MarketTrade, SetRally, Garrison, UngarrisonAll. `TakesGarrisonOrder` reste une fonction C++ (predicat d'etat : CVar + batiment + `UHOLGarrisonSubsystem::GetSlots`), exposee en lecteur ; les 3 appelants du controleur (2065, 2089, 2899) ne changent pas.

## Comment SendNonWorkersTo a ete tranche
`HOLUnitView` / `FHOLUnitView` ne sont pas lies au script : `SendNonWorkersTo` reste C++. Gather, Build et DropOff (deposit avec Shift) l'appellent TOUJOURS en C++, dans la portee joueur, puis passent au pont la liste des travailleurs DEJA filtree ; la regle script ordonne ce qui reste (acceptee meme sans travailleur, comme le C++). Donnees de jeu que le script ne lit pas, calculees en C++ et passees en argument : `bSource` + `SourceLocation` (Gather, UHOLSourceSubsystem), `bDeposit` (DropOff : `FHOLBuildingTypeRow::Accepts` + `TakesDeliveries` + proprietaire), `bSourceStanding` (SetRally). Ligne de journal "Drop-off order of slot..." gardee en C++, emise avant la demande au pont avec la meme condition (deposit et travailleurs non vides). Chainage `bChain` : portee ouverte dans chaque emetteur `*Player` ; `SendNonWorkersTo` tourne sous la portee C++ de la fonction.
Types a exposer pour finir (hors lot) : `FHOLUnitView` + `HOLUnitView::Find(UWorld&, id, FHOLUnitView&)` (ou un lecteur `GetUnitOwnerAndCarry(World, id, &Owner, &Carry)`) pour porter `SendNonWorkersTo` ; `FHOLBuildingTypeRow::Accepts` + `AHOLBuilding::TakesDeliveries/GetType` pour porter `bDeposit` ; `UHOLSourceSubsystem::GetSources()` (+ `Location`, `IsStanding`) pour porter `bSource` / `bSourceStanding`.

## Champs
- BUILDS_DEMANDES: 0 (a lancer par la session principale : `Tools\build.ps1 -LiveCoding=0 -Wait` dans le worktree Haldir ; C++ touche : HOLCommandScriptLibrary, HOLPlayerOrdersBoard, HOLPlayerOrders.cpp).
- TESTS_CPP_RETIRES: 0 (les tests C++ appelants restent : ils exercent maintenant la chaine pont -> script).
- TESTS_SCRIPT_AJOUTES: 9 : `Test_PlayerOrdersGatherEntersGatherQueue`, `Test_PlayerOrdersGatherRefusedOrEmpty`, `Test_PlayerOrdersDropOffWalksWhenNotADeposit`, `Test_PlayerOrdersPlaceEntersQueue`, `Test_PlayerOrdersPlaceDropsBuildersWhenTooMany`, `Test_PlayerOrdersBuildingRulesRefuseNullBuilding` ; accord planche / regle : `Test_PlayerOrdersBoardAgreesPlace`, `Test_PlayerOrdersBoardAgreesGatherAndDropOff`, `Test_PlayerOrdersBoardAgreesBuildingRefusals`.
- TEST_ROUGE: non vu (aucun build autorise). Valeurs a fausser : (1) `HOLPlayerOrders.as`, `Place` : `UnitIds.Num() <= MaxOrderedUnitsValue` -> `true` : `Test_PlayerOrdersPlaceDropsBuildersWhenTooMany` rouge ; (2) `Gather` : `bQueue && bSource` -> `bSource` ; (3) `HOLPlayerOrdersBoard.as` `AnswerPlace` : passer un tableau vide : `Test_PlayerOrdersBoardAgreesPlace` rouge. Puis remettre, revoir vert. Sinon, pour voir tomber la chaine C++ reelle : fausser `Answer()` de la planche (`bDone ? 0 : 1`) et lancer les tests C++ GatherTests / GarrisonOrderTests / AISlotTests (comme en P1).
- TESTS_TAUTOLOGIQUES: 0 (les 3 tests d'accord passent par la planche et la file detachee).
- LIGNES_CPP_SUPPRIMEES: 0 (repli garde ; jumeau a retirer apres la preuve reseau). LIGNES_MIGREES_PAR_LOT: ~215 l. de corps C++ (126-338) maintenant doublees par ~150 l. de regles script.
- EDITS_REFUSES: 0 (un incident : le remplacement des corps avait cree `}}`, corrige et amende dans le meme commit `09f7e41`).
- PROCESSUS_DETACHES: non lance (aucun processus demarre).

## Limites des tests (honnetes)
Sans monde, aucun `AHOLBuilding` : les tests script n'exercent que les refus (batiment nul, slot, 201) pour Build / QueueProduction / MarketTrade / SetRally / Garrison / UngarrisonAll et le chemin accepte seulement pour Gather, DropOff (non-depot), Place. Le chemin accepte des ordres a batiment est prouve par les tests C++ existants (HOLGarrisonOrderTests 924-949, HOLGatherTests, HOLAISlotTests, HOLStartingHeroTests 674) qui passent par le pont une fois compile. Relecture Trade/Rally : `FHOLScriptOrderRead` n'a pas Resource/Lots (rien ajoute, non teste en script). Supposition non verifiee : l'ordre en file d'attente (Shift) de Gather/Build passe par `IssueQueuedWorkOrder` ; ce que `ConsumeMoves` rend pour lui n'est pas teste.

## Risques de compilation (a lire au premier build)
- `UFUNCTION(BlueprintPure) TakesGarrisonOrder(const AHOLBuilding*, int32)` : `const AHOLBuilding*` en UFUNCTION statique, a confirmer ; `Building->GetOwnerPlayer()` const supposee (utilisee sur `AHOLBuilding*` non const ailleurs).
- Les 9 evenements natifs ont des parametres `FName`, `AHOLBuilding*`, `FVector` par valeur ; la surcharge script reprend les memes noms (`GatherAnswer`, `BuildAnswer`, `DropOffAnswer`, `PlaceAnswer`, `QueueProductionAnswer`, `MarketTradeAnswer`, `SetRallyAnswer`, `GarrisonAnswer`, `UngarrisonAllAnswer`).
- Script : `AHOLBuilding NoSite = nullptr;` passe a `IssueQueuedWorkPlayer` ; `Building.GetActorLocation()` sur un handle `AHOLBuilding`.
- `HOLCommandScriptLibrary.cpp` inclut maintenant `Commands/HOLPlayerOrders.h` (dependance biblio -> regles, sans cycle de lien).

## Preuves reseau a rejouer (apres build)
1. `Tools\Tests\quick_check.ps1 -Parts tests` : les 9 nouveaux `Test_PlayerOrders*` verts + C++ GatherTests, GarrisonOrderTests, AISlotTests, AIOrdersTests, StartingHeroTests inchanges.
2. `Tools\Scenarios\rpc_orders_p1p2_run.ps1` et `rpc_orders_p3p4_run.ps1` (RPC joueur : collecte, construction, depot, placement, garnison, ralliement, marche) ; `-Parts net` (netsig3).
3. `-Parts aiserver,netai` : l'IA passe par `HOLAIOrders.cpp` 88-267 (Gather, Place, Build, QueueProduction, SetRally, MarketTrade).
4. Journal serveur : la ligne "Drop-off order of slot" identique ; aucune erreur de script `GetCommands` ni de surcharge `*Answer`.
5. Rouge de chaine : fausser `Answer()` de la planche : au moins les tests C++ Gather/Garrison tombent.

## Reste
Retrait des 9 corps C++ de repli (apres la preuve reseau) ; `SendNonWorkersTo` et les 3 donnees de jeu ci-dessus a porter quand `HOLUnitView`, `FHOLBuildingTypeRow` et `UHOLSourceSubsystem` seront lies. `HOLPlayerOrders.h` : `MaxOrderedUnits` toujours doublee (C++ + `MaxOrderedUnitsValue` script) tant que le repli existe. `Placeholders.txt` : aucun provisoire ajoute ni retire.
