// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLConstructionCommand.h"
#include "Commands/HOLUnitOrderProcessor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLBuildProcessor.h"
#include "Simulation/HOLGatherProcessor.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// IA1.2 F6b on the real chain, as a match's server: a test world, the build orders through the command queue, then at
// each fixed step the order processor, the gathering and the builders' processor (the pipeline's order: the builders run
// after the gathering). No movement runs: the Peasants are born touching their sites; hol.Build.Instant ends a site at its
// first blow, the builders go on at the next step (UHOLBuildProcessor::Advance), as at the end of any site.
namespace UE::HeroesOfLiberty::Tests::FarmBuilders
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FName Peasant(TEXT("Peasant"));

	/** The test world: a game state (slot 0 human, slot 1 AI), the subsystems and the step's processors. */
	struct FBuildersWorld
	{
		UWorld* World = nullptr;
		UHOLUnitSubsystem* Units = nullptr;
		UHOLSourceSubsystem* Sources = nullptr;
		UHOLCommandSubsystem* Commands = nullptr;
		TSharedPtr<FMassEntityManager> EntityManager;
		UHOLUnitOrderProcessor* Orders = nullptr;
		UHOLGatherProcessor* Gather = nullptr;
		UHOLBuildProcessor* Builders = nullptr;

		FBuildersWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			AHOLGameState* State = World->SpawnActor<AHOLGameState>();
			Units = World->GetSubsystem<UHOLUnitSubsystem>();
			Sources = World->GetSubsystem<UHOLSourceSubsystem>();
			Commands = World->GetSubsystem<UHOLCommandSubsystem>();
			UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
			UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
			if (!State || !Units || !Sources || !Commands || !Simulation || !EntitySubsystem)
			{
				return;
			}
			FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
			for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
			{
				Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : (Index == 1 ? EHOLSlotState::AI : EHOLSlotState::Closed);
			}
			State->InitVictory(Config);
			// As the match starts: the registry takes sources (and the farms' fields, F6a) from now on.
			Sources->OnWorldBeginPlay(*World);
			EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
			Orders = NewObject<UHOLUnitOrderProcessor>(Simulation);
			Orders->CallInitialize(Simulation, EntityManager.ToSharedRef());
			Gather = NewObject<UHOLGatherProcessor>(Simulation);
			Gather->CallInitialize(Simulation, EntityManager.ToSharedRef());
			Builders = NewObject<UHOLBuildProcessor>(Simulation);
			Builders->CallInitialize(Simulation, EntityManager.ToSharedRef());
		}

		~FBuildersWorld()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		bool IsReady() const { return Orders && Gather && Builders; }

		/** One fixed step: the orders, the gathering, then the builders (UHOLBuildProcessor runs after HOLGatherProcessor). */
		void RunStep()
		{
			const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
			{
				FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Orders, ProcessingContext);
			}
			{
				FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Gather, ProcessingContext);
			}
			{
				FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Builders, ProcessingContext);
			}
		}

		/** A building of slot 0 at Location (a site when State is Foundation); null if none was spawned. */
		AHOLBuilding* PlaceBuilding(const FName TypeName, const FVector2D& Location, const EHOLConstructionState State = EHOLConstructionState::Finished)
		{
			AHOLBuilding* Building = World->SpawnActor<AHOLBuilding>(FVector(Location, 0.0), FRotator::ZeroRotator);
			if (Building)
			{
				Building->Setup(TypeName, 0, 0, State);
			}
			return Building;
		}

		/** A Peasant of slot 0 born at Location; its identifier, 0 if none was born. */
		uint32 PlacePeasant(const FVector2D& Location)
		{
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = Peasant;
			Request.Count = 1;
			Request.OwnerPlayer = 0;
			Request.Center = FVector(Location, 0.0);
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			TArray<uint32> After;
			Units->GetAllUnitIds(After);
			After.RemoveAll([&Before](const uint32 Id) { return Before.Contains(Id); });
			return After.Num() == 1 ? After[0] : 0;
		}

		/** One or two Peasants touching Site's body (70 cm off), on its west side, then its south side; their identifiers. */
		TArray<uint32> PlaceBuilders(const AHOLBuilding& Site, const int32 Count)
		{
			const FBox2D Body = Site.GetBodyBounds();
			const FVector2D Middle = Body.GetCenter();
			TArray<uint32> Ids;
			Ids.Add(PlacePeasant(FVector2D(Body.Min.X - 70.0, Middle.Y)));
			if (Count > 1)
			{
				Ids.Add(PlacePeasant(FVector2D(Middle.X, Body.Min.Y - 70.0)));
			}
			return Ids;
		}

		/** Slot 0's order to build Site. */
		void OrderBuild(const TArray<uint32>& UnitIds, AHOLBuilding& Site) const
		{
			FHOLBuildOrder Order;
			Order.UnitIds = UnitIds;
			Order.Building = &Site;
			Order.PlayerIndex = 0;
			Commands->IssueBuildOrder(Order);
		}

		const FHOLGatherFragment* GetGather(const uint32 UnitId) const
		{
			const FMassEntityHandle Entity = Units->FindEntity(UnitId);
			return EntityManager->IsEntityValid(Entity) ? EntityManager->GetFragmentDataPtr<FHOLGatherFragment>(Entity) : nullptr;
		}

		/** The unit's gathering task is SourceIndex (on its way there, at work there or carrying its food). */
		bool HasTask(const uint32 UnitId, const int32 SourceIndex) const
		{
			const FHOLGatherFragment* Fragment = GetGather(UnitId);
			return Fragment && Fragment->IsGathering() && Fragment->SourceIndex == SourceIndex;
		}

		int32 CountTasks(const TArray<uint32>& UnitIds, const int32 SourceIndex) const
		{
			int32 Count = 0;
			for (const uint32 Id : UnitIds)
			{
				Count += HasTask(Id, SourceIndex) ? 1 : 0;
			}
			return Count;
		}
	};
}

// (a) A farm finished by two Peasants: one gathers its field (one place), the other never with him: it goes to the free
// field of the farm touching it (IA1.2-Q4, F3's massif). (b) A mill finished beside the berries and a nearer free farm: its
// builders go to the berries (IA1.2-Q6: a mill never takes a farmer's place while there are berries). (c) A mill finished
// beside a free farm alone: one builder takes the field, never two at once. (d) A lumber camp finished beside a tree: its
// builder goes to the tree (J6b.1, unchanged).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLConstructionFarmBuildersTest, "HeroesOfLiberty.Construction.FarmBuilders", UE::HeroesOfLiberty::Tests::FarmBuilders::Flags)
bool FHOLConstructionFarmBuildersTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::FarmBuilders;
	using EState = EHOLConstructionState;
	FBuildersWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	UHOLSourceSubsystem& Registry = *Test.Sources;

	// Each group 100 m from the others (beyond hol.Build.CampGatherRange's 25 m and the next site's 20 m). Farms and mills
	// are 8 m wide: a building centred 8 m away touches.
	// (a) The farm site, a finished farm touching it on the east.
	AHOLBuilding* FarmSite = Test.PlaceBuilding(HOLBuildingTypeNames::Farm, FVector2D(0.0, 0.0), EState::Foundation);
	AHOLBuilding* NextFarm = Test.PlaceBuilding(HOLBuildingTypeNames::Farm, FVector2D(800.0, 0.0));
	// (b) The mill site, a finished farm touching it on the east (its field 3 m from the mill), the berries 10 m north.
	AHOLBuilding* MillSite = Test.PlaceBuilding(HOLBuildingTypeNames::Mill, FVector2D(10000.0, 0.0), EState::Foundation);
	AHOLBuilding* MillFarm = Test.PlaceBuilding(HOLBuildingTypeNames::Farm, FVector2D(10800.0, 0.0));
	const int32 Berries = Registry.AddSource(HOLSourceTypeNames::BerryBush, FVector(10000.0, 1500.0, 0.0));
	// (c) The mill site, a finished farm alone touching it on the east.
	AHOLBuilding* LoneMillSite = Test.PlaceBuilding(HOLBuildingTypeNames::Mill, FVector2D(20000.0, 0.0), EState::Foundation);
	AHOLBuilding* LoneFarm = Test.PlaceBuilding(HOLBuildingTypeNames::Farm, FVector2D(20800.0, 0.0));
	// (d) The lumber camp site, a tree 10 m north.
	AHOLBuilding* CampSite = Test.PlaceBuilding(HOLBuildingTypeNames::LumberCamp, FVector2D(30000.0, 0.0), EState::Foundation);
	const int32 Tree = Registry.AddSource(HOLSourceTypeNames::Tree, FVector(30000.0, 1500.0, 0.0));
	if (!TestTrue(TEXT("four sites and three farms placed"), FarmSite && NextFarm && MillSite && MillFarm && LoneMillSite && LoneFarm && CampSite)
		|| !TestTrue(TEXT("the berries and the tree added"), Berries != INDEX_NONE && Tree != INDEX_NONE))
	{
		return false;
	}
	const int32 NextField = NextFarm->GetLinkedSource();
	const int32 MillField = MillFarm->GetLinkedSource();
	const int32 LoneField = LoneFarm->GetLinkedSource();
	if (!TestTrue(TEXT("the finished farms have their fields (F6a)"), NextField != INDEX_NONE && MillField != INDEX_NONE && LoneField != INDEX_NONE)
		|| !TestEqual(TEXT("the farm site has no field yet"), FarmSite->GetLinkedSource(), int32(INDEX_NONE)))
	{
		return false;
	}

	const TArray<uint32> FarmBuilders = Test.PlaceBuilders(*FarmSite, 2);
	const TArray<uint32> MillBuilders = Test.PlaceBuilders(*MillSite, 2);
	const TArray<uint32> LoneMillBuilders = Test.PlaceBuilders(*LoneMillSite, 2);
	const TArray<uint32> CampBuilders = Test.PlaceBuilders(*CampSite, 1);
	TArray<uint32> All = FarmBuilders;
	All.Append(MillBuilders);
	All.Append(LoneMillBuilders);
	All.Append(CampBuilders);
	if (!TestTrue(TEXT("seven Peasants born"), All.Num() == 7 && !All.Contains(0u)))
	{
		return false;
	}
	Test.OrderBuild(FarmBuilders, *FarmSite);
	Test.OrderBuild(MillBuilders, *MillSite);
	Test.OrderBuild(LoneMillBuilders, *LoneMillSite);
	Test.OrderBuild(CampBuilders, *CampSite);

	// Every site finished at its first blow (hol.Build.Instant), then the step where its builders go on.
	IConsoleVariable* Instant = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Build.Instant"));
	if (!TestNotNull(TEXT("hol.Build.Instant"), Instant))
	{
		return false;
	}
	const int32 InstantBefore = Instant->GetInt();
	Instant->Set(1, ECVF_SetByCode);
	const auto AllFinished = [&]()
	{
		return FarmSite->IsFinished() && MillSite->IsFinished() && LoneMillSite->IsFinished() && CampSite->IsFinished();
	};
	int32 Steps = 0;
	while (!AllFinished() && Steps < 20)
	{
		Test.RunStep();
		++Steps;
	}
	Instant->Set(InstantBefore, ECVF_SetByCode);
	if (!TestTrue(TEXT("the four sites finished by their builders"), AllFinished()))
	{
		return false;
	}
	const int32 FarmField = FarmSite->GetLinkedSource();
	if (!TestTrue(TEXT("the farm finished has its field (F6a)"), FarmField != INDEX_NONE))
	{
		return false;
	}
	Test.RunStep();

	// (a) The farm's builders.
	const bool bFirstHolds = Test.HasTask(FarmBuilders[0], FarmField);
	const uint32 Holder = bFirstHolds ? FarmBuilders[0] : FarmBuilders[1];
	const uint32 Other = bFirstHolds ? FarmBuilders[1] : FarmBuilders[0];
	TestTrue(TEXT("(a) a builder of the farm gathers its field"), Test.HasTask(Holder, FarmField));
	TestFalse(TEXT("(a) the other one not with him (one place)"), Test.HasTask(Other, FarmField));
	TestTrue(TEXT("(a) the other one goes to the free field of the farm touching it"), Test.HasTask(Other, NextField));

	// (b) The mill beside the berries and a nearer farm: the berries.
	TestEqual(TEXT("(b) the mill's builders go to the berries"), Test.CountTasks(MillBuilders, Berries), 2);
	TestEqual(TEXT("(b) none takes the farmer's field"), Test.CountTasks(MillBuilders, MillField), 0);

	// (c) The mill beside a free farm alone: one builder takes it.
	TestEqual(TEXT("(c) one builder of the mill takes the free field"), Test.CountTasks(LoneMillBuilders, LoneField), 1);

	// (d) The lumber camp: the tree, as before.
	TestEqual(TEXT("(d) the lumber camp's builder goes to the tree"), Test.CountTasks(CampBuilders, Tree), 1);

	// The steps after: never two on one field.
	int32 MostOnFarm = 0;
	int32 MostOnLoneField = 0;
	for (int32 Step = 0; Step < 40; ++Step)
	{
		Test.RunStep();
		MostOnFarm = FMath::Max(MostOnFarm, Test.CountTasks(All, FarmField));
		MostOnLoneField = FMath::Max(MostOnLoneField, Test.CountTasks(All, LoneField));
	}
	TestTrue(TEXT("(a) never two on the farm's field"), MostOnFarm <= 1);
	TestTrue(TEXT("(c) never two on the lone field"), MostOnLoneField <= 1);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
