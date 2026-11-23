// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Match/HOLGameState.h"
#include "Match/HOLResources.h"
#include "Subsystems/WorldSubsystem.h"
#include "UObject/Object.h"
#include "Match/HOLSpecializationView.h"
#include "HOLTestWorldLibrary.generated.h"

class AHOLBuilding;
class AHOLGameState;
class AHOLPlayerState;
class UWorld;
class UHOLAIBrainSubsystem;

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
 * Counts the two events of a game state that are plain C++ delegates (not exposed to the script): OnSlotDefeated and OnMatchOver (D853, D839: a
 * binding to expose). Made by UHOLTestWorldLibrary::CountVictoryEvents; the script keeps it (the delegates hold it weakly).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLVictoryEventCounter : public UObject
{
	GENERATED_BODY()

public:
	/** C++ side only: starts counting the events of State. */
	void Bind(AHOLGameState* State);

	/** How many OnSlotDefeated were broadcast since Bind. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	int32 GetDefeats() const { return Defeats; }

	/** How many OnMatchOver were broadcast since Bind. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	int32 GetEnds() const { return Ends; }

private:
	void HandleDefeated(int32 Slot, EHOLDefeatReason Reason) { ++Defeats; }
	void HandleOver() { ++Ends; }

	int32 Defeats = 0;
	int32 Ends = 0;
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

	/** Like SpawnPlayerState but the slot is NOT set (what the C++ Disconnect tests do: SetMatchSlot comes later, HOLGameStateTest::SetPlayerMatchSlot). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static AHOLPlayerState* SpawnPlayerStateNoSlot(UHOLTestWorldHandle* TestWorld, const FString& Name);

	/** AHOLGameState::FindLiveHolder(World, Slot, Except); Except may be null. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static AHOLPlayerState* FindLiveHolder(UHOLTestWorldHandle* TestWorld, int32 Slot, const AHOLPlayerState* Except);

	/** AHOLGameState::GetSlotStock(World, Slot, Out): whether a stock is read (a live holder, an absent record or a held bot). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool HasSlotStock(UHOLTestWorldHandle* TestWorld, int32 Slot);

	/** The stock AHOLGameState::GetSlotStock reads (zero if HasSlotStock is false). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static FHOLResources GetSlotStock(UHOLTestWorldHandle* TestWorld, int32 Slot);

	/** UHOLAIBrainSubsystem::Find(World): the script brain (UHOLAIBrain), null if none in this world. Its ThinkNow, IsHeld, GetThinkingSlots, SetHeld, GetAISlots are UFUNCTIONs already. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static UHOLAIBrainSubsystem* GetAIBrain(UHOLTestWorldHandle* TestWorld);

	/** AHOLBuilding::GetProductionQueue().Num() (0 for null). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 GetProductionQueueCount(const AHOLBuilding* Building);

	/** The TypeName of item Index of the building's production queue (NAME_None out of range). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static FName GetProductionQueueName(const AHOLBuilding* Building, int32 Index);

	/** Item Index of the queue has started (StartStep != NotStarted); false out of range. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool IsProductionQueueItemStarted(const AHOLBuilding* Building, int32 Index);

	/** AHOLBuilding::GetConstructionState() as int (EHOLConstructionState; 0 for null). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 GetBuildingConstructionState(const AHOLBuilding* Building);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int64 GetBuildingConstructionWork(const AHOLBuilding* Building);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int64 GetBuildingTotalWork(const AHOLBuilding* Building);

	/** HOLConstruction::GetCancelRefund(Type->Cost, State, Work, TotalWork) of the building (zero if null or no type row). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static FHOLResources GetBuildingCancelRefund(const AHOLBuilding* Building);

	/** UHOLGatherProcessor::DepositLoad(World, Owner, Load{Resource as EHOLResource, Amount}, Out): the amount deposited (the load is emptied: Amount minus it stays). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 DepositLoad(UHOLTestWorldHandle* TestWorld, int32 Owner, int32 Resource, int32 Amount);

	/** As DepositLoad, and OutRemaining is the load's Amount left after the deposit (what the C++ Disconnect test reads: 0 once emptied). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 DepositLoadWithRemaining(UHOLTestWorldHandle* TestWorld, int32 Owner, int32 Resource, int32 Amount, int32& OutRemaining);

	// ---- Lot D853 d (CELEBORN-1119ba): the readings the Disconnect tests still kept in C++ ----

	/** UHOLConstructionSubsystem::DestroyBuilding(*Building): the combat's path (a foundation gives its share of the cost back, a queue its costs). Not the plain Destroy of DestroyBuilding. Nothing for null. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static void DestroyBuildingWithRefund(AHOLBuilding* Building);

	/** UHOLAIToolsLibrary::GetSlotLevel(World, Slot): EHOLAIDifficulty as an int of an AI slot (a held seat reads Normal = 1), -1 for a slot that is not an AI. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 GetSlotLevel(UHOLTestWorldHandle* TestWorld, int32 Slot);

	/** UHOLAIOrderLibrary::IssueAIProduceOrder(World, Slot, BuildingLocation, TypeName) with the test world as the world context (the script's own context does not reach it). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool IssueAIProduceOrder(UHOLTestWorldHandle* TestWorld, int32 Slot, FVector BuildingLocation, FName TypeName);

	/** UHOLAITechnologyLibrary::ReadAITechnologiesDone(World, Slot, Out): the technologies the slot has researched, in order (empty if none). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static TArray<FName> GetAITechnologiesDone(UHOLTestWorldHandle* TestWorld, int32 Slot);

	/** How many AHOLPlayerState of the game state's PlayerArray have this PlayerIndex (what the C++ tests count: one player state for the seat). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 CountPlayerStatesOfSlot(UHOLTestWorldHandle* TestWorld, int32 Slot);

	/** AHOLPlayerState::GetPlayerIndex() (-1 for null). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 GetPlayerStateSlot(const AHOLPlayerState* Player);

	/** Starts keeping every line the game logs (GLog), for CountCapturedLogLines; a capture already running is ended first. End it with EndLogCapture. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static void BeginLogCapture();

	/** How many captured lines contain both A and B (the pending threaded log is flushed first). 0 without a capture. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 CountCapturedLogLines(const FString& A, const FString& B);

	/** Stops the capture and drops its lines. Nothing without a capture. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static void EndLogCapture();

	// ---- Lot D853 e (CELEBORN-1119ca): the AI build orders (HOLAIBuildOrdersTests.cpp). Thin wrappers: the test world is the world context ----

	/** UHOLAIOrderLibrary::CheckAIPlacement(World, Slot, TypeName, Point): 0 = placeable, an EHOLPlacementRefusal otherwise, -1 for a type nobody builds. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 CheckAIPlacement(UHOLTestWorldHandle* TestWorld, int32 Slot, FName TypeName, FVector Point);

	/** UHOLAIOrderLibrary::FindAIPlacement(World, Slot, TypeName, Centre, MaxRings, OutPoint). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool FindAIPlacement(UHOLTestWorldHandle* TestWorld, int32 Slot, FName TypeName, FVector Centre, int32 MaxRings, FVector& OutPoint);

	/** UHOLAIOrderLibrary::FindAIFarmBlock(World, Slot, Centre, MaxRings, OutMill, OutFarms). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool FindAIFarmBlock(UHOLTestWorldHandle* TestWorld, int32 Slot, FVector Centre, int32 MaxRings, FVector& OutMill, TArray<FVector>& OutFarms);

	/** UHOLAIOrderLibrary::IssueAIPlaceOrder(World, Slot, TypeName, Point, {} units, no queue) as the C++ test calls it. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool IssueAIPlaceOrder(UHOLTestWorldHandle* TestWorld, int32 Slot, FName TypeName, FVector Point);

	/** UHOLAIOrderLibrary::IssueAISetRallyOrder(World, Slot, BuildingLocation, Point) (source index -1). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool IssueAISetRallyOrder(UHOLTestWorldHandle* TestWorld, int32 Slot, FVector BuildingLocation, FVector Point);

	/** static_cast<int32>(EHOLPlacementRefusal::Building): the refusal a spot on a building gets (the enum is not exposed to the script). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 GetPlacementRefusalBuilding();

	/** Whether the footprints (UHOLConstructionSubsystem::SnapFootprint of the mill row at Mill and of the farm row at each farm) overlap each other (HOLConstruction::Overlaps, all pairs). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool FarmBlockOverlaps(FVector Mill, const TArray<FVector>& Farms);

	/** HOLConstruction::TryPay(copy of Stock, UHOLBuildingSettings::FindType(TypeName)->Cost): whether the stock covers it; OutAfter is the stock after paying. False if no type row. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool PayBuildingCost(FName TypeName, const FHOLResources& Stock, FHOLResources& OutAfter);

	/** AHOLBuilding::HasRallyPoint() (false for null). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool HasBuildingRallyPoint(const AHOLBuilding* Building);

	/** AHOLBuilding::GetRallyPoint() (zero for null). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static FVector GetBuildingRallyPoint(const AHOLBuilding* Building);

	// ---- Lot D853 f (CELEBORN-1119cb): the unit tier production (HOLUnitTierProductionTests.cpp). Thin wrappers, bodies under WITH_DEV_AUTOMATION_TESTS ----

	/** AHOLGameState::SetTierOverride(Slot, Tier) on the test world's game state (false if there is none). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool SetSlotTierOverride(UHOLTestWorldHandle* TestWorld, int32 Slot, int32 Tier);

	/** AHOLGameState::GetTier(Slot) (-1 if there is no game state). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 GetSlotTier(UHOLTestWorldHandle* TestWorld, int32 Slot);

	/** UHOLCommandSubsystem::IssueProductionOrder(FHOLProductionCommand{PlayerIndex = Slot, Building, TypeName}): the server's command queue, as a client's order; the order is applied at the next step. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool IssueProductionOrder(UHOLTestWorldHandle* TestWorld, int32 Slot, AHOLBuilding* Building, FName TypeName);

	/** Food + Wood + Stone + Gold of the unit row of TypeName (UHOLUnitSubsystem::FindTypeRow), -1 without a row. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 GetUnitCostTotal(UHOLTestWorldHandle* TestWorld, FName TypeName);

	// ---- Lot D853 g (CELEBORN-1119cc): the building tests without components (HOLBuildingTests.cpp: Foundation, Selection, Damage). Thin wrappers on the real AHOLBuilding methods ----

	/** AHOLBuilding::SetConstructionState(State) (nothing for null). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static void SetBuildingConstructionState(AHOLBuilding* Building, EHOLConstructionState State);

	/** AHOLBuilding::SetConstructionWork(Work) (nothing for null). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static void SetBuildingConstructionWork(AHOLBuilding* Building, int64 Work);

	/** AHOLBuilding::ApplyPendingAppearance(): the look the frame applies (D325), at once. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static void ApplyBuildingAppearance(AHOLBuilding* Building);

	/** AHOLBuilding::GetHealth() (0 for null). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 GetBuildingHealth(const AHOLBuilding* Building);

	/** AHOLBuilding::IsHurt() (false for null). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool IsBuildingHurt(const AHOLBuilding* Building);

	/** AHOLBuilding::IsFoundation() (false for null). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool IsBuildingFoundation(const AHOLBuilding* Building);

	/** AHOLBuilding::IsFinished() (false for null). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool IsBuildingFinished(const AHOLBuilding* Building);

	/** AHOLBuilding::HasWorkStarted() (false for null). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool HasBuildingWorkStarted(const AHOLBuilding* Building);

	/** AHOLBuilding::IsSeenHere() (false for null). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool IsBuildingSeenHere(const AHOLBuilding* Building);

	/** IsValid(Building): false once the actor is destroyed (or for null). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool IsBuildingAlive(const AHOLBuilding* Building);

	/** How many UStaticMeshComponent the building owns (AHOLBuilding::GetComponents, as the C++ test counts its pieces). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 CountBuildingMeshPieces(const AHOLBuilding* Building);

	/** UHOLConstructionSubsystem::DamageBuilding(*Building, Damage, Ranged or Melee): the health actually taken off (armour applied by the game). 0 for null. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 DamageBuildingByAttack(AHOLBuilding* Building, int32 Damage, bool bRanged);

	/** The type row's MaxHealth (UHOLBuildingSettings::FindType(TypeName)), -1 without a row. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 GetBuildingTypeMaxHealth(FName TypeName);

	/** The type row's RangedArmor, -1 without a row. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 GetBuildingTypeRangedArmor(FName TypeName);

	/**
	 * The selection ray as the C++ tests throw it: World->LineTraceSingleByChannel(HOLCollision::Selection) from (X, Y, 5000) straight down 10000 cm.
	 * Returns the actor hit (nullptr if the ray hits nothing).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static AActor* TraceSelectionRayDown(UHOLTestWorldHandle* TestWorld, double X, double Y);

	/** AHOLGameState::DefeatSlot (not a UFUNCTION): the server's defeat of a slot; false for a closed slot, a defeated one, or once the match is over. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool DefeatGameStateSlot(AHOLGameState* State, int32 Slot, EHOLDefeatReason Reason);

	/** AHOLGameState::IsSlotDefeated. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool IsGameStateSlotDefeated(const AHOLGameState* State, int32 Slot);

	/** AHOLGameState::GetDefeatReason (None: still in the match). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static EHOLDefeatReason GetGameStateDefeatReason(const AHOLGameState* State, int32 Slot);

	/** AHOLGameState::IsMatchOver. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool IsGameStateMatchOver(const AHOLGameState* State);

	/** AHOLGameState::GetWinners (slots, ascending). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static TArray<int32> GetGameStateWinners(const AHOLGameState* State);

	/** AHOLGameState::IsWinner. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool IsGameStateWinner(const AHOLGameState* State, int32 Slot);

	/** A counter of the OnSlotDefeated and OnMatchOver broadcasts of State, started now (nullptr without a state). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static UHOLVictoryEventCounter* CountVictoryEvents(AHOLGameState* State);

	// ---- ELROND-1123pf (D839, lot of bindings 2): test-only helpers, bodies under WITH_DEV_AUTOMATION_TESTS (false / 0 elsewhere) ----

	/**
	 * Makes the hero of Slot be born ALIVE in the test world, as a match's hero unit (type Hero, owner Slot, at Location): the hero subsystem's steps are
	 * bound first (as the C++ tests do). Returns the unit identifier (0 if none was born: no world, no unit table, bad slot). The slot's XP is then
	 * set with SetHeroXP (level 4 opens the banner), and UHOLHeroLibrary::PlantHeroBanner / HOLHero abilities have a hero to work with.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static int32 SpawnTestHero(UHOLTestWorldHandle* TestWorld, int32 Slot, FVector Location);

	/**
	 * AHOLPlayerState::SetSpecialization (the server's copy of the specialization view onto the player; not a UFUNCTION): TESTS only, never the game.
	 * Returns what SetSpecialization returns; false without a player.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool SetPlayerSpecialization(AHOLPlayerState* Player, const FHOLSpecializationView& View);
};
