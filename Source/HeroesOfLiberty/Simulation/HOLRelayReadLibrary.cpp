// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLRelayReadLibrary.h"

#include "Match/HOLRelayView.h"
#include "Simulation/HOLRelayRules.h"

namespace
{
	FHOLRelayState StateOf(const int32 Owner, const int32 Taker, const int32 Held)
	{
		FHOLRelayState State;
		State.Owner = Owner;
		State.Taker = Taker;
		State.Held = Held;
		return State;
	}
}

int32 UHOLRelayReadLibrary::FillPercent(const int32 Owner, const int32 Taker, const int32 Held, const int32 HoldSteps)
{
	return HOLRelay::FillPercent(StateOf(Owner, Taker, Held), HoldSteps);
}

FHOLRelayReadView UHOLRelayReadLibrary::MakeView(const FVector2D& Location, const float Radius, const int32 Owner, const int32 Taker, const int32 Held, const int32 HoldSteps)
{
	const FHOLRelayView View = HOLRelay::MakeView(Location, Radius, StateOf(Owner, Taker, Held), HoldSteps);
	FHOLRelayReadView Out;
	Out.X = View.X;
	Out.Y = View.Y;
	Out.Radius = View.Radius;
	Out.Owner = View.Owner;
	Out.Taker = View.Taker;
	Out.FillPercent = View.FillPercent;
	Out.Description = View.Describe();
	return Out;
}