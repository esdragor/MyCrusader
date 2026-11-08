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

bool FHOLSiteLayoutRules::IsCountAllowed(const int32 NumSites)
{
	return NumSites >= MinSites && NumSites <= MaxSites;
}

int32 FHOLSiteLayoutRules::CountUnmatchedByHalfTurn(const TConstArrayView<FHOLPlacedSite> Sites)
{
	int32 Unmatched = 0;
	for (const FHOLPlacedSite& Site : Sites)
	{
		// A turn of 180 degrees around the middle of the map: (x, y) -> (-x, -y). As tools\Tests\sites_test.py: the twin within SymmetryTolerance (distance), same radius.
		const FVector2D Turned(-Site.Location.X, -Site.Location.Y);
		bool bMatched = false;
		for (const FHOLPlacedSite& Other : Sites)
		{
			if (FVector2D::Distance(FVector2D(Other.Location), Turned) <= SymmetryTolerance && FMath::Abs(Other.Radius - Site.Radius) < 1.f)
			{
				bMatched = true;
				break;
			}
		}
		Unmatched += !bMatched;
	}
	return Unmatched;
}

int32 FHOLSiteLayoutRules::CountInBases(const TConstArrayView<FHOLPlacedSite> Sites, const TConstArrayView<FVector2D> SlotCentres, const float BaseReach)
{
	int32 Count = 0;
	for (const FHOLPlacedSite& Site : Sites)
	{
		for (const FVector2D& Slot : SlotCentres)
		{
			if (FVector2D::Distance(FVector2D(Site.Location), Slot) < BaseReach + Site.Radius)
			{
				++Count;
				break;
			}
		}
	}
	return Count;
}

int32 FHOLSiteLayoutRules::CountOutside(const TConstArrayView<FHOLPlacedSite> Sites, const double HalfSize)
{
	int32 Count = 0;
	for (const FHOLPlacedSite& Site : Sites)
	{
		Count += FMath::Abs(Site.Location.X) + Site.Radius > HalfSize || FMath::Abs(Site.Location.Y) + Site.Radius > HalfSize;
	}
	return Count;
}

int32 FHOLSiteLayoutRules::CountBadRadius(const TConstArrayView<FHOLPlacedSite> Sites)
{
	int32 Count = 0;
	for (const FHOLPlacedSite& Site : Sites)
	{
		Count += !(Site.Radius > 0.f);
	}
	return Count;
}

int32 FHOLSiteLayoutRules::CountNearRelays(const TConstArrayView<FHOLPlacedSite> Sites, const TConstArrayView<FHOLPlacedRelay> Relays, const float Margin)
{
	int32 Count = 0;
	for (const FHOLPlacedSite& Site : Sites)
	{
		for (const FHOLPlacedRelay& Relay : Relays)
		{
			if (FVector2D::Distance(FVector2D(Site.Location), FVector2D(Relay.Location)) < Site.Radius + Relay.Radius + Margin)
			{
				++Count;
				break;
			}
		}
	}
	return Count;
}

int32 FHOLSiteLayoutRules::CountNearPairs(const TConstArrayView<FHOLPlacedSite> Sites, const float Margin)
{
	int32 Count = 0;
	for (int32 A = 0; A < Sites.Num(); ++A)
	{
		for (int32 B = A + 1; B < Sites.Num(); ++B)
		{
			Count += FVector2D::Distance(FVector2D(Sites[A].Location), FVector2D(Sites[B].Location)) < Sites[A].Radius + Sites[B].Radius + Margin ? 1 : 0;
		}
	}
	return Count;
}
