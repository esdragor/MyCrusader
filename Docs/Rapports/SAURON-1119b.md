# SAURON-1119b : lot M1, exposition de UHOLCommandSubsystem au script (D851 etape 5)

Impact joueur : nul (personne n'appelle la bibliotheque). Commit `ce8b4e2` sur `envelope` (worktree HeroesOfLiberty-Sauron), NON COMPILE, NON TESTE.
Aucun fichier C++ existant modifie. BUILDS_DEMANDES: 1 (le lot C++ de la seance, a grouper avec C1). TESTS_CPP_RETIRES: 0.
TESTS_SCRIPT_AJOUTES: 9. LIGNES_CPP_SUPPRIMEES: 0. TYPES_A_EXPOSER: FHOLScriptOrderRead (fait, USTRUCT lecture), UHOLCommandScriptLibrary (fait).

## Fichiers (neufs)
- `Source\HeroesOfLiberty\Commands\HOLCommandScriptLibrary.h` / `.cpp` (~170 + ~330 l.)
- `Script\Commands\HOLCommandLibrary_Test.as` (9 tests)

## Conception
- Tous les emetteurs et lecteurs prennent le sous-systeme en 1er parametre : `GetCommands(WorldContext)` en jeu, `MakeDetachedForTest()` en test.
  Raison : aucun test script de ce depot n'a de monde, et `UHOLCommandSubsystem::DoesSupportWorldType` n'admet que Game/PIE. Le sous-systeme detache
  (`NewObject<UHOLCommandSubsystem>(GetTransientPackage())`) n'a pas de monde : `IssueMoveOrder` ne projette rien sur le navmesh (cible telle que donnee),
  `IsRefused` ne refuse rien (pas de game state).
- Les ids d'unites sont `TArray<int32>` cote script (conversion en `uint32` dans le .cpp).
- Lecture : une seule USTRUCT `FHOLScriptOrderRead` (Kind FName, PlayerIndex, UnitIds, Target, TargetUnitId, SourceIndex, bQueue, bFromTask, bHasBuilding, TypeName, Index).
  `EHOLOrderKind` n'est pas un UENUM : le Kind est renvoye comme FName (Move, Attack, AttackMove, Stop, Hold, AttackBuilding, DropOff, Gather, Build, ToggleDeploy, Other).

## Signatures (UHOLCommandScriptLibrary, toutes statiques ; `C` = UHOLCommandSubsystem*, `U` = TArray<int32>)
Emetteurs, file des mouvements : `IssueMove(C, U, FVector Target, int Player, bool bQueue=false, bool bFromTask=false)`, `IssueAttack(C, U, int TargetUnitId, int Player, bQueue)`,
`IssueAttackMove(C, U, Target, Player, bQueue)`, `IssueStop(C, U, Player)`, `IssueHold(C, U, Player)`, `IssueAttackBuilding(C, U, AHOLBuilding, Player, bQueue)`,
`IssueDropOffMove(C, U, AHOLBuilding, FVector Point, Player, bQueue)`, `IssueQueuedWork(C, U, int SourceIndex, AHOLBuilding, FVector Location, Player)`.
Recolte / chantier : `IssueGather(C, U, int SourceIndex, Player, bQueue)`, `IssueDropOff(C, U, AHOLBuilding, Player, bQueue)`, `IssueBuild(C, U, AHOLBuilding, int SourceIndex, Player, bQueue)`.
Production / construction : `IssueProductionQueue(C, Player, AHOLBuilding, FName Type, bQueue)`, `IssueProductionCancel(C, Player, AHOLBuilding, int Index)`,
`IssueRally(C, Player, AHOLBuilding, FVector Location, int SourceIndex, AHOLBuilding RallySite)`, `IssuePlace(C, Player, FName Type, FVector Location, U, bQueue)`,
`IssueConstructionCancel(C, Player, AHOLBuilding)`. Garnison / rage : `IssueGarrison(C, Player, U, AHOLBuilding)`, `IssueUngarrisonAll(C, Player, AHOLBuilding)`, `IssueRage(C, Player, U)`.
Lecteurs (prennent ET vident la file, comme la simulation) : `GetNumPendingMoveOrders(C)`, `ConsumeMoves/Gathers/Builds/Productions/Constructions/Garrisons/Rages(C) -> TArray<FHOLScriptOrderRead>`.

## Tests ecrits (Script\Commands\HOLCommandLibrary_Test.as)
Test_CommandLibraryMoveRoundTrip, Test_CommandLibraryMoveShiftAndTask, Test_CommandLibraryCombatKinds, Test_CommandLibraryGather,
Test_CommandLibraryBuildNeedsASiteOrASource, Test_CommandLibraryProductionAndRally, Test_CommandLibraryConstruction,
Test_CommandLibraryRageAndGarrisonRefusals, Test_CommandLibraryNoSubsystem. TESTS_TAUTOLOGIQUES: 0 (chaque test passe par la vraie file C++ : Issue puis Consume,
avec les regles de refus reelles : liste d'unites vide, source -1, slot hors limites, batiment nul).

## Ce qui manque (dit precisement)
- Les ordres qui nomment un batiment (garnison, drop-off, attaque de batiment, site d'aide, rally site) ne peuvent pas etre relus pleinement : un test script n'a pas de monde, donc
  pas d'`AHOLBuilding`. Testes seulement pour le refus (batiment nul : rien en file). Aide C++ minimale a prevoir si besoin : une fonction de test qui fait apparaitre un
  AHOLBuilding sans monde de jeu est impossible ; le plus simple est un test en monde (lots P1/P2, ou scenario) ou `SpawnActor` dans un monde editeur de test.
- `IssueCombatWalk`, hero (return / ability / talent / duel / diplomacy) et `NotifyNavigationChanged` ne sont pas exposes (non demandes par les lots cites ; a ajouter dans le lot qui les appelle).
- Le sous-systeme detache n'a pas de GC root : le test le garde dans une variable locale (suffisant dans une seule fonction de test).

## Suspects de compilation
1. `GetTransientPackage()` : include `UObject/UObjectGlobals.h` (arrive normalement par CoreMinimal / Engine) ; `UHOLCommandSubsystem` doit etre instanciable par `NewObject` (UWorldSubsystem l'est ; son `Initialize` n'est pas appele : aucun etat requis par les Issue*).
2. `IssueMoveOrder` appelle `UHOLHealProcessor::CancelOrders` (statique, hors monde) : verifier qu'il ne suppose pas un monde (crash a l'execution du test sinon).
3. `EHOLOrderKind::ToggleDeploy` : nom vu dans `IssueCombatOrder` (.cpp), non revu dans l'enum du .h ; si absent, retirer la ligne du switch `KindName`.
4. `FHOLScriptOrderRead` : `TArray<int32>` dans un USTRUCT + UPROPERTY ecrites sur une ligne (macro `UPROPERTY(BlueprintReadWrite) T Name;`) : accepte par UHT.
5. Script : `TArray<int>()` comme argument, `UHOLCommandSubsystem None = nullptr;` (handle) ; si le plugin refuse, creer la variable avec `UHOLCommandSubsystem None;`. Le script suppose que `UHOLCommandSubsystem` est lie (comme `UHOLHeroSubsystem` l'est dans HOLHeroReturn_Test.as).
6. `Orders[i].Kind == n"Move"` : comparaison FName script, supposee valide.

## Rouge a poser une fois
Dans `Test_CommandLibraryMoveRoundTrip`, remplacer `T.AssertEquals(1200, int(Orders[0].Target.X)` par `1201` (ou `AssertEquals(7, Orders[0].UnitIds[0]` par `8`) : attendu 1 test rouge sur 9 de ce fichier,
avec le message "target X as given (no navmesh)" au journal ; remettre la valeur puis revoir vert. Poser le bouchon dans le worktree Sauron, pas dans la copie principale.

## Commandes de validation (session principale ; un build tourne peut-etre deja dans ce worktree)
1. `<worktree>\Tools\build.ps1 -LiveCoding=0 -Wait` (UN build avec le lot C1 de SAURON-1119a).
2. `<worktree>\Tools\...\quick_check -Parts tests` (partie qui lance les tests script ; les 9 `Test_CommandLibrary*` doivent apparaitre ; total quick_check = ancien + 9).
3. Si le script ne compile pas : le journal de l'editeur donne la ligne ; causes probables = suspects 3 a 6.

## Ce que P1 / P2 / C3 pourront appeler
- P1 (`HOLPlayerOrders::Move/Attack/Stop/Hold/AttackMove`), P2 (Gather, DropOff, Build, IssueQueuedWork) et C3 (Do<X> serveur dans `Script\Net\HOLRpcPlayer.as`) :
  `UHOLCommandSubsystem Commands = UHOLCommandScriptLibrary::GetCommands(this);` puis `UHOLCommandScriptLibrary::IssueMove(Commands, Ids, Target, Slot, bShift)` etc.
- NON couvert par M1 (portee de FPlayerOrderScope) : `FPlayerOrderScope` / `FRallyOrderScope` (RAII C++) ne sont pas exposes ; les ordres emis d'ici ne dessinent pas leur chaine (bChain) comme un ordre du joueur
  tant qu'un lot suivant n'ajoute pas un drapeau (ex. `SetPlayerOrders(C, bool)`). A trancher en P1 : sinon la chaine des ordres du joueur est perdue.
- Aucune question de design posee.

PROCESSUS_DETACHES: 0 (aucun lance). DOUTES: la portee de FPlayerOrderScope (ci-dessus).
