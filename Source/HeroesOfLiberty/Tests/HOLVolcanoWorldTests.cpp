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
#include "Simulation/HOLVolcanoLibrary.h"
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

		explicit FWorld(const bool bCatastrophe, const int32 Seed, const bool bSeparateTeams = false)
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
				Config.Slots[Index].Team = bSeparateTeams ? Index : 0;
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

// The AI leaves the lava's way from the alert on (V11, D427): on the real chain, the units caught in the 16 m corridor get a way out before the
// front moves; the one beside it, and those of another slot, none.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoEvacuateWorldTest, "HeroesOfLiberty.Volcano.Evacuate", UE::HeroesOfLiberty::Tests::VolcanoWorld::Flags)
bool FHOLVolcanoEvacuateWorldTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::VolcanoWorld;
	FWorld W(/*bCatastrophe*/ true, 4242);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	HOLVolcano::FSite Site;
	HOLVolcano::FFlow Flow;
	Flow.Points = { FVector(0.0, 0.0, 0.0), FVector(30000.0, 0.0, 0.0) };
	Site.Flows.Add(Flow);
	W.Volcano->SetSites({ Site });
	const FHOLUnitTypeRow* ManAtArms = W.Units->FindTypeRow(TEXT("ManAtArms"));
	if (!TestNotNull(TEXT("the Man-at-arms row"), ManAtArms))
	{
		return false;
	}
	const double Half = W.Volcano->GetSettings().LavaHalfWidth;
	float WarningAt = -1.f;
	float EruptAt = -1.f;
	for (float Now = 0.f; Now < 3000.f && EruptAt < 0.f; Now += 0.5f)
	{
		const HOLVolcano::FSiteState State = HOLVolcano::SiteStateOfMatch(W.GameState->GetLaunchConfig(), 0, Site, Now, W.Volcano->GetSettings(), ManAtArms->Speed);
		if (WarningAt < 0.f && State.Phase == EHOLVolcanoPhase::Warning)
		{
			WarningAt = State.WarningAt;
		}
		if (State.Phase == EHOLVolcanoPhase::Erupting)
		{
			EruptAt = State.EruptAt;
		}
	}
	if (!TestTrue(TEXT("the site warns then erupts"), WarningAt > 0.f && EruptAt > WarningAt))
	{
		return false;
	}

	const FName Peasant(TEXT("Peasant"));
	const uint32 OnTheLine = W.PlaceUnit(Peasant, 1, FVector(5000.0, 0.0, 0.0));
	const uint32 InTheWidth = W.PlaceUnit(Peasant, 1, FVector(9000.0, Half - 100.0, 0.0));
	const uint32 Beside = W.PlaceUnit(Peasant, 1, FVector(5000.0, Half + 400.0, 0.0));
	const uint32 FarAhead = W.PlaceUnit(Peasant, 1, FVector(25000.0, -300.0, 0.0));
	const uint32 OtherSlots = W.PlaceUnit(Peasant, 2, FVector(5000.0, 100.0, 0.0));
	if (!TestTrue(TEXT("everything placed"), OnTheLine && InTheWidth && Beside && FarAhead && OtherSlots))
	{
		return false;
	}
	TArray<UHOLVolcanoSubsystem::FEvacuation> Entries;

	// Armed, before the alert: nobody leaves.
	W.RunTo(WarningAt - 1.f);
	TestEqual(TEXT("before the alert: nobody leaves"), W.Volcano->GetEvacuations(1, Entries), 0);

	// The alert: the three caught by the corridor leave, the whole length of it (the front has not moved); the one beside it and the other slot's do not.
	W.RunTo(WarningAt + 1.f);
	TestEqual(TEXT("at the alert: the three in the corridor of slot 1 leave"), W.Volcano->GetEvacuations(1, Entries), 3);
	TArray<uint32> Ids;
	for (const UHOLVolcanoSubsystem::FEvacuation& Entry : Entries)
	{
		Ids.Add(Entry.UnitId);
		FHOLUnitView View;
		W.Units->GetUnitView(Entry.UnitId, View);
		TestFalse(TEXT("... each to a point out of the corridor (its radius counted)"), HOLVolcano::IsInCorridor(Flow, W.Volcano->GetSettings().LavaHalfWidth, Entry.Target, View.Radius));
	}
	TestTrue(TEXT("... the one on the line"), Ids.Contains(OnTheLine));
	TestTrue(TEXT("... the one in the width"), Ids.Contains(InTheWidth));
	TestTrue(TEXT("... the one far ahead, the front is not near"), Ids.Contains(FarAhead));
	TestFalse(TEXT("the one beside the corridor stays"), Ids.Contains(Beside));
	TestFalse(TEXT("another slot's unit is not slot 1's to move"), Ids.Contains(OtherSlots));
	TestEqual(TEXT("slot 2 has its own: one"), W.Volcano->GetEvacuations(2, Entries), 1);
	TestEqual(TEXT("the orders are sent: three"), W.Volcano->EvacuateOwner(1), 3);
	return true;
}

// Without the Catastrophe option the AI has nothing to flee (D398).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoEvacuateStandardTest, "HeroesOfLiberty.Volcano.EvacuateNothingWithoutCatastrophe", UE::HeroesOfLiberty::Tests::VolcanoWorld::Flags)
bool FHOLVolcanoEvacuateStandardTest::RunTest(const FString& Parameters)
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
	const uint32 OnTheLine = W.PlaceUnit(FName(TEXT("Peasant")), 1, FVector(5000.0, 0.0, 0.0));
	TArray<UHOLVolcanoSubsystem::FEvacuation> Entries;
	W.RunTo(900.f);
	TestTrue(TEXT("placed"), OnTheLine != 0);
	TestEqual(TEXT("no Catastrophe: nobody leaves"), W.Volcano->GetEvacuations(1, Entries), 0);
	return true;
}

// The altar held on the real chain (V8, D380, D348's rule): a server world, the game state's configuration, the units, the steps. One camp in the zone
// counts up; an adversary in it freezes the count; held 60 s starts the alert at once; the site's state and the HUD's view say so.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoAltarWorldTest, "HeroesOfLiberty.Volcano.Altar", UE::HeroesOfLiberty::Tests::VolcanoWorld::Flags)
bool FHOLVolcanoAltarWorldTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::VolcanoWorld;
	FWorld W(/*bCatastrophe*/ true, /*Seed*/ 4242, /*bSeparateTeams*/ true);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	HOLVolcano::FSite Site;
	HOLVolcano::FFlow Flow;
	Flow.Points = { FVector(0.0, 0.0, 0.0), FVector(30000.0, 0.0, 0.0) };
	Site.Flows.Add(Flow);
	Site.Altar.bPresent = true;
	Site.Altar.Location = FVector(0.0, 5000.0, 0.0);
	Site.Altar.Radius = 1200.f;
	W.Volcano->SetSites({ Site });
	const float HoldSeconds = W.Volcano->GetSettings().AltarHoldSeconds;

	TestTrue(TEXT("the altar is lit from the start"), W.Volcano->GetSiteState(0).bAltarLit);
	W.RunTo(5.f);
	TestEqual(TEXT("nobody in the zone: nothing counted"), W.Volcano->GetAltarHeldSeconds(0), 0.f);

	// A unit of slot 0 on the altar: the count rises with the clock.
	const uint32 Holder = W.PlaceUnit(FName(TEXT("Peasant")), 0, FVector(0.0, 5300.0, 0.0));
	const uint32 Outside = W.PlaceUnit(FName(TEXT("Peasant")), 1, FVector(0.0, 5000.0 + 1200.0 + 600.0, 0.0));
	if (!TestTrue(TEXT("units placed"), Holder != 0 && Outside != 0))
	{
		return false;
	}
	W.RunTo(25.f);
	TestTrue(TEXT("one camp in the zone: holding"), W.Volcano->GetAltarHold(0) == HOLVolcano::EAltarHold::Holding);
	const float After20 = W.Volcano->GetAltarHeldSeconds(0);
	TestTrue(TEXT("20 s of one camp: about 20 s held (an adversary just outside the zone does not count)"), After20 > 19.f && After20 < 21.f);
	const FHOLVolcanoSiteView View = UHOLVolcanoLibrary::GetSiteView(W.World, 0);
	TestTrue(TEXT("the HUD's view: the altar is lit and a third held"), View.bAltarLit && View.AltarPercent >= 30 && View.AltarPercent <= 36 && View.AltarHold == 1);
	TestEqual(TEXT("... the countdown at the altar"), View.AltarSecondsLeft, FMath::CeilToInt(HoldSeconds - After20));

	// An adversary in the zone freezes the count (the ruin's rule).
	const uint32 Intruder = W.PlaceUnit(FName(TEXT("Peasant")), 1, FVector(0.0, 4700.0, 0.0));
	if (!TestTrue(TEXT("intruder placed"), Intruder != 0))
	{
		return false;
	}
	W.RunTo(35.f);
	const float Frozen = W.Volcano->GetAltarHeldSeconds(0);
	W.RunTo(55.f);
	TestTrue(TEXT("two camps: contested"), W.Volcano->GetAltarHold(0) == HOLVolcano::EAltarHold::Contested);
	TestTrue(TEXT("... the count is frozen for 20 s"), FMath::IsNearlyEqual(W.Volcano->GetAltarHeldSeconds(0), Frozen, 0.01f));
	TestTrue(TEXT("... still no alert"), W.Volcano->GetSiteState(0).Phase == EHOLVolcanoPhase::Armed);
	TestEqual(TEXT("... no wake yet"), W.Volcano->GetNumWakes(0), 0);

	// The intruder is gone: the count goes on from where it stood and reaches the hold time.
	W.Units->KillUnitsWhere([Intruder](const uint32 UnitId, FName, const uint8, const FVector&, const float) { return UnitId == Intruder; });
	W.RunTo(55.f + (HoldSeconds - Frozen) - 3.f);
	TestEqual(TEXT("3 s before the hold time: not woken"), W.Volcano->GetNumWakes(0), 0);
	TestTrue(TEXT("... still armed"), W.Volcano->GetSiteState(0).Phase == EHOLVolcanoPhase::Armed);
	W.RunTo(55.f + (HoldSeconds - Frozen) + 3.f);
	TestEqual(TEXT("held: the altar woke the cycle"), W.Volcano->GetNumWakes(0), 1);
	const HOLVolcano::FSiteState Woken = W.Volcano->GetSiteState(0);
	TestTrue(TEXT("the alert has begun, at once (Q13 = B)"), Woken.Phase == EHOLVolcanoPhase::Warning && Woken.bWoken);
	TestTrue(TEXT("... long before the plain window opens"), Woken.WarningAt < 200.f);
	TestFalse(TEXT("... the altar is no longer lit"), Woken.bAltarLit);
	const FHOLVolcanoSiteView WokenView = UHOLVolcanoLibrary::GetSiteView(W.World, 0);
	TestTrue(TEXT("the HUD's view: alert with a countdown of 60 to 90 s at most"), WokenView.Phase == static_cast<int32>(EHOLVolcanoPhase::Warning) && WokenView.SecondsToErupt > 0 && WokenView.SecondsToErupt <= 90);
	return true;
}

// The altar's count belongs to its holder (V8, the user's remark on volcano_ai): when the camp that held it is gone and the other camp stands alone in
// the zone, the new holder starts from 0, never from the old holder's percentage. Real chain: server world, two camps, the units, the steps.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoAltarNewHolderTest, "HeroesOfLiberty.Volcano.AltarNewHolderStartsAtZero", UE::HeroesOfLiberty::Tests::VolcanoWorld::Flags)
bool FHOLVolcanoAltarNewHolderTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::VolcanoWorld;
	FWorld W(/*bCatastrophe*/ true, /*Seed*/ 4242, /*bSeparateTeams*/ true);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	HOLVolcano::FSite Site;
	HOLVolcano::FFlow Flow;
	Flow.Points = { FVector(0.0, 0.0, 0.0), FVector(30000.0, 0.0, 0.0) };
	Site.Flows.Add(Flow);
	Site.Altar.bPresent = true;
	Site.Altar.Location = FVector(0.0, 5000.0, 0.0);
	Site.Altar.Radius = 1200.f;
	W.Volcano->SetSites({ Site });

	// Slot 0 holds the altar alone for ~28 s (almost half).
	const uint32 First = W.PlaceUnit(FName(TEXT("Peasant")), 0, FVector(0.0, 5300.0, 0.0));
	if (!TestTrue(TEXT("first holder placed"), First != 0))
	{
		return false;
	}
	W.RunTo(30.f);
	const float FirstCount = W.Volcano->GetAltarHeldSeconds(0);
	TestTrue(TEXT("the first holder has about 28 s"), FirstCount > 25.f && FirstCount < 31.f);

	// The other camp comes in: contested, frozen. The first holder dies: the other camp is alone and starts from 0.
	const uint32 Second = W.PlaceUnit(FName(TEXT("Peasant")), 1, FVector(0.0, 4700.0, 0.0));
	if (!TestTrue(TEXT("second camp placed"), Second != 0))
	{
		return false;
	}
	W.RunTo(40.f);
	TestTrue(TEXT("two camps: contested"), W.Volcano->GetAltarHold(0) == HOLVolcano::EAltarHold::Contested);
	TestTrue(TEXT("... frozen at the first holder's count"), FMath::IsNearlyEqual(W.Volcano->GetAltarHeldSeconds(0), FirstCount, 0.5f));
	W.Units->KillUnitsWhere([First](const uint32 UnitId, FName, const uint8, const FVector&, const float) { return UnitId == First; });
	W.RunTo(45.f);
	TestTrue(TEXT("the other camp alone: holding"), W.Volcano->GetAltarHold(0) == HOLVolcano::EAltarHold::Holding);
	const float NewCount = W.Volcano->GetAltarHeldSeconds(0);
	TestTrue(TEXT("... its count restarted from 0 (about 5 s after 5 s alone), not the loser's 28 s"), NewCount > 0.f && NewCount < 7.f);
	W.RunTo(55.f);
	const float LaterCount = W.Volcano->GetAltarHeldSeconds(0);
	TestTrue(TEXT("... and goes on from there (about 15 s)"), LaterCount > 12.f && LaterCount < 17.f);

	// The same camp coming back after an absence keeps its own decayed count (the ruin's rule): not the object of this change, but it must not restart.
	W.Units->KillUnitsWhere([Second](const uint32 UnitId, FName, const uint8, const FVector&, const float) { return UnitId == Second; });
	W.RunTo(58.f);
	const float Decayed = W.Volcano->GetAltarHeldSeconds(0);
	TestTrue(TEXT("nobody: the count goes down"), Decayed < LaterCount);
	const uint32 Again = W.PlaceUnit(FName(TEXT("Peasant")), 1, FVector(0.0, 5300.0, 0.0));
	TestTrue(TEXT("the holder's camp is back"), Again != 0);
	W.RunTo(63.f);
	TestTrue(TEXT("... it goes on from its decayed count, not from 0"), W.Volcano->GetAltarHeldSeconds(0) > Decayed + 3.f);
	return true;
}

// Without Catastrophe the altar is inert (D398): a unit stands on it as long as it likes.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoAltarInertTest, "HeroesOfLiberty.Volcano.AltarInertWithoutCatastrophe", UE::HeroesOfLiberty::Tests::VolcanoWorld::Flags)
bool FHOLVolcanoAltarInertTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::VolcanoWorld;
	FWorld W(/*bCatastrophe*/ false, /*Seed*/ 4242, /*bSeparateTeams*/ true);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	HOLVolcano::FSite Site;
	Site.Altar.bPresent = true;
	Site.Altar.Location = FVector(0.0, 5000.0, 0.0);
	HOLVolcano::FFlow Flow;
	Flow.Points = { FVector(0.0, 0.0, 0.0), FVector(30000.0, 0.0, 0.0) };
	Site.Flows.Add(Flow);
	W.Volcano->SetSites({ Site });
	W.PlaceUnit(FName(TEXT("Peasant")), 0, FVector(0.0, 5000.0, 0.0));
	W.RunTo(90.f);
	TestEqual(TEXT("no Catastrophe: nothing woken"), W.Volcano->GetNumWakes(0), 0);
	TestEqual(TEXT("no Catastrophe: the HUD sees no site"), UHOLVolcanoLibrary::GetSiteCount(W.World), 0);
	return true;
}

// What the AI reads of the altar on the real chain (V11, D639, D640): the camp alone in the zone, an adversary's strength (a Peasant a quarter),
// the distances of the nearest trace from the buildings and from the altar, the unlit altar not measured, and the library's squad decision.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoAltarMeasureTest, "HeroesOfLiberty.Volcano.AltarMeasure", UE::HeroesOfLiberty::Tests::VolcanoWorld::Flags)
bool FHOLVolcanoAltarMeasureTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::VolcanoWorld;
	FWorld W(/*bCatastrophe*/ true, /*Seed*/ 4242, /*bSeparateTeams*/ true);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	HOLVolcano::FSite Site;
	HOLVolcano::FFlow Flow;
	Flow.Points = { FVector(0.0, 0.0, 0.0), FVector(30000.0, 0.0, 0.0) };
	Site.Flows.Add(Flow);
	Site.Altar.bPresent = true;
	Site.Altar.Location = FVector(0.0, 9000.0, 0.0);
	Site.Altar.Radius = 1200.f;
	W.Volcano->SetSites({ Site });
	W.RunTo(2.f);

	UHOLVolcanoSubsystem::FAltarMeasure Empty;
	TestTrue(TEXT("the lit altar is measured"), W.Volcano->MeasureAltar(0, 1, Empty));
	TestFalse(TEXT("... nobody in the zone: no adversary"), Empty.bAdversary);
	TestFalse(TEXT("... nobody: the camp is not alone in the zone either"), Empty.bOwnCampAlone);
	TestTrue(TEXT("... the altar is 90 m from the line"), FMath::IsNearlyEqual(Empty.TraceFromAltar, 9000.f, 1.f));

	// A building of slot 1 at 60 m from the line: the buildings' distance.
	W.PlaceBuilding(FName(TEXT("House")), 1, FVector(15000.0, 6000.0, 0.0));
	// Slot 1 holds the altar with a Man-at-arms; slot 0 has a Man-at-arms and a Peasant there: strength 1.25.
	W.PlaceUnit(FName(TEXT("ManAtArms")), 1, FVector(0.0, 9100.0, 0.0));
	W.PlaceUnit(FName(TEXT("ManAtArms")), 0, FVector(0.0, 8900.0, 0.0));
	W.PlaceUnit(FName(TEXT("Peasant")), 0, FVector(100.0, 8900.0, 0.0));
	UHOLVolcanoSubsystem::FAltarMeasure Contested;
	TestTrue(TEXT("measured again"), W.Volcano->MeasureAltar(0, 1, Contested));
	TestTrue(TEXT("an adversary stands in the zone"), Contested.bAdversary);
	TestFalse(TEXT("... the camp is not alone"), Contested.bOwnCampAlone);
	TestTrue(TEXT("... strength 1 + 0.25"), FMath::IsNearlyEqual(Contested.AdversaryStrength, 1.25f, 0.01f));
	TestTrue(TEXT("the nearest trace from the building: 60 cm... 6000 cm"), FMath::IsNearlyEqual(Contested.TraceFromBuildings, 6000.f, 1.f));

	TArray<FHOLAIAltar> Altars;
	UHOLVolcanoLibrary::ReadAIAltars(W.World, 1, Altars);
	if (!TestEqual(TEXT("the library hands over the lit altar"), Altars.Num(), 1))
	{
		return false;
	}
	TestTrue(TEXT("... with the adversary"), Altars[0].bAdversary && Altars[0].SiteIndex == 0);
	int32 Squad = 0;
	bool bDiversion = false;
	UHOLVolcanoLibrary::DecideAltarSquad(W.World, Altars[0], 10, 0, 1, Squad, bDiversion);
	TestEqual(TEXT("10 free soldiers against 1.25: at least the minimum, more than him"), Squad, 2);

	// The same altar seen by slot 0's side (it holds it alone once the adversary is gone): the free altar, held by a quarter.
	W.Units->KillUnitsWhere([](const uint32, FName, const uint8 Owner, const FVector&, const float) { return Owner == 1; });
	TArray<FHOLAIAltar> Free;
	UHOLVolcanoLibrary::ReadAIAltars(W.World, 0, Free);
	if (!TestEqual(TEXT("the altar is still lit"), Free.Num(), 1))
	{
		return false;
	}
	TestFalse(TEXT("... no adversary for slot 0"), Free[0].bAdversary);
	UHOLVolcanoLibrary::DecideAltarSquad(W.World, Free[0], 8, 0, 1, Squad, bDiversion);
	TestEqual(TEXT("slot 0 has no building: the trace is infinitely far, a quarter of 8"), Squad, 2);
	return true;
}

#endif