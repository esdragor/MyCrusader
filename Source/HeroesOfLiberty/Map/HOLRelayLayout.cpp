// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLRelayLayout.h"

AHOLRelayLayout::AHOLRelayLayout()
{
	PrimaryActorTick.bCanEverTick = false;
	// Data for the server: the relays are the simulation's (a client receives their views, R5).
	bNetLoadOnClient = false;
	SetCanBeDamaged(false);
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

int32 FHOLRelayLayoutRules::CountUnmatchedByTurn(const TConstArrayView<FHOLPlacedRelay> Relays, const double StepDegrees)
{
	const double Radians = FMath::DegreesToRadians(StepDegrees);
	const double Cos = FMath::Cos(Radians);
	const double Sin = FMath::Sin(Radians);
	int32 Unmatched = 0;
	for (const FHOLPlacedRelay& Relay : Relays)
	{
		const double TurnedX = Relay.Location.X * Cos - Relay.Location.Y * Sin;
		const double TurnedY = Relay.Location.X * Sin + Relay.Location.Y * Cos;
		bool bMatched = false;
		for (const FHOLPlacedRelay& Other : Relays)
		{
			if (FMath::Abs(Other.Location.X - TurnedX) <= SymmetryTolerance && FMath::Abs(Other.Location.Y - TurnedY) <= SymmetryTolerance
				&& FMath::Abs(Other.Radius - Relay.Radius) <= SymmetryTolerance)
			{
				bMatched = true;
				break;
			}
		}
		Unmatched += !bMatched;
	}
	return Unmatched;
}

int32 FHOLRelayLayoutRules::CountInBases(const TConstArrayView<FHOLPlacedRelay> Relays, const TConstArrayView<FVector2D> SlotCentres, const float BaseReach)
{
	int32 Count = 0;
	for (const FHOLPlacedRelay& Relay : Relays)
	{
		for (const FVector2D& Slot : SlotCentres)
		{
			if (FVector2D::Distance(FVector2D(Relay.Location.X, Relay.Location.Y), Slot) < BaseReach + Relay.Radius)
			{
				++Count;
				break;
			}
		}
	}
	return Count;
}

int32 FHOLRelayLayoutRules::CountOutside(const TConstArrayView<FHOLPlacedRelay> Relays, const double HalfSize)
{
	int32 Count = 0;
	for (const FHOLPlacedRelay& Relay : Relays)
	{
		Count += FMath::Abs(Relay.Location.X) + Relay.Radius > HalfSize || FMath::Abs(Relay.Location.Y) + Relay.Radius > HalfSize;
	}
	return Count;
}

int32 FHOLRelayLayoutRules::CountBadRadius(const TConstArrayView<FHOLPlacedRelay> Relays)
{
	int32 Count = 0;
	for (const FHOLPlacedRelay& Relay : Relays)
	{
		Count += !(Relay.Radius > 0.f);
	}
	return Count;
}
