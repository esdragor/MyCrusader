// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Map/HOLVisionSubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLExplorationXPRules.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// B2 (ExplorationXPInProgress.md): the server's fog pays the exploration XP to the camp's hero, through the real chain (units, fog, hero sheet).
namespace UE::HeroesOfLiberty::Tests::ExplorationXP
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** Which cells camp Camp has explored, read from the fog. */
	TArray<bool> ReadExplored(const UHOLVisionSubsystem& Vision, const int32 Camp)
	{
		const FHOLVisionGrid& Grid = Vision.GetMap().GetGrid();
		TArray<bool> Explored;
		Explored.SetNum(Grid.Num());
		for (int32 Index = 0; Index < Grid.Num(); ++Index)
		{
			Explored[Index] = Vision.GetMap().GetVision(Camp, Grid.ToCell(Index)) != EHOLCellVision::Unexplored;
		}
		return Explored;
	}

	/** The tranches with a cell explored between Before and After and none explored before (counted here, from the fog alone). */
	int32 CountNewTranches(const UHOLVisionSubsystem& Vision, const TArray<bool>& Before, const TArray<bool>& After, const int32 CellsPerTranche)
	{
		const FHOLVisionGrid& Grid = Vision.GetMap().GetGrid();
		const int32 PerSide = (Grid.Side + CellsPerTranche - 1) / CellsPerTranche;
		TSet<int32> Old;
		TSet<int32> New;
		for (int32 Index = 0; Index < Grid.Num(); ++Index)
		{
			const FIntPoint Cell = Grid.ToCell(Index);
			const int32 Tranche = (Cell.Y / CellsPerTranche) * PerSide + Cell.X / CellsPerTranche;
			if (Before[Index])
			{
				Old.Add(Tranche);
			}
			else if (After[Index])
			{
				New.Add(Tranche);
			}
		}
		return New.Difference(Old).Num();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLExplorationXPFogPaysTest, "HeroesOfLiberty.ExplorationXP.FogPaysTheHero", UE::HeroesOfLiberty::Tests::ExplorationXP::Flags)
bool FHOLExplorationXPFogPaysTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ExplorationXP;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	ON_SCOPE_EXIT
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	};
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSourceSubsystem* Sources = World->GetSubsystem<UHOLSourceSubsystem>();
	UHOLVisionSubsystem* Vision = World->GetSubsystem<UHOLVisionSubsystem>();
	UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>();
	UMassEntitySubsystem* Entities = World->GetSubsystem<UMassEntitySubsystem>();
	if (!TestTrue(TEXT("the match's subsystems"), Units && Sources && Vision && Heroes && Entities))
	{
		return false;
	}
	Sources->OnWorldBeginPlay(*World);

	// Slot 0 against slot 1, a camp each.
	FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
	for (FHOLMatchSlot& Slot : Config.Slots)
	{
		Slot.State = EHOLSlotState::Closed;
		Slot.Team = 0;
	}
	Config.Slots[0].State = EHOLSlotState::Human;
	Config.Slots[0].Team = 1;
	Config.Slots[1].State = EHOLSlotState::AI;
	Config.Slots[1].Team = 2;
	Vision->Start(Config, EHOLFogMode::Normal);
	const int32 Camp = Vision->GetCampIndex(0);

	FHOLUnitSpawnRequest Request;
	Request.TypeName = FName(TEXT("Peasant"));
	Request.Count = 1;
	Request.OwnerPlayer = 0;
	Request.Center = FVector::ZeroVector;
	Units->RequestUnits(Request);
	Units->ServeBirthRequests();
	TArray<uint32> Ids;
	Units->GetAllUnitIds(Ids);
	if (!TestEqual(TEXT("a Peasant born"), Ids.Num(), 1))
	{
		return false;
	}
	const auto MoveScout = [&](const FVector& Location)
	{
		if (FTransformFragment* Transform = Entities->GetMutableEntityManager().GetFragmentDataPtr<FTransformFragment>(Units->FindEntity(Ids[0])))
		{
			Transform->GetMutableTransform().SetLocation(Location);
		}
	};
	const auto XP = [&] { return Heroes->GetSheets(0).Private.XP; };

	// The first second: the start's sight pays nothing.
	const int32 Rate = FMath::Max(1, GetDefault<UHOLSimulationSettings>()->SimulationRateHz);
	for (int32 Step = 0; Step < Rate; ++Step)
	{
		Vision->Update();
	}
	TestTrue(TEXT("the start's sight is explored"), Vision->GetMap().CountExplored(Camp) > 0);
	TestEqual(TEXT("the start's sight pays nothing"), XP(), 0);

	// Far away: N new tranches, N payments of the preset's tranche.
	const HOLExplorationXP::FSettings Settings = HOLExplorationXP::Preset(HOLExplorationXP::GetPresetIndex());
	const FVector Far(Vision->GetMap().GetGrid().GetCenter(Vision->GetMap().GetGrid().ToCell(FVector2D(20000., 12000.))), 0.);
	const TArray<bool> Before = ReadExplored(*Vision, Camp);
	MoveScout(Far);
	Vision->Update();
	const int32 NewTranches = CountNewTranches(*Vision, Before, ReadExplored(*Vision, Camp), Settings.CellsPerTranche);
	TestTrue(TEXT("new tranches explored"), NewTranches > 0);
	const int32 Paid = XP();
	TestEqual(TEXT("N new tranches: N payments (Exploration source, final XP)"), Paid, NewTranches * Settings.XPPerTranche);

	// Back home and to the same place again: those tranches already paid.
	MoveScout(FVector::ZeroVector);
	Vision->Update();
	MoveScout(Far);
	Vision->Update();
	TestEqual(TEXT("the same tranches a second time: nothing more"), XP(), Paid);
	TestEqual(TEXT("the other camp's hero: nothing"), Heroes->GetSheets(1).Private.XP, 0);
	return true;
}

#endif
