# PLAN-TESTS-1119 : porter en script les tests C++ « de monde sans Mass » (D853), reprise 14, 2026-11-19

Lecture seule (aucun code touché). Source : `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Source\HeroesOfLiberty\Tests\` et `Script\`.
Mesuré par Grep / wc : lignes, nombre de `IMPLEMENT_SIMPLE_AUTOMATION_TEST`, occurrences de `RequestUnits|ServeBirthRequests`, `Executor::Run|FMassEntityManager`, `CallInitialize` (processeurs Mass), `GetSubsystem<>`. ESTIMÉ : tout verdict « en partie » et les lignes qui partent (je n'ai pas lu les cas un à un). Les `wc -l` dépassent de 5 à 10 % les chiffres de PLAN-ENVELOPPE (Building 1051 contre 959) : les miens sont les chiffres du code actuel.

## 1. Monde de test : C++ aujourd'hui, script aujourd'hui

Aucune aide commune en C++ : chaque fichier recopie le même motif (68 fichiers de test contiennent `CreateWorld`) :
`UWorld::CreateWorld(EWorldType::Game, false)` ; `GEngine->CreateNewWorldContext(EWorldType::Game)` ; `Context.SetCurrentWorld(World)` ; `World->InitializeActorsForPlay(FURL())` ; fin : `GEngine->DestroyWorldContext(World)` + `World->DestroyWorld(false)`.
- Exemples : `HOLBuildingRegistryTests.cpp:19-32` (MakeWorld / DestroyWorld) ; `HOLBuildingTests.cpp:60,133,294,393,507,539,610,694,836,955` (un monde par test, recopié) ; `HOLFarmBuildersTests.cpp:41-120` (struct `FBuildersWorld` : monde + `SpawnActor<AHOLGameState>` + `InitVictory(Config)` + sous-systèmes par `World->GetSubsystem<>` + processeurs Mass `NewObject` / `CallInitialize` + `RunStep` par `UE::Mass::Executor::Run`).
- Bâtiment : `World->SpawnActor<AHOLBuilding>()` puis `Building->Setup(Type, Owner, Owner[, State])` (`HOLBuildingRegistryTests.cpp:36-39`, `HOLFarmBuildersTests.cpp:114-117`) ; `SpawnActorDeferred` + `FinishSpawning` (`HOLBuildingTests.cpp:766`).
- Unité / héros : `UHOLUnitSubsystem::RequestUnits(FHOLUnitSpawnRequest)` + `ServeBirthRequests()` (`HOLFarmBuildersTests.cpp:127-133`) : naissance par Mass.
- Partie : `AHOLGameState::InitVictory(FHOLMatchConfig)` ; la config vient de `FHOLMatchConfig::MakeDefault()` puis état des slots.
- Pas : `HOLSimulationSubsystem::GetOnStepEnded().Broadcast` (sans Mass) ou `RunStep` avec processeurs Mass.

Script aujourd'hui :
- `HOLCommandScript::MakeDetachedForTest()` (`Source\HeroesOfLiberty\Commands\HOLCommandScriptLibrary.cpp:68`, `NewObject<UHOLCommandSubsystem>(GetTransientPackage())`) : file d'ordres SANS monde ; 8 usages dans `Script\Commands\HOLCommandLibrary_Test.as:26-142`. Seul monde-libre existant.
- `UHOLScreenTestLibrary` (`Source\HeroesOfLibertyUI\Tests\HOLScreenTestLibrary.h:22`, `.cpp:29-112`) : seul précédent d'un monde créé pour un test script : même motif CreateWorld, tenu par `TStrongObjectPtr<UWorld>` statique, un seul à la fois, détruit par `EndScreenTest()`, corps sous `#if WITH_DEV_AUTOMATION_TESTS` (hors éditeur : ne fait rien, rend nullptr). UFUNCTION `BlueprintCallable`, classe `UCLASS()` dans un module (UI), pas dans le jeu.
- Tables (`FindRow`), règles pures (`HOLSiegeCheck`, `HOLTalent`, `HOLDuel...` livres) : déjà en script, hors sujet.
- `AHOLBuilding`: 7 `UFUNCTION`; `AHOLGameState`: 8 `UFUNCTION` (dont des `NetMulticast`) : presque rien d'appelable (`Match\HOLGameState.h:428-751`).

## 2. Fichiers visés (22 ; lignes = wc, tests = IMPLEMENT, Naiss = RequestUnits+ServeBirth, Proc = processeurs Mass / `Executor::Run`)

| Fichier | Lignes | Tests | Besoin (mesuré) | VERDICT |
|---|---|---|---|---|
| TierCheat | 55 | 1 | monde, GameState, GetTier/SetTierOverride/GetTierOverride ; 1 cas réflexion CPF_Net ; Naiss 0, Proc 0, pas 0 | PORTABLE avec la bibliothèque (lot 1) ; cas CPF_Net via `IsPropertyReplicated` |
| BuildingRegistry | 116 | 1 | monde, SpawnBuilding, itérateur registre / TActorIterator, 2 mondes ; Naiss 0, Proc 0 | PORTABLE (lot 1), sauf « boucle qui détruit » (propriété de l'itérateur C++ : reste en C++ ou itérateur valeur) |
| DisconnectDisplay | 146 | 4 | GameState (DefeatSlot, InitVictory), `UHOLMatchStatsSubsystem` ; Naiss 0, Proc 0, pas 0 | PORTABLE (lot 2) |
| Disconnect | 1168 | 15 | GameState (35 usages : GiveSlotStock 30, AddPlayerState 20, KeepAbsentSlot 16, InitVictory 14, GetAbsentNotices 11...), sous-systèmes Hero / Production / Simulation, 5 commandes console `hol.Net.FakeAbsent` (l.928-1002), 2 cas CPF_Net (l.265, 866), `Setup` bâtiment 3 ; Naiss 0, Proc 0, pas 2 (peu) | PORTABLE (lots 2-3) ; cas console : liaison `RunConsoleCommand` ; ESTIMÉ ~95 % |
| AIBuildOrders | 227 | 3 | GameState, Construction / Production / Source, GetResources / SetResources, pas 5 ; Naiss 0, Proc 0 | PORTABLE (lots 2 et 4) |
| Building | 1051 | 10 | monde, ~20 méthodes de `AHOLBuilding` hors API script (ApplyPendingAppearance 19, GetComponents 16, SetConstructionState 15, SetConstructionWork 12, GetTotalWork 12, GetLinkedSource 11...), 1 `APlayerController`, SpawnActorDeferred ; Naiss 0, Proc 0, GameState 0 | EN PARTIE : cas sans composants (Foundation, Damage, Selection, Archery, Forum) ; FarmField / FarmClick / Walkable (GetComponents, MaillagesInstanciés) restent C++ ; ESTIMÉ ~55 % |
| UnitTierProduction | 221 | 1 | Command / Construction / Production / Source / Unit, pas 4 ; Naiss 0 (les unités naissent par la production : Mass ou pas, NON VÉRIFIÉ) | EN PARTIE / À VÉRIFIER : portable si la production finie ne passe pas par Mass |
| Victory | 1475 | 12 | GameState (GetForumState 55), Naiss 11, Proc 1, pas 45, 8 mondes | EN PARTIE : les cas règles de forum / défaite sans naissance (ESTIMÉ ~45 %) ; cas avec unités : C++ |
| StartingHero | 788 | 9 | monde, Construction / Site / Production, Naiss 5, Proc 0, Net 2 | EN PARTIE : cas sans naissance de héros (ESTIMÉ ~40 %) |
| HeroAbilityOrder | 102 | 1 | héros par Mass (Naiss 2) | EN PARTIE : 1 cas (ordre hors slot, file détachée), ~12 l. ; lot 5 (2 UFUNCTION de `UHOLCommandScriptLibrary`) |
| HeroTalentOrder | 134 | 2 | héros par Mass (Naiss 3) | EN PARTIE : DÉJÀ 4 tests script (commit fd31374, rouge non vu) ; la chaîne reste C++ |
| AIOrders | 478 | 8 | Proc 2, Naiss 2, Executor 4, pas 8 | EN PARTIE : cas d'ordres seuls (ESTIMÉ ~30 %) ; le reste via processeurs : C++ |
| FarmBuilders | 406 | 3 | Proc 3 (Orders / Gather / Builders), Naiss, `RunStep` Mass | MASS : reste C++ (D838) |
| ProductionPopulationEnd | 173 | 1 | Naiss 2 (Peasant), Production | MASS : reste C++ (1 test, naissance Mass) |
| DuelOrder | 192 | 2 | héros nés par Mass (Naiss 2) ; ClearCircle = règle inline C++ sans jumeau script | MASS : reste C++ (le livre de duels est déjà en script, GALADRIEL-1119t) |
| GarrisonOrder | 962 | 7 | Naiss 21, Proc 5, pas 34 | MASS : reste C++ |
| Gather | 731 | 6 | Proc 2, Executor 4, Naiss 2 | MASS : reste C++ |
| DeployButton | 313 | 2 | Naiss 6, Proc 6, Executor 8 | MASS : reste C++ |
| DiplomacyBranch | 655 | 5 | Naiss 9, Proc 5, Executor 8, pas 20 | MASS : reste C++ |
| DiplomacyAllies | 448 | 6 | Naiss 3, Proc 2, pas 13 | MASS : reste C++ |
| HeroReturn | 967 | 15 | Naiss 3, Proc 1, pas 89 | MASS : reste C++ |
| Regicide | 498 | 8 | Naiss 3, Proc 2, pas 20 | MASS : reste C++ |

Décompte : 5 PORTABLES (TierCheat, BuildingRegistry, DisconnectDisplay, Disconnect, AIBuildOrders = 1712 l.) ; 7 EN PARTIE (Building, UnitTierProduction, Victory, StartingHero, HeroAbilityOrder, HeroTalentOrder, AIOrders = 4249 l.) ; 10 MASS (FarmBuilders, ProductionPopulationEnd, DuelOrder, GarrisonOrder, Gather, DeployButton, DiplomacyBranch, DiplomacyAllies, HeroReturn, Regicide = 5345 l.). Total 22 fichiers = 11 306 l.
Note : le constat « le monde sans Mass fait naître par Mass » est vrai pour 10 fichiers sur 22 ; Disconnect (le plus gros portable) n'a aucune naissance.

## 3. Bibliothèque de liaison (C++, raison D839 : liaison à exposer)

Où : module de JEU `HeroesOfLiberty`, dossier `Source\HeroesOfLiberty\Tests\` (fichiers `HOLTestWorldLibrary.h/.cpp`). Pourquoi pas le module UI (où est `UHOLScreenTestLibrary`) : la bibliothèque appelle `AHOLBuilding::Setup`, `AHOLGameState`, les sous-systèmes de simulation : tous dans le module de jeu, que l'UI inclut déjà (sens de dépendance). Le module de tests n'existe pas (les tests C++ sont dans le module de jeu). SHIPPING : le motif existant suffit : classes `UCLASS` toujours compilées (UHT ne gère pas un `#if` autour d'un UCLASS), CORPS sous `#if WITH_DEV_AUTOMATION_TESTS` (rend nullptr / 0 / faux ailleurs), comme `HOLScreenTestLibrary.cpp:10-27,93`. Alternative plus propre : exclure le dossier `Tests\` dans `HeroesOfLiberty.Build.cs` si `Target.Configuration == UnrealTargetConfiguration.Shipping`, mais `GENERATED_BODY` et le script (qui lie au démarrage) veulent la classe : NON VÉRIFIÉ, donc rester sur le motif des corps vides.

Durée de vie du monde : un `UWorld` NE doit PAS être tenu en pointeur brut par le script (non enraciné : le GC peut le prendre entre deux images, et la destruction exige `DestroyWorldContext` + `DestroyWorld`). Motif recommandé : poignée opaque = `UCLASS() class UHOLTestWorld : public UObject` qui possède `TStrongObjectPtr<UWorld>` + le `FWorldContext`, créée par `UHOLTestWorldLibrary::CreateTestWorld()`, détruite par `DestroyTestWorld(Handle)` explicite en fin de test, plus `BeginDestroy` en filet (appelle la même destruction, idempotente) ; `CreateTestWorld` termine d'abord le monde précédent (comme `CreateScreenForTest`). Le script ne manie que la poignée ; toutes les fonctions prennent la poignée. Un test script étant exécuté dans une seule image (`FUnitTest`), pas de GC au milieu : le risque est surtout la fuite en cas d'échec ; le filet `BeginDestroy` + « un seul monde à la fois » la couvre. Les sous-systèmes du monde (`UWorldSubsystem`) naissent seuls avec `CreateWorld` + `InitializeActorsForPlay` (les tests C++ les lisent par `GetSubsystem` aussitôt). Le Mass existe donc dans ce monde même sans naissance (ESTIMÉ : pas de coût si aucun processeur ne tourne).

Lots (~150 l. C++ chacun ; signatures exactes, `UFUNCTION(ScriptCallable)` ou `BlueprintCallable`, toutes `static` dans `UHOLTestWorldLibrary` sauf mention) :

LOT 1 (~150 l.) : monde, bâtiments, état de partie minimal, itérateurs, réflexion.
- `static UHOLTestWorld* CreateTestWorld();` `static void DestroyTestWorld(UHOLTestWorld* TestWorld);`
- `static AHOLBuilding* SpawnBuilding(UHOLTestWorld* TestWorld, FName TypeName, int32 Owner, FVector Location, EHOLConstructionState State);`
- `static AHOLGameState* SpawnGameState(UHOLTestWorld* TestWorld, int32 NumHumans, int32 NumAI);` (`SpawnActor` + `FHOLMatchConfig::MakeDefault()` + états de slots + `InitVictory`, comme `HOLFarmBuildersTests.cpp:58-73`)
- `static void DestroyBuilding(AHOLBuilding* Building);`
- `static TArray<AHOLBuilding*> GetBuildingsByRegistry(UHOLTestWorld* TestWorld);` `static TArray<AHOLBuilding*> GetBuildingsByActors(UHOLTestWorld* TestWorld);`
- `static bool IsPropertyReplicated(UObject* Object, FName PropertyName);` (réflexion, `CPF_Net`)
- Sur `AHOLGameState` (specifier seul, `Match\HOLGameState.h:308-312`) : `int32 GetTier(int32 Slot) const; void SetTierOverride(int32 Slot, int32 Tier); int32 GetTierOverride(int32 Slot) const;`
Débloque : TierCheat (entier), BuildingRegistry (sauf boucle destructrice), amorce de Building.

LOT 2 (~150 l.) : état de partie (défaite, absences, pauses, réserves). Specifiers `ScriptCallable` sur `AHOLGameState` (ou enveloppe statique `UHOLGameStateTestLibrary`) : `bool IsSlotDefeated(int32) const; void DefeatSlot(int32 Slot, EHOLDefeatReason Reason); bool IsMatchOver() const; EHOLDefeatReason GetDefeatReason(int32) const; int32 GetLives(int32) const; EHOLForumState GetForumState(int32) const; void GiveSlotStock(int32, FHOLResources); void KeepAbsentSlot(int32); bool IsSlotAbsent(int32) const; void DefeatExpiredAbsences(); void RequestPause(int32); void RequestResume(int32); void LiftPauseOf(int32); bool IsMatchPaused() const; int32 GetPausesLeft(int32) const; int32 FindSlotOnLogin(FString) const;` + enveloppes de rendu (`TArray<FString> GetAbsenceBannerLines(...)`, `GetAbsentNotices`) ; `static APlayerState* SpawnPlayerState(UHOLTestWorld*, FString Name)` + `static void AddPlayerState(AHOLGameState*, APlayerState*)`. Les types de retour doivent être des `UENUM/USTRUCT` liés (`FHOLResources`, `EHOLDefeatReason`...) : à vérifier au compilateur (~1/3 des 150 l. peut être des USTRUCT à exposer).
Débloque : DisconnectDisplay (entier), Disconnect (cœur), Victory (règles), AIBuildOrders (partiel).

LOT 3 (~150 l.) : Disconnect fin, console et sous-systèmes d'accès.
- `static bool RunConsoleCommand(UHOLTestWorld* TestWorld, const FString& Command);` (`IConsoleManager::ProcessUserConsoleInput`, `HOLDisconnectTests.cpp:928`)
- `static UWorldSubsystem* GetWorldSubsystem(UHOLTestWorld* TestWorld, TSubclassOf<UWorldSubsystem> Class);` (UN accès générique : remplace les 8 `GetSubsystem<>`) + specifiers `ScriptCallable` sur les méthodes réellement lues de `UHOLProductionSubsystem`, `UHOLMatchStatsSubsystem`, `UHOLHeroSubsystem` (`RetireHolder`, `GetMutableProductionQueue` : Disconnect).
Débloque : Disconnect (fin), DisconnectDisplay (rapport).

LOT 4 (~150 l.) : pas, production, ressources.
- `static void AdvanceSteps(UHOLTestWorld* TestWorld, int32 NumSteps);` (diffuse `GetOnStepEnded()` sans processeurs Mass)
- `static void SetResources(...)`/`GetResources(...)` sur le GameState ; specifiers sur `UHOLConstructionSubsystem`, `UHOLSourceSubsystem`, `UHOLSiteSubsystem`, `UHOLProductionSubsystem` (files / complétion).
Débloque : AIBuildOrders, UnitTierProduction (si pas de Mass dans la complétion), StartingHero (cas sans naissance), Victory (cas de pas).

LOT 5 (~100 l.) : ordres sans monde (`UHOLCommandScriptLibrary`, même patron que `IssueRage` / `ConsumeRages`) : `static void IssueHeroAbility(UHOLCommandSubsystem*, int32 PlayerIndex, EHOLHeroAbility, const FVector&); static TArray<FHOLScriptOrderRead> ConsumeHeroAbilities(UHOLCommandSubsystem*);`. Débloque : HeroAbilityOrder (1 cas).
HORS bibliothèque (reste C++ : D838) : naissance d'unité / héros (`RequestUnits`, `ServeBirthRequests`), `RunStep` avec processeurs Mass, `UMassEntitySubsystem`.

## 4. Ordre de portage (un fichier par agent de 15 min ; du moins risqué au plus risqué)

Prérequis : lot 1 construit et vu, puis dans l'ordre. Un test C++ ne part qu'après équivalent script rouge puis vert.
1. TierCheat (55 l.) : lot 1.  2. BuildingRegistry (116 - cas boucle) : lot 1.  3. DisconnectDisplay (146) : lot 2.
4. HeroAbilityOrder, 1 cas (lot 5).  5. Building, cas sans composants (lot 1 + quelques UFUNCTION de `AHOLBuilding`).
6. Disconnect en 3 agents (1168 l. : cas absences, pauses, console ; lots 2-3).  7. AIBuildOrders (lots 2, 4).
8. Victory (cas règles, 2 agents ; lot 2 + 4).  9. UnitTierProduction (vérifier d'abord si la naissance est Mass ; lot 4).
10. StartingHero (cas sans naissance ; lot 4).  11. AIOrders (cas d'ordres seuls) : le plus incertain, en dernier.
Le plus risqué : Victory / StartingHero / AIOrders (mélange de cas Mass et non Mass dans les mêmes tests : il faudra couper un test C++ en deux, ou ne retirer que les `TestTrue` couverts).

Totaux (ESTIMÉS) : 5 portables 1712 l. dont ~85 l. restent (boucle destructrice du registre, détails) -> ~1630 l. partent ; 7 en partie 4249 l. -> ~1900 l. partent (Building ~600, Victory ~650, StartingHero ~300, AIOrders ~150, UnitTierProduction ~150 si pas de Mass, HeroAbility ~15, HeroTalent 0 de plus). TOTAL qui peut réellement partir : ~3 500 l. de test C++ sur 11 306 (~31 %). RESTE C++ par D838 : 5345 (10 fichiers Mass) + ~2 350 (moitiés Mass des 7 en partie) + ~85 = ~7 800 l. Le chiffre de PLAN-ENVELOPPE (~7 500 + ~2 400 portables) est donc trop optimiste d'un facteur 3 à cause des naissances Mass.
Coût C++ de liaison : lots 1-5 ~650 l. de C++ neuf (~120 l. de specifiers sur GameState / sous-systèmes inclus), 2 à 3 builds groupés (lot 1 seul, lots 2-3, lots 4-5).

## 5. Questions pour l'utilisateur (NON VÉRIFIÉ)

1. Les 10 fichiers « monde sans Mass » qui font naître leurs unités par `RequestUnits` + `ServeBirthRequests` (5 345 l. : GarrisonOrder, Gather, DeployButton, Diplomacy x2, HeroReturn, Regicide, FarmBuilders, DuelOrder, PPopulationEnd) : les compter dans D838 (restent C++, ce plan) ou les porter aussi ?
   - Option A (recommandée) : D838 les garde en C++. Conséquence : ~7 800 l. de test C++ restent, le script retire ~3 500 l., coût liaison ~650 l. de C++.
   - Option B : lot 6 de bibliothèque : `SpawnUnit(UHOLTestWorld*, FName Type, int32 Owner, FVector Loc) -> int32` + `RunMassStep(UHOLTestWorld*)` (processeurs Mass pilotés depuis le script). Conséquence : +~400 l. de C++ de liaison à risque (processeurs Mass, ordre du pipeline `HOLFarmBuildersTests.cpp:94-109`, entité manager), 3 à 5 builds de plus, ~5 000 l. de plus peuvent partir mais le test script ne prouve plus ce que prouve le C++ (même pipeline, autre pilote) et le temps de test script monte.
