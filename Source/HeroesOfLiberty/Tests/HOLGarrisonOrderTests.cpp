// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Match/HOLGameState.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationProcessors.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

#if WITH_DEV_AUTOMATION_TESTS

// F7 lot 2 (FortificationsInProgress.md, D693, D374, D793): the garrison order through the REAL chain: the player's order queued by the command
// subsystem, applied at the next step's start by the garrison, read back through the selection's own list (HOLUnitView::Gather) and the
// population count. No tower row exists in BuildingTypes.csv (GarrisonSlots 0 everywhere): the test adds a row of its own (the House's, with 2
// places) to the loaded table for its run, and takes it out again.
namespace UE::HeroesOfLiberty::Tests::GarrisonOrder
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	const FName TestTowerType(TEXT("GarrisonTestTower"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGarrisonOrderTest, "HeroesOfLiberty.Garrison.Orders", UE::HeroesOfLiberty::Tests::GarrisonOrder::Flags)

bool FHOLGarrisonOrderTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::GarrisonOrder;
	UDataTable* Table = const_cast<UDataTable*>(GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous());
	const FHOLBuildingTypeRow* HouseRow = Table ? Table->FindRow<FHOLBuildingTypeRow>(HOLBuildingTypeNames::House, TEXT("garrison test")) : nullptr;
	if (!TestNotNull(TEXT("the building type table and its House row"), HouseRow))
	{
		return false;
	}
	FHOLBuildingTypeRow TowerRow = *HouseRow;
	TowerRow.GarrisonSlots = 2;
	Table->AddRow(TestTowerType, TowerRow);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	AHOLBuilding* Tower = World->SpawnActor<AHOLBuilding>(FVector(3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* House = World->SpawnActor<AHOLBuilding>(FVector(-3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("commands"), Commands)
		&& TestNotNull(TEXT("tower"), Tower) && TestNotNull(TEXT("house"), House))
	{
		Tower->Setup(TestTowerType, 0, 0);
		House->Setup(HOLBuildingTypeNames::House, 0, 0);
		const auto RunStep = [Simulation, Units]
		{
			const int64 Step = Simulation->AdvanceStepIndexForTests();
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		const auto Place = [Units](const TCHAR* Type, const int32 Owner, const FVector& Location)
		{
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = FName(Type);
			Request.Count = 1;
			Request.OwnerPlayer = static_cast<uint8>(Owner);
			Request.Center = Location;
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			TArray<uint32> After;
			Units->GetAllUnitIds(After);
			uint32 Born = 0;
			for (const uint32 Id : After)
			{
				Born = Before.Contains(Id) ? Born : Id;
			}
			return Born;
		};
		// What the selection can pick (the same list the box and the click read).
		const auto Selectable = [World](const uint32 Id)
		{
			TArray<FHOLUnitView> Views;
			HOLUnitView::Gather(*World, Views);
			return Views.ContainsByPredicate([Id](const FHOLUnitView& View) { return View.UnitId == Id; });
		};
		const auto PopulationOf = [Units](const int32 Slot)
		{
			TArray<int32> Population;
			Units->GetPopulationByOwner(Population);
			return Population.IsValidIndex(Slot) ? Population[Slot] : 0;
		};

		const uint32 Engine = Place(TEXT("Mangonel"), 0, FVector(2000.0, 0.0, 0.0));
		const uint32 Rider = Place(TEXT("LightCavalry"), 0, FVector(2000.0, 600.0, 0.0));
		const uint32 First = Place(TEXT("Spearman"), 0, FVector(2000.0, -600.0, 0.0));
		const uint32 Second = Place(TEXT("Spearman"), 0, FVector(2000.0, 1200.0, 0.0));
		const uint32 Foe = Place(TEXT("Spearman"), 3, FVector(4000.0, 0.0, 0.0));
		if (TestTrue(TEXT("the five units born"), Engine && Rider && First && Second && Foe))
		{
			TestTrue(TEXT("control: the four of slot 0 are selectable before the order"), Selectable(Engine) && Selectable(Rider) && Selectable(First)
				&& Selectable(Second));
			const int32 Population = PopulationOf(0);

			// The tower's 2 places: the engine refused (siege never, D793) takes none, the rider enters (cavalry, D793 Q2 B), the first
			// Spearman takes the last place, the second finds none; the foe (slot 3) is refused (D374: only the building's own slot).
			const uint32 Asked[] = { Engine, Rider, First, Second, Foe };
			Commands->IssueGarrisonOrder(0, Asked, Tower);
			RunStep();
			TArray<uint32> Inside;
			Units->GetGarrisonUnitIds(Tower, Inside);
			TestEqual(TEXT("two units in the tower of 2 places"), Inside.Num(), 2);
			TestFalse(TEXT("the rider garrisoned leaves the selection (D793: cavalry enters)"), Selectable(Rider));
			TestFalse(TEXT("the first Spearman garrisoned leaves the selection"), Selectable(First));
			TestTrue(TEXT("the siege engine stays on the field (D793: siege never enters)"), Selectable(Engine));
			TestTrue(TEXT("the second Spearman finds no place and stays on the field"), Selectable(Second));
			TestTrue(TEXT("the foe stays on the field"), Selectable(Foe));
			TestEqual(TEXT("the garrisoned units keep their population places"), PopulationOf(0), Population);

			// An enemy's order on the tower: refused, the foe stays out.
			const uint32 FoeAsks[] = { Foe };
			Commands->IssueGarrisonOrder(3, FoeAsks, Tower);
			RunStep();
			TestTrue(TEXT("an enemy cannot garrison the tower"), Selectable(Foe) && Units->GetGarrison(Foe) == nullptr);

			// A House (GarrisonSlots 0 in the real table) takes nobody.
			const uint32 SecondAsks[] = { Second };
			Commands->IssueGarrisonOrder(0, SecondAsks, House);
			RunStep();
			TestTrue(TEXT("a House (0 places) takes nobody"), Selectable(Second) && Units->GetGarrison(Second) == nullptr);

			// Exit all: every occupant back on the field and selectable.
			Commands->IssueUngarrisonAllOrder(0, Tower);
			RunStep();
			Units->GetGarrisonUnitIds(Tower, Inside);
			TestEqual(TEXT("the tower is empty after exit all"), Inside.Num(), 0);
			TestTrue(TEXT("exit all puts the rider and the Spearman back in the selection"), Selectable(Rider) && Selectable(First));
			TestEqual(TEXT("the population is unchanged after exit all"), PopulationOf(0), Population);
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	Table->RemoveRow(TestTowerType);
	return true;
}

// F7 lot 3a (FortificationsInProgress.md, D693): a garrisoned unit's body is off the field. Through the REAL processors of the step: the keep-out
// (UHOLFootprintKeepOutProcessor) and the combat (UHOLCombatProcessor) run on the world's entities. The garrisoned body stands inside its
// building's body and the keep-out leaves it there (a control unit at a House's centre is pushed out by the same run); an enemy Archer that
// had taken it as its target lets it go and its health does not move.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGarrisonOffFieldTest, "HeroesOfLiberty.Garrison.OffField", UE::HeroesOfLiberty::Tests::GarrisonOrder::Flags)

bool FHOLGarrisonOffFieldTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::GarrisonOrder;
	UDataTable* Table = const_cast<UDataTable*>(GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous());
	const FHOLBuildingTypeRow* HouseRow = Table ? Table->FindRow<FHOLBuildingTypeRow>(HOLBuildingTypeNames::House, TEXT("garrison test")) : nullptr;
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	if (!TestNotNull(TEXT("the building type table and its House row"), HouseRow) || !TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled))
	{
		return false;
	}
	const int32 FormerCombat = CombatEnabled->GetInt();
	CombatEnabled->Set(1, ECVF_SetByCode);
	FHOLBuildingTypeRow TowerRow = *HouseRow;
	TowerRow.GarrisonSlots = 2;
	Table->AddRow(TestTowerType, TowerRow);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	AHOLBuilding* Tower = World->SpawnActor<AHOLBuilding>(FVector(3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* House = World->SpawnActor<AHOLBuilding>(FVector(-3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("commands"), Commands)
		&& TestNotNull(TEXT("entities"), EntitySubsystem) && TestNotNull(TEXT("tower"), Tower) && TestNotNull(TEXT("house"), House))
	{
		Tower->Setup(TestTowerType, 0, 0);
		House->Setup(HOLBuildingTypeNames::House, 0, 0);
		const TSharedRef<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLFootprintKeepOutProcessor* KeepOut = NewObject<UHOLFootprintKeepOutProcessor>(Simulation);
		KeepOut->CallInitialize(Simulation, EntityManager);
		UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
		Combat->CallInitialize(Simulation, EntityManager);
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		const auto RunStep = [Simulation, Units, KeepOut, Combat, EntityManager, StepSeconds]
		{
			Simulation->GetOnStepStarted().Broadcast(Simulation->GetStepIndex());
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*KeepOut, ProcessingContext);
			FMassProcessingContext CombatContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Combat, CombatContext);
			Units->FlushDestructions();
			const int64 Step = Simulation->AdvanceStepIndexForTests();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		const auto Place = [Units](const TCHAR* Type, const int32 Owner, const FVector& Location)
		{
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = FName(Type);
			Request.Count = 1;
			Request.OwnerPlayer = static_cast<uint8>(Owner);
			Request.Center = Location;
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			TArray<uint32> After;
			Units->GetAllUnitIds(After);
			uint32 Born = 0;
			for (const uint32 Id : After)
			{
				Born = Before.Contains(Id) ? Born : Id;
			}
			return Born;
		};
		const auto LocationOf = [Units, EntityManager](const uint32 Id)
		{
			const FMassEntityHandle Entity = Units->FindEntity(Id);
			const FTransformFragment* Transform = Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FTransformFragment>(Entity) : nullptr;
			return Transform ? Transform->GetTransform().GetLocation() : FVector(1.0e9);
		};
		const auto TargetOf = [Units, EntityManager](const uint32 Id)
		{
			const FMassEntityHandle Entity = Units->FindEntity(Id);
			const FHOLCombatFragment* Fragment = Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Entity) : nullptr;
			return Fragment ? Fragment->Target : FMassEntityHandle();
		};
		const auto HealthOf = [Units, EntityManager](const uint32 Id)
		{
			const FMassEntityHandle Entity = Units->FindEntity(Id);
			const FHOLUnitStateFragment* State = Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
			return State ? State->CurrentHealth : -1.f;
		};
		// Inside the building's body (the keep-out's own rectangle, D324), the building unturned.
		const auto InsideBody = [](const AHOLBuilding& Building, const FVector& Location)
		{
			const FHOLBuildingTypeRow* Type = Building.GetType();
			if (!Type)
			{
				return false;
			}
			const FVector2D Half = UHOLBuildingSettings::GetBodySize(Type->Footprint) * 0.5;
			const FVector2D Local = FVector2D(Location) - FVector2D(Building.GetActorLocation());
			return FMath::Abs(Local.X) <= Half.X && FMath::Abs(Local.Y) <= Half.Y;
		};

		const uint32 Ally = Place(TEXT("Spearman"), 0, FVector(2300.0, 0.0, 0.0));
		const uint32 Control = Place(TEXT("Spearman"), 0, FVector(-3000.0, 0.0, 0.0));
		const uint32 Foe = Place(TEXT("Archer"), 3, FVector(2000.0, 0.0, 0.0));
		if (TestTrue(TEXT("the three units born"), Ally && Control && Foe))
		{
			const FMassEntityHandle AllyEntity = Units->FindEntity(Ally);
			for (int32 Step = 0; Step < 20 && TargetOf(Foe) != AllyEntity; ++Step)
			{
				RunStep();
			}
			TestFalse(TEXT("control: the keep-out pushes a unit standing at a House's centre out of its body"), InsideBody(*House, LocationOf(Control)));
			TestTrue(TEXT("control: the enemy Archer takes the Spearman as its target before the garrison"), TargetOf(Foe) == AllyEntity);

			const uint32 Asked[] = { Ally };
			Commands->IssueGarrisonOrder(0, Asked, Tower);
			RunStep();
			TestTrue(TEXT("the Spearman is in the tower"), Units->GetGarrison(Ally) == Tower);
			const float Health = HealthOf(Ally);
			for (int32 Step = 0; Step < 20; ++Step)
			{
				RunStep();
			}
			TestTrue(TEXT("the garrisoned body stands inside the tower's body and the keep-out leaves it there"), InsideBody(*Tower, LocationOf(Ally)));
			TestTrue(TEXT("the enemy Archer lets the garrisoned Spearman go"), TargetOf(Foe) != AllyEntity);
			TestEqual(TEXT("the garrisoned Spearman's health does not move"), HealthOf(Ally), Health);
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	Table->RemoveRow(TestTowerType);
	CombatEnabled->Set(FormerCombat, ECVF_SetByCode);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
