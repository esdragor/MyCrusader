// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLSiteLayout.h"

AHOLSiteLayout::AHOLSiteLayout()
{
	PrimaryActorTick.bCanEverTick = false;
	// Data for the server: the sites are the simulation's (a client receives their views).
	bNetLoadOnClient = false;
	SetCanBeDamaged(false);
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

// STUB (red test first): every rule answers "nothing wrong" and no count is allowed; the implementation comes with the green commit.
bool FHOLSiteLayoutRules::IsCountAllowed(const int32 NumSites)
{
	return false;
}

int32 FHOLSiteLayoutRules::CountUnmatchedByHalfTurn(const TConstArrayView<FHOLPlacedSite> Sites)
{
	return 0;
}

int32 FHOLSiteLayoutRules::CountInBases(const TConstArrayView<FHOLPlacedSite> Sites, const TConstArrayView<FVector2D> SlotCentres, const float BaseReach)
{
	return 0;
}

int32 FHOLSiteLayoutRules::CountOutside(const TConstArrayView<FHOLPlacedSite> Sites, const double HalfSize)
{
	return 0;
}

int32 FHOLSiteLayoutRules::CountBadRadius(const TConstArrayView<FHOLPlacedSite> Sites)
{
	return 0;
}

int32 FHOLSiteLayoutRules::CountNearRelays(const TConstArrayView<FHOLPlacedSite> Sites, const TConstArrayView<FHOLPlacedRelay> Relays, const float Margin)
{
	return 0;
}

int32 FHOLSiteLayoutRules::CountNearPairs(const TConstArrayView<FHOLPlacedSite> Sites, const float Margin)
{
	return 0;
}
