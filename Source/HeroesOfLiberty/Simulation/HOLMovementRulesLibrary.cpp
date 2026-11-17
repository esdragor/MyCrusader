// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLMovementRulesLibrary.h"

#include "Dev/HOLSanityRules.h"
#include "Simulation/HOLAvoidRules.h"

// ENVELOPES ONLY: a copy in, one call of the rule, a copy out. No rule is written here.

namespace UE::HeroesOfLiberty::MovementEnvelope
{
	HOLAvoid::FFootprintBox ToBox(const FHOLFootprintBoxView& View)
	{
		HOLAvoid::FFootprintBox Box;
		Box.Centre = View.Centre;
		Box.HalfExtent = View.HalfExtent;
		Box.Cos = View.Cos;
		Box.Sin = View.Sin;
		return Box;
	}
}

using namespace UE::HeroesOfLiberty::MovementEnvelope;

FHOLSanitySettingsView::FHOLSanitySettingsView()
{
	const HOLSanity::FSettings D;
	MinMove = D.MinMove;
	SpinTurn = D.SpinTurn;
	SpinNetRadii = D.SpinNetRadii;
	SpinMinPathRadii = D.SpinMinPathRadii;
	ShakePerSecond = D.ShakePerSecond;
	ShakeNetShare = D.ShakeNetShare;
	JitterZigzagPerSecond = D.JitterZigzagPerSecond;
}

int32 UHOLMovementRulesLibrary::EaseBackSteps()
{
	return HOLAvoid::EaseBackSteps;
}

double UHOLMovementRulesLibrary::ChooseDetourSide(const FVector2D& Ahead, const FVector2D& ToBlocker, const double ContactDistance)
{
	return HOLAvoid::ChooseDetourSide(Ahead, ToBlocker, ContactDistance);
}

FHOLCrossingChoiceView UHOLMovementRulesLibrary::ChooseCrossingRole(const FVector2D& MyPosition, const FVector2D& MyVelocity, const FVector2D& OtherPosition, const FVector2D& OtherVelocity)
{
	const HOLAvoid::FCrossingChoice Choice = HOLAvoid::ChooseCrossingRole(MyPosition, MyVelocity, OtherPosition, OtherVelocity);
	FHOLCrossingChoiceView View;
	View.Role = static_cast<int32>(Choice.Role);
	View.Side = Choice.Side;
	return View;
}

int32 UHOLMovementRulesLibrary::NextDetourTurn(const int32 RememberedTurn, const int32 MaxTurns, int32& ClearSteps, const TArray<bool>& ClearByTurn)
{
	uint8 Steps = static_cast<uint8>(FMath::Clamp(ClearSteps, 0, 255));
	const int32 Result = HOLAvoid::NextDetourTurn(RememberedTurn, MaxTurns, Steps, [&ClearByTurn](const int32 Turn) { return ClearByTurn.IsValidIndex(Turn) && ClearByTurn[Turn]; });
	ClearSteps = Steps;
	return Result;
}

int32 UHOLMovementRulesLibrary::TurnWithoutClearWay(const int32 RememberedTurn, const int32 MaxTurns, const bool bEnemyAhead)
{
	return HOLAvoid::TurnWithoutClearWay(RememberedTurn, MaxTurns, bEnemyAhead);
}

double UHOLMovementRulesLibrary::TimeToContact(const FVector2D& RelativePosition, const FVector2D& RelativeVelocity, const double ContactDistance, const bool bPressedCounts)
{
	return HOLAvoid::TimeToContact(RelativePosition, RelativeVelocity, ContactDistance, bPressedCounts);
}

double UHOLMovementRulesLibrary::TimeNever()
{
	return TNumericLimits<double>::Max();
}

bool UHOLMovementRulesLibrary::IsHeadOn(const FVector2D& Ahead, const FVector2D& ToOther, const FVector2D& OtherVelocity, const double MaxDistance, const double MinClosing)
{
	return HOLAvoid::IsHeadOn(Ahead, ToOther, OtherVelocity, MaxDistance, MinClosing);
}

bool UHOLMovementRulesLibrary::IsPlaceInBody(const FVector2D& Place, const FVector2D& BodyPosition, const double Radius)
{
	return HOLAvoid::IsPlaceInBody(Place, BodyPosition, Radius);
}

bool UHOLMovementRulesLibrary::IsSlotBlockedSeen(const int32 bSlotBlocked, const int32 DetourOrderSerial, const int32 DetourPhase, const int32 OrderSerial, const int32 OrderPhase)
{
	return HOLAvoid::IsSlotBlockedSeen(static_cast<uint8>(bSlotBlocked), static_cast<uint32>(DetourOrderSerial), static_cast<uint8>(DetourPhase), static_cast<uint32>(OrderSerial), static_cast<uint8>(OrderPhase));
}

double UHOLMovementRulesLibrary::GetFootprintDistance(const FHOLFootprintBoxView& Box, const double Keep, const FVector2D& Point)
{
	return HOLAvoid::GetFootprintDistance(ToBox(Box), Keep, Point);
}

bool UHOLMovementRulesLibrary::LeadsIntoFootprint(const FHOLFootprintBoxView& Box, const double Keep, const FVector2D& Position, const FVector2D& Way, const double Distance)
{
	return HOLAvoid::LeadsIntoFootprint(ToBox(Box), Keep, Position, Way, Distance);
}

bool UHOLMovementRulesLibrary::StepLoopPatience(FHOLLoopPatienceView& State, const FVector2D& Asked, const double DeltaTime, const double PatienceDegrees, const double WaitSeconds, const double ForgetSeconds)
{
	HOLAvoid::FLoopPatience Own;
	Own.Heading = State.Heading;
	Own.Turn = State.Turn;
	Own.WaitSeconds = State.WaitSeconds;
	const bool bWait = HOLAvoid::StepLoopPatience(Own, Asked, DeltaTime, PatienceDegrees, WaitSeconds, ForgetSeconds);
	State.Heading = Own.Heading;
	State.Turn = Own.Turn;
	State.WaitSeconds = Own.WaitSeconds;
	return bWait;
}

bool UHOLMovementRulesLibrary::RemembersMemberDetour(const int32 MaxSteps)
{
	return HOLAvoid::RemembersMemberDetour(MaxSteps);
}

bool UHOLMovementRulesLibrary::IsMemberDetourOver(int32& MemberSteps, const int32 MaxSteps)
{
	uint8 Steps = static_cast<uint8>(FMath::Clamp(MemberSteps, 0, 255));
	const bool bOver = HOLAvoid::IsMemberDetourOver(Steps, MaxSteps);
	MemberSteps = Steps;
	return bOver;
}

FHOLSanityResultView UHOLMovementRulesLibrary::Analyse(const FHOLSanityTraceView& Trace, const float StepSeconds, const FHOLSanitySettingsView& Settings)
{
	HOLSanity::FTrace Own;
	Own.Radius = Trace.Radius;
	Own.Places = Trace.Places;
	Own.Ordered = Trace.Ordered;
	Own.Flanking = Trace.Flanking;
	Own.PlaceWalk = Trace.PlaceWalk;
	HOLSanity::FSettings Own2;
	Own2.MinMove = Settings.MinMove;
	Own2.SpinTurn = Settings.SpinTurn;
	Own2.SpinNetRadii = Settings.SpinNetRadii;
	Own2.SpinMinPathRadii = Settings.SpinMinPathRadii;
	Own2.ShakePerSecond = Settings.ShakePerSecond;
	Own2.ShakeNetShare = Settings.ShakeNetShare;
	Own2.JitterZigzagPerSecond = Settings.JitterZigzagPerSecond;
	const HOLSanity::FResult R = HOLSanity::Analyse(Own, StepSeconds, Own2);
	FHOLSanityResultView View;
	View.bSpin = R.bSpin;
	View.bShake = R.bShake;
	View.Path = R.Path;
	View.Net = R.Net;
	View.Turn = R.Turn;
	View.SignedTurn = R.SignedTurn;
	View.Zigzag = R.Zigzag;
	View.Reversals = R.Reversals;
	View.OrderedShare = R.OrderedShare;
	return View;
}

FString UHOLMovementRulesLibrary::JudgeTrack(const int32 Motion, const TArray<double>& Times, const TArray<FVector2D>& Places)
{
	const HOLSanity::FSanityRules Rules;
	HOLSanity::FUnitTrack Track;
	const HOLSanity::EUnitMotion Kind = Motion == 0 ? HOLSanity::EUnitMotion::Ordered : Motion == 1 ? HOLSanity::EUnitMotion::Idle : HOLSanity::EUnitMotion::Busy;
	for (int32 Index = 0; Index < Times.Num() && Index < Places.Num(); ++Index)
	{
		Track.Add(Times[Index], Places[Index], Kind, Rules);
		const FString Fault = Track.Judge(Rules);
		if (!Fault.IsEmpty())
		{
			return Fault;
		}
	}
	return FString();
}

double UHOLMovementRulesLibrary::StallLimitSeconds(const double BaseSeconds, const bool bFinalLegWalk, const double FinalLegSeconds, const double MarginSeconds)
{
	return HOLSanity::StallLimitSeconds(BaseSeconds, bFinalLegWalk, FinalLegSeconds, MarginSeconds);
}

bool UHOLMovementRulesLibrary::StepStall(FHOLStallView& State, const bool bStalledWithRoom, const double StepSeconds, const double LimitSeconds)
{
	HOLSanity::FStall Own;
	Own.Seconds = State.Seconds;
	Own.Longest = State.Longest;
	Own.bTold = State.bTold;
	Own.Runs = State.Runs;
	const bool bTold = Own.Step(bStalledWithRoom, StepSeconds, LimitSeconds);
	State.Seconds = Own.Seconds;
	State.Longest = Own.Longest;
	State.bTold = Own.bTold;
	State.Runs = Own.Runs;
	return bTold;
}

bool UHOLMovementRulesLibrary::StepNoStrike(FHOLNoStrikeView& State, const bool bInReachOfLivingTarget, const int32 Hits, const double StepSeconds, const double LimitSeconds)
{
	HOLSanity::FNoStrike Own;
	Own.Seconds = State.Seconds;
	Own.Longest = State.Longest;
	Own.LastHits = State.LastHits;
	Own.bTold = State.bTold;
	Own.Runs = State.Runs;
	const bool bTold = Own.Step(bInReachOfLivingTarget, Hits, StepSeconds, LimitSeconds);
	State.Seconds = Own.Seconds;
	State.Longest = Own.Longest;
	State.LastHits = Own.LastHits;
	State.bTold = Own.bTold;
	State.Runs = Own.Runs;
	return bTold;
}

TArray<FString> UHOLMovementRulesLibrary::FindColourClashes(const TArray<int32>& Neutrals, const TArray<int32>& PlayerSlots, const TArray<uint8>& SlotColorIndices)
{
	return HOLSanity::FindColourClashes(Neutrals, PlayerSlots, SlotColorIndices);
}
