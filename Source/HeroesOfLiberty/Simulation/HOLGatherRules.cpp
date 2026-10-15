// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLGatherRules.h"

int32 HOLGather::StartGathering(FHOLGatherLoad& Load, const EHOLResource Resource)
{
	if (Load.Resource == Resource)
	{
		return 0;
	}
	const int32 Lost = Load.Amount;
	Load.Resource = Resource;
	Load.Amount = 0;
	Load.Progress = 0.f;
	return Lost;
}

int32 HOLGather::Gather(FHOLGatherLoad& Load, const float Rate, const float StepSeconds, const int32 Capacity, int32& SourceRemaining)
{
	if (Rate <= 0.f || StepSeconds <= 0.f)
	{
		return 0;
	}
	int32 Taken = 0;
	Load.Progress += Rate * StepSeconds;
	while (Load.Progress >= 1.f && Load.Amount < Capacity && SourceRemaining > 0)
	{
		Load.Progress -= 1.f;
		++Load.Amount;
		--SourceRemaining;
		++Taken;
	}
	if (Load.Amount >= Capacity || SourceRemaining <= 0)
	{
		Load.Progress = 0.f;
	}
	return Taken;
}

bool HOLGather::IsFull(const FHOLGatherLoad& Load, const int32 Capacity)
{
	return Load.Amount >= Capacity;
}

int32 HOLGather::Deposit(FHOLGatherLoad& Load, FHOLResources& Stock)
{
	const int32 Amount = Load.Amount;
	Stock[Load.Resource] += Amount;
	Load.Amount = 0;
	Load.Progress = 0.f;
	return Amount;
}

double HOLGather::GetEdgeDistance(const FVector2D& Position, const FHOLGatherSite& Site)
{
	return FVector2D::Distance(Position, Site.Position) - Site.Radius;
}

bool HOLGather::IsInContact(const FVector2D& Position, const float BodyRadius, const FHOLGatherSite& Site)
{
	return GetEdgeDistance(Position, Site) - BodyRadius <= ContactGap;
}

int32 HOLGather::FindNearestDeposit(const FVector2D& Position, TConstArrayView<FHOLGatherSite> Deposits)
{
	int32 Nearest = INDEX_NONE;
	double NearestDistance = TNumericLimits<double>::Max();
	for (int32 Index = 0; Index < Deposits.Num(); ++Index)
	{
		const double Distance = GetEdgeDistance(Position, Deposits[Index]);
		if (Distance < NearestDistance)
		{
			Nearest = Index;
			NearestDistance = Distance;
		}
	}
	return Nearest;
}

int32 HOLGather::FindNeighbourSource(const FVector2D& From, const EHOLResource Resource, TConstArrayView<FHOLGatherSource> Sources,
	const double MaxDistance)
{
	int32 Nearest = INDEX_NONE;
	double NearestDistanceSquared = FMath::Square(MaxDistance);
	for (int32 Index = 0; Index < Sources.Num(); ++Index)
	{
		const FHOLGatherSource& Source = Sources[Index];
		if (Source.Resource != Resource || Source.Remaining <= 0)
		{
			continue;
		}
		const double DistanceSquared = FVector2D::DistSquared(From, Source.Site.Position);
		if (DistanceSquared <= NearestDistanceSquared && (Nearest == INDEX_NONE || DistanceSquared < NearestDistanceSquared))
		{
			Nearest = Index;
			NearestDistanceSquared = DistanceSquared;
		}
	}
	return Nearest;
}
