// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Match/HOLResources.h"
#include "Simulation/HOLGatherRules.h"
#include "HOLAISummary.generated.h"

class UWorld;
struct FHOLSource;
struct FHOLUnitView;

/**
 * The world as an AI slot reads it (IA-Q2, IA0.3): a summary per player slot, made in C++ (the 1 400 units are walked
 * here, not in script), read by the decisions (Angelscript, IA0.4). Made of three parts so far: the idle Peasants, the
 * sources near the slot's deposits, the stock. To come with their tranches: armies and their composition (IA2), threats
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
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	FVector Location = FVector::ZeroVector;

	/** What it still holds. */
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	int32 Remaining = 0;

	/**
	 * Distance (cm) from its ground point to the footprint of the slot's nearest deposit taking its resource
	 * (HOLGather::GetDepositDistance): the length of a Peasant's round trip, give or take the bodies.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float DepositDistance = 0.f;
};

/** One resource of the summary (IA0.3): the slot's stock of it and its sources near the slot's deposits. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLAIResourceSummary
{
	GENERATED_BODY()

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

namespace HOLAISummary
{
	/** Sources kept per resource: enough to spread the Peasants, few enough for a script to walk at each thought. */
	inline constexpr int32 DefaultMaxSourcesPerResource = 8;

	/**
	 * The rule (pure, tested alone): PlayerIndex's summary from the units (FHOLUnitView: the dying left out), the sources
	 * (the used-up and unused ones left out), the deposits (the slot's own only) and its stock (none: zeros, bHasStock
	 * false).
	 */
	HEROESOFLIBERTY_API FHOLAISummary Make(int32 PlayerIndex, TConstArrayView<FHOLUnitView> Units, TConstArrayView<FHOLSource> Sources,
		TConstArrayView<FHOLAIDeposit> Deposits, const TOptional<FHOLResources>& Stock, int32 MaxSourcesPerResource = DefaultMaxSourcesPerResource);

	/**
	 * The deposits of the world: the buildings that take deliveries (finished, a forum not down, D312, D351) and accept a
	 * resource, as UHOLGatherProcessor::RefreshDeposits reads them.
	 */
	HEROESOFLIBERTY_API void GatherDeposits(const UWorld& World, TArray<FHOLAIDeposit>& OutDeposits);

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
