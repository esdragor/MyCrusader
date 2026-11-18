// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLRelayRules.h"

int32 HOLRelay::FillPercent(const FHOLRelayState& State, const int32 HoldSteps)
{
	return HoldSteps > 0 ? FMath::Clamp(static_cast<int32>(static_cast<int64>(State.Held) * 100 / HoldSteps), 0, 99) : 0;
}

FHOLRelayView HOLRelay::MakeView(const FVector2D& Location, const float Radius, const FHOLRelayState& State, const int32 HoldSteps)
{
	FHOLRelayView View;
	View.X = static_cast<float>(Location.X);
	View.Y = static_cast<float>(Location.Y);
	View.Radius = Radius;
	View.Owner = static_cast<int8>(State.Owner);
	View.Taker = static_cast<int8>(State.Taker);
	View.FillPercent = static_cast<uint8>(FillPercent(State, HoldSteps));
	return View;
}
