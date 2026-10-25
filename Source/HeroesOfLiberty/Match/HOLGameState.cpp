// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLGameState.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Commands/HOLCommandSubsystem.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLForumPresence.h"
#include "Match/HOLMatchStatsSubsystem.h"
#include "Match/HOLPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

AHOLGameState::AHOLGameState()
{
	SlotDefeats.Init(EHOLDefeatReason::None, FHOLMatchConfig::MaxSlots);
	ForumStates.Init(FHOLForumState(), FHOLMatchConfig::MaxSlots);
	ForumBuilders.Init(0, FHOLMatchConfig::MaxSlots);
	LaunchConfig = FHOLMatchConfig::MakeDefault();
}

void AHOLGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHOLGameState, SlotDefeats);
	DOREPLIFETIME(AHOLGameState, bMatchOver);
	DOREPLIFETIME(AHOLGameState, WinnerMask);
	DOREPLIFETIME(AHOLGameState, LaunchConfig);
	DOREPLIFETIME(AHOLGameState, ForumStates);
	DOREPLIFETIME(AHOLGameState, bWaitingForPlayers);
	DOREPLIFETIME(AHOLGameState, MissingPlayers);
	DOREPLIFETIME(AHOLGameState, WaitEndSeconds);
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
	SlotDefeats.Init(EHOLDefeatReason::None, FHOLMatchConfig::MaxSlots);
	bMatchOver = false;
	WinnerMask = 0;
	ForumStates.Init(FHOLForumState(), FHOLMatchConfig::MaxSlots);
	ForumBuilders.Init(0, FHOLMatchConfig::MaxSlots);
	ForumPresenceOverrides.Reset();
	ForumPresenceOverrides.SetNum(FHOLMatchConfig::MaxSlots);
	PendingForumDefeats = 0;
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
	AdvanceForums();
}

void AHOLGameState::OnStepEnded(const int64 StepIndex)
{
	ApplyPendingDefeats();
}

const FHOLForumState& AHOLGameState::GetForumState(const int32 Slot) const
{
	static const FHOLForumState Standing;
	return ForumStates.IsValidIndex(Slot) ? ForumStates[Slot] : Standing;
}

int32 AHOLGameState::GetLives(const int32 Slot) const
{
	return HOLForumRules::GetLives(GetForumState(Slot), FHOLForumTuning::FromSettings());
}

float AHOLGameState::GetForumRebuildProgress(const int32 Slot) const
{
	return HOLForumRules::GetRebuildProgress(GetForumState(Slot), FHOLForumTuning::FromSettings());
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
	for (TActorIterator<AHOLBuilding> It(World); It; ++It)
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
	for (TActorIterator<AHOLBuilding> It(&World); It; ++It)
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
	const int32 Lives = HOLForumRules::Fall(State, FHOLForumTuning::FromSettings());
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

void AHOLGameState::ApplyPendingDefeats()
{
	if (!PendingForumDefeats)
	{
		return;
	}
	const uint8 Pending = PendingForumDefeats;
	PendingForumDefeats = 0;
	for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		if (Pending & (1 << Slot))
		{
			DefeatSlot(Slot, EHOLDefeatReason::Forum);
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
	FHOLForumTuning Tuning;
	bool bTuning = false;
	for (int32 Slot = 0; Slot < ForumStates.Num(); ++Slot)
	{
		// A forum that fell for good waits for its defeat, which destroys it.
		if (!ForumStates[Slot].IsDown() || IsSlotDefeated(Slot) || (PendingForumDefeats & (1 << Slot)))
		{
			continue;
		}
		if (!bTuning)
		{
			Tuning = FHOLForumTuning::FromSettings();
			bTuning = true;
		}
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
	UE_LOG(LogHOLMatch, Display, TEXT("Victory: slot %d defeated (%s)"), Slot, GetReasonName(Reason));
	if (UHOLMatchStatsSubsystem* MatchStats = GetWorld() ? GetWorld()->GetSubsystem<UHOLMatchStatsSubsystem>() : nullptr)
	{
		MatchStats->RecordEvent(Slot, EHOLMatchStatsEvent::Defeated);
	}
	SendForumEvent(FHOLForumEvent::Make(EHOLForumEventKind::Defeated, Slot));
	CollapseSlot(Slot);
	OnSlotDefeated.Broadcast(Slot, Reason);

	const FHOLVictoryResult Result = HOLVictoryRules::Evaluate(LaunchConfig, GetDefeatedMask());
	if (Result.bOver)
	{
		bMatchOver = true;
		WinnerMask = Result.WinnerMask;
		const TArray<int32> Winners = Result.GetWinners();
		UE_LOG(LogHOLMatch, Display, TEXT("Victory: match over, %s"), Winners.IsEmpty() ? TEXT("no winner")
			: *FString::Printf(TEXT("winners %s"), *FString::JoinBy(Winners, TEXT(","), [](const int32 Winner) { return FString::FromInt(Winner); })));
		OnMatchOver.Broadcast();
	}
	ForceNetUpdate();
	// No OnRep on the server (listen or solo): its own player hears it here.
	OnVictoryChanged.Broadcast();
	return true;
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
		const FHOLForumMessage Message = HOLForumMessages::GetMessage(Event, LaunchConfig, ViewerSlot);
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
	for (TActorIterator<AHOLBuilding> It(World); It; ++It)
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
