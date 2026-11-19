// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tests/HOLTestWorldLibrary.h"

#include "Buildings/HOLBuilding.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"
#include "UObject/Package.h"

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
#include "AI/HOLAIBrain.h"
#include "Buildings/HOLConstructionRules.h"
#include "Simulation/HOLGatherProcessor.h"
#include "AI/HOLAIOrders.h"
#include "AI/HOLAITechnologyLibrary.h"
#include "AI/HOLAITools.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLProductionCommand.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
#include "Components/StaticMeshComponent.h"
#include "HOLCollision.h"

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

AHOLPlayerState* UHOLTestWorldLibrary::SpawnPlayerStateNoSlot(UHOLTestWorldHandle* TestWorld, const FString& Name)
{
#if WITH_DEV_AUTOMATION_TESTS
	UWorld* World = TestWorld ? TestWorld->GetTestWorld() : nullptr;
	AHOLPlayerState* Player = World ? World->SpawnActor<AHOLPlayerState>() : nullptr;
	if (Player)
	{
		Player->SetPlayerName(Name);
	}
	return Player;
#else
	return nullptr;
#endif
}

AHOLPlayerState* UHOLTestWorldLibrary::FindLiveHolder(UHOLTestWorldHandle* TestWorld, const int32 Slot, const AHOLPlayerState* Except)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (const UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		return AHOLGameState::FindLiveHolder(*World, Slot, Except);
	}
#endif
	return nullptr;
}

bool UHOLTestWorldLibrary::HasSlotStock(UHOLTestWorldHandle* TestWorld, const int32 Slot)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (const UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		FHOLResources Stock;
		return AHOLGameState::GetSlotStock(*World, Slot, Stock);
	}
#endif
	return false;
}

FHOLResources UHOLTestWorldLibrary::GetSlotStock(UHOLTestWorldHandle* TestWorld, const int32 Slot)
{
	FHOLResources Stock;
#if WITH_DEV_AUTOMATION_TESTS
	if (const UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		if (!AHOLGameState::GetSlotStock(*World, Slot, Stock))
		{
			Stock = FHOLResources();
		}
	}
#endif
	return Stock;
}

UHOLAIBrainSubsystem* UHOLTestWorldLibrary::GetAIBrain(UHOLTestWorldHandle* TestWorld)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (const UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		return UHOLAIBrainSubsystem::Find(World);
	}
#endif
	return nullptr;
}

int32 UHOLTestWorldLibrary::GetProductionQueueCount(const AHOLBuilding* Building)
{
#if WITH_DEV_AUTOMATION_TESTS
	return Building ? Building->GetProductionQueue().Num() : 0;
#else
	return 0;
#endif
}

FName UHOLTestWorldLibrary::GetProductionQueueName(const AHOLBuilding* Building, const int32 Index)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (Building && Building->GetProductionQueue().IsValidIndex(Index))
	{
		return Building->GetProductionQueue()[Index].TypeName;
	}
#endif
	return NAME_None;
}

bool UHOLTestWorldLibrary::IsProductionQueueItemStarted(const AHOLBuilding* Building, const int32 Index)
{
#if WITH_DEV_AUTOMATION_TESTS
	return Building && Building->GetProductionQueue().IsValidIndex(Index) && Building->GetProductionQueue()[Index].IsStarted();
#else
	return false;
#endif
}

int32 UHOLTestWorldLibrary::GetBuildingConstructionState(const AHOLBuilding* Building)
{
#if WITH_DEV_AUTOMATION_TESTS
	return Building ? static_cast<int32>(Building->GetConstructionState()) : 0;
#else
	return 0;
#endif
}

int64 UHOLTestWorldLibrary::GetBuildingConstructionWork(const AHOLBuilding* Building)
{
#if WITH_DEV_AUTOMATION_TESTS
	return Building ? Building->GetConstructionWork() : 0;
#else
	return 0;
#endif
}

int64 UHOLTestWorldLibrary::GetBuildingTotalWork(const AHOLBuilding* Building)
{
#if WITH_DEV_AUTOMATION_TESTS
	return Building ? Building->GetTotalWork() : 0;
#else
	return 0;
#endif
}

FHOLResources UHOLTestWorldLibrary::GetBuildingCancelRefund(const AHOLBuilding* Building)
{
#if WITH_DEV_AUTOMATION_TESTS
	const FHOLBuildingTypeRow* Type = Building ? Building->GetType() : nullptr;
	if (Type)
	{
		return HOLConstruction::GetCancelRefund(Type->Cost, Building->GetConstructionState(), Building->GetConstructionWork(), Building->GetTotalWork());
	}
#endif
	return FHOLResources();
}

int32 UHOLTestWorldLibrary::DepositLoad(UHOLTestWorldHandle* TestWorld, const int32 Owner, const int32 Resource, const int32 Amount)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		FHOLGatherLoad Load;
		Load.Resource = static_cast<EHOLResource>(Resource);
		Load.Amount = Amount;
		FHOLResources Stock;
		return UHOLGatherProcessor::DepositLoad(*World, Owner, Load, Stock);
	}
#endif
	return 0;
}

int32 UHOLTestWorldLibrary::DepositLoadWithRemaining(UHOLTestWorldHandle* TestWorld, const int32 Owner, const int32 Resource, const int32 Amount, int32& OutRemaining)
{
	OutRemaining = Amount;
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		FHOLGatherLoad Load;
		Load.Resource = static_cast<EHOLResource>(Resource);
		Load.Amount = Amount;
		FHOLResources Stock;
		const int32 Deposited = UHOLGatherProcessor::DepositLoad(*World, Owner, Load, Stock);
		OutRemaining = Load.Amount;
		return Deposited;
	}
#endif
	return 0;
}

// ---- Lot D853 d (CELEBORN-1119ba) ----

void UHOLTestWorldLibrary::DestroyBuildingWithRefund(AHOLBuilding* Building)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (Building)
	{
		UHOLConstructionSubsystem::DestroyBuilding(*Building);
	}
#endif
}

int32 UHOLTestWorldLibrary::GetSlotLevel(UHOLTestWorldHandle* TestWorld, const int32 Slot)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		return UHOLAIToolsLibrary::GetSlotLevel(World, Slot);
	}
#endif
	return -1;
}

bool UHOLTestWorldLibrary::IssueAIProduceOrder(UHOLTestWorldHandle* TestWorld, const int32 Slot, const FVector BuildingLocation, const FName TypeName)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		return UHOLAIOrderLibrary::IssueAIProduceOrder(World, Slot, BuildingLocation, TypeName);
	}
#endif
	return false;
}

TArray<FName> UHOLTestWorldLibrary::GetAITechnologiesDone(UHOLTestWorldHandle* TestWorld, const int32 Slot)
{
	TArray<FName> Done;
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		UHOLAITechnologyLibrary::ReadAITechnologiesDone(World, Slot, Done);
	}
#endif
	return Done;
}

int32 UHOLTestWorldLibrary::CountPlayerStatesOfSlot(UHOLTestWorldHandle* TestWorld, const int32 Slot)
{
	int32 Seats = 0;
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		for (TActorIterator<AHOLGameState> It(World); It; ++It)
		{
			for (const APlayerState* Each : It->PlayerArray)
			{
				const AHOLPlayerState* HOLEach = Cast<AHOLPlayerState>(Each);
				Seats += HOLEach && HOLEach->GetPlayerIndex() == Slot ? 1 : 0;
			}
		}
	}
#endif
	return Seats;
}

namespace HOLTestWorldLibNs
{
	/** What the game logs between BeginLogCapture and EndLogCapture. */
	class FHOLTestLogCapture : public FOutputDevice
	{
	public:
		TArray<FString> Lines;
		virtual void Serialize(const TCHAR* Text, ELogVerbosity::Type, const FName&) override { Lines.Add(Text); }
	};
	TUniquePtr<FHOLTestLogCapture> GHOLTestLogCapture;
}

void UHOLTestWorldLibrary::BeginLogCapture()
{
#if WITH_DEV_AUTOMATION_TESTS
	EndLogCapture();
	HOLTestWorldLibNs::GHOLTestLogCapture = MakeUnique<HOLTestWorldLibNs::FHOLTestLogCapture>();
	GLog->AddOutputDevice(HOLTestWorldLibNs::GHOLTestLogCapture.Get());
#endif
}

int32 UHOLTestWorldLibrary::CountCapturedLogLines(const FString& A, const FString& B)
{
	int32 Count = 0;
#if WITH_DEV_AUTOMATION_TESTS
	if (HOLTestWorldLibNs::GHOLTestLogCapture)
	{
		GLog->FlushThreadedLogs();
		for (const FString& Line : HOLTestWorldLibNs::GHOLTestLogCapture->Lines)
		{
			Count += Line.Contains(A) && Line.Contains(B) ? 1 : 0;
		}
	}
#endif
	return Count;
}

void UHOLTestWorldLibrary::EndLogCapture()
{
#if WITH_DEV_AUTOMATION_TESTS
	if (HOLTestWorldLibNs::GHOLTestLogCapture)
	{
		GLog->FlushThreadedLogs();
		GLog->RemoveOutputDevice(HOLTestWorldLibNs::GHOLTestLogCapture.Get());
		HOLTestWorldLibNs::GHOLTestLogCapture.Reset();
	}
#endif
}

int32 UHOLTestWorldLibrary::GetPlayerStateSlot(const AHOLPlayerState* Player)
{
	return Player ? Player->GetPlayerIndex() : -1;
}

// ---- Lot D853 e (CELEBORN-1119ca): the AI build orders ----

int32 UHOLTestWorldLibrary::CheckAIPlacement(UHOLTestWorldHandle* TestWorld, const int32 Slot, const FName TypeName, const FVector Point)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		return UHOLAIOrderLibrary::CheckAIPlacement(World, Slot, TypeName, Point);
	}
#endif
	return -2;
}

bool UHOLTestWorldLibrary::FindAIPlacement(UHOLTestWorldHandle* TestWorld, const int32 Slot, const FName TypeName, const FVector Centre, const int32 MaxRings, FVector& OutPoint)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		return UHOLAIOrderLibrary::FindAIPlacement(World, Slot, TypeName, Centre, MaxRings, OutPoint);
	}
#endif
	return false;
}

bool UHOLTestWorldLibrary::FindAIFarmBlock(UHOLTestWorldHandle* TestWorld, const int32 Slot, const FVector Centre, const int32 MaxRings, FVector& OutMill, TArray<FVector>& OutFarms)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		return UHOLAIOrderLibrary::FindAIFarmBlock(World, Slot, Centre, MaxRings, OutMill, OutFarms);
	}
#endif
	return false;
}

bool UHOLTestWorldLibrary::IssueAIPlaceOrder(UHOLTestWorldHandle* TestWorld, const int32 Slot, const FName TypeName, const FVector Point)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		return UHOLAIOrderLibrary::IssueAIPlaceOrder(World, Slot, TypeName, Point, {});
	}
#endif
	return false;
}

bool UHOLTestWorldLibrary::IssueAISetRallyOrder(UHOLTestWorldHandle* TestWorld, const int32 Slot, const FVector BuildingLocation, const FVector Point)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		return UHOLAIOrderLibrary::IssueAISetRallyOrder(World, Slot, BuildingLocation, Point);
	}
#endif
	return false;
}

int32 UHOLTestWorldLibrary::GetPlacementRefusalBuilding()
{
#if WITH_DEV_AUTOMATION_TESTS
	return static_cast<int32>(EHOLPlacementRefusal::Building);
#else
	return -2;
#endif
}

bool UHOLTestWorldLibrary::FarmBlockOverlaps(const FVector Mill, const TArray<FVector>& Farms)
{
	bool bOverlap = false;
#if WITH_DEV_AUTOMATION_TESTS
	const FHOLBuildingTypeRow* MillType = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Mill);
	const FHOLBuildingTypeRow* FarmType = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Farm);
	if (MillType && FarmType)
	{
		TArray<FBox2D> Boxes;
		Boxes.Add(UHOLConstructionSubsystem::SnapFootprint(*MillType, Mill));
		for (const FVector& Farm : Farms)
		{
			Boxes.Add(UHOLConstructionSubsystem::SnapFootprint(*FarmType, Farm));
		}
		for (int32 A = 0; A < Boxes.Num(); ++A)
		{
			for (int32 B = A + 1; B < Boxes.Num(); ++B)
			{
				bOverlap |= HOLConstruction::Overlaps(Boxes[A], Boxes[B]);
			}
		}
	}
#endif
	return bOverlap;
}

bool UHOLTestWorldLibrary::PayBuildingCost(const FName TypeName, const FHOLResources& Stock, FHOLResources& OutAfter)
{
	OutAfter = Stock;
#if WITH_DEV_AUTOMATION_TESTS
	if (const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(TypeName))
	{
		return HOLConstruction::TryPay(OutAfter, Type->Cost);
	}
#endif
	return false;
}

bool UHOLTestWorldLibrary::HasBuildingRallyPoint(const AHOLBuilding* Building)
{
	return Building && Building->HasRallyPoint();
}

FVector UHOLTestWorldLibrary::GetBuildingRallyPoint(const AHOLBuilding* Building)
{
	return Building ? Building->GetRallyPoint() : FVector::ZeroVector;
}

// ---- Lot D853 f (CELEBORN-1119cb): the unit tier production ----

bool UHOLTestWorldLibrary::SetSlotTierOverride(UHOLTestWorldHandle* TestWorld, const int32 Slot, const int32 Tier)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		for (TActorIterator<AHOLGameState> It(World); It; ++It)
		{
			It->SetTierOverride(Slot, Tier);
			return true;
		}
	}
#endif
	return false;
}

int32 UHOLTestWorldLibrary::GetSlotTier(UHOLTestWorldHandle* TestWorld, const int32 Slot)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		for (TActorIterator<AHOLGameState> It(World); It; ++It)
		{
			return It->GetTier(Slot);
		}
	}
#endif
	return -1;
}

bool UHOLTestWorldLibrary::IssueProductionOrder(UHOLTestWorldHandle* TestWorld, const int32 Slot, AHOLBuilding* Building, const FName TypeName)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		if (UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>())
		{
			FHOLProductionCommand Command;
			Command.PlayerIndex = Slot;
			Command.Building = Building;
			Command.TypeName = TypeName;
			Commands->IssueProductionOrder(Command);
			return true;
		}
	}
#endif
	return false;
}

int32 UHOLTestWorldLibrary::GetUnitCostTotal(UHOLTestWorldHandle* TestWorld, const FName TypeName)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		if (UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>())
		{
			if (const FHOLUnitTypeRow* Row = Units->FindTypeRow(TypeName))
			{
				return Row->Cost.Food + Row->Cost.Wood + Row->Cost.Stone + Row->Cost.Gold;
			}
		}
	}
#endif
	return -1;
}

// ---- Lot D853 g (CELEBORN-1119cc): the building tests without components ----

void UHOLTestWorldLibrary::SetBuildingConstructionState(AHOLBuilding* Building, const EHOLConstructionState State)
{
	if (Building)
	{
		Building->SetConstructionState(State);
	}
}

void UHOLTestWorldLibrary::SetBuildingConstructionWork(AHOLBuilding* Building, const int64 Work)
{
	if (Building)
	{
		Building->SetConstructionWork(Work);
	}
}

void UHOLTestWorldLibrary::ApplyBuildingAppearance(AHOLBuilding* Building)
{
	if (Building)
	{
		Building->ApplyPendingAppearance();
	}
}

int32 UHOLTestWorldLibrary::GetBuildingHealth(const AHOLBuilding* Building)
{
	return Building ? Building->GetHealth() : 0;
}

bool UHOLTestWorldLibrary::IsBuildingHurt(const AHOLBuilding* Building)
{
	return Building && Building->IsHurt();
}

bool UHOLTestWorldLibrary::IsBuildingFoundation(const AHOLBuilding* Building)
{
	return Building && Building->IsFoundation();
}

bool UHOLTestWorldLibrary::IsBuildingFinished(const AHOLBuilding* Building)
{
	return Building && Building->IsFinished();
}

bool UHOLTestWorldLibrary::HasBuildingWorkStarted(const AHOLBuilding* Building)
{
	return Building && Building->HasWorkStarted();
}

bool UHOLTestWorldLibrary::IsBuildingSeenHere(const AHOLBuilding* Building)
{
	return Building && Building->IsSeenHere();
}

bool UHOLTestWorldLibrary::IsBuildingAlive(const AHOLBuilding* Building)
{
	return IsValid(Building);
}

int32 UHOLTestWorldLibrary::CountBuildingMeshPieces(const AHOLBuilding* Building)
{
	int32 Count = 0;
#if WITH_DEV_AUTOMATION_TESTS
	if (Building)
	{
		TArray<UStaticMeshComponent*> Pieces;
		Building->GetComponents(Pieces);
		Count = Pieces.Num();
	}
#endif
	return Count;
}

int32 UHOLTestWorldLibrary::DamageBuildingByAttack(AHOLBuilding* Building, const int32 Damage, const bool bRanged)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (Building)
	{
		return UHOLConstructionSubsystem::DamageBuilding(*Building, Damage, bRanged ? EHOLAttackType::Ranged : EHOLAttackType::Melee);
	}
#endif
	return 0;
}

int32 UHOLTestWorldLibrary::GetBuildingTypeMaxHealth(const FName TypeName)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(TypeName))
	{
		return Type->MaxHealth;
	}
#endif
	return -1;
}

int32 UHOLTestWorldLibrary::GetBuildingTypeRangedArmor(const FName TypeName)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(TypeName))
	{
		return Type->RangedArmor;
	}
#endif
	return -1;
}

AActor* UHOLTestWorldLibrary::TraceSelectionRayDown(UHOLTestWorldHandle* TestWorld, const double X, const double Y)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (UWorld* World = HOLTestWorldLibNs::HOLTestWorldLibWorldOf(TestWorld))
	{
		FHitResult Hit;
		const FVector Above(X, Y, 5000.0);
		if (World->LineTraceSingleByChannel(Hit, Above, Above - FVector(0.0, 0.0, 10000.0), HOLCollision::Selection))
		{
			return Hit.GetActor();
		}
	}
#endif
	return nullptr;
}

void UHOLVictoryEventCounter::Bind(AHOLGameState* State)
{
	if (State)
	{
		State->OnSlotDefeated.AddUObject(this, &UHOLVictoryEventCounter::HandleDefeated);
		State->OnMatchOver.AddUObject(this, &UHOLVictoryEventCounter::HandleOver);
	}
}

bool UHOLTestWorldLibrary::DefeatGameStateSlot(AHOLGameState* State, const int32 Slot, const EHOLDefeatReason Reason)
{
	return State && State->DefeatSlot(Slot, Reason);
}

bool UHOLTestWorldLibrary::IsGameStateSlotDefeated(const AHOLGameState* State, const int32 Slot)
{
	return State && State->IsSlotDefeated(Slot);
}

EHOLDefeatReason UHOLTestWorldLibrary::GetGameStateDefeatReason(const AHOLGameState* State, const int32 Slot)
{
	return State ? State->GetDefeatReason(Slot) : EHOLDefeatReason::None;
}

bool UHOLTestWorldLibrary::IsGameStateMatchOver(const AHOLGameState* State)
{
	return State && State->IsMatchOver();
}

TArray<int32> UHOLTestWorldLibrary::GetGameStateWinners(const AHOLGameState* State)
{
	return State ? State->GetWinners() : TArray<int32>();
}

bool UHOLTestWorldLibrary::IsGameStateWinner(const AHOLGameState* State, const int32 Slot)
{
	return State && State->IsWinner(Slot);
}

UHOLVictoryEventCounter* UHOLTestWorldLibrary::CountVictoryEvents(AHOLGameState* State)
{
	if (!State)
	{
		return nullptr;
	}
	UHOLVictoryEventCounter* Counter = NewObject<UHOLVictoryEventCounter>(GetTransientPackage());
	Counter->Bind(State);
	return Counter;
}
