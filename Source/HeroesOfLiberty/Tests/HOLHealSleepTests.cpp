// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLHealProcessor.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// PerfSpikeInProgress.md, part 1: the Monk's heal sleeps while no unit of the world can heal. The step then gathers no table of units
// (GetLastCandidateCount() == 0), instead of one entry per light unit (~0.1 ms a step with 2000 units, even in a match with no Monk).
// Through the real heal processor and the real unit table, as the match runs them.
namespace UE::HeroesOfLiberty::Tests::HealSleep
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** A unit of Type born for Owner at Location, its identifier (0 if none). */
	uint32 Place(UHOLUnitSubsystem* Units, const FName Type, const int32 Owner, const FVector& Location)
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
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHealSleepTest, "HeroesOfLiberty.Monk.HealSleep", UE::HeroesOfLiberty::Tests::HealSleep::Flags)

bool FHOLHealSleepTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HealSleep;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (TestNotNull(TEXT("game state"), GameState) && TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation)
		&& TestNotNull(TEXT("entities"), EntitySubsystem))
	{
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLHealProcessor* Heal = NewObject<UHOLHealProcessor>(Simulation);
		Heal->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		int64 Step = 0;
		const auto RunStep = [Simulation, Heal, &EntityManager, StepSeconds, &Step]
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Heal, ProcessingContext);
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		const auto StateOf = [Units, &EntityManager](const uint32 Id) -> FHOLUnitStateFragment*
		{
			const FMassEntityHandle Entity = Id != 0 ? Units->FindEntity(Id) : FMassEntityHandle();
			return Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
		};

		// 1. Spearmen only: nobody can heal, the step gathers no table.
		const uint32 First = Place(Units, FName(TEXT("Spearman")), 0, FVector::ZeroVector);
		const uint32 Second = Place(Units, FName(TEXT("Spearman")), 0, FVector(150., 0., 0.));
		const uint32 Enemy = Place(Units, FName(TEXT("Spearman")), 1, FVector(5000., 0., 0.));
		if (TestTrue(TEXT("three Spearmen born"), First != 0 && Second != 0 && Enemy != 0))
		{
			RunStep();
			RunStep();
			TestEqual(TEXT("no Monk in the world: the step gathers no unit"), UHOLHealProcessor::GetLastCandidateCount(), 0);
			TestEqual(TEXT("no Monk in the world: no heal link"), UHOLHealProcessor::GetLinks().Num(), 0);

			// 2. A Monk is born next to a wounded Spearman: the step wakes up (the table holds every light unit), the heal goes on.
			const uint32 Monk = Place(Units, FName(TEXT("Monk")), 0, FVector(75., 0., 0.));
			FHOLUnitStateFragment* Patient = StateOf(Second);
			if (TestTrue(TEXT("a Monk born, the Spearman there"), Monk != 0 && Patient != nullptr))
			{
				Patient->CurrentHealth = 1.f;
				for (int32 Index = 0; Index < 40; ++Index)
				{
					RunStep();
				}
				TestEqual(TEXT("a Monk in the world: the step gathers the four units"), UHOLHealProcessor::GetLastCandidateCount(), 4);
				TestTrue(TEXT("a Monk in the world: the wounded Spearman is healed"), Patient->CurrentHealth > 1.f);

				// 3. The Monk dies (removed): the heal sleeps again at the next step.
				Units->RequestDestroyUnits({ Monk });
				Units->FlushDestructions();
				RunStep();
				RunStep();
				TestEqual(TEXT("the Monk is gone: the step sleeps again"), UHOLHealProcessor::GetLastCandidateCount(), 0);
				TestEqual(TEXT("the Monk is gone: no heal link"), UHOLHealProcessor::GetLinks().Num(), 0);
			}
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
