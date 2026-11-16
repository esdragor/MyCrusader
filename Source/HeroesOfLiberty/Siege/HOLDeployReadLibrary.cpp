// Copyright Epic Games, Inc. All Rights Reserved.

#include "Siege/HOLDeployReadLibrary.h"

#include "Siege/HOLDeployRules.h"

namespace
{
	HOLDeploy::EPhase ToRule(const EHOLDeployPhase Phase) { return static_cast<HOLDeploy::EPhase>(static_cast<uint8>(Phase)); }
	EHOLDeployPhase FromRule(const HOLDeploy::EPhase Phase) { return static_cast<EHOLDeployPhase>(static_cast<uint8>(Phase)); }
	HOLDeploy::EOrder ToRule(const EHOLDeployOrder Order) { return static_cast<HOLDeploy::EOrder>(static_cast<uint8>(Order)); }
	EHOLDeployAction FromRule(const HOLDeploy::EAction Action) { return static_cast<EHOLDeployAction>(static_cast<uint8>(Action)); }
	HOLDeploy::EAction ToRule(const EHOLDeployAction Action) { return static_cast<HOLDeploy::EAction>(static_cast<uint8>(Action)); }

	HOLDeploy::FProfile ToRule(const FHOLDeployProfile& Profile)
	{
		HOLDeploy::FProfile Out;
		Out.DeployMs = Profile.DeployMs;
		Out.PackMs = Profile.PackMs;
		Out.MinRangeCm = Profile.MinRangeCm;
		Out.MaxRangeCm = Profile.MaxRangeCm;
		return Out;
	}

	FHOLDeployProfile FromRule(const HOLDeploy::FProfile& Profile)
	{
		FHOLDeployProfile Out;
		Out.DeployMs = Profile.DeployMs;
		Out.PackMs = Profile.PackMs;
		Out.MinRangeCm = Profile.MinRangeCm;
		Out.MaxRangeCm = Profile.MaxRangeCm;
		return Out;
	}

	HOLDeploy::FState ToRule(const FHOLDeployState& State)
	{
		HOLDeploy::FState Out;
		Out.Phase = ToRule(State.Phase);
		Out.ElapsedMs = State.ElapsedMs;
		return Out;
	}

	FHOLDeployState FromRule(const HOLDeploy::FState& State)
	{
		FHOLDeployState Out;
		Out.Phase = FromRule(State.Phase);
		Out.ElapsedMs = State.ElapsedMs;
		return Out;
	}

	HOLDeploy::FDecision ToRule(const FHOLDeployDecision& Decision)
	{
		HOLDeploy::FDecision Out;
		Out.Action = ToRule(Decision.Action);
		Out.Reason = Decision.Reason;
		Out.NextPhase = ToRule(Decision.NextPhase);
		return Out;
	}

	FHOLDeployDecision FromRule(const HOLDeploy::FDecision& Decision)
	{
		FHOLDeployDecision Out;
		Out.Action = FromRule(Decision.Action);
		Out.Reason = Decision.Reason;
		Out.NextPhase = FromRule(Decision.NextPhase);
		return Out;
	}
}

int32 UHOLDeployReadLibrary::GetTooClose() { return HOLDeploy::TooClose; }

EHOLDeployPhase UHOLDeployReadLibrary::PhaseAt(const int32 Index) { return FromRule(HOLDeploy::PhaseAt(Index)); }

FHOLDeployProfile UHOLDeployReadLibrary::DefaultProfile() { return FromRule(HOLDeploy::DefaultProfile()); }

bool UHOLDeployReadLibrary::CanFire(const FHOLDeployState& State) { return HOLDeploy::CanFire(ToRule(State)); }

bool UHOLDeployReadLibrary::CanMove(const FHOLDeployState& State) { return HOLDeploy::CanMove(ToRule(State)); }

bool UHOLDeployReadLibrary::CanAct(const FHOLDeployState& State) { return HOLDeploy::CanAct(ToRule(State)); }

bool UHOLDeployReadLibrary::CanAim(const FHOLDeployProfile& Profile, const int32 DistanceCm) { return HOLDeploy::CanAim(ToRule(Profile), DistanceCm); }

int32 UHOLDeployReadLibrary::Shortened(const int32 BaseMs, const int32 CounterweightPercent) { return HOLDeploy::Shortened(BaseMs, CounterweightPercent); }

int32 UHOLDeployReadLibrary::DeployDelayMs(const FHOLDeployProfile& Profile, const int32 CounterweightPercent)
{
	return HOLDeploy::DeployDelayMs(ToRule(Profile), CounterweightPercent);
}

int32 UHOLDeployReadLibrary::PackDelayMs(const FHOLDeployProfile& Profile, const int32 CounterweightPercent)
{
	return HOLDeploy::PackDelayMs(ToRule(Profile), CounterweightPercent);
}

FHOLDeployState UHOLDeployReadLibrary::Advance(const FHOLDeployState& State, const int32 DtMs, const FHOLDeployProfile& Profile, const int32 CounterweightPercent)
{
	return FromRule(HOLDeploy::Advance(ToRule(State), DtMs, ToRule(Profile), CounterweightPercent));
}

FHOLDeployDecision UHOLDeployReadLibrary::Decide(const FHOLDeployState& State, const EHOLDeployOrder Order, const int32 DistanceCm, const FHOLDeployProfile& Profile)
{
	return FromRule(HOLDeploy::Decide(ToRule(State), ToRule(Order), DistanceCm, ToRule(Profile)));
}

FHOLDeployDecision UHOLDeployReadLibrary::Toggle(const FHOLDeployState& State) { return FromRule(HOLDeploy::Toggle(ToRule(State))); }

FHOLDeployState UHOLDeployReadLibrary::Apply(const FHOLDeployState& State, const FHOLDeployDecision& Decision)
{
	return FromRule(HOLDeploy::Apply(ToRule(State), ToRule(Decision)));
}