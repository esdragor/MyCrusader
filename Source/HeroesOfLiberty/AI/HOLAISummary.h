// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Match/HOLResources.h"
#include "Simulation/HOLGatherRules.h"
#include "HOLAISummary.generated.h"

class UWorld;
class AHOLBuilding;
struct FHOLSource;
struct FHOLUnitView;

/**
 * The world as an AI slot reads it (IA-Q2, IA0.3): a summary per player slot, made in C++ (the 1 400 units are walked
 * here, not in script), read by the decisions (Angelscript, IA0.4). Made of these parts so far: the idle Peasants, the
 * sources near the slot's deposits, the stock, the population, the slot's own buildings and sites, its Peasants by
 * resource, its centre (IA1.3: what its player's HUD shows). To come with their tranches: armies and their composition (IA2), threats
 * near the base (IA2), forums (IA3); with M7, the reading goes through the slot's vision.
 * The fields the brain's rule reads are ScriptReadWrite (IA0.4-Q2 A): the script tests build summaries by hand
 * (Script/AI/HOLAIEconomy_Test.as); Blueprint still only reads them.
 */

/** A source near the slot's deposits (IA0.3): where its Peasants may gather. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLAISourceNear
{
	GENERATED_BODY()

	/** Its index in UHOLSourceSubsystem::GetSources: what UHOLCommandSubsystem::IssueGatherOrder takes. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 SourceIndex = INDEX_NONE;

	/** Its ground point (cm). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	FVector Location = FVector::ZeroVector;

	/** What it still holds. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 Remaining = 0;

	/**
	 * Distance (cm) from its ground point to the footprint of the slot's nearest deposit taking its resource
	 * (HOLGather::GetDepositDistance): the length of a Peasant's round trip, give or take the bodies.
	 */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float DepositDistance = 0.f;

	/**
	 * Its places (IA1.7b): how many gatherers it takes at once (a farm field: 1); 0 = places all around (a tree, a bush, a
	 * vein: as many as the room allows).
	 */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 Places = 0;

	/**
	 * The places held now (IA1.7b): the gatherers on their way to it, at work or unloading (what a player sees as a worker
	 * on the field). 0 for a source with places all around.
	 */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 Occupants = 0;

	/**
	 * A Peasant sent to it stood idle in front of it, no spot free, a few seconds ago (IA2.6b: what a player sees as workers
	 * standing round a vein): the brain sends no one there until it has room again.
	 */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	bool bFull = false;
};

/** One resource of the summary (IA0.3): the slot's stock of it and its sources near the slot's deposits. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLAIResourceSummary
{
	GENERATED_BODY()

	/** Its Peasants gathering this resource, on their round trip or at the source (the HUD's count; IA1.3). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 Peasants = 0;

	/** The slot's stock of this resource (0 without a stock, see FHOLAISummary::bHasStock). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 Stock = 0;

	/**
	 * The standing sources of this resource, nearest a deposit of the slot taking it first (a tie: the lower index), at
	 * most HOLAISummary::DefaultMaxSourcesPerResource. Empty when the slot has no finished deposit taking it.
	 */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	TArray<FHOLAISourceNear> Sources;
};

/** A building or site of the slot (IA1.3): what its owner sees of it. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLAIBuilding
{
	GENERATED_BODY()

	/** Its owner's slot. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 OwnerPlayer = INDEX_NONE;

	/** Its type's row name (HOLBuildingTypeNames). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	FName TypeName;

	/** Foundation (placed and paid), UnderConstruction, Finished. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	EHOLConstructionState State = EHOLConstructionState::Finished;

	/** Share of its construction done, 0 to 1 (1 once finished). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float Progress = 0.f;

	/** Its ground point (cm). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	FVector Location = FVector::ZeroVector;

	/** Its production queue, head first: the unit types waiting or in production (empty for a site). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	TArray<FName> Queue;

	/** The head of the queue is in production (not waiting for room in the population). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	bool bProducing = false;
};

/**
 * A slot's main forum as the AI reads it (IA3.1; D353 makes the lives and the phase public): its lives, whether it stands,
 * lies in ruin or rises again, and what the ruin hold has counted. GatherForums reads the game state; Make adds Location
 * (the slot's main centre, in any state).
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLAIForum
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 Slot = INDEX_NONE;

	/** Lives left (D349), the one it stands on included. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 Lives = 0;

	/** Fallen: in ruin or rising again (inert and unbreakable, D351). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	bool bDown = false;

	/** In ruin: the rubble phase, nothing rebuilds it yet. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	bool bRuin = false;

	/** Rising again (the rebuilding runs, its Peasants speed it). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	bool bRebuilding = false;

	/** In ruin: seconds before the rebuilding starts. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float RuinSecondsLeft = 0.f;

	/** Share of the rebuilding done, 0 to 1 (1 standing). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float RebuildProgress = 1.f;

	/** Share of the ruin hold counted (V3: 1 razes the foundations), 0 standing. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float HoldProgress = 0.f;

	/** Its defenders contest the hold (the count is frozen). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	bool bContested = false;

	/** Where its main centre stands (cm); bHasLocation false when the slot has none. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	bool bHasLocation = false;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	FVector Location = FVector::ZeroVector;
};

/** A soldier or an enemy unit as the AI reads it (IA2.1): the slot's army, or what it knows of the others' units. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLAIUnit
{
	GENERATED_BODY()

	/** Its unit identifier (uint32 in C++; it fits an int32, Blueprint and script have no uint32). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 Id = INDEX_NONE;

	/** Its owner's slot. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 OwnerSlot = INDEX_NONE;

	/** Its type's row name (Spearman, ManAtArms, Archer, Crossbowman, Peasant...). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	FName Type;

	/** Where it stands (cm). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	FVector Location = FVector::ZeroVector;

	/** Its health points now. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 Health = 0;

	/** It stands with nothing to do (no order, not fighting: the HUD's Idle activity). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	bool bIdle = false;

	/** While it strikes: the unit identifier of its target; INDEX_NONE otherwise. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 TargetId = INDEX_NONE;
};

/** The summary of the world for one player slot (IA0.3). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLAISummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "AI")
	int32 PlayerIndex = INDEX_NONE;

	/**
	 * Its idle Peasants, by identifier (ascending): its workers (units whose type gathers, a CarryCapacity) standing with
	 * nothing to do, as the HUD's idle worker count (D307): a gatherer on its round trip or a builder is not idle. Unit
	 * identifiers are uint32 in C++; they fit an int32 (Blueprint and script have no uint32).
	 */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	TArray<int32> IdlePeasants;

	/**
	 * All its Peasants (IA1.5: the workers, idle or not, the dying left out), by identifier (ascending): the opening takes
	 * its builders from them when none stands idle.
	 */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	TArray<int32> PeasantIds;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	FHOLAIResourceSummary Food;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	FHOLAIResourceSummary Wood;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	FHOLAIResourceSummary Stone;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	FHOLAIResourceSummary Gold;

	/** A stock was found for the slot (HOLAISummary::ReadStock: its PlayerState); without one, every Stock is 0. */
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	bool bHasStock = false;

	/** Population (IA1.3): places taken (units and units in production), places given by its finished buildings (175 at most). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 PopulationUsed = 0;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 PopulationCap = 0;

	/** Units in the production queues of its buildings, started or not. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 PopulationQueued = 0;

	/** The population and the Peasants by resource were found for the slot (its PlayerState); without one they are 0. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	bool bHasPopulation = false;

	/** Its buildings and sites (foundations too: its owner sees them), by type name, then position. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	TArray<FHOLAIBuilding> Buildings;

	/** It has a main centre (any state) and where it stands (the first, by position, if it has several). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	bool bHasCentre = false;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	FVector CentreLocation = FVector::ZeroVector;

	/**
	 * Its army (IA2.1): its soldiers (Spearman, ManAtArms, Archer, Crossbowman...), the dying left out, by identifier
	 * (ascending). Not its Peasants (PeasantIds), nor its King and its hero (left out of the army: they do not take the
	 * soldiers' orders; the hero comes with his own chantier).
	 */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	TArray<FHOLAIUnit> Army;

	/**
	 * Its Peasants as units (IA2.6f): where each stands and its health, the dying left out, by identifier (ascending), the
	 * same workers as PeasantIds: the brain makes the ones near an enemy soldier flee.
	 */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	TArray<FHOLAIUnit> Peasants;

	/**
	 * The units of every other slot that is not defeated (IA2.1; in the prototype every other slot is an enemy), Peasants,
	 * King and hero included (Type tells), the dying left out, by identifier (ascending). Read in one place,
	 * HOLAISummary::GatherEnemies: that is where the fog (M7) will cut the list to what the slot's vision shows.
	 */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	TArray<FHOLAIUnit> Enemies;

	/**
	 * An enemy main centre is known (IA2.1) and where the nearest stands to the slot's own centre (a tie: the lower slot;
	 * without a centre of its own: the lowest slot's): the target of the attack waves. Like Enemies, cut by the fog later.
	 */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	bool bHasEnemyCentre = false;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	FVector EnemyCentreLocation = FVector::ZeroVector;

	/**
	 * Its own main forum (IA3.1): its lives and phase; bHasOwnForum false when the slot has none (or no game state).
	 */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	bool bHasOwnForum = false;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	FHOLAIForum OwnForum;

	/** The main forums of the other slots that are not defeated (IA3.1), by slot (ascending). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	TArray<FHOLAIForum> EnemyForums;

	/** Its King (IA3.1, Regicide): alive and not dying; he does not belong to the army. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	bool bHasKing = false;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	FHOLAIUnit King;

	/** The match is Regicide (a dead King defeats his slot, D355). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	bool bRegicide = false;

	/** The part of one resource. */
	FHOLAIResourceSummary& Get(EHOLResource Resource);
	const FHOLAIResourceSummary& Get(const EHOLResource Resource) const { return const_cast<FHOLAISummary&>(*this).Get(Resource); }

	/** The stock of the four resources. */
	FHOLResources GetStock() const { return FHOLResources(Food.Stock, Wood.Stock, Stone.Stock, Gold.Stock); }

	/** "slot 1: 3 idle; stock F 200 W 200 S 200 G 200; sources F 4 W 8 S 2 G 1 (nearest W 350 cm)": for the logs. */
	FString ToString() const;
};

/** A deposit as the summary reads it (IA0.3): a finished building that takes deliveries, its owner and what it takes. */
struct FHOLAIDeposit
{
	FHOLGatherDeposit Footprint;
	uint8 OwnerPlayer = 0;
	TArray<EHOLResource> Accepts;
};

/** The population and gatherers a slot's PlayerState holds (IA1.3). */
struct FHOLAIPlayerFigures
{
	int32 PopulationUsed = 0;
	int32 PopulationCap = 0;
	FHOLResources Gatherers;
};

/** What a source of few places holds (IA1.7b), by source index: Make reads it beside the sources. */
struct FHOLAISourceUse
{
	int32 Places = 0;
	int32 Occupants = 0;
	bool bFull = false;
};

namespace HOLAISummary
{
	/** Sources kept per resource: enough to spread the Peasants, few enough for a script to walk at each thought. */
	inline constexpr int32 DefaultMaxSourcesPerResource = 8;

	/**
	 * The rule (pure, tested alone): PlayerIndex's summary from the units (FHOLUnitView: the dying left out), the sources
	 * (the used-up and unused ones left out), the deposits (the slot's own only) and its stock (none: zeros, bHasStock
	 * false). IA2.1: its army comes from Units (its soldiers); the Enemies come ONLY from the Enemies argument (what
	 * GatherEnemies read: Make keeps the units of a slot other than PlayerIndex, not defeated (bit of DefeatedMask), not
	 * dying), and the enemy centre from the Buildings of those slots.
	 */
	HEROESOFLIBERTY_API FHOLAISummary Make(int32 PlayerIndex, TConstArrayView<FHOLUnitView> Units, TConstArrayView<FHOLSource> Sources,
		TConstArrayView<FHOLAIDeposit> Deposits, const TOptional<FHOLResources>& Stock, int32 MaxSourcesPerResource = DefaultMaxSourcesPerResource,
		TConstArrayView<FHOLAIBuilding> Buildings = {}, const TOptional<FHOLAIPlayerFigures>& Figures = {},
		TConstArrayView<FHOLAISourceUse> Uses = {}, TConstArrayView<FHOLUnitView> Enemies = {}, uint8 DefeatedMask = 0,
		TConstArrayView<FHOLAIForum> Forums = {}, bool bRegicide = false);

	/**
	 * The deposits of the world: the buildings that take deliveries (finished, a forum not down, D312, D351) and accept a
	 * resource, as UHOLGatherProcessor::RefreshDeposits reads them.
	 */
	HEROESOFLIBERTY_API void GatherDeposits(const UWorld& World, TArray<FHOLAIDeposit>& OutDeposits);

	/**
	 * The main forum of every occupied slot (IA3.1): lives, phase, ruin seconds, rebuilding and hold, from the game state
	 * (public, D353); the place is added by Make. The fog (M7) never hides the lives or the phase, only where a camp's forum stands.
	 */
	HEROESOFLIBERTY_API void GatherForums(const UWorld& World, TArray<FHOLAIForum>& OutForums);

	/** Every building and site of the world as its owner sees it (IA1.3); Make keeps the slot's own. */
	HEROESOFLIBERTY_API void GatherBuildings(const UWorld& World, TArray<FHOLAIBuilding>& OutBuildings);

	/**
	 * THE one place the AI reads the other slots' units (IA2-Q1 A): every unit of the world that is not the slot's, today
	 * (nothing is hidden). With the fog (M7) this is the only function to cut to the camp's vision; the rules in script and
	 * Make do not change. The list as FHOLUnitView, the dying left out.
	 */
	HEROESOFLIBERTY_API void GatherEnemies(const UWorld& World, int32 PlayerIndex, TArray<FHOLUnitView>& OutEnemies);

	/** The population and gatherers of a slot's PlayerState (as ReadStock), unset without one. */
	HEROESOFLIBERTY_API TOptional<FHOLAIPlayerFigures> ReadFigures(const UWorld& World, int32 PlayerIndex);

	/**
	 * The stock of a slot: its AHOLPlayerState's (found by its index in the game state's PlayerArray), unset without one.
	 * Kept apart on purpose: an AI slot has no PlayerState until IA0.1, and the stock may move to the game state
	 * (AHOLGameState::GetSlotStock, R7 of the network work): only this function changes then.
	 */
	HEROESOFLIBERTY_API TOptional<FHOLResources> ReadStock(const UWorld& World, int32 PlayerIndex);

	/** PlayerIndex's summary of a world (the server's or a solo game's): Make fed by the subsystems. */
	HEROESOFLIBERTY_API FHOLAISummary Read(const UWorld& World, int32 PlayerIndex);
}

/** The summary for Blueprint and Angelscript (IA0.4's brain). */
UCLASS()
class HEROESOFLIBERTY_API UHOLAISummaryLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** PlayerIndex's summary of the world of WorldContextObject (HOLAISummary::Read); empty without a world. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static FHOLAISummary ReadAISummary(const UObject* WorldContextObject, int32 PlayerIndex);
};
