// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLHealRules.h"

namespace HOLHeal
{
	bool IsHealable(const uint32 HealerId, const uint8 HealerCamp, const FCandidate& Candidate)
	{
		return Candidate.UnitId != HealerId && Candidate.Camp == HealerCamp && !Candidate.bUnhealable
			&& Candidate.Health > 0.f && Candidate.Health < Candidate.MaxHealth;
	}

	int32 ChooseTarget(const uint32 HealerId, const uint8 HealerCamp, const FVector2D& HealerLocation, const float Range, const uint32 KeptId, const TConstArrayView<FCandidate> Candidates)
	{
		const double RangeSquared = static_cast<double>(Range) * static_cast<double>(Range);
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
			// The one it healed before, still healable in reach: kept, whatever is nearer.
			if (KeptId != 0 && Candidate.UnitId == KeptId)
			{
				return Index;
			}
			if (Best == INDEX_NONE || Distance < BestDistance || (Distance == BestDistance && Candidate.UnitId < Candidates[Best].UnitId))
			{
				Best = Index;
				BestDistance = Distance;
			}
		}
		return Best;
	}

	float GetHealedHealth(const float Health, const float MaxHealth, const float HealRate, const int32 StepMs)
	{
		return FMath::Min(MaxHealth, Health + HealRate * static_cast<float>(StepMs) * 0.001f);
	}
}
