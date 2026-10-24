// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLAvoidRules.h"

#include "Templates/Function.h"

double HOLAvoid::ChooseDetourSide(const FVector2D& Ahead, const FVector2D& ToBlocker, const double ContactDistance)
{
	// Ahead is a unit vector: the cross product is the body's offset across the way (positive: on the right).
	const double Across = FVector2D::CrossProduct(Ahead, ToBlocker);
	if (FMath::Abs(Across) < FaceToFaceScale * ContactDistance)
	{
		return 1.;
	}
	return Across > 0. ? -1. : 1.;
}

HOLAvoid::FCrossingChoice HOLAvoid::ChooseCrossingRole(const FVector2D& MyPosition, const FVector2D& MyVelocity, const FVector2D& OtherPosition, const FVector2D& OtherVelocity)
{
	FCrossingChoice Choice;
	const FVector2D Mine = MyVelocity.GetSafeNormal();
	const FVector2D Its = OtherVelocity.GetSafeNormal();
	const double Sine = FVector2D::CrossProduct(Mine, Its);
	if (Mine.IsNearlyZero() || Its.IsNearlyZero() || FMath::Abs(Sine) < CrossingMinSine)
	{
		return Choice;
	}
	// Times (s) to the crossing point of the two lines: MyPosition + Mine * T = OtherPosition + Its * S.
	const FVector2D ToOther = OtherPosition - MyPosition;
	const double Denominator = FVector2D::CrossProduct(MyVelocity, OtherVelocity);
	const double MyTime = FVector2D::CrossProduct(ToOther, OtherVelocity) / Denominator;
	const double ItsTime = FVector2D::CrossProduct(ToOther, MyVelocity) / Denominator;
	if (MyTime < 0. && ItsTime < 0.)
	{
		return Choice;
	}
	bool bFirst;
	if (MyTime < 0. || ItsTime < 0.)
	{
		bFirst = MyTime < 0.;
	}
	else if (FMath::Abs(MyTime - ItsTime) <= CrossingTogetherTime)
	{
		// Priority to the right: the one having the other on its right gives way.
		bFirst = FVector2D::CrossProduct(Mine, ToOther) <= 0.;
	}
	else
	{
		bFirst = MyTime < ItsTime;
	}
	Choice.Role = bFirst ? ECrossingRole::KeepCourse : ECrossingRole::PassBehind;
	// It moves across my way towards my right (Sine > 0): it comes from my left, behind it is on my left.
	Choice.Side = bFirst ? 0. : (Sine > 0. ? -1. : 1.);
	return Choice;
}

int32 HOLAvoid::NextDetourTurn(const int32 RememberedTurn, const int32 MaxTurns, uint8& ClearSteps, const TFunctionRef<bool(int32 Turn)> IsClear)
{
	const int32 Remembered = FMath::Max(RememberedTurn, 1);
	if (IsClear(Remembered - 1))
	{
		// Eased back only after a run of clear steps, or at once when the remembered course itself is no longer clear.
		if (++ClearSteps >= EaseBackSteps || !IsClear(Remembered))
		{
			ClearSteps = 0;
			return Remembered - 1;
		}
		return Remembered;
	}
	ClearSteps = 0;
	for (int32 Turn = Remembered; Turn <= MaxTurns; ++Turn)
	{
		if (IsClear(Turn))
		{
			return Turn;
		}
	}
	return INDEX_NONE;
}

int32 HOLAvoid::TurnWithoutClearWay(const int32 RememberedTurn, const int32 MaxTurns, const bool bEnemyAhead)
{
	return bEnemyAhead ? FMath::Min(RememberedTurn + 1, MaxTurns) : RememberedTurn - 1;
}

double HOLAvoid::GetFootprintDistance(const FFootprintBox& Box, const double Keep, const FVector2D& Point)
{
	// In the box's frame (as the keep-out's), then the distance to the rectangle, rounded by Keep.
	const FVector2D Offset = Point - Box.Centre;
	const FVector2D Local(Offset.X * Box.Cos + Offset.Y * Box.Sin, -Offset.X * Box.Sin + Offset.Y * Box.Cos);
	const FVector2D Out(FMath::Abs(Local.X) - Box.HalfExtent.X, FMath::Abs(Local.Y) - Box.HalfExtent.Y);
	return FVector2D(FMath::Max(Out.X, 0.), FMath::Max(Out.Y, 0.)).Size() + FMath::Min(FMath::Max(Out.X, Out.Y), 0.) - Keep;
}

bool HOLAvoid::LeadsIntoFootprint(const FFootprintBox& Box, const double Keep, const FVector2D& Position, const FVector2D& Way, const double Distance)
{
	// Deeper by more than a centimetre: along the wall, the keep-out's own slack is no wall.
	constexpr double DeeperSlack = 1.;
	const double Start = GetFootprintDistance(Box, Keep, Position);
	for (const double Part : { 0.5, 1. })
	{
		const double There = GetFootprintDistance(Box, Keep, Position + Way * (Distance * Part));
		if (There < 0. && There < Start - DeeperSlack)
		{
			return true;
		}
	}
	return false;
}
