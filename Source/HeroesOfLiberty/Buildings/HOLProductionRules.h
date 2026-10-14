// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLResources.h"
#include "HOLProductionRules.generated.h"

/**
 * Pure rules of the production of units at a building (J3, D280, D282), applied by the server at each simulation
 * step: the cost is taken when an item is queued and given back in full when it is cancelled; the item at the head of
 * the queue starts when the player has room for it in the population, and waits otherwise (as in AoE); it is done
 * BuildSteps steps after it started. No engine state: tested on their own.
 */

/** One item of a building's production queue, as replicated to the building's owner (D282). */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLProductionItem
{
	GENERATED_BODY()

	/** Unit type (row of the unit type table). */
	UPROPERTY()
	FName TypeName;

	/** Simulation step the item started at, NotStarted while it waits (not at the head, or population full). */
	UPROPERTY()
	int64 StartStep = NotStarted;

	static constexpr int64 NotStarted = -1;

	bool IsStarted() const { return StartStep != NotStarted; }

	bool operator==(const FHOLProductionItem& Other) const = default;
};

/** What producing one unit of a type takes (values of the unit type table, D283). */
struct FHOLProductionCost
{
	FHOLResources Resources;
	/** Production time in simulation steps, at least 1. */
	int32 BuildSteps = 1;
	/** Population places the unit takes. */
	int32 Population = 1;
};

/** Why an item could not be queued. */
enum class EHOLProductionRefusal : uint8
{
	None,
	/** The building does not produce this type. */
	NotProducible,
	QueueFull,
	NotEnoughResources,
};

namespace HOLProduction
{
	/** Population cap of every player (D03). */
	inline constexpr int32 MaxPopulation = 175;

	/** Population cap of a player whose buildings provide Provided places (centres, then houses, D83). */
	HEROESOFLIBERTY_API int32 GetPopulationCap(int32 Provided);

	/** Queues one unit of TypeName and takes its cost from Stock, or leaves both unchanged and says why not. */
	HEROESOFLIBERTY_API EHOLProductionRefusal TryEnqueue(TArray<FHOLProductionItem>& Queue, FName TypeName,
		const FHOLProductionCost& Cost, int32 MaxQueue, FHOLResources& Stock);

	/** Removes the item at Index and gives its cost back to Stock. False (nothing changed) when Index is out of the queue. */
	HEROESOFLIBERTY_API bool Cancel(TArray<FHOLProductionItem>& Queue, int32 Index, const FHOLProductionCost& Cost, FHOLResources& Stock);

	/**
	 * Advances a queue to Step: the head item that is done leaves the queue (its type goes to OutDone), and the new
	 * head starts if FreePopulation has room for it (FreePopulation then loses its places). A started item already
	 * holds its places: the caller counts them in the population used, with the player's units.
	 */
	HEROESOFLIBERTY_API void Advance(TArray<FHOLProductionItem>& Queue, int64 Step,
		TFunctionRef<FHOLProductionCost(FName)> GetCost, int32& FreePopulation, TArray<FName>& OutDone);

	/** Population places held by the started items of a queue. */
	HEROESOFLIBERTY_API int32 GetStartedPopulation(const TArray<FHOLProductionItem>& Queue, TFunctionRef<FHOLProductionCost(FName)> GetCost);

	/** Share of Item done at Step (with its fraction, the client's estimate of the server clock), 0 to 1. */
	HEROESOFLIBERTY_API float GetProgress(const FHOLProductionItem& Item, double Step, int32 BuildSteps);
}
