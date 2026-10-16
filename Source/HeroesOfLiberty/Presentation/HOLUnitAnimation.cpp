// Copyright Epic Games, Inc. All Rights Reserved.

#include "Presentation/HOLUnitAnimation.h"

#include "Simulation/HOLUnitTypes.h"

bool HOLUnitAnimation::IsWalkSequence(const FHOLUnitTypeConstants& Type, const int32 Sequence)
{
	return Sequence != INDEX_NONE && (Sequence == Type.WalkSequence || Type.CarryWalkSequences.Contains(Sequence));
}

FHOLUnitAnimationChoice HOLUnitAnimation::Choose(const FHOLUnitTypeConstants& Type, const FHOLUnitAnimationInput& Input)
{
	FHOLUnitAnimationChoice Choice;
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
	const float MeshScale = FMath::Max(Type.MeshScale, UE_KINDA_SMALL_NUMBER);
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
