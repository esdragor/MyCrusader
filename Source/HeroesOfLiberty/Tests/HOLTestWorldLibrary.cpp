// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tests/HOLTestWorldLibrary.h"

#include "Buildings/HOLBuilding.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Buildings/HOLProductionRules.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyGameModeBase.h"
#include "Match/HOLMatchSettings.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLSimulationSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogHOLTestWorld, Log, All);

namespace HOLTestWorldLibNs
{
	// The handles still alive, kept by strong pointers until their DestroyTestWorld. Prefixed name: unity build.
	TArray<TStrongObjectPtr<UHOLTestWorldHandle>> HOLTestWorldLibLive;
}
#endif

void UHOLTestWorldHandle::DestroyOwnedWorld()
{
#if WITH_DEV_AUTOMATION_TESTS
	UWorld* Owned = World;
	World = nullptr;
	if (Owned && GEngine && !IsEngineExitRequested())
	{
		// As the end of the C++ tests does.
		GEngine->DestroyWorldContext(Owned);
		Owned->DestroyWorld(/*bInformEngineOfWorld*/ false);
	}
#endif
}

void UHOLTestWorldHandle::BeginDestroy()
{
	// The net: the library keeps every handle alive until DestroyTestWorld, so this only ends a world nobody ended.
	DestroyOwnedWorld();
	Super::BeginDestroy();
}

UHOLTestWorldHandle* UHOLTestWorldLibrary::CreateTestWorld()
{
#if WITH_DEV_AUTOMATION_TESTS
	if (!GEngine)
	{
		UE_LOG(LogHOLTestWorld, Error, TEXT("CreateTestWorld: no GEngine."));
		return nullptr;
	}
	// The motif of HOLBuildingRegistryTests.cpp (MakeWorld): a game world, its context, initialized for play.
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	if (!World)
	{
		UE_LOG(LogHOLTestWorld, Error, TEXT("CreateTestWorld: UWorld::CreateWorld returned nullptr."));
		return nullptr;
	}
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	UHOLTestWorldHandle* Handle = NewObject<UHOLTestWorldHandle>(GetTransientPackage());
	Handle->World = World;
	HOLTestWorldLibNs::HOLTestWorldLibLive.Emplace(Handle);
	return Handle;
#else
	return nullptr;
#endif
}

void UHOLTestWorldLibrary::DestroyTestWorld(UHOLTestWorldHandle* TestWorld)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (!TestWorld)
	{
		return;
	}
	TestWorld->DestroyOwnedWorld();
	HOLTestWorldLibNs::HOLTestWorldLibLive.RemoveAll([TestWorld](const TStrongObjectPtr<UHOLTestWorldHandle>& Held) { return Held.Get() == TestWorld; });
#endif
}

AHOLBuilding* UHOLTestWorldLibrary::SpawnBuilding(UHOLTestWorldHandle* TestWorld, const FName TypeName, const int32 Owner, const FVector Location, const EHOLConstructionState State)
{
#if WITH_DEV_AUTOMATION_TESTS
	UWorld* World = TestWorld ? TestWorld->GetTestWorld() : nullptr;
	if (!World)
	{
		return nullptr;
	}
	AHOLBuilding* Building = World->SpawnActor<AHOLBuilding>(Location, FRotator::ZeroRotator);
	if (Building)
	{
		Building->Setup(TypeName, static_cast<uint8>(Owner), static_cast<uint8>(Owner), State);
	}
	return Building;
#else
	return nullptr;
#endif
}

void UHOLTestWorldLibrary::DestroyBuilding(AHOLBuilding* Building)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (Building)
	{
		Building->Destroy();
	}
#endif
}

AHOLGameState* UHOLTestWorldLibrary::SpawnGameState(UHOLTestWorldHandle* TestWorld, const int32 NumHumans, const int32 NumAI)
{
#if WITH_DEV_AUTOMATION_TESTS
	UWorld* World = TestWorld ? TestWorld->GetTestWorld() : nullptr;
	if (!World)
	{
		return nullptr;
	}
	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	if (State && NumHumans + NumAI > 0)
	{
		// The configuration of HOLFarmBuildersTests.cpp: the default one, then the state of each slot.
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Index < NumHumans ? EHOLSlotState::Human : (Index < NumHumans + NumAI ? EHOLSlotState::AI : EHOLSlotState::Closed);
		}
		State->InitVictory(Config);
	}
	return State;
#else
	return nullptr;
#endif
}

AHOLPlayerState* UHOLTestWorldLibrary::SpawnPlayerState(UHOLTestWorldHandle* TestWorld, const int32 Slot, const int32 ColorIndex, const FString& Name)
{
#if WITH_DEV_AUTOMATION_TESTS
	UWorld* World = TestWorld ? TestWorld->GetTestWorld() : nullptr;
	AHOLPlayerState* Player = World ? World->SpawnActor<AHOLPlayerState>() : nullptr;
	if (Player)
	{
		Player->SetMatchSlot(Slot, ColorIndex);
		Player->SetPlayerName(Name);
	}
	return Player;
#else
	return nullptr;
#endif
}

TArray<AHOLBuilding*> UHOLTestWorldLibrary::GetBuildingsByRegistry(UHOLTestWorldHandle* TestWorld)
{
	TArray<AHOLBuilding*> Found;
#if WITH_DEV_AUTOMATION_TESTS
	if (const UWorld* World = TestWorld ? TestWorld->GetTestWorld() : nullptr)
	{
		for (FHOLBuildingIterator It(World); It; ++It)
		{
			Found.Add(*It);
		}
	}
#endif
	return Found;
}

TArray<AHOLBuilding*> UHOLTestWorldLibrary::GetBuildingsByActors(UHOLTestWorldHandle* TestWorld)
{
	TArray<AHOLBuilding*> Found;
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = TestWorld ? TestWorld->GetTestWorld() : nullptr)
	{
		for (TActorIterator<AHOLBuilding> It(World); It; ++It)
		{
			Found.Add(*It);
		}
	}
#endif
	return Found;
}

bool UHOLTestWorldLibrary::IsPropertyReplicated(UObject* Object, const FName PropertyName)
{
	if (!Object)
	{
		return false;
	}
	const FProperty* Property = Object->GetClass()->FindPropertyByName(PropertyName);
	return Property && Property->HasAnyPropertyFlags(CPF_Net);
}

// ---- Lots 3 and 4 (ELROND-1119u) ----

#if WITH_DEV_AUTOMATION_TESTS
namespace HOLTestWorldLibNs
{
	// Step index per world, counting on from 1 as the tests' StepCount does. Prefixed name: unity build.
	TMap<TWeakObjectPtr<UWorld>, int64> HOLTestWorldLibSteps;

	UWorld* HOLTestWorldLibWorldOf(const UHOLTestWorldHandle* TestWorld)
	{
		return TestWorld ? TestWorld->GetTestWorld() : nullptr;
	}

	AHOLGameState* HOLTestWorldLibGameStateOf(UWorld* World)
	{
		if (World)
		{
			for (TActorIterator<AHOLGameState> It(World); It; ++It)
			{
				return *It;
			}
		}
		return nullptr;
	}

	AHOLPlayerState* HOLTestWorldLibPlayerOfSlot(UWorld* World, const int32 Slot)
	{
		if (const AHOLGameState* State = HOLTestWorldLibGameStateOf(World))
		{
			for (APlayerState* Each : State->PlayerArray)
			{
				AHOLPlayerState* HOLEach = Cast<AHOLPlayerState>(Each);
				if (HOLEach && HOLEach->GetPlayerIndex() == Slot)
				{
					return HOLEach;
				}
			}
		}
		return nullptr;
	}

	int32 HOLTestWorldLibBroadcast(UWorld* World, const int32 NumSteps, const bool bWithEnd)
	{
		UHOLSimulationSubsystem* Simulation = World ? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
		if (!Simulation)
		{
			return 0;
		}
		int64& Index = HOLTestWorldLibSteps.FindOrAdd(World);
		for (int32 Each = 0; Each < NumSteps; ++Each)
		{
			Simulation->GetOnStepStarted().Broadcast(++Index);
			if (bWithEnd)
			{
				Simulation->GetOnStepEnded().Broadcast(Index);
			}
		}
		return FMath::Max(NumSteps, 0);
	}
}
#endif

UObject* UHOLTestWorldLibrary::GetWorldObject(UHOLTestWorldHandle* TestWorld)
{
#if WITH_DEV_AUTOMATION_TESTS
	return HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld);
#else
	return nullptr;
#endif
}

bool UHOLTestWorldLibrary::RunConsoleCommand(UHOLTestWorldHandle* TestWorld, const FString& Command)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		return IConsoleManager::Get().ProcessUserConsoleInput(*Command, *GLog, World);
	}
#endif
	return false;
}

UWorldSubsystem* UHOLTestWorldLibrary::GetWorldSubsystem(UHOLTestWorldHandle* TestWorld, TSubclassOf<UWorldSubsystem> SubsystemClass)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		if (SubsystemClass)
		{
			return World->GetSubsystemBase(SubsystemClass);
		}
	}
#endif
	return nullptr;
}

bool UHOLTestWorldLibrary::InitProductionSubsystem(UHOLTestWorldHandle* TestWorld)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		if (UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>())
		{
			Production->OnWorldBeginPlay(*World);
			return true;
		}
	}
#endif
	return false;
}

bool UHOLTestWorldLibrary::InitSourceSubsystem(UHOLTestWorldHandle* TestWorld)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		if (UHOLSourceSubsystem* Sources = World->GetSubsystem<UHOLSourceSubsystem>())
		{
			Sources->OnWorldBeginPlay(*World);
			return true;
		}
	}
#endif
	return false;
}

bool UHOLTestWorldLibrary::InitConstructionSubsystem(UHOLTestWorldHandle* TestWorld)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		if (UHOLConstructionSubsystem* Construction = World->GetSubsystem<UHOLConstructionSubsystem>())
		{
			Construction->OnWorldBeginPlay(*World);
			return true;
		}
	}
#endif
	return false;
}

FHOLResources UHOLTestWorldLibrary::GetProductionCostResources(UHOLTestWorldHandle* TestWorld, const FName TypeName, const int32 PlayerIndex)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		if (const UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>())
		{
			return Production->GetCost(TypeName, PlayerIndex).Resources;
		}
	}
#endif
	return FHOLResources();
}

int32 UHOLTestWorldLibrary::GetProductionCostSteps(UHOLTestWorldHandle* TestWorld, const FName TypeName, const int32 PlayerIndex)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		if (const UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>())
		{
			return Production->GetCost(TypeName, PlayerIndex).BuildSteps;
		}
	}
#endif
	return 0;
}

bool UHOLTestWorldLibrary::EnqueueProduction(UHOLTestWorldHandle* TestWorld, AHOLBuilding* Building, const FName Technology, const int32 Slot, AHOLPlayerState* Payer, const FHOLResources& FallbackStock, const int32 MaxQueue)
{
#if WITH_DEV_AUTOMATION_TESTS
	UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld);
	const UHOLProductionSubsystem* Production = World ? World->GetSubsystem<UHOLProductionSubsystem>() : nullptr;
	if (Production && Building)
	{
		const FHOLProductionCost Cost = Production->GetCost(Technology, Slot);
		FHOLResources Stock = Payer ? Payer->GetResources() : FallbackStock;
		const bool bQueued = HOLProduction::TryEnqueue(Building->GetMutableProductionQueue(), Technology, Cost, MaxQueue, Stock) == EHOLProductionRefusal::None;
		if (Payer)
		{
			Payer->SetResources(Stock);
		}
		return bQueued;
	}
#endif
	return false;
}

bool UHOLTestWorldLibrary::SetHeroXP(UHOLTestWorldHandle* TestWorld, const int32 Slot, const int32 XP)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		if (UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>())
		{
			return Heroes->SetXP(Slot, XP);
		}
	}
#endif
	return false;
}

int32 UHOLTestWorldLibrary::GetHeroSheetXP(UHOLTestWorldHandle* TestWorld, const int32 Slot)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		if (const UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>())
		{
			return Heroes->GetSheets(Slot).Private.XP;
		}
	}
#endif
	return 0;
}

bool UHOLTestWorldLibrary::HeroSheetEqualsPlayer(UHOLTestWorldHandle* TestWorld, const int32 Slot, AHOLPlayerState* Player)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		if (const UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>())
		{
			return Player && Heroes->GetSheets(Slot).Private == Player->GetHeroPrivate();
		}
	}
#endif
	return false;
}

AHOLPlayerState* UHOLTestWorldLibrary::FindHeldBot(UHOLTestWorldHandle* TestWorld, const int32 Slot)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (const UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		return AHOLGameState::FindHeldBot(*World, Slot);
	}
#endif
	return nullptr;
}

bool UHOLTestWorldLibrary::AddSlotStock(UHOLTestWorldHandle* TestWorld, const int32 Slot, const FHOLResources& Gain)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		FHOLResources Stock;
		return AHOLGameState::AddSlotStock(*World, Slot, Gain, Stock);
	}
#endif
	return false;
}

int32 UHOLTestWorldLibrary::SpawnAIPlayerStates(UHOLTestWorldHandle* TestWorld, const FString& Json)
{
#if WITH_DEV_AUTOMATION_TESTS
	FHOLMatchConfig Config;
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		if (FHOLMatchConfig::FromJson(Json, Config))
		{
			return AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates(*World, Config, GetDefault<UHOLMatchSettings>()->StartingResources);
		}
	}
#endif
	return 0;
}

int32 UHOLTestWorldLibrary::AdvanceSteps(UHOLTestWorldHandle* TestWorld, const int32 NumSteps)
{
#if WITH_DEV_AUTOMATION_TESTS
	return HOLTestWorldLibNs::HOLTestWorldLibBroadcast(HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld), NumSteps, false);
#else
	return 0;
#endif
}

int32 UHOLTestWorldLibrary::AdvanceStepsWithEnd(UHOLTestWorldHandle* TestWorld, const int32 NumSteps)
{
#if WITH_DEV_AUTOMATION_TESTS
	return HOLTestWorldLibNs::HOLTestWorldLibBroadcast(HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld), NumSteps, true);
#else
	return 0;
#endif
}

FHOLResources UHOLTestWorldLibrary::GetSlotResources(UHOLTestWorldHandle* TestWorld, const int32 Slot)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (const AHOLPlayerState* Player = HOLTestWorldLibNs::HOLTestWorldLibPlayerOfSlot(HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld), Slot))
	{
		return Player->GetResources();
	}
#endif
	return FHOLResources();
}

bool UHOLTestWorldLibrary::SetSlotResources(UHOLTestWorldHandle* TestWorld, const int32 Slot, const FHOLResources& Resources)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (AHOLPlayerState* Player = HOLTestWorldLibNs::HOLTestWorldLibPlayerOfSlot(HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld), Slot))
	{
		Player->SetResources(Resources);
		return true;
	}
#endif
	return false;
}

int32 UHOLTestWorldLibrary::CountBuildings(UHOLTestWorldHandle* TestWorld, const FName TypeName, const int32 Owner)
{
	int32 Count = 0;
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		for (TActorIterator<AHOLBuilding> It(World); It; ++It)
		{
			Count += (It->GetTypeName() == TypeName && It->GetOwnerPlayer() == Owner) ? 1 : 0;
		}
	}
#endif
	return Count;
}
