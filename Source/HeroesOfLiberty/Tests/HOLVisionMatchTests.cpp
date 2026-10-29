// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Map/HOLVisionSubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

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

		uint32 Place(const FName Type, const int32 Owner, const FVector& Location) const
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

#endif
