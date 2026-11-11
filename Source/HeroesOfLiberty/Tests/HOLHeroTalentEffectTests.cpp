// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Match/HOLForumMessages.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Misc/AutomationTest.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Presentation/HOLBannerSubsystem.h"
#include "Sound/SoundBase.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLHealProcessor.h"
#include "Simulation/HOLHeroAbilityRules.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLHeroXPRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// T4: the Captain's row-2 talent Boucliers levees (GDD 13.2, D111): +1 armour for the units in the Dawn Aura. A Spearman (armour 0) hit by a
// Man-at-arms (12): 10.2 in the aura (-15 %), and with the talent (12 - 1) x 0.85 = 9.35; out of the aura, no change.
// The talent is taken through the order queue (HeroesOfLiberty.HeroTalent.Orders), the hits come off the combat processor as in a match.
namespace UE::HeroesOfLiberty::Tests::HeroTalentEffect
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	const FName Hero(TEXT("Hero"));
	const FName Spearman(TEXT("Spearman"));
	const FName ManAtArms(TEXT("ManAtArms"));
	constexpr float TestTolerance = 1.e-3f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroTalentShieldsUpTest, "HeroesOfLiberty.HeroTalent.ShieldsUp", UE::HeroesOfLiberty::Tests::HeroTalentEffect::Flags)

bool FHOLHeroTalentShieldsUpTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroTalentEffect;
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
			RunStep();
			TestEqual(TEXT("no talent: 11 m from the hero, aura, 10.2"), FirstHit(FVector(0.0, 1100.0, 0.0)), 10.2f, TestTolerance);
			Heroes->SetXP(0, HOLHeroXP::GetLevelXP(2));
			RunStep();
			// The Captain's row-2 talent (family 1) by the order queue's own path: in the aura (11 m) -1 armour, out of it (13 m) nothing.
			TestTrue(TEXT("the Captain's talent taken"), Heroes->PickTalent(0, 1));
			TestEqual(TEXT("Boucliers levés, 11 m from the hero: (12 - 1) x 0.85 = 9.35"), FirstHit(FVector(0.0, 1100.0, 0.0)), 9.35f, TestTolerance);
			TestEqual(TEXT("... 13 m from the hero, out of the aura: the whole 12"), FirstHit(FVector(0.0, 1300.0, 0.0)), 12.f, TestTolerance);
		}
	}
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// D581 (TQ2 = A): the cry Serrez les rangs puts the units in a defensive formation: held in place (hol.Hero.CloseRanksSpeed, default 0 %)
// while it lasts; the Captain's row-4 talent Pas de l'Aube lets them advance at reduced speed (50 %) with their damage reduction. The
// combat processor writes the speed part of each unit (FHOLCombatFragment::CloseRanksSpeedPercent) that the movement processor applies.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroTalentDawnStepTest, "HeroesOfLiberty.HeroTalent.DawnStep", UE::HeroesOfLiberty::Tests::HeroTalentEffect::Flags)

bool FHOLHeroTalentDawnStepTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroTalentEffect;
	// The pure rule.
	TestEqual(TEXT("no cry: the whole speed"), HOLHeroAbility::GetCloseRanksSpeedPercent(false, false, 0), 100);
	TestEqual(TEXT("no cry, the talent: the whole speed"), HOLHeroAbility::GetCloseRanksSpeedPercent(false, true, 0), 100);
	TestEqual(TEXT("under the cry: held (the base setting)"), HOLHeroAbility::GetCloseRanksSpeedPercent(true, false, 0), 0);
	TestEqual(TEXT("under the cry, a base of 40 %"), HOLHeroAbility::GetCloseRanksSpeedPercent(true, false, 40), 40);
	TestEqual(TEXT("under the cry with Pas de l'Aube: half speed"), HOLHeroAbility::GetCloseRanksSpeedPercent(true, true, 0), HOLHeroAbility::DawnStepSpeedPercent);
	TestEqual(TEXT("... never below the base"), HOLHeroAbility::GetCloseRanksSpeedPercent(true, true, 80), 80);
	TestEqual(TEXT("a base of more than 100 % is 100"), HOLHeroAbility::GetCloseRanksSpeedPercent(true, false, 250), 100);

	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	IConsoleVariable* BaseSpeed = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Hero.CloseRanksSpeed"));
	if (!TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled) || !TestNotNull(TEXT("hol.Hero.CloseRanksSpeed"), BaseSpeed))
	{
		return false;
	}
	const int32 FormerEnabled = CombatEnabled->GetInt();
	const int32 FormerBase = BaseSpeed->GetInt();
	CombatEnabled->Set(1, ECVF_SetByCode);
	BaseSpeed->Set(0, ECVF_SetByCode);

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
		const auto CombatOf = [Units, &EntityManager](const uint32 Id)
		{
			const FMassEntityHandle Entity = Id != 0 ? Units->FindEntity(Id) : FMassEntityHandle();
			return Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Entity) : nullptr;
		};
		// The cry cast, then the speed part of a Spearman 11 m and one 13 m from the hero, read after the step of the cry.
		const auto Cry = [&](int32& OutNear, int32& OutFar, int32& OutAfter)
		{
			const uint32 Near = Place(Spearman, 0, FVector(0.0, -1100.0, 0.0));
			const uint32 Far = Place(Spearman, 0, FVector(0.0, -1300.0, 0.0));
			Heroes->ResetAbilities(0);
			TestTrue(TEXT("the cry cast"), Heroes->CastCloseRanks(0));
			RunStep();
			const FHOLCombatFragment* NearCombat = CombatOf(Near);
			const FHOLCombatFragment* FarCombat = CombatOf(Far);
			OutNear = NearCombat ? NearCombat->CloseRanksSpeedPercent : -1;
			OutFar = FarCombat ? FarCombat->CloseRanksSpeedPercent : -1;
			// Its 15 s over: the whole speed again.
			for (int32 Steps = 0; Steps < FMath::CeilToInt(HOLHeroAbility::CloseRanksDurationMs / 1000.f / StepSeconds) + 2; ++Steps)
			{
				RunStep();
			}
			OutAfter = NearCombat ? NearCombat->CloseRanksSpeedPercent : -1;
			const uint32 Gone[] = { Near, Far };
			Units->RequestDestroyUnits(Gone);
			Units->FlushDestructions();
		};

		const uint32 HeroId = Place(Hero, 0, FVector(0.0, 0.0, 0.0));
		if (TestTrue(TEXT("slot 0's hero born"), HeroId != 0))
		{
			RunStep();
			int32 Near = -2, Far = -2, After = -2;
			Cry(Near, Far, After);
			TestEqual(TEXT("no talent: the unit at 11 m under the cry is held"), Near, 0);
			TestEqual(TEXT("... the one at 13 m walks as ever"), Far, 100);
			TestEqual(TEXT("... the cry over, the whole speed again"), After, 100);
			BaseSpeed->Set(40, ECVF_SetByCode);
			Cry(Near, Far, After);
			TestEqual(TEXT("a base setting of 40 %: 40 under the cry"), Near, 40);
			BaseSpeed->Set(0, ECVF_SetByCode);
			// Level 4, the Captain's talents of rows 2 to 4 (family 1): Boucliers levés, Mains du guérisseur, Pas de l'Aube.
			Heroes->SetXP(0, HOLHeroXP::GetLevelXP(4));
			RunStep();
			TestTrue(TEXT("row 2"), Heroes->PickTalent(0, 1));
			TestTrue(TEXT("row 3"), Heroes->PickTalent(0, 1));
			Cry(Near, Far, After);
			TestEqual(TEXT("two Captain talents, not yet Pas de l'Aube: held"), Near, 0);
			TestTrue(TEXT("row 4, Pas de l'Aube"), Heroes->PickTalent(0, 1));
			Cry(Near, Far, After);
			TestEqual(TEXT("Pas de l'Aube: the unit at 11 m under the cry advances at half speed"), Near, HOLHeroAbility::DawnStepSpeedPercent);
			TestEqual(TEXT("... the one at 13 m walks as ever"), Far, 100);
			TestEqual(TEXT("... the cry over, the whole speed again"), After, 100);
		}
	}
	BaseSpeed->Set(FormerBase, ECVF_SetByCode);
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// T4b: the Captain's row-3 talent Mains du guérisseur (GDD 13.2, D111): the Monks in the Dawn Aura heal 20 % more. The health a Monk gives a
// wounded ally over 10 steps, through the real heal processor, the hero born by the unit subsystem and the talent taken by the order queue's
// own path (PickTalent): without the talent the base; with it, 1.2 x when the MONK stands within the aura (the patient may stand outside);
// the Monk out of the aura, or the hero dead with no banner: the base.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroTalentHealersHandsTest, "HeroesOfLiberty.HeroTalent.HealersHands", UE::HeroesOfLiberty::Tests::HeroTalentEffect::Flags)

bool FHOLHeroTalentHealersHandsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroTalentEffect;
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
		UHOLHealProcessor* Heal = NewObject<UHOLHealProcessor>(Simulation);
		Heal->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		int64 Step = 0;
		const auto RunStep = [Simulation, Units, Heal, &EntityManager, StepSeconds, &Step]
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Heal, ProcessingContext);
			Units->FlushDestructions();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		const auto Place = [Units](const FName Type, const int32 Owner, const FVector& Location)
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
			return After.Num() == 1 ? After[0] : 0u;
		};
		const auto StateOf = [Units, &EntityManager](const uint32 Id)
		{
			const FMassEntityHandle Entity = Id != 0 ? Units->FindEntity(Id) : FMassEntityHandle();
			return Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
		};
		// A Monk of slot 0 at Location and a Spearman of slot 0 (health 1) 1.5 m from it: the health the Monk gives it in 10 steps (-1: none born).
		const auto Gain = [&](const FVector& Location)
		{
			const uint32 Monk = Place(FName(TEXT("Monk")), 0, Location);
			const uint32 Patient = Place(Spearman, 0, Location + FVector(150.0, 0.0, 0.0));
			float Gained = -1.f;
			if (Monk != 0 && StateOf(Patient))
			{
				StateOf(Patient)->CurrentHealth = 1.f;
				for (int32 Steps = 0; Steps < 10; ++Steps)
				{
					RunStep();
				}
				Gained = StateOf(Patient) ? StateOf(Patient)->CurrentHealth - 1.f : -1.f;
			}
			const uint32 Gone[] = { Monk, Patient };
			Units->RequestDestroyUnits(Gone);
			Units->FlushDestructions();
			return Gained;
		};

		const uint32 HeroId = Place(Hero, 0, FVector(0.0, 0.0, 0.0));
		if (TestTrue(TEXT("slot 0's hero born"), HeroId != 0))
		{
			RunStep();
			const float Base = Gain(FVector(0.0, 1100.0, 0.0));
			TestTrue(TEXT("no talent: a Monk heals (the base gain is positive)"), Base > 0.f);
			// Level 3, the Captain's talents of rows 2 and 3 (family 1): Boucliers levés, Mains du guérisseur.
			Heroes->SetXP(0, HOLHeroXP::GetLevelXP(3));
			RunStep();
			TestTrue(TEXT("row 2"), Heroes->PickTalent(0, 1));
			TestEqual(TEXT("row 2 only: the base"), Gain(FVector(0.0, 1100.0, 0.0)), Base, TestTolerance);
			TestTrue(TEXT("row 3, Mains du guérisseur"), Heroes->PickTalent(0, 1));
			TestEqual(TEXT("Mains du guérisseur, the Monk 11 m from the hero (in the aura): 1.2 x the base"), Gain(FVector(0.0, 1100.0, 0.0)), Base * 1.2f, Base * 0.01f);
			TestEqual(TEXT("... the Monk 13 m from the hero, out of the aura: the base"), Gain(FVector(0.0, 1300.0, 0.0)), Base, TestTolerance);
			if (FHOLUnitStateFragment* HeroState = StateOf(HeroId))
			{
				HeroState->CurrentHealth = 0.f;
			}
			TestEqual(TEXT("... the hero dead, no banner: the base"), Gain(FVector(0.0, 1100.0, 0.0)), Base, TestTolerance);
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
