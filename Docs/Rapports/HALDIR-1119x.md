# HALDIR-1119x : lot P1b (le jeu appelle la regle du script), worktree HeroesOfLiberty-Haldir, branche `selection`

Impact joueur : nul si le pont est fidele (memes ordres, memes refus, memes valeurs de retour, chaine `bChain` gardee). Tout est NON COMPILE, aucun build, aucun jeu lance.

## En tete : appelants hors fil de jeu (point dur b)
Grep (Source hors Tests) des appelants de `HOLPlayerOrders::{IsAccepted,Move,Attack,AttackBuilding,AttackMove,Stop,ToggleDeploy}` : `AI/HOLAIOrders.cpp` (95, 149, 156, 166, 223), `Player/HOLPlayerController.cpp` (RPC serveur, 4300, 5513, 5885-5946), `Simulation/HOLVolcanoSubsystem.cpp:395` (via `EvacuateOwner`, appelee par `HOLAIOrders.cpp:103`). Aucun ne se trouve dans un processeur Mass : aucun fichier de `AI/` ne cite `MassProcessor/MassExecutor/AsyncTask/ParallelFor` ; `HOLPlayerOrders.h` n'est inclus que par ces 3 fichiers + tests ; `HOLAIOrders` est appele par `Script\AI\HOLAIGarrisonRules.as`, `HOLMarketSubsystem.h` (non lu en detail) et des tests. NON PROUVE a l'execution : par prudence `UHOLPlayerOrdersBoard::Find` et `FindAny` rendent nullptr hors fil de jeu (`IsInGameThread()`), donc un appel d'un autre fil garde le corps C++ (-1). Aucun branchement retire.

## Fait (3 commits, `git add` de chemins exacts)
- `7caf120` Pont (raison D839 : liaison a exposer) : `Source\HeroesOfLiberty\Commands\HOLPlayerOrdersBoard.h/.cpp` (neufs). `UCLASS(Abstract) UHOLPlayerOrdersBoard : UWorldSubsystem`, Game + PIE, `ShouldCreateSubsystem` comme `UHOLConstructionBoard` ; 7 `BlueprintNativeEvent` protegees (`IsAcceptedAnswer`, `MoveAnswer`, `AttackAnswer`, `AttackBuildingAnswer`, `AttackMoveAnswer`, `StopAnswer`, `ToggleDeployAnswer`), `_Implementation` = -1 ; statiques `Ask*(World, ...)` + `AskIsAcceptedAnywhere` (IsAccepted n'a pas d'argument monde : prend la planche du premier monde Game/PIE par `GEngine->GetWorldContexts()`, -1 sans planche donc les 6 tests sans monde de `HOLAIOrdersTests.cpp:159-164` gardent la regle C++). Reponse : 1 accepte, 0 refuse, -1 pas de planche.
- `d4e6588` Script : `Script\Commands\HOLPlayerOrdersBoard.as` (neuf, a cote de `HOLPlayerOrders.as`) : namespace `HOLPlayerOrdersBoard` (`Answer*`, prennent le sous-systeme en argument) + classe `UHOLPlayerOrdersBoardScript : UHOLPlayerOrdersBoard` dont les 7 surcharges `BlueprintOverride` appellent `HOLPlayerOrders::` avec `HOLCommandScript::GetCommands(this)`. Les emetteurs `Issue*Player` ouvrent la portee joueur en C++ : `bChain` est conserve. Test d'accord `Script\Commands\HOLPlayerOrdersBoard_Test.as` (5 tests).
- `21249ed` Branchement : `HOLPlayerOrders.cpp` : chacun des 7 corps demande d'abord au pont (`Answer >= 0` : `return Answer != 0`), sinon corps C++ d'aujourd'hui (REPLI GARDE). `IsAccepted` : pont `AskIsAcceptedAnywhere`, sinon `IsAcceptedCpp` (meme formule). `GetQueue` (P2 inclus) passe par `IsAccepted`. `.h` inchange, `SendNonWorkersTo` inchange, aucune ligne de journal touchee (ces 7 corps n'en ecrivent pas). Ids `uint32` -> `int32` par copie (`ToBridgeIds`) ; `TargetUnitId` cast en int32 puis recast par la liaison. Garde game-thread dans `Find` (voir en tete) incluse dans ce commit.

## Tests script ajoutes (5) dans `HOLPlayerOrdersBoard_Test.as`
`Test_PlayerOrdersBoardAgreesIsAccepted` (les 7 cas des tests de P1a/HOLAIOrdersTests : pont == regle directe, et 201 ids rend 0 et non -1), `Test_PlayerOrdersBoardAgreesMove` (verdict, nombre d'ordres, Kind, ids, cible, bQueue, bChain identiques ; chaine vraie), `Test_PlayerOrdersBoardAgreesRefusals` (slot -1, MaxSlots, 201 ids, sans sous-systeme, batiment nul : 0 et file vide ; 200 ids : 1), `Test_PlayerOrdersBoardAgreesStopHold` (Stop, Hold, ToggleDeploy), `Test_PlayerOrdersBoardAgreesAttackKinds` (Attack, AttackMove, cible, bChain, bQueue).
Limite honnete : le test compare `HOLPlayerOrdersBoard::Answer*` (ce que la classe script renvoie au C++) a `HOLPlayerOrders::*` sur deux files detachees. La repartition C++ `Ask*` -> classe script exige un monde : elle est a prouver par le rejeu reseau ci-dessous, pas par ce test.

Rouge a poser (non pose, non vu) : dans `HOLPlayerOrdersBoard.as`, `Answer()` : `bDone ? 1 : 0` -> `bDone ? 0 : 1` (tous les tests Agrees* rouges) ; plus fin : dans `AnswerStop`, passer `!bHold` : seul `Test_PlayerOrdersBoardAgreesStopHold` rouge (Kind Stop/Hold inverses). Poser une fois, remettre, revoir vert.

## Preuves a rejouer apres le build (session principale)
1. `Tools\build.ps1 -LiveCoding=0 -Wait` dans `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir` ; le journal de demarrage doit nommer la classe script `UHOLPlayerOrdersBoardScript` (sinon la planche est absente et tout repasse en C++ : aucune erreur, c'est le piege du repli).
2. `Tools\Tests\quick_check.ps1 -Parts tests` : les `Test_PlayerOrders*` (8 de P1a) + `Test_PlayerOrdersBoardAgrees*` (5 neufs, dont IsAccepted/Move/Refusals/StopHold/AttackKinds) verts ; tests C++ appelants verts (`HOLAIOrdersTests`, `HOLDeployButtonTests`: `HeroesOfLiberty.Siege.Deploy.PlayerButton` et `.PressWhileWalking`, `HOLDiplomacyBranchTests`, `HOLGatherTests`).
3. Preuve que le jeu passe par le pont : `-Parts move,combat` puis `-Parts net` (clic droit -> RPC `HOLPlayerController.cpp:5885-5946` -> pont) et `-Parts aiserver,netai` (IA : `HOLAIOrders.cpp`), et `Tools\Scenarios\rpc_orders_p1p2_run.ps1` (contenu non lu). Le test qui prouverait la route serait un test avec monde qui lit une file apres `HOLPlayerOrders::Move` ; non ecrit ici (C++, hors perimetre). Rouge de route : renommer une surcharge script (p. ex. `MoveAnswer`) doit faire retomber sur le repli C++ SANS rouge : c'est la faiblesse a garder en tete avant de retirer le repli.

## Champs
- BUILDS_DEMANDES: 0 ; la session principale compile (C++ pont + branchement + 3 `.as`, un build)
- TESTS_CPP_RETIRES: 0
- TESTS_SCRIPT_AJOUTES: 5 (liste ci-dessus)
- LIGNES_CPP_SUPPRIMEES: 0 (repli garde ; retrait en lot ulterieur) ; LIGNES_MIGREES_PAR_LOT: 0 supprimees ; ~80 l. de pont + ~55 l. de branchement ajoutees
- TYPES_A_EXPOSER: aucun
- TEST_ROUGE: non vu (pas de build autorise)
- TESTS_TAUTOLOGIQUES: 0 (chaque test lit la file reelle apres `Consume*`) ; EDITS_REFUSES: 0 ; PROCESSUS_DETACHES: aucun lance

## NON VERIFIE
- Rien n'est compile (C++ ni script) ; toutes les hypotheses de syntaxe de P1a restent valables.
- `UFUNCTION(BlueprintNativeEvent)` avec `const TArray<int32>&` et `FVector2D` par valeur, et surcharge script `const TArray<int>&` : non compile ; si le plugin refuse, passer les tableaux par valeur.
- Methode script `QueueOfWorld()` non-UFUNCTION appelee depuis les surcharges : suppose OK.
- Surcharge script d'un evenement natif `protected` : meme patron que `UHOLConstructionBoardScript` (compile ce matin).
- `FindAny` : `GEngine->GetWorldContexts()` ; en PIE avec plusieurs mondes la planche peut etre celle d'un autre monde (meme regle, sans effet).
- `Content\Maps\Crown.umap` non suivi laisse tel quel. Pas de merge de main (DLL non reconstruite). `HOLMarketSubsystem.h` : fil d'appel non lu.

## Anomalies
- Aucune bloquante. Piege du repli : une planche absente ne produit aucune erreur (voir preuve 1).
