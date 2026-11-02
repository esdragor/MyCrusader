// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLHealRules.h"

namespace HOLHeal
{
	bool IsHealable(const uint32 HealerId, const uint8 HealerCamp, const FCandidate& Candidate)
	{
		return Candidate.UnitId != HealerId && Candidate.Camp == HealerCamp && !Candidate.bUnhealable
			&& Candidate.Health > 0.f && Candidate.Health < Candidate.MaxHealth;
	}

	int32 ChooseTarget(const uint32 HealerId, const uint8 HealerCamp, const FVector2D& HealerLocation, const float Range, const uint32 OrderedId, const uint32 KeptId, const TConstArrayView<FCandidate> Candidates)
	{
		const double RangeSquared = static_cast<double>(Range) * static_cast<double>(Range);
		int32 Ordered = INDEX_NONE;
		int32 Kept = INDEX_NONE;
		int32 Best = INDEX_NONE;
		double BestDistance = 0.;
		for (int32 Index = 0; Index < Candidates.Num(); ++Index)
		{
			const FCandidate& Candidate = Candidates[Index];
			if (!IsHealable(HealerId, HealerCamp, Candidate))
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
	float GetHealedHealth(const float Health, const float MaxHealth, const float HealRate, const int32 StepMs)
	{
		return FMath::Min(MaxHealth, Health + HealRate * static_cast<float>(StepMs) * 0.001f);
	}
}
