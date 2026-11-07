// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLGameState.h"

#include "AI/HOLAIBrain.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "HeroesOfLibertyLog.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Map/HOLStartSlot.h"
#include "Map/HOLVisionSubsystem.h"
#include "Match/HOLForumPresence.h"
#include "Match/HOLMatchStatsSubsystem.h"
#include "Match/HOLPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Simulation/HOLHeroLibrary.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLHeroXPRules.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

namespace
{
	/** P5 (D513): the pauses of a match, shared by all its players (« réglable »). */
	int32 PausesPerMatch = FHOLPauseRules::DefaultPausesPerMatch;
	FAutoConsoleVariableRef CVarPausesPerMatch(TEXT("hol.Match.PausesPerMatch"), PausesPerMatch,
		TEXT("hol.Match.PausesPerMatch N: the pauses of a networked match, shared by all its players (D513, default 3); read at its start"));
}

AHOLGameState::AHOLGameState()
{
	SlotDefeats.Init(EHOLDefeatReason::None, FHOLMatchConfig::MaxSlots);
	ForumStates.Init(FHOLForumState(), FHOLMatchConfig::MaxSlots);
	TierOverrides.Init(-1, FHOLMatchConfig::MaxSlots);
	ForumBuilders.Init(0, FHOLMatchConfig::MaxSlots);
	LaunchConfig = FHOLMatchConfig::MakeDefault();
	HeroSheets.Init(FHOLHeroSheet(), FHOLMatchConfig::MaxSlots);
}

void AHOLGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// J8a.3: the heroes' public sheets (kept apart from the list below, which other work items extend).
	DOREPLIFETIME(AHOLGameState, HeroSheets);
	// M4c: the Market's common prices.
	DOREPLIFETIME(AHOLGameState, MarketPrices);
	DOREPLIFETIME(AHOLGameState, DiplomacyStances);
	DOREPLIFETIME(AHOLGameState, DiplomacyVision);
	DOREPLIFETIME(AHOLGameState, SlotDefeats);
	DOREPLIFETIME(AHOLGameState, bMatchOver);
	DOREPLIFETIME(AHOLGameState, WinnerMask);
	DOREPLIFETIME(AHOLGameState, LaunchConfig);
	DOREPLIFETIME(AHOLGameState, FogMode);
	DOREPLIFETIME(AHOLGameState, ForumStates);
	DOREPLIFETIME(AHOLGameState, TierOverrides);
	DOREPLIFETIME(AHOLGameState, bWaitingForPlayers);
	DOREPLIFETIME(AHOLGameState, MissingPlayers);
	DOREPLIFETIME(AHOLGameState, WaitEndSeconds);
	DOREPLIFETIME(AHOLGameState, AbsentNotices);
	DOREPLIFETIME(AHOLGameState, AIHeldMask);
	DOREPLIFETIME(AHOLGameState, RelayViews);
	DOREPLIFETIME(AHOLGameState, VolcanoViews);
	DOREPLIFETIME(AHOLGameState, PausesLeft);
	DOREPLIFETIME(AHOLGameState, PausedBySlot);
	DOREPLIFETIME(AHOLGameState, PausedByName);
	DOREPLIFETIME(AHOLGameState, PausedAtSeconds);
}

void AHOLGameState::SetWaitingForPlayers(const bool bWaiting, const TArray<FString>& Missing, const double EndSeconds)
{
	bWaitingForPlayers = bWaiting;
	MissingPlayers = Missing;
	WaitEndSeconds = EndSeconds;
	ForceNetUpdate();
}

void AHOLGameState::InitVictory(const FHOLMatchConfig& Config)
{
	LaunchConfig = Config;
	LaunchConfig.Normalize();
	UpdateStartPlaces();
	SlotDefeats.Init(EHOLDefeatReason::None, FHOLMatchConfig::MaxSlots);
	bMatchOver = false;
	WinnerMask = 0;
	ForumStates.Init(FHOLForumState(), FHOLMatchConfig::MaxSlots);
	ForumBuilders.Init(0, FHOLMatchConfig::MaxSlots);
	ForumPresenceOverrides.Reset();
	ForumPresenceOverrides.SetNum(FHOLMatchConfig::MaxSlots);
	PendingForumDefeats = 0;
	AbsentSlots.Reset();
	AbsentNotices.Reset();
	ResetDiplomacy();
	PausesLeft = FMath::Max(PausesPerMatch, 0);
	PausedBySlot = INDEX_NONE;
	PausedByName.Reset();
	CurrentStep = -1;
	// V3.2: who stands near a forum down, the living units counted by team, on the server.
	ForumPresenceCounter.Reset();
	ForumPresenceProvider = nullptr;
	if (HasAuthority())
	{
		ForumPresenceCounter = MakeShared<FHOLForumPresenceCounter>();
		ForumPresenceProvider = FHOLForumPresenceCounter::MakeProvider(ForumPresenceCounter.ToSharedRef());
	}

	// V2: the forums' phases and their deferred defeats follow the server's simulation steps (not the wall clock).
	UnbindSteps();
	UHOLSimulationSubsystem* Simulation = HasAuthority() && GetWorld() ? GetWorld()->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	if (Simulation)
	{
		StepStartedHandle = Simulation->GetOnStepStarted().AddUObject(this, &AHOLGameState::OnStepStarted);
		StepEndedHandle = Simulation->GetOnStepEnded().AddUObject(this, &AHOLGameState::OnStepEnded);
	}
}

void AHOLGameState::UpdateStartPlaces()
{
	// A configuration refused (FHOLMapSizeRules::CheckPlaces, logged by the game mode): no start for anyone.
	StartPlaces.Init(INDEX_NONE, FHOLMatchConfig::MaxSlots);
	if (!FHOLMapSizeRules::CheckPlaces(LaunchConfig))
	{
		return;
	}
	const TArray<int32> Places = FHOLMapSizeRules::GetPlaces(LaunchConfig);
	for (int32 Slot = 0; Slot < FMath::Min(Places.Num(), StartPlaces.Num()); ++Slot)
	{
		StartPlaces[Slot] = Places[Slot];
	}
}

void AHOLGameState::OnRep_LaunchConfig()
{
	UpdateStartPlaces();
}

int32 AHOLGameState::GetStartPlace(const int32 Slot) const
{
	if (!HasStartPlaces())
	{
		return Slot;
	}
	return StartPlaces.IsValidIndex(Slot) ? StartPlaces[Slot] : INDEX_NONE;
}

void AHOLGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindSteps();
	Super::EndPlay(EndPlayReason);
}

void AHOLGameState::UnbindSteps()
{
	UHOLSimulationSubsystem* Simulation = GetWorld() ? GetWorld()->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	if (Simulation)
	{
		Simulation->GetOnStepStarted().Remove(StepStartedHandle);
		Simulation->GetOnStepEnded().Remove(StepEndedHandle);
	}
	StepStartedHandle.Reset();
	StepEndedHandle.Reset();
}

void AHOLGameState::OnStepStarted(const int64 StepIndex)
{
	CurrentStep = StepIndex;
	// D5: the stances asked since the last step take effect now, before anything else of the step reads them.
	ApplyDiplomacyOrders();
	AdvanceForums();
}

void AHOLGameState::OnStepEnded(const int64 StepIndex)
{
	ApplyPendingDefeats();
}

void AHOLGameState::SetRelayViews(const TArray<FHOLRelayView>& Views)
{
	if (HasAuthority() && RelayViews != Views)
	{
		RelayViews = Views;
	}
}

void AHOLGameState::SetVolcanoViews(const TArray<FHOLVolcanoSiteNet>& Views)
{
	if (HasAuthority() && VolcanoViews != Views)
	{
		VolcanoViews = Views;
	}
}

const FHOLForumState& AHOLGameState::GetForumState(const int32 Slot) const
{
	static const FHOLForumState Standing;
	return ForumStates.IsValidIndex(Slot) ? ForumStates[Slot] : Standing;
}

int32 AHOLGameState::GetTierOverride(const int32 Slot) const
{
	return TierOverrides.IsValidIndex(Slot) ? TierOverrides[Slot] : -1;
}

void AHOLGameState::SetTierOverride(const int32 Slot, const int32 Tier)
{
	if (TierOverrides.IsValidIndex(Slot))
	{
		TierOverrides[Slot] = Tier < 0 ? -1 : FMath::Min(Tier, 3);
	}
}

int32 AHOLGameState::GetTier(const int32 Slot) const
{
	// G0 (D710): a tier forced by hol.Cheat.Tier wins over the hero's level, for the grid, the server, the technologies and the AI alike.
	if (GetTierOverride(Slot) >= 0)
	{
		return GetTierOverride(Slot);
	}
	// The level is the public sheet's on a client as on the server (UHOLHeroLibrary), so the tier needs nothing sent more.
	return HOLHeroXP::GetTier(UHOLHeroLibrary::GetHeroLevel(this, Slot));
}

FHOLForumTuning AHOLGameState::GetForumTuning(const int32 Slot) const
{
	FHOLForumTuning Tuning = FHOLForumTuning::FromSettings();
	Tuning.Tier = FMath::Max(Tuning.Tier, GetTier(Slot));
	return Tuning;
}

int32 AHOLGameState::GetLives(const int32 Slot) const
{
	return HOLForumRules::GetLives(GetForumState(Slot), GetForumTuning(Slot));
}

float AHOLGameState::GetForumRebuildProgress(const int32 Slot) const
{
	return HOLForumRules::GetRebuildProgress(GetForumState(Slot), GetForumTuning(Slot));
}

void AHOLGameState::SetForumState(const int32 Slot, const FHOLForumState& State)
{
	ForumStates[Slot] = State;
	ForceNetUpdate();
	// No OnRep on the server (listen or solo): its own player hears it here.
	OnForumsChanged.Broadcast();
	RefreshForumLooks();
}

namespace UE::HeroesOfLiberty::Forums
{
	/** V2.3a: the look of a forum changes with its phase, or its rise by a visible step (AHOLBuilding::RiseStepRatio). */
	bool LookChanged(const FHOLForumState& Before, const FHOLForumState& After, const FHOLForumTuning& Tuning)
	{
		return Before.Phase != After.Phase
			|| FMath::FloorToInt(HOLForumRules::GetRebuildProgress(Before, Tuning) / AHOLBuilding::RiseStepRatio)
				!= FMath::FloorToInt(HOLForumRules::GetRebuildProgress(After, Tuning) / AHOLBuilding::RiseStepRatio);
	}
}

void AHOLGameState::OnRep_Forums(const TArray<FHOLForumState>& OldForumStates)
{
	OnForumsChanged.Broadcast();
	const FHOLForumTuning Tuning = FHOLForumTuning::FromSettings();
	for (int32 Slot = 0; Slot < ForumStates.Num(); ++Slot)
	{
		if (!OldForumStates.IsValidIndex(Slot) || UE::HeroesOfLiberty::Forums::LookChanged(OldForumStates[Slot], ForumStates[Slot], Tuning))
		{
			RefreshForumLooks();
			return;
		}
	}
}

void AHOLGameState::RefreshForumLooks() const
{
	UWorld* World = GetWorld();
	if (!World || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	for (FHOLBuildingIterator It(World); It; ++It)
	{
		if (It->IsPrimaryForum())
		{
			It->NotifyForumChanged();
		}
	}
}

bool AHOLGameState::AnyForumBuildersWait() const
{
	for (const FHOLForumState& State : ForumStates)
	{
		if (HOLForumRules::BuildersWait(State))
		{
			return true;
		}
	}
	return false;
}

AHOLBuilding* AHOLGameState::FindPrimaryForum(const UWorld& World, const int32 Slot)
{
	for (FHOLBuildingIterator It(&World); It; ++It)
	{
		if (It->IsPrimaryForum() && It->GetOwnerPlayer() == Slot && !It->IsActorBeingDestroyed())
		{
			return *It;
		}
	}
	return nullptr;
}

bool AHOLGameState::HandleForumFall(AHOLBuilding& Forum)
{
	const int32 Slot = Forum.GetOwnerPlayer();
	if (!HasAuthority() || !Forum.IsPrimaryForum() || !ForumStates.IsValidIndex(Slot) || IsSlotDefeated(Slot)
		|| (PendingForumDefeats & (1 << Slot)) || HOLVictoryRules::GetCamp(LaunchConfig, Slot) == INDEX_NONE)
	{
		return false;
	}
	FHOLForumState State = ForumStates[Slot];
	const int32 Lives = HOLForumRules::Fall(State, GetForumTuning(Slot));
	if (Lives == INDEX_NONE)
	{
		return false;
	}
	// As a destruction (V2-Q3): its whole queue refunded; its rally point kept for after the rebuilding.
	const FHOLResources Refund = UHOLConstructionSubsystem::RefundProductionQueue(Forum);
	ForumBuilders[Slot] = 0;
	ForumHoldAnnounced[Slot] = 0;
	SetForumState(Slot, State);
	UE_LOG(LogHOLMatch, Display, TEXT("Victory: forum of slot %d fell, lives %d (queue refunded %s)"), Slot, Lives, *Refund.ToString());
	if (UHOLMatchStatsSubsystem* MatchStats = GetWorld() ? GetWorld()->GetSubsystem<UHOLMatchStatsSubsystem>() : nullptr)
	{
		MatchStats->RecordEvent(Slot, EHOLMatchStatsEvent::ForumFell);
	}
	// V4.1: sent at once, even from a Mass processor (combat): a reliable call queued, no entity touched.
	SendForumEvent(FHOLForumEvent::Make(EHOLForumEventKind::Fell, Slot, INDEX_NONE, Lives));
	if (Lives <= 0)
	{
		PendingForumDefeats |= 1 << Slot;
	}
	return true;
}

bool AHOLGameState::HandleKingDeath(const int32 Slot)
{
	if (!HasAuthority() || bMatchOver || !SlotDefeats.IsValidIndex(Slot) || IsSlotDefeated(Slot)
		|| (PendingKingDefeats & (1 << Slot)) || HOLVictoryRules::GetCamp(LaunchConfig, Slot) == INDEX_NONE)
	{
		return false;
	}
	UE_LOG(LogHOLMatch, Display, TEXT("Regicide: king of slot %d died"), Slot);
	// As the forum's fall: sent at once, even from a Mass processor; the defeat (its collapse) at the end of the step.
	SendForumEvent(FHOLForumEvent::Make(EHOLForumEventKind::KingDied, Slot));
	PendingKingDefeats |= 1 << Slot;
	return true;
}

void AHOLGameState::ApplyPendingDefeats()
{
	if (!PendingForumDefeats && !PendingKingDefeats)
	{
		return;
	}
	const uint8 PendingForum = PendingForumDefeats;
	const uint8 PendingKing = PendingKingDefeats;
	PendingForumDefeats = 0;
	PendingKingDefeats = 0;
	for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		if (PendingForum & (1 << Slot))
		{
			DefeatSlot(Slot, EHOLDefeatReason::Forum);
		}
		if (PendingKing & (1 << Slot))
		{
			DefeatSlot(Slot, EHOLDefeatReason::King);
		}
	}
}

void AHOLGameState::AdvanceForums()
{
	UWorld* World = GetWorld();
	if (!HasAuthority() || !World)
	{
		return;
	}
	for (int32 Slot = 0; Slot < ForumStates.Num(); ++Slot)
	{
		// A forum that fell for good waits for its defeat, which destroys it.
		if (!ForumStates[Slot].IsDown() || IsSlotDefeated(Slot) || (PendingForumDefeats & (1 << Slot)))
		{
			continue;
		}
		const FHOLForumTuning Tuning = GetForumTuning(Slot);
		FHOLForumState State = ForumStates[Slot];
		const FHOLForumState Old = State;
		const EHOLForumPhase Before = State.Phase;
		// V3.1: the hold with who stands in the zone (D348); it can raze the foundations, a new fall.
		const HOLForumRules::FStepResult Result = HOLForumRules::Step(State, Tuning, ForumBuilders.IsValidIndex(Slot) ? ForumBuilders[Slot] : 0,
			GetForumPresence(Slot));
		const bool bChanged = Result.bPhaseChanged;
		const bool bLookChanged = UE::HeroesOfLiberty::Forums::LookChanged(ForumStates[Slot], State, Tuning);
		const bool bHoldChanged = Old.Hold != State.Hold || (State.Hold == EHOLForumHold::Holding && Old.HolderSlot != State.HolderSlot);
		ForumStates[Slot] = State;
		// V4.2: an attacker new to Holding since the count was at 0 announced once (from None or from Contested).
		const bool bAnnounceHold = !Result.bRazed && HOLForumMessages::ShouldAnnounceHold(State, ForumHoldAnnounced[Slot]);
		if (Result.bRazed)
		{
			ForumBuilders[Slot] = 0;
			ForumHoldAnnounced[Slot] = 0;
			UE_LOG(LogHOLMatch, Display, TEXT("Victory: forum of slot %d razed, lives %d"), Slot, Result.Lives);
			SendForumEvent(FHOLForumEvent::Make(EHOLForumEventKind::Razed, Slot, INDEX_NONE, Result.Lives));
			SetForumState(Slot, State);
			if (Result.Lives <= 0)
			{
				// As a fall with no life left (V2): the defeat at the end of the step.
				PendingForumDefeats |= 1 << Slot;
			}
			continue;
		}
		if (bHoldChanged)
		{
			switch (State.Hold)
			{
			case EHOLForumHold::Holding:
				UE_LOG(LogHOLMatch, Display, TEXT("Victory: forum of slot %d held by slot %d (%d s)"), Slot, State.HolderSlot,
					HOLForumRules::GetHoldSeconds(State, Tuning));
				break;
			case EHOLForumHold::Contested:
				UE_LOG(LogHOLMatch, Display, TEXT("Victory: forum of slot %d contested (%d s)"), Slot, HOLForumRules::GetHoldSeconds(State, Tuning));
				break;
			case EHOLForumHold::Receding:
				UE_LOG(LogHOLMatch, Display, TEXT("Victory: forum of slot %d released (%d s)"), Slot, HOLForumRules::GetHoldSeconds(State, Tuning));
				break;
			default:
				UE_LOG(LogHOLMatch, Display, TEXT("Victory: forum of slot %d no longer held"), Slot);
				break;
			}
		}
		if (bAnnounceHold)
		{
			SendForumEvent(FHOLForumEvent::Make(EHOLForumEventKind::HoldStarted, Slot, State.HolderSlot));
		}
		if (!bChanged)
		{
			if (bHoldChanged)
			{
				// The HUD hears a change of hold at once (the ring of V3.3); the count itself travels without telling it.
				SetForumState(Slot, State);
				continue;
			}
			// The count and the progress travel too (the table, the selection block), without telling the HUD every step;
			// the forum rising again redoes its look by visible steps (V2.3a).
			ForceNetUpdate();
			if (bLookChanged)
			{
				RefreshForumLooks();
			}
			continue;
		}
		if (State.Phase == EHOLForumPhase::Standing)
		{
			if (AHOLBuilding* Forum = FindPrimaryForum(*World, Slot))
			{
				Forum->RestoreHealth();
			}
			ForumBuilders[Slot] = 0;
			UE_LOG(LogHOLMatch, Display, TEXT("Victory: forum of slot %d rebuilt"), Slot);
		}
		else
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Victory: forum of slot %d %s -> %s"), Slot, HOLForumRules::GetPhaseName(Before),
				HOLForumRules::GetPhaseName(State.Phase));
		}
		SetForumState(Slot, State);
	}
}

bool AHOLGameState::SkipForumPhase(const int32 Slot)
{
	if (!HasAuthority() || !ForumStates.IsValidIndex(Slot) || IsSlotDefeated(Slot) || (PendingForumDefeats & (1 << Slot)))
	{
		return false;
	}
	FHOLForumState State = ForumStates[Slot];
	const EHOLForumPhase Before = State.Phase;
	if (!HOLForumRules::Skip(State))
	{
		return false;
	}
	if (State.Phase == EHOLForumPhase::Standing)
	{
		if (AHOLBuilding* Forum = GetWorld() ? FindPrimaryForum(*GetWorld(), Slot) : nullptr)
		{
			Forum->RestoreHealth();
		}
		UE_LOG(LogHOLMatch, Display, TEXT("Victory: forum of slot %d rebuilt (skipped)"), Slot);
	}
	else
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Victory: forum of slot %d %s -> %s (skipped)"), Slot, HOLForumRules::GetPhaseName(Before),
			HOLForumRules::GetPhaseName(State.Phase));
	}
	SetForumState(Slot, State);
	return true;
}

void AHOLGameState::SetForumPresenceOverride(const int32 Slot, const FHOLForumPresence& Presence)
{
	if (Slot < 0 || Slot >= FHOLMatchConfig::MaxSlots)
	{
		return;
	}
	if (ForumPresenceOverrides.Num() < FHOLMatchConfig::MaxSlots)
	{
		ForumPresenceOverrides.SetNum(FHOLMatchConfig::MaxSlots);
	}
	FHOLForumPresence Fixed = Presence;
	if (Fixed.bAttackers && Fixed.AttackerSlot == INDEX_NONE)
	{
		// The first slot of another camp (D357's camps of the launch).
		const int32 Camp = HOLVictoryRules::GetCamp(LaunchConfig, Slot);
		for (int32 Other = 0; Other < FHOLMatchConfig::MaxSlots; ++Other)
		{
			const int32 OtherCamp = HOLVictoryRules::GetCamp(LaunchConfig, Other);
			if (Other != Slot && OtherCamp != INDEX_NONE && OtherCamp != Camp)
			{
				Fixed.AttackerSlot = Other;
				break;
			}
		}
	}
	ForumPresenceOverrides[Slot] = Fixed;
	UE_LOG(LogHOLMatch, Display, TEXT("Victory: presence at the forum of slot %d fixed: attackers %d (slot %d), defenders %d"), Slot,
		Fixed.bAttackers ? 1 : 0, Fixed.AttackerSlot, Fixed.bDefenders ? 1 : 0);
}

void AHOLGameState::ClearForumPresenceOverride(const int32 Slot)
{
	if (ForumPresenceOverrides.IsValidIndex(Slot))
	{
		ForumPresenceOverrides[Slot].Reset();
	}
}

FHOLForumPresence AHOLGameState::GetForumPresence(const int32 Slot) const
{
	if (ForumPresenceOverrides.IsValidIndex(Slot) && ForumPresenceOverrides[Slot].IsSet())
	{
		return ForumPresenceOverrides[Slot].GetValue();
	}
	return ForumPresenceProvider ? ForumPresenceProvider(*this, Slot) : FHOLForumPresence();
}

void AHOLGameState::SetForumBuilders(const int32 Slot, const int32 Builders)
{
	if (ForumBuilders.IsValidIndex(Slot))
	{
		ForumBuilders[Slot] = FMath::Max(Builders, 0);
	}
}

bool AHOLGameState::DefeatSlot(const int32 Slot, const EHOLDefeatReason Reason)
{
	if (!HasAuthority() || bMatchOver || Reason == EHOLDefeatReason::None || !SlotDefeats.IsValidIndex(Slot)
		|| IsSlotDefeated(Slot) || HOLVictoryRules::GetCamp(LaunchConfig, Slot) == INDEX_NONE)
	{
		return false;
	}
	SlotDefeats[Slot] = Reason;
	AIHeldMask &= ~(1 << Slot);
	// R7: a slot defeated while its player is away is no longer kept for him.
	AbsentSlots.RemoveAll([Slot](const FHOLAbsentSlot& Absent) { return Absent.Slot == Slot; });
	AbsentNotices.RemoveAll([Slot](const FHOLAbsentNotice& Notice) { return Notice.Slot == Slot; });
	if (UHOLAIBrainSubsystem* Brain = UHOLAIBrainSubsystem::Find(GetWorld()))
	{
		Brain->SetHeld(Slot, false);
	}
	UE_LOG(LogHOLMatch, Display, TEXT("Victory: slot %d defeated (%s)"), Slot, GetReasonName(Reason));
	if (UHOLMatchStatsSubsystem* MatchStats = GetWorld() ? GetWorld()->GetSubsystem<UHOLMatchStatsSubsystem>() : nullptr)
	{
		MatchStats->RecordEvent(Slot, EHOLMatchStatsEvent::Defeated);
	}
	// T8 (D722, answer B of Q-disconnect-left): a player who left and did not come back is a defeat like any other (the statistics
	// above counted it), and the event carries the reason, which picks the text « Bob a quitté la partie ».
	SendForumEvent(FHOLForumEvent::Make(EHOLForumEventKind::Defeated, Slot, INDEX_NONE, 0,
		Reason == EHOLDefeatReason::Disconnected ? EHOLForumEventReason::Disconnected : EHOLForumEventReason::None));
	CollapseSlot(Slot);
	// D717 (T2): the AI's player state of a defeated seat goes away (the seat is no longer held, nobody thinks for it).
	if (AHOLPlayerState* Bot = GetWorld() ? FindHeldBot(*GetWorld(), Slot) : nullptr)
	{
		Bot->Destroy();
	}
	OnSlotDefeated.Broadcast(Slot, Reason);

	EvaluateMatchEnd();
	ForceNetUpdate();
	// No OnRep on the server (listen or solo): its own player hears it here.
	OnVictoryChanged.Broadcast();
	return true;
}

bool AHOLGameState::EvaluateMatchEnd()
{
	if (!HasAuthority() || bMatchOver)
	{
		return false;
	}
	FHOLVictoryResult Result = HOLVictoryRules::Evaluate(LaunchConfig, GetDefeatedMask());
	if (!Result.bOver && IsDiplomacyMode() && GetDefeatedMask() != 0)
	{
		// D373: the survivors all allied to each other and at least one beaten: they win together, at once. A closed slot counts as beaten
		// for the rule (it is nobody's ally) but never makes the "at least one beaten" (the mask above holds only real defeats).
		TArray<bool> Gone;
		for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
		{
			Gone.Add(HOLVictoryRules::GetCamp(LaunchConfig, Slot) == INDEX_NONE || IsSlotDefeated(Slot));
		}
		const HOLDiplomacy::FAlliedVictory Allied = HOLDiplomacy::AlliedVictory(GetDiplomacyBook(), Gone);
		if (Allied.bEnded)
		{
			Result.bOver = true;
			for (const int32 Winner : Allied.Winners)
			{
				Result.WinnerMask |= static_cast<uint8>(1 << Winner);
			}
		}
	}
	if (!Result.bOver)
	{
		return false;
	}
	bMatchOver = true;
	WinnerMask = Result.WinnerMask;
	const TArray<int32> Winners = Result.GetWinners();
	UE_LOG(LogHOLMatch, Display, TEXT("Victory: match over, %s"), Winners.IsEmpty() ? TEXT("no winner")
		: *FString::Printf(TEXT("winners %s"), *FString::JoinBy(Winners, TEXT(","), [](const int32 Winner) { return FString::FromInt(Winner); })));
	OnMatchOver.Broadcast();
	return true;
}

bool AHOLGameState::KeepAbsentSlot(const AHOLPlayerState& Leaving, const double Now, const double WindowSeconds)
{
	// D717 (T4): an old connection closing late, while a newer one of his name already holds the seat: nobody is "away" under the new player.
	if (const UWorld* World = GetWorld(); World && !Leaving.IsOnlyASpectator() && FindLiveHolder(*World, Leaving.GetPlayerIndex(), &Leaving))
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Disconnect: slot %d left by an old connection, a newer one holds it: not kept"), Leaving.GetPlayerIndex());
		return false;
	}
	return !Leaving.IsOnlyASpectator()
		&& KeepMissingSlot(Leaving.GetPlayerIndex(), Leaving.GetResources(), Now, WindowSeconds, Leaving.GetTechnologies());
}

bool AHOLGameState::KeepMissingSlot(const int32 Slot, const FHOLResources& Resources, const double Now, const double WindowSeconds,
	const FHOLTechnologyState& Technologies)
{
	if (!HasAuthority() || IsSlotAbsent(Slot)
		|| !FHOLDisconnectRules::KeepsSlot(LaunchConfig, Slot, bWaitingForPlayers, bMatchOver, IsSlotDefeated(Slot)))
	{
		return false;
	}
	const FString Name = LaunchConfig.Slots[Slot].PlayerName.TrimStartAndEnd();
	AbsentSlots.Add({ Slot, Name, Now, Resources, Technologies });
	// D717 (T2): the AI's player state for his seat, with his stock and technologies: the brain's orders are paid on it.
	SpawnHeldBot(Slot, Name, Resources, Technologies);
	// R7.5: the banner of the others (a notice of his last return replaced).
	AbsentNotices.RemoveAll([Slot](const FHOLAbsentNotice& Notice) { return Notice.Slot == Slot; });
	AbsentNotices.Add({ Slot, Name, Now + WindowSeconds, 0., WindowSeconds });
	// D717 (T2): the AI holds his seat from now (base economy and defence), until he is back or the seat is defeated.
	if (UHOLAIBrainSubsystem* Brain = UHOLAIBrainSubsystem::Find(GetWorld()))
	{
		Brain->SetHeld(Slot, true);
		UE_LOG(LogHOLMatch, Display, TEXT("Disconnect: slot %d (%s) held by AI"), Slot, *Name);
	}
	ForceNetUpdate();
	UE_LOG(LogHOLMatch, Display, TEXT("Disconnect: slot %d (%s) kept, F %d W %d S %d G %d, %d technologies"), Slot, *Name, Resources.Food,
		Resources.Wood, Resources.Stone, Resources.Gold, Technologies.Done.Num());
	return true;
}

AHOLPlayerState* AHOLGameState::FindHeldBot(const UWorld& World, const int32 Slot)
{
	const AHOLGameState* HOLGameState = Cast<AHOLGameState>(World.GetGameState());
	if (!HOLGameState || !HOLGameState->LaunchConfig.Slots.IsValidIndex(Slot) || HOLGameState->LaunchConfig.Slots[Slot].State == EHOLSlotState::AI)
	{
		return nullptr;
	}
	for (APlayerState* Each : HOLGameState->PlayerArray)
	{
		AHOLPlayerState* HOLEach = Cast<AHOLPlayerState>(Each);
		if (HOLEach && HOLEach->IsABot() && HOLEach->GetPlayerIndex() == Slot)
		{
			return HOLEach;
		}
	}
	return nullptr;
}

TArray<int32> AHOLGameState::GetSlotsInUse(const AHOLPlayerState* Except) const
{
	TArray<int32> Slots;
	for (const APlayerState* Each : PlayerArray)
	{
		const AHOLPlayerState* HOLEach = Cast<AHOLPlayerState>(Each);
		if (HOLEach && HOLEach != Except && FindHeldBot(*GetWorld(), HOLEach->GetPlayerIndex()) != HOLEach)
		{
			Slots.Add(HOLEach->GetPlayerIndex());
		}
	}
	return Slots;
}

void AHOLGameState::SpawnHeldBot(const int32 Slot, const FString& Name, const FHOLResources& Resources, const FHOLTechnologyState& Technologies)
{
	UWorld* World = GetWorld();
	if (!World || !LaunchConfig.Slots.IsValidIndex(Slot) || LaunchConfig.Slots[Slot].State == EHOLSlotState::AI || FindHeldBot(*World, Slot))
	{
		return;
	}
	// As the AI's own seats (AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates): no controller, it lists itself in PlayerArray.
	FActorSpawnParameters Parameters;
	Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHOLPlayerState* Bot = World->SpawnActor<AHOLPlayerState>(AHOLPlayerState::StaticClass(), Parameters);
	if (!Bot)
	{
		return;
	}
	Bot->SetIsABot(true);
	Bot->SetPlayerName(Name);
	Bot->SetMatchSlot(Slot, LaunchConfig.Slots[Slot].ColorIndex);
	Bot->SetResources(Resources);
	// What he had was raised when he researched it: as it is, no second raising of the Rams.
	Bot->RestoreTechnologies(Technologies);
	AddPlayerState(Bot);
}

FHOLResources* AHOLGameState::FindAbsentStock(const UWorld& World, const int32 Slot)
{
	AHOLGameState* HOLGameState = Cast<AHOLGameState>(World.GetGameState());
	const int32 Index = HOLGameState ? FHOLDisconnectRules::FindAbsent(HOLGameState->AbsentSlots, Slot) : INDEX_NONE;
	return Index != INDEX_NONE ? &HOLGameState->AbsentSlots[Index].Resources : nullptr;
}

namespace UE::HeroesOfLiberty::Match
{
	AHOLPlayerState* FindSlotPlayerState(const UWorld& World, const int32 Slot)
	{
		const AGameStateBase* GameState = World.GetGameState();
		if (!GameState || Slot == INDEX_NONE)
		{
			return nullptr;
		}
		for (APlayerState* Each : GameState->PlayerArray)
		{
			AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(Each);
			if (HOLPlayerState && HOLPlayerState->GetPlayerIndex() == Slot)
			{
				return HOLPlayerState;
			}
		}
		return nullptr;
	}
}

bool AHOLGameState::GetSlotStock(const UWorld& World, const int32 Slot, FHOLResources& OutStock)
{
	// D717 (T2): the AI that holds the seat of an absent player has his stock (where its orders are paid), before anything else.
	if (const AHOLPlayerState* Bot = FindHeldBot(World, Slot))
	{
		OutStock = Bot->GetResources();
		return true;
	}
	// Absent first: between the Logout and its destruction, the leaving player state still holds the slot.
	if (const FHOLResources* Kept = FindAbsentStock(World, Slot))
	{
		OutStock = *Kept;
		return true;
	}
	const AHOLPlayerState* PlayerState = UE::HeroesOfLiberty::Match::FindSlotPlayerState(World, Slot);
	OutStock = PlayerState ? PlayerState->GetResources() : FHOLResources();
	return PlayerState != nullptr;
}

bool AHOLGameState::SetSlotStock(UWorld& World, const int32 Slot, const FHOLResources& Stock)
{
	if (AHOLPlayerState* Bot = FindHeldBot(World, Slot))
	{
		Bot->SetResources(Stock);
		return true;
	}
	if (FHOLResources* Kept = FindAbsentStock(World, Slot))
	{
		*Kept = Stock;
		return true;
	}
	AHOLPlayerState* PlayerState = UE::HeroesOfLiberty::Match::FindSlotPlayerState(World, Slot);
	if (PlayerState)
	{
		PlayerState->SetResources(Stock);
	}
	return PlayerState != nullptr;
}

bool AHOLGameState::AddSlotStock(UWorld& World, const int32 Slot, const FHOLResources& Gain, FHOLResources& OutStock)
{
	if (!GetSlotStock(World, Slot, OutStock))
	{
		return false;
	}
	OutStock += Gain;
	return SetSlotStock(World, Slot, OutStock);
}

bool AHOLGameState::GetSlotTechnologies(const UWorld& World, const int32 Slot, FHOLTechnologyState& OutTechnologies)
{
	// The AI that holds the seat first (D717, T2), then absent, as the stock.
	if (const AHOLPlayerState* Bot = FindHeldBot(World, Slot))
	{
		OutTechnologies = Bot->GetTechnologies();
		return true;
	}
	const AHOLGameState* HOLGameState = Cast<AHOLGameState>(World.GetGameState());
	const int32 Index = HOLGameState ? FHOLDisconnectRules::FindAbsent(HOLGameState->AbsentSlots, Slot) : INDEX_NONE;
	if (Index != INDEX_NONE)
	{
		OutTechnologies = HOLGameState->AbsentSlots[Index].Technologies;
		return true;
	}
	const AHOLPlayerState* PlayerState = UE::HeroesOfLiberty::Match::FindSlotPlayerState(World, Slot);
	OutTechnologies = PlayerState ? PlayerState->GetTechnologies() : FHOLTechnologyState();
	return PlayerState != nullptr;
}

bool AHOLGameState::AddSlotTechnology(UWorld& World, const int32 Slot, const FName Technology)
{
	if (AHOLPlayerState* Bot = FindHeldBot(World, Slot))
	{
		return Bot->AddTechnology(Technology);
	}
	AHOLGameState* HOLGameState = Cast<AHOLGameState>(World.GetGameState());
	const int32 Index = HOLGameState ? FHOLDisconnectRules::FindAbsent(HOLGameState->AbsentSlots, Slot) : INDEX_NONE;
	if (Index != INDEX_NONE)
	{
		return HOLGameState->AbsentSlots[Index].Technologies.Add(Technology);
	}
	AHOLPlayerState* PlayerState = UE::HeroesOfLiberty::Match::FindSlotPlayerState(World, Slot);
	return PlayerState && PlayerState->AddTechnology(Technology);
}

int32 AHOLGameState::FindSlotOnLogin(const FHOLMatchConfig& Config, const FString& Name, const TConstArrayView<int32> TakenSlots,
	const bool bLog) const
{
	const int32 Slot = Config.FindSlotForName(Name, TakenSlots);
	if (Slot == INDEX_NONE || !IsSlotDefeated(Slot))
	{
		return Slot;
	}
	if (!Config.HasPlayerNames())
	{
		// Without names (solo, direct): the next free human slot not defeated, if any.
		TArray<int32> Unavailable(TakenSlots);
		for (int32 Index = 0; Index < FHOLMatchConfig::MaxSlots; ++Index)
		{
			if (IsSlotDefeated(Index))
			{
				Unavailable.AddUnique(Index);
			}
		}
		return Config.FindSlotForName(Name, Unavailable);
	}
	if (bLog)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Disconnect: slot %d (%s) defeated, joins as spectator"), Slot, *Name.TrimStartAndEnd());
	}
	return INDEX_NONE;
}

int32 AHOLGameState::FindSlotForLogin(const FHOLMatchConfig& Config, const FString& Name, const AHOLPlayerState& Logging,
	AHOLPlayerState*& OutReplaced) const
{
	OutReplaced = nullptr;
	TArray<int32> Taken = GetSlotsInUse(&Logging);
	const UWorld* World = GetWorld();
	if (World && Config.HasPlayerNames())
	{
		// His name's seat, whoever holds it: a live holder of another connection is the old connection of this very player (the lobby's names
		// are unique), to be handed over to him, unless the seat is defeated (then he watches, as FindSlotOnLogin says).
		const int32 NameSlot = Config.FindSlotForName(Name, {});
		if (NameSlot != INDEX_NONE && !IsSlotDefeated(NameSlot) && Taken.Contains(NameSlot))
		{
			if (AHOLPlayerState* Old = FindLiveHolder(*World, NameSlot, &Logging))
			{
				OutReplaced = Old;
				Taken.Remove(NameSlot);
			}
		}
	}
	return FindSlotOnLogin(Config, Name, Taken);
}

void AHOLGameState::RetireHolder(AHOLPlayerState& Old, const double Now, const double WindowSeconds)
{
	const int32 Slot = Old.GetPlayerIndex();
	if (!HasAuthority() || Slot == INDEX_NONE)
	{
		return;
	}
	// What a Logout would keep for him (stock, technologies; the AI's player state holds them until his successor takes the seat, at once);
	// no banner for the others: nobody left, a player restarted.
	if (KeepMissingSlot(Slot, Old.GetResources(), Now, WindowSeconds, Old.GetTechnologies()))
	{
		AbsentNotices.RemoveAll([Slot](const FHOLAbsentNotice& Notice) { return Notice.Slot == Slot; });
	}
	// The old connection no longer holds the seat: its late Logout (KeepAbsentSlot) keeps nothing, and the seat is read on the new player.
	Old.SetIsOnlyASpectator(true);
	Old.SetMatchSlot(INDEX_NONE, Old.GetColorIndex());
	UE_LOG(LogHOLMatch, Display, TEXT("Disconnect: slot %d (%s) handed over from his old connection"), Slot, *Old.GetPlayerName());
	ForceNetUpdate();
}

AHOLPlayerState* AHOLGameState::FindLiveHolder(const UWorld& World, const int32 Slot, const AHOLPlayerState* Except)
{
	const AGameStateBase* HolderState = World.GetGameState();
	if (!HolderState || Slot == INDEX_NONE)
	{
		return nullptr;
	}
	for (APlayerState* Each : HolderState->PlayerArray)
	{
		AHOLPlayerState* HOLEach = Cast<AHOLPlayerState>(Each);
		if (HOLEach && HOLEach != Except && !HOLEach->IsABot() && !HOLEach->IsOnlyASpectator() && HOLEach->GetPlayerIndex() == Slot)
		{
			return HOLEach;
		}
	}
	return nullptr;
}

bool AHOLGameState::GiveSlotStock(AHOLPlayerState& Taking, const FHOLResources& Starting)
{
	return GiveSlotStock(Taking, Starting, GetServerWorldTimeSeconds());
}

bool AHOLGameState::GiveSlotStock(AHOLPlayerState& Taking, const FHOLResources& Starting, const double Now)
{
	const int32 Index = FHOLDisconnectRules::FindAbsent(AbsentSlots, Taking.GetPlayerIndex());
	// D717 (T2): the AI that held his seat has the stock and technologies as they are now (what it spent, what it researched).
	AHOLPlayerState* Bot = GetWorld() ? FindHeldBot(*GetWorld(), Taking.GetPlayerIndex()) : nullptr;
	if (Bot == &Taking)
	{
		Bot = nullptr;
	}
	if (Bot && AbsentSlots.IsValidIndex(Index))
	{
		AbsentSlots[Index].Resources = Bot->GetResources();
		AbsentSlots[Index].Technologies = Bot->GetTechnologies();
	}
	Taking.SetResources(FHOLDisconnectRules::GetStockOnTaking(AbsentSlots.IsValidIndex(Index) ? &AbsentSlots[Index] : nullptr, Starting));
	if (AbsentSlots.IsValidIndex(Index))
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Disconnect: slot %d (%s) back, %s"), AbsentSlots[Index].Slot, *AbsentSlots[Index].Name,
			*Taking.DescribeEconomy());
		const int32 Slot = AbsentSlots[Index].Slot;
		// D719 (T0): his technologies, those acquired and those whose research ended while he was away, with their effects.
		const FHOLTechnologyState Kept = AbsentSlots[Index].Technologies;
		AbsentSlots.RemoveAt(Index);
			AIHeldMask &= ~(1 << Slot);
		// D717 (T2): he is back, the AI lets his seat go (what it remembered of it is dropped).
		if (UHOLAIBrainSubsystem* Brain = UHOLAIBrainSubsystem::Find(GetWorld()))
		{
			Brain->SetHeld(Slot, false);
		}
		if (Bot)
		{
			// The AI's seat goes away first (the hero's sheet below is written to his new player state), his technologies as they are.
			Bot->Destroy();
			Taking.RestoreTechnologies(Kept);
		}
		else
		{
			for (const FName Technology : Kept.Done)
			{
				Taking.AddTechnology(Technology);
			}
		}
		// R7.10: his hero's private sheet (kept by the hero subsystem) in his new player state at once, as his stock.
		if (UHOLHeroSubsystem* Heroes = GetWorld() ? GetWorld()->GetSubsystem<UHOLHeroSubsystem>() : nullptr)
		{
			Heroes->RewriteCopies(Slot);
		}
		// R7.5: « Bob : de retour » for BackNoticeSeconds.
		if (FHOLAbsentNotice* Notice = AbsentNotices.FindByPredicate([Slot](const FHOLAbsentNotice& Each) { return Each.Slot == Slot; }))
		{
			Notice->BackAt = Now;
			ForceNetUpdate();
		}
		return true;
	}
	return false;
}

TArray<int32> AHOLGameState::DefeatExpiredAbsences(const double Now, const double WindowSeconds)
{
	// R7.5: the returns shown long enough.
	if (AbsentNotices.RemoveAll([Now](const FHOLAbsentNotice& Notice)
		{ return Notice.BackAt > 0. && Now >= Notice.BackAt + FHOLDisconnectRules::BackNoticeSeconds; }) > 0)
	{
		ForceNetUpdate();
	}
	TArray<int32> Defeated;
	for (const int32 Slot : FHOLDisconnectRules::GetExpired(AbsentSlots, Now, WindowSeconds))
	{
		// D718 (T5): by the camps of the launch, the AI keeps the seat (full brain, the banner out, his return open to the end of the match) or it is a defeat.
		if (FHOLDisconnectRules::GetWindowEnd(LaunchConfig, Slot) == FHOLDisconnectRules::EWindowEnd::AIHolds && GetWorld() && FindHeldBot(*GetWorld(), Slot))
		{
			AbsentSlots[FHOLDisconnectRules::FindAbsent(AbsentSlots, Slot)].bWindowOver = true;
				AIHeldMask |= 1 << Slot;
			AbsentNotices.RemoveAll([Slot](const FHOLAbsentNotice& Notice) { return Notice.Slot == Slot; });
			if (UHOLAIBrainSubsystem* Brain = UHOLAIBrainSubsystem::Find(GetWorld()))
			{
				Brain->SetFullBrain(Slot);
			}
			UE_LOG(LogHOLMatch, Display, TEXT("Disconnect: slot %d not back within %.0f s, the AI keeps the seat (full brain)"), Slot, WindowSeconds);
			ForceNetUpdate();
			continue;
		}
		AbsentSlots.RemoveAt(FHOLDisconnectRules::FindAbsent(AbsentSlots, Slot));
		UE_LOG(LogHOLMatch, Display, TEXT("Disconnect: slot %d not back within %.0f s"), Slot, WindowSeconds);
		// As a surrender (R-Q5): the collapse of V1 (D358), the camps' rules decide the end.
		if (DefeatSlot(Slot, EHOLDefeatReason::Disconnected))
		{
			Defeated.Add(Slot);
		}
	}
	return Defeated;
}

void AHOLGameState::OnRep_Victory()
{
	OnVictoryChanged.Broadcast();
}

void AHOLGameState::SendForumEvent(const FHOLForumEvent& Event)
{
	if (HasAuthority())
	{
		MulticastForumEvent(Event);
	}
}

bool AHOLGameState::DoesViewerSeeForum(const int32 ViewerSlot, const int32 ForumSlot) const
{
	const UWorld* World = GetWorld();
	const UHOLVisionSubsystem* Vision = World ? World->GetSubsystem<UHOLVisionSubsystem>() : nullptr;
	if (!Vision || !Vision->IsStarted() || Vision->GetMode() == EHOLFogMode::Disabled || ViewerSlot == INDEX_NONE)
	{
		return false;
	}
	if (Vision->GetCampIndex(ViewerSlot) != INDEX_NONE && Vision->GetViewerCamp(ViewerSlot) == INDEX_NONE)
	{
		return true;	// D376: a player defeated alone sees the whole map.
	}
	const AHOLStartSlot* Start = AHOLStartSlot::FindForSlot(World, ForumSlot);
	const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::MainCentre);
	return Start && Type && Vision->IsAreaVisible(ViewerSlot, Start->GetMainCentreLocation(), 0.5 * Type->Footprint);
}

void AHOLGameState::MulticastForumEvent_Implementation(const FHOLForumEvent& Event)
{
	const UWorld* World = GetWorld();
	if (World && World->GetNetMode() != NM_DedicatedServer)
	{
		// As this machine's player sees it (the players' table's way); a spectator, or no player, has no slot.
		int32 ViewerSlot = INDEX_NONE;
		if (const APlayerController* Controller = World->GetFirstPlayerController())
		{
			if (const AHOLPlayerState* PlayerState = Controller->GetPlayerState<AHOLPlayerState>())
			{
				ViewerSlot = PlayerState->GetPlayerIndex();
			}
		}
		const FHOLForumMessage Message = HOLForumMessages::GetMessage(Event, LaunchConfig, ViewerSlot, DoesViewerSeeForum(ViewerSlot, Event.Slot));
		UE_LOG(LogHOLMatch, Display, TEXT("%s"), *Message.Shown);
	}
	OnForumEvent.Broadcast(Event);
}

void AHOLGameState::CollapseSlot(const int32 Slot)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	// D358: its units killed as in combat (dying now, their fall and bodies at the next steps).
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	const int32 UnitsKilled = Units ? Units->KillUnitsOfOwner(static_cast<uint8>(Slot)) : 0;
	// Its buildings destroyed as in combat, sites and foundations too (their queues go with them: nothing produced after).
	TArray<AHOLBuilding*> Buildings;
	for (FHOLBuildingIterator It(World); It; ++It)
	{
		if (It->GetOwnerPlayer() == Slot && !It->IsActorBeingDestroyed())
		{
			Buildings.Add(*It);
		}
	}
	for (AHOLBuilding* Building : Buildings)
	{
		UHOLConstructionSubsystem::DestroyBuilding(*Building);
	}
	// Its stock lost, the refunds of the destruction included (D358: no refund).
	for (APlayerState* Each : PlayerArray)
	{
		AHOLPlayerState* PlayerState = Cast<AHOLPlayerState>(Each);
		if (PlayerState && PlayerState->GetPlayerIndex() == Slot)
		{
			PlayerState->SetResources(FHOLResources());
		}
	}
	if (UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>())
	{
		Commands->DropOrdersOf(Slot);
	}
	UE_LOG(LogHOLMatch, Display, TEXT("Victory: slot %d collapsed (%d units, %d buildings)"), Slot, UnitsKilled, Buildings.Num());
}

TArray<int32> AHOLGameState::GetWinners() const
{
	FHOLVictoryResult Result;
	Result.WinnerMask = WinnerMask;
	return Result.GetWinners();
}

const TCHAR* AHOLGameState::GetReasonName(const EHOLDefeatReason Reason)
{
	switch (Reason)
	{
	case EHOLDefeatReason::Surrender: return TEXT("Surrender");
	case EHOLDefeatReason::Cheat: return TEXT("Cheat");
	case EHOLDefeatReason::Forum: return TEXT("Forum");
	case EHOLDefeatReason::King: return TEXT("King");
	case EHOLDefeatReason::Disconnected: return TEXT("Disconnected");
	default: return TEXT("None");
	}
}

uint8 AHOLGameState::GetDefeatedMask() const
{
	uint8 Mask = 0;
	for (int32 Slot = 0; Slot < SlotDefeats.Num() && Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		if (SlotDefeats[Slot] != EHOLDefeatReason::None)
		{
			Mask |= 1 << Slot;
		}
	}
	return Mask;
}

TArray<FText> AHOLGameState::GetAbsenceBannerLines(const double Now) const
{
	TArray<FText> Lines;
	for (const FHOLAbsentNotice& Notice : AbsentNotices)
	{
		if (Notice.BackAt <= 0.)
		{
			Lines.Add(FHOLDisconnectRules::FormatAbsentLine(Notice.Name, Notice.EndsAt - Now, Notice.WindowSeconds));
		}
		// Read by the time: gone at its end on every machine, whenever the server drops it.
		else if (Now < Notice.BackAt + FHOLDisconnectRules::BackNoticeSeconds)
		{
			Lines.Add(FHOLDisconnectRules::FormatBackLine(Notice.Name));
		}
	}
	return Lines;
}

void AHOLGameState::SetFakeAbsentNotice(const FString& Name, const double SecondsLeft)
{
	if (!HasAuthority())
	{
		return;
	}
	const double Now = GetServerWorldTimeSeconds();
	if (Name.IsEmpty())
	{
		AbsentNotices.RemoveAll([](const FHOLAbsentNotice& Notice) { return Notice.Slot == INDEX_NONE; });
	}
	else if (FHOLAbsentNotice* Fake = AbsentNotices.FindByPredicate([&Name](const FHOLAbsentNotice& Notice)
		{ return Notice.Slot == INDEX_NONE && Notice.Name == Name; }))
	{
		Fake->BackAt = SecondsLeft < 0. ? Now : 0.;
		Fake->EndsAt = SecondsLeft < 0. ? Fake->EndsAt : Now + SecondsLeft;
		Fake->WindowSeconds = SecondsLeft < 0. ? Fake->WindowSeconds : SecondsLeft;
	}
	else
	{
		AbsentNotices.Add({ INDEX_NONE, Name, Now + FMath::Max(0., SecondsLeft), SecondsLeft < 0. ? Now : 0., FMath::Max(0., SecondsLeft) });
	}
	ForceNetUpdate();
}

const FHOLHeroSheet& AHOLGameState::GetHeroSheet(const int32 Slot) const
{
	static const FHOLHeroSheet NoHero;
	return HeroSheets.IsValidIndex(Slot) ? HeroSheets[Slot] : NoHero;
}

bool AHOLGameState::SetHeroSheet(const int32 Slot, const FHOLHeroSheet& Sheet)
{
	if (!HeroSheets.IsValidIndex(Slot) || HeroSheets[Slot] == Sheet)
	{
		return false;
	}
	HeroSheets[Slot] = Sheet;
	ForceNetUpdate();
	// No OnRep on the server (listen or solo): its own player hears it here.
	NoteHeroReady();
	NoteHeroLevels();
	OnHeroSheetsChanged.Broadcast();
	return true;
}

void AHOLGameState::OnRep_HeroSheets()
{
	NoteHeroReady();
	NoteHeroLevels();
	OnHeroSheetsChanged.Broadcast();
}

void AHOLGameState::NoteHeroLevels()
{
	const UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	for (int32 Slot = 0; Slot < HeroSheets.Num(); ++Slot)
	{
		const int32 Level = HeroSheets[Slot].Level;
		const int32 OldLevel = HeroLevelWatch.Update(Slot, Level);
		if (OldLevel > 0)
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Hero level up: slot %d level %d -> %d"), Slot, OldLevel, Level);
			OnHeroLevelUp.Broadcast(Slot, OldLevel, Level);
		}
	}
}

void AHOLGameState::NoteTributeReceived(const int32 GiverSlot, const int32 ReceiverSlot, const int32 Resource, const int32 Arrives, const int32 Tax)
{
	const FHOLForumMessage Message = HOLForumMessages::GetTributeMessage(GiverSlot, Resource, Arrives, Tax, GetLaunchConfig(), ReceiverSlot);
	UE_LOG(LogHOLMatch, Display, TEXT("%s (slot %d)"), *Message.Shown, ReceiverSlot);
	OnTributeMessage.Broadcast(ReceiverSlot, Message);
}

void AHOLGameState::NoteHeroReady()
{
	const UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	// This machine's player (a spectator, or no player, has no slot: never).
	int32 ViewerSlot = INDEX_NONE;
	if (const APlayerController* Controller = World->GetFirstPlayerController())
	{
		if (const AHOLPlayerState* PlayerState = Controller->GetPlayerState<AHOLPlayerState>())
		{
			ViewerSlot = PlayerState->GetPlayerIndex();
		}
	}
	if (HeroReadyWatch.Update(ViewerSlot, GetHeroSheet(ViewerSlot).State))
	{
		const FHOLForumMessage Message = HOLForumMessages::GetHeroReadyMessage();
		UE_LOG(LogHOLMatch, Display, TEXT("%s (slot %d)"), *Message.Shown, ViewerSlot);
		OnHeroReadyMessage.Broadcast(Message);
	}
}

double AHOLGameState::GetPauseTimerLeft(const double Now) const
{
	if (IsPauseFree() || !IsMatchPaused())
	{
		return 0.;
	}
	if (GetNetMode() == NM_Client)
	{
		return FMath::Max(FHOLPauseRules::DefaultTimerSeconds - (FPlatformTime::Seconds() - PauseRealStartSeconds), 0.);
	}
	FHOLPauseRules::FState State;
	State.PausedBySlot = PausedBySlot;
	State.PausedAtSeconds = PausedAtSeconds;
	return FHOLPauseRules::TimerLeft(State, Now);
}

void AHOLGameState::NotifyPauseChanged()
{
	UWorld* World = GetWorld();
	if (World && GetNetMode() != NM_DedicatedServer && IsMatchPaused() != bLocalPauseApplied)
	{
		bLocalPauseApplied = IsMatchPaused();
		if (GetNetMode() == NM_Client && IsMatchPaused())
		{
			// Before the local pause: the server's clock as known now, less what has run since the pause.
			PauseRealStartSeconds = FPlatformTime::Seconds() - FMath::Max(GetServerWorldTimeSeconds() - PausedAtSeconds, 0.);
		}
		const APlayerController* Local = World->GetFirstPlayerController();
		if (AWorldSettings* Settings = World->GetWorldSettings(); Settings && Local)
		{
			Settings->SetPauserPlayerState(IsMatchPaused() ? Local->PlayerState : nullptr);
			UE_LOG(LogHOLMatch, Display, TEXT("Pause: this machine's world %s"), World->IsPaused() ? TEXT("paused") : TEXT("running"));
		}
	}
	OnPauseChanged.Broadcast();
}

bool AHOLGameState::IsInMatch(const AHOLPlayerState& Requester) const
{
	const int32 Slot = Requester.GetPlayerIndex();
	return Slot != INDEX_NONE && !Requester.IsOnlyASpectator() && !IsSlotDefeated(Slot) && !bMatchOver;
}

bool AHOLGameState::RequestPause(const AHOLPlayerState& Requester, const double Now)
{
	FHOLPauseRules::FState State;
	State.PausesLeft = PausesLeft;
	State.PausedBySlot = PausedBySlot;
	State.PausedAtSeconds = PausedAtSeconds;
	const int32 Slot = Requester.GetPlayerIndex();
	const FHOLPauseRules::EAnswer Answer = FHOLPauseRules::CanPause(GetNetMode(), State, IsInMatch(Requester));
	if (Answer != FHOLPauseRules::EAnswer::Allowed)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Pause: refused to %s (slot %d): %s"), *Requester.GetPlayerName(), Slot, FHOLPauseRules::ToString(Answer));
		return false;
	}
	FHOLPauseRules::Pause(GetNetMode(), State, Slot, Now);
	PausesLeft = State.PausesLeft;
	PausedBySlot = State.PausedBySlot;
	PausedAtSeconds = State.PausedAtSeconds;
	PausedByName = Requester.GetPlayerName();
	UE_LOG(LogHOLMatch, Display, TEXT("Pause: slot %d (%s) paused, %d left"), Slot, *PausedByName, PausesLeft);
	ForceNetUpdate();
	NotifyPauseChanged();
	return true;
}

bool AHOLGameState::RequestResume(const AHOLPlayerState& Requester, const double Now)
{
	FHOLPauseRules::FState State;
	State.PausesLeft = PausesLeft;
	State.PausedBySlot = PausedBySlot;
	State.PausedAtSeconds = PausedAtSeconds;
	const int32 Slot = Requester.GetPlayerIndex();
	const FHOLPauseRules::EAnswer Answer = FHOLPauseRules::CanResume(GetNetMode(), State, Slot, IsInMatch(Requester), Now);
	if (Answer != FHOLPauseRules::EAnswer::Allowed)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Pause: refused to %s (slot %d): %s"), *Requester.GetPlayerName(), Slot, FHOLPauseRules::ToString(Answer));
		return false;
	}
	WindowPausedSeconds += FMath::Max(Now - PausedAtSeconds, 0.);
	PausedBySlot = INDEX_NONE;
	PausedByName.Reset();
	UE_LOG(LogHOLMatch, Display, TEXT("Pause: resumed by %s (slot %d), %d left"), *Requester.GetPlayerName(), Slot, PausesLeft);
	ForceNetUpdate();
	NotifyPauseChanged();
	return true;
}

bool AHOLGameState::LiftPauseOf(const int32 Slot, const double Now)
{
	if (!HasAuthority() || !IsMatchPaused() || PausedBySlot != Slot)
	{
		return false;
	}
	// D720: the pause of a player who leaves is lifted at once; it stays counted (PausesLeft untouched).
	WindowPausedSeconds += FMath::Max(Now - PausedAtSeconds, 0.);
	UE_LOG(LogHOLMatch, Display, TEXT("Pause: lifted, %s (slot %d) left, %d left"), *PausedByName, Slot, PausesLeft);
	PausedBySlot = INDEX_NONE;
	PausedByName.Reset();
	ForceNetUpdate();
	NotifyPauseChanged();
	return true;
}

void AHOLGameState::OnRep_Pause()
{
	NotifyPauseChanged();
}

HOLMarket::FBook AHOLGameState::GetMarketBook() const
{
	HOLMarket::FBook Book;
	if (MarketPrices.Num() == HOLMarket::BookSize)
	{
		for (int32 Resource = 0; Resource < HOLMarket::BookSize; ++Resource)
		{
			Book.Prices[Resource] = MarketPrices[Resource];
		}
	}
	return Book;
}

void AHOLGameState::SetMarketBook(const HOLMarket::FBook& InBook)
{
	MarketPrices.SetNum(HOLMarket::BookSize);
	for (int32 Resource = 0; Resource < HOLMarket::BookSize; ++Resource)
	{
		MarketPrices[Resource] = InBook.Prices[Resource];
	}
}

void AHOLGameState::ResetMarketBook()
{
	HOLMarket::FSettings Settings;
	if (HOLMarket::LoadStandardSettings(Settings))
	{
		SetMarketBook(HOLMarket::NewBook(Settings));
	}
}

namespace
{
	const TCHAR* StanceName(const int32 Stance)
	{
		return Stance == HOLDiplomacy::Ally ? TEXT("Ally") : Stance == HOLDiplomacy::Neutral ? TEXT("Neutral") : TEXT("Enemy");
	}
}

HOLDiplomacy::FBook AHOLGameState::GetDiplomacyBook() const
{
	constexpr int32 Slots = FHOLMatchConfig::MaxSlots;
	TArray<bool> IsAI;
	TArray<int32> TeamBySlot;
	for (int32 Slot = 0; Slot < Slots; ++Slot)
	{
		const bool bValid = LaunchConfig.Slots.IsValidIndex(Slot);
		IsAI.Add(bValid && LaunchConfig.Slots[Slot].State == EHOLSlotState::AI);
		TeamBySlot.Add(bValid ? LaunchConfig.Slots[Slot].Team : 0);
	}
	if (!IsDiplomacyMode())
	{
		return HOLDiplomacy::NewTeamBook(TeamBySlot, IsAI);
	}
	HOLDiplomacy::FBook Book = HOLDiplomacy::NewBook(Slots, IsAI);
	if (DiplomacyStances.Num() == Slots * Slots && DiplomacyVision.Num() == Slots * Slots)
	{
		for (int32 Index = 0; Index < Slots * Slots; ++Index)
		{
			Book.Stances[Index] = DiplomacyStances[Index];
			Book.Vision[Index] = DiplomacyVision[Index] != 0;
		}
	}
	return Book;
}

int32 AHOLGameState::GetDiplomacyRelation(const int32 A, const int32 B) const
{
	return HOLDiplomacy::Relation(GetDiplomacyBook(), A, B);
}

void AHOLGameState::ResetDiplomacy()
{
	DiplomacyStances.Reset();
	DiplomacyVision.Reset();
	if (HasAuthority() && IsDiplomacyMode())
	{
		StoreDiplomacyBook(GetDiplomacyBook());
	}
}

void AHOLGameState::StoreDiplomacyBook(const HOLDiplomacy::FBook& Book)
{
	DiplomacyStances.SetNum(Book.Stances.Num());
	DiplomacyVision.SetNum(Book.Vision.Num());
	for (int32 Index = 0; Index < Book.Stances.Num(); ++Index)
	{
		DiplomacyStances[Index] = static_cast<int8>(Book.Stances[Index]);
		DiplomacyVision[Index] = Book.Vision[Index] ? 1 : 0;
	}
	ForceNetUpdate();
	OnDiplomacyChanged.Broadcast();
}

HOLDiplomacy::FStanceChange AHOLGameState::ApplyStance(const int32 From, const int32 To, const int32 Stance)
{
	HOLDiplomacy::FStanceChange Change;
	const auto Occupied = [this](const int32 Slot) { return LaunchConfig.Slots.IsValidIndex(Slot) && LaunchConfig.Slots[Slot].State != EHOLSlotState::Closed; };
	if (!HasAuthority() || !IsDiplomacyMode() || !Occupied(From) || !Occupied(To))
	{
		Change.Result = HOLDiplomacy::EResult::NoSuchSlot;
		UE_LOG(LogHOLMatch, Display, TEXT("Diplomacy: slot %d toward slot %d (%s) refused (%s)"), From, To, StanceName(Stance),
			!IsDiplomacyMode() ? TEXT("not the Diplomacy mode") : TEXT("no such player"));
		return Change;
	}
	HOLDiplomacy::FBook Book = GetDiplomacyBook();
	Change = HOLDiplomacy::SetStance(Book, From, To, Stance);
	if (Change.Result != HOLDiplomacy::EResult::Ok)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Diplomacy: slot %d toward slot %d (%s) refused (code %d)"), From, To, StanceName(Stance), static_cast<int32>(Change.Result));
		return Change;
	}
	if (Change.bAnnounce)
	{
		StoreDiplomacyBook(Book);
		UE_LOG(LogHOLMatch, Display, TEXT("Diplomacy: slot %d now %s toward slot %d (relation %s -> %s)%s%s"), From, StanceName(Stance), To,
			StanceName(Change.Before), StanceName(Change.After), Change.bBroke ? TEXT(", alliance broken") : TEXT(""),
			Change.bFormed ? TEXT(", alliance formed") : TEXT(""));
		OnDiplomacyAnnounced.Broadcast(Change);
		// D373: an alliance formed after the last defeat can end the match at that second.
		if (Change.bFormed && EvaluateMatchEnd())
		{
			ForceNetUpdate();
			OnVictoryChanged.Broadcast();
		}
	}
	return Change;
}

HOLDiplomacy::FVisionChange AHOLGameState::ApplyVision(const int32 From, const int32 To, const bool bShare)
{
	HOLDiplomacy::FVisionChange Change;
	if (!HasAuthority() || !IsDiplomacyMode())
	{
		Change.Result = HOLDiplomacy::EResult::NoSuchSlot;
		return Change;
	}
	HOLDiplomacy::FBook Book = GetDiplomacyBook();
	Change = HOLDiplomacy::SetVision(Book, From, To, bShare);
	if (Change.Result != HOLDiplomacy::EResult::Ok)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Diplomacy: slot %d vision toward slot %d refused (code %d)"), From, To, static_cast<int32>(Change.Result));
		return Change;
	}
	if (Change.bAnnounce)
	{
		StoreDiplomacyBook(Book);
		UE_LOG(LogHOLMatch, Display, TEXT("Diplomacy: slot %d %s its vision with slot %d"), From, bShare ? TEXT("shares") : TEXT("stops sharing"), To);
	}
	return Change;
}

void AHOLGameState::ApplyDiplomacyOrders()
{
	UHOLCommandSubsystem* Commands = HasAuthority() && GetWorld() ? GetWorld()->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	if (!Commands)
	{
		return;
	}
	TArray<FHOLDiplomacyOrder> Orders;
	Commands->ConsumeDiplomacyOrders(Orders);
	for (const FHOLDiplomacyOrder& Order : Orders)
	{
		if (Order.Kind == EHOLDiplomacyOrderKind::Stance)
		{
			ApplyStance(Order.PlayerIndex, Order.OtherSlot, Order.Value);
		}
		else
		{
			ApplyVision(Order.PlayerIndex, Order.OtherSlot, Order.Value != 0);
		}
	}
}

void AHOLGameState::OnRep_Diplomacy()
{
	UE_LOG(LogHOLMatch, Display, TEXT("Diplomacy: state received"));
	OnDiplomacyChanged.Broadcast();
}

void AHOLGameState::ReportDiplomacy() const
{
	const HOLDiplomacy::FBook Book = GetDiplomacyBook();
	UE_LOG(LogHOLMatch, Display, TEXT("Diplomacy report: mode %s%s"), IsDiplomacyMode() ? TEXT("Diplomacy") : TEXT("Standard"), Book.bLocked ? TEXT(" (locked)") : TEXT(""));
	const auto Occupied = [this](const int32 Slot) { return LaunchConfig.Slots.IsValidIndex(Slot) && LaunchConfig.Slots[Slot].State != EHOLSlotState::Closed; };
	for (int32 From = 0; From < FHOLMatchConfig::MaxSlots; ++From)
	{
		for (int32 To = 0; To < FHOLMatchConfig::MaxSlots; ++To)
		{
			if (From != To && Occupied(From) && Occupied(To))
			{
				UE_LOG(LogHOLMatch, Display, TEXT("Diplomacy report: slot %d toward slot %d: stance %s, relation %s, vision %d, fires %d, adversary %d"), From, To,
					StanceName(HOLDiplomacy::StanceOf(Book, From, To)), StanceName(HOLDiplomacy::Relation(Book, From, To)),
					HOLDiplomacy::SharesVision(Book, From, To) ? 1 : 0, HOLDiplomacy::FiresAtOnce(Book, From, To) ? 1 : 0,
					HOLDiplomacy::IsAdversary(Book, From, To) ? 1 : 0);
			}
		}
	}
}

uint8 AHOLGameState::GetAbsentMask() const
{
	uint8 Mask = 0;
	for (const FHOLAbsentNotice& Notice : AbsentNotices)
	{
		if (Notice.Slot >= 0 && Notice.Slot < FHOLMatchConfig::MaxSlots && Notice.BackAt <= 0.)
		{
			Mask |= 1 << Notice.Slot;
		}
	}
	return Mask;
}

TArray<FString> AHOLGameState::DescribeDisconnections(const double Now, const double WindowSeconds) const
{
	TArray<FString> Lines;
	for (int32 Slot = 0; Slot < LaunchConfig.Slots.Num() && Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		const FString Name = LaunchConfig.Slots[Slot].PlayerName.TrimStartAndEnd();
		if (LaunchConfig.Slots[Slot].State != EHOLSlotState::Human || Name.IsEmpty())
		{
			continue;
		}
		const int32 Index = FHOLDisconnectRules::FindAbsent(AbsentSlots, Slot);
		using ESeat = FHOLDisconnectRules::ESeatState;
		const ESeat State = IsSlotDefeated(Slot) ? ESeat::Defeated : Index == INDEX_NONE ? ESeat::Present
			: AbsentSlots[Index].bWindowOver ? ESeat::AIHolds : ESeat::AbsentAIHolds;
		Lines.Add(FHOLDisconnectRules::FormatSeatReport(Slot, Name, State,
			Index != INDEX_NONE ? FHOLDisconnectRules::GetSecondsLeft(AbsentSlots[Index].LeftAt, Now, WindowSeconds) : 0.));
	}
	return Lines;
}
