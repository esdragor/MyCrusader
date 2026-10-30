// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAISummary.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Map/HOLStartSlot.h"
#include "Map/HOLVisionSubsystem.h"
#include "Match/HOLGameState.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

#if WITH_DEV_AUTOMATION_TESTS

// F3 checks (FogInProgress.md, D381-D383, D521, D522): the fog's subsystem on a match's server, its real units and trees.
namespace UE::HeroesOfLiberty::Tests::VisionMatch
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FName Peasant(TEXT("Peasant"));

	/** Slots 0 and 1 in team 1, slot 2 in team 2, the others closed. */
	FHOLMatchConfig MakeTeams()
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (FHOLMatchSlot& Slot : Config.Slots)
		{
			Slot.State = EHOLSlotState::Closed;
			Slot.Team = 0;
		}
		Config.Slots[0].State = EHOLSlotState::Human;
		Config.Slots[0].Team = 1;
		Config.Slots[1].State = EHOLSlotState::AI;
		Config.Slots[1].Team = 1;
		Config.Slots[2].State = EHOLSlotState::AI;
		Config.Slots[2].Team = 2;
		return Config;
	}

	/** A match's server: its unit and source registries and the fog, no movement (the test moves the units). */
	struct FVisionWorld
	{
		UWorld* World = nullptr;
		UHOLUnitSubsystem* Units = nullptr;
		UHOLSourceSubsystem* Sources = nullptr;
		UHOLVisionSubsystem* Vision = nullptr;
		UMassEntitySubsystem* Entities = nullptr;

		FVisionWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			Units = World->GetSubsystem<UHOLUnitSubsystem>();
			Sources = World->GetSubsystem<UHOLSourceSubsystem>();
			Vision = World->GetSubsystem<UHOLVisionSubsystem>();
			Entities = World->GetSubsystem<UMassEntitySubsystem>();
			if (Sources)
			{
				Sources->OnWorldBeginPlay(*World);
			}
		}

		~FVisionWorld()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		bool IsReady() const { return Units && Sources && Vision && Entities && World->GetSubsystem<UHOLSimulationSubsystem>(); }

		/**
		 * A unit of Type born for Owner at Location, its identifier (0 if none). bSearching: born again until its identifier
		 * is a multiple of 4, so it searches for a target each step (the test world's step index stays 0, a unit searches
		 * one step of 4 by its identifier, UHOLCombatProcessor).
		 */
		uint32 Place(const FName Type, const int32 Owner, const FVector& Location, const bool bSearching = false) const
		{
			for (int32 Attempt = 0; Attempt < 8; ++Attempt)
			{
				const uint32 Born = PlaceOnce(Type, Owner, Location);
				if (Born == 0 || !bSearching || Born % 4 == 0)
				{
					return Born;
				}
				Units->RequestDestroyUnits({ Born });
				Units->FlushDestructions();
			}
			return 0;
		}

		uint32 PlaceOnce(const FName Type, const int32 Owner, const FVector& Location) const
		{
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = Type;
			Request.Count = 1;
			Request.OwnerPlayer = static_cast<uint8>(Owner);
			Request.Center = Location;
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			TArray<uint32> After;
			Units->GetAllUnitIds(After);
			After.RemoveAll([&Before](const uint32 Id) { return Before.Contains(Id); });
			return After.Num() == 1 ? After[0] : 0;
		}

		void Move(const uint32 UnitId, const FVector& Location) const
		{
			if (FTransformFragment* Transform = Entities->GetMutableEntityManager().GetFragmentDataPtr<FTransformFragment>(Units->FindEntity(UnitId)))
			{
				Transform->GetMutableTransform().SetLocation(Location);
			}
		}

		FVector Centre(const FIntPoint& Cell) const
		{
			return FVector(Vision->GetMap().GetGrid().GetCenter(Cell), 0.);
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionMatchUnitsTest, "HeroesOfLiberty.Vision.Match.Units", UE::HeroesOfLiberty::Tests::VisionMatch::Flags)
bool FHOLVisionMatchUnitsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::VisionMatch;
	FVisionWorld Match;
	if (!TestTrue(TEXT("the match's subsystems"), Match.IsReady()))
	{
		return false;
	}
	const FHOLMatchConfig Teams = MakeTeams();
	Match.Vision->Start(Teams, EHOLFogMode::Normal);
	TestTrue(TEXT("started"), Match.Vision->IsStarted());
	TestEqual(TEXT("one camp per team"), Match.Vision->GetMap().GetNumCamps(), 2);
	TestEqual(TEXT("allies share a camp"), Match.Vision->GetCampIndex(1), Match.Vision->GetCampIndex(0));
	TestNotEqual(TEXT("the enemy has its own"), Match.Vision->GetCampIndex(2), Match.Vision->GetCampIndex(0));
	TestEqual(TEXT("a closed slot has none"), Match.Vision->GetCampIndex(5), INDEX_NONE);

	const uint32 Scout = Match.Place(Peasant, 0, FVector::ZeroVector);
	if (!TestNotEqual(TEXT("a Peasant born"), Scout, 0u))
	{
		return false;
	}
	const FVector Near(2000., 0., 0.);
	TestEqual(TEXT("before the step: black"), Match.Vision->GetVision(0, Near), EHOLCellVision::Unexplored);
	Match.Vision->Update();
	TestEqual(TEXT("its observer"), Match.Vision->GetMap().GetNumObservers(), 1);
	TestTrue(TEXT("it sees around it"), Match.Vision->IsVisible(0, Near));
	TestTrue(TEXT("its ally sees it too"), Match.Vision->IsVisible(1, Near));
	TestEqual(TEXT("the enemy: black"), Match.Vision->GetVision(2, Near), EHOLCellVision::Unexplored);
	TestFalse(TEXT("beyond its sight: black"), Match.Vision->IsVisible(0, FVector(5000., 0., 0.)));

	// A wood across its way: 3 trees a cell on a column three cells ahead of where it goes, before it gets there.
	const FIntPoint Goal = Match.Vision->GetMap().GetGrid().ToCell(FVector2D(20000., 0.));
	for (int32 Row = -9; Row <= 9; ++Row)
	{
		for (const double Offset : { -100., 0., 100. })
		{
			Match.Sources->AddSource(HOLSourceTypeNames::Tree, Match.Centre(FIntPoint(Goal.X + 3, Goal.Y + Row)) + FVector(Offset, Offset, 0.));
		}
	}
	Match.Move(Scout, Match.Centre(Goal));
	Match.Vision->Update();
	TestEqual(TEXT("left behind: explored"), Match.Vision->GetVision(0, Near), EHOLCellVision::Explored);
	TestTrue(TEXT("where it stands: in sight"), Match.Vision->IsVisible(0, Match.Centre(Goal)));
	TestTrue(TEXT("the wood's edge: in sight"), Match.Vision->IsVisible(0, Match.Centre(FIntPoint(Goal.X + 3, Goal.Y))));
	TestEqual(TEXT("behind the wood: black"), Match.Vision->GetVision(0, Match.Centre(FIntPoint(Goal.X + 5, Goal.Y))), EHOLCellVision::Unexplored);
	TestFalse(TEXT("an enemy behind the wood: not seen"), Match.Vision->IsVisible(0, Match.Centre(FIntPoint(Goal.X + 4, Goal.Y))));

	Match.Units->RequestDestroyUnits({ Scout });
	Match.Units->FlushDestructions();
	Match.Vision->Update();
	TestEqual(TEXT("gone: no observer"), Match.Vision->GetMap().GetNumObservers(), 0);
	TestEqual(TEXT("gone: what it saw stays explored"), Match.Vision->GetVision(0, Match.Centre(Goal)), EHOLCellVision::Explored);

	// D522: no fog, everything in sight for everyone.
	Match.Vision->Start(Teams, EHOLFogMode::Disabled);
	TestTrue(TEXT("no fog: the enemy sees everything"), Match.Vision->IsVisible(2, FVector(30000., -30000., 0.)));
	// D389: the map explored, nothing in sight without an observer.
	Match.Vision->Start(Teams, EHOLFogMode::Explored);
	TestEqual(TEXT("explored map: grey from the start"), Match.Vision->GetVision(2, FVector(30000., -30000., 0.)), EHOLCellVision::Explored);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionMatchBuildingsTest, "HeroesOfLiberty.Vision.Match.Buildings", UE::HeroesOfLiberty::Tests::VisionMatch::Flags)
bool FHOLVisionMatchBuildingsTest::RunTest(const FString& Parameters)
{
	// F3.2: the buildings see too, a site half as far (their sight from the real table).
	using namespace UE::HeroesOfLiberty::Tests::VisionMatch;
	FVisionWorld Match;
	if (!TestTrue(TEXT("the match's subsystems"), Match.IsReady()))
	{
		return false;
	}
	Match.Vision->Start(MakeTeams(), EHOLFogMode::Normal);
	const UDataTable* Types = GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous();
	const FHOLBuildingTypeRow* HouseRow = Types ? Types->FindRow<FHOLBuildingTypeRow>(HOLBuildingTypeNames::House, TEXT("Vision test")) : nullptr;
	const FHOLBuildingTypeRow* BarracksRow = Types ? Types->FindRow<FHOLBuildingTypeRow>(HOLBuildingTypeNames::Barracks, TEXT("Vision test")) : nullptr;
	if (!TestNotNull(TEXT("the house's row"), HouseRow) || !TestNotNull(TEXT("the barracks' row"), BarracksRow))
	{
		return false;
	}
	AHOLBuilding* House = Match.World->SpawnActor<AHOLBuilding>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHOLBuilding* Site = Match.World->SpawnActor<AHOLBuilding>(FVector(30000., 0., 0.), FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("the house"), House) || !TestNotNull(TEXT("the site"), Site))
	{
		return false;
	}
	House->Setup(HOLBuildingTypeNames::House, 0, 0);
	Site->Setup(HOLBuildingTypeNames::Barracks, 2, 2, EHOLConstructionState::Foundation);
	Match.Vision->Update();
	const double Tile = UHOLBuildingSettings::GetTileSize();
	TestTrue(TEXT("the house sees around it"), Match.Vision->IsVisible(0, FVector(HouseRow->SightRadius - Tile, 0., 0.)));
	TestTrue(TEXT("its owner's ally too"), Match.Vision->IsVisible(1, FVector(HouseRow->SightRadius - Tile, 0., 0.)));
	TestFalse(TEXT("the enemy does not"), Match.Vision->IsVisible(2, FVector::ZeroVector));
	TestFalse(TEXT("not beyond its sight"), Match.Vision->IsVisible(0, FVector(HouseRow->SightRadius + Tile, 0., 0.)));
	const double Half = 0.5 * BarracksRow->SightRadius;
	TestTrue(TEXT("a site sees half as far"), Match.Vision->IsVisible(2, FVector(30000. + Half - Tile, 0., 0.)));
	TestFalse(TEXT("a site: not beyond the half"), Match.Vision->IsVisible(2, FVector(30000. + Half + Tile, 0., 0.)));

	House->Destroy();
	Match.Vision->Update();
	TestFalse(TEXT("destroyed: no more sight"), Match.Vision->IsVisible(0, FVector::ZeroVector));
	TestEqual(TEXT("destroyed: what it saw stays explored"), Match.Vision->GetVision(0, FVector::ZeroVector), EHOLCellVision::Explored);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionMatchGhostsTest, "HeroesOfLiberty.Vision.Match.Ghosts", UE::HeroesOfLiberty::Tests::VisionMatch::Flags)
bool FHOLVisionMatchGhostsTest::RunTest(const FString& Parameters)
{
	// F4 (D382, D520): the server remembers per camp the enemy's buildings seen, never its units.
	using namespace UE::HeroesOfLiberty::Tests::VisionMatch;
	FVisionWorld Match;
	if (!TestTrue(TEXT("the match's subsystems"), Match.IsReady()))
	{
		return false;
	}
	Match.Vision->Start(MakeTeams(), EHOLFogMode::Normal);
	const FVector Far(-30000., 0., 0.);
	const FVector HousePlace(30000., 0., 0.);
	AHOLBuilding* House = Match.World->SpawnActor<AHOLBuilding>(HousePlace, FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("the enemy's house"), House))
	{
		return false;
	}
	House->Setup(HOLBuildingTypeNames::House, 2, 2);
	const uint64 HouseKey = HOLVisionRules::MakeGhostKey(false, House->GetUniqueID());
	const uint32 Enemy = Match.Place(Peasant, 2, HousePlace + FVector(0., 1200., 0.));
	const uint32 Scout = Match.Place(Peasant, 0, HousePlace + FVector(-1600., 0., 0.));
	if (!TestNotEqual(TEXT("the scout"), Scout, 0u) || !TestNotEqual(TEXT("the enemy's Peasant"), Enemy, 0u))
	{
		return false;
	}
	Match.Vision->Update();
	const TMap<uint64, FHOLGhost>* Seen = Match.Vision->GetGhosts(0);
	if (!TestNotNull(TEXT("its camp's memory"), Seen))
	{
		return false;
	}
	TestTrue(TEXT("the house seen: remembered"), Seen->Contains(HouseKey));
	TestEqual(TEXT("only the house (no unit, no source here)"), Seen->Num(), 1);
	TestTrue(TEXT("its ally remembers it too"), Match.Vision->GetGhosts(1) && Match.Vision->GetGhosts(1)->Contains(HouseKey));

	Match.Move(Scout, Far);
	Match.Vision->Update();
	House->Destroy();
	Match.Vision->Update();
	TestTrue(TEXT("destroyed out of sight: still remembered"), Match.Vision->GetGhosts(0)->Contains(HouseKey));

	Match.Move(Scout, HousePlace + FVector(-1600., 0., 0.));
	Match.Vision->Update();
	TestFalse(TEXT("back there: forgotten"), Match.Vision->GetGhosts(0)->Contains(HouseKey));

	Match.Vision->Start(MakeTeams(), EHOLFogMode::Disabled);
	TestNull(TEXT("no fog: no memory"), Match.Vision->GetGhosts(0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionMatchCampsSeeingTest, "HeroesOfLiberty.Vision.Match.CampsSeeing", UE::HeroesOfLiberty::Tests::VisionMatch::Flags)
bool FHOLVisionMatchCampsSeeingTest::RunTest(const FString& Parameters)
{
	// F6c (D518): the camps that see a building, the set the server sends it to (an Iris group, UHOLFogNetSubsystem).
	using namespace UE::HeroesOfLiberty::Tests::VisionMatch;
	FVisionWorld Match;
	if (!TestTrue(TEXT("the match's subsystems"), Match.IsReady()))
	{
		return false;
	}
	Match.Vision->Start(MakeTeams(), EHOLFogMode::Normal);
	const FVector HousePlace(30000., 0., 0.);
	AHOLBuilding* House = Match.World->SpawnActor<AHOLBuilding>(HousePlace, FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("the enemy's house"), House))
	{
		return false;
	}
	House->Setup(HOLBuildingTypeNames::House, 2, 2);
	const FHOLBuildingTypeRow* Row = House->GetType();
	if (!TestNotNull(TEXT("its row"), Row))
	{
		return false;
	}
	const FVector2D Half = 0.5 * Row->Footprint;
	const uint32 Owner = 1u << Match.Vision->GetCampIndex(2);
	const uint32 Team = 1u << Match.Vision->GetCampIndex(0);
	Match.Vision->Update();
	TestEqual(TEXT("seen by its owner's camp only"), Match.Vision->GetCampsSeeing(HousePlace, Half), Owner);
	const uint32 Scout = Match.Place(Peasant, 0, HousePlace + FVector(-1600., 0., 0.));
	if (!TestNotEqual(TEXT("the scout"), Scout, 0u))
	{
		return false;
	}
	Match.Vision->Update();
	TestEqual(TEXT("a scout beside it: seen by both"), Match.Vision->GetCampsSeeing(HousePlace, Half), Owner | Team);
	Match.Move(Scout, FVector(-30000., 0., 0.));
	Match.Vision->Update();
	TestEqual(TEXT("the scout gone: its owner's again"), Match.Vision->GetCampsSeeing(HousePlace, Half), Owner);
	Match.Vision->Start(MakeTeams(), EHOLFogMode::Disabled);
	TestEqual(TEXT("no fog: every camp"), Match.Vision->GetCampsSeeing(HousePlace, Half), HOLVisionRules::AllCamps);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionMatchOrdersTest, "HeroesOfLiberty.Vision.Match.Orders", UE::HeroesOfLiberty::Tests::VisionMatch::Flags)
bool FHOLVisionMatchOrdersTest::RunTest(const FString& Parameters)
{
	// F5 (FogInProgress.md point 3): a player's attack on what its camp does not see is refused (a modified client, a ghost).
	using namespace UE::HeroesOfLiberty::Tests::VisionMatch;
	FVisionWorld Match;
	UHOLCommandSubsystem* Commands = Match.World->GetSubsystem<UHOLCommandSubsystem>();
	if (!TestTrue(TEXT("the match's subsystems"), Match.IsReady() && Commands != nullptr))
	{
		return false;
	}
	Match.Vision->Start(MakeTeams(), EHOLFogMode::Normal);
	const FVector Far(30000., 0., 0.);
	const uint32 Mine = Match.Place(Peasant, 0, FVector::ZeroVector);
	const uint32 Enemy = Match.Place(Peasant, 2, Far);
	AHOLBuilding* House = Match.World->SpawnActor<AHOLBuilding>(Far + FVector(0., 2000., 0.), FRotator::ZeroRotator);
	if (!TestTrue(TEXT("the units and the house"), Mine != 0 && Enemy != 0 && House != nullptr))
	{
		return false;
	}
	House->Setup(HOLBuildingTypeNames::House, 2, 2);
	Match.Vision->Update();
	const uint32 Attackers[] = { Mine };

	const int32 Before = Commands->GetNumPendingOrders();
	Commands->IssueCombatOrder(Attackers, EHOLOrderKind::Attack, Enemy, 0);
	TestEqual(TEXT("a hidden unit: the attack refused"), Commands->GetNumPendingOrders(), Before);
	Commands->IssueAttackBuildingOrder(Attackers, House, 0);
	TestEqual(TEXT("a hidden building: the attack refused"), Commands->GetNumPendingOrders(), Before);
	Commands->IssueCombatOrder(Attackers, EHOLOrderKind::Attack, Enemy, INDEX_NONE);
	TestEqual(TEXT("the game's own order (no slot): taken"), Commands->GetNumPendingOrders(), Before + 1);

	Match.Move(Mine, Far - FVector(1600., 0., 0.));
	Match.Vision->Update();
	Commands->IssueCombatOrder(Attackers, EHOLOrderKind::Attack, Enemy, 0);
	TestEqual(TEXT("in sight: the attack taken"), Commands->GetNumPendingOrders(), Before + 2);
	Commands->IssueAttackBuildingOrder(Attackers, House, 0);
	TestEqual(TEXT("the building in sight: taken"), Commands->GetNumPendingOrders(), Before + 3);
	return true;
}

namespace UE::HeroesOfLiberty::Tests::VisionMatch
{
	/**
	 * Two enemy Archers 18 m apart (in each other's reach), a wall of trees between them, through the combat processor for
	 * up to 10 s with the fog of Mode: whether both end up shooting.
	 */
	bool ArchersShoot(FAutomationTestBase& Test, const EHOLFogMode Mode)
	{
		FVisionWorld Match;
		UHOLSimulationSubsystem* Simulation = Match.World->GetSubsystem<UHOLSimulationSubsystem>();
		if (!Test.TestTrue(TEXT("the match's subsystems"), Match.IsReady() && Simulation != nullptr))
		{
			return false;
		}
		Match.Vision->Start(MakeTeams(), Mode);
		const FIntPoint Wall = Match.Vision->GetMap().GetGrid().ToCell(FVector2D(900., 0.));
		for (int32 Row = -12; Row <= 12; ++Row)
		{
			for (const double Offset : { -100., 0., 100. })
			{
				Match.Sources->AddSource(HOLSourceTypeNames::Tree, Match.Centre(FIntPoint(Wall.X, Wall.Y + Row)) + FVector(Offset, Offset, 0.));
			}
		}
		const uint32 Mine = Match.Place(TEXT("Archer"), 0, FVector::ZeroVector, true);
		const uint32 Theirs = Match.Place(TEXT("Archer"), 2, FVector(1800., 0., 0.), true);
		if (!Test.TestTrue(TEXT("two Archers searching each step"), Mine != 0 && Theirs != 0))
		{
			return false;
		}
		TSharedPtr<FMassEntityManager> EntityManager = Match.Entities->GetMutableEntityManager().AsShared();
		UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
		Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		Match.Vision->Update();
		for (int64 Step = 1; Step <= FMath::CeilToInt(10.f / StepSeconds); ++Step)
		{
			Simulation->GetOnStepStarted().Broadcast(Step);
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			Match.Units->FlushDestructions();
			Match.Vision->Update();
			FHOLUnitView Mine1;
			FHOLUnitView Theirs1;
			if (Match.Units->GetUnitView(Mine, Mine1) && Match.Units->GetUnitView(Theirs, Theirs1)
				&& Mine1.Activity == EHOLUnitActivity::Attacking && Theirs1.Activity == EHOLUnitActivity::Attacking)
			{
				return true;
			}
		}
		return false;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionMatchAutoTargetTest, "HeroesOfLiberty.Vision.Match.AutoTarget", UE::HeroesOfLiberty::Tests::VisionMatch::Flags)
bool FHOLVisionMatchAutoTargetTest::RunTest(const FString& Parameters)
{
	// F5: a unit takes on by itself only what its camp sees (two Archers in reach, a wood between them).
	using namespace UE::HeroesOfLiberty::Tests::VisionMatch;
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	if (!TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled))
	{
		return false;
	}
	const int32 FormerEnabled = CombatEnabled->GetInt();
	CombatEnabled->Set(1, ECVF_SetByCode);
	TestTrue(TEXT("no fog: they shoot each other"), ArchersShoot(*this, EHOLFogMode::Disabled));
	TestFalse(TEXT("the fog: the wood hides them, nobody shoots"), ArchersShoot(*this, EHOLFogMode::Normal));
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionMatchAISourcesTest, "HeroesOfLiberty.Vision.Match.AISources", UE::HeroesOfLiberty::Tests::VisionMatch::Flags)
bool FHOLVisionMatchAISourcesTest::RunTest(const FString& Parameters)
{
	// F-Q11 = B: the AI's economy knows only the sources of the cells its camp explored, as a player (D519).
	using namespace UE::HeroesOfLiberty::Tests::VisionMatch;
	FVisionWorld Match;
	if (!TestTrue(TEXT("the match's subsystems"), Match.IsReady()))
	{
		return false;
	}
	AHOLBuilding* Centre = Match.World->SpawnActor<AHOLBuilding>(FVector::ZeroVector, FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("the AI's centre"), Centre))
	{
		return false;
	}
	Centre->Setup(HOLBuildingTypeNames::MainCentre, 2, 2);
	const int32 Near = Match.Sources->AddSource(HOLSourceTypeNames::Tree, FVector(3000., 0., 0.));
	const int32 Far = Match.Sources->AddSource(HOLSourceTypeNames::Tree, FVector(30000., 0., 0.));
	const auto Knows = [&Match](const int32 SourceIndex)
	{
		const FHOLAISummary Summary = HOLAISummary::Read(*Match.World, 2);
		return Summary.Wood.Sources.ContainsByPredicate([SourceIndex](const FHOLAISourceNear& Source) { return Source.SourceIndex == SourceIndex; });
	};

	Match.Vision->Start(MakeTeams(), EHOLFogMode::Disabled);
	TestTrue(TEXT("no fog: the near tree known"), Knows(Near));
	TestTrue(TEXT("no fog: the far tree known"), Knows(Far));

	Match.Vision->Start(MakeTeams(), EHOLFogMode::Normal);
	Match.Vision->Update();
	TestTrue(TEXT("the fog: the tree its centre sees known"), Knows(Near));
	TestFalse(TEXT("the fog: the tree never seen unknown"), Knows(Far));

	Match.Vision->Start(MakeTeams(), EHOLFogMode::Explored);
	Match.Vision->Update();
	TestTrue(TEXT("explored map: the far tree known"), Knows(Far));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionMatchForumSightTest, "HeroesOfLiberty.Vision.Match.ForumSight", UE::HeroesOfLiberty::Tests::VisionMatch::Flags)
bool FHOLVisionMatchForumSightTest::RunTest(const FString& Parameters)
{
	// F6e.4 (D353): who has the vision of a slot's main forum, as the forum's alert asks it (AHOLGameState::DoesViewerSeeForum).
	using namespace UE::HeroesOfLiberty::Tests::VisionMatch;
	FVisionWorld Match;
	if (!TestTrue(TEXT("the match's subsystems"), Match.IsReady()))
	{
		return false;
	}
	AHOLStartSlot* Start = Match.World->SpawnActor<AHOLStartSlot>(FVector(30000., 0., 0.), FRotator::ZeroRotator);
	AHOLGameState* State = Match.World->SpawnActor<AHOLGameState>();
	if (!TestNotNull(TEXT("the start of slot 0"), Start) || !TestNotNull(TEXT("the game state"), State))
	{
		return false;
	}
	Start->SetSlotIndex(0);
	const FVector Forum = Start->GetMainCentreLocation();
	Match.Vision->Start(MakeTeams(), EHOLFogMode::Normal);
	Match.Vision->Update();
	TestFalse(TEXT("nobody near: the adversary does not see it"), State->DoesViewerSeeForum(2, 0));
	TestFalse(TEXT("a spectator never does"), State->DoesViewerSeeForum(INDEX_NONE, 0));

	const uint32 Scout = Match.Place(Peasant, 2, Forum + FVector(-1600., 0., 0.));
	if (!TestNotEqual(TEXT("the adversary's scout"), Scout, 0u))
	{
		return false;
	}
	Match.Vision->Update();
	TestTrue(TEXT("a scout beside it: its camp sees it"), State->DoesViewerSeeForum(2, 0));
	TestFalse(TEXT("the other camp still does not"), State->DoesViewerSeeForum(0, 0));
	Match.Move(Scout, FVector(-30000., 0., 0.));
	Match.Vision->Update();
	TestFalse(TEXT("the scout gone: no more (the cells stay explored, not in sight)"), State->DoesViewerSeeForum(2, 0));

	// Without a fog the alerts stay those of the camps.
	Match.Vision->Start(MakeTeams(), EHOLFogMode::Disabled);
	TestFalse(TEXT("no fog: no vision to count"), State->DoesViewerSeeForum(2, 0));
	return true;
}

#endif
