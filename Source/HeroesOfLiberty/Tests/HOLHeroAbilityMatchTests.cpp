// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Misc/AutomationTest.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Presentation/HOLBannerSubsystem.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLHeroAbilityRules.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLHeroXPRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// J8c.2: the aura, the cry and the banner in a match (J8c-Q1 to Q3), numbers written from the decisions, not from the code.
namespace UE::HeroesOfLiberty::Tests::HeroAbilityMatch
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FName Hero(TEXT("Hero"));
	const FName Spearman(TEXT("Spearman"));
	const FName ManAtArms(TEXT("ManAtArms"));

	constexpr float Tolerance = 1.e-3f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroAbilityInMatchTest, "HeroesOfLiberty.HeroAbility.InMatch", UE::HeroesOfLiberty::Tests::HeroAbilityMatch::Flags)

// Through the combat processor, as in a match (no movement: nobody walks). Slot 0's Spearmen (armour 0) each hit by a
// Man-at-arms of slot 1 (12) standing 1.5 m from them: 12 far from the hero, 10.2 in his aura (-15 %), 7.2 under aura and
// cry (-40 %), 9 under the cry alone once out of the aura; a banner planted 16 m away gives the aura where he is not, a
// banner is refused at level 1 and 16.5 m away, the cry refused during its cool-down; the hero dead, his aura goes, the
// banner's stays.
bool FHOLHeroAbilityInMatchTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroAbilityMatch;
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
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("heroes"), Heroes)
		&& TestNotNull(TEXT("entities"), EntitySubsystem))
	{
		Heroes->BindSteps();
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
		// As HeroesOfLiberty.HeroXP.KillInMatch: born again until its identifier is a multiple of 4 (one search step of 4).
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
		// A Spearman of slot 0 at Location and a Man-at-arms of slot 1 1.5 m from it; the first hit it takes (5 s at most);
		// both go after it (-1: none).
		const auto FirstHit = [&](const FVector& Location)
		{
			const uint32 Victim = Place(Spearman, 0, Location);
			const uint32 Attacker = Place(ManAtArms, 1, Location + FVector(150.0, 0.0, 0.0));
			FHOLUnitStateFragment* State = StateOf(Victim);
			float Taken = -1.f;
			if (State && Attacker != 0)
			{
				const float Before = State->CurrentHealth;
				for (int32 Steps = 0; Steps < FMath::CeilToInt(5.f / StepSeconds); ++Steps)
				{
					RunStep();
					State = StateOf(Victim);
					if (State && State->CurrentHealth < Before)
					{
						Taken = Before - State->CurrentHealth;
						break;
					}
				}
			}
			const uint32 Gone[] = { Victim, Attacker };
			Units->RequestDestroyUnits(Gone);
			Units->FlushDestructions();
			return Taken;
		};

		const uint32 HeroId = Place(Hero, 0, FVector(0.0, 0.0, 0.0));
		if (TestTrue(TEXT("slot 0's hero born"), HeroId != 0))
		{
			RunStep(); // his sheet: alive, level 1
			TestEqual(TEXT("far from the hero (50 m): the whole 12"), FirstHit(FVector(5000.0, 0.0, 0.0)), 12.f, Tolerance);
			TestEqual(TEXT("11 m from the hero: aura, 10.2"), FirstHit(FVector(0.0, 1100.0, 0.0)), 10.2f, Tolerance);
			TestEqual(TEXT("13 m from the hero: the whole 12"), FirstHit(FVector(0.0, 1300.0, 0.0)), 12.f, Tolerance);

			// The cry (J8c-Q2): the units within 12 m when it is cast, 15 s; then 60 s of cool-down.
			const uint32 Near = Place(Spearman, 0, FVector(0.0, -1100.0, 0.0));
			const uint32 Far = Place(Spearman, 0, FVector(0.0, -1300.0, 0.0));
			TestTrue(TEXT("the cry cast"), Heroes->CastCloseRanks(0));
			TestFalse(TEXT("... not again during its cool-down"), Heroes->CastCloseRanks(0));
			RunStep();
			const FHOLCombatFragment* NearCombat = Near != 0 ? EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Units->FindEntity(Near)) : nullptr;
			const FHOLCombatFragment* FarCombat = Far != 0 ? EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Units->FindEntity(Far)) : nullptr;
			if (TestTrue(TEXT("two Spearmen 11 m and 13 m from the hero"), NearCombat && FarCombat))
			{
				TestEqual(TEXT("the one at 11 m under the cry for 15 s"), NearCombat->CloseRanksMs, HOLHeroAbility::CloseRanksDurationMs);
				TestEqual(TEXT("the one at 13 m not"), FarCombat->CloseRanksMs, 0);
			}
			const uint32 Cried[] = { Near, Far };
			Units->RequestDestroyUnits(Cried);
			Units->FlushDestructions();
			// A unit under the cry, hit in the aura: -40 %; the cry is on the unit, the aura on the place.
			FHOLUnitStateFragment* HeroState = StateOf(HeroId);
			TestTrue(TEXT("the cry cast again after 60 s"), [&] { for (int32 Steps = 0; Steps < 1200; ++Steps) { RunStep(); } return Heroes->CastCloseRanks(0); }());
			const uint32 Cry = Place(Spearman, 0, FVector(0.0, 1100.0, 0.0));
			const uint32 CryAttacker = Place(ManAtArms, 1, FVector(150.0, 1100.0, 0.0));
			RunStep();
			FHOLUnitStateFragment* CryState = StateOf(Cry);
			if (TestTrue(TEXT("a Spearman under the cry and its Man-at-arms"), CryState != nullptr && CryAttacker != 0 && HeroState != nullptr))
			{
				float Before = CryState->CurrentHealth;
				float Taken = -1.f;
				for (int32 Steps = 0; Steps < 100 && Taken < 0.f; ++Steps)
				{
					RunStep();
					CryState = StateOf(Cry);
					Taken = CryState && CryState->CurrentHealth < Before ? Before - CryState->CurrentHealth : -1.f;
				}
				TestEqual(TEXT("under aura and cry: 7.2"), Taken, 7.2f, Tolerance);
			}
			const uint32 CryGone[] = { Cry, CryAttacker };
			Units->RequestDestroyUnits(CryGone);
			Units->FlushDestructions();

			// The banner (J8c-Q3): level 4, 16 m from the hero at most, 45 s; 120 s of cool-down.
			TestFalse(TEXT("no banner at level 1"), Heroes->PlantBanner(0, FVector(1600.0, 0.0, 0.0)));
			Heroes->SetXP(0, HOLHeroXP::GetLevelXP(4));
			RunStep();
			TestFalse(TEXT("no banner 16.5 m away"), Heroes->PlantBanner(0, FVector(1650.0, 0.0, 0.0)));
			TestTrue(TEXT("a banner 16 m away"), Heroes->PlantBanner(0, FVector(1600.0, 0.0, 0.0)));
			TestFalse(TEXT("... not another during its cool-down"), Heroes->PlantBanner(0, FVector(-1600.0, 0.0, 0.0)));
			// Its standard drawn for all (J8c.4 loop 4): one instance of SM_Banner, no shadow, at the place planted, from the next frame.
			UHOLBannerSubsystem* BannerDisplay = World->GetSubsystem<UHOLBannerSubsystem>();
			if (TestNotNull(TEXT("the banner display in a game world"), BannerDisplay))
			{
				BannerDisplay->Tick(0.01f);
				if (TestEqual(TEXT("one banner drawn once planted"), BannerDisplay->GetShown().Num(), 1))
				{
					TestEqual(TEXT("... of slot 0"), BannerDisplay->GetShown()[0].Slot, 0);
					TestEqual(TEXT("... planted 16 m away: x"), BannerDisplay->GetShown()[0].Location.X, 1600.0, 1.0);
					TestEqual(TEXT("... y"), BannerDisplay->GetShown()[0].Location.Y, 0.0, 1.0);
				}
				const UInstancedStaticMeshComponent* BannerInstances = BannerDisplay->GetInstances();
				TestTrue(TEXT("one instance, without shadow"), BannerInstances && BannerInstances->GetInstanceCount() == 1 && !BannerInstances->CastShadow);
			}
			TestEqual(TEXT("26 m from the hero, 10 m from the banner: aura, 10.2"), FirstHit(FVector(2600.0, 0.0, 0.0)), 10.2f, Tolerance);
			TestEqual(TEXT("11 m from both: one aura, 10.2"), FirstHit(FVector(800.0, 800.0, 0.0)), 10.2f, Tolerance);

			// His death: his aura goes, the banner's stays.
			Units->KillUnitsOfOwner(0, Hero);
			RunStep();
			TestEqual(TEXT("hero dead, 11 m from where he fell: the whole 12"), FirstHit(FVector(-1100.0, 0.0, 0.0)), 12.f, Tolerance);
			TestEqual(TEXT("hero dead, 10 m from the banner: aura, 10.2"), FirstHit(FVector(2600.0, 0.0, 0.0)), 10.2f, Tolerance);
			TestFalse(TEXT("no cry while he is dead"), Heroes->CastCloseRanks(0));

			// 45 s after it was planted, the banner is gone.
			for (int32 Steps = 0; Steps < 900; ++Steps)
			{
				RunStep();
			}
			FVector BannerLocation;
			TestFalse(TEXT("the banner gone after 45 s"), Heroes->GetBanner(0, BannerLocation));
			if (UHOLBannerSubsystem* FallenDisplay = World->GetSubsystem<UHOLBannerSubsystem>())
			{
				// The display reads the simulation's step index, which this test's RunStep does not advance: caught up with its own count.
				while (Simulation->GetStepIndex() < Step)
				{
					Simulation->AdvanceStepIndexForTests();
				}
				FallenDisplay->Tick(0.01f);
				TestEqual(TEXT("no banner drawn once it fell"), FallenDisplay->GetShown().Num(), 0);
				TestTrue(TEXT("... no instance left"), !FallenDisplay->GetInstances() || FallenDisplay->GetInstances()->GetInstanceCount() == 0);
			}
			TestEqual(TEXT("... 10 m from where it stood: the whole 12"), FirstHit(FVector(2600.0, 0.0, 0.0)), 12.f, Tolerance);
		}
	}
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
