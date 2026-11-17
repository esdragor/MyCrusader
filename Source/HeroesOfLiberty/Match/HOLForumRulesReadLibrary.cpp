// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLForumRulesReadLibrary.h"

#include "Match/HOLForumRules.h"

namespace
{
	FHOLForumState ForumRead_ToRule(const FHOLForumStateRead& Read)
	{
		FHOLForumState State;
		State.Falls = static_cast<uint8>(Read.Falls);
		State.Phase = static_cast<EHOLForumPhase>(Read.Phase);
		State.RuinStepsLeft = Read.RuinStepsLeft;
		State.RebuildWork = Read.RebuildWork;
		State.HoldSteps = Read.HoldSteps;
		State.Hold = static_cast<EHOLForumHold>(Read.Hold);
		State.HolderSlot = static_cast<int8>(Read.HolderSlot);
		return State;
	}

	void ForumRead_FromRule(const FHOLForumState& State, FHOLForumStateRead& Read)
	{
		Read.Falls = State.Falls;
		Read.Phase = static_cast<int32>(State.Phase);
		Read.RuinStepsLeft = State.RuinStepsLeft;
		Read.RebuildWork = State.RebuildWork;
		Read.HoldSteps = State.HoldSteps;
		Read.Hold = static_cast<int32>(State.Hold);
		Read.HolderSlot = State.HolderSlot;
	}

	FHOLForumTuning ForumRead_ToRule(const FHOLForumTuningRead& Read)
	{
		FHOLForumTuning Tuning;
		Tuning.Tier = Read.Tier;
		Tuning.RuinSteps = Read.RuinSteps;
		Tuning.RebuildSteps = Read.RebuildSteps;
		Tuning.BuilderExponent = Read.BuilderExponent;
		Tuning.StepSeconds = Read.StepSeconds;
		Tuning.HoldSteps = Read.HoldSteps;
		Tuning.HoldDecaySteps = Read.HoldDecaySteps;
		Tuning.HoldRadius = Read.HoldRadius;
		return Tuning;
	}
}

int32 UHOLForumRulesReadLibrary::GetBaseLives()
{
	return HOLForumRules::BaseLives;
}

FHOLForumTuningRead UHOLForumRulesReadLibrary::GetGameTuning()
{
	const FHOLForumTuning Game = FHOLForumTuning::FromSettings();
	FHOLForumTuningRead Read;
	Read.Tier = Game.Tier;
	Read.RuinSteps = Game.RuinSteps;
	Read.RebuildSteps = Game.RebuildSteps;
	Read.BuilderExponent = Game.BuilderExponent;
	Read.StepSeconds = Game.StepSeconds;
	Read.HoldSteps = Game.HoldSteps;
	Read.HoldDecaySteps = Game.HoldDecaySteps;
	Read.HoldRadius = Game.HoldRadius;
	return Read;
}

int32 UHOLForumRulesReadLibrary::GetLivesByTier(const int32 Tier, const int32 Falls)
{
	return HOLForumRules::GetLives(Tier, Falls);
}

int32 UHOLForumRulesReadLibrary::GetLives(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning)
{
	return HOLForumRules::GetLives(ForumRead_ToRule(State), ForumRead_ToRule(Tuning));
}

int64 UHOLForumRulesReadLibrary::GetRebuildTotalWork(const FHOLForumTuningRead& Tuning)
{
	return HOLForumRules::GetRebuildTotalWork(ForumRead_ToRule(Tuning));
}

int32 UHOLForumRulesReadLibrary::GetRebuildWorkPerStep(const int32 Builders, const float Exponent)
{
	return HOLForumRules::GetRebuildWorkPerStep(Builders, Exponent);
}

int32 UHOLForumRulesReadLibrary::GetStepsToRebuild(const FHOLForumTuningRead& Tuning, const int32 Builders)
{
	return HOLForumRules::GetStepsToRebuild(ForumRead_ToRule(Tuning), Builders);
}

float UHOLForumRulesReadLibrary::GetRebuildProgress(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning)
{
	return HOLForumRules::GetRebuildProgress(ForumRead_ToRule(State), ForumRead_ToRule(Tuning));
}

int32 UHOLForumRulesReadLibrary::GetRebuildPercent(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning)
{
	return HOLForumRules::GetRebuildPercent(ForumRead_ToRule(State), ForumRead_ToRule(Tuning));
}

int32 UHOLForumRulesReadLibrary::GetRuinSecondsLeft(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning)
{
	return HOLForumRules::GetRuinSecondsLeft(ForumRead_ToRule(State), ForumRead_ToRule(Tuning));
}

int32 UHOLForumRulesReadLibrary::Fall(FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning)
{
	FHOLForumState Rule = ForumRead_ToRule(State);
	const int32 Lives = HOLForumRules::Fall(Rule, ForumRead_ToRule(Tuning));
	ForumRead_FromRule(Rule, State);
	return Lives;
}

bool UHOLForumRulesReadLibrary::Advance(FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning, const int32 Builders)
{
	FHOLForumState Rule = ForumRead_ToRule(State);
	const bool bChanged = HOLForumRules::Advance(Rule, ForumRead_ToRule(Tuning), Builders);
	ForumRead_FromRule(Rule, State);
	return bChanged;
}

FHOLForumStepResultRead UHOLForumRulesReadLibrary::Step(FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning, const int32 Builders,
	const bool bAttackers, const bool bDefenders, const int32 AttackerSlot)
{
	FHOLForumPresence Presence;
	Presence.bAttackers = bAttackers;
	Presence.bDefenders = bDefenders;
	Presence.AttackerSlot = AttackerSlot;
	FHOLForumState Rule = ForumRead_ToRule(State);
	const HOLForumRules::FStepResult Result = HOLForumRules::Step(Rule, ForumRead_ToRule(Tuning), Builders, Presence);
	ForumRead_FromRule(Rule, State);
	FHOLForumStepResultRead Read;
	Read.bPhaseChanged = Result.bPhaseChanged;
	Read.bRazed = Result.bRazed;
	Read.Lives = Result.Lives;
	return Read;
}

int32 UHOLForumRulesReadLibrary::Raze(FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning)
{
	FHOLForumState Rule = ForumRead_ToRule(State);
	const int32 Lives = HOLForumRules::Raze(Rule, ForumRead_ToRule(Tuning));
	ForumRead_FromRule(Rule, State);
	return Lives;
}

bool UHOLForumRulesReadLibrary::Skip(FHOLForumStateRead& State)
{
	FHOLForumState Rule = ForumRead_ToRule(State);
	const bool bSkipped = HOLForumRules::Skip(Rule);
	ForumRead_FromRule(Rule, State);
	return bSkipped;
}

int32 UHOLForumRulesReadLibrary::GetHoldSeconds(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning)
{
	return HOLForumRules::GetHoldSeconds(ForumRead_ToRule(State), ForumRead_ToRule(Tuning));
}

float UHOLForumRulesReadLibrary::GetHoldProgress(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning)
{
	return HOLForumRules::GetHoldProgress(ForumRead_ToRule(State), ForumRead_ToRule(Tuning));
}

bool UHOLForumRulesReadLibrary::ShowsHold(const FHOLForumStateRead& State)
{
	return HOLForumRules::ShowsHold(ForumRead_ToRule(State));
}

FString UHOLForumRulesReadLibrary::DescribeHold(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning)
{
	return HOLForumRules::DescribeHold(ForumRead_ToRule(State), ForumRead_ToRule(Tuning));
}

bool UHOLForumRulesReadLibrary::IsRingShown(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning)
{
	return HOLForumRules::GetRing(ForumRead_ToRule(State), ForumRead_ToRule(Tuning)).bShown;
}

float UHOLForumRulesReadLibrary::GetRingFill(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning)
{
	return HOLForumRules::GetRing(ForumRead_ToRule(State), ForumRead_ToRule(Tuning)).Fill;
}

int32 UHOLForumRulesReadLibrary::GetRingHolderSlot(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning)
{
	return HOLForumRules::GetRing(ForumRead_ToRule(State), ForumRead_ToRule(Tuning)).HolderSlot;
}

FString UHOLForumRulesReadLibrary::GetRingShown(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning)
{
	return HOLForumRules::GetRing(ForumRead_ToRule(State), ForumRead_ToRule(Tuning)).Shown;
}

FHOLForumBlockRead UHOLForumRulesReadLibrary::GetSelectionBlock(const bool bIsMainForum, const FHOLForumStateRead& State,
	const FHOLForumTuningRead& Tuning, const bool bOwn, const int32 Builders)
{
	const FHOLForumState Rule = ForumRead_ToRule(State);
	const FHOLForumBlock Block = HOLForumRules::GetSelectionBlock(bIsMainForum ? &Rule : nullptr, ForumRead_ToRule(Tuning), bOwn, Builders);
	FHOLForumBlockRead Read;
	Read.bShown = Block.bShown;
	Read.bHideHealth = Block.bHideHealth;
	Read.Shown = Block.Shown;
	Read.Text = Block.Text.BuildSourceString();
	Read.Lives = Block.Lives;
	return Read;
}

int32 UHOLForumRulesReadLibrary::GetLook(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning)
{
	return static_cast<int32>(HOLForumRules::GetLook(ForumRead_ToRule(State), ForumRead_ToRule(Tuning)).Look);
}

float UHOLForumRulesReadLibrary::GetLookProgress(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning)
{
	return HOLForumRules::GetLook(ForumRead_ToRule(State), ForumRead_ToRule(Tuning)).Progress;
}

bool UHOLForumRulesReadLibrary::BuildersWait(const FHOLForumStateRead& State)
{
	return HOLForumRules::BuildersWait(ForumRead_ToRule(State));
}
