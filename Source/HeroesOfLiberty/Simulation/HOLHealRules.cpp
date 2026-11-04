// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLHealRules.h"

namespace HOLHeal
{
	bool IsAllyOf(const uint8 HealerCamp, const uint32 HealerAllyOwners, const FCandidate& Candidate)
	{
		return Candidate.Camp == HealerCamp || (Candidate.Owner < 32 && (HealerAllyOwners & (1u << Candidate.Owner)) != 0);
	}

	bool IsHealable(const uint32 HealerId, const uint8 HealerCamp, const FCandidate& Candidate, const uint32 HealerAllyOwners)
	{
		return Candidate.UnitId != HealerId && IsAllyOf(HealerCamp, HealerAllyOwners, Candidate) && !Candidate.bUnhealable
			&& Candidate.Health > 0.f && Candidate.Health < Candidate.MaxHealth;
	}

	int32 ChooseTarget(const uint32 HealerId, const uint8 HealerCamp, const FVector2D& HealerLocation, const float Range, const uint32 OrderedId, const uint32 KeptId, const TConstArrayView<FCandidate> Candidates, const uint32 HealerAllyOwners)
	{
		const double RangeSquared = static_cast<double>(Range) * static_cast<double>(Range);
		int32 Ordered = INDEX_NONE;
		int32 Kept = INDEX_NONE;
		int32 Best = INDEX_NONE;
		double BestDistance = 0.;
		for (int32 Index = 0; Index < Candidates.Num(); ++Index)
		{
			const FCandidate& Candidate = Candidates[Index];
			if (!IsHealable(HealerId, HealerCamp, Candidate, HealerAllyOwners))
			{
				continue;
			}
			const double Distance = FVector2D::DistSquared(HealerLocation, Candidate.Location);
			if (Distance > RangeSquared)
			{
				continue;
			}
			if (OrderedId != 0 && Candidate.UnitId == OrderedId)
			{
				Ordered = Index;
			}
			if (KeptId != 0 && Candidate.UnitId == KeptId)
			{
				Kept = Index;
			}
			if (Best == INDEX_NONE || Distance < BestDistance || (Distance == BestDistance && Candidate.UnitId < Candidates[Best].UnitId))
			{
				Best = Index;
				BestDistance = Distance;
			}
		}
		// The one its player ordered it to heal, healable and in reach, whatever is nearer or kept; else the one it healed before, still
		// healable in reach (one at a time); else the nearest.
		return Ordered != INDEX_NONE ? Ordered : Kept != INDEX_NONE ? Kept : Best;
	}
	EOrderWalk GetOrderWalk(const float Distance, const float Range, const bool bWalking, const float GoalDrift)
	{
		if (bWalking)
		{
			// Stop a little inside the range; else keep walking, unless the target moved off the goal.
			return Distance <= Range - OrderStopMargin ? EOrderWalk::Stop : GoalDrift > OrderRepathDistance ? EOrderWalk::Walk : EOrderWalk::Keep;
		}
		// Standing: in reach, nothing to do; out of reach, walk to it.
		return Distance <= Range ? EOrderWalk::Keep : EOrderWalk::Walk;
	}
	float GetHealedHealth(const float Health, const float MaxHealth, const float HealRate, const int32 StepMs)
	{
		return FMath::Min(MaxHealth, Health + HealRate * static_cast<float>(StepMs) * 0.001f);
	}
	void FRiseTracker::Update(const TConstArrayView<TTuple<uint32, int32, bool>> Units, const double Now, const double KeepSeconds)
	{
		TMap<uint32, int32> NewHealth;
		NewHealth.Reserve(Units.Num());
		for (const TTuple<uint32, int32, bool>& Unit : Units)
		{
			NewHealth.Add(Unit.Get<0>(), Unit.Get<1>());
			const int32* Last = LastHealth.Find(Unit.Get<0>());
			if (Last && Unit.Get<1>() > *Last && Unit.Get<2>())
			{
				LastRise.Add(Unit.Get<0>(), Now);
			}
		}
		LastHealth = MoveTemp(NewHealth);
		TArray<uint32, TInlineAllocator<16>> Forgotten;
		for (const TPair<uint32, double>& Rise : LastRise)
		{
			if (Now - Rise.Value > KeepSeconds || !LastHealth.Contains(Rise.Key))
			{
				Forgotten.Add(Rise.Key);
			}
		}
		for (const uint32 UnitId : Forgotten)
		{
			LastRise.Remove(UnitId);
		}
	}

	bool FRiseTracker::IsRising(const uint32 UnitId, const double Now, const double WindowSeconds) const
	{
		const double* Rise = LastRise.Find(UnitId);
		return Rise && Now - *Rise <= WindowSeconds;
	}
}