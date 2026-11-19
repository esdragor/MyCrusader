// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Match/HOLResources.h"
#include "Subsystems/WorldSubsystem.h"
#include "UObject/Object.h"
#include "HOLTestWorldLibrary.generated.h"

class AHOLBuilding;
class AHOLGameState;
class AHOLPlayerState;
class UWorld;

/**
 * Handle of the world UHOLTestWorldLibrary::CreateTestWorld makes (D853, lot 1 of PLAN-TESTS-1119): the script keeps this object, never the UWorld
 * (a UWorld is not rooted: the GC could take it between two frames, and its end needs DestroyWorldContext + DestroyWorld). The handle owns the
 * world (UPROPERTY: kept alive by it) and is itself kept alive by the library until DestroyTestWorld. Named Handle because the script namespace
 * of UHOLTestWorldLibrary is HOLTestWorld (a class UHOLTestWorld would take the same script name).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLTestWorldHandle : public UObject
{
	GENERATED_BODY()

public:
	/** The world, nullptr once destroyed (or without WITH_DEV_AUTOMATION_TESTS). C++ side only. */
	UWorld* GetTestWorld() const { return World; }

	/** Ends the world as the C++ tests do (GEngine->DestroyWorldContext, then UWorld::DestroyWorld). Idempotent. */
	void DestroyOwnedWorld();

	virtual void BeginDestroy() override;

	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
};

/**
 * World harness of the tests for Angelscript (namespace HOLTestWorld there, D839: a binding exposed to the script, a test harness; D853: the C++
 * tests of a world without Mass are ported to script). Same world as the C++ tests (UWorld::CreateWorld(EWorldType::Game, false), its context,
 * InitializeActorsForPlay), so the world subsystems exist as in them. The bodies exist only where WITH_DEV_AUTOMATION_TESTS (the editor
 * target); elsewhere every function does nothing and returns nullptr / false / empty. Several worlds can live at once (the registry test needs
 * two); each is ended by its own DestroyTestWorld.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLTestWorldLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** A new game world with its context, initialized for play. nullptr, with a line in the log, if a step fails. End it with DestroyTestWorld. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static UHOLTestWorldHandle* CreateTestWorld();

	/** Ends the world of the handle (context, then world). Safe to call twice or with nullptr. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static void DestroyTestWorld(UHOLTestWorldHandle* TestWorld);

	/** Spawns an AHOLBuilding at Location and sets it up (AHOLBuilding::Setup(TypeName, Owner, Owner, State): the colour is the owner's). nullptr if it fails. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static AHOLBuilding* SpawnBuilding(UHOLTestWorldHandle* TestWorld, FName TypeName, int32 Owner, FVector Location, EHOLConstructionState State);

	/** Destroys the building (AActor::Destroy); nothing for nullptr. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static void DestroyBuilding(AHOLBuilding* Building);

	/**
	 * Spawns the world's AHOLGameState. With NumHumans + NumAI > 0, then InitVictory with FHOLMatchConfig::MakeDefault() whose first NumHumans slots are
	 * Human, the next NumAI AI, the rest Closed (HOLFarmBuildersTests.cpp); with 0 and 0 the bare actor, no InitVictory (the case of the tier cheat).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static AHOLGameState* SpawnGameState(UHOLTestWorldHandle* TestWorld, int32 NumHumans, int32 NumAI);

	/**
	 * Spawns an AHOLPlayerState of the world (SpawnActor, SetMatchSlot(Slot, ColorIndex), SetPlayerName(Name)) as the Disconnect tests do; it is NOT
	 * added to the game state (see UHOLGameStateTestLibrary::AddPlayerState). nullptr if it fails.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static AHOLPlayerState* SpawnPlayerState(UHOLTestWorldHandle* TestWorld, int32 Slot, int32 ColorIndex, const FString& Name);

	/** The buildings of the world as FHOLBuildingIterator walks them (the registry), in its order. Empty for nullptr. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static TArray<AHOLBuilding*> GetBuildingsByRegistry(UHOLTestWorldHandle* TestWorld);

	/** The buildings of the world as TActorIterator<AHOLBuilding> walks them, in its order. Empty for nullptr. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static TArray<AHOLBuilding*> GetBuildingsByActors(UHOLTestWorldHandle* TestWorld);

	/** Whether the object's class has a property of this name with CPF_Net (a replicated property); false if none. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool IsPropertyReplicated(UObject* Object, FName PropertyName);

	// ---- Lots 3 and 4 (D853, ELROND-1119u): console, subsystem access, steps, stock, production ----

	/** The world itself as a plain UObject: the world context a static of a library taking const UObject* (UHOLAIOrderLibrary and the like) wants. nullptr once destroyed. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static UObject* GetWorldObject(UHOLTestWorldHandle* TestWorld);

	/** IConsoleManager::ProcessUserConsoleInput(Command, *GLog, World) as HOLDisconnectTests.cpp:928 (hol.Net.FakeAbsent and the like). Its return. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool RunConsoleCommand(UHOLTestWorldHandle* TestWorld, const FString& Command);

	/** UWorld::GetSubsystemBase(Class): any world subsystem of the test world (Cast it in script); nullptr if none. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static UWorldSubsystem* GetWorldSubsystem(UHOLTestWorldHandle* TestWorld, TSubclassOf<UWorldSubsystem> SubsystemClass);

	/** The world's UHOLProductionSubsystem::OnWorldBeginPlay (a test world calls it itself, as the tests do). False without the subsystem. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool InitProductionSubsystem(UHOLTestWorldHandle* TestWorld);

	/** The world's UHOLSourceSubsystem::OnWorldBeginPlay. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool InitSourceSubsystem(UHOLTestWorldHandle* TestWorld);

	/** The world's UHOLConstructionSubsystem::OnWorldBeginPlay (it subscribes to the step as at the match start). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool InitConstructionSubsystem(UHOLTestWorldHandle* TestWorld);

	/** UHOLProductionSubsystem::GetCost(TypeName, PlayerIndex).Resources (zero without the subsystem). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static FHOLResources GetProductionCostResources(UHOLTestWorldHandle* TestWorld, FName TypeName, int32 PlayerIndex);

	/** UHOLProductionSubsystem::GetCost(TypeName, PlayerIndex).BuildSteps (0 without the subsystem). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 GetProductionCostSteps(UHOLTestWorldHandle* TestWorld, FName TypeName, int32 PlayerIndex);

	/**
	 * What the Disconnect tests' Queue lambda does: Cost = GetCost(Technology, Slot); Stock = Payer's resources (FallbackStock if Payer is null);
	 * HOLProduction::TryEnqueue(Building->GetMutableProductionQueue(), Technology, Cost, MaxQueue, Stock); Payer gets Stock back. True when queued.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool EnqueueProduction(UHOLTestWorldHandle* TestWorld, AHOLBuilding* Building, FName Technology, int32 Slot, AHOLPlayerState* Payer, const FHOLResources& FallbackStock, int32 MaxQueue);

	/** UHOLHeroSubsystem::SetXP(Slot, XP) (hol.Hero.SetXP's entry). False without the subsystem or refused. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool SetHeroXP(UHOLTestWorldHandle* TestWorld, int32 Slot, int32 XP);

	/** UHOLHeroSubsystem::GetSheets(Slot).Private.XP (0 without the subsystem). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 GetHeroSheetXP(UHOLTestWorldHandle* TestWorld, int32 Slot);

	/** Whether UHOLHeroSubsystem::GetSheets(Slot).Private equals Player->GetHeroPrivate() (the whole private sheet). False for null. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool HeroSheetEqualsPlayer(UHOLTestWorldHandle* TestWorld, int32 Slot, AHOLPlayerState* Player);

	/** AHOLGameState::FindHeldBot(World, Slot): the AI's player state holding the seat of an absent player; nullptr if none. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static AHOLPlayerState* FindHeldBot(UHOLTestWorldHandle* TestWorld, int32 Slot);

	/** AHOLGameState::AddSlotStock(World, Slot, Gain, OutStock): a deposit on a slot's stock (the player's, the AI's or the absent slot's). True if it took. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool AddSlotStock(UHOLTestWorldHandle* TestWorld, int32 Slot, const FHOLResources& Gain);

	/** AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates(World, Config from Json, the settings' StartingResources): how many it made (0 if the JSON fails). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 SpawnAIPlayerStates(UHOLTestWorldHandle* TestWorld, const FString& Json);

	/**
	 * Broadcasts UHOLSimulationSubsystem::GetOnStepStarted() NumSteps times (the head of a step: what the subsystems issued is applied; no Mass
	 * processor runs), the step index counting on from 1 per world as the tests' StepCount does. Returns the number broadcast (0 without the subsystem).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 AdvanceSteps(UHOLTestWorldHandle* TestWorld, int32 NumSteps);

	/** As AdvanceSteps, and GetOnStepEnded() after each started broadcast (the tail of the step). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 AdvanceStepsWithEnd(UHOLTestWorldHandle* TestWorld, int32 NumSteps);

	/** A slot's stock, read from the player state of that PlayerIndex in the world's game state (zero if none). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static FHOLResources GetSlotResources(UHOLTestWorldHandle* TestWorld, int32 Slot);

	/** AHOLPlayerState::SetResources on the player state of that PlayerIndex (false if none). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool SetSlotResources(UHOLTestWorldHandle* TestWorld, int32 Slot, const FHOLResources& Resources);

	/** How many AHOLBuilding of TypeName owned by Owner the world holds (TActorIterator, as HOLAIBuildOrdersTests.cpp CountBuildings). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 CountBuildings(UHOLTestWorldHandle* TestWorld, FName TypeName, int32 Owner);
};
