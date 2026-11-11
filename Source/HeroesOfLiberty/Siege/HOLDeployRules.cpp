// Copyright Epic Games, Inc. All Rights Reserved.

#include "Siege/HOLDeployRules.h"

// The C++ twin of Script/Siege/HOLDeployRules.as (Palier3InProgress.md G5): same rules, same cases (Tests/HOLDeployRulesTests.cpp).
namespace HOLDeploy
{
	namespace
	{
		FDecision Make(const EAction Action, const EPhase NextPhase, const int32 Reason = 0)
		{
			FDecision Decision;
			Decision.Action = Action;
			Decision.NextPhase = NextPhase;
			Decision.Reason = Reason;
			return Decision;
		}
	}

	EPhase PhaseAt(const int32 Index)
	{
		switch (Index)
		{
			case 0: return EPhase::Packed;
			case 1: return EPhase::Deploying;
			case 2: return EPhase::Deployed;
			default: return EPhase::Packing;
		}
	}

	FProfile DefaultProfile()
	{
		FProfile Profile;
		Profile.DeployMs = ProposedDeployMs;
		Profile.PackMs = ProposedPackMs;
		Profile.MinRangeCm = ProposedMinRangeCm;
		Profile.MaxRangeCm = ProposedMaxRangeCm;
		return Profile;
	}

	bool CanFire(const FState& State)
	{
		return State.Phase == EPhase::Deployed;
	}

	bool CanMove(const FState& State)
	{
		return State.Phase == EPhase::Packed;
	}

	bool CanAct(const FState& State)
	{
		return State.Phase == EPhase::Packed || State.Phase == EPhase::Deployed;
	}

	bool CanAim(const FProfile& Profile, const int32 DistanceCm)
	{
		return DistanceCm >= Profile.MinRangeCm && DistanceCm <= Profile.MaxRangeCm;
	}

	int32 Shortened(const int32 BaseMs, const int32 CounterweightPercent)
	{
		const int32 Percent = FMath::Clamp(CounterweightPercent, 0, 100);
		return FMath::Max(0, BaseMs * (100 - Percent) / 100);
	}

	int32 DeployDelayMs(const FProfile& Profile, const int32 CounterweightPercent)
	{
		return Shortened(Profile.DeployMs, CounterweightPercent);
	}

	int32 PackDelayMs(const FProfile& Profile, const int32 CounterweightPercent)
	{
		return Shortened(Profile.PackMs, CounterweightPercent);
	}

	FState Advance(const FState& State, const int32 DtMs, const FProfile& Profile, const int32 CounterweightPercent)
	{
		FState Next = State;
		if (State.Phase == EPhase::Deploying)
		{
			Next.ElapsedMs = State.ElapsedMs + FMath::Max(0, DtMs);
			if (Next.ElapsedMs >= DeployDelayMs(Profile, CounterweightPercent))
			{
				Next.Phase = EPhase::Deployed;
				Next.ElapsedMs = 0;
			}
		}
		else if (State.Phase == EPhase::Packing)
		{
			Next.ElapsedMs = State.ElapsedMs + FMath::Max(0, DtMs);
			if (Next.ElapsedMs >= PackDelayMs(Profile, CounterweightPercent))
			{
				Next.Phase = EPhase::Packed;
				Next.ElapsedMs = 0;
			}
		}
		else
		{
			Next.ElapsedMs = 0;
		}
		return Next;
	}

	FDecision Decide(const FState& State, const EOrder Order, const int32 DistanceCm, const FProfile& Profile)
	{
		const EPhase Phase = State.Phase;
		if (Order == EOrder::Move)
		{
			switch (Phase)
			{
				case EPhase::Packed: return Make(EAction::March, Phase);
				case EPhase::Deployed: return Make(EAction::Pack, EPhase::Packing);
				case EPhase::Deploying: return Make(EAction::CancelDeploy, EPhase::Packed);
				default: return Make(EAction::Wait, Phase);
			}
		}
		if (DistanceCm < Profile.MinRangeCm)
		{
			return Make(EAction::Refuse, Phase, TooClose);
		}
		const bool bInReach = DistanceCm <= Profile.MaxRangeCm;
		switch (Phase)
		{
			case EPhase::Packed:
				return bInReach ? Make(EAction::Deploy, EPhase::Deploying) : Make(EAction::March, Phase);
			case EPhase::Deploying:
				return bInReach ? Make(EAction::Wait, Phase) : Make(EAction::CancelDeploy, EPhase::Packed);
			case EPhase::Deployed:
				return bInReach ? Make(EAction::Fire, Phase) : Make(EAction::Pack, EPhase::Packing);
			default:
				return Make(EAction::Wait, Phase);
		}
	}

	FDecision Toggle(const FState& State)
	{
		switch (State.Phase)
		{
			case EPhase::Packed: return Make(EAction::Deploy, EPhase::Deploying);
			case EPhase::Deployed: return Make(EAction::Pack, EPhase::Packing);
			case EPhase::Deploying: return Make(EAction::CancelDeploy, EPhase::Packed);
			default: return Make(EAction::Wait, State.Phase);
		}
	}

	FState Apply(const FState& State, const FDecision& Decision)
	{
		FState Next = State;
		if (Decision.NextPhase != State.Phase)
		{
			Next.Phase = Decision.NextPhase;
			Next.ElapsedMs = 0;
		}
		return Next;
	}
}
