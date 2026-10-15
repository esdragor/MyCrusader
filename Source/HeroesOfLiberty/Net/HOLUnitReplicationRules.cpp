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

FHOLNetUnitState FHOLNetUnitState::Make(const FVector& Position, const FVector& Velocity, const EHOLUnitActivity Activity, const int64 Step,
	const EHOLResource LoadResource, const int32 LoadAmount)
{
	FHOLNetUnitState State;
	State.X = ReduceNetValue(Position.X, PositionStep, HorizontalBits);
	State.Y = ReduceNetValue(Position.Y, PositionStep, HorizontalBits);
	State.VelocityX = ReduceNetValue(Velocity.X, VelocityStep, VelocityBits);
	State.VelocityY = ReduceNetValue(Velocity.Y, VelocityStep, VelocityBits);
	State.Activity = Activity;
	State.Step = static_cast<uint8>(Step & (NetStepCycle - 1));
	State.bCarrying = LoadAmount > 0;
	// Nothing carried: the resource is not sent, the same state whatever it was.
	State.LoadResource = State.bCarrying ? LoadResource : EHOLResource::Food;
	return State;
}

FVector FHOLNetUnitState::GetPosition() const
{
	return FVector(X, Y, 0.0) * PositionStep;
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
	if (FVector::DistSquared2D(Predicted, Position) > FMath::Square(Rules.ErrorThreshold))
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
	NumFarLateSamples = 0;
	bValid = false;
}

void FHOLServerClock::AddSample(const int64 ServerStep, const double LocalSeconds, const double OneWaySeconds)
{
	// Offsets are local time minus server time: a later sample has a larger offset.
	const double SampleOffset = LocalSeconds - (static_cast<double>(ServerStep) * StepSeconds + OneWaySeconds);
	const double Gap = SampleOffset - Offset;
	if (Gap > SnapSeconds && bValid)
	{
		// Far later: a hitch delayed it, unless it lasts.
		if (++NumFarLateSamples >= LateSamplesToFollow)
		{
			Offset = SampleOffset;
			NumFarLateSamples = 0;
		}
		return;
	}
	NumFarLateSamples = 0;
	if (!bValid || Gap < -SnapSeconds)
	{
		Offset = SampleOffset;
		bValid = true;
	}
	else
	{
		Offset += Gap * Blend;
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
