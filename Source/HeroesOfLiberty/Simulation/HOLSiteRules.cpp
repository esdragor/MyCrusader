// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLSiteRules.h"

FHOLSiteView HOLSite::MakeView(const int32 Index, const FVector2D& Location, const float Radius, const FHOLSiteState& State, const int32 HoldSteps)
{
	FHOLSiteView View;
	View.Index = static_cast<uint8>(FMath::Clamp(Index, 0, 255));
	View.X = static_cast<float>(Location.X);
	View.Y = static_cast<float>(Location.Y);
	View.Radius = Radius;
	View.Owner = static_cast<int8>(State.Owner);
	View.Taker = static_cast<int8>(State.Taker);
	View.FillPercent = static_cast<uint8>(HoldSteps > 0 ? FMath::Clamp(static_cast<int32>(static_cast<int64>(State.Held) * 100 / HoldSteps), 0, 99) : 0);
	return View;
}

int32 HOLSite::GoldStep(int32& Carry, const int32 Sites, const int32 RatePerMinute, const int32 StepsPerSecond)
{
	const int32 StepsPerMinute = 60 * FMath::Max(1, StepsPerSecond);
	Carry += FMath::Max(0, Sites) * FMath::Max(0, RatePerMinute);
	const int32 Award = Carry / StepsPerMinute;
	Carry -= Award * StepsPerMinute;
	return Award;
}

void HOLSite::ResetCounts(TArray<int32>& Takers, TArray<int32>& Disturbers, const int32 Slots)
{
	Takers.Init(0, FMath::Max(0, Slots));
	Disturbers.Init(0, FMath::Max(0, Slots));
}

void HOLSite::AddUnit(TArray<int32>& Takers, TArray<int32>& Disturbers, const int32 Slot, const bool bCanTake, const bool bIsPeasant)
{
	if (bIsPeasant || Slot < 0 || Slot >= Takers.Num() || Slot >= Disturbers.Num())
	{
		return;
	}
	++Disturbers[Slot];
	if (bCanTake)
	{
		++Takers[Slot];
	}
}
