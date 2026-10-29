// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLHeroSubsystem.h"

#include "Buildings/HOLBuilding.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyGameModeBase.h"
#include "HeroesOfLibertyLog.h"
#include "MassEntitySubsystem.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"
#include "Match/HOLStartRules.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLHeroLibrary.h"
#include "Simulation/HOLHeroReturnSubsystem.h"
#include "Simulation/HOLHeroXPRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

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
					// J8a.4 loop 5: the gold as this machine has it (a paid return seen taken on its owner's client).
					UE_LOG(LogHOLMatch, Display, TEXT("Hero gold slot %d %s: %d"), HOLPlayerState->GetPlayerIndex(),
						PlayerState == Local ? TEXT("local") : TEXT("other"), HOLPlayerState->GetResources().Gold);
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

	FAutoConsoleCommandWithWorldAndArgs CommandReturnProbe(TEXT("hol.Hero.ReturnProbe"),
		TEXT("hol.Hero.ReturnProbe [Slot=0]: asks this machine's return decision to buy the slot's return and to decide at the "
			 "next step, and says what it answered: refused on a client, which decides nothing (J8a.4 loop 5; development only)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			const int32 Slot = Args.IsEmpty() ? 0 : FCString::Atoi(*Args[0]);
			UHOLHeroReturnSubsystem* Decision = UHOLHeroReturnSubsystem::Find(World);
			if (!World || !Decision)
			{
				UE_LOG(LogHOLMatch, Display, TEXT("Hero return probe: net mode %d, no decision"), World ? static_cast<int32>(World->GetNetMode()) : -1);
				return;
			}
			const int32 Step = UHOLHeroLibrary::GetServerStep(World) + 1;
			const bool bBought = Decision->BuyNow(Slot, Step);
			const bool bDecided = Decision->DecideNow(Step);
			UE_LOG(LogHOLMatch, Display, TEXT("Hero return probe: net mode %d, buy %s, decide %s, decisions %d"), static_cast<int32>(World->GetNetMode()),
				bBought ? TEXT("done") : TEXT("refused"), bDecided ? TEXT("done") : TEXT("refused"), Decision->GetNumDecisions());
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
	// J8a.4: the script's return rule, the sheets of this step written.
	OnHeroStep.Broadcast(static_cast<int32>(StepIndex));
	UHOLHeroReturnSubsystem* Return = UHOLHeroReturnSubsystem::Find(GetWorld());
	// J8a.4 loop 4: the players' paid returns (ServerReturnHero) taken from the queue at this step, before the returns by
	// themselves, at the same step as the decision (StepIndex, the step after this one's increment: GetServerStep after it).
	// The script prices and decides each (no script: dropped, nothing paid).
	if (UHOLCommandSubsystem* Commands = GetWorld() ? GetWorld()->GetSubsystem<UHOLCommandSubsystem>() : nullptr)
	{
		TArray<int32> Orders;
		Commands->ConsumeHeroReturnOrders(Orders);
		for (const int32 Slot : Orders)
		{
			const bool bBought = Return && Return->BuyNow(Slot, static_cast<int32>(StepIndex));
			UE_LOG(LogHOLMatch, Display, TEXT("Hero return order: slot %d at step %d %s"), Slot, static_cast<int32>(StepIndex),
				bBought ? TEXT("bought") : TEXT("refused"));
		}
	}
	// J8a.4 loop 3: the return decision (script), called here so it always reads the sheets this step wrote.
	if (Return)
	{
		Return->DecideNow(static_cast<int32>(StepIndex));
	}
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
	UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (!Units || !EntitySubsystem || World->GetNetMode() == NM_Client)
	{
		return;
	}
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	for (int32 Slot = 0; Slot < Sheets.Num(); ++Slot)
	{
		FHOLHeroFacts Facts;
		const uint32 HeroId = Units->FindHeroUnit(Slot);
		const FMassEntityHandle Entity = HeroId != 0 ? Units->FindEntity(HeroId) : FMassEntityHandle();
		FHOLUnitStateFragment* State = Entity.IsSet() && EntityManager.IsEntityValid(Entity)
			? EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
		if (State)
		{
			FHOLCombatFragment* Combat = EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entity);
			ApplyLevel(Slot, State, Combat, EntityManager.GetConstSharedFragmentDataPtr<FHOLUnitTypeConstants>(Entity));
			Facts.bPresent = true;
			Facts.Health = State->CurrentHealth;
			Facts.DeathStep = Combat ? Combat->DeathStep : -1;
		}
		Sheets[Slot] = HOLHeroSheet::Make(Facts, Sheets[Slot]);
		WriteCopies(Slot);
	}
	// J8a.4: the returns asked were born at this step's start (or failed): they may be asked again.
	PendingReturns = 0;
}

void UHOLHeroSubsystem::ApplyLevel(const int32 Slot, FHOLUnitStateFragment* State, FHOLCombatFragment* Combat, const FHOLUnitTypeConstants* Constants) const
{
	// J8b.2: the level of his XP on his body, alive only (a dying one keeps his; born again at level 1 and full health, he
	// rises here at once to full health at his level: the max HP gained added, J8b-Q3).
	const int32 Level = HOLHeroXP::GetLevel(Sheets[Slot].Private.XP);
	if (!State || !Combat || !Constants || State->CurrentHealth <= 0.f || Combat->DeathStep >= 0 || Combat->Level == Level)
	{
		return;
	}
	const int32 OldLevel = Combat->Level;
	State->CurrentHealth = HOLHeroXP::GetHealthAfterLevelUp(State->CurrentHealth, static_cast<float>(Constants->MaxHealth), OldLevel, Level);
	Combat->Level = static_cast<uint8>(Level);
	UE_LOG(LogHOLMatch, Display, TEXT("Hero level: slot %d level %d -> %d, hp %d / %d"), Slot, OldLevel, Level,
		HOLCombat::GetShownHealth(State->CurrentHealth), HOLCombat::GetMaxHealth(*Constants, Level));
}

void UHOLHeroSubsystem::AddXP(const int32 Slot, const int32 XP, const EHOLXPSource Source)
{
	if (!Sheets.IsValidIndex(Slot) || XP <= 0 || !GetWorld() || GetWorld()->GetNetMode() == NM_Client)
	{
		return;
	}
	static const TCHAR* const SourceNames[] = { TEXT("combat"), TEXT("command"), TEXT("development"), TEXT("exploration") };
	const int32 SourceIndex = static_cast<int32>(Source);
	Sheets[Slot].Private.XP += XP;
	UE_LOG(LogHOLMatch, Display, TEXT("Hero XP: slot %d +%d (%s), xp %d"), Slot, XP,
		SourceIndex < UE_ARRAY_COUNT(SourceNames) ? SourceNames[SourceIndex] : TEXT("?"), Sheets[Slot].Private.XP);
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

bool UHOLHeroSubsystem::RewriteCopies(const int32 Slot)
{
	if (!Sheets.IsValidIndex(Slot) || !GetWorld() || GetWorld()->GetNetMode() == NM_Client)
	{
		return false;
	}
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

bool UHOLHeroSubsystem::SetReturnReady(const int32 Slot)
{
	if (!Sheets.IsValidIndex(Slot) || !GetWorld() || GetWorld()->GetNetMode() == NM_Client)
	{
		return false;
	}
	FHOLHeroSheet& Public = Sheets[Slot].Public;
	if (Public.State != EHOLHeroState::Dead && Public.State != EHOLHeroState::ReadyToReturn)
	{
		return false;
	}
	if (Public.State == EHOLHeroState::Dead)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Hero return: slot %d ready at step %d"), Slot, GetWorld()->GetSubsystem<UHOLSimulationSubsystem>()
			? static_cast<int32>(GetWorld()->GetSubsystem<UHOLSimulationSubsystem>()->GetStepIndex()) : -1);
	}
	Public.State = EHOLHeroState::ReadyToReturn;
	WriteCopies(Slot);
	return true;
}

bool UHOLHeroSubsystem::RequestReturn(const int32 Slot, const int32 PlaceIndex)
{
	UWorld* World = GetWorld();
	if (!Sheets.IsValidIndex(Slot) || !World || World->GetNetMode() == NM_Client || (PendingReturns & (1 << Slot)))
	{
		return false;
	}
	const AHOLGameState* GameState = World->GetGameState<AHOLGameState>();
	const EHOLHeroState State = Sheets[Slot].Public.State;
	if ((GameState && GameState->IsSlotDefeated(Slot)) || (State != EHOLHeroState::Dead && State != EHOLHeroState::ReadyToReturn))
	{
		return false;
	}
	// The main forum standing; the other forums come with D384 (none before: any other index names no place).
	const AHOLBuilding* Forum = PlaceIndex == -1 ? AHOLGameState::FindPrimaryForum(*World, Slot) : nullptr;
	if (!Forum || Forum->IsForumDown() || !AHeroesOfLibertyGameModeBase::SpawnStartingHero(*World, Slot, *Forum))
	{
		return false;
	}
	// Born at the next step's start: no second return until UpdateSheets has seen that step.
	PendingReturns |= 1 << Slot;
	UE_LOG(LogHOLMatch, Display, TEXT("Hero return: slot %d by its main forum"), Slot);
	return true;
}

bool UHOLHeroSubsystem::BuyReturn(const int32 Slot, const int32 PlaceIndex, const int32 Price)
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client || Price < 0)
	{
		return false;
	}
	AHOLPlayerState* PlayerState = FindPlayerState(*World, Slot);
	if (!PlayerState || PlayerState->GetResources().Gold < Price)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Hero return: slot %d cannot pay %d gold (%s)"), Slot, Price,
			PlayerState ? *FString::Printf(TEXT("%d gold"), PlayerState->GetResources().Gold) : TEXT("no stock"));
		return false;
	}
	// The birth first: refused, nothing is taken (the gold is never lost).
	if (!RequestReturn(Slot, PlaceIndex))
	{
		return false;
	}
	FHOLResources Stock = PlayerState->GetResources();
	Stock.Gold -= Price;
	PlayerState->SetResources(Stock);
	UE_LOG(LogHOLMatch, Display, TEXT("Hero return: slot %d bought for %d gold"), Slot, Price);
	return true;
}
