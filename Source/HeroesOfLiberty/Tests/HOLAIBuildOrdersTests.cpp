// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIOrders.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLPlayerOrders.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HeroesOfLibertyGameModeBase.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLMatchSettings.h"
#include "Match/HOLPlayerState.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLSimulationSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// IA1.4 checks: an AI slot's building and production orders (UHOLAIOrderLibrary) go through the same queue, the same
// placement rules and the same payment as a human's (HOLPlayerOrders, UHOLConstructionSubsystem, UHOLProductionSubsystem).
namespace UE::HeroesOfLiberty::Tests::AIBuild
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** A match's server: slot 0 human with a centre at (-20000, 0), slot 1 AI (real PlayerState, starting stock) with its centre at (0, 0). */
	struct FWorld
	{
		UWorld* World = nullptr;
		AHOLGameState* State = nullptr;
		AHOLPlayerState* AIState = nullptr;
		AHOLBuilding* Centre = nullptr;
		AHOLBuilding* EnemyCentre = nullptr;

		FWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			State = World->SpawnActor<AHOLGameState>();
			Centre = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
			EnemyCentre = World->SpawnActor<AHOLBuilding>(FVector(-20000.0, 0.0, 0.0), FRotator::ZeroRotator);
			UHOLConstructionSubsystem* Construction = World->GetSubsystem<UHOLConstructionSubsystem>();
			UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>();
			UHOLSourceSubsystem* Sources = World->GetSubsystem<UHOLSourceSubsystem>();
			if (!State || !Centre || !EnemyCentre || !Construction || !Production || !Sources || !World->GetSubsystem<UHOLSimulationSubsystem>())
			{
				return;
			}
			FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
			for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
			{
				Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : (Index == 1 ? EHOLSlotState::AI : EHOLSlotState::Closed);
			}
			State->InitVictory(Config);
			AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates(*World, Config, GetDefault<UHOLMatchSettings>()->StartingResources);
			for (APlayerState* Each : State->PlayerArray)
			{
				AHOLPlayerState* HOLEach = Cast<AHOLPlayerState>(Each);
				if (HOLEach && HOLEach->GetPlayerIndex() == 1)
				{
					AIState = HOLEach;
				}
			}
			Centre->Setup(HOLBuildingTypeNames::MainCentre, 1, 1);
			EnemyCentre->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
			Sources->OnWorldBeginPlay(*World);
			// The two subsystems that apply the queue subscribe to the step as at the match start.
			Construction->OnWorldBeginPlay(*World);
			Production->OnWorldBeginPlay(*World);
		}

		~FWorld()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		bool IsReady() const { return AIState != nullptr; }

		/** One step start: the construction and the production apply their orders. */
		void RunStep() { World->GetSubsystem<UHOLSimulationSubsystem>()->GetOnStepStarted().Broadcast(1); }

		int32 CountBuildings(const FName Type, const int32 Owner) const
		{
			int32 Count = 0;
			for (TActorIterator<AHOLBuilding> It(World); It; ++It)
			{
				Count += (It->GetTypeName() == Type && It->GetOwnerPlayer() == Owner) ? 1 : 0;
			}
			return Count;
		}
	};
}

// The placement query: the centre's own spot is refused (on a building), a spot in rings around it is found and passes the
// player's own check; the farm block is a mill and 8 farms, free, none on another.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIBuildPlacementTest, "HeroesOfLiberty.AI.Build.Placement", UE::HeroesOfLiberty::Tests::AIBuild::Flags)
bool FHOLAIBuildPlacementTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIBuild;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	const UObject* Ctx = Test.World;
	TestEqual(TEXT("a House on the centre: refused as on a building"),
		UHOLAIOrderLibrary::CheckAIPlacement(Ctx, 1, HOLBuildingTypeNames::House, FVector::ZeroVector), static_cast<int32>(EHOLPlacementRefusal::Building));
	TestEqual(TEXT("a type nobody builds (the main centre): -1"), UHOLAIOrderLibrary::CheckAIPlacement(Ctx, 1, HOLBuildingTypeNames::MainCentre, FVector::ZeroVector), -1);

	FVector House;
	if (TestTrue(TEXT("a House spot is found around the centre"), UHOLAIOrderLibrary::FindAIPlacement(Ctx, 1, HOLBuildingTypeNames::House, FVector::ZeroVector, 6, House)))
	{
		TestEqual(TEXT("that spot passes the player's check"), UHOLAIOrderLibrary::CheckAIPlacement(Ctx, 1, HOLBuildingTypeNames::House, House), 0);
		TestTrue(TEXT("and is beside the centre, not on it"), FVector::Dist2D(House, FVector::ZeroVector) > 1.0);
	}
	TestFalse(TEXT("no ring (MaxRings -1): nothing found"), UHOLAIOrderLibrary::FindAIPlacement(Ctx, 1, HOLBuildingTypeNames::House, FVector::ZeroVector, -1, House));

	FVector Mill;
	TArray<FVector> Farms;
	if (TestTrue(TEXT("a farm block is found"), UHOLAIOrderLibrary::FindAIFarmBlock(Ctx, 1, FVector::ZeroVector, 8, Mill, Farms)) && TestEqual(TEXT("8 farms"), Farms.Num(), 8))
	{
		TestEqual(TEXT("the mill passes the check"), UHOLAIOrderLibrary::CheckAIPlacement(Ctx, 1, HOLBuildingTypeNames::Mill, Mill), 0);
		TArray<FBox2D> Boxes;
		const FHOLBuildingTypeRow* MillType = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Mill);
		const FHOLBuildingTypeRow* FarmType = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Farm);
		Boxes.Add(UHOLConstructionSubsystem::SnapFootprint(*MillType, Mill));
		for (const FVector& Farm : Farms)
		{
			TestEqual(TEXT("each farm passes the check"), UHOLAIOrderLibrary::CheckAIPlacement(Ctx, 1, HOLBuildingTypeNames::Farm, Farm), 0);
			Boxes.Add(UHOLConstructionSubsystem::SnapFootprint(*FarmType, Farm));
		}
		bool bOverlap = false;
		for (int32 A = 0; A < Boxes.Num(); ++A)
		{
			for (int32 B = A + 1; B < Boxes.Num(); ++B)
			{
				bOverlap |= HOLConstruction::Overlaps(Boxes[A], Boxes[B]);
			}
		}
		TestFalse(TEXT("none of the nine on another"), bOverlap);
	}
	return true;
}

// A foundation placed by the AI through the queue: refused on a building before it enters the queue; at the next step it
// is owned by slot 1 and paid the type's cost, exactly; without the stock it is not placed (the human's refusal).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIBuildPlaceTest, "HeroesOfLiberty.AI.Build.Place", UE::HeroesOfLiberty::Tests::AIBuild::Flags)
bool FHOLAIBuildPlaceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIBuild;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	const UObject* Ctx = Test.World;
	const FHOLBuildingTypeRow* HouseType = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::House);
	if (!TestNotNull(TEXT("the House row"), HouseType))
	{
		return false;
	}
	FVector Spot;
	if (!TestTrue(TEXT("a spot"), UHOLAIOrderLibrary::FindAIPlacement(Ctx, 1, HOLBuildingTypeNames::House, FVector::ZeroVector, 6, Spot)))
	{
		return false;
	}
	TestFalse(TEXT("on the centre: refused before the queue"), UHOLAIOrderLibrary::IssueAIPlaceOrder(Ctx, 1, HOLBuildingTypeNames::House, FVector::ZeroVector, {}));
	TestFalse(TEXT("no slot: refused"), UHOLAIOrderLibrary::IssueAIPlaceOrder(Ctx, INDEX_NONE, HOLBuildingTypeNames::House, Spot, {}));

	const FHOLResources Before = Test.AIState->GetResources();
	TestTrue(TEXT("on the free spot: enters the queue"), UHOLAIOrderLibrary::IssueAIPlaceOrder(Ctx, 1, HOLBuildingTypeNames::House, Spot, {}));
	TestEqual(TEXT("nothing before the step"), Test.CountBuildings(HOLBuildingTypeNames::House, 1), 0);
	Test.RunStep();
	TestEqual(TEXT("next step: a House foundation of slot 1"), Test.CountBuildings(HOLBuildingTypeNames::House, 1), 1);
	FHOLResources Expected = Before;
	TestTrue(TEXT("the stock covered it"), HOLConstruction::TryPay(Expected, HouseType->Cost));
	TestEqual(TEXT("paid exactly the cost"), Test.AIState->GetResources().ToString(), Expected.ToString());

	// No stock: the order enters the queue (the AI cannot know better than the server), the foundation is not placed.
	FVector Other;
	TestTrue(TEXT("a second spot"), UHOLAIOrderLibrary::FindAIPlacement(Ctx, 1, HOLBuildingTypeNames::House, FVector::ZeroVector, 6, Other));
	TestTrue(TEXT("the first is taken: the second differs"), FVector::Dist2D(Spot, Other) > 1.0);
	Test.AIState->SetResources(FHOLResources());
	UHOLAIOrderLibrary::IssueAIPlaceOrder(Ctx, 1, HOLBuildingTypeNames::House, Other, {});
	Test.RunStep();
	TestEqual(TEXT("without the stock: still one House"), Test.CountBuildings(HOLBuildingTypeNames::House, 1), 1);
	return true;
}

// Production and rally of a finished building: queued (paid) and set at the next step; slot 1 cannot order slot 0's centre.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIBuildProductionTest, "HeroesOfLiberty.AI.Build.Production", UE::HeroesOfLiberty::Tests::AIBuild::Flags)
bool FHOLAIBuildProductionTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIBuild;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	const UObject* Ctx = Test.World;
	const FName Peasant(TEXT("Peasant"));
	const FHOLResources Before = Test.AIState->GetResources();
	TestFalse(TEXT("slot 0's centre: not slot 1's building, refused"), UHOLAIOrderLibrary::IssueAIProduceOrder(Ctx, 1, FVector(-20000.0, 0.0, 0.0), Peasant));
	TestFalse(TEXT("nothing at that point: refused"), UHOLAIOrderLibrary::IssueAIProduceOrder(Ctx, 1, FVector(9000.0, 9000.0, 0.0), Peasant));
	TestTrue(TEXT("a Peasant at its centre: enters the queue"), UHOLAIOrderLibrary::IssueAIProduceOrder(Ctx, 1, FVector::ZeroVector, Peasant));
	TestEqual(TEXT("nothing queued before the step"), Test.Centre->GetProductionQueue().Num(), 0);
	Test.RunStep();
	TestEqual(TEXT("next step: one Peasant in the centre's queue"), Test.Centre->GetProductionQueue().Num(), 1);
	TestTrue(TEXT("paid: the food went down"), Test.AIState->GetResources().Food < Before.Food);

	TestFalse(TEXT("no building there: no rally"), UHOLAIOrderLibrary::IssueAISetRallyOrder(Ctx, 1, FVector(9000.0, 9000.0, 0.0), FVector(500.0, 500.0, 0.0)));
	TestTrue(TEXT("the centre's rally point enters the queue"), UHOLAIOrderLibrary::IssueAISetRallyOrder(Ctx, 1, FVector::ZeroVector, FVector(3000.0, 1000.0, 0.0)));
	Test.RunStep();
	TestTrue(TEXT("next step: the rally point is set"), Test.Centre->HasRallyPoint());
	TestTrue(TEXT("where it was asked"), FVector::Dist2D(Test.Centre->GetRallyPoint(), FVector(3000.0, 1000.0, 0.0)) < 1.0);
	return true;
}

#endif
