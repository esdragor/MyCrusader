// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Map/HOLRelayLayout.h"
#include "HOLSiteLayout.generated.h"

/** One sacred site a map lays (SacredSitesInProgress.md S4, D697): its centre on the ground and the radius of its zone in centimetres. */
USTRUCT(BlueprintType)
struct FHOLPlacedSite
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Site")
	FVector Location = FVector::ZeroVector;

	/** The zone where units take the site (provisional default: the relay's and the ruin's, 12 m; Tools/Editor/maps/common.py SITE_RADIUS). */
	UPROPERTY(EditAnywhere, Category = "Site")
	float Radius = 1200.f;
};

/**
 * The sacred sites of a map, as data: the map script (Tools/Editor/maps/common.py, `sites(recipe)` of the map's module, like `relays(recipe)`)
 * fills the list; the site subsystem reads it when the match begins. Like the relay layout, nothing of this actor is drawn or sent.
 *
 * What the maps owe (the user, 2026-11-07, Retours\Q-sites-nombre.md and Q-sites-symetrie.md): 2 or 3 sites IN ALL (not per player), symmetric by a
 * half turn around the middle of the map (the middle is its own image), no zone entering a base's reach or another site's or a relay's (with a margin),
 * all inside the bounds, a radius above 0. Where they lie is level design (the map's module; tools\Tests\sites_test.py checks the same rules).
 */
UCLASS(NotBlueprintable)
class HEROESOFLIBERTY_API AHOLSiteLayout : public AActor
{
	GENERATED_BODY()

public:
	AHOLSiteLayout();

	const TArray<FHOLPlacedSite>& GetSites() const { return Sites; }
	/** For the tests and the map scripts. */
	void SetSites(const TArray<FHOLPlacedSite>& InSites) { Sites = InSites; }

private:
	UPROPERTY(EditAnywhere, Category = "Site")
	TArray<FHOLPlacedSite> Sites;
};

/** The rules every map's sites must satisfy (pure: the Map.Table test of a map calls them; they mirror tools\Tests\sites_test.py). */
struct HEROESOFLIBERTY_API FHOLSiteLayoutRules
{
	/** Tolerance of the symmetry, cm: a rounding of the map script is accepted. */
	static constexpr double SymmetryTolerance = 100.;
	/** Sites IN ALL on a map (the user's answer: "2 a 3 selon la map"; the default is 2, common.py SITE_COUNT_DEFAULT). */
	static constexpr int32 MinSites = 2;
	static constexpr int32 MaxSites = 3;
	/** Free ground kept between two zones (a site and a relay, or two sites), cm (common.py SITE_MARGIN). */
	static constexpr float ZoneMargin = 500.f;

	/** Whether a map may hold this many sites in all. */
	static bool IsCountAllowed(int32 NumSites);
	/** Symmetry by a half turn around the middle of the map: how many sites do not land on a site of the same radius (within SymmetryTolerance). 0 = symmetric. */
	static int32 CountUnmatchedByHalfTurn(TConstArrayView<FHOLPlacedSite> Sites);
	/** Sites whose zone enters the reach (cm) of a start slot. */
	static int32 CountInBases(TConstArrayView<FHOLPlacedSite> Sites, TConstArrayView<FVector2D> SlotCentres, float BaseReach);
	/** Sites whose zone leaves the square of HalfSize (cm) around the middle. */
	static int32 CountOutside(TConstArrayView<FHOLPlacedSite> Sites, double HalfSize);
	/** Sites whose radius is not above 0. */
	static int32 CountBadRadius(TConstArrayView<FHOLPlacedSite> Sites);
	/** Sites whose zone is closer than Margin to a relay's zone. */
	static int32 CountNearRelays(TConstArrayView<FHOLPlacedSite> Sites, TConstArrayView<FHOLPlacedRelay> Relays, float Margin);
	/** Pairs of sites whose zones are closer than Margin to each other. */
	static int32 CountNearPairs(TConstArrayView<FHOLPlacedSite> Sites, float Margin);
};
