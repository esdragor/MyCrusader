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
#include "Simulation/HOLCombatProcessor.h"
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

	// D810 (Q-forest-ford Q2 = A): F4 the forest slows cavalry (80) and siege (70), never the walker on foot; F5 a ford weakens the defence (120).
	using FRules = FHOLTerrainRules;
	FHOLTerrainZone ForestZone = MakeMarsh(9000, 0, 500);
	ForestZone.Kind = HOLTerrainKind::Forest;
	FHOLTerrainZone FordZone = MakeMarsh(12000, 0, 500);
	FordZone.Kind = HOLTerrainKind::Ford;
	const TArray<FHOLTerrainZone> Painted{ ForestZone, FordZone, MakeMarsh(0, 0, 500), MakeMarsh(9000, 0, 200) };
	TestEqual(TEXT("cavalry in a forest: 80 %"), FRules::GetWalkerSpeedPercentAt(Strength, Painted, HOLTerrainWalker::Cavalry, 9400, 0), 80);
	TestEqual(TEXT("siege in a forest: 70 %"), FRules::GetWalkerSpeedPercentAt(Strength, Painted, HOLTerrainWalker::Siege, 9400, 0), 70);
	TestEqual(TEXT("on foot in a forest: 100 %"), FRules::GetWalkerSpeedPercentAt(Strength, Painted, HOLTerrainWalker::OnFoot, 9400, 0), 100);
	TestEqual(TEXT("cavalry one centimetre out of the forest"), FRules::GetWalkerSpeedPercentAt(Strength, Painted, HOLTerrainWalker::Cavalry, 9501, 0), 100);
	TestEqual(TEXT("a ford slows nobody"), FRules::GetWalkerSpeedPercentAt(Strength, Painted, HOLTerrainWalker::Cavalry, 12000, 0), 100);
	TestEqual(TEXT("marsh under a forest: the slowest wins"), FRules::GetWalkerSpeedPercentAt(Strength, Painted, HOLTerrainWalker::Cavalry, 9000, 0), FMath::Min(Strength, 80));
	TestEqual(TEXT("the marsh alone slows cavalry as before"), FRules::GetWalkerSpeedPercentAt(Strength, Painted, HOLTerrainWalker::Cavalry, 0, 0), Strength);
	TestEqual(TEXT("the forest scales a real speed"), FRules::GetScaledSpeed(500, FRules::GetWalkerSpeedPercentAt(Strength, Painted, HOLTerrainWalker::Cavalry, 9400, 0)), 400);
	TestTrue(TEXT("a unit on the ford is read in the ford"), FRules::IsInZoneOfKind(Painted, HOLTerrainKind::Ford, 12500, 0));
	TestFalse(TEXT("one centimetre off the ford"), FRules::IsInZoneOfKind(Painted, HOLTerrainKind::Ford, 12501, 0));
	TestEqual(TEXT("a hit of 100 on a ford: 120"), FRules::GetFordDamageTaken(100, FRules::IsInZoneOfKind(Painted, HOLTerrainKind::Ford, 12000, 0)), 120);
	TestEqual(TEXT("a hit of 100 off a ford: 100"), FRules::GetFordDamageTaken(100, FRules::IsInZoneOfKind(Painted, HOLTerrainKind::Ford, 0, 0)), 100);
	TestEqual(TEXT("a ford percent that composes"), FRules::GetFordDamageTakenPercent(true), 120);
	TestEqual(TEXT("no hit, no damage on a ford"), FRules::GetFordDamageTaken(0, true), 0);
	TestEqual(TEXT("a negative hit is 0"), FRules::GetFordDamageTaken(-5, true), 0);
	TestTrue(TEXT("a charge stops in a marsh"), FRules::StopsCharge(true, Painted, 100, 0));
	TestFalse(TEXT("a charge does not stop out of a marsh"), FRules::StopsCharge(true, Painted, 2000, 0));
	TestFalse(TEXT("a charge does not stop in a forest or on a ford"), FRules::StopsCharge(true, Painted, 9400, 0) || FRules::StopsCharge(true, Painted, 12000, 0));
	TestFalse(TEXT("the setting off: the marsh stops nothing"), FRules::StopsCharge(false, Painted, 100, 0));
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

// D810 (Q-forest-ford Q1 = A, Q2 = A) on the real chain, as a match's server: the move orders through the command queue and the order
// processor's steps. In a painted forest the speed a walker is asked for is the table's for a man on foot, 80 % of it for cavalry and 70 % for
// siege; outside every zone (a forest not painted there) the table's for all three: the ground off the paint is untouched.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLTerrainForestSpeedTest, "HeroesOfLiberty.Terrain.ForestSpeed", UE::HeroesOfLiberty::Tests::Terrain::Flags)
bool FHOLTerrainForestSpeedTest::RunTest(const FString& Parameters)
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

		// The forest: a disc of 50 m at the origin. A walker of Type born at Center, ordered 60 m on: the table's speed and the one it is asked for.
		FHOLTerrainZone Forest = MakeMarsh(0, 0, 5000);
		Forest.Kind = HOLTerrainKind::Forest;
		Terrain->SetZones(TArray<FHOLTerrainZone>{ Forest });
		const auto Measure = [&](const FName Type, const FVector& Center, float& OutTable, float& OutAsked)
		{
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = Type;
			Request.Count = 1;
			Request.OwnerPlayer = 0;
			Request.Center = Center;
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			TArray<uint32> After;
			Units->GetAllUnitIds(After);
			After.RemoveAll([&Before](const uint32 Id) { return Before.Contains(Id); });
			OutTable = -1.f;
			OutAsked = -1.f;
			if (After.Num() != 1)
			{
				return false;
			}
			uint32 Id = After[0];
			const FMassEntityHandle Entity = Units->FindEntity(Id);
			const FVector At = EntityManager->GetFragmentDataChecked<FTransformFragment>(Entity).GetTransform().GetLocation();
			OutTable = EntityManager->GetConstSharedFragmentDataChecked<FHOLUnitTypeConstants>(Entity).Speed;
			Commands->IssueMoveOrder(MakeArrayView(&Id, 1), At + FVector(0., 6000., 0.), 0);
			for (int32 Step = 0; Step < 10; ++Step)
			{
				FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Orders, ProcessingContext);
				const FMassMoveTargetFragment& Target = EntityManager->GetFragmentDataChecked<FMassMoveTargetFragment>(Entity);
				if (Target.GetCurrentAction() == EMassMovementAction::Move && Target.DesiredSpeed.Get() > 0.f)
				{
					OutAsked = Target.DesiredSpeed.Get();
				}
			}
			const uint32 Gone[] = { Id };
			Units->RequestDestroyUnits(Gone);
			Units->FlushDestructions();
			return OutAsked > 0.f;
		};
		const FName LightCavalry(TEXT("LightCavalry"));
		const FName Ram(TEXT("Ram"));
		const FVector InForest(0., 0., 0.);
		const FVector Unpainted(-30000., 0., 0.);
		float Table = 0.f;
		float Asked = 0.f;
		TestTrue(TEXT("a man at arms walks in the forest"), Measure(ManAtArms, InForest, Table, Asked));
		TestEqual(TEXT("on foot in the forest: the table's speed"), Asked, Table, 0.5f);
		TestTrue(TEXT("a light cavalryman walks in the forest"), Measure(LightCavalry, InForest, Table, Asked));
		TestEqual(TEXT("cavalry in the forest: 80 % of the table's speed"), Asked, Table * FHOLTerrainRules::DefaultForestCavalrySpeedPercent / 100.f, 0.5f);
		TestTrue(TEXT("a ram walks in the forest"), Measure(Ram, InForest, Table, Asked));
		TestEqual(TEXT("siege in the forest: 70 % of the table's speed"), Asked, Table * FHOLTerrainRules::DefaultForestSiegeSpeedPercent / 100.f, 0.5f);
		TestTrue(TEXT("a light cavalryman walks off the paint"), Measure(LightCavalry, Unpainted, Table, Asked));
		TestEqual(TEXT("cavalry off the painted forest: the table's speed"), Asked, Table, 0.5f);
		TestTrue(TEXT("a ram walks off the paint"), Measure(Ram, Unpainted, Table, Asked));
		TestEqual(TEXT("siege off the painted forest: the table's speed"), Asked, Table, 0.5f);
		Terrain->SetZones(TConstArrayView<FHOLTerrainZone>());
		TestTrue(TEXT("a light cavalryman walks on a map without zones"), Measure(LightCavalry, InForest, Table, Asked));
		TestEqual(TEXT("a map without zones: the table's speed"), Asked, Table, 0.5f);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// D810 F5 on the real chain: a hit the combat processor lands on a Spearman standing in a painted ford is 120 % of the same hit off it; with
// no zone on the map the hit is the one off the ford. The victim is born, struck by a Man-at-arms 1.5 m away; the first hit it takes is read.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLTerrainFordHitTest, "HeroesOfLiberty.Terrain.FordHit", UE::HeroesOfLiberty::Tests::Terrain::Flags)
bool FHOLTerrainFordHitTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Terrain;
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	if (!TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled))
	{
		return false;
	}
	const int32 FormerEnabled = CombatEnabled->GetInt();
	CombatEnabled->Set(1, ECVF_SetByCode);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UHOLTerrainSubsystem* Terrain = World->GetSubsystem<UHOLTerrainSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (TestTrue(TEXT("test world ready"), Units && Simulation && Terrain && EntitySubsystem))
	{
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
		Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		int64 Step = 0;
		const auto RunStep = [&]
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			Units->FlushDestructions();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		// As HeroesOfLiberty.HeroAbility.InMatch: born again until its identifier is a multiple of 4 (one search step of 4).
		const auto Place = [Units](const FName Type, const int32 Owner, const FVector& Location)
		{
			for (int32 Attempt = 0; Attempt < 8; ++Attempt)
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
		const auto StateOf = [Units, &EntityManager](const uint32 Id)
		{
			const FMassEntityHandle Entity = Id != 0 ? Units->FindEntity(Id) : FMassEntityHandle();
			return Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
		};
		const FName Spearman(TEXT("Spearman"));
		const auto FirstHit = [&](const FVector& Location)
		{
			const uint32 Victim = Place(Spearman, 0, Location);
			const uint32 Attacker = Place(ManAtArms, 1, Location + FVector(150.0, 0.0, 0.0));
			FHOLUnitStateFragment* VictimState = StateOf(Victim);
			float Taken = -1.f;
			if (VictimState && Attacker != 0)
			{
				const float Before = VictimState->CurrentHealth;
				for (int32 Steps = 0; Steps < FMath::CeilToInt(5.f / StepSeconds); ++Steps)
				{
					RunStep();
					VictimState = StateOf(Victim);
					if (VictimState && VictimState->CurrentHealth < Before)
					{
						Taken = Before - VictimState->CurrentHealth;
						break;
					}
				}
			}
			const uint32 Gone[] = { Victim, Attacker };
			Units->RequestDestroyUnits(Gone);
			Units->FlushDestructions();
			return Taken;
		};

		FHOLTerrainZone Ford = MakeMarsh(0, 0, 3000);
		Ford.Kind = HOLTerrainKind::Ford;
		Terrain->SetZones(TArray<FHOLTerrainZone>{ Ford });
		const float OnFord = FirstHit(FVector(0., 0., 0.));
		const float OffFord = FirstHit(FVector(-30000., 0., 0.));
		Terrain->SetZones(TConstArrayView<FHOLTerrainZone>());
		const float NoZones = FirstHit(FVector(0., 0., 0.));
		if (TestTrue(TEXT("a hit landed on the ford"), OnFord > 0.f) && TestTrue(TEXT("a hit landed off the ford"), OffFord > 0.f)
			&& TestTrue(TEXT("a hit landed on a map without zones"), NoZones > 0.f))
		{
			TestEqual(TEXT("on the ford: 120 % of the hit off it"), OnFord, OffFord * FHOLTerrainRules::DefaultFordDamageTakenPercent / 100.f, 0.05f);
			TestEqual(TEXT("a map without zones: the hit is the one off the ford"), NoZones, OffFord, 0.05f);
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
