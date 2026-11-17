# SAURON-1117f : port script de HOLProductionPopulationEndTests.cpp et HOLUnitTierProductionTests.cpp

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` (branche `visual`, 62df53d). Binaire a jour au depart (Get-StaleBinaryProblem vide).
Aucun test C++ supprime. Aucun build.

## Constat

Les deux tests C++ montent un monde complet (`UWorld::CreateWorld`, `AHOLGameState`, `SpawnAIPlayerStates`, sous-systemes Unit / Command /
Production / Construction / Source, `AHOLBuilding::Setup`) et mesurent la vraie chaine : file d'ordres, file de production du batiment, stock et
population du `AHOLPlayerState`. Rien de cela n'est expose au script : `UHOLCommandSubsystem` (0 UFUNCTION), `UHOLUnitSubsystem` (0),
`AHOLGameState::SetTierOverride/GetTier` (non UFUNCTION), `AHOLBuilding::GetProductionQueue` (const ref sur `TArray<FHOLProductionItem>`, struct non
exposee), `FHOLProductionCommand` (struct C++ simple), `AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates` (statique C++). Aucun test script ne cree
de monde. Le monde de test n'est donc pas portable sans bibliotheque.

## Registre : nom C++ -> test script / a exposer

| Test C++ | Part portee en script | Reste en C++ / a exposer |
|---|---|---|
| `HeroesOfLiberty.Production.UnitTier` (FHOLUnitTierProductionTest) | DECISION des ordres par palier : `Script\Tests\HOLUnitTierProduction_Test.as` (`Test_UnitTierProduction_LotAtTier0And1`, `_SecondLotAfterTierLift`, `_MangonelAtTier2`) : mêmes comptes de files `1/1/0/0/0`, `2/2/1/1/1`, `3/3/1/1/1`, atelier a 2 au palier 2, via `HOLUnitTier::Check` (l'appel du serveur). | Files reelles de `AHOLBuilding`, stock paye (sommes `Cost`), refus par la file d'ordres (meme pour un client qui demande). Restent testes en C++. |
| `HeroesOfLiberty.Production.PopulationCheckedAtEnd` (FHOLProductionPopulationEndTest) | Deja couvert en script (ELROND-1117d, `HOLProduction_Test.as`) : `GetProgress` (1.0 plafonne), `IsWaitingForRoom` (100 % attend une place), `GetPopulationCap`. | Tout le reste : naissances reelles `RequestUnits/ServeBirthRequests`, `MarkUnitDying`, population du joueur, file du forum. **Simulation Mass (D838) : le test reste en C++**, aucune exposition demandee. |

## Bibliotheque minimale a exposer (si on veut un jour le port complet du test Tier)

Une `UHOLProductionFixtureLibrary` (C++, UFUNCTION statiques a arguments plats, un lot C++ de la seance), le monde etant cree par le fixture :

1. `UObject* MakeProductionTestWorld(int32 Slots, EHOLFaction)` : cree le monde, `InitVictory`, `SpawnAIPlayerStates`, `OnWorldBeginPlay` des 3 sous-systemes, renvoie un handle (ou un UObject fixture qui se detruit avec la reference).
2. `AHOLBuilding* PutBuilding(World, FName Type, int32 Slot, FVector)` (= `Setup` fini).
3. `void SetResources(World, Slot, F, W, S, G)` et `int32 GetResourcesTotal(World, Slot)`.
4. `void SetTierOverride(World, Slot, Tier)` et `int32 GetTier(World, Slot)`.
5. `void IssueProductionOrder(World, Slot, AHOLBuilding*, FName Type)`.
6. `int32 GetProductionQueueNum(AHOLBuilding*)` (et `IsStarted/GetProgress` du premier si on veut le test Population).
7. `void RunSimulationStep(World, int64 Step)`.
8. `int32 UnitCost(World, FName Type)` (somme de la colonne Cost).

Pour le test Population, en plus (Mass, D838, non recommande) : `RequestUnits`+`ServeBirthRequests`, `MarkUnitDying` d'un id de `GetUnitIdsOfType`, `GetPopulationUsed/Cap`, et `HOLCommandGrid::IsPopulationBlocked(Used, Cap, Places)` (fonction libre plate : a exposer seule si on veut son test en script, 3 lignes).

## Tests script ajoutes et preuve

3 tests dans `Script\Tests\HOLUnitTierProduction_Test.as`. Rouge vu : valeur faussee `1/1/1/0/0` (au lieu de `1/1/0/0/0`) au palier 0, puis corrigee. Les comptes quick_check sont dans le journal AngelscriptInProgress.md.
