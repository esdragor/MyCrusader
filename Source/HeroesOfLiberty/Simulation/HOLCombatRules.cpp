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
		// V5.3: a unit that only ripostes (the King) never takes the attack-move's radius: its attack-move is a plain move.
		if (Radius <= 0.f && Seeker.Mode == ESeekMode::AttackMove && !Seeker.bRipostes)
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

			/** Perf: the flags alone (all but Distance and Id) rank this one after Other: it cannot come first, whatever its distance. */
			bool FlagsAfter(const FRank& Other) const
			{
				if (bIsBuilding != Other.bIsBuilding) { return bIsBuilding; }
				if (bOutOfRange != Other.bOutOfRange) { return bOutOfRange; }
				if (bNoPlace != Other.bNoPlace) { return bNoPlace; }
				if (bNoPlaceNotCurrent != Other.bNoPlaceNotCurrent) { return bNoPlaceNotCurrent; }
				if (bNotAttackingMe != Other.bNotAttackingMe) { return bNotAttackingMe; }
				if (bCrowded != Other.bCrowded) { return bCrowded; }
				return false;
			}

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
		// V5.1: no search radius but a riposte (the King): only an enemy attacking it, wherever it stands.
		// V5.3: on hold too (in reach only), never an enemy not attacking it.
		const bool bRiposteOnly = Seeker.bRipostes && Seeker.AggressionRadius <= 0.f;
		if (SearchRadius <= 0.f && Seeker.Mode != ESeekMode::Hold && !bRiposteOnly)
		{
			return INDEX_NONE;
		}
		int32 Best = INDEX_NONE;
		FRank BestRank;
		for (int32 Index = 0; Index < Candidates.Num(); ++Index)
		{
			const FCandidate& Candidate = Candidates[Index];
			if (bRiposteOnly && (!Candidate.bAttacksMe || Candidate.bIsBuilding))
			{
				continue;
			}
			// Perf: IsInRange's and GetEdgeDistance's very sums on one squared distance (the same values, bit for bit).
			const double DistanceSquared = FVector2D::DistSquared(Seeker.Position, Candidate.Position);
			const double Radii = static_cast<double>(Seeker.Radius) + static_cast<double>(Candidate.Radius);
			const double RangeReach = Radii + FMath::Max(static_cast<double>(Seeker.AttackRange), 0.);
			const bool bInRange = DistanceSquared <= RangeReach * RangeReach;
			if (Seeker.Mode == ESeekMode::Hold && !bInRange)
			{
				continue;
			}
			if (bInRange && Candidate.bIsCurrentTarget)
			{
				return Index;
			}
			const double SearchReach = Radii + FMath::Max(static_cast<double>(SearchRadius), 0.);
			const bool bInSearch = bInRange || DistanceSquared <= SearchReach * SearchReach;
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
			if (Best != INDEX_NONE && Rank.FlagsAfter(BestRank))
			{
				continue; // ranked after the best by its flags: never chosen, its distance not needed
			}
			Rank.Distance = FMath::Sqrt(DistanceSquared) - static_cast<double>(Seeker.Radius) - static_cast<double>(Candidate.Radius);
			Rank.Id = Candidate.Id;
			if (Best == INDEX_NONE || Rank < BestRank)
			{
				Best = Index;
				BestRank = Rank;
			}
		}
		return Best;
	}

	bool IsFriendFighting(const bool bAttacking, const int32 AttackerCount, const bool bHasLastAttacker)
	{
		return bAttacking || AttackerCount > 0 || bHasLastAttacker;
	}

	bool ShouldComeToAid(const ESeekMode Mode, const bool bRanged, const float AggressionRadius, const bool bHasTarget, const double FightingFriendDistance, const float AidRadius)
	{
		return Mode == ESeekMode::Auto && !bRanged && AggressionRadius > 0.f && !bHasTarget && AidRadius > 0.f
			&& FightingFriendDistance >= 0. && FightingFriendDistance <= AidRadius;
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

double HOLCombat::GetEdgeDistanceToBox(const FVector2D& Position, const float Radius, const FBox2D& Box)
{
	const double DX = FMath::Max3(Box.Min.X - Position.X, 0.0, Position.X - Box.Max.X);
	const double DY = FMath::Max3(Box.Min.Y - Position.Y, 0.0, Position.Y - Box.Max.Y);
	return FMath::Sqrt(DX * DX + DY * DY) - Radius;
}

bool HOLCombat::IsBuildingShot(const FHOLUnitTypeConstants& Attacker)
{
	return Attacker.BuildingAttackType == EHOLAttackType::Ranged;
}

bool HOLCombat::IsBuildingInReach(const FHOLUnitTypeConstants& Attacker, const double EdgeDistance, const bool bStriking, const bool bWalkEndedShort, const bool bHold)
{
	const double Reach = Attacker.BuildingAttackRange;
	if (bStriking || bWalkEndedShort || bHold || IsBuildingShot(Attacker))
	{
		return EdgeDistance <= Reach;
	}
	return EdgeDistance <= FMath::Min(Reach, static_cast<double>(BuildingApproachReach));
}

float HOLCombat::GetBuildingSearchRadius(const FSeeker& Seeker, const FHOLUnitTypeConstants& Attacker)
{
	if (Attacker.BuildingAttack <= 0)
	{
		return 0.f;
	}
	switch (Seeker.Mode)
	{
	case ESeekMode::Hold:
		return Attacker.BuildingAttackRange;
	case ESeekMode::AttackMove:
		return FMath::Max(Seeker.AggressionRadius > 0.f ? Seeker.AggressionRadius : AttackMoveRadius, Attacker.BuildingAttackRange);
	default:
		return Seeker.AggressionRadius > 0.f ? FMath::Max(Seeker.AggressionRadius, Attacker.BuildingAttackRange) : 0.f;
	}
}

int32 HOLCombat::ChooseBuilding(const FVector2D& Position, const float Radius, const float SearchRadius, const TConstArrayView<FBuildingCandidate> Candidates)
{
	int32 Best = INDEX_NONE;
	double BestDistance = 0.0;
	if (SearchRadius <= 0.f)
	{
		return Best;
	}
	for (int32 Index = 0; Index < Candidates.Num(); ++Index)
	{
		const double Distance = GetEdgeDistanceToBox(Position, Radius, Candidates[Index].Box);
		if (Distance > SearchRadius)
		{
			continue;
		}
		if (Best == INDEX_NONE || Distance < BestDistance || (Distance == BestDistance && Candidates[Index].Id < Candidates[Best].Id))
		{
			Best = Index;
			BestDistance = Distance;
		}
	}
	return Best;
}

FVector2D HOLCombat::GetBuildingApproachPoint(const FVector2D& Position, const float Radius, const FBox2D& Box, const double Gap)
{
	const FVector2D Nearest(FMath::Clamp(Position.X, Box.Min.X, Box.Max.X), FMath::Clamp(Position.Y, Box.Min.Y, Box.Max.Y));
	FVector2D Out = Position - Nearest;
	if (Out.IsNearlyZero())
	{
		// Inside the box (pushed in): out through the nearest side.
		const double Left = Position.X - Box.Min.X, Right = Box.Max.X - Position.X, Down = Position.Y - Box.Min.Y, Up = Box.Max.Y - Position.Y;
		const double Least = FMath::Min(FMath::Min(Left, Right), FMath::Min(Down, Up));
		Out = Least == Left ? FVector2D(-1, 0) : Least == Right ? FVector2D(1, 0) : Least == Down ? FVector2D(0, -1) : FVector2D(0, 1);
		const FVector2D Side(Out.X < 0 ? Box.Min.X : Out.X > 0 ? Box.Max.X : Position.X, Out.Y < 0 ? Box.Min.Y : Out.Y > 0 ? Box.Max.Y : Position.Y);
		return Side + Out * (Radius + Gap);
	}
	return Nearest + Out.GetSafeNormal() * (Radius + Gap);
}
