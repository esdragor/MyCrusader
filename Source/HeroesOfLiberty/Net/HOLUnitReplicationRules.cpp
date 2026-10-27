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
	const EHOLResource LoadResource, const int32 LoadAmount, const EHOLResource GatherResource)
{
	FHOLNetUnitState State;
	State.X = ReduceNetValue(Position.X, PositionStep, HorizontalBits);
	State.Y = ReduceNetValue(Position.Y, PositionStep, HorizontalBits);
	State.VelocityX = ReduceNetValue(Velocity.X, VelocityStep, VelocityBits);
	State.VelocityY = ReduceNetValue(Velocity.Y, VelocityStep, VelocityBits);
	State.Activity = Activity;
	// Not gathering: what it gathered does not count, the same state whatever it was.
	State.GatherResource = Activity == EHOLUnitActivity::Gathering ? GatherResource : EHOLResource::Food;
	State.Step = static_cast<uint8>(Step & (NetStepCycle - 1));
	State.bCarrying = LoadAmount > 0;
	// Nothing carried: the resource is not sent, the same state whatever it was.
	State.LoadResource = State.bCarrying ? LoadResource : EHOLResource::Food;
	return State;
}

void FHOLNetUnitState::SetCombat(const int32 InHealth, const int32 MaxHealth, const uint16 InTargetSlot, const bool bInFacesFront,
	const float InFrontYawDegrees, const bool bHealthShare)
{
	bFullHealth = InHealth >= MaxHealth;
	Health = bFullHealth ? 0
		: bHealthShare ? HOLUnitReplication::GetWireHealthShare(InHealth, MaxHealth) : HOLUnitReplication::GetWireHealth(InHealth, MaxHealth);
	TargetSlot = Activity == EHOLUnitActivity::Attacking ? FMath::Min<uint16>(InTargetSlot, InvalidTargetSlot) : 0;
	bFacesFront = bInFacesFront && Activity == EHOLUnitActivity::Idle;
	constexpr int32 NumDirections = 1 << FrontYawBits;
	FrontYaw = bFacesFront
		? static_cast<uint8>(FMath::RoundToInt(FRotator::ClampAxis(InFrontYawDegrees) / 360.f * NumDirections) & (NumDirections - 1)) : 0;
}

float FHOLNetUnitState::GetFrontYaw() const
{
	return FRotator::NormalizeAxis(static_cast<float>(FrontYaw) * 360.f / static_cast<float>(1 << FrontYawBits));
}

uint16 HOLUnitReplication::GetWireHealth(const int32 Health, const int32 MaxHealth)
{
	(void)MaxHealth; // Exact health points: the maximum only bounds them (FitsWireHealth); a level would scale by it.
	return Health <= 0 ? 0 : static_cast<uint16>(FMath::Min<int32>(Health, FHOLNetUnitState::MaxWireHealth));
}

uint16 HOLUnitReplication::GetWireHealthShare(const int32 Health, const int32 MaxHealth)
{
	if (Health <= 0)
	{
		return 0;
	}
	if (MaxHealth <= 0)
	{
		return FHOLNetUnitState::MaxWireHealth;
	}
	const int64 Share = (static_cast<int64>(Health) * FHOLNetUnitState::MaxWireHealth + MaxHealth - 1) / MaxHealth;
	return static_cast<uint16>(FMath::Clamp<int64>(Share, 1, FHOLNetUnitState::MaxWireHealth));
}

bool HOLUnitReplication::IsSameAction(const FHOLNetUnitState& A, const FHOLNetUnitState& B)
{
	return EncodeActivity(A.Activity, A.GatherResource, A.bFacesFront) == EncodeActivity(B.Activity, B.GatherResource, B.bFacesFront)
		&& A.bFullHealth == B.bFullHealth && A.Health == B.Health && A.TargetSlot == B.TargetSlot && A.FrontYaw == B.FrontYaw;
}

FVector FHOLNetUnitState::GetPosition() const
{
	return FVector(X, Y, 0.0) * PositionStep;
}

FVector FHOLNetUnitState::GetVelocity() const
{
	return FVector(VelocityX, VelocityY, 0.0) * VelocityStep;
}

uint8 HOLUnitReplication::EncodeActivity(const EHOLUnitActivity Activity, const EHOLResource GatherResource, const bool bFacesFront)
{
	switch (Activity)
	{
	case EHOLUnitActivity::Building: return BuildingWireValue;
	case EHOLUnitActivity::Attacking: return AttackingWireValue;
	case EHOLUnitActivity::Dying: return DyingWireValue;
	case EHOLUnitActivity::Idle: return bFacesFront ? IdleFacingFrontWireValue : 0;
	default: break;
	}
	return Activity == EHOLUnitActivity::Gathering ? static_cast<uint8>(GatheringWireBase + static_cast<uint8>(GatherResource))
		: static_cast<uint8>(Activity);
}

bool HOLUnitReplication::IsValidActivity(const EHOLUnitActivity Activity)
{
	return static_cast<uint8>(Activity) <= static_cast<uint8>(EHOLUnitActivity::Dying);
}

void HOLUnitReplication::DecodeActivity(const uint8 WireActivity, EHOLUnitActivity& OutActivity, EHOLResource& OutGatherResource)
{
	bool bFacesFront = false;
	DecodeActivity(WireActivity, OutActivity, OutGatherResource, bFacesFront);
}

void HOLUnitReplication::DecodeActivity(const uint8 WireActivity, EHOLUnitActivity& OutActivity, EHOLResource& OutGatherResource, bool& bOutFacesFront)
{
	bOutFacesFront = false;
	OutGatherResource = EHOLResource::Food;
	switch (WireActivity)
	{
	case AttackingWireValue: OutActivity = EHOLUnitActivity::Attacking; return;
	case DyingWireValue: OutActivity = EHOLUnitActivity::Dying; return;
	case IdleFacingFrontWireValue: OutActivity = EHOLUnitActivity::Idle; bOutFacesFront = true; return;
	default: break;
	}
	if (WireActivity > IdleFacingFrontWireValue)
	{
		// A free value (a newer server): shown idle.
		OutActivity = EHOLUnitActivity::Idle;
	}
	else if (WireActivity >= GatheringWireBase)
	{
		OutActivity = EHOLUnitActivity::Gathering;
		OutGatherResource = static_cast<EHOLResource>(WireActivity - GatheringWireBase);
	}
	else
	{
		// Gathering's own value carries Building (J6a.4).
		OutActivity = WireActivity == BuildingWireValue ? EHOLUnitActivity::Building : static_cast<EHOLUnitActivity>(WireActivity);
		OutGatherResource = EHOLResource::Food;
	}
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

bool HOLUnitReplication::SendsVelocity(const EHOLUnitActivity Activity)
{
	return IsWalkingActivity(Activity);
}

FVector HOLUnitReplication::PredictPosition(const FHOLNetUnitState& State, const double StepsElapsed, const float StepSeconds)
{
	return State.GetPosition() + State.GetVelocity() * (StepsElapsed * StepSeconds);
}

EHOLUnitSendReason HOLUnitReplication::GetSendReason(const FHOLUnitSentRecord& Sent, const FVector& Position,
	const EHOLUnitActivity Activity, const uint32 UnitId, const int64 Step, const float StepSeconds, const FHOLUnitSendRules& Rules,
	const EHOLResource GatherResource)
{
	// The fight and the front as last sent: only the activity is compared (the rule of J5c.1).
	FHOLNetUnitState Current = Sent.State;
	Current.Activity = Activity;
	Current.GatherResource = Activity == EHOLUnitActivity::Gathering ? GatherResource : EHOLResource::Food;
	Current.bFacesFront = Sent.State.bFacesFront && Activity == EHOLUnitActivity::Idle;
	Current.FrontYaw = Current.bFacesFront ? Sent.State.FrontYaw : 0;
	Current.TargetSlot = Activity == EHOLUnitActivity::Attacking ? Sent.State.TargetSlot : 0;
	return GetSendReason(Sent, Position, Current, UnitId, Step, StepSeconds, Rules);
}

EHOLUnitSendReason HOLUnitReplication::GetSendReason(const FHOLUnitSentRecord& Sent, const FVector& Position,
	const FHOLNetUnitState& Current, const uint32 UnitId, const int64 Step, const float StepSeconds, const FHOLUnitSendRules& Rules)
{
	if (!IsSameAction(Current, Sent.State))
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
