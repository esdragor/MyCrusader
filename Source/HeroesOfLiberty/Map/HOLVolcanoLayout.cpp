// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLVolcanoLayout.h"

AHOLVolcanoLayout::AHOLVolcanoLayout()
{
	PrimaryActorTick.bCanEverTick = false;
	// Data for the server: the sites are the simulation's.
	bNetLoadOnClient = false;
	SetCanBeDamaged(false);
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

int32 FHOLVolcanoLayoutRules::CountUnbalancedFlows(const TConstArrayView<FHOLPlacedVolcano> Sites, const double StepDegrees)
{
	const double Radians = FMath::DegreesToRadians(StepDegrees);
	const double Cos = FMath::Cos(Radians);
	const double Sin = FMath::Sin(Radians);
	int32 Unmatched = 0;
	for (const FHOLPlacedVolcano& Site : Sites)
	{
		for (const FHOLPlacedLavaFlow& Flow : Site.Flows)
		{
			bool bMatched = false;
			for (const FHOLPlacedLavaFlow& Other : Site.Flows)
			{
				bool bSame = Other.Points.Num() == Flow.Points.Num();
				for (int32 Index = 0; bSame && Index < Flow.Points.Num(); ++Index)
				{
					const double TurnedX = Flow.Points[Index].X * Cos - Flow.Points[Index].Y * Sin;
					const double TurnedY = Flow.Points[Index].X * Sin + Flow.Points[Index].Y * Cos;
					bSame = FMath::Abs(Other.Points[Index].X - TurnedX) <= SymmetryTolerance && FMath::Abs(Other.Points[Index].Y - TurnedY) <= SymmetryTolerance;
				}
				if (bSame)
				{
					bMatched = true;
					break;
				}
			}
			Unmatched += bMatched ? 0 : 1;
		}
	}
	return Unmatched;
}

double FHOLVolcanoLayoutRules::AltarSpread(const FHOLPlacedVolcano& Site, const TConstArrayView<FVector2D> SlotCentres)
{
	if (!Site.bAltar || SlotCentres.Num() < 2)
	{
		return 0.;
	}
	double Nearest = TNumericLimits<double>::Max();
	double Farthest = 0.;
	for (const FVector2D& Slot : SlotCentres)
	{
		const double Distance = FVector2D::Distance(FVector2D(Site.AltarLocation.X, Site.AltarLocation.Y), Slot);
		Nearest = FMath::Min(Nearest, Distance);
		Farthest = FMath::Max(Farthest, Distance);
	}
	return Farthest - Nearest;
}

int32 FHOLVolcanoLayoutRules::CountUnfairAltars(const TConstArrayView<FHOLPlacedVolcano> Sites, const TConstArrayView<FVector2D> SlotCentres)
{
	int32 Count = 0;
	for (const FHOLPlacedVolcano& Site : Sites)
	{
		Count += AltarSpread(Site, SlotCentres) > SymmetryTolerance ? 1 : 0;
	}
	return Count;
}

int32 FHOLVolcanoLayoutRules::CountEmptyFlows(const TConstArrayView<FHOLPlacedVolcano> Sites)
{
	int32 Count = 0;
	for (const FHOLPlacedVolcano& Site : Sites)
	{
		Count += Site.Flows.IsEmpty() ? 1 : 0;
		for (const FHOLPlacedLavaFlow& Flow : Site.Flows)
		{
			HOLVolcano::FFlow Converted;
			Converted.Points = Flow.Points;
			Count += Converted.Length() > 0.f ? 0 : 1;
		}
	}
	return Count;
}

int32 FHOLVolcanoLayoutRules::CountOutside(const TConstArrayView<FHOLPlacedVolcano> Sites, const double HalfSize)
{
	int32 Count = 0;
	for (const FHOLPlacedVolcano& Site : Sites)
	{
		for (const FHOLPlacedLavaFlow& Flow : Site.Flows)
		{
			for (const FVector& Point : Flow.Points)
			{
				Count += FMath::Abs(Point.X) > HalfSize || FMath::Abs(Point.Y) > HalfSize ? 1 : 0;
			}
		}
		if (Site.bAltar)
		{
			Count += FMath::Abs(Site.AltarLocation.X) + Site.AltarRadius > HalfSize || FMath::Abs(Site.AltarLocation.Y) + Site.AltarRadius > HalfSize ? 1 : 0;
		}
	}
	return Count;
}

TArray<HOLVolcano::FSite> FHOLVolcanoLayoutRules::ToSites(const TConstArrayView<FHOLPlacedVolcano> Placed)
{
	TArray<HOLVolcano::FSite> Sites;
	for (const FHOLPlacedVolcano& Site : Placed)
	{
		HOLVolcano::FSite& Converted = Sites.AddDefaulted_GetRef();
		for (const FHOLPlacedLavaFlow& Flow : Site.Flows)
		{
			Converted.Flows.AddDefaulted_GetRef().Points = Flow.Points;
		}
		Converted.Altar.bPresent = Site.bAltar;
		Converted.Altar.Location = Site.AltarLocation;
		Converted.Altar.Radius = Site.AltarRadius;
	}
	return Sites;
}
