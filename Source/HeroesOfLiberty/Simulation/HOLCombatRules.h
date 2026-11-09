// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FHOLUnitTypeConstants;
enum class EHOLAttackType : uint8;

/**
 * Pure rules of the melee (J7b.1, J7-Q4: home-made processor, AoE4-like rules): the damage of a hit, the cadence of the
 * hits on the fixed step, the reach, and which enemy a unit takes on. No engine state, integers and plain comparisons
 * (the server runs them; the outcome does not depend on the order the candidates come in): tested on their own. The
 * combat processor of J7b.2 calls them.
 */
namespace HOLCombat
{
	/** Steps a unit at 0 health stays Dying before its destruction (J7b.4, J7-Q5): ~1 s at 20 Hz, for its fall. */
	constexpr int32 DyingSteps = 20;

	/** The defender's armour against an attack of this type (D345): melee, ranged, siege or hero armour. */
	HEROESOFLIBERTY_API int32 GetArmor(const FHOLUnitTypeConstants& Defender, EHOLAttackType AttackType);

	/**
	 * Heroic resistance (D17, J8a.1): share of a troop's hit a hero (EHOLArmorCategory::Hero) takes, exactly (health is a
	 * float). A hero's hit on a hero is not cut.
	 */
	inline constexpr float HeroTroopDamageFactor = 0.3f;

	/**
	 * Damage of one hit (D21, D345): Attack, plus each of the attacker's bonuses aimed at one of the defender's armour
	 * categories (they add up), minus the defender's armour of the attacker's AttackType, 1 at least (a hit always hurts).
	 * On a hero hit by anyone but a hero, then HeroTroopDamageFactor of that, unrounded (D17: 5 -> 1.5, 3 -> 0.9). The
	 * hero's own weakness against buildings is his table's BuildingAttack. J8b.2: the attacker's level scales its Attack
	 * (HOLHeroXP::GetLevelStat, +8 % a level, unrounded) before the bonuses and the armour; 1 for every troop. T4:
	 * DefenderArmorBonus is armour added to the defender's (a talent in the aura), 0 by default. T6: AttackerAttackPercent
	 * is the percent the attacker's player gained on its attack by technologies (Lames affutees, Empennage), 0 by default; it
	 * scales the Attack (at the attacker's level) before the bonuses and the armour, like the level.
	 */
	HEROESOFLIBERTY_API float GetDamage(const FHOLUnitTypeConstants& Attacker, const FHOLUnitTypeConstants& Defender, int32 AttackerLevel = 1, int32 DefenderArmorBonus = 0, int32 AttackerAttackPercent = 0);

	/**
	 * J8b.2: a unit's maximum health at its level (the hero's grows +8 % a level, J8b-Q3), rounded to nearest. S5: HealthPercent
	 * is the percent its player's technologies add to it (Reinforced Ram: +10 % for the Ram), applied after the level; 0 by default.
	 */
	HEROESOFLIBERTY_API int32 GetMaxHealth(const FHOLUnitTypeConstants& Type, int32 Level, int32 HealthPercent = 0);

	/**
	 * Health as shown (HUD, bars) and sent (the wire's exact health, D455): the simulation's float health rounded up, so a
	 * living unit shows 1 at least; 0 dead (0 or below).
	 */
	inline int32 GetShownHealth(const float Health)
	{
		return Health > 0.f ? FMath::Max(FMath::CeilToInt(Health), 1) : 0;
	}

	/** Hits that take Health down to 0 (0 when Health <= 0; a Damage of 0 or less counts as 1). */
	HEROESOFLIBERTY_API int32 GetHitsToKill(float Health, float Damage);

	// Cadence. The time between two hits is counted in whole milliseconds, not in steps: a cool-down loses the step's
	// length each step, a hit falls when it reaches 0, then the interval is added back. The average cadence is the
	// table's exactly (1.875 s on 50 ms steps: hits 37 or 38 steps apart, no drift from rounding to 38 steps), and it
	// stays integer, so deterministic.

	/** Seconds to whole milliseconds (the table's 1.875 s -> 1875). */
	HEROESOFLIBERTY_API int32 ToMilliseconds(float Seconds);

	/**
	 * Wind-up: the first hit on a target falls WindUpPercent of an interval after the unit starts attacking it (in reach),
	 * not at once, so the strike gesture has time to show and a unit coming into reach does not hit in the same step. The
	 * percent is the type's (FHOLUnitTypeConstants::AttackWindUpPercent, J7c, retour d'essai 8: hit and run): half the
	 * interval for the melee, 12 % for the Archer (it looses ~0.2 s after it stops, then may walk off while it
	 * reloads, the cool-down running down meanwhile, TickIdle). Clamped to 1..100.
	 */
	inline constexpr int32 DefaultWindUpPercent = 50;

	HEROESOFLIBERTY_API int32 GetWindUpMilliseconds(int32 IntervalMs, int32 WindUpPercent);

	/**
	 * The cool-down when a unit starts (or switches) an attack: what it still had to wait, the wind-up at least. A unit
	 * that just struck keeps its own rhythm; a switch of target never gives a free hit.
	 */
	HEROESOFLIBERTY_API int32 StartAttack(int32 CooldownMs, int32 IntervalMs, int32 WindUpPercent);

	/**
	 * One step of an attacking unit (in reach of its target): the cool-down loses StepMs; when it reaches 0 the unit
	 * strikes (true) and IntervalMs is added back. One hit per step at most (an interval shorter than the step is taken
	 * as one step).
	 */
	HEROESOFLIBERTY_API bool TickAttack(int32& CooldownMs, int32 StepMs, int32 IntervalMs);

	/** One step of a unit that does not attack: the cool-down runs down to 0 and stays there. */
	HEROESOFLIBERTY_API void TickIdle(int32& CooldownMs, int32 StepMs);

	/** Gap between the edges of two bodies (negative when they overlap). Inline (perf, 2026-10-24): met per neighbour. */
	inline double GetEdgeDistance(const FVector2D& A, const float RadiusA, const FVector2D& B, const float RadiusB)
	{
		return FVector2D::Distance(A, B) - static_cast<double>(RadiusA) - static_cast<double>(RadiusB);
	}

	/**
	 * Reach (AttackRange is between the two bodies' edges): the gap between the edges, centre distance minus both radii,
	 * at most Range. Touching bodies (gap 0 or less, pushed into each other) are in reach. Inline (perf): met per neighbour.
	 */
	inline bool IsInRange(const FVector2D& A, const float RadiusA, const FVector2D& B, const float RadiusB, const float Range)
	{
		const double Reach = static_cast<double>(RadiusA) + static_cast<double>(RadiusB) + FMath::Max(static_cast<double>(Range), 0.);
		return FVector2D::DistSquared(A, B) <= Reach * Reach;
	}

	/**
	 * Share (0, 0.5 or 1) of the overlap of two bodies that the separation gives to A, the unit that looks for its
	 * neighbours (a walker, or a standing unit pushed this step), B taking the rest. Two units of one player take half
	 * each, the soft push of the separation (D292). Two enemies never push each other back (the user's trial of J7b: the
	 * lines hold, as in AoE4): a walker stops against a standing enemy (1), a standing enemy is never moved, and a unit
	 * pushed into an enemy by its own side steps back itself; two enemies both walking into each other take half each.
	 */
	HEROESOFLIBERTY_API double GetOverlapShare(bool bEnemies, bool bWalkingA, bool bWalkingB);

	// Target choice.

	/** How the unit looks for an enemy. */
	enum class ESeekMode : uint8
	{
		/** Standing or walking without an order to fight: takes on enemies within its aggression radius (0: none). */
		Auto,
		/** Attack-move: as Auto, but a unit without an aggression radius uses AttackMoveRadius. */
		AttackMove,
		/** Hold position: only enemies already in reach, never a chase. */
		Hold,
	};

	/** Search radius (cm, edge to edge) on an attack-move of a unit with no aggression of its own (16 m, 4 tiles of 4 m, D345). */
	inline constexpr float AttackMoveRadius = 1600.f;

	/**
	 * Spreading: an enemy out of reach already taken on by MaxAttackers others comes after one less crowded (about six
	 * bodies of 60 cm fit around one of 60 cm). One in reach is never passed over for that: it can be hit now.
	 */
	inline constexpr int32 MaxAttackers = 6;

	/** The unit looking for a target. */
	struct FSeeker
	{
		FVector2D Position = FVector2D::ZeroVector;
		float Radius = 60.f;
		float AttackRange = 50.f;
		float AggressionRadius = 0.f;
		ESeekMode Mode = ESeekMode::Auto;
		/** A shooter (IsRangedAttack, J7c-Q6): no spreading to a less crowded enemy, the nearest, kept while in reach. */
		bool bRanged = false;
		/**
		 * With no aggression radius, it still takes on an enemy attacking it (bAttacksMe), never another (V5.1,
		 * FHOLUnitTypeConstants::bRipostes: the King), on attack-move too (no AttackMoveRadius, V5.3) and on hold (in reach
		 * only). Without it, such a unit takes on nobody by itself.
		 */
		bool bRipostes = false;
		/** S4 (SQ1 = B, D614): false for the Ram (no attack on units): it never takes a unit on by itself, only buildings. */
		bool bCanAttackUnits = true;
	};

	/** S4 (D614): whether Other lies within an area of Radius cm around Centre (centre to centre, the circle's edge in). */
	inline bool IsInArea(const FVector2D& Centre, const FVector2D& Other, const float Radius)
	{
		return FVector2D::DistSquared(Centre, Other) <= FMath::Square(Radius);
	}

	/** One enemy the grid found near the seeker (the processor fills them). */
	struct FCandidate
	{
		/** Tie-break, last: a stable id of the entity (in the processor, its index then its serial number). */
		uint64 Id = 0;
		FVector2D Position = FVector2D::ZeroVector;
		float Radius = 60.f;
		/** Hook for later parts: a building comes after any unit (none in J7b). */
		bool bIsBuilding = false;
		/** It attacks the seeker (riposte: such an enemy is taken on even beyond the search radius). */
		bool bAttacksMe = false;
		/** The seeker's current target: kept while in reach, so the choice does not flicker between two equal ones. */
		bool bIsCurrentTarget = false;
		/** Others of the seeker's side already attacking it (the seeker left out). */
		int32 AttackerCount = 0;
		/**
		 * Out of the seeker's reach, every place around it taken (MaxAttackers bodies of any side touching it; the processor
		 * sets it under hol.Combat.PreferFreeTarget): one with a place comes first, the current target first among the full.
		 */
		bool bNoPlace = false;
	};

	/**
	 * Radius (cm, edge to edge) within which the seeker takes on an enemy it was not ordered at: on hold, its reach;
	 * else its aggression radius (an attack-move gives AttackMoveRadius to a unit without one), never less than its
	 * reach. 0: it takes on nobody by itself (the Peasant, unless on an attack-move or on hold).
	 */
	HEROESOFLIBERTY_API float GetSearchRadius(const FSeeker& Seeker);

	/**
	 * The enemy the seeker takes on by itself, as an index into Candidates, INDEX_NONE for none (an ordered target does
	 * not come through here). Eligible: within the search radius, or attacking the seeker (riposte) when it has a search
	 * radius at all; with none but bRipostes (V5.1, the King), only an enemy attacking it; on hold, in reach only. The current target, in reach, is kept. Otherwise the best by, in order: a
	 * unit before a building; in reach before out of reach; attacking the seeker before not; (out of reach) fewer than
	 * MaxAttackers attackers before crowded (not for a shooter, bRanged); nearest by edge distance; smallest Id. Any order of the same candidates
	 * gives the same enemy.
	 */
	HEROESOFLIBERTY_API int32 ChooseTarget(const FSeeker& Seeker, TConstArrayView<FCandidate> Candidates);

	/**
	 * The unit takes on enemies by itself (its search, its riposte, its zone of control), J7b.3 and D345: not while it walks a
	 * player's order but on an attack-move; never with an ordered target (a unit, until it dies) nor an ordered building
	 * (a right click on it, Shift or not: until it falls, whatever enemy unit comes).
	 */
	inline bool MaySeek(const bool bWalkingOrder, const bool bAttackMove, const bool bOrderedTarget, const bool bOrderedBuilding, const bool bWalkStalled = false)
	{
		return (!bWalkingOrder || bAttackMove || bWalkStalled) && !bOrderedTarget && !bOrderedBuilding;
	}

	/**
	 * Q5 = C: a walk that no longer progresses (its final leg or its path held StuckSteps steps without headway: the place barred, a friend in the
	 * way) no longer shields the walker from the fight: struck meanwhile, it ripostes (MaySeek's bWalkStalled). A normal walk is not cut (D345,
	 * J7b.3). MinSteps <= 0: off (hol.Combat.WalkRiposteSteps).
	 */
	inline bool IsWalkStalled(const int32 StuckSteps, const int32 MinSteps)
	{
		return MinSteps > 0 && StuckSteps >= MinSteps;
	}

	// Attacks on buildings (D345.2b). Against a building a unit always uses its building attack (FHOLUnitTypeConstants::
	// BuildingAttack, its type, interval and reach). It takes one on by order (a right click on an enemy building), on an
	// attack-move when no enemy unit is found, or at rest within its aggression radius when no enemy unit is (the Peasant,
	// without one, only by order). One taken on by itself is dropped for an enemy unit it takes on; an ordered one never
	// (D345: no search while it stands, MaySeek), struck until it falls. Never an own or allied building, never a bare
	// foundation (unseen by the enemy).

	/** Edge gap (cm) from a body (Position, Radius) to a building's body box (D324): 0 or less when it touches it. */
	HEROESOFLIBERTY_API double GetEdgeDistanceToBox(const FVector2D& Position, float Radius, const FBox2D& Box);

	/** D345.2b: a shooter walking to shoot a building aims this far (cm) beyond its radius off the body's box (it stops at its reach). */
	inline constexpr double BuildingApproachGap = 40.0;

	/**
	 * EtapeDefini O.7 (the user's rule of 2026-10-28, from an AoE4 clip, replacing J's and O.5's places): a melee walks to the
	 * visible wall's point nearest it (AHOLBuilding::GetVisibleBodyBounds), off it by its radius and this gap (cm): a
	 * Man-at-arms' edge 95 cm off a House's body, clear of the navmesh's forbidden area. It strikes as soon as in reach
	 * (IsBuildingInReach) and closes in to there while it strikes: the crowd packs against the wall.
	 */
	inline constexpr double BuildingWallGap = 10.0;
	/**
	 * EtapeDefini O.7: a melee closing in while it strikes stops its walk once BuildingCloseInStopMs (ms) without coming this
	 * much (cm) nearer the visible wall (the friends in front hold it), or once within BuildingWallGap + this of it.
	 */
	inline constexpr double BuildingCloseInProgress = 10.0;
	inline constexpr int32 BuildingCloseInStopMs = 600;

	/**
	 * L114 (the Spearman that goes round): an enemy a flanking melee may take as the "free enemy" met on the way (it ends the flank):
	 * not its own target before it is round the line's end (Along past End on its FlankSide). It was held from that very enemy by its
	 * friends in front; taking it again ended the flank 0.2 s after it began and sent it back to the same blocked place.
	 */
	inline bool CanFlankTakeEnemy(const bool bIsOwnTarget, const int32 FlankSide, const double Along, const double End)
	{
		return !bIsOwnTarget || FlankSide * (Along - End) >= 0.0;
	}

	/** L114: an enemy at OtherAlong (along the line's axis) lies at or beyond the line's End on the flank's side, less Slack (cm). */
	inline bool IsBeyondLineEnd(const int32 FlankSide, const double OtherAlong, const double End, const double Slack)
	{
		return FlankSide * (OtherAlong - End) >= -Slack;
	}

	/**
	 * L114 (the tours): a melee held behind its line goes round its end only if there is something to go round for: an enemy with a free
	 * place at or beyond that end (bFreeEnemyBeyondEnd), and the end within the cap (MaxShift, 0 = none). Otherwise it waited, set off,
	 * found nobody at the end, came back to its blocked place (the target it had) and set off again: a tour of the camp every 3 s.
	 */
	inline bool ShouldGoRound(const bool bFreeEnemyBeyondEnd, const double Shift, const double MaxShift)
	{
		return bFreeEnemyBeyondEnd && (MaxShift <= 0.0 || Shift <= MaxShift);
	}

	/** A body on the ground: where, and how wide (cm). */
	struct FBody
	{
		FVector2D Where = FVector2D::ZeroVector;
		float Radius = 60.f;
	};

	/**
	 * D641 (the user's retour on the neutrals trial: a soldier that does nothing though it could go round to strike): whether a body of SelfRadius could
	 * stand touching the target (TargetWhere, TargetRadius) without overlapping any of Others (the target and the unit itself left out by the caller):
	 * sixteen spots round the target, each free when every other body is further than Tolerance (0.85) of the two radii. The reading of "there is
	 * room to strike" by eye; hol.Test.Sanity's check 4 (HOLSanity::HasFreePlaceAround) and the flank (hol.Combat.FlankToTarget) are the same call.
	 */
	inline bool HasFreePlaceAround(const FVector2D& TargetWhere, const float TargetRadius, const float SelfRadius, TConstArrayView<FBody> Others, const float Tolerance = 0.85f)
	{
		constexpr int32 Spots = 16;
		for (int32 Spot = 0; Spot < Spots; ++Spot)
		{
			const double Angle = 2. * UE_DOUBLE_PI * Spot / Spots;
			const FVector2D Place = TargetWhere + (TargetRadius + SelfRadius + 2.f) * FVector2D(FMath::Cos(Angle), FMath::Sin(Angle));
			bool bFree = true;
			for (const FBody& Other : Others)
			{
				if (FVector2D::Distance(Place, Other.Where) < Tolerance * (SelfRadius + Other.Radius))
				{
					bFree = false;
					break;
				}
			}
			if (bFree)
			{
				return true;
			}
		}
		return false;
	}

	/**
	 * D641 (hol.Combat.ClaimPlace): the free spot (the same sixteen as HasFreePlaceAround) whose angle round the target is the nearest to PreferAngle (radians;
	 * the lower spot on a tie); false when none is free. OutAngle is the spot's angle. A soldier that keeps the angle it claimed keeps the same side of the target.
	 */
	inline bool FindFreePlaceAround(const FVector2D& TargetWhere, const float TargetRadius, const float SelfRadius, TConstArrayView<FBody> Others, const double PreferAngle, double& OutAngle, const float Tolerance = 0.85f)
	{
		constexpr int32 Spots = 16;
		bool bFound = false;
		double BestGap = 0.;
		for (int32 Spot = 0; Spot < Spots; ++Spot)
		{
			const double Angle = 2. * UE_DOUBLE_PI * Spot / Spots;
			const FVector2D Place = TargetWhere + (TargetRadius + SelfRadius + 2.f) * FVector2D(FMath::Cos(Angle), FMath::Sin(Angle));
			bool bFree = true;
			for (const FBody& Other : Others)
			{
				if (FVector2D::Distance(Place, Other.Where) < Tolerance * (SelfRadius + Other.Radius))
				{
					bFree = false;
					break;
				}
			}
			if (!bFree)
			{
				continue;
			}
			double Gap = FMath::Fmod(FMath::Abs(Angle - PreferAngle), 2. * UE_DOUBLE_PI);
			Gap = FMath::Min(Gap, 2. * UE_DOUBLE_PI - Gap);
			if (!bFound || Gap < BestGap - 1e-9)
			{
				bFound = true;
				BestGap = Gap;
				OutAngle = Angle;
			}
		}
		return bFound;
	}

	/**
	 * D641 (hol.Combat.PlaceBySide, the walk-round trace of 2026-11-05): the spot a soldier claims round its target. A claimed place that is still free
	 * is kept (the same side of the target). A place taken is replaced by the free spot nearest to SelfAngle (the soldier's own side of the target,
	 * radians), the shortest way round the ring from where it stands; with bBySide false it is the spot nearest to the old angle (the rule as it was:
	 * once the soldier had slid round, the spot nearest to the OLD place could lie across the whole melee, a walk round the crowd that reverses on its
	 * steps). A soldier without a place takes the spot nearest to SelfAngle. False when no spot is free.
	 */
	inline bool ChooseFreePlace(const FVector2D& TargetWhere, const float TargetRadius, const float SelfRadius, TConstArrayView<FBody> Others, const bool bHasPlace,
		const double PlaceAngle, const double SelfAngle, const bool bBySide, double& OutAngle)
	{
		if (bHasPlace)
		{
			double Kept = 0.;
			if (!FindFreePlaceAround(TargetWhere, TargetRadius, SelfRadius, Others, PlaceAngle, Kept))
			{
				return false;
			}
			if (FMath::Abs(Kept - PlaceAngle) <= 1e-3 || !bBySide)
			{
				OutAngle = Kept;
				return true;
			}
		}
		return FindFreePlaceAround(TargetWhere, TargetRadius, SelfRadius, Others, SelfAngle, OutAngle);
	}

	/** A place a friend claimed round the same target (BUG-melee-stuck-attack): the friend's body and the angle of its place round the target. */
	struct FPlaceClaim
	{
		FVector2D Where = FVector2D::ZeroVector;
		float Radius = 60.f;
		double Angle = 0.;
		uint64 UnitId = 0;
	};

	/**
	 * BUG-melee-stuck-attack (hol.Combat.ClaimedSlots; the melee_stuck_8 pair #13/#14, side by side 70 to 156 steps, the same target, the same place):
	 * two friends of one target that claim the same free spot both walk to it, and one waits behind the other, who is held behind friends in turn. The
	 * claims of the friends are read as bodies standing on their spots (Out), so the free spot a soldier takes is never one a friend holds. A claim holds
	 * against the soldier when its holder is nearer its spot than the soldier is its own (by more than Margin cm), or as near and of the lower id: one of
	 * two soldiers on the same spot always keeps it. SelfWhere = the soldier's own body; Claims = the friends' claims (the soldier's own left out by the caller).
	 */
	inline void AppendClaimedPlaceBodies(const FVector2D& TargetWhere, const float TargetRadius, const FVector2D& SelfWhere, const uint64 SelfId, const double SelfPlaceAngle,
		const bool bSelfHasPlace, const float SelfRadius, TConstArrayView<FPlaceClaim> Claims, TArray<FBody>& Out, const double Margin = 30.)
	{
		for (const FPlaceClaim& Claim : Claims)
		{
			const FVector2D Spot = TargetWhere + (TargetRadius + Claim.Radius + 2.f) * FVector2D(FMath::Cos(Claim.Angle), FMath::Sin(Claim.Angle));
			// How near the soldier is to the same spot: its claimed one when it has one, else the spot of the friend's claim (it would stand there).
			const FVector2D SelfSpot = bSelfHasPlace
				? TargetWhere + (TargetRadius + SelfRadius + 2.f) * FVector2D(FMath::Cos(SelfPlaceAngle), FMath::Sin(SelfPlaceAngle))
				: Spot;
			const double MineToMine = FVector2D::Distance(SelfWhere, SelfSpot);
			const double TheirsToTheirs = FVector2D::Distance(Claim.Where, Spot);
			const bool bHolderCloser = TheirsToTheirs < MineToMine - Margin || (FMath::Abs(TheirsToTheirs - MineToMine) <= Margin && Claim.UnitId < SelfId);
			if (bHolderCloser)
			{
				Out.Add({ Spot, Claim.Radius });
			}
		}
	}

	/** The kept side of a rear slide (UHOLFullSpeedProcessor::SlideAlongFriends): Hand +1 / -1 = the left / right of the way to the blocker; ShutSeconds < 0 = a committed turn. */
	struct FSlideSideState
	{
		double Hand = 0.;
		float ShutSeconds = 0.f;
	};

	struct FSlideSideResult
	{
		double Hand = 0.;
		bool bMoves = false;
	};

	/**
	 * BUG-melee-stuck-attack (hol.Combat.RearSlideTurnCommit): one step of the side a soldier kept while it slides along the friends barring its way.
	 * State.Hand = the side it kept; bApplyGoalRule: that side is the kept one (turned when the goal is clearly on the other); bShutPlus / bShutMinus:
	 * a friend at contact on the left (Hand +1) / right (Hand -1). Out.bMoves: it walks its side this step; else it stands (or creeps).
	 * Before: the turn after ShutTime of a shut side was undone the next step by the goal rule (the target on the shut side), the shut time set back to 0:
	 * a cycle of one second, standing for 120+ steps. Now (Commit > 0) the turn walks at once and the goal rule is put by for Commit seconds
	 * (ShutSeconds negative while committed); a committed side that shuts in turn starts the count again.
	 */
	inline FSlideSideResult AdvanceSlideSide(FSlideSideState& State, const bool bApplyGoalRule, const double GoalSide, const bool bShutPlus, const bool bShutMinus,
		const float Dt, const float ShutTime, const double TurnGoalSide, const float Commit)
	{
		if (bApplyGoalRule && (State.ShutSeconds >= 0.f || Commit <= 0.f) && GoalSide * State.Hand < -TurnGoalSide)
		{
			State.Hand = -State.Hand;
			State.ShutSeconds = 0.f;
		}
		FSlideSideResult Out;
		const bool bWayShut = State.Hand > 0. ? bShutPlus : bShutMinus;
		const bool bOppositeShut = State.Hand > 0. ? bShutMinus : bShutPlus;
		if (!bWayShut)
		{
			Out.bMoves = true;
			State.ShutSeconds = State.ShutSeconds < 0.f ? FMath::Min(0.f, State.ShutSeconds + Dt) : 0.f;
		}
		else
		{
			State.ShutSeconds = State.ShutSeconds < 0.f ? 0.f : State.ShutSeconds;
			State.ShutSeconds += Dt;
			if (State.ShutSeconds >= ShutTime && !bOppositeShut)
			{
				State.Hand = -State.Hand;
				if (Commit > 0.f)
				{
					State.ShutSeconds = -Commit;
					Out.bMoves = true;
				}
				else
				{
					State.ShutSeconds = 0.f;
				}
			}
		}
		Out.Hand = State.Hand;
		return Out;
	}

	/**
	 * D641 (Q-neutrals-hold = C): whether a free place round the target is out of the soldier's reach for the audit: bodies close on BOTH sides of it
	 * (relative to its way to the target), within Slack of touching (1.5 x the two radii by default: a gap of up to half a body), wall it in between two
	 * friends who strike; it waits, rightly. Others = every body but the soldier and its target. The audit (hol.Test.Sanity check 4) counts a stall only
	 * when this is false.
	 */
	inline bool IsBoxedIn(const FVector2D& SelfWhere, const float SelfRadius, const FVector2D& TargetWhere, TConstArrayView<FBody> Others, const float Slack = 1.5f)
	{
		FVector2D Ahead = TargetWhere - SelfWhere;
		if (Ahead.IsNearlyZero())
		{
			return false;
		}
		Ahead.Normalize();
		bool bLeft = false;
		bool bRight = false;
		TArray<const FBody*, TInlineAllocator<8>> Near;
		for (const FBody& Other : Others)
		{
			const FVector2D Off = Other.Where - SelfWhere;
			if (Off.Size() > Slack * (SelfRadius + Other.Radius))
			{
				continue;
			}
			const double Cross = Ahead.X * Off.Y - Ahead.Y * Off.X;
			bLeft |= Cross > 0.;
			bRight |= Cross < 0.;
			if (Ahead.X * Off.X + Ahead.Y * Off.Y > 0.)
			{
				Near.Add(&Other);
			}
		}
		if (bLeft && bRight)
		{
			return true;
		}
		// 2026-11-05 (trace of neutrals, #11 behind #9 and #13): two close bodies in front that are both on the same side of the line to the target
		// (one almost on it) still wall the soldier in when the gap between them cannot take its body; the sign test above misses that pair.
		for (int32 A = 0; A < Near.Num(); ++A)
		{
			for (int32 B = A + 1; B < Near.Num(); ++B)
			{
				if (FVector2D::Distance(Near[A]->Where, Near[B]->Where) - Near[A]->Radius - Near[B]->Radius < 2.f * SelfRadius)
				{
					return true;
				}
			}
		}
		return false;
	}

	/**
	 * Celeborn-24 (hol.Combat.PlaceBoxedIn, BUG-melee-spin-sanity): whether the free place a soldier would claim round its target is walled in from where it stands:
	 * the friends and enemies close round its way to the place (IsBoxedIn read towards the place, not the target) leave no gap its body takes. A second-rank soldier
	 * that claimed such a place ran a U of 1100 cm round its friends to a place they closed (spin of melee_stuck, #11 and #2: 1000 cm in 3 s for 200 cm net);
	 * it no longer claims it: it walks to the target and waits behind the crowd. Others = every body but the soldier and its target.
	 */
	inline bool IsPlaceWalledIn(const FVector2D& SelfWhere, const float SelfRadius, const FVector2D& PlaceWhere, TConstArrayView<FBody> Others)
	{
		return false; // STUB (Celeborn-24): the test is red first
	}

	/** D641 (hol.Combat.ClaimPlace): the rear flank (a 600 cm tour round the line) is suspended while the soldier holds a claimed place that is within MaxDistance (400 cm). */
	inline bool ShouldSuspendRearFlank(const bool bHasPlace, const double DistanceToPlace, const double MaxDistance = 400.0)
	{
		return bHasPlace && DistanceToPlace <= MaxDistance;
	}

	/** D641: a new target within MaxDistance (150 cm) of the last one is "the next enemy along", not a new quarrel: the no-headway count goes on. */
	inline bool IsNeighbourTarget(const FVector2D& OldWhere, const FVector2D& NewWhere, const double MaxDistance = 150.0)
	{
		return FVector2D::DistSquared(OldWhere, NewWhere) <= MaxDistance * MaxDistance;
	}

	/** Angular gap (radians, 0 to pi) between two angles round a target. */
	inline double GetPlaceGap(const double PlaceAngle, const double SelfAngle)
	{
		double Gap = FMath::Fmod(FMath::Abs(PlaceAngle - SelfAngle), 2. * UE_DOUBLE_PI);
		return FMath::Min(Gap, 2. * UE_DOUBLE_PI - Gap);
	}

	/**
	 * D641 (hol.Combat.FarPlaceRetarget, the melee that spins of 2026-11-05: 6 soldiers on 3 targets that die one after the other): a place claimed more than
	 * MaxGap (1.5 rad, about 90 degrees) from the side of the target the soldier is on lies across the melee: reaching it is a walk round the crowd that
	 * reverses on its steps, and the target is dead before the arrival.
	 */
	inline bool IsPlaceTooFar(const double PlaceAngle, const double SelfAngle, const double MaxGap = 1.5)
	{
		return GetPlaceGap(PlaceAngle, SelfAngle) > MaxGap;
	}

	/**
	 * Q-neutrals-second-rank = C (hol.Combat.BoxedRetarget): a soldier walled in (IsBoxedIn) with no headway for MinSeconds changes target, unless the target was
	 * ordered by the player.
	 */
	inline bool ShouldRetargetWhenBoxedIn(const float NoProgressSeconds, const bool bBoxedIn, const bool bOrderedTarget, const float MinSeconds = 1.f)
	{
		return bBoxedIn && !bOrderedTarget && NoProgressSeconds >= MinSeconds;
	}

	/**
	 * Q-neutrals-second-rank = C: the enemy a walled-in soldier turns to: among EnemyBodies (indices into Bodies, with EnemyIds as tie-break) the one whose edge is
	 * nearest to the soldier, having a free place (HasFreePlaceAround, every other body counted) that the soldier can reach without going round (not IsBoxedIn
	 * towards it); ExcludeBody (the target it leaves) is never chosen. Bodies = every body near but the soldier. Returns the index into EnemyBodies, or INDEX_NONE.
	 * With MaxPlaceGap >= 0 (D641, hol.Combat.FarPlaceRetarget) the enemy's free place nearest to the soldier's own side of it must lie within MaxPlaceGap rad of that side.
	 */
	inline int32 FindReachableEnemy(const FVector2D& SelfWhere, const float SelfRadius, TConstArrayView<FBody> Bodies, TConstArrayView<int32> EnemyBodies, TConstArrayView<uint64> EnemyIds, const int32 ExcludeBody, const double MaxPlaceGap = -1., const bool bLoose = false)
	{
		// bLoose (hol.Combat.StuckLooseTarget, BUG-melee-stuck-attack): the nearest other enemy, whether or not it has a free place or the way to it is walled in.
		int32 Best = INDEX_NONE;
		double BestEdge = 0.;
		TArray<FBody, TInlineAllocator<32>> Others;
		for (int32 Slot = 0; Slot < EnemyBodies.Num(); ++Slot)
		{
			const int32 Body = EnemyBodies[Slot];
			if (Body == ExcludeBody || !Bodies.IsValidIndex(Body) || !EnemyIds.IsValidIndex(Slot))
			{
				continue;
			}
			Others.Reset();
			for (int32 Other = 0; Other < Bodies.Num(); ++Other)
			{
				if (Other != Body)
				{
					Others.Add(Bodies[Other]);
				}
			}
			const FBody& Enemy = Bodies[Body];
			if (!bLoose && (!HasFreePlaceAround(Enemy.Where, Enemy.Radius, SelfRadius, Others) || IsBoxedIn(SelfWhere, SelfRadius, Enemy.Where, Others)))
			{
				continue;
			}
			if (!bLoose && MaxPlaceGap >= 0.)
			{
				const FVector2D FromEnemy = SelfWhere - Enemy.Where;
				double SelfSide = FromEnemy.IsNearlyZero() ? 0. : FMath::Atan2(FromEnemy.Y, FromEnemy.X);
				SelfSide += SelfSide < 0. ? 2. * UE_DOUBLE_PI : 0.;
				double Spot = 0.;
				if (!FindFreePlaceAround(Enemy.Where, Enemy.Radius, SelfRadius, Others, SelfSide, Spot) || IsPlaceTooFar(Spot, SelfSide, MaxPlaceGap))
				{
					continue;
				}
			}
			const double Edge = FVector2D::Distance(SelfWhere, Enemy.Where) - SelfRadius - Enemy.Radius;
			if (Best == INDEX_NONE || Edge < BestEdge - 1e-6 || (FMath::Abs(Edge - BestEdge) <= 1e-6 && EnemyIds[Slot] < EnemyIds[Best]))
			{
				Best = Slot;
				BestEdge = Edge;
			}
		}
		return Best;
	}

	/** Its hits on a building are arrows in flight: a building attack of type Ranged (a melee's reach of a tile is beyond RangedReach). */
	HEROESOFLIBERTY_API bool IsBuildingShot(const FHOLUnitTypeConstants& Attacker);

	/**
	 * In reach of a building at EdgeDistance (its edge to the body's box): within its building reach (D497: 400 cm for a
	 * melee), walking or not. EtapeDefini O.7: no shorter reach on its way in, it strikes as soon as it comes within it.
	 */
	HEROESOFLIBERTY_API bool IsBuildingInReach(const FHOLUnitTypeConstants& Attacker, double EdgeDistance);

	/**
	 * Radius (cm, its edge to the box) within which the seeker takes on an enemy building by itself, when it found no enemy
	 * unit: 0 without a building attack; on hold, its building reach; on an attack-move, its aggression radius or
	 * AttackMoveRadius; at rest, its aggression radius (0: none, the Peasant); never less than its building reach.
	 */
	HEROESOFLIBERTY_API float GetBuildingSearchRadius(const FSeeker& Seeker, const FHOLUnitTypeConstants& Attacker);

	/** An enemy building near the seeker (the processor fills them from the step's buildings). */
	struct FBuildingCandidate
	{
		/** Tie-break: a stable id (the actor's unique id). */
		uint64 Id = 0;
		FBox2D Box = FBox2D(ForceInit);
	};

	/**
	 * The building the seeker takes on by itself, an index into Candidates or INDEX_NONE: within SearchRadius of its edge,
	 * the nearest by edge distance, then smallest Id. Only called with no enemy unit chosen (a unit always comes first).
	 */
	HEROESOFLIBERTY_API int32 ChooseBuilding(const FVector2D& Position, float Radius, float SearchRadius, TConstArrayView<FBuildingCandidate> Candidates);

	/** Where a unit walks to strike a building: the box's point nearest it, pushed out by its radius and Gap (cm). */
	HEROESOFLIBERTY_API FVector2D GetBuildingApproachPoint(const FVector2D& Position, float Radius, const FBox2D& Box, double Gap);
	/**
	 * EtapeDefini O.7b loop 3 (the crowd wraps round, AoE4: "ils entourent de plus en plus"): where a melee out of reach walks
	 * to strike a building: on the box pushed out by its radius and Gap (the round), the point along it nearest the one facing
	 * it that no striking body (centres Bodies, radii BodyRadii) covers, searched both ways round a radius at a time; the
	 * facing one when free, or when the whole round is covered. A body covers a point when on it, or (loop 6) in the column
	 * out from it, as wide as the two bodies, as deep as the walker stands or as the reach goes (Reach: the edge's building
	 * reach measured from Box; loop 9), whichever is deeper: the crowd's back rows bar the walk there as well, and the
	 * columns round the corner are judged like the one ahead.
	 */
	HEROESOFLIBERTY_API FVector2D GetFreeBuildingWallPoint(const FVector2D& Position, float Radius, const FBox2D& Box, double Gap, double Reach,
		TConstArrayView<FVector2D> Bodies, TConstArrayView<float> BodyRadii);

	// Shots (J7c.2, J7c-Q1 to Q4). A shooter's hit is an arrow: released at the strike's moment of the gesture (the
	// wind-up, the type's AttackWindUpPercent of the cadence, 12 % for the Archer), it flies the centre-to-centre
	// distance of the shooter and its target at release at the arrow speed, and always hits: its damage (GetDamage, the armour of its attack type, reckoned at release)
	// falls on the target when it lands, lost when the target died meanwhile. No minimum reach, no height bonus. The client
	// reckons the same flight from the same positions (J7c.2b): nothing more is replicated.

	/** Reach (cm) above which a unit's hits are shots (an arrow in flight; the armour taken off comes from its AttackType, D345): the Archer's 15 m, not a melee's 50 cm. */
	inline constexpr float RangedReach = 200.f;

	/** Speed of an arrow by default (cm/s): 37.5 m/s (J7c-Q2 25 m/s, x1.5 at the trial of D345), hol.Combat.ArrowSpeed. */
	inline constexpr float DefaultArrowSpeed = 3750.f;

	/** A unit of this reach shoots (AttackRange > RangedReach). Inline (perf): met per neighbour. */
	inline bool IsRangedAttack(const float AttackRange)
	{
		return AttackRange > RangedReach;
	}

	/** Radius (cm, edge to edge) within which an idle soldier comes to a fighting friend's aid by default (hol.Combat.AidRadius). */
	inline constexpr float DefaultAidRadius = 2000.f;

	/** A friend fighting for ShouldComeToAid: striking, or taken on by an enemy (its attackers, or one that hit it). */
	HEROESOFLIBERTY_API bool IsFriendFighting(bool bAttacking, int32 AttackerCount, bool bHasLastAttacker);

	/**
	 * How long (ms, simulation steps) a unit remembers the last enemy that hit it (FHOLCombatFragment::LastAttacker: the
	 * riposte beyond its search radius) without a new hit (EtapeDefini S): past it, an attacker that walked off out of its
	 * search radius is forgotten (no chase across the map, no "fighting" friend for the aid). The fog (M7) will add the
	 * oblivion out of sight.
	 */
	inline constexpr int32 LastAttackerMemoryMs = 5000;

	/**
	 * Whether the last attacker is still remembered at StepIndex: its last hit (LastHitStep; -1: none) less than
	 * LastAttackerMemoryMs ago, in whole steps of StepMs (100 steps at 20 Hz: forgotten at the 100th). A dead or gone
	 * attacker is forgotten at once by the processor, whatever this says.
	 */
	HEROESOFLIBERTY_API bool KeepsLastAttacker(int64 StepIndex, int64 LastHitStep, int32 StepMs);

	/**
	 * Coming to a friend's aid (MV.4, choice C, AoE4): a melee soldier free to seek by itself (Auto: not on Hold nor an
	 * attack-move; a Peasant has no aggression), with no target found, goes to help a friend fighting within AidRadius
	 * (FightingFriendDistance, edge to edge; negative: none). AidRadius 0: off.
	 */
	HEROESOFLIBERTY_API bool ShouldComeToAid(ESeekMode Mode, bool bRanged, float AggressionRadius, bool bHasTarget, double FightingFriendDistance, float AidRadius);

	/** Flight of an arrow (whole ms): the 2D distance From -> To (centres, at release) over SpeedCmPerSecond, rounded. */
	HEROESOFLIBERTY_API int32 GetArrowFlightMilliseconds(const FVector2D& From, const FVector2D& To, float SpeedCmPerSecond);

	/**
	 * Steps from the release to the landing: the flight rounded up to whole steps, 1 at least (an arrow released at step S
	 * lands at the start of step S + GetArrowFlightSteps, never in its own step).
	 */
	HEROESOFLIBERTY_API int32 GetArrowFlightSteps(int32 FlightMs, int32 StepMs);

	/** What an arrow does when it lands. */
	enum class EArrowOutcome : uint8
	{
		/** Its target died or is gone meanwhile: lost, no damage (J7c-Q2). */
		Lost,
		/** It hurt its target, which still stands. */
		Hit,
		/** It took its target to 0: the target dies (the dying path, as in melee). */
		Kill,
	};

	/** An arrow landing: on a standing target (bTargetStands, Health > 0) Damage comes off Health; Lost otherwise. */
	HEROESOFLIBERTY_API EArrowOutcome ResolveArrow(bool bTargetStands, float& Health, float Damage);

	/** hol.Combat.ArrowSpeed (cm/s, 1 at least): not a pure rule, the setting the server and the client both read. */
	HEROESOFLIBERTY_API float GetArrowSpeed();
}
