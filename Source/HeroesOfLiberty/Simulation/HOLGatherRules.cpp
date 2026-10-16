// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLGatherRules.h"

void HOLGather::StartGathering(FHOLGatherLoad& Load, const EHOLResource Resource)
{
	if (Load.Amount == 0)
	{
		Load.Resource = Resource;
	}
}

int32 HOLGather::Gather(FHOLGatherLoad& Load, const EHOLResource Resource, const float Rate, const float StepSeconds, const int32 Capacity,
	int32& SourceRemaining, int32* OutLost)
{
	if (Rate <= 0.f || StepSeconds <= 0.f)
	{
		return 0;
	}
	StartGathering(Load, Resource);
	if (Load.ProgressResource != Resource)
	{
		Load.ProgressResource = Resource;
		Load.Progress = 0.f;
	}
	int32 Taken = 0;
	Load.Progress += Rate * StepSeconds;
	while (Load.Progress >= 1.f && (Load.Resource != Resource || Load.Amount < Capacity) && SourceRemaining > 0)
	{
		if (Load.Resource != Resource)
		{
			// Its first unit: what it carried is lost (D306).
			if (OutLost)
			{
				*OutLost += Load.Amount;
			}
			Load.Resource = Resource;
			Load.Amount = 0;
		}
		Load.Progress -= 1.f;
		++Load.Amount;
		--SourceRemaining;
		++Taken;
	}
	if (IsFullOf(Load, Resource, Capacity) || SourceRemaining <= 0)
	{
		Load.Progress = 0.f;
	}
	return Taken;
}

bool HOLGather::IsFull(const FHOLGatherLoad& Load, const int32 Capacity)
{
	return Load.Amount >= Capacity;
}

bool HOLGather::IsFullOf(const FHOLGatherLoad& Load, const EHOLResource Resource, const int32 Capacity)
{
	return Load.Resource == Resource && IsFull(Load, Capacity);
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

namespace UE::HeroesOfLiberty::Gather
{
	/** Position in the deposit's frame (X, Y along its footprint's sides). */
	FVector2D ToLocal(const FVector2D& Position, const FHOLGatherDeposit& Deposit)
	{
		return (Position - Deposit.Centre).GetRotated(-Deposit.Yaw);
	}
}

double HOLGather::GetDepositDistance(const FVector2D& Position, const FHOLGatherDeposit& Deposit)
{
	const FVector2D Local = UE::HeroesOfLiberty::Gather::ToLocal(Position, Deposit);
	const FVector2D Outside(FMath::Max(FMath::Abs(Local.X) - Deposit.HalfExtent.X, 0.), FMath::Max(FMath::Abs(Local.Y) - Deposit.HalfExtent.Y, 0.));
	return Outside.Size();
}

bool HOLGather::IsAtDeposit(const FVector2D& Position, const float BodyRadius, const FHOLGatherDeposit& Deposit)
{
	return GetDepositDistance(Position, Deposit) - BodyRadius <= DepositGap;
}

FVector2D HOLGather::GetDepositSpot(const FVector2D& From, const FHOLGatherDeposit& Deposit, const double Gap, const double Along)
{
	const FVector2D Local = UE::HeroesOfLiberty::Gather::ToLocal(From, Deposit);
	const FVector2D& Half = Deposit.HalfExtent;
	// The side it faces: the one it stands farthest out of (inside: the nearest side).
	const double OutX = FMath::Abs(Local.X) - Half.X;
	const double OutY = FMath::Abs(Local.Y) - Half.Y;
	FVector2D Spot;
	if (OutX >= OutY)
	{
		const double Side = Local.X >= 0. ? 1. : -1.;
		Spot = FVector2D(Side * (Half.X + Gap), FMath::Clamp(Local.Y + Along, -Half.Y, Half.Y));
	}
	else
	{
		const double Side = Local.Y >= 0. ? 1. : -1.;
		Spot = FVector2D(FMath::Clamp(Local.X + Along, -Half.X, Half.X), Side * (Half.Y + Gap));
	}
	return Deposit.Centre + Spot.GetRotated(Deposit.Yaw);
}

int32 HOLGather::FindNearestDeposit(const FVector2D& Position, TConstArrayView<FHOLGatherDeposit> Deposits)
{
	int32 Nearest = INDEX_NONE;
	double NearestDistance = TNumericLimits<double>::Max();
	for (int32 Index = 0; Index < Deposits.Num(); ++Index)
	{
		const double Distance = GetDepositDistance(Position, Deposits[Index]);
		if (Distance < NearestDistance)
		{
			Nearest = Index;
			NearestDistance = Distance;
		}
	}
	return Nearest;
}

void HOLGather::GetSourceSpots(const FVector2D& Centre, const double Distance, const double Spacing, const FVector2D& Towards, TArray<FVector2D>& OutSpots)
{
	OutSpots.Reset();
	if (Distance <= 0.)
	{
		OutSpots.Add(Centre);
		return;
	}
	const FVector2D Facing = Towards.GetSafeNormal();
	const double BaseAngle = Facing.IsZero() ? 0. : FMath::Atan2(Facing.Y, Facing.X);
	// As many as the circle holds Spacing apart, spread evenly around it: no two closer at the end of the turn.
	const double MinStep = 2. * FMath::Asin(FMath::Min(1., (FMath::Max(Spacing, 0.) + 1.) / (2. * Distance)));
	const int32 Count = FMath::Max(1, FMath::FloorToInt32(UE_DOUBLE_TWO_PI / FMath::Max(MinStep, UE_DOUBLE_KINDA_SMALL_NUMBER) + 1e-6));
	const double Step = UE_DOUBLE_TWO_PI / Count;
	OutSpots.Reserve(Count);
	for (int32 Try = 0; Try < Count; ++Try)
	{
		// 0, +1, -1, +2, -2... steps from the side facing Towards.
		const int32 Turn = (Try + 1) / 2;
		const double Angle = BaseAngle + (Try % 2 == 1 ? 1. : -1.) * Turn * Step;
		OutSpots.Add(Centre + Distance * FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)));
	}
}

void HOLGather::GetStandPoints(const FVector2D& Centre, const double SpotDistance, const double Spacing, const FVector2D& Towards, const int32 Rows,
	TArray<FVector2D>& OutPoints, const int32 FirstRow)
{
	OutPoints.Reset();
	if (Spacing <= 0.)
	{
		return;
	}
	const FVector2D Facing = Towards.GetSafeNormal();
	const double BaseAngle = Facing.IsZero() ? 0. : FMath::Atan2(Facing.Y, Facing.X);
	// 1 cm more than the spacing: two points side by side are not taken for too close by a rounding.
	const double Step = Spacing + 1.;
	for (int32 Row = FMath::Max(FirstRow, 0); Row < FirstRow + Rows; ++Row)
	{
		const double Distance = SpotDistance + (Row + 1) * Step;
		const double AngleStep = 2. * FMath::Asin(FMath::Min(1., Step / (2. * Distance)));
		// 0, +1, -1, +2, -2... steps, up to a quarter turn each way.
		const int32 MaxTurn = FMath::FloorToInt32(UE_DOUBLE_HALF_PI / AngleStep + 1e-6);
		for (int32 Try = 0; Try <= 2 * MaxTurn; ++Try)
		{
			const int32 Turn = (Try + 1) / 2;
			const double Angle = BaseAngle + (Try % 2 == 1 ? 1. : -1.) * Turn * AngleStep;
			OutPoints.Add(Centre + Distance * FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)));
		}
	}
}

bool HOLGather::IsClearOfSpots(const FVector2D& Point, TConstArrayView<FHOLGatherSite> Sites, const double SpotOffset, const double Spacing)
{
	for (const FHOLGatherSite& Site : Sites)
	{
		// Its spots lie on a circle: the nearest one is as far as that circle (1 cm of rounding allowed).
		if (FMath::Abs(FVector2D::Distance(Point, Site.Position) - (Site.Radius + SpotOffset)) < Spacing - 1.)
		{
			return false;
		}
	}
	return true;
}

FIntPoint FHOLSpotClaims::GetCell(const FVector2D& Spot)
{
	return FIntPoint(FMath::FloorToInt32(Spot.X / CellSize), FMath::FloorToInt32(Spot.Y / CellSize));
}

void FHOLSpotClaims::Add(const uint64 Owner, const int32 SourceIndex, const FVector2D& Spot)
{
	Cells.FindOrAdd(GetCell(Spot)).Add({ Owner, SourceIndex, Spot });
}

bool FHOLSpotClaims::IsTaken(const FVector2D& Spot, const double MinSpacing, const uint64 Owner, TFunctionRef<bool(const FClaim&)> IsStale)
{
	// Cells as wide as the spacing: those around Spot's cell hold every spot close enough.
	const int32 Reach = FMath::Max(1, FMath::CeilToInt32(MinSpacing / CellSize));
	const FIntPoint Centre = GetCell(Spot);
	const double MinSpacingSquared = MinSpacing * MinSpacing;
	for (int32 Y = -Reach; Y <= Reach; ++Y)
	{
		for (int32 X = -Reach; X <= Reach; ++X)
		{
			TArray<FClaim>* Claims = Cells.Find(Centre + FIntPoint(X, Y));
			if (!Claims)
			{
				continue;
			}
			for (int32 Index = Claims->Num() - 1; Index >= 0; --Index)
			{
				const FClaim& Claim = (*Claims)[Index];
				const bool bNear = FVector2D::DistSquared(Claim.Spot, Spot) < MinSpacingSquared;
				// Staleness asked of the near ones only: the others stay until a look close to them.
				if (Claim.Owner == Owner || (bNear && IsStale(Claim)))
				{
					Claims->RemoveAtSwap(Index, EAllowShrinking::No);
				}
				else if (bNear)
				{
					return true;
				}
			}
		}
	}
	return false;
}

int32 FHOLSpotClaims::Num() const
{
	int32 Total = 0;
	for (const TPair<FIntPoint, TArray<FClaim>>& Cell : Cells)
	{
		Total += Cell.Value.Num();
	}
	return Total;
}

void FHOLSourceMassifs::Build(TConstArrayView<FHOLGatherSource> Sources, const double LinkDistance, const double NearbyDistance)
{
	Positions.Reset(Sources.Num());
	FirstLinks.Reset(Sources.Num() + 1);
	Links.Reset();
	for (const FHOLGatherSource& Source : Sources)
	{
		Positions.Add(Source.Site.Position);
	}
	// Cells as wide as a link: a source's links lie in the 3 x 3 cells around its own.
	const double CellSize = FMath::Max(LinkDistance, 1.);
	const auto GetCell = [](const FVector2D& Position, const double Size)
	{
		return FIntPoint(FMath::FloorToInt32(Position.X / Size), FMath::FloorToInt32(Position.Y / Size));
	};
	TMap<FIntPoint, TArray<int32>> Cells;
	for (int32 Index = 0; Index < Sources.Num(); ++Index)
	{
		Cells.FindOrAdd(GetCell(Positions[Index], CellSize)).Add(Index);
	}
	const double LinkDistanceSquared = LinkDistance * LinkDistance;
	for (int32 Index = 0; Index < Sources.Num(); ++Index)
	{
		FirstLinks.Add(Links.Num());
		const FIntPoint Cell = GetCell(Positions[Index], CellSize);
		for (int32 Y = -1; Y <= 1; ++Y)
		{
			for (int32 X = -1; X <= 1; ++X)
			{
				const TArray<int32>* Others = Cells.Find(Cell + FIntPoint(X, Y));
				if (!Others)
				{
					continue;
				}
				for (const int32 Other : *Others)
				{
					if (Other != Index && Sources[Other].Resource == Sources[Index].Resource
						&& FVector2D::DistSquared(Positions[Other], Positions[Index]) <= LinkDistanceSquared)
					{
						Links.Add(Other);
					}
				}
			}
		}
	}
	FirstLinks.Add(Links.Num());

	// The massifs: the sources linked step by step, each massif numbered in the order of its first source.
	MassifOf.Init(INDEX_NONE, Sources.Num());
	int32 NumMassifs = 0;
	TArray<int32> Pending;
	for (int32 Index = 0; Index < Sources.Num(); ++Index)
	{
		if (MassifOf[Index] != INDEX_NONE)
		{
			continue;
		}
		MassifOf[Index] = NumMassifs;
		Pending.Reset();
		Pending.Add(Index);
		while (Pending.Num() > 0)
		{
			for (const int32 Link : GetLinks(Pending.Pop(EAllowShrinking::No)))
			{
				if (MassifOf[Link] == INDEX_NONE)
				{
					MassifOf[Link] = NumMassifs;
					Pending.Add(Link);
				}
			}
		}
		++NumMassifs;
	}
	FirstMassifSources.Init(0, NumMassifs + 1);
	for (const int32 Massif : MassifOf)
	{
		++FirstMassifSources[Massif + 1];
	}
	for (int32 Massif = 0; Massif < NumMassifs; ++Massif)
	{
		FirstMassifSources[Massif + 1] += FirstMassifSources[Massif];
	}
	MassifSources.SetNumUninitialized(Sources.Num());
	{
		TArray<int32> Filled(FirstMassifSources.GetData(), NumMassifs);
		for (int32 Index = 0; Index < Sources.Num(); ++Index)
		{
			MassifSources[Filled[MassifOf[Index]]++] = Index;
		}
	}

	// The nearby groves (D309 point 6): built once too, through cells as wide as their distance (every pair of sources
	// close enough lies in 3 x 3 cells), each pair of massifs kept once.
	TArray<TArray<int32>> Nearby;
	Nearby.SetNum(NumMassifs);
	if (NearbyDistance > 0.)
	{
		TMap<FIntPoint, TArray<int32>> NearbyCells;
		for (int32 Index = 0; Index < Sources.Num(); ++Index)
		{
			NearbyCells.FindOrAdd(GetCell(Positions[Index], NearbyDistance)).Add(Index);
		}
		const double NearbyDistanceSquared = NearbyDistance * NearbyDistance;
		for (int32 Index = 0; Index < Sources.Num(); ++Index)
		{
			const int32 Massif = MassifOf[Index];
			const FIntPoint Cell = GetCell(Positions[Index], NearbyDistance);
			for (int32 Y = -1; Y <= 1; ++Y)
			{
				for (int32 X = -1; X <= 1; ++X)
				{
					const TArray<int32>* Others = NearbyCells.Find(Cell + FIntPoint(X, Y));
					if (!Others)
					{
						continue;
					}
					for (const int32 Other : *Others)
					{
						// Each pair of massifs from its lower one, once.
						const int32 OtherMassif = MassifOf[Other];
						if (OtherMassif > Massif && Sources[Other].Resource == Sources[Index].Resource
							&& !Nearby[Massif].Contains(OtherMassif)
							&& FVector2D::DistSquared(Positions[Other], Positions[Index]) <= NearbyDistanceSquared)
						{
							Nearby[Massif].Add(OtherMassif);
							Nearby[OtherMassif].Add(Massif);
						}
					}
				}
			}
		}
	}
	FirstNearbyMassifs.Reset(NumMassifs + 1);
	NearbyMassifs.Reset();
	for (int32 Massif = 0; Massif < NumMassifs; ++Massif)
	{
		FirstNearbyMassifs.Add(NearbyMassifs.Num());
		NearbyMassifs.Append(Nearby[Massif]);
	}
	FirstNearbyMassifs.Add(NearbyMassifs.Num());
}

TConstArrayView<int32> FHOLSourceMassifs::GetLinks(const int32 Index) const
{
	if (!Positions.IsValidIndex(Index))
	{
		return TConstArrayView<int32>();
	}
	return TConstArrayView<int32>(Links.GetData() + FirstLinks[Index], FirstLinks[Index + 1] - FirstLinks[Index]);
}

TConstArrayView<int32> FHOLSourceMassifs::GetNearbyMassifs(const int32 Massif) const
{
	if (Massif < 0 || Massif + 1 >= FirstNearbyMassifs.Num())
	{
		return TConstArrayView<int32>();
	}
	return TConstArrayView<int32>(NearbyMassifs.GetData() + FirstNearbyMassifs[Massif], FirstNearbyMassifs[Massif + 1] - FirstNearbyMassifs[Massif]);
}

int32 FHOLSourceMassifs::Search(const int32 Start, TFunctionRef<bool(int32)> Accept, const FHOLGatherDeposit* Deposit) const
{
	if (!Positions.IsValidIndex(Start))
	{
		return INDEX_NONE;
	}
	// The sources met so far and not tried yet, the lowest cost first (the first index of a tie): nearest Start, the
	// massif filling from the source clicked outward, as far as the links lead; toward a deposit, the edge facing it
	// first (MassifStartWeight).
	using FMet = TPair<double, int32>;
	const auto Nearer = [](const FMet& A, const FMet& B) { return A.Key < B.Key || (A.Key == B.Key && A.Value < B.Value); };
	TArray<FMet, TInlineAllocator<64>> Met;
	TBitArray<TInlineAllocator<16>> bMet(false, Positions.Num());
	const FVector2D& Origin = Positions[Start];
	const auto GetCost = [this, &Origin, Deposit](const int32 Index)
	{
		const double FromStart = FVector2D::Distance(Positions[Index], Origin);
		return Deposit ? HOLGather::GetDepositDistance(Positions[Index], *Deposit) + HOLGather::MassifStartWeight * FromStart : FromStart;
	};
	const auto Meet = [this, &Met, &bMet, &GetCost, &Nearer](const int32 From)
	{
		for (const int32 Link : GetLinks(From))
		{
			if (!bMet[Link])
			{
				bMet[Link] = true;
				Met.HeapPush(FMet(GetCost(Link), Link), Nearer);
			}
		}
	};
	bMet[Start] = true;
	Meet(Start);
	bool bNearbyMet = false;
	while (Met.Num() > 0 || !bNearbyMet)
	{
		if (Met.Num() == 0)
		{
			// The whole massif refused: the sources of its nearby groves, all of them at once, in the same order (the best
			// grove's best source first, D309 point 6). Their links lead nowhere new: each grove is met whole.
			bNearbyMet = true;
			for (const int32 Grove : GetNearbyMassifs(GetMassif(Start)))
			{
				for (int32 Slot = FirstMassifSources[Grove]; Slot < FirstMassifSources[Grove + 1]; ++Slot)
				{
					const int32 Index = MassifSources[Slot];
					if (!bMet[Index])
					{
						bMet[Index] = true;
						Met.HeapPush(FMet(GetCost(Index), Index), Nearer);
					}
				}
			}
			continue;
		}
		FMet Next;
		Met.HeapPop(Next, Nearer, EAllowShrinking::No);
		if (Accept(Next.Value))
		{
			return Next.Value;
		}
		Meet(Next.Value);
	}
	return INDEX_NONE;
}
