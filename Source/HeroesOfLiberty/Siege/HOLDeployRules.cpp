// Copyright Epic Games, Inc. All Rights Reserved.

#include "Siege/HOLDeployRules.h"

// STUB of the red test (Palier3InProgress.md G5, C++ twin): no behaviour yet, the test must fail on its assertions.
namespace HOLDeploy
{
	EPhase PhaseAt(int32 Index) { return EPhase::Packed; }
	FProfile DefaultProfile() { return FProfile(); }
	bool CanFire(const FState& State) { return false; }
	bool CanMove(const FState& State) { return false; }
	bool CanAct(const FState& State) { return false; }
	bool CanAim(const FProfile& Profile, int32 DistanceCm) { return false; }
	int32 Shortened(int32 BaseMs, int32 CounterweightPercent) { return BaseMs; }
	int32 DeployDelayMs(const FProfile& Profile, int32 CounterweightPercent) { return 0; }
	int32 PackDelayMs(const FProfile& Profile, int32 CounterweightPercent) { return 0; }
	FState Advance(const FState& State, int32 DtMs, const FProfile& Profile, int32 CounterweightPercent) { return State; }
	FDecision Decide(const FState& State, EOrder Order, int32 DistanceCm, const FProfile& Profile) { return FDecision(); }
	FDecision Toggle(const FState& State) { return FDecision(); }
	FState Apply(const FState& State, const FDecision& Decision) { return State; }
}
