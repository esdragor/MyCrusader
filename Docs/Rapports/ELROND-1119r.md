# ELROND-1119r : lot 1 de la bibliotheque de monde de test + portage TierCheat / BuildingRegistry (2026-11-19)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, branche test-world. Aucun build, aucun quick_check, aucun editeur (interdits). Aucun test C++ supprime. Jamais de push.

## Commits (anglais, sans Co-Authored-By)
- 8531b91 : bibliotheque lot 1 (`Source\HeroesOfLiberty\Tests\HOLTestWorldLibrary.h/.cpp`) + 3 UFUNCTION sur `AHOLGameState`
- b7c1963 : include `UObject/Package.h` (GetTransientPackage)
- 8f1da5c : `Script\Tests\HOLTierCheat_Test.as`
- 0e0cb1a : `Script\Tests\HOLBuildingRegistry_Test.as`

## Nommage
La classe-poignee s'appelle `UHOLTestWorldHandle` (script : `UHOLTestWorldHandle`), pour ne pas heurter le namespace script `HOLTestWorld::` (de `UHOLTestWorldLibrary`).

## Signatures livrees (C++ ; script : `HOLTestWorld::Fonction(...)`)
Toutes `UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")`, `static`, dans `UCLASS() class HEROESOFLIBERTY_API UHOLTestWorldLibrary : UBlueprintFunctionLibrary` (corps sous `#if WITH_DEV_AUTOMATION_TESTS`, nullptr / vide / faux sinon ; `IsPropertyReplicated` et les structures sans corps conditionnel).
- `static UHOLTestWorldHandle* CreateTestWorld();`
- `static void DestroyTestWorld(UHOLTestWorldHandle* TestWorld);` (idempotent, accepte nullptr)
- `static AHOLBuilding* SpawnBuilding(UHOLTestWorldHandle* TestWorld, FName TypeName, int32 Owner, FVector Location, EHOLConstructionState State);` (`SpawnActor<AHOLBuilding>(Location, ZeroRotator)` puis `Setup(TypeName, Owner, Owner, State)`)
- `static void DestroyBuilding(AHOLBuilding* Building);`
- `static AHOLGameState* SpawnGameState(UHOLTestWorldHandle* TestWorld, int32 NumHumans, int32 NumAI);`
- `static TArray<AHOLBuilding*> GetBuildingsByRegistry(UHOLTestWorldHandle* TestWorld);` (`FHOLBuildingIterator`)
- `static TArray<AHOLBuilding*> GetBuildingsByActors(UHOLTestWorldHandle* TestWorld);` (`TActorIterator<AHOLBuilding>`)
- `static bool IsPropertyReplicated(UObject* Object, FName PropertyName);` (`FindPropertyByName` + `CPF_Net`)
- Poignee : `UCLASS() class UHOLTestWorldHandle : UObject` ; `UPROPERTY() TObjectPtr<UWorld> World` ; `UWorld* GetTestWorld() const` (C++ seulement) ; `void DestroyOwnedWorld()` (idempotent : `GEngine->DestroyWorldContext(World)` puis `World->DestroyWorld(false)`) ; `BeginDestroy()` rappelle `DestroyOwnedWorld()` (filet).
- `AHOLGameState` (`Match\HOLGameState.h:308-315`) : `UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Tier")` sur `int32 GetTier(int32 Slot) const; void SetTierOverride(int32 Slot, int32 Tier); int32 GetTierOverride(int32 Slot) const;`

Motif du monde recopie de `HOLBuildingRegistryTests.cpp:19-32` : `UWorld::CreateWorld(EWorldType::Game, false)`, `GEngine->CreateNewWorldContext(EWorldType::Game)`, `Context.SetCurrentWorld(World)`, `World->InitializeActorsForPlay(FURL())` ; destruction `DestroyWorldContext` + `DestroyWorld(false)`.

## Ecarts par rapport au plan (a valider, aucun design tranche)
1. `BlueprintCallable` et non `ScriptCallable` sur `AHOLGameState` : aucun `ScriptCallable` n'existe dans `Source\` (jamais vu compiler ici) ; `BlueprintCallable` est le specifier de toutes les liaisons existantes.
2. PLUSIEURS mondes vivants a la fois (le plan disait : `CreateTestWorld` termine le precedent) : le test du registre a besoin de deux mondes (World + Other). Chaque poignee est tenue par un `TStrongObjectPtr` statique (tableau) jusqu'a son `DestroyTestWorld`.
3. `SpawnGameState(h, 0, 0)` = acteur nu, sans `InitVictory` (comme le test C++ TierCheat, qui n'appelle pas `InitVictory`) ; des que `NumHumans + NumAI > 0` : `FHOLMatchConfig::MakeDefault()`, `Slots[i].State` = Human pour `i < NumHumans`, AI pour les `NumAI` suivants, Closed ensuite, puis `InitVictory` (HOLFarmBuildersTests.cpp:58-73).
4. `DestroyBuilding` ne prend que le batiment (signature du plan), `DestroyTestWorld` ne le sait pas : un batiment detruit est pointeur perime cote script jusqu'au GC (comme en C++).

## Table test C++ -> test script
| Test C++ | Test script | Cas portes | Reste en C++ |
|---|---|---|---|
| `HeroesOfLiberty.Match.TierCheat` (HOLTierCheatTests.cpp, 1 test, 14 assertions) | `Test_TierCheat` (`Script\Tests\HOLTierCheat_Test.as`) | tous : tier 0 sans commande, Ram refusee (`HOLSiegeCheck::Check`, `EHOLSiegeBlock::TierTooLow`) puis `Available` a tier 3, autres slots intacts, override -1, 9 plafonne a 3, 1 relu, 0 est un override, -1 retablit, slot 99 sans effet, `TierOverrides` replique (`IsPropertyReplicated`) ; +1 assertion de controle neuf (un nom inexistant n'est pas replique : evite un `IsPropertyReplicated` qui dit oui a tout) | rien : le test C++ peut partir apres rouge puis vert |
| `HeroesOfLiberty.Buildings.Registry` (HOLBuildingRegistryTests.cpp, 1 test) | `Test_BuildingRegistry` (`Script\Tests\HOLBuildingRegistry_Test.as`) | deux mondes ; aucun batiment : 0 ; 4 batiments ; registre == acteurs ; 3 du monde ; l'autre monde seul ; apres un `Destroy` : registre == acteurs et == {A, C} ; un nouveau vient en dernier == {A, C, D} et acteurs d'accord ; fin du cas C++ (aucun restant, autre monde intact) rejouee SANS la boucle (A, C, D detruits un a un, puis 0 restant) | la boucle qui detruit en iterant (`for (FHOLBuildingIterator It...) { Seen.Add(*It); It->Destroy(); }`, assertion « sees all three ») : propriete de l'iterateur C++, le script ne recoit qu'un instantane (`TArray`) ; le test C++ reste donc ENTIER (il couvre aussi tout le reste, retrait impossible sans perdre cette boucle) |

Chaque test script detruit ses mondes en fin de fonction (les assertions `T.Assert*` ne sortent pas de la fonction ; une sortie anticipee de `Run` passe quand meme par le `DestroyTestWorld` de l'appelant) ; les aides sont dans `namespace TierCheatTest` / `namespace BuildingRegistryTest`.

## NON VERIFIE (suspects de compilation, rien compile)
- UHT : `TArray<AHOLBuilding*>` en retour de `UFUNCTION` statique ; `FVector` / `EHOLConstructionState` par valeur ; `TObjectPtr<UWorld>` en `UPROPERTY` avec `UWorld` declare en avant ; `BeginDestroy` surcharge dans une classe a corps conditionnel.
- Liaison script : le nom exact des types (`UHOLTestWorldHandle`, `AHOLBuilding`, `AHOLGameState` avec leur prefixe, comme `UHOLNewGameScreen` dans HOLNewGameScreen_Test.as) ; `FVector(0.0, 0.0, 0.0)` ; comparaison `AHOLBuilding != AHOLBuilding` ; parametre `const TArray<AHOLBuilding>&` ; `n"MainCentre"` ; `EHOLConstructionState::Finished` (existe en C++, `UENUM(BlueprintType)`).
- `FUnitTest::AssertTrue` ne rend peut-etre rien : je n'utilise jamais sa valeur de retour.
- Registre : un `AHOLBuilding` spawne par `SpawnActor` + `Setup` dans un monde vide a la meme place que le test C++ (3 au meme point d'origine : collision par defaut AlwaysSpawn) ; le registre libere B a `Destroy()` comme dans le test C++.
- `SpawnGameState(1, 1)` avec `InitVictory` : non exerce ici (aucun test du lot 1 ne s'en sert) ; couvert par le motif de HOLFarmBuildersTests.cpp.
- Libération au GC : la poignee est tenue par un `TStrongObjectPtr` statique jusqu'a `DestroyTestWorld` ; un test qui planterait avant ce point laisse son monde vivant jusqu'a la fin du processus (filet `BeginDestroy` seulement a l'arret du moteur, ignore si `IsEngineExitRequested`).

## Ce que le lot 2 devra ajouter (PLAN-TESTS-1119 section 3, lot 2)
- Sur `AHOLGameState` (specifiers `BlueprintCallable`, ou enveloppe statique) : `IsSlotDefeated`, `DefeatSlot(int32, EHOLDefeatReason)`, `IsMatchOver`, `GetDefeatReason`, `GetLives`, `GetForumState` (renvoie `const FHOLForumState&` : demande un `USTRUCT` expose), `GiveSlotStock(int32, FHOLResources)`, `KeepAbsentSlot`, `IsSlotAbsent`, `DefeatExpiredAbsences`, `RequestPause`, `RequestResume`, `LiftPauseOf`, `IsMatchPaused`, `GetPausesLeft`, `FindSlotOnLogin`, `GetAbsentNotices` / lignes de banniere ; `static APlayerState* SpawnPlayerState(UHOLTestWorldHandle*, FString)` et `static void AddPlayerState(AHOLGameState*, APlayerState*)`.
- `EHOLDefeatReason` est `UENUM()` sans `BlueprintType` (HOLGameState.h:26) : a verifier / passer en `BlueprintType` ; `FHOLAbsentNotice` est `USTRUCT()` sans `BlueprintType` (:48) ; `FHOLResources`, `FHOLForumState` : meme question. Un `InitVictory` appele depuis le script (`SpawnGameState` le fait deja des que NumHumans + NumAI > 0) n'est PAS encore exerce par un test script.
- Lot 3 (console `RunConsoleCommand`, `GetWorldSubsystem`) et lot 4 (`AdvanceSteps`, ressources) restent a ecrire ; le lot 1 n'expose aucun sous-systeme du monde.
