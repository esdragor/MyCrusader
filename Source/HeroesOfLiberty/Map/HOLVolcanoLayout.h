// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Simulation/HOLVolcanoRules.h"
#include "HOLVolcanoLayout.generated.h"

/** One lava flow of a volcano site a map lays (V4b, D607): a polyline on the ground in centimetres, the crater being its first point. */
USTRUCT(BlueprintType)
struct FHOLPlacedLavaFlow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Volcano")
	TArray<FVector> Points;
};

/** One volcano site a map lays (V4b, D607): its flows and its ancient altar (D380), the altar being optional. */
USTRUCT(BlueprintType)
struct FHOLPlacedVolcano
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Volcano")
	TArray<FHOLPlacedLavaFlow> Flows;

	UPROPERTY(EditAnywhere, Category = "Volcano")
	bool bAltar = false;

	UPROPERTY(EditAnywhere, Category = "Volcano")
	FVector AltarLocation = FVector::ZeroVector;

	/** The zone where units hold the altar (the ruin's, ~1200 cm, D348). */
	UPROPERTY(EditAnywhere, Category = "Volcano")
	float AltarRadius = 1200.f;
};

/**
 * The volcano sites of a map (V4b, D607), as data: the map script (Tools/Editor/maps/common.py, `volcano(recipe)` of the map's module) fills
 * the list, symmetric by construction; UHOLVolcanoSubsystem reads it when the match begins. Like the relay layout, nothing of this actor is
 * drawn or sent (a client gets the HUD views and, from V9, the drawn flows).
 *
 * What the maps owe (D573, D607; checked by Map.Table with FHOLVolcanoLayoutRules): the flows of a site are the same by a turn of one place
 * (as many flows towards each camp), the altar is as far from every base (equidistant), everything inside the bounds, every flow has a length.
 */
UCLASS(NotBlueprintable)
class HEROESOFLIBERTY_API AHOLVolcanoLayout : public AActor
{
	GENERATED_BODY()

public:
	AHOLVolcanoLayout();

	const TArray<FHOLPlacedVolcano>& GetSites() const { return Sites; }
	/** For the tests and the map scripts. */
	void SetSites(const TArray<FHOLPlacedVolcano>& InSites) { Sites = InSites; }

private:
	UPROPERTY(EditAnywhere, Category = "Volcano")
	TArray<FHOLPlacedVolcano> Sites;
};

/** The rules every map's volcano sites must satisfy (pure: the Map.Table test of a map calls them). */
struct HEROESOFLIBERTY_API FHOLVolcanoLayoutRules
{
	/** Tolerance of the symmetry, cm: a rounding of the map script is accepted. */
	static constexpr double SymmetryTolerance = 100.;

	/**
	 * Fairness of the flows (D573): the flows of each site turned by StepDegrees around the middle of the map, how many do not land on a flow of the
	 * same site (same number of points, each point within SymmetryTolerance of the turned one). 0 = every camp has as many flows towards it.
	 */
	static int32 CountUnbalancedFlows(TConstArrayView<FHOLPlacedVolcano> Sites, double StepDegrees);
	/** Fairness of the altar: the gap (cm) between the farthest and the nearest of the slots from the altar (0 without an altar or with fewer than two slots). */
	static double AltarSpread(const FHOLPlacedVolcano& Site, TConstArrayView<FVector2D> SlotCentres);
	/** Sites whose altar is more than SymmetryTolerance farther from one base than from another. */
	static int32 CountUnfairAltars(TConstArrayView<FHOLPlacedVolcano> Sites, TConstArrayView<FVector2D> SlotCentres);
	/** Flows with fewer than two points or no length, and sites without a flow. */
	static int32 CountEmptyFlows(TConstArrayView<FHOLPlacedVolcano> Sites);
	/** Points of the flows and altars (zone included) outside the square of HalfSize (cm) around the middle. */
	static int32 CountOutside(TConstArrayView<FHOLPlacedVolcano> Sites, double HalfSize);
	/** The sites as the simulation reads them (HOLVolcano::FSite). */
	static TArray<HOLVolcano::FSite> ToSites(TConstArrayView<FHOLPlacedVolcano> Placed);
};
