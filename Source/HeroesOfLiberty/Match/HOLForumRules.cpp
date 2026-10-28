// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLForumRules.h"

#include "Algo/StableSort.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLVictoryRules.h"
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
	Tuning.HoldSteps = FMath::Max(FMath::RoundToInt(Settings->ForumHoldTime / StepSeconds), 1);
	Tuning.HoldDecaySteps = FMath::Max(FMath::RoundToInt(Settings->ForumHoldDecay), 0);
	Tuning.HoldRadius = FMath::Max(Settings->ForumHoldRadius, 0.f);
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
	// V3 (D352): the hold counts from the fall on.
	State.HoldSteps = 0;
	State.Hold = EHOLForumHold::None;
	State.HolderSlot = INDEX_NONE;
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

HOLForumRules::FStepResult HOLForumRules::Step(FHOLForumState& State, const FHOLForumTuning& Tuning, const int32 Builders, const FHOLForumPresence& Presence)
{
	FStepResult Result;
	if (!State.IsDown())
	{
		State.HoldSteps = 0;
		State.Hold = EHOLForumHold::None;
		State.HolderSlot = INDEX_NONE;
		return Result;
	}

	// D348: attackers without defenders hold, with defenders the count freezes (contested); nobody, it recedes (V3-Q3).
	if (Presence.bAttackers)
	{
		State.Hold = Presence.bDefenders ? EHOLForumHold::Contested : EHOLForumHold::Holding;
		if (Presence.AttackerSlot != INDEX_NONE)
		{
			State.HolderSlot = static_cast<int8>(Presence.AttackerSlot);
		}
		if (!Presence.bDefenders)
		{
			++State.HoldSteps;
		}
	}
	else
	{
		State.HoldSteps = FMath::Max(State.HoldSteps - FMath::Max(Tuning.HoldDecaySteps, 0), 0);
		State.Hold = State.HoldSteps > 0 ? EHOLForumHold::Receding : EHOLForumHold::None;
		if (State.HoldSteps == 0)
		{
			State.HolderSlot = INDEX_NONE;
		}
	}

	if (State.HoldSteps >= FMath::Max(Tuning.HoldSteps, 1))
	{
		// V3-Q4: the foundations razed, a new complete fall; nothing else this step.
		Result.Lives = Raze(State, Tuning);
		Result.bRazed = true;
		Result.bPhaseChanged = true;
		return Result;
	}

	// D348: the rebuilding frozen while attackers stand there, contested or not; the ruin counts down as ever.
	if (State.Phase == EHOLForumPhase::Rebuilding && Presence.bAttackers)
	{
		return Result;
	}
	Result.bPhaseChanged = Advance(State, Tuning, Builders);
	if (!State.IsDown())
	{
		State.HoldSteps = 0;
		State.Hold = EHOLForumHold::None;
		State.HolderSlot = INDEX_NONE;
	}
	return Result;
}

int32 HOLForumRules::Raze(FHOLForumState& State, const FHOLForumTuning& Tuning)
{
	if (!State.IsDown())
	{
		return INDEX_NONE;
	}
	State.Falls = static_cast<uint8>(FMath::Min(static_cast<int32>(State.Falls) + 1, static_cast<int32>(MAX_uint8)));
	State.Phase = EHOLForumPhase::Ruin;
	State.RuinStepsLeft = FMath::Max(Tuning.RuinSteps, 0);
	State.RebuildWork = 0;
	State.HoldSteps = 0;
	State.Hold = EHOLForumHold::None;
	State.HolderSlot = INDEX_NONE;
	return GetLives(State, Tuning);
}

int32 HOLForumRules::GetHoldSeconds(const FHOLForumState& State, const FHOLForumTuning& Tuning)
{
	// Rounded down, with a small margin: 25 steps of 0.1 s show 2 s, 600 show 60.
	return FMath::Max(FMath::FloorToInt(static_cast<float>(FMath::Max(State.HoldSteps, 0)) * Tuning.StepSeconds + UE_KINDA_SMALL_NUMBER), 0);
}

float HOLForumRules::GetHoldProgress(const FHOLForumState& State, const FHOLForumTuning& Tuning)
{
	return FMath::Clamp(static_cast<float>(FMath::Max(State.HoldSteps, 0)) / static_cast<float>(FMath::Max(Tuning.HoldSteps, 1)), 0.f, 1.f);
}

const TCHAR* HOLForumRules::GetHoldName(const EHOLForumHold Hold)
{
	switch (Hold)
	{
	case EHOLForumHold::Holding: return TEXT("holding");
	case EHOLForumHold::Contested: return TEXT("contested");
	case EHOLForumHold::Receding: return TEXT("receding");
	default: return TEXT("none");
	}
}

bool HOLForumRules::ShowsHold(const FHOLForumState& State)
{
	return State.IsDown() && (State.HoldSteps > 0 || State.Hold != EHOLForumHold::None);
}

FString HOLForumRules::DescribeHold(const FHOLForumState& State, const FHOLForumTuning& Tuning)
{
	if (!ShowsHold(State))
	{
		return FString();
	}
	FString Shown = FString::Printf(TEXT("hold %d s %s"), GetHoldSeconds(State, Tuning), GetHoldName(State.Hold));
	if (State.HolderSlot != INDEX_NONE)
	{
		Shown += FString::Printf(TEXT(" by slot %d"), static_cast<int32>(State.HolderSlot));
	}
	return Shown;
}

FHOLForumRing HOLForumRules::GetRing(const FHOLForumState& State, const FHOLForumTuning& Tuning)
{
	FHOLForumRing Ring;
	Ring.bShown = ShowsHold(State);
	if (!Ring.bShown)
	{
		Ring.Shown = TEXT("none");
		return Ring;
	}
	Ring.Fill = GetHoldProgress(State, Tuning);
	Ring.HolderSlot = State.HolderSlot;
	// Contested, or no holder known (a cheat without an attacker): grey; receding: the holder's colour faded.
	Ring.Tint = State.Hold == EHOLForumHold::Contested || Ring.HolderSlot == INDEX_NONE ? EHOLForumRingTint::Grey
		: State.Hold == EHOLForumHold::Holding ? EHOLForumRingTint::Holder : EHOLForumRingTint::Faded;
	Ring.Shown = FString::Printf(TEXT("fill %.2f %s"), Ring.Fill, Ring.Tint == EHOLForumRingTint::Grey ? TEXT("grey")
		: *FString::Printf(TEXT("%s slot %d"), Ring.Tint == EHOLForumRingTint::Holder ? TEXT("holder") : TEXT("faded"), Ring.HolderSlot));
	return Ring;
}

FLinearColor HOLForumRules::GetRingColor(const EHOLForumRingTint Tint, const FLinearColor& HolderColor)
{
	// V3.4b: a dark grey for a frozen count, which stands out on the light ground (a mid grey melted into it); a darker
	// grey the receding colour is darkened halfway to (a light one paled it), still apart from the contested grey.
	static const FLinearColor ContestedGrey(0.18f, 0.18f, 0.18f);
	static const FLinearColor FadeGrey(0.1f, 0.1f, 0.1f);
	switch (Tint)
	{
	case EHOLForumRingTint::Holder: return HolderColor;
	case EHOLForumRingTint::Faded: return HolderColor * 0.5f + FadeGrey * 0.5f;
	default: return ContestedGrey;
	}
}

FVector2D HOLForumRules::GetZoneOutlinePoint(const FBox2D& Body, const double Radius, const double Share)
{
	// The outline's pieces in order from the +X side's middle, clockwise seen from above (+X up, +Y right): a straight
	// piece runs along a side, at Radius out; an arc turns around a corner of the footprint.
	const double R = FMath::Max(Radius, 0.0);
	const FVector2D Min = Body.Min;
	const FVector2D Max = Body.Max;
	const double CentreY = (Min.Y + Max.Y) * 0.5;
	const double SizeX = FMath::Max(Max.X - Min.X, 0.0);
	const double SizeY = FMath::Max(Max.Y - Min.Y, 0.0);
	const double Quarter = UE_HALF_PI * R;
	const double Length = 2.0 * (SizeX + SizeY) + 4.0 * Quarter;
	if (Length <= UE_KINDA_SMALL_NUMBER)
	{
		return Body.GetCenter();
	}
	double Left = FMath::Clamp(Share - FMath::FloorToDouble(Share), 0.0, 1.0) * Length;
	if (Share >= 1.0)
	{
		Left = Length;
	}
	struct FPiece
	{
		FVector2D Start;
		FVector2D End;
		/** An arc: its corner and its first angle (radians); a straight piece otherwise. */
		bool bArc;
		FVector2D Corner;
		double Angle;
	};
	const FPiece Pieces[] = {
		{ FVector2D(Max.X + R, CentreY), FVector2D(Max.X + R, Max.Y), false, FVector2D::ZeroVector, 0.0 },
		{ FVector2D::ZeroVector, FVector2D::ZeroVector, true, FVector2D(Max.X, Max.Y), 0.0 },
		{ FVector2D(Max.X, Max.Y + R), FVector2D(Min.X, Max.Y + R), false, FVector2D::ZeroVector, 0.0 },
		{ FVector2D::ZeroVector, FVector2D::ZeroVector, true, FVector2D(Min.X, Max.Y), UE_HALF_PI },
		{ FVector2D(Min.X - R, Max.Y), FVector2D(Min.X - R, Min.Y), false, FVector2D::ZeroVector, 0.0 },
		{ FVector2D::ZeroVector, FVector2D::ZeroVector, true, FVector2D(Min.X, Min.Y), UE_PI },
		{ FVector2D(Min.X, Min.Y - R), FVector2D(Max.X, Min.Y - R), false, FVector2D::ZeroVector, 0.0 },
		{ FVector2D::ZeroVector, FVector2D::ZeroVector, true, FVector2D(Max.X, Min.Y), 1.5 * UE_PI },
		{ FVector2D(Max.X + R, Min.Y), FVector2D(Max.X + R, CentreY), false, FVector2D::ZeroVector, 0.0 },
	};
	FVector2D Point = Pieces[0].Start;
	for (const FPiece& Piece : Pieces)
	{
		const double PieceLength = Piece.bArc ? Quarter : FVector2D::Distance(Piece.Start, Piece.End);
		const double Along = FMath::Min(Left, PieceLength);
		const double Alpha = PieceLength > UE_KINDA_SMALL_NUMBER ? Along / PieceLength : 1.0;
		if (Piece.bArc)
		{
			const double Angle = Piece.Angle + Alpha * UE_HALF_PI;
			Point = Piece.Corner + R * FVector2D(FMath::Cos(Angle), FMath::Sin(Angle));
		}
		else
		{
			Point = FMath::Lerp(Piece.Start, Piece.End, Alpha);
		}
		Left -= Along;
		if (Left <= 0.0)
		{
			break;
		}
	}
	return Point;
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
		// Standing: no hold (V3).
		State.HoldSteps = 0;
		State.Hold = EHOLForumHold::None;
		State.HolderSlot = INDEX_NONE;
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
	// V3.3 (V3-Q5): the hold, to anyone: « Tenue » while held or receding (its seconds going down), « Contestée » frozen.
	if (ShowsHold(*State))
	{
		const int32 HoldSeconds = GetHoldSeconds(*State, Tuning);
		Block.Shown += TEXT(", ") + DescribeHold(*State, Tuning);
		Lines.Add(State->IsContested() ? FText::Format(LOCTEXT("ForumHoldContested", "Contestée — {0} s"), FText::AsNumber(HoldSeconds))
			: FText::Format(LOCTEXT("ForumHold", "Tenue — {0} s"), FText::AsNumber(HoldSeconds)));
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
		// E4-2: a defeated slot's name struck through, as its row greyed.
		Row.bStruck = bDefeated;
		// E4-1: same camp as the viewer an ally, another an enemy; a spectator (no slot) sees no relation.
		const int32 ViewerCamp = LocalSlot != INDEX_NONE ? HOLVictoryRules::GetCamp(Config, LocalSlot) : INDEX_NONE;
		Row.Relation = bLocal ? EHOLPlayerRelation::Self
			: ViewerCamp == INDEX_NONE ? EHOLPlayerRelation::None
			: HOLVictoryRules::GetCamp(Config, Slot) == ViewerCamp ? EHOLPlayerRelation::Ally
			: EHOLPlayerRelation::Enemy;
		Row.bNewGroup = bTeams && Rows.Num() > 1 && Rows[Rows.Num() - 2].Team != Row.Team;
		// M6-Q12: the name the lobby gave the slot when it has one (a match from a lobby), else Joueur N; an AI's
		// « IA Difficile 3 » (IA0.1-Q1).
		const FString PlayerName = MatchSlot.PlayerName.TrimStartAndEnd();
		Row.Name = HOLForumMessages::GetPlayerName(Config, Slot, bLocal ? Slot : INDEX_NONE);
		// The singular for exactly 1, the plural for 0 (V2.3d); a defeated slot's lives not shown, « Vaincu » alone.
		Row.Lives = bDefeated ? FText::GetEmpty()
			: FText::Format(Lives == 1 ? LOCTEXT("TableLife", "{0} vie") : LOCTEXT("TableLives", "{0} vies"), Lives);
		FString Phase;
		if (bDefeated)
		{
			Phase = TEXT("defeated greyed struck");
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
		// V3.3 (V3-Q5): the hold after the phase, short: « Ruine · tenue 35 s », « Reconstruction 40 % · contestée 35 s ».
		if (!bDefeated && ShowsHold(State))
		{
			const int32 HoldSeconds = GetHoldSeconds(State, Tuning);
			Phase += TEXT(" ") + DescribeHold(State, Tuning);
			Row.State = FText::Format(State.IsContested() ? LOCTEXT("TableHoldContested", "{0} · contestée {1} s") : LOCTEXT("TableHold", "{0} · tenue {1} s"),
				Row.State, FText::AsNumber(HoldSeconds));
		}
		const TCHAR* const RelationShown = Row.Relation == EHOLPlayerRelation::Self ? TEXT(" you")
			: Row.Relation == EHOLPlayerRelation::Ally ? TEXT(" ally")
			: Row.Relation == EHOLPlayerRelation::Enemy ? TEXT(" enemy") : TEXT("");
		Row.Shown = FString::Printf(TEXT("slot %d%s%s%s colour %d lives %d %s"), Slot, RelationShown,
			PlayerName.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" '%s'"), *PlayerName),
			Row.Team > 0 ? *FString::Printf(TEXT(" team %d"), Row.Team) : TEXT(""), Row.ColorIndex, Lives, *Phase);
	}
	return Rows;
}

#undef LOCTEXT_NAMESPACE
