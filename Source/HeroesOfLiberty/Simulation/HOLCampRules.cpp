// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLCampRules.h"

namespace HOLCampRules
{
	FStep Step(EState& InOutState, const FSettings& Settings, const FVector2D& Post, const TArray<FMember>& Members, const TArray<FIntruder>& Intruders,
		const bool bAttacked)
	{
		FStep Result;
		if (InOutState == EState::Cleared)
		{
			return Result;
		}
		if (Members.Num() == 0)
		{
			InOutState = EState::Cleared;
			Result.bChanged = true;
			Result.bJustCleared = true;
			return Result;
		}

		// What draws the camp: an intruder within the aggro radius (OnSight), or a blow from one it can reach (within the leash radius).
		bool bSighted = false;
		bool bReachable = false;
		for (const FIntruder& Intruder : Intruders)
		{
			const float Distance = static_cast<float>(FVector2D::Distance(Intruder.Location, Post));
			bSighted = bSighted || Distance <= Settings.AggroRadius;
			bReachable = bReachable || Distance <= Settings.LeashRadius;
		}
		const bool bDrawn = (bAttacked && bReachable) || (Settings.Aggro == EAggro::OnSight && bSighted);

		const EState Before = InOutState;
		if (InOutState == EState::Guarding)
		{
			if (bDrawn)
			{
				InOutState = EState::Fighting;
			}
		}
		else if (InOutState == EState::Fighting)
		{
			if (!bReachable)
			{
				InOutState = EState::Returning;
				for (const FMember& Member : Members)
				{
					Result.ReturnIds.Add(Member.Id);
				}
			}
			else
			{
				for (const FMember& Member : Members)
				{
					if (FVector2D::Distance(Member.Location, Post) > Settings.LeashRadius)
					{
						Result.ReturnIds.Add(Member.Id);
					}
				}
			}
		}
		else if (InOutState == EState::Returning)
		{
			bool bAllHome = true;
			for (const FMember& Member : Members)
			{
				bAllHome = bAllHome && FVector2D::Distance(Member.Location, Post) <= Settings.ArriveRadius;
			}
			if (bDrawn)
			{
				InOutState = EState::Fighting;
			}
			else if (bAllHome)
			{
				InOutState = EState::Guarding;
			}
		}
		Result.bChanged = InOutState != Before;
		return Result;
	}

	TArray<FShare> Rewards(const FSettings& Settings, const TArray<FHit>& Hits)
	{
		TArray<FShare> Shares;
		if (Hits.Num() == 0)
		{
			return Shares;
		}

		// The damage by owner, ascending by owner.
		TArray<int32> Owners;
		TArray<int32> Damages;
		for (const FHit& Hit : Hits)
		{
			int32 Slot = Owners.IndexOfByKey(Hit.Owner);
			if (Slot == INDEX_NONE)
			{
				Slot = 0;
				while (Slot < Owners.Num() && Owners[Slot] < Hit.Owner)
				{
					++Slot;
				}
				Owners.Insert(Hit.Owner, Slot);
				Damages.Insert(0, Slot);
			}
			Damages[Slot] += Hit.Damage;
		}

		// The owner who dealt the most (the lowest on a tie: only a strictly greater total replaces).
		int32 Biggest = 0;
		int32 Total = 0;
		for (int32 Index = 0; Index < Owners.Num(); ++Index)
		{
			Total += Damages[Index];
			if (Damages[Index] > Damages[Biggest])
			{
				Biggest = Index;
			}
		}

		if (Settings.Credit == ECredit::LastBlow || (Settings.Credit == ECredit::Shared && Total <= 0))
		{
			Shares.Add(FShare{ Hits.Last().Owner, Settings.RewardXP });
		}
		else if (Settings.Credit == ECredit::MostDamage)
		{
			Shares.Add(FShare{ Owners[Biggest], Settings.RewardXP });
		}
		else
		{
			// In proportion to the damage, rounded down; the points lost by rounding go to the biggest.
			int32 Given = 0;
			for (int32 Index = 0; Index < Owners.Num(); ++Index)
			{
				const int32 XP = Settings.RewardXP * Damages[Index] / Total;
				Given += XP;
				Shares.Add(FShare{ Owners[Index], XP });
			}
			Shares[Biggest].XP += Settings.RewardXP - Given;
		}
		return Shares;
	}

	const TCHAR* GetStateName(const EState State)
	{
		switch (State)
		{
		case EState::Guarding: return TEXT("guarding");
		case EState::Fighting: return TEXT("fighting");
		case EState::Returning: return TEXT("returning");
		default: return TEXT("cleared");
		}
	}
}
