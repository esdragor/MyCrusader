// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLGameState.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Commands/HOLCommandSubsystem.h"
#include "EngineUtils.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Simulation/HOLUnitSubsystem.h"

AHOLGameState::AHOLGameState()
{
	SlotDefeats.Init(EHOLDefeatReason::None, FHOLMatchConfig::MaxSlots);
	LaunchConfig = FHOLMatchConfig::MakeDefault();
}

void AHOLGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHOLGameState, SlotDefeats);
	DOREPLIFETIME(AHOLGameState, bMatchOver);
	DOREPLIFETIME(AHOLGameState, WinnerMask);
	DOREPLIFETIME(AHOLGameState, LaunchConfig);
}

void AHOLGameState::InitVictory(const FHOLMatchConfig& Config)
{
	LaunchConfig = Config;
	LaunchConfig.Normalize();
	SlotDefeats.Init(EHOLDefeatReason::None, FHOLMatchConfig::MaxSlots);
	bMatchOver = false;
	WinnerMask = 0;
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
