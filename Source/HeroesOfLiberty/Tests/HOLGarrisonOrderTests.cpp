// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLPlayerOrders.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassNavigationFragments.h"
#include "MassProcessingContext.h"
#include "Match/HOLGameState.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLHealProcessor.h"
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

// F7 lot 3a, the rest (FortificationsInProgress.md, D693): the separation and the heal of the step leave a garrisoned body out, through their REAL
// processors (UHOLUnitSeparationProcessor, UHOLHealProcessor) run on the world's entities. Separation: two overlapping units on the field are pushed
// apart by the run (control), two garrisoned bodies in the tower are not. Heal (Retours/Q-garrison-soin.md, default A = hol.Garrison.HealInside 0):
// a Monk within range of the tower's centre heals a wounded Spearman on the field (control) and not the wounded garrisoned one; with the option B
// (hol.Garrison.HealInside 1) it heals the garrisoned one.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGarrisonSeparationHealTest, "HeroesOfLiberty.Garrison.SeparationAndHeal", UE::HeroesOfLiberty::Tests::GarrisonOrder::Flags)

bool FHOLGarrisonSeparationHealTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::GarrisonOrder;
	UDataTable* Table = const_cast<UDataTable*>(GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous());
	const FHOLBuildingTypeRow* HouseRow = Table ? Table->FindRow<FHOLBuildingTypeRow>(HOLBuildingTypeNames::House, TEXT("garrison test")) : nullptr;
	IConsoleVariable* HealInside = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Garrison.HealInside"));
	if (!TestNotNull(TEXT("the building type table and its House row"), HouseRow) || !TestNotNull(TEXT("hol.Garrison.HealInside"), HealInside))
	{
		return false;
	}
	const int32 FormerHealInside = HealInside->GetInt();
	HealInside->Set(0, ECVF_SetByCode);
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
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("commands"), Commands)
		&& TestNotNull(TEXT("entities"), EntitySubsystem) && TestNotNull(TEXT("tower"), Tower))
	{
		Tower->Setup(TestTowerType, 0, 0);
		const TSharedRef<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLUnitSeparationProcessor* Separation = NewObject<UHOLUnitSeparationProcessor>(Simulation);
		Separation->CallInitialize(Simulation, EntityManager);
		UHOLHealProcessor* Heal = NewObject<UHOLHealProcessor>(Simulation);
		Heal->CallInitialize(Simulation, EntityManager);
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		const auto RunStep = [Simulation, Units, Separation, Heal, EntityManager, StepSeconds]
		{
			Simulation->GetOnStepStarted().Broadcast(Simulation->GetStepIndex());
			Units->ServeBirthRequests();
			FMassProcessingContext SeparationContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Separation, SeparationContext);
			FMassProcessingContext HealContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Heal, HealContext);
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
		const auto TransformOf = [Units, EntityManager](const uint32 Id) -> FTransformFragment*
		{
			const FMassEntityHandle Entity = Units->FindEntity(Id);
			return Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FTransformFragment>(Entity) : nullptr;
		};
		const auto StateOf = [Units, EntityManager](const uint32 Id) -> FHOLUnitStateFragment*
		{
			const FMassEntityHandle Entity = Units->FindEntity(Id);
			return Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
		};
		const auto Distance2D = [&TransformOf](const uint32 A, const uint32 B)
		{
			const FTransformFragment* TA = TransformOf(A);
			const FTransformFragment* TB = TransformOf(B);
			return TA && TB ? FVector2D::Distance(FVector2D(TA->GetTransform().GetLocation()), FVector2D(TB->GetTransform().GetLocation())) : -1.0;
		};
		const auto MoveTo = [&TransformOf](const uint32 Id, const FVector& Location)
		{
			if (FTransformFragment* Transform = TransformOf(Id))
			{
				Transform->GetMutableTransform().SetLocation(FVector(Location.X, Location.Y, Transform->GetTransform().GetLocation().Z));
			}
		};

		const uint32 FieldA = Place(TEXT("Spearman"), 0, FVector(0.0, 3000.0, 0.0));
		const uint32 FieldB = Place(TEXT("Spearman"), 0, FVector(0.0, 3000.0, 0.0));
		const uint32 InA = Place(TEXT("Spearman"), 0, FVector(2000.0, 600.0, 0.0));
		const uint32 InB = Place(TEXT("Spearman"), 0, FVector(2000.0, -600.0, 0.0));
		if (TestTrue(TEXT("the four Spearmen born"), FieldA && FieldB && InA && InB))
		{
			// Separation: two bodies 5 cm apart on the field, the same two in the tower (the bench nudges one 5 cm off the centre).
			const uint32 Asked[] = { InA, InB };
			Commands->IssueGarrisonOrder(0, Asked, Tower);
			RunStep();
			TestTrue(TEXT("both Spearmen in the tower"), Units->GetGarrison(InA) == Tower && Units->GetGarrison(InB) == Tower);
			MoveTo(FieldA, FVector(0.0, 3000.0, 0.0));
			MoveTo(FieldB, FVector(5.0, 3000.0, 0.0));
			MoveTo(InA, Tower->GetActorLocation());
			MoveTo(InB, Tower->GetActorLocation() + FVector(5.0, 0.0, 0.0));
			// The separation only looks from a walker (a standing pair is left as it stands): one of each pair walks, so that only the garrison
			// tag can keep the two garrisoned bodies where they stand.
			for (const uint32 Walker : { FieldB, InB })
			{
				const FMassEntityHandle Entity = Units->FindEntity(Walker);
				if (FMassMoveTargetFragment* MoveTarget = Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FMassMoveTargetFragment>(Entity) : nullptr)
				{
					MoveTarget->CreateNewAction(EMassMovementAction::Move, *World);
				}
			}
			const double InsideBefore = Distance2D(InA, InB);
			for (int32 Step = 0; Step < 10; ++Step)
			{
				RunStep();
			}
			TestTrue(TEXT("control: the separation pushes two overlapping Spearmen on the field apart"), Distance2D(FieldA, FieldB) > 20.0);
			TestTrue(TEXT("the separation leaves the two garrisoned bodies where they stand"), FMath::IsNearlyEqual(Distance2D(InA, InB), InsideBefore, 0.5));

			// Heal: a Monk beside the tower, the wounded control beside it, the wounded InA in the tower.
			const uint32 Monk = Place(TEXT("Monk"), 0, Tower->GetActorLocation() + FVector(0.0, -700.0, 0.0));
			if (TestTrue(TEXT("the Monk born"), Monk != 0) && TestNotNull(TEXT("states"), StateOf(FieldA)) && TestNotNull(TEXT("state in"), StateOf(InA)))
			{
				MoveTo(Monk, Tower->GetActorLocation() + FVector(0.0, -700.0, 0.0));
				MoveTo(FieldA, Tower->GetActorLocation() + FVector(0.0, -1000.0, 0.0));
				MoveTo(FieldB, FVector(-20000.0, 0.0, 0.0));
				StateOf(FieldA)->CurrentHealth = 30.f;
				StateOf(InA)->CurrentHealth = 30.f;
				StateOf(InB)->CurrentHealth = 30.f;
				for (int32 Step = 0; Step < 40; ++Step)
				{
					RunStep();
				}
				TestTrue(TEXT("control: the Monk heals the wounded Spearman on the field"), StateOf(FieldA)->CurrentHealth > 30.f);
				TestEqual(TEXT("A (hol.Garrison.HealInside 0): the garrisoned Spearmen are not healed"), StateOf(InA)->CurrentHealth + StateOf(InB)->CurrentHealth, 60.f);

				HealInside->Set(1, ECVF_SetByCode);
				MoveTo(FieldA, FVector(-20000.0, 500.0, 0.0));
				for (int32 Step = 0; Step < 40; ++Step)
				{
					RunStep();
				}
				TestTrue(TEXT("B (hol.Garrison.HealInside 1): the Monk heals a garrisoned Spearman"), StateOf(InA)->CurrentHealth + StateOf(InB)->CurrentHealth > 60.f);
			}
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	Table->RemoveRow(TestTowerType);
	HealInside->Set(FormerHealInside, ECVF_SetByCode);
	return true;
}

// F7 lot 3b (FortificationsInProgress.md, D693): exit all puts every occupant on a free point out of the building's body (where the building's own
// births stand, UHOLProductionSubsystem::GetBirthLocation), each on its own point: through the real order and the garrison of the step, no keep-out run.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGarrisonExitPointTest, "HeroesOfLiberty.Garrison.ExitPoint", UE::HeroesOfLiberty::Tests::GarrisonOrder::Flags)

bool FHOLGarrisonExitPointTest::RunTest(const FString& Parameters)
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
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	AHOLBuilding* Tower = World->SpawnActor<AHOLBuilding>(FVector(3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("commands"), Commands)
		&& TestNotNull(TEXT("entities"), EntitySubsystem) && TestNotNull(TEXT("tower"), Tower))
	{
		Tower->Setup(TestTowerType, 0, 0);
		const TSharedRef<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
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
		const auto LocationOf = [Units, EntityManager](const uint32 Id)
		{
			const FMassEntityHandle Entity = Units->FindEntity(Id);
			const FTransformFragment* Transform = Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FTransformFragment>(Entity) : nullptr;
			return Transform ? Transform->GetTransform().GetLocation() : FVector(1.0e9);
		};
		const auto RadiusOf = [Units, EntityManager](const uint32 Id)
		{
			const FMassEntityHandle Entity = Units->FindEntity(Id);
			const FAgentRadiusFragment* Radius = Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FAgentRadiusFragment>(Entity) : nullptr;
			return Radius ? static_cast<double>(Radius->Radius) : 1.0e9;
		};
		const auto InsideBody = [](const AHOLBuilding& Building, const FVector& Location)
		{
			const FHOLBuildingTypeRow* Type = Building.GetType();
			if (!Type)
			{
				return true;
			}
			const FVector2D Half = UHOLBuildingSettings::GetBodySize(Type->Footprint) * 0.5;
			const FVector2D Local = FVector2D(Location) - FVector2D(Building.GetActorLocation());
			return FMath::Abs(Local.X) <= Half.X && FMath::Abs(Local.Y) <= Half.Y;
		};

		const uint32 A = Place(TEXT("Spearman"), 0, FVector(2000.0, 600.0, 0.0));
		const uint32 B = Place(TEXT("Spearman"), 0, FVector(2000.0, -600.0, 0.0));
		if (TestTrue(TEXT("the two Spearmen born"), A && B))
		{
			const uint32 Asked[] = { A, B };
			Commands->IssueGarrisonOrder(0, Asked, Tower);
			RunStep();
			TestTrue(TEXT("control: both garrisoned bodies stand inside the tower's body"), Units->GetGarrison(A) == Tower && Units->GetGarrison(B) == Tower
				&& InsideBody(*Tower, LocationOf(A)) && InsideBody(*Tower, LocationOf(B)));

			Commands->IssueUngarrisonAllOrder(0, Tower);
			RunStep();
			TestTrue(TEXT("both back on the field"), Units->GetGarrison(A) == nullptr && Units->GetGarrison(B) == nullptr);
			TestFalse(TEXT("exit all puts the first Spearman out of the tower's body"), InsideBody(*Tower, LocationOf(A)));
			TestFalse(TEXT("exit all puts the second Spearman out of the tower's body"), InsideBody(*Tower, LocationOf(B)));
			TestTrue(TEXT("each on its own point (farther apart than their two radii)"),
				FVector2D::Distance(FVector2D(LocationOf(A)), FVector2D(LocationOf(B))) > RadiusOf(A) + RadiusOf(B));
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	Table->RemoveRow(TestTowerType);
	return true;
}

// F7 lot 3c (FortificationsInProgress.md, D693): a garrisoned building destroyed (its health brought to 0 through the real damage of the server,
// UHOLConstructionSubsystem::DamageBuilding) ejects its occupants: back on the field, each out of the fallen body on its own point, health unchanged.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGarrisonEjectionTest, "HeroesOfLiberty.Garrison.Ejection", UE::HeroesOfLiberty::Tests::GarrisonOrder::Flags)

bool FHOLGarrisonEjectionTest::RunTest(const FString& Parameters)
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
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	AHOLBuilding* Tower = World->SpawnActor<AHOLBuilding>(FVector(3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("commands"), Commands)
		&& TestNotNull(TEXT("entities"), EntitySubsystem) && TestNotNull(TEXT("tower"), Tower))
	{
		Tower->Setup(TestTowerType, 0, 0);
		const TSharedRef<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
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
		const auto LocationOf = [Units, EntityManager](const uint32 Id)
		{
			const FMassEntityHandle Entity = Units->FindEntity(Id);
			const FTransformFragment* Transform = Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FTransformFragment>(Entity) : nullptr;
			return Transform ? Transform->GetTransform().GetLocation() : FVector(1.0e9);
		};
		const auto StateOf = [Units, EntityManager](const uint32 Id) -> FHOLUnitStateFragment*
		{
			const FMassEntityHandle Entity = Units->FindEntity(Id);
			return Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
		};
		const FHOLBuildingTypeRow* Type = Tower->GetType();
		const FVector2D Centre(Tower->GetActorLocation());
		const FVector2D Half = Type ? UHOLBuildingSettings::GetBodySize(Type->Footprint) * 0.5 : FVector2D(1.0e9);
		const auto InsideBody = [Centre, Half](const FVector& Location)
		{
			const FVector2D Local = FVector2D(Location) - Centre;
			return FMath::Abs(Local.X) <= Half.X && FMath::Abs(Local.Y) <= Half.Y;
		};

		const uint32 A = Place(TEXT("Spearman"), 0, FVector(2000.0, 600.0, 0.0));
		const uint32 B = Place(TEXT("Spearman"), 0, FVector(2000.0, -600.0, 0.0));
		if (TestTrue(TEXT("the two Spearmen born"), A && B) && TestNotNull(TEXT("state of A"), StateOf(A)) && TestNotNull(TEXT("state of B"), StateOf(B)))
		{
			StateOf(A)->CurrentHealth = 50.f;
			const float HealthA = StateOf(A)->CurrentHealth;
			const float HealthB = StateOf(B)->CurrentHealth;
			const uint32 Asked[] = { A, B };
			Commands->IssueGarrisonOrder(0, Asked, Tower);
			RunStep();
			TestTrue(TEXT("control: both garrisoned bodies stand inside the tower's body"), Units->GetGarrison(A) == Tower && Units->GetGarrison(B) == Tower
				&& InsideBody(LocationOf(A)) && InsideBody(LocationOf(B)));

			const int32 Dealt = UHOLConstructionSubsystem::DamageBuilding(*Tower, 1000000, EHOLAttackType::Siege);
			TestTrue(TEXT("control: the damage of the server destroys the tower"), Dealt > 0 && !IsValid(Tower));
			RunStep();
			TestTrue(TEXT("both back on the field"), Units->GetGarrison(A) == nullptr && Units->GetGarrison(B) == nullptr);
			TestFalse(TEXT("the fall ejects the first Spearman out of the tower's body"), InsideBody(LocationOf(A)));
			TestFalse(TEXT("the fall ejects the second Spearman out of the tower's body"), InsideBody(LocationOf(B)));
			TestTrue(TEXT("each on its own point"), FVector2D::Distance(FVector2D(LocationOf(A)), FVector2D(LocationOf(B))) > 1.0);
			TestEqual(TEXT("the first Spearman's health unchanged by the fall"), StateOf(A) ? StateOf(A)->CurrentHealth : -1.f, HealthA);
			TestEqual(TEXT("the second Spearman's health unchanged by the fall"), StateOf(B) ? StateOf(B)->CurrentHealth : -1.f, HealthB);
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	Table->RemoveRow(TestTowerType);
	return true;
}

// F7 lot 3d (FortificationsInProgress.md, D693, D793 Q1 = A: the occupants are sheltered): an area blow (the Mangonel's, S4, D614) that falls by
// the tower does not lower its occupants' health; a witness outside the tower, within the same circle, is hit. Through the REAL combat processor
// (UHOLCombatProcessor) and the real Mangonel row: an enemy Mangonel ordered on a Spearman standing at the tower's foot. The occupants stand at the
// tower's centre (UHOLUnitSubsystem::SetGarrison), nearer the blow than the witness (checked first): on the field they would be hit.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGarrisonShelterTest, "HeroesOfLiberty.Garrison.Shelter", UE::HeroesOfLiberty::Tests::GarrisonOrder::Flags)

bool FHOLGarrisonShelterTest::RunTest(const FString& Parameters)
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
	const FVector TowerAt(3000.0, 0.0, 0.0);
	AHOLBuilding* Tower = World->SpawnActor<AHOLBuilding>(TowerAt, FRotator::ZeroRotator);
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("commands"), Commands)
		&& TestNotNull(TEXT("entities"), EntitySubsystem) && TestNotNull(TEXT("tower"), Tower))
	{
		Tower->Setup(TestTowerType, 0, 0);
		const TSharedRef<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
		Combat->CallInitialize(Simulation, EntityManager);
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		const auto RunStep = [Simulation, Units, Combat, EntityManager, StepSeconds]
		{
			// The step index moves first: the Mangonel's blow lands when the step reaches its ArrivalStep.
			const int64 Step = Simulation->AdvanceStepIndexForTests();
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext CombatContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Combat, CombatContext);
			Units->FlushDestructions();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		// Born again until its identifier is a multiple of 4 (one search step of 4), as HeroesOfLiberty.Siege.Deploy.InMatch.
		const auto Place = [Units](const TCHAR* Type, const int32 Owner, const FVector& Location)
		{
			for (int32 Attempt = 0; Attempt < 8; ++Attempt)
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
				if (Born == 0 || Born % 4 == 0)
				{
					return Born;
				}
				const uint32 Again[] = { Born };
				Units->RequestDestroyUnits(Again);
				Units->FlushDestructions();
			}
			return 0u;
		};
		const auto LocationOf = [Units, EntityManager](const uint32 Id)
		{
			const FMassEntityHandle Entity = Units->FindEntity(Id);
			const FTransformFragment* Transform = Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FTransformFragment>(Entity) : nullptr;
			return Transform ? Transform->GetTransform().GetLocation() : FVector(1.0e9);
		};
		const auto HealthOf = [Units, EntityManager](const uint32 Id)
		{
			const FMassEntityHandle Entity = Units->FindEntity(Id);
			const FHOLUnitStateFragment* State = Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
			return State ? State->CurrentHealth : -1.f;
		};
		const auto Hold = [Units, EntityManager](const uint32 Id)
		{
			FHOLCombatFragment* Fragment = EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Units->FindEntity(Id));
			if (Fragment)
			{
				Fragment->Mode = EHOLCombatMode::Hold;
			}
			return Fragment != nullptr;
		};

		// Slot 0's tower, two occupants; the target at the tower's foot (100 cm from its centre) and the witness 224 cm from the target,
		// both of slot 0 and holding their ground; slot 3's Mangonel 20 m from the target (beyond its proposed minimum range, 1200 cm).
		const uint32 OccupantA = Place(TEXT("Spearman"), 0, FVector(3000.0, -900.0, 0.0));
		const uint32 OccupantB = Place(TEXT("Spearman"), 0, FVector(3000.0, 900.0, 0.0));
		const uint32 Target = Place(TEXT("Spearman"), 0, FVector(3100.0, 0.0, 0.0));
		const uint32 Witness = Place(TEXT("Spearman"), 0, FVector(3000.0, 200.0, 0.0));
		const uint32 Engine = Place(TEXT("Mangonel"), 3, FVector(1100.0, 0.0, 0.0));
		if (TestTrue(TEXT("the five units born"), OccupantA && OccupantB && Target && Witness && Engine) && TestTrue(TEXT("the target and the witness hold"), Hold(Target) && Hold(Witness)))
		{
			const uint32 Asked[] = { OccupantA, OccupantB };
			Commands->IssueGarrisonOrder(0, Asked, Tower);
			RunStep();
			TestTrue(TEXT("the two Spearmen are in the tower"), Units->GetGarrison(OccupantA) == Tower && Units->GetGarrison(OccupantB) == Tower);
			const FVector2D TargetAt(LocationOf(Target));
			const double WitnessGap = FVector2D::Distance(TargetAt, FVector2D(LocationOf(Witness)));
			TestTrue(TEXT("control: the first occupant stands nearer the blow than the witness (on the field it would be hit)"),
				FVector2D::Distance(TargetAt, FVector2D(LocationOf(OccupantA))) < WitnessGap);
			TestTrue(TEXT("control: the second occupant stands nearer the blow than the witness"), FVector2D::Distance(TargetAt, FVector2D(LocationOf(OccupantB))) < WitnessGap);
			const float StartA = HealthOf(OccupantA);
			const float StartB = HealthOf(OccupantB);
			const float StartTarget = HealthOf(Target);
			const float StartWitness = HealthOf(Witness);

			FHOLCombatFragment* EngineCombat = EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Units->FindEntity(Engine));
			if (TestNotNull(TEXT("the Mangonel's combat"), EngineCombat))
			{
				EngineCombat->Target = Units->FindEntity(Target);
				EngineCombat->bOrderedTarget = true;
			}
			const int32 TenSeconds = FMath::CeilToInt(10.f / StepSeconds);
			bool bHit = false;
			for (int32 Index = 0; Index < TenSeconds && !bHit; ++Index)
			{
				RunStep();
				bHit = HealthOf(Target) < StartTarget;
			}
			TestTrue(TEXT("control: the Mangonel's blow lands on the target at the tower's foot"), bHit);
			TestTrue(TEXT("the witness outside the tower, within the circle, is hit"), HealthOf(Witness) < StartWitness);
			TestEqual(TEXT("the first occupant's health does not move (sheltered)"), HealthOf(OccupantA), StartA);
			TestEqual(TEXT("the second occupant's health does not move (sheltered)"), HealthOf(OccupantB), StartB);
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	Table->RemoveRow(TestTowerType);
	CombatEnabled->Set(FormerCombat, ECVF_SetByCode);
	return true;
}

// F7 lot 4b (FortificationsInProgress.md, D693 « ordre sur un batiment », D374, D793): a right click on one of the player's own finished buildings
// with places is a garrison order (HOLPlayerOrders::TakesGarrisonOrder, read by AHOLPlayerController::OrderAt on the client and again by the
// server). Through the REAL chain (HOLPlayerOrders::Garrison, the command queue, the garrison at the next step) the Spearman ends inside; a House
// (0 place), an enemy's tower and the setting hol.Garrison.RightClickEnters 0 take nobody (the controller falls back to a move). Exit all
// (HOLPlayerOrders::UngarrisonAll, the order of the coming button, lot 4a) empties the tower, and is refused to another slot.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGarrisonRightClickTest, "HeroesOfLiberty.Garrison.RightClick", UE::HeroesOfLiberty::Tests::GarrisonOrder::Flags)

bool FHOLGarrisonRightClickTest::RunTest(const FString& Parameters)
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
	AHOLBuilding* Tower = World->SpawnActor<AHOLBuilding>(FVector(3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* House = World->SpawnActor<AHOLBuilding>(FVector(-3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* FoeTower = World->SpawnActor<AHOLBuilding>(FVector(0.0, 4000.0, 0.0), FRotator::ZeroRotator);
	IConsoleVariable* RightClick = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Garrison.RightClickEnters"));
	TestNotNull(TEXT("the setting hol.Garrison.RightClickEnters exists (Retours/Q-garrison-ui.md)"), RightClick);
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("tower"), Tower) && TestNotNull(TEXT("house"), House)
		&& TestNotNull(TEXT("foe tower"), FoeTower))
	{
		Tower->Setup(TestTowerType, 0, 0);
		House->Setup(HOLBuildingTypeNames::House, 0, 0);
		FoeTower->Setup(TestTowerType, 3, 3);
		const auto RunStep = [Simulation, Units]
		{
			const int64 Step = Simulation->AdvanceStepIndexForTests();
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		const auto Place = [Units](const int32 Owner, const FVector& Location)
		{
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = FName(TEXT("Spearman"));
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
		const auto Selectable = [World](const uint32 Id)
		{
			TArray<FHOLUnitView> Views;
			HOLUnitView::Gather(*World, Views);
			return Views.ContainsByPredicate([Id](const FHOLUnitView& View) { return View.UnitId == Id; });
		};

		const uint32 First = Place(0, FVector(2000.0, -600.0, 0.0));
		const uint32 Second = Place(0, FVector(2000.0, 600.0, 0.0));
		if (TestTrue(TEXT("the two Spearmen born"), First && Second))
		{
			TestTrue(TEXT("the player's own finished tower with places takes the right click"), HOLPlayerOrders::TakesGarrisonOrder(*Tower, 0));
			TestFalse(TEXT("a House (0 place) does not take it"), HOLPlayerOrders::TakesGarrisonOrder(*House, 0));
			TestFalse(TEXT("an enemy's tower does not take it (D374)"), HOLPlayerOrders::TakesGarrisonOrder(*FoeTower, 0));

			const uint32 FirstAsks[] = { First };
			TestTrue(TEXT("the right click's garrison order enters the queue"), HOLPlayerOrders::Garrison(*World, 0, FirstAsks, Tower));
			RunStep();
			TestTrue(TEXT("after the step the first Spearman is inside the tower"), Units->GetGarrison(First) == Tower);
			TestFalse(TEXT("and has left the selection"), Selectable(First));

			const uint32 SecondAsks[] = { Second };
			TestFalse(TEXT("a House refuses the garrison order"), HOLPlayerOrders::Garrison(*World, 0, SecondAsks, House));
			TestFalse(TEXT("an enemy's tower refuses it"), HOLPlayerOrders::Garrison(*World, 0, SecondAsks, FoeTower));
			if (RightClick)
			{
				const int32 Saved = RightClick->GetInt();
				RightClick->Set(0, ECVF_SetByCode);
				TestFalse(TEXT("with hol.Garrison.RightClickEnters 0 the tower does not take the right click"), HOLPlayerOrders::TakesGarrisonOrder(*Tower, 0));
				TestFalse(TEXT("nor the garrison order"), HOLPlayerOrders::Garrison(*World, 0, SecondAsks, Tower));
				RightClick->Set(Saved, ECVF_SetByCode);
			}
			RunStep();
			TestTrue(TEXT("the second Spearman stays on the field"), Units->GetGarrison(Second) == nullptr && Selectable(Second));

			TestFalse(TEXT("another slot cannot empty the tower"), HOLPlayerOrders::UngarrisonAll(*World, 3, Tower));
			TestTrue(TEXT("exit all enters the queue"), HOLPlayerOrders::UngarrisonAll(*World, 0, Tower));
			RunStep();
			TArray<uint32> Inside;
			Units->GetGarrisonUnitIds(Tower, Inside);
			TestEqual(TEXT("the tower is empty after exit all"), Inside.Num(), 0);
			TestTrue(TEXT("the first Spearman is back in the selection"), Selectable(First));
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	Table->RemoveRow(TestTowerType);
	return true;
}
// F7 lot 4a (ELROND-1120h): the order "this unit comes out" (HOLPlayerOrders::UngarrisonUnit) takes ONE occupant out of its tower at the next step, the others stay
// inside; refused to another slot, to a unit on the field and to an unknown unit. The owner's list of occupants (AHOLBuilding::GetGarrisonOccupants, replicated to
// the owner: what the client's frame reads) follows every enter and exit.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGarrisonUngarrisonUnitTest, "HeroesOfLiberty.Garrison.UngarrisonUnit", UE::HeroesOfLiberty::Tests::GarrisonOrder::Flags)

bool FHOLGarrisonUngarrisonUnitTest::RunTest(const FString& Parameters)
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
	AHOLBuilding* Tower = World->SpawnActor<AHOLBuilding>(FVector(3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("tower"), Tower))
	{
		Tower->Setup(TestTowerType, 0, 0);
		const auto RunStep = [Simulation, Units]
		{
			const int64 Step = Simulation->AdvanceStepIndexForTests();
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		const auto Place = [Units](const int32 Owner, const FVector& Location)
		{
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = FName(TEXT("Spearman"));
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
		const auto Selectable = [World](const uint32 Id)
		{
			TArray<FHOLUnitView> Views;
			HOLUnitView::Gather(*World, Views);
			return Views.ContainsByPredicate([Id](const FHOLUnitView& View) { return View.UnitId == Id; });
		};
		const auto Occupants = [Tower]
		{
			TArray<int32> Ids;
			Tower->GetGarrisonOccupants(Ids);
			return Ids;
		};

		const uint32 First = Place(0, FVector(2000.0, -600.0, 0.0));
		const uint32 Second = Place(0, FVector(2000.0, 600.0, 0.0));
		if (TestTrue(TEXT("the two Spearmen born"), First && Second))
		{
			const uint32 Both[] = { First, Second };
			TestTrue(TEXT("the garrison order enters the queue"), HOLPlayerOrders::Garrison(*World, 0, Both, Tower));
			RunStep();
			TestTrue(TEXT("both are inside"), Units->GetGarrison(First) == Tower && Units->GetGarrison(Second) == Tower);
			TestTrue(TEXT("the tower's list of occupants has the two ids, in order"), Occupants() == TArray<int32>({ static_cast<int32>(First), static_cast<int32>(Second) }));

			TestFalse(TEXT("another slot cannot take a unit out"), HOLPlayerOrders::UngarrisonUnit(*World, 3, First));
			TestFalse(TEXT("an unknown unit is refused"), HOLPlayerOrders::UngarrisonUnit(*World, 0, 0x7FFFFFF0u));
			TestTrue(TEXT("the unit's own player takes it out: the order enters the queue"), HOLPlayerOrders::UngarrisonUnit(*World, 0, First));
			RunStep();
			TestTrue(TEXT("after the step the first Spearman is on the field again and selectable"), Units->GetGarrison(First) == nullptr && Selectable(First));
			TestTrue(TEXT("the second Spearman is still inside, not selectable"), Units->GetGarrison(Second) == Tower && !Selectable(Second));
			TestTrue(TEXT("the list of occupants lost only the first"), Occupants() == TArray<int32>({ static_cast<int32>(Second) }));
			TestFalse(TEXT("a unit already on the field is refused"), HOLPlayerOrders::UngarrisonUnit(*World, 0, First));
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	Table->RemoveRow(TestTowerType);
	return true;
}
#endif // WITH_DEV_AUTOMATION_TESTS
