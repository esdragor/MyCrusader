// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLGatherReadLibrary.generated.h"

/**
 * Read library of the pure gathering rules (HOLGatherRules.h; MIG-C-Gather, HALDIR-1117s, D830, D836, D839), envelopes only (no rule here),
 * so that the pure tests of HOLGatherTests.cpp can be ported to script (Script\Tests\HOLGather_Test.as). The rules stay in C++. A resource is its
 * number (EHOLResource: 0 Food, 1 Wood, 2 Stone, 3 Gold); a stock is four numbers in that order; a position is an FVector2D (cm); a deposit is its
 * centre, its half extent and its yaw (degrees); a source is its position and its resource, in parallel arrays, read by index. Functional style:
 * a load goes in by value and the new one comes back (no reference in/out parameter for the script).
 */

/** The load of a gatherer as the script sees it (FHOLGatherLoad with the resources as numbers). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLGatherLoadScript
{
	GENERATED_BODY()

	/** The resource carried (0 Food, 1 Wood, 2 Stone, 3 Gold). */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Gather")
	int32 Resource = 0;

	/** Whole units carried. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Gather")
	int32 Amount = 0;

	/** Share of the next unit already gathered (0 to 1). */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Gather")
	float Progress = 0.f;

	/** The resource Progress counts toward. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Gather")
	int32 ProgressResource = 0;
};

/** HOLGather:: for Angelscript (namespace HOLGatherRead there). */
UCLASS()
class HEROESOFLIBERTY_API UHOLGatherReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLGather::NoRoomSourceDistance (cm). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static double GetNoRoomSourceDistance();

	/** HOLGather::NearbyMassifDistance (cm). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static double GetNearbyMassifDistance();

	/** HOLGather::StartGathering: the load Load becomes once ordered to gather Resource. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static FHOLGatherLoadScript StartGathering(const FHOLGatherLoadScript& Load, int32 Resource);

	/**
	 * HOLGather::Gather called Steps times in a row (StepSeconds each) on a source holding SourceRemaining: the new load; the units taken in all, the
	 * load lost in all (D306) and what the source holds after.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static FHOLGatherLoadScript GatherSteps(const FHOLGatherLoadScript& Load, int32 Resource, double Rate, double StepSeconds, int32 Steps, int32 Capacity,
		int32 SourceRemaining, int32& OutTaken, int32& OutLost, int32& OutSourceRemaining);

	/** HOLGather::IsFull. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static bool IsFull(const FHOLGatherLoadScript& Load, int32 Capacity);

	/** HOLGather::IsFullOf. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static bool IsFullOf(const FHOLGatherLoadScript& Load, int32 Resource, int32 Capacity);

	/** HOLGather::Deposit into Stock (Food, Wood, Stone, Gold): the emptied load; the amount deposited and the stock after. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static FHOLGatherLoadScript Deposit(const FHOLGatherLoadScript& Load, const TArray<int32>& Stock, int32& OutDeposited, TArray<int32>& OutStock);

	/** HOLGather::GetEdgeDistance to the body of a site (centre, radius). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static double GetEdgeDistance(const FVector2D& Position, const FVector2D& SiteCentre, double SiteRadius);

	/** HOLGather::IsInContact with the body of a site. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static bool IsInContact(const FVector2D& Position, double BodyRadius, const FVector2D& SiteCentre, double SiteRadius);

	/** HOLGather::GetDepositDistance to the footprint (centre, half extent, yaw in degrees). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static double GetDepositDistance(const FVector2D& Position, const FVector2D& Centre, const FVector2D& HalfExtent, double Yaw);

	/** HOLGather::IsAtDeposit. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static bool IsAtDeposit(const FVector2D& Position, double BodyRadius, const FVector2D& Centre, const FVector2D& HalfExtent, double Yaw);

	/** HOLGather::GetDepositSpot. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static FVector2D GetDepositSpot(const FVector2D& From, const FVector2D& Centre, const FVector2D& HalfExtent, double Yaw, double Gap, double Along);

	/** HOLGather::FindNearestDeposit among the deposits read by index from the parallel arrays: an index, -1 (INDEX_NONE) if none. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static int32 FindNearestDeposit(const FVector2D& Position, const TArray<FVector2D>& Centres, const TArray<FVector2D>& HalfExtents, const TArray<float>& Yaws);

	/** HOLGather::GetSanctuaryFarmPercent. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static int32 GetSanctuaryFarmPercent(const TArray<FVector2D>& Sanctuaries, const FVector2D& Field);

	/** HOLGather::GetSourceSpots. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static void GetSourceSpots(const FVector2D& Centre, double Distance, double Spacing, const FVector2D& Towards, TArray<FVector2D>& OutSpots);

	/** HOLGather::GetStandPoints. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static void GetStandPoints(const FVector2D& Centre, double SpotDistance, double Spacing, const FVector2D& Towards, int32 Rows, int32 FirstRow,
		TArray<FVector2D>& OutPoints);

	/** HOLGather::IsClearOfSpots for the sources read by index (their positions and radii). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static bool IsClearOfSpots(const FVector2D& Point, const TArray<FVector2D>& SitePositions, const TArray<float>& SiteRadii, double SpotOffset, double Spacing);

	/**
	 * FHOLSpotClaims::IsTaken on a fresh grid holding the claims read by index (their owners and spots), the owners in StaleOwners being stale: whether
	 * Spot is taken for Owner; the claims left on the grid after the look (the own and stale ones met are dropped) and the staleness questions asked.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static bool IsSpotTaken(const TArray<int32>& ClaimOwners, const TArray<FVector2D>& ClaimSpots, const FVector2D& Spot, double MinSpacing, int32 Owner,
		const TArray<int32>& StaleOwners, int32& OutClaimsLeft, int32& OutStaleAsked);

	/** FHOLSourceMassifs::Build for the sources (positions, resources): each source's massif number (-1 for none). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static void GetMassifs(const TArray<FVector2D>& Positions, const TArray<int32>& Resources, double LinkDistance, double NearbyDistance, TArray<int32>& OutMassifOf);

	/** FHOLSourceMassifs::GetLinks(Index).Num() of the same build. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static int32 GetLinkCount(const TArray<FVector2D>& Positions, const TArray<int32>& Resources, double LinkDistance, double NearbyDistance, int32 Index);

	/** FHOLSourceMassifs::GetNearbyMassifs(Massif).Num() of the same build. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static int32 GetNearbyMassifCount(const TArray<FVector2D>& Positions, const TArray<int32>& Resources, double LinkDistance, double NearbyDistance, int32 Massif);

	/**
	 * FHOLSourceMassifs::Search from Start: the first source whose index is in Accepted (every other one is refused), -1 (INDEX_NONE) if none; the
	 * sources tried, in order, in OutTried. bHasDeposit gives the deposit unloaded at (centre, half extent, yaw), none otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Gather")
	static int32 SearchMassif(const TArray<FVector2D>& Positions, const TArray<int32>& Resources, double LinkDistance, double NearbyDistance, int32 Start,
		const TArray<int32>& Accepted, bool bHasDeposit, const FVector2D& DepositCentre, const FVector2D& DepositHalfExtent, double DepositYaw,
		TArray<int32>& OutTried);
};
