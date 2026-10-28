// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLHeroLibrary.h"

#include "Buildings/HOLBuilding.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"
#include "Net/HOLUnitReceptionSubsystem.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"

namespace
{
	/**
	 * J8a.4 loop 5: the hero's return times (the delay and the price's drop) at this percent of the rules', for the
	 * network test (quick_check's net hero phase). 100, the default: the real rules. The price keeps its formula.
	 */
	TAutoConsoleVariable<int32> CVarHeroReturnTimePercent(
		TEXT("hol.Hero.ReturnTimePercent"),
		100,
		TEXT("Tests (J8a.4): the hero's return delay and price drop at this percent of the rules' (100: the rules; 1 at least). "
			 "Set it on the server and the clients alike."),
		ECVF_Cheat);

	UWorld* GetWorldOf(const UObject* WorldContextObject)
	{
		return GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	}

	/** The hero subsystem where the true sheets live (server, solo); null on a client. */
	UHOLHeroSubsystem* GetAuthorityHeroes(const UWorld* World)
	{
		return World && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLHeroSubsystem>() : nullptr;
	}

	/** The slot's public sheet: the true one on the server, the game state's copy on a client; the default otherwise. */
	FHOLHeroSheet GetPublicSheet(const UWorld* World, const int32 Slot)
	{
		if (const UHOLHeroSubsystem* Heroes = GetAuthorityHeroes(World))
		{
			return Heroes->GetSheets(Slot).Public;
		}
		const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
		return GameState ? GameState->GetHeroSheet(Slot) : FHOLHeroSheet();
	}
}

EHOLHeroState UHOLHeroLibrary::GetHeroState(const UObject* WorldContextObject, const int32 Slot)
{
	return GetPublicSheet(GetWorldOf(WorldContextObject), Slot).State;
}

int32 UHOLHeroLibrary::GetHeroLevel(const UObject* WorldContextObject, const int32 Slot)
{
	return GetPublicSheet(GetWorldOf(WorldContextObject), Slot).Level;
}

int32 UHOLHeroLibrary::GetHeroDeathStep(const UObject* WorldContextObject, const int32 Slot)
{
	const UWorld* World = GetWorldOf(WorldContextObject);
	if (const UHOLHeroSubsystem* Heroes = GetAuthorityHeroes(World))
	{
		return Heroes->GetSheets(Slot).Private.DeathStep;
	}
	// A client knows its own private sheet only.
	const APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	const AHOLPlayerState* Local = Controller ? Controller->GetPlayerState<AHOLPlayerState>() : nullptr;
	return Local && Local->GetPlayerIndex() == Slot ? Local->GetHeroPrivate().DeathStep : -1;
}

int32 UHOLHeroLibrary::GetSimulationRateHz()
{
	return GetDefault<UHOLSimulationSettings>()->SimulationRateHz;
}

int32 UHOLHeroLibrary::GetReturnTimePercent()
{
	return FMath::Max(1, CVarHeroReturnTimePercent.GetValueOnAnyThread());
}

int32 UHOLHeroLibrary::GetServerStep(const UObject* WorldContextObject)
{
	const UWorld* World = GetWorldOf(WorldContextObject);
	if (!World)
	{
		return -1;
	}
	if (World->GetNetMode() == NM_Client)
	{
		const UHOLUnitReceptionSubsystem* Reception = World->GetSubsystem<UHOLUnitReceptionSubsystem>();
		return Reception && Reception->GetClock().IsValid()
			? FMath::FloorToInt32(Reception->GetClock().GetStep(FPlatformTime::Seconds())) : -1;
	}
	const UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	return Simulation ? static_cast<int32>(Simulation->GetStepIndex()) : -1;
}

bool UHOLHeroLibrary::IsMainForumStanding(const UObject* WorldContextObject, const int32 Slot)
{
	const UWorld* World = GetWorldOf(WorldContextObject);
	const AHOLBuilding* Forum = World ? AHOLGameState::FindPrimaryForum(*World, Slot) : nullptr;
	return Forum && !Forum->IsForumDown();
}

FIntPoint UHOLHeroLibrary::GetMainForumPosition(const UObject* WorldContextObject, const int32 Slot)
{
	const UWorld* World = GetWorldOf(WorldContextObject);
	const AHOLBuilding* Forum = World ? AHOLGameState::FindPrimaryForum(*World, Slot) : nullptr;
	if (!Forum)
	{
		return FIntPoint::ZeroValue;
	}
	const FVector Location = Forum->GetActorLocation();
	return FIntPoint(FMath::RoundToInt32(Location.X), FMath::RoundToInt32(Location.Y));
}

int32 UHOLHeroLibrary::GetNumSlots()
{
	return FHOLMatchConfig::MaxSlots;
}

bool UHOLHeroLibrary::IsSlotDefeated(const UObject* WorldContextObject, const int32 Slot)
{
	const UWorld* World = GetWorldOf(WorldContextObject);
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	return GameState && GameState->IsSlotDefeated(Slot);
}

TArray<FIntPoint> UHOLHeroLibrary::GetOtherForumPositions(const UObject* WorldContextObject, const int32 Slot)
{
	// No other forum can be built yet (D384 later).
	return {};
}

bool UHOLHeroLibrary::SetHeroReturnReady(const UObject* WorldContextObject, const int32 Slot)
{
	UHOLHeroSubsystem* Heroes = GetAuthorityHeroes(GetWorldOf(WorldContextObject));
	return Heroes && Heroes->SetReturnReady(Slot);
}

bool UHOLHeroLibrary::TryPayGold(const UObject* WorldContextObject, const int32 Slot, const int32 Amount)
{
	const UWorld* World = GetWorldOf(WorldContextObject);
	if (!World || World->GetNetMode() == NM_Client || Amount < 0 || Slot < 0 || Slot >= FHOLMatchConfig::MaxSlots)
	{
		return false;
	}
	AHOLPlayerState* PlayerState = UHOLHeroSubsystem::FindPlayerState(*World, Slot);
	if (!PlayerState || PlayerState->GetResources().Gold < Amount)
	{
		return false;
	}
	FHOLResources Stock = PlayerState->GetResources();
	Stock.Gold -= Amount;
	PlayerState->SetResources(Stock);
	return true;
}

bool UHOLHeroLibrary::RequestHeroReturn(const UObject* WorldContextObject, const int32 Slot, const int32 PlaceIndex)
{
	UHOLHeroSubsystem* Heroes = GetAuthorityHeroes(GetWorldOf(WorldContextObject));
	return Heroes && Heroes->RequestReturn(Slot, PlaceIndex);
}

bool UHOLHeroLibrary::BuyHeroReturn(const UObject* WorldContextObject, const int32 Slot, const int32 PlaceIndex, const int32 Price)
{
	UHOLHeroSubsystem* Heroes = GetAuthorityHeroes(GetWorldOf(WorldContextObject));
	return Heroes && Heroes->BuyReturn(Slot, PlaceIndex, Price);
}
