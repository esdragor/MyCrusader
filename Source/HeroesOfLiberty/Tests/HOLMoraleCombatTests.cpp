// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLMoraleRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// PLAN-MORAL-1120 step 5 (S2-a): the combat reads the morale levers of the STRIKER, Dealt (damage dealt) and Rate (attack rate), as it already
// reads Taken on the victim (HOLMorale::TakenPercent). The REAL chain: a Spearman of slot 1 carrying a morale effect strikes a Man-at-arms of
// slot 0 through UHOLCombatProcessor; the loss of the victim is read off its fragment. The expected ratio is CALCULATED by the rule
// (HOLMorale::Multiplier), never typed.
namespace UE::HeroesOfLiberty::Tests::MoraleCombat
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	constexpr int32 TestEffectId = 900;

	struct FRun
	{
		bool bValid = false;
		float FirstBlowLoss = 0.f;
		int32 Blows = 0;
	};

	// One fight of ten seconds: the striker carries (Dealt, Rate) benefit percents (0, 0 = no morale effect at all).
	FRun Fight(FAutomationTestBase& Test, const int32 DealtPercent, const int32 RatePercent)
	{
		FRun Result;
		IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
		if (!Test.TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled))
		{
			return Result;
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
		if (Test.TestNotNull(TEXT("units"), Units) && Test.TestNotNull(TEXT("simulation"), Simulation) && Test.TestNotNull(TEXT("entities"), EntitySubsystem))
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
			// Born again until its identifier is a multiple of 4 (one search step of 4), as HeroesOfLiberty.DamageRedirect.Chain.
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

			const uint32 Victim = Place(FName(TEXT("ManAtArms")), 0, FVector(300.0, 0.0, 0.0));
			const uint32 Striker = Place(FName(TEXT("Spearman")), 1, FVector(400.0, 0.0, 0.0));
			FHOLUnitStateFragment* VictimState = StateOf(Victim);
			FHOLUnitStateFragment* StrikerState = StateOf(Striker);
			const FMassEntityHandle StrikerEntity = Striker != 0 ? Units->FindEntity(Striker) : FMassEntityHandle();
			FHOLMoraleFragment* StrikerMorale = StrikerEntity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLMoraleFragment>(StrikerEntity) : nullptr;
			if (Test.TestTrue(TEXT("a Man-at-arms of slot 0, a Spearman of slot 1 with its morale fragment"), VictimState && StrikerState && StrikerMorale))
			{
				VictimState->CurrentHealth = 100000.f;
				StrikerState->CurrentHealth = 100000.f;
				if (DealtPercent != 0 || RatePercent != 0)
				{
					// An effect that does not run down in ten seconds (the morale processor is not run here: only the combat reads the fragment).
					HOLMorale::Apply(StrikerMorale->State, HOLMorale::MakeEffect(TestEffectId, DealtPercent, 0, RatePercent, 100000, false));
				}
				float Last = VictimState->CurrentHealth;
				for (int32 Steps = 0; Steps < FMath::CeilToInt(10.f / StepSeconds); ++Steps)
				{
					RunStep();
					const float Now = StateOf(Victim)->CurrentHealth;
					if (Now < Last)
					{
						Result.FirstBlowLoss = Result.Blows == 0 ? Last - Now : Result.FirstBlowLoss;
						++Result.Blows;
					}
					Last = Now;
				}
				Result.bValid = true;
			}
		}

		CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		return Result;
	}
}

// RED while the combat reads the Taken lever of the victim alone: the striker's Dealt +30 % leaves the blow unchanged.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMoraleDealtPercentRaisesDamageTest, "HeroesOfLiberty.Morale.MoraleDealtPercentRaisesDamage", UE::HeroesOfLiberty::Tests::MoraleCombat::Flags)

bool FHOLMoraleDealtPercentRaisesDamageTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MoraleCombat;
	const int32 Percent = 30;
	HOLMorale::FHOLMoraleState Expected;
	HOLMorale::Apply(Expected, HOLMorale::MakeEffect(TestEffectId, Percent, 0, 0, 100000, false));
	const float Multiplier = static_cast<float>(HOLMorale::Multiplier(Expected, HOLMorale::LeverDealt)) / 100.f;
	TestTrue(TEXT("the rule gives more than 100 % of the damage for a Dealt bonus"), Multiplier > 1.f);

	const FRun Base = Fight(*this, 0, 0);
	const FRun Boosted = Fight(*this, Percent, 0);
	if (TestTrue(TEXT("both fights ran"), Base.bValid && Boosted.bValid) && TestTrue(TEXT("the striker hit in both fights"), Base.FirstBlowLoss > 0.f && Boosted.FirstBlowLoss > 0.f))
	{
		TestEqual(TEXT("the first blow does the rule's Dealt multiplier of the plain blow"), Boosted.FirstBlowLoss / Base.FirstBlowLoss, Multiplier, 0.02f);
	}
	return true;
}

// RED while the combat ignores the Rate lever: a striker with Rate +30 % strikes no more often in ten seconds.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMoraleRatePercentRaisesBlowsTest, "HeroesOfLiberty.Morale.MoraleRatePercentRaisesBlows", UE::HeroesOfLiberty::Tests::MoraleCombat::Flags)

bool FHOLMoraleRatePercentRaisesBlowsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MoraleCombat;
	const FRun Base = Fight(*this, 0, 0);
	const FRun Fast = Fight(*this, 0, 30);
	if (TestTrue(TEXT("both fights ran"), Base.bValid && Fast.bValid) && TestTrue(TEXT("the striker hit in the plain fight"), Base.Blows > 0))
	{
		TestTrue(*FString::Printf(TEXT("Rate +30 %% gives more blows in ten seconds (%d against %d)"), Fast.Blows, Base.Blows), Fast.Blows > Base.Blows);
		// The rate changes the time between blows, not the damage of one blow.
		TestEqual(TEXT("a blow does the same damage (the Rate lever is not the Dealt lever)"), Fast.FirstBlowLoss, Base.FirstBlowLoss, 0.01f * FMath::Max(Base.FirstBlowLoss, 1.f));
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
