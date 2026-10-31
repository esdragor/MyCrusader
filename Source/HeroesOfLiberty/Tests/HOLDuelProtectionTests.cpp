// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Match/HOLStartRules.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLDuelSubsystem.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// D5 (DuelInProgress.md; GDD 10.2, D18): a hero in a duel can be hit by his adversary only, and hits nobody else. Through the
// real combat processor, the hero subsystem's step end and the script book of duels (Script/Duel/HOLDuelLedger.as), no number
// typed in: every expectation is a unit's health read before and after.
namespace UE::HeroesOfLiberty::Tests::DuelProtection
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FName Hero(TEXT("Hero"));
	const FName Spearman(TEXT("Spearman"));
	const FName ManAtArms(TEXT("ManAtArms"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDuelProtectedInMatchTest, "HeroesOfLiberty.Duel.ProtectedInMatch", UE::HeroesOfLiberty::Tests::DuelProtection::Flags)

// Three heroes (slots 0, 1, 2, enemies of each other, a Man-at-arms of slot 3 beside slot 0's hero first):
// - no duel: the Man-at-arms hurts the hero of slot 0 (the setup lands blows);
// - slot 0 and 2 in a duel (side by side): the Man-at-arms of slot 3 does not touch hero 0 nor chases him, but fights a
//   Spearman of slot 0 standing near (the battle goes on around); the duellists hurt each other and hurt nobody else;
// - the duel over (hero 2 fallen, the book told by the hero subsystem's step end): hero 0 can be hit again.
bool FHOLDuelProtectedInMatchTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::DuelProtection;
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
	UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	UHOLDuelSubsystem* Duels = UHOLDuelSubsystem::Find(World);
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("heroes"), Heroes)
		&& TestNotNull(TEXT("entities"), EntitySubsystem) && TestNotNull(TEXT("the script book of duels"), Duels))
	{
		Heroes->BindSteps();
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
		Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		const int32 FiveSeconds = FMath::CeilToInt(5.f / StepSeconds);
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
		const auto HealthOf = [&StateOf](const uint32 Id)
		{
			const FHOLUnitStateFragment* State = StateOf(Id);
			return State ? State->CurrentHealth : -1.f;
		};
		// Runs up to Steps steps, until the unit's health is below Before; whether it was hit.
		const auto HitWithin = [&](const uint32 Id, const float Before, const int32 Steps)
		{
			for (int32 Index = 0; Index < Steps; ++Index)
			{
				RunStep();
				const float Now = HealthOf(Id);
				if (Now >= 0.f && Now < Before)
				{
					return true;
				}
			}
			return false;
		};
		const auto Remove = [Units](std::initializer_list<uint32> Ids)
		{
			TArray<uint32> Gone;
			for (const uint32 Id : Ids)
			{
				Gone.Add(Id);
			}
			Units->RequestDestroyUnits(Gone);
			Units->FlushDestructions();
		};

		// Without a duel: the setup lands blows.
		const uint32 HeroA = Place(Hero, 0, FVector(0.0, 0.0, 0.0));
		const uint32 Guard = Place(ManAtArms, 3, FVector(150.0, 0.0, 0.0));
		if (TestTrue(TEXT("hero of slot 0 and a Man-at-arms of slot 3 born"), HeroA != 0 && Guard != 0))
		{
			RunStep();
			TestFalse(TEXT("no duel on"), Duels->IsDuelOn());
			TestTrue(TEXT("no duel: the Man-at-arms hurts the hero"), HitWithin(HeroA, HealthOf(HeroA), 2 * FiveSeconds));
			Remove({ Guard });

			// A duel of slot 0 and slot 2, the heroes side by side.
			const uint32 HeroC = Place(Hero, 2, FVector(-150.0, 0.0, 0.0));
			if (TestTrue(TEXT("hero of slot 2 born"), HeroC != 0))
			{
				RunStep();
				TestTrue(TEXT("the duel begins"), Duels->BeginDuelNow(0, 2));
				TestTrue(TEXT("a duel is on"), Duels->IsDuelOn());
				TestFalse(TEXT("a hero already in a duel cannot begin another"), Duels->BeginDuelNow(0, 1));
				// A Man-at-arms of slot 3 beside hero 0 and a Spearman of slot 0 behind him, 3 m from the hero.
				const uint32 Guard2 = Place(ManAtArms, 3, FVector(150.0, 0.0, 0.0));
				const uint32 Friend = Place(Spearman, 0, FVector(300.0, 0.0, 0.0));
				if (TestTrue(TEXT("a Man-at-arms of slot 3 and a Spearman of slot 0 born"), Guard2 != 0 && Friend != 0))
				{
					const float HeroAAtStart = HealthOf(HeroA);
					const float FriendAtStart = HealthOf(Friend);
					bool bHeroHit = false;
					bool bFriendHit = false;
					bool bGuardHit = false;
					const float GuardAtStart = HealthOf(Guard2);
					for (int32 Index = 0; Index < FiveSeconds; ++Index)
					{
						RunStep();
						bFriendHit |= HealthOf(Friend) < FriendAtStart;
						bGuardHit |= HealthOf(Guard2) < GuardAtStart;
					}
					bHeroHit = HealthOf(HeroA) < HeroAAtStart;
					// Hero 0 took blows from hero 2 only: the Man-at-arms never targets him, and his spearman friend was hurt
					// by the Man-at-arms (the battle goes on around), nobody of slot 3 was touched by a duellist.
					const FMassEntityHandle GuardEntity = Units->FindEntity(Guard2);
					const FHOLCombatFragment* GuardCombat = GuardEntity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(GuardEntity) : nullptr;
					if (TestNotNull(TEXT("the Man-at-arms' combat"), GuardCombat))
					{
						TestTrue(TEXT("the Man-at-arms does not aim at the duellist"), GuardCombat->Target != Units->FindEntity(HeroA) && GuardCombat->Target != Units->FindEntity(HeroC));
					}
					TestTrue(TEXT("the duellists hurt each other"), bHeroHit);
					TestTrue(TEXT("the battle goes on around: the Man-at-arms hurts the Spearman"), bFriendHit);
					// A duellist hurts nobody else: a Spearman of slot 4 beside hero 2, nearer to him (1 m) than hero 0 is (1.5 m), alone
						// (nobody else of the battle left near), is not touched by either hero in 5 s, while they still hurt each other.
						(void)bGuardHit; // not asserted: the Spearman friend of slot 0 fights the Man-at-arms
						Remove({ Guard2, Friend });
						const uint32 Bystander = Place(Spearman, 4, FVector(-150.0, 100.0, 0.0));
						if (TestTrue(TEXT("a bystander of slot 4 born"), Bystander != 0))
						{
							const float BystanderAtStart = HealthOf(Bystander);
							const float HeroCAtStart = HealthOf(HeroC);
							const float HeroAMid = HealthOf(HeroA);
							bool bBystanderHit = false;
							for (int32 Index = 0; Index < FiveSeconds; ++Index)
							{
								RunStep();
								bBystanderHit |= HealthOf(Bystander) < BystanderAtStart;
							}
							TestFalse(TEXT("a duellist hurts nobody else: the bystander beside hero 2 is untouched"), bBystanderHit);
							TestTrue(TEXT("... while the duellists still hurt each other"), HealthOf(HeroC) < HeroCAtStart || HealthOf(HeroA) < HeroAMid);
							Remove({ Bystander });
						}

					// The hero of slot 2 falls (a duel won): the book is told at the step end, the protection goes.
					const int32 EndedBefore = Duels->GetNumEnded();
					Units->KillUnitsOfOwner(2, FHOLStartRules::HeroTypeName);
					for (int32 Index = 0; Index < 4; ++Index)
					{
						RunStep();
					}
					TestEqual(TEXT("the duel ended once the hero fell"), Duels->GetNumEnded(), EndedBefore + 1);
					TestFalse(TEXT("no duel on any more"), Duels->IsDuelOn());
					const uint32 Guard3 = Place(ManAtArms, 3, FVector(150.0, 0.0, 0.0));
					TestTrue(TEXT("a Man-at-arms born"), Guard3 != 0);
					TestTrue(TEXT("the duel over: the Man-at-arms hurts the hero again"), HitWithin(HeroA, HealthOf(HeroA), 3 * FiveSeconds));
				}
			}
		}
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
