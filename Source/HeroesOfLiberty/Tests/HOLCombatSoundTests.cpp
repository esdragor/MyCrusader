#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Misc/AutomationTest.h"
#include "Presentation/HOLCombatSoundSubsystem.h"
#include "Presentation/HOLCorpse.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace UE::HeroesOfLiberty::Tests::CombatSound
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** A unit of Type born for Owner at Location, its identifier a multiple of 4 (it searches each step: HOLRegicideTests). */
	uint32 PlaceUnit(UHOLUnitSubsystem& Units, const FName Type, const int32 Owner, const FVector& Location)
	{
		for (int32 Attempt = 0; Attempt < 8; ++Attempt)
		{
			TArray<uint32> Before;
			Units.GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = Type;
			Request.Count = 1;
			Request.OwnerPlayer = static_cast<uint8>(Owner);
			Request.Center = Location;
			Units.RequestUnits(Request);
			Units.ServeBirthRequests();
			TArray<uint32> After;
			Units.GetAllUnitIds(After);
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
			Units.RequestDestroyUnits(Again);
			Units.FlushDestructions();
		}
		return 0;
	}
}

// AR6-c (ArtistesInProgress.md): the combat sound events come from the real simulation: two Men-at-arms fighting through the
// combat processor raise ONE Attack event per blow (the number of blows counted by the processor itself, not per step), and the
// bodies HOLCorpse::Lay lays raise one Death event each (the number of bodies made by Lay). A game world has the subsystem (solo /
// host / client), a dedicated server does not.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCombatSoundEventsTest, "HeroesOfLiberty.Combat.SoundEvents", UE::HeroesOfLiberty::Tests::CombatSound::Flags)

bool FHOLCombatSoundEventsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::CombatSound;

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
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	UHOLCombatSoundSubsystem* Sounds = World->GetSubsystem<UHOLCombatSoundSubsystem>();
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("entities"), EntitySubsystem)
		&& TestNotNull(TEXT("the combat sounds in a game world"), Sounds))
	{
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
		Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		int64 Step = 0;
		const auto RunStep = [Simulation, Units, Combat, &EntityManager, StepSeconds, &Step]
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			Units->FlushDestructions();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};

		const uint32 Soldier = PlaceUnit(*Units, FName(TEXT("ManAtArms")), 0, FVector(0.0, 0.0, 0.0));
		const uint32 Foe = PlaceUnit(*Units, FName(TEXT("ManAtArms")), 1, FVector(150.0, 0.0, 0.0));
		TestTrue(TEXT("two Men-at-arms born"), Soldier != 0 && Foe != 0);

		// Bound as the game screen binds it: what reaches the script is what is counted.
		int32 Heard = 0;
		const int64 HitsBefore = UHOLCombatProcessor::GetTotalHits();
		const int32 AttacksBefore = Sounds->GetEventCount(UHOLCombatSoundSubsystem::AttackSlot);
		const int32 Steps = FMath::CeilToInt(10.f / StepSeconds);
		for (int32 Index = 0; Index < Steps; ++Index)
		{
			RunStep();
		}
		const int64 Hits = UHOLCombatProcessor::GetTotalHits() - HitsBefore;
		const int32 Attacks = Sounds->GetEventCount(UHOLCombatSoundSubsystem::AttackSlot) - AttacksBefore;
		TestTrue(TEXT("they strike in 10 s"), Hits > 0);
		TestEqual(TEXT("one Attack sound event per blow (the processor's count)"), static_cast<int64>(Attacks), Hits);
		TestTrue(TEXT("a blow is not a step: fewer events than steps x fighters"), Attacks < Steps * 2);
		Heard = Attacks;

		// Two bodies laid (a death each): one Death event each, at the body's place.
		TArray<FHOLCorpseSeed> Seeds;
		for (int32 Slot = 0; Slot < 2; ++Slot)
		{
			FHOLCorpseSeed& Seed = Seeds.AddDefaulted_GetRef();
			Seed.TypeName = FName(TEXT("ManAtArms"));
			Seed.Corpse.TypeName = Seed.TypeName;
			Seed.Corpse.OwnerPlayer = static_cast<uint8>(Slot);
			Seed.Corpse.Base = FTransform(FVector(1000.0 * (Slot + 1), 0.0, 0.0));
		}
		const int32 DeathsBefore = Sounds->GetEventCount(UHOLCombatSoundSubsystem::DeathSlot);
		TArray<FMassEntityHandle> Bodies;
		HOLCorpse::Lay(EntitySubsystem->GetMutableEntityManager(), *Units, Seeds, &Bodies);
		TestTrue(TEXT("bodies laid"), Bodies.Num() > 0);
		TestEqual(TEXT("one Death sound event per body laid"), Sounds->GetEventCount(UHOLCombatSoundSubsystem::DeathSlot) - DeathsBefore, Bodies.Num());
		TestTrue(TEXT("the attack events were heard"), Heard > 0);
	}
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// AR6-d (ArtistesInProgress.md): an order of the local player raises one Order sound event of the type of the first commanded unit this machine knows
// (HOLUnitView::Find), nothing without a unit; a foundation laid raises one Construction event of its building type. Counted by the subsystem itself.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLOrderSoundEventsTest, "HeroesOfLiberty.Audio.OrderSoundEvents", UE::HeroesOfLiberty::Tests::CombatSound::Flags)

bool FHOLOrderSoundEventsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::CombatSound;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLCombatSoundSubsystem* Sounds = World->GetSubsystem<UHOLCombatSoundSubsystem>();
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("the sound subsystem in a game world"), Sounds))
	{
		const FName ManAtArms(TEXT("ManAtArms"));
		const FName House(TEXT("House"));
		const uint32 Soldier = PlaceUnit(*Units, ManAtArms, 0, FVector(0.0, 0.0, 0.0));
		TestTrue(TEXT("a Man-at-arms born"), Soldier != 0);
		const uint32 Ids[] = { Soldier };
		TestEqual(TEXT("the first unit's type"), UHOLCombatSoundSubsystem::GetFirstUnitType(World, Ids), ManAtArms);
		TestEqual(TEXT("no unit: no type"), UHOLCombatSoundSubsystem::GetFirstUnitType(World, TConstArrayView<uint32>()), FName(NAME_None));

		const int32 OrdersBefore = Sounds->GetEventCount(UHOLCombatSoundSubsystem::OrderSlot);
		const int32 TypedBefore = Sounds->GetEventCountFor(ManAtArms, UHOLCombatSoundSubsystem::OrderSlot);
		UHOLCombatSoundSubsystem::RaiseOrder(World, UHOLCombatSoundSubsystem::MoveKind, Ids, FVector(500.0, 0.0, 0.0));
		TestEqual(TEXT("one Order event per order"), Sounds->GetEventCount(UHOLCombatSoundSubsystem::OrderSlot) - OrdersBefore, 1);
		TestEqual(TEXT("the Order event is the Man-at-arms'"), Sounds->GetEventCountFor(ManAtArms, UHOLCombatSoundSubsystem::OrderSlot) - TypedBefore, 1);
		UHOLCombatSoundSubsystem::RaiseOrder(World, UHOLCombatSoundSubsystem::MoveKind, TConstArrayView<uint32>(), FVector::ZeroVector);
		TestEqual(TEXT("an order without units rings nothing"), Sounds->GetEventCount(UHOLCombatSoundSubsystem::OrderSlot) - OrdersBefore, 1);

		const int32 PlacedBefore = Sounds->GetEventCountFor(House, UHOLCombatSoundSubsystem::ConstructionSlot);
		UHOLCombatSoundSubsystem::RaisePlacement(World, House, FVector(2000.0, 0.0, 0.0));
		TestEqual(TEXT("a foundation laid rings its building's Construction once"), Sounds->GetEventCountFor(House, UHOLCombatSoundSubsystem::ConstructionSlot) - PlacedBefore, 1);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

#endif
