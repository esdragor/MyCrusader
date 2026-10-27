// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLHeroSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "MassEntitySubsystem.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"
#include "Match/HOLStartRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

namespace UE::HeroesOfLiberty::Hero
{
	FString DescribeState(const EHOLHeroState State)
	{
		return StaticEnum<EHOLHeroState>()->GetNameStringByValue(static_cast<int64>(State));
	}

	FString DescribePrivate(const FHOLHeroPrivateSheet& Private)
	{
		return FString::Printf(TEXT("xp %d death %d cooldowns %d"), Private.XP, Private.DeathStep, Private.CooldownEndSteps.Num());
	}

	/**
	 * Any machine: each slot's public sheet as the game state has it ("Hero sheet slot 1: state Alive level 1 hp 447"), the
	 * local player's private sheet ("Hero private slot 1 local: xp 7 death -1 cooldowns 0"), and the private values of
	 * every other player state as they reached this machine ("Hero private slot 0 other: ..."), default on a client
	 * (COND_OwnerOnly). On the server or in solo, the true sheets too ("Hero true slot 1: state Alive level 1 hp 447 xp 7 ...").
	 */
	FAutoConsoleCommandWithWorld CommandReport(TEXT("hol.Hero.Report"),
		TEXT("Hero sheets (J8-Q5): each slot's public sheet, the local player's private sheet and the private values of the other player states as this machine has them"),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
			if (!GameState)
			{
				UE_LOG(LogHOLMatch, Display, TEXT("Hero report: no game state"));
				return;
			}
			const ENetMode NetMode = World->GetNetMode();
			UE_LOG(LogHOLMatch, Display, TEXT("Hero report: net mode %d"), static_cast<int32>(NetMode));
			for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
			{
				const FHOLHeroSheet& Sheet = GameState->GetHeroSheet(Slot);
				UE_LOG(LogHOLMatch, Display, TEXT("Hero sheet slot %d: state %s level %d hp %d"), Slot, *DescribeState(Sheet.State), Sheet.Level, Sheet.Health);
			}
			const APlayerController* LocalController = World->GetFirstPlayerController();
			const APlayerState* Local = LocalController && LocalController->IsLocalController() ? LocalController->PlayerState.Get() : nullptr;
			for (const APlayerState* PlayerState : GameState->PlayerArray)
			{
				if (const AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState))
				{
					UE_LOG(LogHOLMatch, Display, TEXT("Hero private slot %d %s: %s"), HOLPlayerState->GetPlayerIndex(),
						PlayerState == Local ? TEXT("local") : TEXT("other"), *DescribePrivate(HOLPlayerState->GetHeroPrivate()));
				}
			}
			const UHOLHeroSubsystem* Heroes = NetMode != NM_Client ? World->GetSubsystem<UHOLHeroSubsystem>() : nullptr;
			for (int32 Slot = 0; Heroes && Slot < FHOLMatchConfig::MaxSlots; ++Slot)
			{
				const FHOLHeroSheets& True = Heroes->GetSheets(Slot);
				UE_LOG(LogHOLMatch, Display, TEXT("Hero true slot %d: state %s level %d hp %d %s"), Slot, *DescribeState(True.Public.State),
					True.Public.Level, True.Public.Health, *DescribePrivate(True.Private));
			}
		}));

	FAutoConsoleCommandWithWorldAndArgs CommandSetXP(TEXT("hol.Hero.SetXP"),
		TEXT("hol.Hero.SetXP <Slot> <XP>: the slot's hero XP, server or solo (development only)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UHOLHeroSubsystem* Heroes = World && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLHeroSubsystem>() : nullptr;
			const int32 Slot = Args.Num() >= 2 ? FCString::Atoi(*Args[0]) : INDEX_NONE;
			if (!Heroes || !Heroes->SetXP(Slot, Args.Num() >= 2 ? FCString::Atoi(*Args[1]) : 0))
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("hol.Hero.SetXP: on the server or in solo, <Slot 0 to %d> <XP>"), FHOLMatchConfig::MaxSlots - 1);
				return;
			}
			UE_LOG(LogHOLMatch, Display, TEXT("hol.Hero.SetXP: slot %d, xp %d"), Slot, Heroes->GetSheets(Slot).Private.XP);
		}));

	FAutoConsoleCommandWithWorldAndArgs CommandKill(TEXT("hol.Hero.Kill"),
		TEXT("hol.Hero.Kill [Slot=0]: the slot's hero at 0 health, dying as killed in combat, server or solo (development only)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			const int32 Slot = Args.IsEmpty() ? 0 : FCString::Atoi(*Args[0]);
			UHOLUnitSubsystem* Units = World && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
			if (!Units || Slot < 0 || Slot >= FHOLMatchConfig::MaxSlots)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("hol.Hero.Kill: on the server or in solo, a slot 0 to %d"), FHOLMatchConfig::MaxSlots - 1);
				return;
			}
			const int32 Killed = Units->KillUnitsOfOwner(static_cast<uint8>(Slot), FHOLStartRules::HeroTypeName);
			UE_LOG(LogHOLMatch, Display, TEXT("hol.Hero.Kill: slot %d, %d hero(es) at 0 health"), Slot, Killed);
		}));
}

bool UHOLHeroSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLHeroSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Sheets.Init(FHOLHeroSheets(), FHOLMatchConfig::MaxSlots);
}

void UHOLHeroSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	BindSteps();
}

void UHOLHeroSubsystem::Deinitialize()
{
	UnbindSteps();
	Super::Deinitialize();
}

bool UHOLHeroSubsystem::BindSteps()
{
	UWorld* World = GetWorld();
	// Where the simulation runs only (solo, server): a client gets the copies.
	UHOLSimulationSubsystem* Simulation = World && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	if (!Simulation)
	{
		return false;
	}
	UnbindSteps();
	StepEndedHandle = Simulation->GetOnStepEnded().AddUObject(this, &UHOLHeroSubsystem::OnStepEnded);
	return true;
}

void UHOLHeroSubsystem::UnbindSteps()
{
	UHOLSimulationSubsystem* Simulation = GetWorld() ? GetWorld()->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	if (Simulation && StepEndedHandle.IsValid())
	{
		Simulation->GetOnStepEnded().Remove(StepEndedHandle);
	}
	StepEndedHandle.Reset();
}

void UHOLHeroSubsystem::OnStepEnded(const int64 StepIndex)
{
	UpdateSheets();
}

const FHOLHeroSheets& UHOLHeroSubsystem::GetSheets(const int32 Slot) const
{
	static const FHOLHeroSheets NoHero;
	return Sheets.IsValidIndex(Slot) ? Sheets[Slot] : NoHero;
}

void UHOLHeroSubsystem::UpdateSheets()
{
	UWorld* World = GetWorld();
	const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (!Units || !EntitySubsystem || World->GetNetMode() == NM_Client)
	{
		return;
	}
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	for (int32 Slot = 0; Slot < Sheets.Num(); ++Slot)
	{
		FHOLHeroFacts Facts;
		const uint32 HeroId = Units->FindHeroUnit(Slot);
		const FMassEntityHandle Entity = HeroId != 0 ? Units->FindEntity(HeroId) : FMassEntityHandle();
		const FHOLUnitStateFragment* State = Entity.IsSet() && EntityManager.IsEntityValid(Entity)
			? EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
		if (State)
		{
			Facts.bPresent = true;
			Facts.Health = State->CurrentHealth;
			const FHOLCombatFragment* Combat = EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entity);
			Facts.DeathStep = Combat ? Combat->DeathStep : -1;
		}
		Sheets[Slot] = HOLHeroSheet::Make(Facts, Sheets[Slot]);
		WriteCopies(Slot);
	}
}

bool UHOLHeroSubsystem::SetXP(const int32 Slot, const int32 XP)
{
	if (!Sheets.IsValidIndex(Slot) || !GetWorld() || GetWorld()->GetNetMode() == NM_Client)
	{
		return false;
	}
	Sheets[Slot].Private.XP = FMath::Max(XP, 0);
	WriteCopies(Slot);
	return true;
}

void UHOLHeroSubsystem::WriteCopies(const int32 Slot) const
{
	UWorld* World = GetWorld();
	if (AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr)
	{
		GameState->SetHeroSheet(Slot, Sheets[Slot].Public);
	}
	// An AI slot has no player state: its private part stays here.
	if (AHOLPlayerState* PlayerState = World ? FindPlayerState(*World, Slot) : nullptr)
	{
		PlayerState->SetHeroPrivate(Sheets[Slot].Private);
	}
}

AHOLPlayerState* UHOLHeroSubsystem::FindPlayerState(const UWorld& World, const int32 Slot)
{
	const AGameStateBase* GameState = World.GetGameState();
	if (!GameState || Slot == INDEX_NONE)
	{
		return nullptr;
	}
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState);
		if (HOLPlayerState && HOLPlayerState->GetPlayerIndex() == Slot)
		{
			return HOLPlayerState;
		}
	}
	return nullptr;
}
