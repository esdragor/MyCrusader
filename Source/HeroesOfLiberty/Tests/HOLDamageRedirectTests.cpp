// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLDamageRedirectRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace UE::HeroesOfLiberty::Tests::DamageRedirect
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	float ShareOf(const HOLDamageRedirect::FResult& Result, int32 KnightId)
	{
		for (const HOLDamageRedirect::FShare& Share : Result.Shares)
		{
			if (Share.KnightId == KnightId)
			{
				return Share.Damage;
			}
		}
		return 0.f;
	}

	float SharesSum(const HOLDamageRedirect::FResult& Result)
	{
		float Sum = 0.f;
		for (const HOLDamageRedirect::FShare& Share : Result.Shares)
		{
			Sum += Share.Damage;
		}
		return Sum;
	}
}

// Aube A2, D19 passive: who takes which part of a blow. Expected numbers by hand (damage 100, 30 %, radius 500 cm; the point (300, 400) is exactly
// 500 cm from the origin, a 3-4-5 triangle), none copied from the code under test. Conservation is checked as an EFFECT: Kept + shares = damage.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDamageRedirectRulesTest, "HeroesOfLiberty.DamageRedirect.Rules", UE::HeroesOfLiberty::Tests::DamageRedirect::Flags)

bool FHOLDamageRedirectRulesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::DamageRedirect;
	using namespace HOLDamageRedirect;
	constexpr int32 Radius = 500;
	constexpr int32 Percent = 30;
	const FKnight Knight{ 7, 0, 0, true };

	// 1. An ally on the edge: 30 % goes to the Knight, 70 % stays; the sum is conserved.
	{
		const FKnight Knights[] = { Knight };
		const FResult Result = Redirect(100.f, false, 1, 300, 400, Knights, Radius, Percent, ECumulMode::Nearest, 50);
		TestEqual(TEXT("the ally keeps 70"), Result.Kept, 70.f);
		TestEqual(TEXT("the Knight takes 30"), ShareOf(Result, 7), 30.f);
		TestEqual(TEXT("conservation: kept + shares = 100"), Result.Kept + SharesSum(Result), 100.f);
	}
	// 2. One centimetre out: no effect.
	{
		const FKnight Knights[] = { Knight };
		const FResult Result = Redirect(100.f, false, 1, 300, 400, Knights, Radius - 1, Percent, ECumulMode::Nearest, 50);
		TestEqual(TEXT("out of the radius: the ally keeps 100"), Result.Kept, 100.f);
		TestEqual(TEXT("out of the radius: no share"), Result.Shares.Num(), 0);
	}
	// 3. A hero is never protected; a dead Knight, and the Knight itself, redirect nothing.
	{
		const FKnight Knights[] = { Knight };
		const FResult Hero = Redirect(100.f, true, 1, 100, 0, Knights, Radius, Percent, ECumulMode::Nearest, 50);
		TestEqual(TEXT("a hero keeps 100"), Hero.Kept, 100.f);
		const FKnight Dead{ 8, 0, 0, false };
		const FKnight DeadOnly[] = { Dead };
		const FResult DeadResult = Redirect(100.f, false, 1, 100, 0, DeadOnly, Radius, Percent, ECumulMode::Nearest, 50);
		TestEqual(TEXT("a dead Knight takes nothing"), DeadResult.Kept, 100.f);
		const FKnight Knights2[] = { Knight };
		const FResult Self = Redirect(100.f, false, 7, 0, 0, Knights2, Radius, Percent, ECumulMode::Nearest, 50);
		TestEqual(TEXT("the Knight does not redirect its own damage"), Self.Kept, 100.f);
	}
	// 4. Two Knights, Nearest: only the nearer one takes the percent (never twice).
	{
		const FKnight Near{ 7, 100, 0, true };
		const FKnight Far{ 3, 0, 300, true };
		const FKnight Knights[] = { Far, Near };
		const FResult Result = Redirect(100.f, false, 1, 150, 0, Knights, Radius, Percent, ECumulMode::Nearest, 50);
		TestEqual(TEXT("Nearest: the nearer Knight takes 30"), ShareOf(Result, 7), 30.f);
		TestEqual(TEXT("Nearest: the farther Knight takes nothing"), ShareOf(Result, 3), 0.f);
		TestEqual(TEXT("Nearest: the ally keeps 70"), Result.Kept, 70.f);
	}
	// 5. Two Knights, CappedSum, 30 % each, cap 50 %: 50 redirected (not 60), 25 each, 50 kept.
	{
		const FKnight A{ 7, 100, 0, true };
		const FKnight B{ 3, 0, 300, true };
		const FKnight Knights[] = { A, B };
		const FResult Result = Redirect(100.f, false, 1, 150, 0, Knights, Radius, Percent, ECumulMode::CappedSum, 50);
		TestEqual(TEXT("CappedSum: the ally keeps 50"), Result.Kept, 50.f);
		TestEqual(TEXT("CappedSum: the first Knight takes 25"), ShareOf(Result, 7), 25.f);
		TestEqual(TEXT("CappedSum: the second Knight takes 25"), ShareOf(Result, 3), 25.f);
		TestEqual(TEXT("CappedSum: conservation"), Result.Kept + SharesSum(Result), 100.f);
	}
	// 6. The active version (100 %) takes everything; a percent above 100 is clamped; zero or negative damage redirects nothing.
	{
		const FKnight Knights[] = { Knight };
		const FResult Active = Redirect(80.f, false, 1, 100, 0, Knights, Radius, 100, ECumulMode::Nearest, 50);
		TestEqual(TEXT("100 %: the ally keeps 0"), Active.Kept, 0.f);
		TestEqual(TEXT("100 %: the Knight takes 80"), ShareOf(Active, 7), 80.f);
		const FResult Clamped = Redirect(80.f, false, 1, 100, 0, Knights, Radius, 250, ECumulMode::Nearest, 50);
		TestEqual(TEXT("250 % clamped to 100: the Knight takes 80"), ShareOf(Clamped, 7), 80.f);
		const FResult Zero = Redirect(0.f, false, 1, 100, 0, Knights, Radius, Percent, ECumulMode::Nearest, 50);
		TestEqual(TEXT("zero damage: nothing"), Zero.Shares.Num(), 0);
	}
	return true;
}

// D864 + D865: the passive's percent and radius come from the unit table (columns DamageRedirectPercent / DamageRedirectRadius of
// FHOLUnitTypeRow -> ToConstants -> FHOLUnitTypeConstants), not from a literal. Real chain: a row filled as the CSV would fill it (30, 800), its
// constants, then the rule. Expected numbers by hand: damage 100, a Knight 800 cm away redirects 30, one at 801 cm nothing, and of two Knights
// only the nearer takes the 30 (D864, Nearest: never 60).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDamageRedirectTableTest, "HeroesOfLiberty.DamageRedirect.Table", UE::HeroesOfLiberty::Tests::DamageRedirect::Flags)

bool FHOLDamageRedirectTableTest::RunTest(const FString& Parameters)
{
	using namespace HOLDamageRedirect;
	FHOLUnitTypeRow Row;
	Row.DamageRedirectPercent = 30;
	Row.DamageRedirectRadius = 800.f;
	const FHOLUnitTypeConstants Constants = Row.ToConstants(TEXT("VirtuousKnight"));
	TestEqual(TEXT("the table's percent reaches the constants"), Constants.DamageRedirectPercent, 30);
	TestEqual(TEXT("the table's radius reaches the constants"), Constants.DamageRedirectRadius, 800.f);

	const int32 RadiusCm = FMath::RoundToInt(Constants.DamageRedirectRadius);
	const FKnight Knight{ 7, 0, 0, true };
	const FKnight Knights[] = { Knight };
	const FResult OnEdge = Redirect(100.f, false, 1, 800, 0, Knights, RadiusCm, Constants.DamageRedirectPercent, DefaultCumulMode, 50);
	TestEqual(TEXT("an ally 8 m away: the Knight takes 30"), UE::HeroesOfLiberty::Tests::DamageRedirect::ShareOf(OnEdge, 7), 30.f);
	TestEqual(TEXT("an ally 8 m away: it keeps 70"), OnEdge.Kept, 70.f);
	const FResult Out = Redirect(100.f, false, 1, 801, 0, Knights, RadiusCm, Constants.DamageRedirectPercent, DefaultCumulMode, 50);
	TestEqual(TEXT("an ally 801 cm away: it keeps 100"), Out.Kept, 100.f);

	const FKnight Near{ 7, 100, 0, true };
	const FKnight Far{ 3, 0, 300, true };
	const FKnight Two[] = { Far, Near };
	const FResult Nearest = Redirect(100.f, false, 1, 150, 0, Two, RadiusCm, Constants.DamageRedirectPercent, DefaultCumulMode, 50);
	TestEqual(TEXT("D864: only the nearest Knight takes the part"), UE::HeroesOfLiberty::Tests::DamageRedirect::ShareOf(Nearest, 7), 30.f);
	TestEqual(TEXT("D864: the farther Knight takes nothing"), UE::HeroesOfLiberty::Tests::DamageRedirect::ShareOf(Nearest, 3), 0.f);

	// A row with the columns left at their default redirects nothing (every unit but the Knight).
	const FHOLUnitTypeConstants Plain = FHOLUnitTypeRow().ToConstants(TEXT("Plain"));
	const FResult None = Redirect(100.f, false, 1, 100, 0, Knights, FMath::RoundToInt(Plain.DamageRedirectRadius), Plain.DamageRedirectPercent, DefaultCumulMode, 50);
	TestEqual(TEXT("default columns: nothing redirected"), None.Kept, 100.f);
	return true;
}

// D864 + D865 on the REAL Mass chain (SAURON-1120pc): a Virtuous Knight and an allied Man-at-arms of slot 0, a Spearman of slot 1 (health
// raised so that it never dies) that hits the ally through the combat processor. The Knight stands 3 m from the ally, inside its
// DamageRedirectRadius (8 m, read from the unit table). Measured EFFECT on the first blow (the Knight has not reached the fight yet, so it
// takes nothing by its own fight): the Knight's health goes down by DamageRedirectPercent % (30, read from the table) of the blow, the
// ally keeps the rest. Not a stored value: both losses are read off the fragments after the blow.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDamageRedirectChainTest, "HeroesOfLiberty.DamageRedirect.Chain", UE::HeroesOfLiberty::Tests::DamageRedirect::Flags)

bool FHOLDamageRedirectChainTest::RunTest(const FString& Parameters)
{
	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	const FHOLUnitTypeRow* KnightRow = UnitTypes ? UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("VirtuousKnight"), TEXT("Damage redirect test"), false) : nullptr;
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	if (!TestNotNull(TEXT("VirtuousKnight in the unit type table"), KnightRow) || !TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled))
	{
		return false;
	}
	const float ExpectedShare = static_cast<float>(KnightRow->DamageRedirectPercent) / 100.f;
	TestTrue(TEXT("the Knight's row redirects something"), KnightRow->DamageRedirectPercent > 0 && KnightRow->DamageRedirectRadius >= 300.f);
	const int32 FormerEnabled = CombatEnabled->GetInt();
	CombatEnabled->Set(1, ECVF_SetByCode);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("entities"), EntitySubsystem))
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
		// As HeroesOfLiberty.Combat.DamnedWarrior.Rage: born again until its identifier is a multiple of 4 (one search step of 4).
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

		const uint32 Knight = Place(FName(TEXT("VirtuousKnight")), 0, FVector(0.0, 0.0, 0.0));
		const uint32 Ally = Place(FName(TEXT("ManAtArms")), 0, FVector(300.0, 0.0, 0.0));
		const uint32 Striker = Place(FName(TEXT("Spearman")), 1, FVector(400.0, 0.0, 0.0));
		FHOLUnitStateFragment* KnightState = StateOf(Knight);
		FHOLUnitStateFragment* AllyState = StateOf(Ally);
		FHOLUnitStateFragment* StrikerState = StateOf(Striker);
		if (TestTrue(TEXT("a Knight and an ally of slot 0, a Spearman of slot 1"), KnightState && AllyState && StrikerState))
		{
			StrikerState->CurrentHealth = 100000.f;
			const float KnightBefore = KnightState->CurrentHealth;
			const float AllyBefore = AllyState->CurrentHealth;
			bool bBlow = false;
			for (int32 Steps = 0; Steps < FMath::CeilToInt(4.f / StepSeconds) && !bBlow; ++Steps)
			{
				RunStep();
				bBlow = StateOf(Ally)->CurrentHealth < AllyBefore;
			}
			if (TestTrue(TEXT("the Spearman strikes the ally within 4 s"), bBlow))
			{
				const float AllyLoss = AllyBefore - StateOf(Ally)->CurrentHealth;
				const float KnightLoss = KnightBefore - StateOf(Knight)->CurrentHealth;
				TestTrue(TEXT("the Knight lost health: the blow was redirected to it"), KnightLoss > 0.f);
				TestEqual(TEXT("the Knight took DamageRedirectPercent % of the blow (the table's 30)"), KnightLoss / FMath::Max(AllyLoss + KnightLoss, 0.001f), ExpectedShare, 0.01f);
				TestEqual(TEXT("the ally kept the rest"), AllyLoss / FMath::Max(AllyLoss + KnightLoss, 0.001f), 1.f - ExpectedShare, 0.01f);
			}
		}
	}

	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
