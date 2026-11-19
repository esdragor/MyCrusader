// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tests/HOLTestWorldLibrary.h"

#include "Buildings/HOLBuilding.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

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
