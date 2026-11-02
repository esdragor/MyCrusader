// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"
#include "Match/HOLStartRules.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"
#include "Simulation/HOLVolcanoRules.h"
#include "Simulation/HOLVolcanoSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// The lava on the real chain (V5, GDD 12.2, D377): a server world with the real game state, units, buildings, simulation steps and the
// volcano subsystem; the lava of a site with Catastrophe runs over what lies on its flow, nothing else, nothing before its eruption.
namespace UE::HeroesOfLiberty::Tests::VolcanoWorld
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	struct FWorld
	{
		UWorld* World = nullptr;
		AHOLGameState* GameState = nullptr;
		AHOLPlayerState* Player = nullptr;
		UHOLUnitSubsystem* Units = nullptr;
		UHOLSimulationSubsystem* Simulation = nullptr;
		UHOLVolcanoSubsystem* Volcano = nullptr;

		explicit FWorld(const bool bCatastrophe, const int32 Seed)
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			GameState = World->SpawnActor<AHOLGameState>();
			World->SetGameState(GameState);
			Player = World->SpawnActor<AHOLPlayerState>();
			Units = World->GetSubsystem<UHOLUnitSubsystem>();
			Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
			Volcano = World->GetSubsystem<UHOLVolcanoSubsystem>();
			if (!GameState || !Player || !Units || !Simulation || !Volcano)
			{
				return;
			}
			FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
			for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
			{
				Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : Index <= 2 ? EHOLSlotState::AI : EHOLSlotState::Closed;
				Config.Slots[Index].Team = 0;
			}
			Config.bCatastrophe = bCatastrophe;
			Config.Seed = Seed;
			GameState->InitVictory(Config);
			Player->SetMatchSlot(0, 0);
			if (!GameState->PlayerArray.Contains(Player))
			{
				GameState->AddPlayerState(Player);
			}
			Volcano->BindSteps();
		}

		~FWorld()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		bool IsReady() const { return Volcano != nullptr && Units != nullptr; }

		/** The steps run until the match clock reaches Seconds. */
		void RunTo(const float Seconds)
		{
			while (Volcano->GetClockOf(Simulation->GetStepIndex()) < Seconds)
			{
				Simulation->GetOnStepStarted().Broadcast(Simulation->GetStepIndex());
				Simulation->AdvanceStepIndexForTests();
			}
		}

		uint32 PlaceUnit(const FName Type, const int32 Slot, const FVector& Location)
		{
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = Type;
			Request.Count = 1;
			Request.OwnerPlayer = static_cast<uint8>(Slot);
			Request.Center = Location;
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			TArray<uint32> After;
			Units->GetAllUnitIds(After);
			for (const uint32 Id : After)
			{
				if (!Before.Contains(Id))
				{
					return Id;
				}
			}
			return 0;
		}

		bool IsAlive(const uint32 Id) const
		{
			FHOLUnitView View;
			return Units->GetUnitView(Id, View) && View.Health > 0;
		}

		AHOLBuilding* PlaceBuilding(const FName Type, const int32 Slot, const FVector& Location, const bool bForum = false)
		{
			AHOLBuilding* Building = World->SpawnActor<AHOLBuilding>(Location, FRotator::ZeroRotator);
			if (Building)
			{
				Building->Setup(Type, static_cast<uint8>(Slot), static_cast<uint8>(Slot));
				Building->SetPrimaryForum(bForum);
			}
			return Building;
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoBurnTest, "HeroesOfLiberty.Volcano.Burn", UE::HeroesOfLiberty::Tests::VolcanoWorld::Flags)
bool FHOLVolcanoBurnTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::VolcanoWorld;
	constexpr int32 Seed = 4242;
	FWorld W(/*bCatastrophe*/ true, Seed);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	// One site, one flow: 30000 cm east of the crater at the origin.
	HOLVolcano::FSite Site;
	HOLVolcano::FFlow Flow;
	Flow.Points = { FVector(0.0, 0.0, 0.0), FVector(30000.0, 0.0, 0.0) };
	Site.Flows.Add(Flow);
	W.Volcano->SetSites({ Site });
	const double Half = W.Volcano->GetSettings().LavaHalfWidth;

	// When the real chain says it erupts (the real SiteStateOfMatch of this configuration), found by walking the clock.
	const FHOLUnitTypeRow* ManAtArms = W.Units->FindTypeRow(TEXT("ManAtArms"));
	if (!TestNotNull(TEXT("the unit table has a Man-at-arms"), ManAtArms))
	{
		return false;
	}
	float EruptAt = -1.f;
	float EndAt = -1.f;
	for (float Now = 0.f; Now < 3000.f; Now += 0.5f)
	{
		const HOLVolcano::FSiteState State = HOLVolcano::SiteStateOfMatch(W.GameState->GetLaunchConfig(), 0, Site, Now, W.Volcano->GetSettings(), ManAtArms->Speed);
		if (EruptAt < 0.f && State.Phase == EHOLVolcanoPhase::Erupting)
		{
			EruptAt = State.EruptAt;
		}
		if (EruptAt >= 0.f && State.Phase == EHOLVolcanoPhase::Cooling)
		{
			EndAt = Now;
			break;
		}
	}
	if (!TestTrue(TEXT("the site erupts in the first 50 minutes"), EruptAt > 0.f && EndAt > EruptAt))
	{
		return false;
	}

	const FName Peasant(TEXT("Peasant"));
	const uint32 OnTheLine = W.PlaceUnit(Peasant, 1, FVector(1000.0, 0.0, 0.0));
	const uint32 InTheWidth = W.PlaceUnit(Peasant, 1, FVector(1000.0, Half - 100.0, 0.0));
	const uint32 OutOfTheWidth = W.PlaceUnit(Peasant, 1, FVector(1000.0, Half + 400.0, 0.0));
	const uint32 FarAhead = W.PlaceUnit(Peasant, 1, FVector(25000.0, 0.0, 0.0));
	const uint32 OtherSide = W.PlaceUnit(Peasant, 0, FVector(1500.0, -(Half + 400.0), 0.0));
	AHOLBuilding* HouseOnTheLine = W.PlaceBuilding(HOLBuildingTypeNames::House, 1, FVector(4000.0, 0.0, 0.0));
	AHOLBuilding* HouseAside = W.PlaceBuilding(HOLBuildingTypeNames::House, 1, FVector(4000.0, 3500.0, 0.0));
	AHOLBuilding* ForumOnTheLine = W.PlaceBuilding(HOLBuildingTypeNames::MainCentre, 1, FVector(9000.0, 0.0, 0.0), /*bForum*/ true);
	const bool bPlaced = OnTheLine && InTheWidth && OutOfTheWidth && FarAhead && OtherSide && HouseOnTheLine && HouseAside && ForumOnTheLine;
	if (!TestTrue(TEXT("everything placed"), bPlaced))
	{
		return false;
	}

	// Before the eruption: nothing is touched, whatever lies on the flow.
	W.RunTo(EruptAt - 1.f);
	TestTrue(TEXT("before the eruption: the unit on the line lives"), W.IsAlive(OnTheLine));
	TestTrue(TEXT("before the eruption: the house on the line stands"), IsValid(HouseOnTheLine) && HouseOnTheLine->GetHealth() > 0);
	TestEqual(TEXT("before the eruption: nothing burnt"), W.Volcano->GetNumUnitsBurnt() + W.Volcano->GetNumBuildingsBurnt(), 0);

	// 5 s in: the front is a little over 2400 cm out.
	W.RunTo(EruptAt + 5.f);
	TestFalse(TEXT("the lava passed the unit on the line"), W.IsAlive(OnTheLine));
	TestFalse(TEXT("... and the one within the width"), W.IsAlive(InTheWidth));
	TestTrue(TEXT("... not the one beside the width (16 m flow)"), W.IsAlive(OutOfTheWidth));
	TestTrue(TEXT("... nor the one on the other side"), W.IsAlive(OtherSide));
	TestTrue(TEXT("the front has not reached the unit far ahead"), W.IsAlive(FarAhead));
	TestTrue(TEXT("... nor the house 4000 cm out"), HouseOnTheLine->GetHealth() > 0);

	// Its end: the whole flow was run over.
	W.RunTo(EndAt + 1.f);
	TestFalse(TEXT("the unit far ahead was reached in the end"), W.IsAlive(FarAhead));
	TestTrue(TEXT("the unit beside the width never was"), W.IsAlive(OutOfTheWidth));
	TestTrue(TEXT("... nor the one on the other side"), W.IsAlive(OtherSide));
	TestTrue(TEXT("the house on the line was destroyed"), !IsValid(HouseOnTheLine) || HouseOnTheLine->GetHealth() <= 0);
	TestTrue(TEXT("the house 3500 cm aside stands"), IsValid(HouseAside) && HouseAside->GetHealth() > 0);
	TestTrue(TEXT("the main forum on the line fell (V5-Q2 = A: ruin or collapse, as from any blow)"), !IsValid(ForumOnTheLine) || ForumOnTheLine->GetHealth() <= 0 || ForumOnTheLine->IsForumDown());
	TestEqual(TEXT("units burnt: the three on the flow"), W.Volcano->GetNumUnitsBurnt(), 3);
	TestEqual(TEXT("buildings burnt: the house and the forum, each once"), W.Volcano->GetNumBuildingsBurnt(), 2);
	return true;
}

// The cooled flow forbids its ground (V6, D379): an obstacle for the navmesh while it cools, none before and none once it is basalt.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoObstacleWorldTest, "HeroesOfLiberty.Volcano.Obstacle", UE::HeroesOfLiberty::Tests::VolcanoWorld::Flags)
bool FHOLVolcanoObstacleWorldTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::VolcanoWorld;
	FWorld W(/*bCatastrophe*/ true, 4242);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	HOLVolcano::FSite Site;
	HOLVolcano::FFlow Flow;
	Flow.Points = { FVector(0.0, 0.0, 0.0), FVector(6000.0, 0.0, 0.0), FVector(6000.0, 5000.0, 0.0) };
	Site.Flows.Add(Flow);
	W.Volcano->SetSites({ Site });
	const FHOLUnitTypeRow* ManAtArms = W.Units->FindTypeRow(TEXT("ManAtArms"));
	if (!TestNotNull(TEXT("the unit table has a Man-at-arms"), ManAtArms))
	{
		return false;
	}
	float EruptAt = -1.f;
	float CoolingAt = -1.f;
	float RestAt = -1.f;
	for (float Now = 0.f; Now < 3000.f && RestAt < 0.f; Now += 0.5f)
	{
		const HOLVolcano::FSiteState State = HOLVolcano::SiteStateOfMatch(W.GameState->GetLaunchConfig(), 0, Site, Now, W.Volcano->GetSettings(), ManAtArms->Speed);
		if (EruptAt < 0.f && State.Phase == EHOLVolcanoPhase::Erupting)
		{
			EruptAt = Now;
		}
		if (CoolingAt < 0.f && State.Phase == EHOLVolcanoPhase::Cooling)
		{
			CoolingAt = Now;
		}
		if (CoolingAt >= 0.f && State.Phase == EHOLVolcanoPhase::Rest)
		{
			RestAt = Now;
		}
	}
	if (!TestTrue(TEXT("the site erupts, cools and rests within 50 minutes"), EruptAt > 0.f && CoolingAt > EruptAt && RestAt > CoolingAt))
	{
		return false;
	}
	W.RunTo(EruptAt - 1.f);
	TestNull(TEXT("before the eruption: free ground"), W.Volcano->FindObstacle(0));
	W.RunTo(EruptAt + 3.f);
	TestNull(TEXT("while the lava advances: nothing forbidden yet (it burns instead)"), W.Volcano->FindObstacle(0));
	W.RunTo(CoolingAt + 1.f);
	AHOLLavaObstacle* Obstacle = W.Volcano->FindObstacle(0);
	if (TestNotNull(TEXT("cooling: an obstacle stands"), Obstacle))
	{
		TestEqual(TEXT("... one box per segment of the flow"), Obstacle->GetNumBoxes(), 2);
		TestTrue(TEXT("... counted by the navigation"), Obstacle->IsBlockingNavigation());
	}
	W.RunTo(CoolingAt + 20.f);
	TestTrue(TEXT("still the same obstacle 20 s later (not rebuilt every step)"), W.Volcano->FindObstacle(0) == Obstacle);
	W.RunTo(RestAt + 1.f);
	TestNull(TEXT("basalt: the ground is free again"), W.Volcano->FindObstacle(0));
	TestFalse(TEXT("... the obstacle is gone"), IsValid(Obstacle));
	return true;
}

// Without the Catastrophe option the site stays dormant: the lava never runs, nothing is touched (D398).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoNoCatastropheTest, "HeroesOfLiberty.Volcano.BurnNothingWithoutCatastrophe", UE::HeroesOfLiberty::Tests::VolcanoWorld::Flags)
bool FHOLVolcanoNoCatastropheTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::VolcanoWorld;
	FWorld W(/*bCatastrophe*/ false, 4242);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	HOLVolcano::FSite Site;
	HOLVolcano::FFlow Flow;
	Flow.Points = { FVector(0.0, 0.0, 0.0), FVector(30000.0, 0.0, 0.0) };
	Site.Flows.Add(Flow);
	W.Volcano->SetSites({ Site });
	const uint32 OnTheLine = W.PlaceUnit(FName(TEXT("Peasant")), 1, FVector(1000.0, 0.0, 0.0));
	if (!TestTrue(TEXT("unit placed"), OnTheLine != 0))
	{
		return false;
	}
	W.RunTo(1500.f);
	TestTrue(TEXT("25 minutes without Catastrophe: the unit on the flow lives"), W.IsAlive(OnTheLine));
	TestEqual(TEXT("nothing burnt"), W.Volcano->GetNumUnitsBurnt(), 0);
	return true;
}

#endif