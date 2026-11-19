// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/Object.h"
#include "HOLTestWorldLibrary.generated.h"

class AHOLBuilding;
class AHOLGameState;
class UWorld;

/**
 * Handle of the world UHOLTestWorldLibrary::CreateTestWorld makes (D853, lot 1 of PLAN-TESTS-1119): the script keeps this object, never the UWorld
 * (a UWorld is not rooted: the GC could take it between two frames, and its end needs DestroyWorldContext + DestroyWorld). The handle owns the
 * world (UPROPERTY: kept alive by it) and is itself kept alive by the library until DestroyTestWorld. Named Handle because the script namespace
 * of UHOLTestWorldLibrary is HOLTestWorld (a class UHOLTestWorld would take the same script name).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLTestWorldHandle : public UObject
{
	GENERATED_BODY()

public:
	/** The world, nullptr once destroyed (or without WITH_DEV_AUTOMATION_TESTS). C++ side only. */
	UWorld* GetTestWorld() const { return World; }

	/** Ends the world as the C++ tests do (GEngine->DestroyWorldContext, then UWorld::DestroyWorld). Idempotent. */
	void DestroyOwnedWorld();

	virtual void BeginDestroy() override;

	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
};

/**
 * World harness of the tests for Angelscript (namespace HOLTestWorld there, D839: a binding exposed to the script, a test harness; D853: the C++
 * tests of a world without Mass are ported to script). Same world as the C++ tests (UWorld::CreateWorld(EWorldType::Game, false), its context,
 * InitializeActorsForPlay), so the world subsystems exist as in them. The bodies exist only where WITH_DEV_AUTOMATION_TESTS (the editor
 * target); elsewhere every function does nothing and returns nullptr / false / empty. Several worlds can live at once (the registry test needs
 * two); each is ended by its own DestroyTestWorld.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLTestWorldLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** A new game world with its context, initialized for play. nullptr, with a line in the log, if a step fails. End it with DestroyTestWorld. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static UHOLTestWorldHandle* CreateTestWorld();

	/** Ends the world of the handle (context, then world). Safe to call twice or with nullptr. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static void DestroyTestWorld(UHOLTestWorldHandle* TestWorld);

	/** Spawns an AHOLBuilding at Location and sets it up (AHOLBuilding::Setup(TypeName, Owner, Owner, State): the colour is the owner's). nullptr if it fails. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static AHOLBuilding* SpawnBuilding(UHOLTestWorldHandle* TestWorld, FName TypeName, int32 Owner, FVector Location, EHOLConstructionState State);

	/** Destroys the building (AActor::Destroy); nothing for nullptr. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static void DestroyBuilding(AHOLBuilding* Building);

	/**
	 * Spawns the world's AHOLGameState. With NumHumans + NumAI > 0, then InitVictory with FHOLMatchConfig::MakeDefault() whose first NumHumans slots are
	 * Human, the next NumAI AI, the rest Closed (HOLFarmBuildersTests.cpp); with 0 and 0 the bare actor, no InitVictory (as HOLTierCheatTests.cpp).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static AHOLGameState* SpawnGameState(UHOLTestWorldHandle* TestWorld, int32 NumHumans, int32 NumAI);

	/** The buildings of the world as FHOLBuildingIterator walks them (the registry), in its order. Empty for nullptr. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static TArray<AHOLBuilding*> GetBuildingsByRegistry(UHOLTestWorldHandle* TestWorld);

	/** The buildings of the world as TActorIterator<AHOLBuilding> walks them, in its order. Empty for nullptr. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static TArray<AHOLBuilding*> GetBuildingsByActors(UHOLTestWorldHandle* TestWorld);

	/** Whether the object's class has a property of this name with CPF_Net (a replicated property); false if none. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|TestWorld")
	static bool IsPropertyReplicated(UObject* Object, FName PropertyName);
};
