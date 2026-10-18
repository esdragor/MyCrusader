// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLCombatRules.h"

#include "Simulation/HOLUnitTypes.h"

namespace HOLCombat
{
	int32 GetDamage(const FHOLUnitTypeConstants& Attacker, const FHOLUnitTypeConstants& Defender, const bool bRanged)
	{
		int32 Damage = Attacker.Attack;
		if (Attacker.BonusDamage > 0 && Attacker.BonusCategory == Defender.ArmorCategory)
		{
			Damage += Attacker.BonusDamage;
		}
		Damage -= bRanged ? Defender.RangedArmor : Defender.MeleeArmor;
		return FMath::Max(Damage, 1);
	}

	int32 GetHitsToKill(const int32 Health, const int32 Damage)
	{
		return Health <= 0 ? 0 : FMath::DivideAndRoundUp(Health, FMath::Max(Damage, 1));
	}

	int32 ToMilliseconds(const float Seconds)
	{
		return static_cast<int32>(FMath::RoundToInt(static_cast<double>(Seconds) * 1000.));
	}

	int32 GetWindUpMilliseconds(const int32 IntervalMs)
	{
		return IntervalMs * WindUpPercent / 100;
	}

	int32 StartAttack(const int32 CooldownMs, const int32 IntervalMs)
	{
		return FMath::Max(CooldownMs, GetWindUpMilliseconds(IntervalMs));
	}

	bool TickAttack(int32& CooldownMs, const int32 StepMs, const int32 IntervalMs)
	{
		CooldownMs -= StepMs;
		if (CooldownMs > 0)
		{
			return false;
		}
		CooldownMs = FMath::Max(CooldownMs + FMath::Max(IntervalMs, StepMs), 1);
		return true;
	}

	void TickIdle(int32& CooldownMs, const int32 StepMs)
	{
		CooldownMs = FMath::Max(CooldownMs - StepMs, 0);
	}

	double GetEdgeDistance(const FVector2D& A, const float RadiusA, const FVector2D& B, const float RadiusB)
	{
		return FVector2D::Distance(A, B) - static_cast<double>(RadiusA) - static_cast<double>(RadiusB);
	}

	bool IsInRange(const FVector2D& A, const float RadiusA, const FVector2D& B, const float RadiusB, const float Range)
	{
		const double Reach = static_cast<double>(RadiusA) + static_cast<double>(RadiusB) + FMath::Max(static_cast<double>(Range), 0.);
		return FVector2D::DistSquared(A, B) <= Reach * Reach;
	}

	float GetSearchRadius(const FSeeker& Seeker)
	{
		if (Seeker.Mode == ESeekMode::Hold)
		{
			return Seeker.AttackRange;
		}
		float Radius = Seeker.AggressionRadius;
		if (Radius <= 0.f && Seeker.Mode == ESeekMode::AttackMove)
		{
			Radius = AttackMoveRadius;
		}
		return Radius > 0.f ? FMath::Max(Radius, Seeker.AttackRange) : 0.f;
	}

	namespace
	{
		/** A candidate's rank, smaller first (the order of ChooseTarget). */
		struct FRank
		{
			bool bIsBuilding = false;
			bool bOutOfRange = false;
			bool bNotAttackingMe = false;
			bool bCrowded = false;
			double Distance = 0.;
			uint64 Id = 0;

			bool operator<(const FRank& Other) const
			{
				if (bIsBuilding != Other.bIsBuilding) { return Other.bIsBuilding; }
				if (bOutOfRange != Other.bOutOfRange) { return Other.bOutOfRange; }
				if (bNotAttackingMe != Other.bNotAttackingMe) { return Other.bNotAttackingMe; }
				if (bCrowded != Other.bCrowded) { return Other.bCrowded; }
				if (Distance != Other.Distance) { return Distance < Other.Distance; }
				return Id < Other.Id;
			}
		};
	}

	int32 ChooseTarget(const FSeeker& Seeker, const TConstArrayView<FCandidate> Candidates)
	{
		const float SearchRadius = GetSearchRadius(Seeker);
		if (SearchRadius <= 0.f && Seeker.Mode != ESeekMode::Hold)
		{
			return INDEX_NONE;
		}
		int32 Best = INDEX_NONE;
		FRank BestRank;
		for (int32 Index = 0; Index < Candidates.Num(); ++Index)
		{
			const FCandidate& Candidate = Candidates[Index];
			const bool bInRange = IsInRange(Seeker.Position, Seeker.Radius, Candidate.Position, Candidate.Radius, Seeker.AttackRange);
			if (Seeker.Mode == ESeekMode::Hold && !bInRange)
			{
				continue;
			}
			if (bInRange && Candidate.bIsCurrentTarget)
			{
				return Index;
			}
			const bool bInSearch = bInRange
				|| IsInRange(Seeker.Position, Seeker.Radius, Candidate.Position, Candidate.Radius, SearchRadius);
			if (!bInSearch && !Candidate.bAttacksMe)
			{
				continue;
			}
			FRank Rank;
			Rank.bIsBuilding = Candidate.bIsBuilding;
			Rank.bOutOfRange = !bInRange;
			Rank.bNotAttackingMe = !Candidate.bAttacksMe;
			Rank.bCrowded = !bInRange && Candidate.AttackerCount >= MaxAttackers;
			Rank.Distance = GetEdgeDistance(Seeker.Position, Seeker.Radius, Candidate.Position, Candidate.Radius);
			Rank.Id = Candidate.Id;
			if (Best == INDEX_NONE || Rank < BestRank)
			{
				Best = Index;
				BestRank = Rank;
			}
		}
		return Best;
	}
}
