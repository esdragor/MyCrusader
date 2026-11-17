// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLGatherReadLibrary.h"

#include "Match/HOLResources.h"
#include "Simulation/HOLGatherRules.h"

namespace
{
	EHOLResource ToResource(const int32 Resource)
	{
		return Resource >= 0 && Resource <= static_cast<int32>(EHOLResource::Gold) ? static_cast<EHOLResource>(Resource) : EHOLResource::Food;
	}

	FHOLGatherLoad ToLoad(const FHOLGatherLoadScript& Script)
	{
		FHOLGatherLoad Load;
		Load.Resource = ToResource(Script.Resource);
		Load.Amount = Script.Amount;
		Load.Progress = Script.Progress;
		Load.ProgressResource = ToResource(Script.ProgressResource);
		return Load;
	}

	FHOLGatherLoadScript ToScript(const FHOLGatherLoad& Load)
	{
		FHOLGatherLoadScript Script;
		Script.Resource = static_cast<int32>(Load.Resource);
		Script.Amount = Load.Amount;
		Script.Progress = Load.Progress;
		Script.ProgressResource = static_cast<int32>(Load.ProgressResource);
		return Script;
	}

	FHOLGatherDeposit MakeDeposit(const FVector2D& Centre, const FVector2D& HalfExtent, const double Yaw)
	{
		FHOLGatherDeposit Deposit;
		Deposit.Centre = Centre;
		Deposit.HalfExtent = HalfExtent;
		Deposit.Yaw = static_cast<float>(Yaw);
		return Deposit;
	}

	FHOLGatherSite MakeSite(const FVector2D& Centre, const double Radius)
	{
		FHOLGatherSite Site;
		Site.Position = Centre;
		Site.Radius = static_cast<float>(Radius);
		return Site;
	}

	void BuildMassifs(FHOLSourceMassifs& Massifs, const TArray<FVector2D>& Positions, const TArray<int32>& Resources, const double LinkDistance, const double NearbyDistance)
	{
		TArray<FHOLGatherSource> Sources;
		Sources.SetNum(Positions.Num());
		for (int32 Index = 0; Index < Positions.Num(); ++Index)
		{
			Sources[Index].Site.Position = Positions[Index];
			Sources[Index].Resource = ToResource(Resources.IsValidIndex(Index) ? Resources[Index] : 0);
		}
		Massifs.Build(Sources, LinkDistance, NearbyDistance);
	}
}

double UHOLGatherReadLibrary::GetNoRoomSourceDistance() { return HOLGather::NoRoomSourceDistance; }

double UHOLGatherReadLibrary::GetNearbyMassifDistance() { return HOLGather::NearbyMassifDistance; }

FHOLGatherLoadScript UHOLGatherReadLibrary::StartGathering(const FHOLGatherLoadScript& Load, const int32 Resource)
{
	FHOLGatherLoad Result = ToLoad(Load);
	HOLGather::StartGathering(Result, ToResource(Resource));
	return ToScript(Result);
}

FHOLGatherLoadScript UHOLGatherReadLibrary::GatherSteps(const FHOLGatherLoadScript& Load, const int32 Resource, const double Rate, const double StepSeconds,
	const int32 Steps, const int32 Capacity, const int32 SourceRemaining, int32& OutTaken, int32& OutLost, int32& OutSourceRemaining)
{
	FHOLGatherLoad Result = ToLoad(Load);
	int32 Remaining = SourceRemaining;
	OutTaken = 0;
	OutLost = 0;
	for (int32 Step = 0; Step < Steps; ++Step)
	{
		OutTaken += HOLGather::Gather(Result, ToResource(Resource), static_cast<float>(Rate), static_cast<float>(StepSeconds), Capacity, Remaining, &OutLost);
	}
	OutSourceRemaining = Remaining;
	return ToScript(Result);
}

bool UHOLGatherReadLibrary::IsFull(const FHOLGatherLoadScript& Load, const int32 Capacity) { return HOLGather::IsFull(ToLoad(Load), Capacity); }

bool UHOLGatherReadLibrary::IsFullOf(const FHOLGatherLoadScript& Load, const int32 Resource, const int32 Capacity)
{
	return HOLGather::IsFullOf(ToLoad(Load), ToResource(Resource), Capacity);
}

FHOLGatherLoadScript UHOLGatherReadLibrary::Deposit(const FHOLGatherLoadScript& Load, const TArray<int32>& Stock, int32& OutDeposited, TArray<int32>& OutStock)
{
	FHOLGatherLoad Result = ToLoad(Load);
	FHOLResources Resources(Stock.IsValidIndex(0) ? Stock[0] : 0, Stock.IsValidIndex(1) ? Stock[1] : 0, Stock.IsValidIndex(2) ? Stock[2] : 0,
		Stock.IsValidIndex(3) ? Stock[3] : 0);
	OutDeposited = HOLGather::Deposit(Result, Resources);
	OutStock = { Resources.Food, Resources.Wood, Resources.Stone, Resources.Gold };
	return ToScript(Result);
}

double UHOLGatherReadLibrary::GetEdgeDistance(const FVector2D& Position, const FVector2D& SiteCentre, const double SiteRadius)
{
	return HOLGather::GetEdgeDistance(Position, MakeSite(SiteCentre, SiteRadius));
}

bool UHOLGatherReadLibrary::IsInContact(const FVector2D& Position, const double BodyRadius, const FVector2D& SiteCentre, const double SiteRadius)
{
	return HOLGather::IsInContact(Position, static_cast<float>(BodyRadius), MakeSite(SiteCentre, SiteRadius));
}

double UHOLGatherReadLibrary::GetDepositDistance(const FVector2D& Position, const FVector2D& Centre, const FVector2D& HalfExtent, const double Yaw)
{
	return HOLGather::GetDepositDistance(Position, MakeDeposit(Centre, HalfExtent, Yaw));
}

bool UHOLGatherReadLibrary::IsAtDeposit(const FVector2D& Position, const double BodyRadius, const FVector2D& Centre, const FVector2D& HalfExtent, const double Yaw)
{
	return HOLGather::IsAtDeposit(Position, static_cast<float>(BodyRadius), MakeDeposit(Centre, HalfExtent, Yaw));
}

FVector2D UHOLGatherReadLibrary::GetDepositSpot(const FVector2D& From, const FVector2D& Centre, const FVector2D& HalfExtent, const double Yaw, const double Gap,
	const double Along)
{
	return HOLGather::GetDepositSpot(From, MakeDeposit(Centre, HalfExtent, Yaw), Gap, Along);
}

int32 UHOLGatherReadLibrary::FindNearestDeposit(const FVector2D& Position, const TArray<FVector2D>& Centres, const TArray<FVector2D>& HalfExtents, const TArray<float>& Yaws)
{
	TArray<FHOLGatherDeposit> Deposits;
	for (int32 Index = 0; Index < Centres.Num(); ++Index)
	{
		Deposits.Add(MakeDeposit(Centres[Index], HalfExtents.IsValidIndex(Index) ? HalfExtents[Index] : FVector2D::ZeroVector, Yaws.IsValidIndex(Index) ? Yaws[Index] : 0.f));
	}
	return HOLGather::FindNearestDeposit(Position, Deposits);
}

int32 UHOLGatherReadLibrary::GetSanctuaryFarmPercent(const TArray<FVector2D>& Sanctuaries, const FVector2D& Field)
{
	return HOLGather::GetSanctuaryFarmPercent(Sanctuaries, Field);
}

void UHOLGatherReadLibrary::GetSourceSpots(const FVector2D& Centre, const double Distance, const double Spacing, const FVector2D& Towards, TArray<FVector2D>& OutSpots)
{
	HOLGather::GetSourceSpots(Centre, Distance, Spacing, Towards, OutSpots);
}

void UHOLGatherReadLibrary::GetStandPoints(const FVector2D& Centre, const double SpotDistance, const double Spacing, const FVector2D& Towards, const int32 Rows,
	const int32 FirstRow, TArray<FVector2D>& OutPoints)
{
	HOLGather::GetStandPoints(Centre, SpotDistance, Spacing, Towards, Rows, OutPoints, FirstRow);
}

bool UHOLGatherReadLibrary::IsClearOfSpots(const FVector2D& Point, const TArray<FVector2D>& SitePositions, const TArray<float>& SiteRadii, const double SpotOffset,
	const double Spacing)
{
	TArray<FHOLGatherSite> Sites;
	for (int32 Index = 0; Index < SitePositions.Num(); ++Index)
	{
		Sites.Add(MakeSite(SitePositions[Index], SiteRadii.IsValidIndex(Index) ? SiteRadii[Index] : 0.));
	}
	return HOLGather::IsClearOfSpots(Point, Sites, SpotOffset, Spacing);
}

bool UHOLGatherReadLibrary::IsSpotTaken(const TArray<int32>& ClaimOwners, const TArray<FVector2D>& ClaimSpots, const FVector2D& Spot, const double MinSpacing,
	const int32 Owner, const TArray<int32>& StaleOwners, int32& OutClaimsLeft, int32& OutStaleAsked)
{
	FHOLSpotClaims Claims;
	for (int32 Index = 0; Index < ClaimSpots.Num(); ++Index)
	{
		Claims.Add(static_cast<uint64>(ClaimOwners.IsValidIndex(Index) ? ClaimOwners[Index] : 0), 0, ClaimSpots[Index]);
	}
	int32 StaleAsked = 0;
	const bool bTaken = Claims.IsTaken(Spot, MinSpacing, static_cast<uint64>(Owner), [&StaleOwners, &StaleAsked](const FHOLSpotClaims::FClaim& Claim)
	{
		++StaleAsked;
		return StaleOwners.Contains(static_cast<int32>(Claim.Owner));
	});
	OutClaimsLeft = Claims.Num();
	OutStaleAsked = StaleAsked;
	return bTaken;
}

void UHOLGatherReadLibrary::GetMassifs(const TArray<FVector2D>& Positions, const TArray<int32>& Resources, const double LinkDistance, const double NearbyDistance,
	TArray<int32>& OutMassifOf)
{
	FHOLSourceMassifs Massifs;
	BuildMassifs(Massifs, Positions, Resources, LinkDistance, NearbyDistance);
	OutMassifOf.Reset();
	for (int32 Index = 0; Index < Positions.Num(); ++Index)
	{
		OutMassifOf.Add(Massifs.GetMassif(Index));
	}
}

int32 UHOLGatherReadLibrary::GetLinkCount(const TArray<FVector2D>& Positions, const TArray<int32>& Resources, const double LinkDistance, const double NearbyDistance,
	const int32 Index)
{
	FHOLSourceMassifs Massifs;
	BuildMassifs(Massifs, Positions, Resources, LinkDistance, NearbyDistance);
	return Positions.IsValidIndex(Index) ? Massifs.GetLinks(Index).Num() : 0;
}

int32 UHOLGatherReadLibrary::GetNearbyMassifCount(const TArray<FVector2D>& Positions, const TArray<int32>& Resources, const double LinkDistance,
	const double NearbyDistance, const int32 Massif)
{
	FHOLSourceMassifs Massifs;
	BuildMassifs(Massifs, Positions, Resources, LinkDistance, NearbyDistance);
	return Massifs.GetNearbyMassifs(Massif).Num();
}

int32 UHOLGatherReadLibrary::SearchMassif(const TArray<FVector2D>& Positions, const TArray<int32>& Resources, const double LinkDistance, const double NearbyDistance,
	const int32 Start, const TArray<int32>& Accepted, const bool bHasDeposit, const FVector2D& DepositCentre, const FVector2D& DepositHalfExtent,
	const double DepositYaw, TArray<int32>& OutTried)
{
	FHOLSourceMassifs Massifs;
	BuildMassifs(Massifs, Positions, Resources, LinkDistance, NearbyDistance);
	OutTried.Reset();
	const FHOLGatherDeposit Deposit = MakeDeposit(DepositCentre, DepositHalfExtent, DepositYaw);
	return Massifs.Search(Start, [&OutTried, &Accepted](const int32 Index)
	{
		OutTried.Add(Index);
		return Accepted.Contains(Index);
	}, bHasDeposit ? &Deposit : nullptr);
}
