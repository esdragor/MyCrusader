// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLUnitOrderProcessor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Map/HOLTerrainLayout.h"
#include "Map/HOLTerrainRules.h"
#include "Map/HOLTerrainSubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassProcessingContext.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// C3 (Cartes, El Dorado): the zones' speed rule, pure. The marsh slows a walker standing in it to the strength's percent (its edge
// counts in), nothing outside, the slowest of two overlapping zones (not a product); a speed is never scaled to 0.
namespace UE::HeroesOfLiberty::Tests::Terrain
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	const FName ManAtArms(TEXT("ManAtArms"));

	FHOLTerrainZone MakeMarsh(const int32 X, const int32 Y, const int32 Radius)
	{
		FHOLTerrainZone Zone;
		Zone.Kind = HOLTerrainKind::Marsh;
		Zone.X = X;
		Zone.Y = Y;
		Zone.Radius = Radius;
		return Zone;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLTerrainRulesTest, "HeroesOfLiberty.Terrain.Rules", UE::HeroesOfLiberty::Tests::Terrain::Flags)
bool FHOLTerrainRulesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Terrain;
	const int32 Strength = FHOLTerrainRules::DefaultMarshSpeedPercent;
	TestTrue(TEXT("the marsh slows"), Strength < 100);
	TestTrue(TEXT("the marsh does not stop"), Strength > 0);
	const TArray<FHOLTerrainZone> One{ MakeMarsh(1000, 2000, 500) };
	TestEqual(TEXT("in the middle"), FHOLTerrainRules::GetSpeedPercentAt(Strength, One, 1000, 2000), Strength);
	TestEqual(TEXT("on the edge"), FHOLTerrainRules::GetSpeedPercentAt(Strength, One, 1500, 2000), Strength);
	TestEqual(TEXT("one centimetre outside"), FHOLTerrainRules::GetSpeedPercentAt(Strength, One, 1501, 2000), 100);
	TestEqual(TEXT("no zone, full speed"), FHOLTerrainRules::GetSpeedPercentAt(Strength, TConstArrayView<FHOLTerrainZone>(), 0, 0), 100);
	const TArray<FHOLTerrainZone> Two{ MakeMarsh(0, 0, 800), MakeMarsh(300, 0, 800) };
	TestEqual(TEXT("in both marshes: slowed once"), FHOLTerrainRules::GetSpeedPercentAt(Strength, Two, 200, 0), Strength);
	// C5 (La Foret profonde, CQ6a = A): a road speeds the walker up to the road's percent, edge in; a marsh over a road wins.
	const int32 Road = FHOLTerrainRules::DefaultRoadSpeedPercent;
	TestTrue(TEXT("the road speeds up"), Road > 100);
	FHOLTerrainZone RoadZone = MakeMarsh(5000, 0, 400);
	RoadZone.Kind = HOLTerrainKind::Road;
	const TArray<FHOLTerrainZone> OneRoad{ RoadZone };
	TestEqual(TEXT("on the road"), FHOLTerrainRules::GetSpeedPercentAt(Strength, OneRoad, 5000, 0), Road);
	TestEqual(TEXT("on the road's edge"), FHOLTerrainRules::GetSpeedPercentAt(Strength, OneRoad, 5400, 0), Road);
	TestEqual(TEXT("one centimetre off the road"), FHOLTerrainRules::GetSpeedPercentAt(Strength, OneRoad, 5401, 0), 100);
	TestEqual(TEXT("the road's own percent is read"), FHOLTerrainRules::GetSpeedPercentAt(Strength, OneRoad, 5000, 0, 150), 150);
	const TArray<FHOLTerrainZone> RoadUnderMarsh{ RoadZone, MakeMarsh(5000, 0, 200) };
	TestEqual(TEXT("a marsh over a road slows"), FHOLTerrainRules::GetSpeedPercentAt(Strength, RoadUnderMarsh, 5000, 0), Strength);
	TestEqual(TEXT("the road part of the same zones"), FHOLTerrainRules::GetSpeedPercentAt(Strength, RoadUnderMarsh, 5300, 0), Road);
	TestEqual(TEXT("a walker never reaches 0"), FHOLTerrainRules::GetScaledSpeed(1, 10), 1);
	TestEqual(TEXT("a unit that does not walk stays still"), FHOLTerrainRules::GetScaledSpeed(0, Strength), 0);
	TestEqual(TEXT("the percent of the speed"), FHOLTerrainRules::GetScaledSpeed(500, Strength), 500 * Strength / 100);
	return true;
}

// C3b on the real chain, as a match's server: a test world, the move orders through the command queue, the order processor's
// steps; the speed each walker is asked for (the move target's desired speed) is the table's outside the marsh and the marsh's
// percent of it inside, and back to the table's once the marsh zones are taken away.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLTerrainWalkSpeedTest, "HeroesOfLiberty.Terrain.WalkSpeed", UE::HeroesOfLiberty::Tests::Terrain::Flags)
bool FHOLTerrainWalkSpeedTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Terrain;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	UHOLTerrainSubsystem* Terrain = World->GetSubsystem<UHOLTerrainSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (TestTrue(TEXT("test world ready"), State && Units && Commands && Terrain && Simulation && EntitySubsystem))
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : (Index == 1 ? EHOLSlotState::AI : EHOLSlotState::Closed);
		}
		State->InitVictory(Config);
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLUnitOrderProcessor* Orders = NewObject<UHOLUnitOrderProcessor>(Simulation);
		Orders->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();

		// Two walkers a long way apart: one in the marsh (disc of 30 m at the origin), one far outside.
		Terrain->SetZones(TArray<FHOLTerrainZone>{ MakeMarsh(0, 0, 3000) });
		const FVector InMarsh(0., 0., 0.);
		const FVector OutOfMarsh(-30000., 0., 0.);
		uint32 Ids[2] = {};
		const FVector Places[2] = { InMarsh, OutOfMarsh };
		for (int32 Walker = 0; Walker < 2; ++Walker)
		{
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = ManAtArms;
			Request.Count = 1;
			Request.OwnerPlayer = 0;
			Request.Center = Places[Walker];
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			TArray<uint32> After;
			Units->GetAllUnitIds(After);
			After.RemoveAll([&Before](const uint32 Id) { return Before.Contains(Id); });
			Ids[Walker] = After.Num() == 1 ? After[0] : 0;
		}
		if (TestTrue(TEXT("two walkers born"), Ids[0] != 0 && Ids[1] != 0))
		{
			const FMassEntityHandle InEntity = Units->FindEntity(Ids[0]);
			const FMassEntityHandle OutEntity = Units->FindEntity(Ids[1]);
			// Where they stand when born (the spawn spreads a little): both orders are far, 60 m on.
			const FVector InAt = EntityManager->GetFragmentDataChecked<FTransformFragment>(InEntity).GetTransform().GetLocation();
			const FVector OutAt = EntityManager->GetFragmentDataChecked<FTransformFragment>(OutEntity).GetTransform().GetLocation();
			TestEqual(TEXT("the first walker is in the marsh"), Terrain->GetSpeedPercentAt(InAt), FHOLTerrainRules::DefaultMarshSpeedPercent);
			TestEqual(TEXT("the second is out of it"), Terrain->GetSpeedPercentAt(OutAt), 100);

			const float TableSpeed = EntityManager->GetConstSharedFragmentDataChecked<FHOLUnitTypeConstants>(InEntity).Speed;
			Commands->IssueMoveOrder(MakeArrayView(&Ids[0], 1), InAt + FVector(0., 6000., 0.), 0);
			Commands->IssueMoveOrder(MakeArrayView(&Ids[1], 1), OutAt + FVector(0., 6000., 0.), 0);
			float InSpeed = -1.f;
			float OutSpeed = -1.f;
			for (int32 Step = 0; Step < 10; ++Step)
			{
				FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Orders, ProcessingContext);
				const FMassMoveTargetFragment& InTarget = EntityManager->GetFragmentDataChecked<FMassMoveTargetFragment>(InEntity);
				const FMassMoveTargetFragment& OutTarget = EntityManager->GetFragmentDataChecked<FMassMoveTargetFragment>(OutEntity);
				if (InTarget.GetCurrentAction() == EMassMovementAction::Move && InTarget.DesiredSpeed.Get() > 0.f)
				{
					InSpeed = InTarget.DesiredSpeed.Get();
				}
				if (OutTarget.GetCurrentAction() == EMassMovementAction::Move && OutTarget.DesiredSpeed.Get() > 0.f)
				{
					OutSpeed = OutTarget.DesiredSpeed.Get();
				}
			}
			TestTrue(TEXT("the walker out of the marsh was set walking"), OutSpeed > 0.f);
			TestTrue(TEXT("the walker in the marsh was set walking"), InSpeed > 0.f);
			TestEqual(TEXT("out of the marsh: the table's speed (hol.Units.SpeedScale 1)"), OutSpeed, TableSpeed);
			TestEqual(TEXT("in the marsh: the marsh's percent of it"), InSpeed, TableSpeed * FHOLTerrainRules::DefaultMarshSpeedPercent / 100.f, 0.5f);
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
