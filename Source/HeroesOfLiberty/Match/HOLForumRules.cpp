// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLForumRules.h"

#include "Algo/StableSort.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Match/HOLMatchConfig.h"
#include "Simulation/HOLSimulationSettings.h"

FHOLForumTuning FHOLForumTuning::FromSettings()
{
	const UHOLBuildingSettings* Settings = GetDefault<UHOLBuildingSettings>();
	const float StepSeconds = FMath::Max(GetDefault<UHOLSimulationSettings>()->GetStepSeconds(), UE_KINDA_SMALL_NUMBER);
	FHOLForumTuning Tuning;
	Tuning.Tier = FMath::Max(Settings->ForumLivesTier, 0);
	Tuning.RuinSteps = FMath::Max(FMath::RoundToInt(Settings->ForumRuinTime / StepSeconds), 0);
	Tuning.RebuildSteps = FMath::Max(FMath::RoundToInt(Settings->ForumRebuildTime / StepSeconds), 1);
	Tuning.BuilderExponent = Settings->BuilderExponent;
	Tuning.StepSeconds = StepSeconds;
	return Tuning;
}

int32 HOLForumRules::GetLives(const int32 Tier, const int32 Falls)
{
	return FMath::Max(BaseLives + FMath::Max(Tier, 0) - FMath::Max(Falls, 0), 0);
}

int32 HOLForumRules::GetLives(const FHOLForumState& State, const FHOLForumTuning& Tuning)
{
	return GetLives(Tuning.Tier, State.Falls);
}

int64 HOLForumRules::GetRebuildTotalWork(const FHOLForumTuning& Tuning)
{
	return HOLConstruction::GetTotalWork(FMath::Max(Tuning.RebuildSteps, 1));
}

int32 HOLForumRules::GetRebuildWorkPerStep(const int32 Builders, const float Exponent)
{
	// The rebuilding alone works as one builder; the Peasants add theirs by D312 (one Peasant: twice as fast).
	return HOLConstruction::WorkPerBuilder + HOLConstruction::GetWorkPerStep(FMath::Max(Builders, 0), Exponent);
}

int32 HOLForumRules::GetStepsToRebuild(const FHOLForumTuning& Tuning, const int32 Builders)
{
	const int64 PerStep = GetRebuildWorkPerStep(Builders, Tuning.BuilderExponent);
	return static_cast<int32>((GetRebuildTotalWork(Tuning) + PerStep - 1) / PerStep);
}

float HOLForumRules::GetRebuildProgress(const FHOLForumState& State, const FHOLForumTuning& Tuning)
{
	switch (State.Phase)
	{
	case EHOLForumPhase::Ruin: return 0.f;
	case EHOLForumPhase::Rebuilding:
		return FMath::Clamp(static_cast<float>(static_cast<double>(State.RebuildWork) / static_cast<double>(GetRebuildTotalWork(Tuning))), 0.f, 1.f);
	default: return 1.f;
	}
}

FHOLForumLookChoice HOLForumRules::GetLook(const FHOLForumState& State, const FHOLForumTuning& Tuning)
{
	FHOLForumLookChoice Choice;
	Choice.Look = State.Phase == EHOLForumPhase::Ruin ? EHOLForumLook::Rubble
		: State.Phase == EHOLForumPhase::Rebuilding ? EHOLForumLook::Site : EHOLForumLook::Standing;
	Choice.Progress = GetRebuildProgress(State, Tuning);
	return Choice;
}

bool HOLForumRules::BuildersWait(const FHOLForumState& State)
{
	return State.Phase == EHOLForumPhase::Ruin;
}

int32 HOLForumRules::Fall(FHOLForumState& State, const FHOLForumTuning& Tuning)
{
	if (State.IsDown())
	{
		return INDEX_NONE;
	}
	State.Falls = static_cast<uint8>(FMath::Min(static_cast<int32>(State.Falls) + 1, static_cast<int32>(MAX_uint8)));
	State.Phase = EHOLForumPhase::Ruin;
	State.RuinStepsLeft = FMath::Max(Tuning.RuinSteps, 0);
	State.RebuildWork = 0;
	return GetLives(State, Tuning);
}

bool HOLForumRules::Advance(FHOLForumState& State, const FHOLForumTuning& Tuning, const int32 Builders)
{
	switch (State.Phase)
	{
	case EHOLForumPhase::Ruin:
		// D352: nothing builds in the ruin, the Peasants wait.
		State.RuinStepsLeft = FMath::Max(State.RuinStepsLeft - 1, 0);
		if (State.RuinStepsLeft > 0)
		{
			return false;
		}
		State.Phase = EHOLForumPhase::Rebuilding;
		State.RebuildWork = 0;
		return true;
	case EHOLForumPhase::Rebuilding:
		State.RebuildWork += GetRebuildWorkPerStep(Builders, Tuning.BuilderExponent);
		if (State.RebuildWork < GetRebuildTotalWork(Tuning))
		{
			return false;
		}
		State.Phase = EHOLForumPhase::Standing;
		State.RebuildWork = 0;
		return true;
	default:
		return false;
	}
}

bool HOLForumRules::Skip(FHOLForumState& State)
{
	switch (State.Phase)
	{
	case EHOLForumPhase::Ruin:
		State.Phase = EHOLForumPhase::Rebuilding;
		State.RuinStepsLeft = 0;
		State.RebuildWork = 0;
		return true;
	case EHOLForumPhase::Rebuilding:
		State.Phase = EHOLForumPhase::Standing;
		State.RebuildWork = 0;
		return true;
	default:
		return false;
	}
}

const TCHAR* HOLForumRules::GetPhaseName(const EHOLForumPhase Phase)
{
	switch (Phase)
	{
	case EHOLForumPhase::Ruin: return TEXT("ruin");
	case EHOLForumPhase::Rebuilding: return TEXT("rebuilding");
	default: return TEXT("standing");
	}
}

int32 HOLForumRules::GetRuinSecondsLeft(const FHOLForumState& State, const FHOLForumTuning& Tuning)
{
	// Rounded up: the last tenth of a second still shows 1 s (the small margin keeps 250 steps of 0.1 s at 25 s).
	return State.Phase != EHOLForumPhase::Ruin ? 0
		: FMath::Max(FMath::CeilToInt(static_cast<float>(FMath::Max(State.RuinStepsLeft, 0)) * Tuning.StepSeconds - UE_KINDA_SMALL_NUMBER), 0);
}

int32 HOLForumRules::GetRebuildPercent(const FHOLForumState& State, const FHOLForumTuning& Tuning)
{
	switch (State.Phase)
	{
	case EHOLForumPhase::Ruin: return 0;
	case EHOLForumPhase::Rebuilding:
		// In whole work, not a float: 40 % of the work shows 40, never 39.
		return static_cast<int32>(FMath::Clamp<int64>(FMath::Max<int64>(State.RebuildWork, 0) * 100 / FMath::Max<int64>(GetRebuildTotalWork(Tuning), 1), 0, 100));
	default: return 100;
	}
}

// The selection block's keys stay with the HUD's own (UHOLGameScreen), where they are shown.
#define LOCTEXT_NAMESPACE "HOLGameScreen"

FHOLForumBlock HOLForumRules::GetSelectionBlock(const FHOLForumState* State, const FHOLForumTuning& Tuning, const bool bOwn, const int32 Builders)
{
	FHOLForumBlock Block;
	if (!State)
	{
		return Block;
	}
	Block.bShown = true;
	Block.bHideHealth = State->IsDown();
	const int32 Lives = GetLives(*State, Tuning);
	TArray<FText> Lines;
	if (State->Phase == EHOLForumPhase::Ruin)
	{
		const int32 Seconds = GetRuinSecondsLeft(*State, Tuning);
		Block.Shown += FString::Printf(TEXT(", in ruin %d s"), Seconds);
		Lines.Add(FText::Format(LOCTEXT("ForumRuin", "En ruine — {0} s"), FText::AsNumber(Seconds)));
	}
	else if (State->Phase == EHOLForumPhase::Rebuilding)
	{
		const int32 Percent = GetRebuildPercent(*State, Tuning);
		Block.Shown += FString::Printf(TEXT(", rebuilding %d%%"), Percent);
		Lines.Add(FText::Format(LOCTEXT("ForumRebuilding", "Reconstruction — {0} %"), FText::AsNumber(Percent)));
		if (bOwn)
		{
			// Its builders at work (D313), which only its owner receives.
			const int32 AtWork = FMath::Max(Builders, 0);
			Block.Shown += FString::Printf(TEXT(", builders %d"), AtWork);
			Lines.Add(FText::Format(LOCTEXT("Builders", "Bâtisseurs : {0}"), AtWork));
		}
	}
	// Its lives on a row of their own, as the health's (V2.3d): the HUD gives them their label.
	Block.Shown += FString::Printf(TEXT(", lives %d"), Lives);
	Block.Lives = Lives;
	Block.Text = FText::Join(FText::AsCultureInvariant(TEXT("\n")), Lines);
	return Block;
}

TArray<FHOLPlayersTableRow> HOLForumRules::GetPlayersTable(const FHOLMatchConfig& Config, const TConstArrayView<FHOLForumState> Forums,
	const uint8 DefeatedMask, const int32 LocalSlot, const FHOLForumTuning& Tuning)
{
	// The occupied slots, in their order; grouped by team when there are teams (no team last).
	TArray<int32> Slots;
	bool bTeams = false;
	for (int32 Slot = 0; Slot < Config.Slots.Num() && Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		if (Config.Slots[Slot].State != EHOLSlotState::Closed)
		{
			Slots.Add(Slot);
			bTeams |= Config.Slots[Slot].Team > 0;
		}
	}
	if (bTeams)
	{
		const auto GroupOf = [&Config](const int32 Slot) { return Config.Slots[Slot].Team > 0 ? Config.Slots[Slot].Team : FHOLMatchConfig::MaxTeams + 1; };
		Algo::StableSortBy(Slots, GroupOf);
	}

	TArray<FHOLPlayersTableRow> Rows;
	Rows.Reserve(Slots.Num());
	for (const int32 Slot : Slots)
	{
		const FHOLMatchSlot& MatchSlot = Config.Slots[Slot];
		const FHOLForumState State = Forums.IsValidIndex(Slot) ? Forums[Slot] : FHOLForumState();
		const int32 Lives = GetLives(State, Tuning);
		const bool bLocal = Slot == LocalSlot;
		const bool bDefeated = (DefeatedMask & (1 << Slot)) != 0;

		FHOLPlayersTableRow& Row = Rows.AddDefaulted_GetRef();
		Row.Slot = Slot;
		Row.ColorIndex = MatchSlot.ColorIndex;
		Row.Team = MatchSlot.Team;
		Row.bGreyed = bDefeated;
		Row.bNewGroup = bTeams && Rows.Num() > 1 && Rows[Rows.Num() - 2].Team != Row.Team;
		Row.Name = bLocal ? LOCTEXT("You", "Vous") : FText::Format(LOCTEXT("PlayerN", "Joueur {0}"), Slot + 1);
		// The singular for exactly 1, the plural for 0 (V2.3d); a defeated slot's lives not shown, « Vaincu » alone.
		Row.Lives = bDefeated ? FText::GetEmpty()
			: FText::Format(Lives == 1 ? LOCTEXT("TableLife", "{0} vie") : LOCTEXT("TableLives", "{0} vies"), Lives);
		FString Phase;
		if (bDefeated)
		{
			Phase = TEXT("defeated greyed");
			Row.State = LOCTEXT("TableDefeated", "Vaincu");
		}
		else if (State.Phase == EHOLForumPhase::Ruin)
		{
			Phase = TEXT("ruin");
			Row.State = LOCTEXT("TableRuin", "Ruine");
		}
		else if (State.Phase == EHOLForumPhase::Rebuilding)
		{
			const int32 Percent = GetRebuildPercent(State, Tuning);
			Phase = FString::Printf(TEXT("rebuilding %d%%"), Percent);
			Row.State = FText::Format(LOCTEXT("TableRebuilding", "Reconstruction {0} %"), FText::AsNumber(Percent));
		}
		else
		{
			Phase = TEXT("standing");
		}
		Row.Shown = FString::Printf(TEXT("slot %d%s%s colour %d lives %d %s"), Slot, bLocal ? TEXT(" you") : TEXT(""),
			Row.Team > 0 ? *FString::Printf(TEXT(" team %d"), Row.Team) : TEXT(""), Row.ColorIndex, Lives, *Phase);
	}
	return Rows;
}

#undef LOCTEXT_NAMESPACE
