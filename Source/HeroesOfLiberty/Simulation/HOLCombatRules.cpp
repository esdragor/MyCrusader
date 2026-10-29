// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLCombatRules.h"

#include "Algo/Sort.h"
#include "HAL/IConsoleManager.h"
#include "Simulation/HOLHeroXPRules.h"
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

	float GetDamage(const FHOLUnitTypeConstants& Attacker, const FHOLUnitTypeConstants& Defender, const int32 AttackerLevel)
	{
		int32 Damage = 0;
		for (const FHOLAttackBonus& Bonus : Attacker.Bonuses)
		{
			if (Bonus.Damage > 0 && Defender.HasArmorCategory(Bonus.Category))
			{
				Damage += Bonus.Damage;
			}
		}
		Damage -= GetArmor(Defender, Attacker.AttackType);
		// J8b.2: the attack at the attacker's level (exact at level 1: 30 x 100 / 100 = 30).
		const float Hit = FMath::Max(HOLHeroXP::GetLevelStat(static_cast<float>(Attacker.Attack), AttackerLevel) + static_cast<float>(Damage), 1.f);
		// D17: a hero takes HeroTroopDamageFactor of a troop's hit (any attacker but another hero), unrounded.
		if (Defender.HasArmorCategory(EHOLArmorCategory::Hero) && !Attacker.HasArmorCategory(EHOLArmorCategory::Hero))
		{
			return Hit * HeroTroopDamageFactor;
		}
		return Hit;
	}

	int32 GetMaxHealth(const FHOLUnitTypeConstants& Type, const int32 Level)
	{
		return FMath::RoundToInt(HOLHeroXP::GetLevelStat(static_cast<float>(Type.MaxHealth), Level));
	}

	int32 GetHitsToKill(const float Health, const float Damage)
	{
		// Counted by subtraction, as the hits come off (no division: 450 / 3.6 must give the hits the simulation needs). No
		// damage counts as 1, as before; a hit below 1 (the hero's 0.9, D17) stays as it is.
		if (Health <= 0.f)
		{
			return 0;
		}
		const float Hit = Damage > 0.f ? Damage : 1.f;
		int32 Hits = 0;
		for (float Left = Health; Left > 0.f; Left -= Hit)
		{
			++Hits;
		}
		return Hits;
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

	bool KeepsLastAttacker(const int64 StepIndex, const int64 LastHitStep, const int32 StepMs)
	{
		return LastHitStep >= 0 && (StepIndex - LastHitStep) * static_cast<int64>(FMath::Max(StepMs, 1)) < static_cast<int64>(LastAttackerMemoryMs);
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

	EArrowOutcome ResolveArrow(const bool bTargetStands, float& Health, const float Damage)
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

bool HOLCombat::IsBuildingInReach(const FHOLUnitTypeConstants& Attacker, const double EdgeDistance)
{
	return EdgeDistance <= Attacker.BuildingAttackRange;
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

FVector2D HOLCombat::GetFreeBuildingWallPoint(const FVector2D& Position, const float Radius, const FBox2D& Box, const double Gap, const double Reach,
	const TConstArrayView<FVector2D> Bodies, const TConstArrayView<float> BodyRadii)
{
	const FVector2D Facing = GetBuildingApproachPoint(Position, Radius, Box, Gap);
	// O.7b loop 6: a point is covered by a body on it, or by one in the column ahead of it (out from the wall, as wide as the
	// two bodies), as deep as the walker stands or as the reach goes, whichever is deeper (loop 9: by the walker's depth alone,
	// the columns round the corner, beside which it stood, looked free and it went round and round; loop 8, a column with
	// room in reach behind its last row, sent it pressing into the crowd's back, out of reach, in queues): the crowd's back
	// rows, off the wall, bar the walk there too, and the walk goes to the nearest column clear of strikers.
	const double ReachDepth = Reach - Gap;
	const auto IsFree = [&](const FVector2D& Point)
	{
		const FVector2D OnBox(FMath::Clamp(Point.X, Box.Min.X, Box.Max.X), FMath::Clamp(Point.Y, Box.Min.Y, Box.Max.Y));
		const FVector2D Out = (Point - OnBox).GetSafeNormal();
		const double Depth = FMath::Max(FVector2D::DotProduct(Position - Point, Out), ReachDepth);
		for (int32 Body = 0; Body < Bodies.Num(); ++Body)
		{
			const double Touch = double(Radius) + BodyRadii[Body];
			if (FVector2D::DistSquared(Point, Bodies[Body]) < FMath::Square(Touch))
			{
				return false;
			}
			const FVector2D ToBody = Bodies[Body] - Point;
			const double Along = FVector2D::DotProduct(ToBody, Out);
			const double Beside = FMath::Abs(FVector2D::CrossProduct(Out, ToBody));
			if (!Out.IsZero() && Along > -Touch && Along <= Depth && Beside < Touch)
			{
				return false;
			}
		}
		return true;
	};
	if (Radius <= 0.f || IsFree(Facing))
	{
		return Facing;
	}
	// The pushed-out box's round, by its length T from its min corner: bottom, right, top, left (corners square: within reach).
	const FBox2D Round(Box.Min - FVector2D(Radius + Gap), Box.Max + FVector2D(Radius + Gap));
	const double Width = Round.Max.X - Round.Min.X;
	const double Height = Round.Max.Y - Round.Min.Y;
	const double Length = 2. * (Width + Height);
	const auto PointAt = [&](double T)
	{
		T = FMath::Fmod(T, Length);
		T = T < 0. ? T + Length : T;
		if (T < Width)
		{
			return FVector2D(Round.Min.X + T, Round.Min.Y);
		}
		if (T < Width + Height)
		{
			return FVector2D(Round.Max.X, Round.Min.Y + (T - Width));
		}
		if (T < 2. * Width + Height)
		{
			return FVector2D(Round.Max.X - (T - Width - Height), Round.Max.Y);
		}
		return FVector2D(Round.Min.X, Round.Max.Y - (T - 2. * Width - Height));
	};
	const FVector2D On(FMath::Clamp(Facing.X, Round.Min.X, Round.Max.X), FMath::Clamp(Facing.Y, Round.Min.Y, Round.Max.Y));
	const double ToBottom = FMath::Abs(On.Y - Round.Min.Y), ToRight = FMath::Abs(On.X - Round.Max.X);
	const double ToTop = FMath::Abs(On.Y - Round.Max.Y), ToLeft = FMath::Abs(On.X - Round.Min.X);
	const double Least = FMath::Min(FMath::Min(ToBottom, ToRight), FMath::Min(ToTop, ToLeft));
	const double Start = Least == ToBottom ? On.X - Round.Min.X : Least == ToRight ? Width + (On.Y - Round.Min.Y)
		: Least == ToTop ? Width + Height + (Round.Max.X - On.X) : 2. * Width + Height + (Round.Max.Y - On.Y);
	const int32 Steps = FMath::CeilToInt32(0.5 * Length / Radius);
	for (int32 Step = 1; Step <= Steps; ++Step)
	{
		for (const double Sign : { 1., -1. })
		{
			const FVector2D Point = PointAt(Start + Sign * Step * Radius);
			if (IsFree(Point))
			{
				return Point;
			}
		}
	}
	return Facing;
}
