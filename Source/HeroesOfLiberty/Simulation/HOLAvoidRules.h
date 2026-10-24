// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/FunctionFwd.h"

/**
 * Pure rules of a walker's remembered detour around a body ahead (D293, MV.2): it keeps its side and its turn while the
 * same body is ahead, and eases back towards its place by one turn a step when that course is clear. Without the memory,
 * each step started again from the straight course: two walkers turned, the way looked clear, both went straight and
 * met again, their heading flipping every 2 steps. No engine state: tested on their own.
 */
namespace HOLAvoid
{
	/** Part of the contact distance under which a body ahead is met face to face (D293, MV.2). */
	inline constexpr double FaceToFaceScale = 0.15;

	/**
	 * The hand a walker turns to around a body ahead (+1: its right, Ahead rotated by positive angles; -1: its left):
	 * away from it; face to face (the body's offset across the way under FaceToFaceScale of the contact distance) on
	 * its own right, so two walkers face to face take opposite sides of the world and pass each other.
	 */
	HEROESOFLIBERTY_API double ChooseDetourSide(const FVector2D& Ahead, const FVector2D& ToBlocker, double ContactDistance);

	/** A walker's part at a crossing (MV.2, user's choice A): on its course, or behind the other. */
	enum class ECrossingRole : uint8
	{
		/** Not a crossing (face to face, the same way, no crossing point ahead): the detour side as before. */
		None,
		/** First at the crossing point: no detour for that walker (but for a contact very close, as a safety). */
		KeepCourse,
		/** Second: passes behind it, turning to the side it comes from. */
		PassBehind,
	};

	struct FCrossingChoice
	{
		ECrossingRole Role = ECrossingRole::None;

		/** PassBehind's hand (+1 right, -1 left): towards the side the other comes from. */
		double Side = 0.;
	};

	/** Below this sine of the angle between the two ways (30 degrees), face to face or the same way: not a crossing. */
	inline constexpr double CrossingMinSine = 0.5;

	/** Arrivals at the crossing point closer than this (s, 4 steps at 20 Hz) are together: priority to the right. */
	inline constexpr double CrossingTogetherTime = 0.2;

	/** Part of the horizon under which one keeping its course still turns from the other, as a safety (0.2 s of 0.5 s). */
	inline constexpr double KeepCourseSafetyScale = 0.4;

	/**
	 * The part of a walker at MyPosition going at MyVelocity against another at OtherPosition going at OtherVelocity (its
	 * last step's velocity), both on straight lines: the one reaching their crossing point first keeps its course, the
	 * second passes behind it; together (within CrossingTogetherTime), the one having the other on its right passes
	 * behind it. One already past the point keeps its course. Face to face, the same way or no crossing ahead: None.
	 */
	HEROESOFLIBERTY_API FCrossingChoice ChooseCrossingRole(const FVector2D& MyPosition, const FVector2D& MyVelocity, const FVector2D& OtherPosition, const FVector2D& OtherVelocity);

	/** Steps the eased course must be clear in a row before the detour eases back by one turn (MV.2: 0.2 s at 20 Hz). */
	inline constexpr int32 EaseBackSteps = 4;

	/**
	 * The detour's turn this step (in angle steps, on the remembered hand, RememberedTurn >= 1): one turn back towards the
	 * place (0: the straight course, the detour is over) once that course was clear EaseBackSteps steps in a row (counted
	 * in ClearSteps; the other walker, predicted from its last velocity, made it look clear every other step), else the
	 * remembered turn, else at once wider, up to MaxTurns. INDEX_NONE when no turn up to MaxTurns is clear.
	 */
	HEROESOFLIBERTY_API int32 NextDetourTurn(int32 RememberedTurn, int32 MaxTurns, uint8& ClearSteps, TFunctionRef<bool(int32 Turn)> IsClear);

	/**
	 * The detour's turn when no turn up to MaxTurns is clear: against an enemy ahead (not pushed, D293, MV.4), one turn
	 * wider up to MaxTurns, held along it; else one turn back towards the place, pushing (MV.2).
	 */
	HEROESOFLIBERTY_API int32 TurnWithoutClearWay(int32 RememberedTurn, int32 MaxTurns, bool bEnemyAhead);

	/**
	 * Time (s) at which two bodies come into contact, the other one at RelativePosition from this one and this one moving
	 * at RelativeVelocity towards it: never (the largest double) if they do not meet or move apart; 0 when already within
	 * ContactDistance and closing in on a body pressed against that counts (bPressedCounts: an enemy walked around; MV.4: never, every way into an enemy pressed against looked clear and the Paysans
	 * wedged into a wall of enemies); never on a friend (it gives way: counting it held back Paysans abreast before a narrow way).
	 */
	inline double TimeToContact(const FVector2D& RelativePosition, const FVector2D& RelativeVelocity, const double ContactDistance, const bool bPressedCounts)
	{
		// Inline (perf, 2026-10-24): the avoidance calls it some 200 times per walker and step.
		const double SpeedSquared = RelativeVelocity.SizeSquared();
		const double Approach = FVector2D::DotProduct(RelativePosition, RelativeVelocity);
		const double Gap = RelativePosition.SizeSquared() - FMath::Square(ContactDistance);
		if (Approach <= 0. || SpeedSquared <= UE_KINDA_SMALL_NUMBER)
		{
			return TNumericLimits<double>::Max();
		}
		if (Gap <= 0.)
		{
			return bPressedCounts ? 0. : TNumericLimits<double>::Max();
		}
		const double Discriminant = FMath::Square(Approach) - SpeedSquared * Gap;
		return Discriminant < 0. ? TNumericLimits<double>::Max() : (Approach - FMath::Sqrt(Discriminant)) / SpeedSquared;
	}

	/** A building's body as a walker sees it (D324's body, turned by the building's yaw: Cos, Sin). */
	struct FFootprintBox
	{
		FVector2D Centre = FVector2D::ZeroVector;
		FVector2D HalfExtent = FVector2D::ZeroVector;
		double Cos = 1.;
		double Sin = 0.;
	};

	/** Distance from Point to Box widened by Keep (a body's radius), negative inside. */
	HEROESOFLIBERTY_API double GetFootprintDistance(const FFootprintBox& Box, double Keep, const FVector2D& Point);

	/**
	 * Whether a detour going Distance along Way (unit) from Position leads into Box widened by Keep: a point on it (half
	 * way, the end) inside and deeper than Position. One along the wall or turning away from it is not led in. EtapeDefini
	 * E: the avoidance did not see the buildings, a follower held its remembered turn into a house's wall (MV.2) and a
	 * walker turned wide from still builders straight into the site's wall (MV.4); the keep-out put them back.
	 */
	HEROESOFLIBERTY_API bool LeadsIntoFootprint(const FFootprintBox& Box, double Keep, const FVector2D& Position, const FVector2D& Way, double Distance);
}
