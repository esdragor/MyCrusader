// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLUnitReplicationRules.h"

namespace
{
	/** Value / Step rounded, held within a signed integer of Bits bits. */
	int16 ReduceNetValue(const double Value, const float Step, const int32 Bits)
	{
		const int32 Max = (1 << (Bits - 1)) - 1;
		return static_cast<int16>(FMath::Clamp(FMath::RoundToInt(Value / Step), -Max - 1, Max));
	}

	constexpr int64 NetStepCycle = int64(1) << FHOLNetUnitState::StepBits;
}

FHOLNetUnitState FHOLNetUnitState::Make(const FVector& Position, const FVector& Velocity, const EHOLUnitActivity Activity, const int64 Step)
{
	FHOLNetUnitState State;
	State.X = ReduceNetValue(Position.X, PositionStep, HorizontalBits);
	State.Y = ReduceNetValue(Position.Y, PositionStep, HorizontalBits);
	State.Z = ReduceNetValue(Position.Z, PositionStep, HeightBits);
	State.VelocityX = ReduceNetValue(Velocity.X, VelocityStep, VelocityBits);
	State.VelocityY = ReduceNetValue(Velocity.Y, VelocityStep, VelocityBits);
	State.Activity = Activity;
	State.Step = static_cast<uint8>(Step & (NetStepCycle - 1));
	return State;
}

FVector FHOLNetUnitState::GetPosition() const
{
	return FVector(X, Y, Z) * PositionStep;
}

FVector FHOLNetUnitState::GetVelocity() const
{
	return FVector(VelocityX, VelocityY, 0.0) * VelocityStep;
}

int64 HOLUnitReplication::UnwrapStep(const uint8 StepLowBits, const int64 NearStep)
{
	int64 Ahead = (static_cast<int64>(StepLowBits) - NearStep) & (NetStepCycle - 1);
	if (Ahead >= NetStepCycle / 2)
	{
		Ahead -= NetStepCycle;
	}
	return NearStep + Ahead;
}

FVector HOLUnitReplication::PredictPosition(const FHOLNetUnitState& State, const double StepsElapsed, const float StepSeconds)
{
	return State.GetPosition() + State.GetVelocity() * (StepsElapsed * StepSeconds);
}

EHOLUnitSendReason HOLUnitReplication::GetSendReason(const FHOLUnitSentRecord& Sent, const FVector& Position,
	const EHOLUnitActivity Activity, const uint32 UnitId, const int64 Step, const float StepSeconds, const FHOLUnitSendRules& Rules)
{
	if (Activity != Sent.State.Activity)
	{
		return EHOLUnitSendReason::Activity;
	}
	const int64 Elapsed = Step - Sent.Step;
	const FVector Predicted = PredictPosition(Sent.State, static_cast<double>(Elapsed), StepSeconds);
	if (FVector::DistSquared(Predicted, Position) > FMath::Square(Rules.ErrorThreshold))
	{
		return EHOLUnitSendReason::Error;
	}
	const int64 Cycle = FMath::Max(Rules.RefreshSteps, 1);
	if ((Step + UnitId) % Cycle == 0 && Elapsed >= Cycle / 4)
	{
		return EHOLUnitSendReason::Refresh;
	}
	return EHOLUnitSendReason::None;
}

void FHOLServerClock::Reset(const float InStepSeconds)
{
	StepSeconds = InStepSeconds;
	Offset = 0.0;
	bValid = false;
}

void FHOLServerClock::AddSample(const int64 ServerStep, const double LocalSeconds, const double OneWaySeconds)
{
	const double SampleOffset = LocalSeconds - (static_cast<double>(ServerStep) * StepSeconds + OneWaySeconds);
	if (!bValid || FMath::Abs(SampleOffset - Offset) > SnapSeconds)
	{
		Offset = SampleOffset;
		bValid = true;
	}
	else
	{
		Offset += (SampleOffset - Offset) * Blend;
	}
}

double FHOLServerClock::GetStep(const double LocalSeconds) const
{
	return (LocalSeconds - Offset) / StepSeconds;
}

void FHOLDisplayCorrection::Start(const FVector& Displayed, const FVector& Predicted, const double LocalSeconds)
{
	const FVector Gap = Displayed - Predicted;
	Offset = Gap.SizeSquared() > FMath::Square(SnapDistance) ? FVector::ZeroVector : Gap;
	StartSeconds = LocalSeconds;
}

FVector FHOLDisplayCorrection::GetOffset(const double LocalSeconds) const
{
	const double Remaining = 1.0 - (LocalSeconds - StartSeconds) / SmoothingSeconds;
	return Offset * FMath::Clamp(Remaining, 0.0, 1.0);
}
