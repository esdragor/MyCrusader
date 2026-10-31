// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLRelaySubsystem.h"

#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLVictoryRules.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLHeroXPRules.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

namespace
{
	float HoldSeconds = HOLRelay::DefaultHoldSeconds;
	FAutoConsoleVariableRef CVarHoldSeconds(TEXT("hol.Relay.HoldSeconds"), HoldSeconds,
		TEXT("Seconds an adverse side must hold a relay's zone alone to take it (D388, provisional 60, like the ruin's)"));

	FAutoConsoleCommandWithWorldAndArgs CommandAdd(TEXT("hol.Relay.Add"),
		TEXT("hol.Relay.Add <X> <Y> [Radius=1200]: a neutral relay there, server or solo (development only)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UHOLRelaySubsystem* Relays = World ? World->GetSubsystem<UHOLRelaySubsystem>() : nullptr;
			const int32 Index = Relays && Args.Num() >= 2
				? Relays->AddRelay(FVector2D(FCString::Atod(*Args[0]), FCString::Atod(*Args[1])), Args.Num() >= 3 ? static_cast<float>(FCString::Atod(*Args[2])) : 1200.f)
				: INDEX_NONE;
			if (Index != INDEX_NONE)
			{
				UE_LOG(LogHOLMatch, Display, TEXT("hol.Relay.Add: relay %d"), Index);
			}
			else
			{
				UE_LOG(LogHOLMatch, Display, TEXT("hol.Relay.Add: refused (<X> <Y>, server or solo)"));
			}
		}));

	FAutoConsoleCommandWithWorldAndArgs CommandClear(TEXT("hol.Relay.Clear"), TEXT("hol.Relay.Clear: no relay left (development only)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (UHOLRelaySubsystem* Relays = World ? World->GetSubsystem<UHOLRelaySubsystem>() : nullptr)
			{
				Relays->ClearRelays();
			}
		}));

	FAutoConsoleCommandWithWorldAndArgs CommandReport(TEXT("hol.Relay.Report"), TEXT("hol.Relay.Report: each relay's owner, taker and count in the log"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			const UHOLRelaySubsystem* Relays = World ? World->GetSubsystem<UHOLRelaySubsystem>() : nullptr;
			if (!Relays)
			{
				return;
			}
			FString Line = FString::Printf(TEXT("Relay report: %d relay(s)"), Relays->GetRelays().Num());
			for (int32 Index = 0; Index < Relays->GetRelays().Num(); ++Index)
			{
				const FHOLRelayState& State = Relays->GetRelays()[Index].State;
				Line += FString::Printf(TEXT(" | #%d owner %d taker %d held %d"), Index, State.Owner, State.Taker, State.Held);
			}
			UE_LOG(LogHOLMatch, Display, TEXT("%s"), *Line);
		}));
}

bool UHOLRelaySubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLRelaySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	BindSteps();
}

void UHOLRelaySubsystem::Deinitialize()
{
	UnbindSteps();
	Super::Deinitialize();
}

bool UHOLRelaySubsystem::BindSteps()
{
	UWorld* World = GetWorld();
	UHOLSimulationSubsystem* Simulation = World && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	if (!Simulation)
	{
		return false;
	}
	UnbindSteps();
	StepEndedHandle = Simulation->GetOnStepEnded().AddUObject(this, &UHOLRelaySubsystem::OnStepEnded);
	return true;
}

void UHOLRelaySubsystem::UnbindSteps()
{
	UHOLSimulationSubsystem* Simulation = GetWorld() ? GetWorld()->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	if (Simulation && StepEndedHandle.IsValid())
	{
		Simulation->GetOnStepEnded().Remove(StepEndedHandle);
	}
	StepEndedHandle.Reset();
}

int32 UHOLRelaySubsystem::AddRelay(const FVector2D& Location, const float Radius)
{
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client)
	{
		return INDEX_NONE;
	}
	FRelaySite& Site = Relays.AddDefaulted_GetRef();
	Site.Location = Location;
	Site.Radius = FMath::Max(1.f, Radius);
	return Relays.Num() - 1;
}

void UHOLRelaySubsystem::ClearRelays()
{
	Relays.Reset();
	FMemory::Memzero(Carries);
}

int32 UHOLRelaySubsystem::CountOwned(const int32 Slot) const
{
	int32 Count = 0;
	for (const FRelaySite& Site : Relays)
	{
		Count += Site.State.Owner == Slot ? 1 : 0;
	}
	return Count;
}

void UHOLRelaySubsystem::OnStepEnded(const int64 StepIndex)
{
	UWorld* World = GetWorld();
	const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	if (Relays.IsEmpty() || !Units || !GameState)
	{
		return;
	}
	const FHOLMatchConfig& Config = GameState->GetLaunchConfig();
	const int32 Slots = FHOLMatchConfig::MaxSlots;
	TArray<int32> TeamBySlot;
	for (int32 Slot = 0; Slot < Slots; ++Slot)
	{
		const int32 Camp = HOLVictoryRules::GetCamp(Config, Slot);
		TeamBySlot.Add(Camp != INDEX_NONE ? Camp : 1000 + Slot);
	}

	// One walk of the units for all the relays; a neutral owner's units (out of the slots) never take one.
	TArray<TArray<int32>> Counts;
	Counts.SetNum(Relays.Num());
	for (TArray<int32>& Row : Counts)
	{
		Row.Init(0, Slots);
	}
	Units->ForEachLivingUnit([this, &Counts, Slots](const uint8 Owner, const FVector& Location)
	{
		if (Owner >= Slots)
		{
			return;
		}
		const FVector2D Point(Location);
		for (int32 Index = 0; Index < Relays.Num(); ++Index)
		{
			if (FVector2D::DistSquared(Point, Relays[Index].Location) <= FMath::Square(Relays[Index].Radius))
			{
				++Counts[Index][Owner];
			}
		}
	});

	const int32 Rate = GetDefault<UHOLSimulationSettings>()->SimulationRateHz;
	const int32 HoldSteps = FMath::Max(1, FMath::RoundToInt(HoldSeconds * Rate));
	for (int32 Index = 0; Index < Relays.Num(); ++Index)
	{
		FHOLRelayState& State = Relays[Index].State;
		const int32 OldOwner = State.Owner;
		State = HOLRelay::Step(State, Counts[Index], TeamBySlot, HoldSteps, HOLRelay::DefaultDecaySteps);
		if (State.Owner != OldOwner)
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Relay %d: slot %d takes it from slot %d at step %d"), Index, State.Owner, OldOwner, static_cast<int32>(StepIndex));
		}
	}

	// Territory XP by the minute to the owners' heroes (D388; no secondary centre in the game yet).
	UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>();
	for (int32 Slot = 0; Slot < Slots; ++Slot)
	{
		const int32 Owned = CountOwned(Slot);
		if (Owned == 0 && Carries[Slot] == 0)
		{
			continue;
		}
		const int32 XP = HOLRelay::TerritoryXP(Carries[Slot], Owned, 0, Rate);
		if (XP > 0 && Heroes)
		{
			Heroes->AddXP(Slot, XP, EHOLXPSource::Exploration);
		}
	}
}
