// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLCombatRules.h"

#include "HAL/IConsoleManager.h"
#include "Simulation/HOLUnitTypes.h"

namespace HOLCombat
{
	int32 GetArmor(const FHOLUnitTypeConstants& Defender, const EHOLAttackType AttackType)
	{
		switch (AttackType)
		{
		case EHOLAttackType::Ranged: return Defender.RangedArmor;
		case EHOLAttackType::Siege: return Defender.SiegeArmor;
		case EHOLAttackType::Hero: return Defender.HeroArmor;
		default: return Defender.MeleeArmor;
		}
	}

	int32 GetDamage(const FHOLUnitTypeConstants& Attacker, const FHOLUnitTypeConstants& Defender)
	{
		int32 Damage = Attacker.Attack;
		for (const FHOLAttackBonus& Bonus : Attacker.Bonuses)
		{
			if (Bonus.Damage > 0 && Defender.HasArmorCategory(Bonus.Category))
			{
				Damage += Bonus.Damage;
			}
		}
		Damage -= GetArmor(Defender, Attacker.AttackType);
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

	int32 GetWindUpMilliseconds(const int32 IntervalMs, const int32 WindUpPercent)
	{
		return IntervalMs * FMath::Clamp(WindUpPercent, 1, 100) / 100;
	}

	int32 StartAttack(const int32 CooldownMs, const int32 IntervalMs, const int32 WindUpPercent)
	{
		return FMath::Max(CooldownMs, GetWindUpMilliseconds(IntervalMs, WindUpPercent));
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

	double GetOverlapShare(const bool bEnemies, const bool bWalkingA, const bool bWalkingB)
	{
		if (!bEnemies || (bWalkingA && bWalkingB))
		{
			return 0.5;
		}
		// B standing: it is not moved (A walking, or pushed by its own side). B walking, A standing: B stops against A.
		return bWalkingB ? 0. : 1.;
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
			bool bNoPlace = false;
			bool bNoPlaceNotCurrent = false;
			bool bNotAttackingMe = false;
			bool bCrowded = false;
			double Distance = 0.;
			uint64 Id = 0;

			bool operator<(const FRank& Other) const
			{
				if (bIsBuilding != Other.bIsBuilding) { return Other.bIsBuilding; }
				if (bOutOfRange != Other.bOutOfRange) { return Other.bOutOfRange; }
				if (bNoPlace != Other.bNoPlace) { return Other.bNoPlace; }
				if (bNoPlaceNotCurrent != Other.bNoPlaceNotCurrent) { return Other.bNoPlaceNotCurrent; }
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
			Rank.bNoPlace = !bInRange && Candidate.bNoPlace;
			Rank.bNoPlaceNotCurrent = Rank.bNoPlace && !Candidate.bIsCurrentTarget;
			Rank.bNotAttackingMe = !Candidate.bAttacksMe;
			Rank.bCrowded = !Seeker.bRanged && !bInRange && Candidate.AttackerCount >= MaxAttackers;
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

	bool IsRangedAttack(const float AttackRange)
	{
		return AttackRange > RangedReach;
	}

	int32 GetArrowFlightMilliseconds(const FVector2D& From, const FVector2D& To, const float SpeedCmPerSecond)
	{
		const double Distance = FVector2D::Distance(From, To);
		const double Speed = FMath::Max(static_cast<double>(SpeedCmPerSecond), 1.0);
		return FMath::Max(0, FMath::RoundToInt32(Distance * 1000.0 / Speed));
	}

	EArrowOutcome ResolveArrow(const bool bTargetStands, int32& Health, const int32 Damage)
	{
		if (!bTargetStands || Health <= 0)
		{
			return EArrowOutcome::Lost;
		}
		Health -= Damage;
		return Health <= 0 ? EArrowOutcome::Kill : EArrowOutcome::Hit;
	}

	int32 GetArrowFlightSteps(const int32 FlightMs, const int32 StepMs)
	{
		const int32 Step = FMath::Max(StepMs, 1);
		return FMath::Max(1, (FlightMs + Step - 1) / Step);
	}
}

namespace HOLCombat
{
	namespace
	{
		float ArrowSpeed = DefaultArrowSpeed;
		FAutoConsoleVariableRef CVarArrowSpeed(TEXT("hol.Combat.ArrowSpeed"), ArrowSpeed,
			TEXT("Speed of an arrow (cm/s, 2500 = 25 m/s): its flight lasts the shooter-to-target distance at release over it (server and client alike)"));
	}

	float GetArrowSpeed()
	{
		return FMath::Max(ArrowSpeed, 1.f);
	}
}
