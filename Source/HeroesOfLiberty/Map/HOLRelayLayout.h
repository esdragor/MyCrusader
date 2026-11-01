// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HOLRelayLayout.generated.h"

/** One neutral relay a map lays (R2, D388): its centre on the ground and the radius of its zone in centimetres. */
USTRUCT(BlueprintType)
struct FHOLPlacedRelay
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Relay")
	FVector Location = FVector::ZeroVector;

	/** The zone where units hold the relay (provisional default: the ruin's, HOLForumRules ForumHoldRadius ~ 1200 cm). */
	UPROPERTY(EditAnywhere, Category = "Relay")
	float Radius = 1200.f;
};

/**
 * The relays of a map (R2), as data: the map script (Tools/Editor/maps/common.py, `relays(recipe)` of the map's module, like `zones`) fills
 * the list; UHOLRelaySubsystem reads it when the match begins. Like the terrain layout, nothing of this actor is drawn or sent (the rings
 * are drawn from the replicated views, R5).
 *
 * What the maps owe (RelaysInProgress.md): a list of (x, y, radius) symmetric by a turn of one place (FHOLRelayLayoutRules::
 * CountUnmatchedByTurn = 0 for the map's number of places), the middle of the map allowed, no zone entering a base's reach, all inside the
 * bounds, a radius above 0. How many relays and where is level design.
 */
UCLASS(NotBlueprintable)
class HEROESOFLIBERTY_API AHOLRelayLayout : public AActor
{
	GENERATED_BODY()

public:
	AHOLRelayLayout();

	const TArray<FHOLPlacedRelay>& GetRelays() const { return Relays; }
	/** For the tests and the map scripts. */
	void SetRelays(const TArray<FHOLPlacedRelay>& InRelays) { Relays = InRelays; }

private:
	UPROPERTY(EditAnywhere, Category = "Relay")
	TArray<FHOLPlacedRelay> Relays;
};

/** The rules every map's relays must satisfy (pure: the Map.Table test of a map calls them). */
struct HEROESOFLIBERTY_API FHOLRelayLayoutRules
{
	/** Tolerance of the symmetry, cm: a rounding of the map script is accepted. */
	static constexpr double SymmetryTolerance = 100.;

	/**
	 * Symmetry (TC-Q7, 8.1): the relays turned by StepDegrees around the middle of the map, how many do not land on a relay of the same
	 * radius (within SymmetryTolerance). 0 = symmetric. The middle of the map is its own image.
	 */
	static int32 CountUnmatchedByTurn(TConstArrayView<FHOLPlacedRelay> Relays, double StepDegrees);
	/** Relays whose zone enters the reach (cm) of a start slot. */
	static int32 CountInBases(TConstArrayView<FHOLPlacedRelay> Relays, TConstArrayView<FVector2D> SlotCentres, float BaseReach);
	/** Relays whose zone leaves the square of HalfSize (cm) around the middle. */
	static int32 CountOutside(TConstArrayView<FHOLPlacedRelay> Relays, double HalfSize);
	/** Relays whose radius is not above 0. */
	static int32 CountBadRadius(TConstArrayView<FHOLPlacedRelay> Relays);
};
