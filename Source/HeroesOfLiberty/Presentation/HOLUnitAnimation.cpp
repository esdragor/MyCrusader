// Copyright Epic Games, Inc. All Rights Reserved.

#include "Presentation/HOLUnitAnimation.h"

#include "Simulation/HOLUnitTypes.h"

bool HOLUnitAnimation::IsWalkSequence(const FHOLUnitTypeConstants& Type, const int32 Sequence)
{
	return Sequence != INDEX_NONE && (Sequence == Type.WalkSequence || Type.CarryWalkSequences.Contains(Sequence));
}

float HOLUnitAnimation::GetWalkRateShare(const float Speed, const float WalkSpeed)
{
	return WalkSpeed > UE_KINDA_SMALL_NUMBER ? FMath::Clamp(Speed / WalkSpeed, MinWalkRateShare, MaxWalkRateShare) : 1.f;
}

float HOLUnitAnimation::GetStrikeStartPosition(const float Length, const float PlayRate, const float SecondsToHit)
{
	if (Length <= UE_KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}
	const float Position = FMath::Fmod(StrikeHitShare * Length - FMath::Max(SecondsToHit, 0.f) * PlayRate, Length);
	return Position < 0.f ? Position + Length : Position;
}

float HOLUnitAnimation::GetSecondsToHit(const int32 CooldownMs, const int32 StepMs)
{
	if (StepMs <= 0)
	{
		return FMath::Max(CooldownMs, 0) / 1000.f;
	}
	const int32 Steps = FMath::DivideAndRoundUp(FMath::Max(CooldownMs, 0), StepMs);
	return static_cast<float>(Steps * StepMs) / 1000.f;
}

FHOLUnitAnimationChoice HOLUnitAnimation::Choose(const FHOLUnitTypeConstants& Type, const FHOLUnitAnimationInput& Input)
{
	FHOLUnitAnimationChoice Choice;
	if (Input.Activity == EHOLUnitActivity::Dying && Type.FallSequence != INDEX_NONE)
	{
		Choice.Sequence = Type.FallSequence;
		Choice.bOnce = true;
		return Choice;
	}
	if (Input.Activity == EHOLUnitActivity::Attacking && Type.AttackSequence != INDEX_NONE)
	{
		Choice.Sequence = Type.AttackSequence;
		const float Length = Type.SequenceLengths.IsValidIndex(Type.AttackSequence) ? Type.SequenceLengths[Type.AttackSequence] : 0.f;
		Choice.PlayRate = Length > UE_KINDA_SMALL_NUMBER && Type.AttackInterval > UE_KINDA_SMALL_NUMBER ? Length / Type.AttackInterval : 1.f;
		Choice.bStrike = true;
		return Choice;
	}
	if (Input.Activity == EHOLUnitActivity::Gathering)
	{
		const int32 GatherSequence = Type.GatherSequences[Input.GatherResource];
		if (GatherSequence != INDEX_NONE)
		{
			Choice.Sequence = GatherSequence;
			Choice.PlayRate = Type.GatherPlayRate;
			Choice.bFacesSource = true;
			Choice.Prop = static_cast<uint8>(Type.GatherProps[Input.GatherResource]);
			return Choice;
		}
	}
	if (Input.Activity == EHOLUnitActivity::Building && Type.BuildSequence != INDEX_NONE)
	{
		Choice.Sequence = Type.BuildSequence;
		Choice.PlayRate = Type.BuildPlayRate;
		Choice.bFacesSite = true;
		Choice.Prop = static_cast<uint8>(Type.BuildProp);
		return Choice;
	}

	const bool bWalking = IsWalkSequence(Type, Input.PlayingSequence);
	const bool bWantsWalk = bWalking ? Input.Speed >= Type.Speed * WalkStopSpeedShare : Input.Speed > Type.Speed * WalkStartSpeedShare;
	if (!bWantsWalk)
	{
		const int32 CarryIdleSequence = Input.bCarrying ? Type.CarryIdleSequences[Input.LoadResource] : INDEX_NONE;
		if (CarryIdleSequence != INDEX_NONE)
		{
			Choice.Sequence = CarryIdleSequence;
			Choice.Prop = static_cast<uint8>(Type.CarryProps[Input.LoadResource]);
		}
		else
		{
			Choice.Sequence = Type.IdleSequence;
		}
		return Choice;
	}
	const int32 CarryWalkSequence = Input.bCarrying ? Type.CarryWalkSequences[Input.LoadResource] : INDEX_NONE;
	const float MeshScale = FMath::Max(Type.MeshScale, UE_KINDA_SMALL_NUMBER) / GetWalkRateShare(Input.Speed, Input.WalkSpeed);
	if (CarryWalkSequence != INDEX_NONE)
	{
		Choice.Sequence = CarryWalkSequence;
		Choice.PlayRate = Type.CarryWalkPlayRate / MeshScale;
		Choice.Prop = static_cast<uint8>(Type.CarryProps[Input.LoadResource]);
	}
	else
	{
		Choice.Sequence = Type.WalkSequence;
		Choice.PlayRate = Type.WalkPlayRate / MeshScale;
	}
	return Choice;
}
