// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLGameState.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Commands/HOLCommandSubsystem.h"
#include "EngineUtils.h"
#include "HeroesOfLibertyLog.h"
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
	PendingForumDefeats = 0;

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
	SetForumState(Slot, State);
	UE_LOG(LogHOLMatch, Display, TEXT("Victory: forum of slot %d fell, lives %d (queue refunded %s)"), Slot, Lives, *Refund.ToString());
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
		const EHOLForumPhase Before = State.Phase;
		const bool bChanged = HOLForumRules::Advance(State, Tuning, ForumBuilders.IsValidIndex(Slot) ? ForumBuilders[Slot] : 0);
		const bool bLookChanged = UE::HeroesOfLiberty::Forums::LookChanged(ForumStates[Slot], State, Tuning);
		ForumStates[Slot] = State;
		if (!bChanged)
		{
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
