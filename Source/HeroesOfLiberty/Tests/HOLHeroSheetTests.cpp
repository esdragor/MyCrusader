// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerState.h"
#include "Match/HOLStartRules.h"
#include "Misc/AutomationTest.h"
#include "Net/UnrealNetwork.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLHeroSheet.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
#include "Simulation/HOLUnitView.h"

#if WITH_DEV_AUTOMATION_TESTS

// J8a.3 checks (J8-Q5, J8a3-Q1 = A, J8a3-Q2 = A): the hero sheet made from what the server knows, and its two carriers
// (public part in the game state, private part in the owner's player state).
namespace UE::HeroesOfLiberty::Tests::HeroSheet
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FHOLUnitTypeRow* FindRow(const TCHAR* Name)
	{
		const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
		return UnitTypes ? UnitTypes->FindRow<FHOLUnitTypeRow>(Name, TEXT("Hero sheet test"), false) : nullptr;
	}

	FString Describe(const FHOLHeroSheets& Sheets)
	{
		return FString::Printf(TEXT("state %d, level %d, %d HP, XP %d, death step %d"), static_cast<int32>(Sheets.Public.State),
			Sheets.Public.Level, Sheets.Public.Health, Sheets.Private.XP, Sheets.Private.DeathStep);
	}

	/** The replication condition GetLifetimeReplicatedProps gives Property of Object; COND_Max when not replicated. */
	ELifetimeCondition FindCondition(const UObject& Object, const FName Property)
	{
		// Outside a net driver the class's rep indices may not be set yet (all 0: every property would look the same).
		UClass* Class = Object.GetClass();
		Class->SetUpRuntimeReplicationData();
		const FProperty* Found = Class->FindPropertyByName(Property);
		if (!Found)
		{
			return COND_Max;
		}
		TArray<FLifetimeProperty> Props;
		Object.GetLifetimeReplicatedProps(Props);
		for (const FLifetimeProperty& Prop : Props)
		{
			if (Prop.RepIndex == Found->RepIndex)
			{
				return Prop.Condition;
			}
		}
		return COND_Max;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroSheetRuleTest, "HeroesOfLiberty.Hero.SheetRule", UE::HeroesOfLiberty::Tests::HeroSheet::Flags)

// From birth to after the body is gone, with the real Hero row and a real Man-at-arms hit on him (x0.3, D17).
bool FHOLHeroSheetRuleTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroSheet;
	const FHOLUnitTypeRow* HeroRow = FindRow(TEXT("Hero"));
	const FHOLUnitTypeRow* ManAtArmsRow = FindRow(TEXT("ManAtArms"));
	if (!TestNotNull(TEXT("The table has a Hero"), HeroRow) || !TestNotNull(TEXT("... a Man-at-arms"), ManAtArmsRow))
	{
		return false;
	}
	const FHOLUnitTypeConstants Hero = HeroRow->ToConstants(FName(TEXT("Hero")));
	const FHOLUnitTypeConstants ManAtArms = ManAtArmsRow->ToConstants(FName(TEXT("ManAtArms")));

	// No hero ever: None, nothing else.
	const FHOLHeroSheets NoHero = HOLHeroSheet::Make(FHOLHeroFacts(), FHOLHeroSheets());
	TestTrue(FString::Printf(TEXT("No hero: None (%s)"), *Describe(NoHero)), NoHero.Public.State == EHOLHeroState::None);
	TestEqual(TEXT("... level 0"), static_cast<int32>(NoHero.Public.Level), 0);
	TestEqual(TEXT("... 0 HP"), static_cast<int32>(NoHero.Public.Health), 0);
	TestEqual(TEXT("... no death step"), NoHero.Private.DeathStep, -1);

	// Birth: Alive, level 1, full health from the table.
	FHOLHeroFacts Facts;
	Facts.bPresent = true;
	Facts.Health = static_cast<float>(Hero.MaxHealth);
	const FHOLHeroSheets Born = HOLHeroSheet::Make(Facts, NoHero);
	TestTrue(FString::Printf(TEXT("Born: Alive (%s)"), *Describe(Born)), Born.Public.State == EHOLHeroState::Alive);
	TestEqual(TEXT("... level 1"), static_cast<int32>(Born.Public.Level), 1);
	TestEqual(TEXT("... 450 HP (the Hero row)"), static_cast<int32>(Born.Public.Health), 450);
	TestEqual(TEXT("... no death step"), Born.Private.DeathStep, -1);

	// A Man-at-arms' hit: 12 x 0.3 = 3.6, 446.4 left, shown 447.
	Facts.Health -= HOLCombat::GetDamage(ManAtArms, Hero);
	TestEqual(TEXT("446.4 left"), Facts.Health, 446.4f, 1.e-3f);
	FHOLHeroSheets Hurt = Born;
	Hurt.Private.XP = 7;
	Hurt = HOLHeroSheet::Make(Facts, Hurt);
	TestTrue(FString::Printf(TEXT("Hurt: Alive (%s)"), *Describe(Hurt)), Hurt.Public.State == EHOLHeroState::Alive);
	TestEqual(TEXT("... 447 HP (rounded up)"), static_cast<int32>(Hurt.Public.Health), 447);
	TestEqual(TEXT("... his XP kept"), Hurt.Private.XP, 7);

	// He falls at step 120: Dead, 0 HP, step 120.
	Facts.Health = -2.f;
	Facts.DeathStep = 120;
	const FHOLHeroSheets Dying = HOLHeroSheet::Make(Facts, Hurt);
	TestTrue(FString::Printf(TEXT("Dying: Dead (%s)"), *Describe(Dying)), Dying.Public.State == EHOLHeroState::Dead);
	TestEqual(TEXT("... 0 HP"), static_cast<int32>(Dying.Public.Health), 0);
	TestEqual(TEXT("... level 1"), static_cast<int32>(Dying.Public.Level), 1);
	TestEqual(TEXT("... death step 120"), Dying.Private.DeathStep, 120);
	TestEqual(TEXT("... his XP kept"), Dying.Private.XP, 7);

	// His body destroyed: still Dead, still step 120.
	const FHOLHeroSheets Gone = HOLHeroSheet::Make(FHOLHeroFacts(), Dying);
	TestTrue(FString::Printf(TEXT("Body gone: still Dead (%s)"), *Describe(Gone)), Gone.Public.State == EHOLHeroState::Dead);
	TestEqual(TEXT("... 0 HP"), static_cast<int32>(Gone.Public.Health), 0);
	TestEqual(TEXT("... death step 120"), Gone.Private.DeathStep, 120);
	TestTrue(TEXT("... the same sheet (nothing to send)"), Gone.Public == Dying.Public && Gone.Private == Dying.Private);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroSheetCarriersTest, "HeroesOfLiberty.Hero.SheetCarriers", UE::HeroesOfLiberty::Tests::HeroSheet::Flags)

// The real game state and player state in a test world: set on change only (the delegate heard once per change), and the
// private part replicated to its owner only.
bool FHOLHeroSheetCarriersTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroSheet;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	AHOLPlayerState* Player = World->SpawnActor<AHOLPlayerState>();
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("player state"), Player))
	{
		int32 PublicHeard = 0;
		int32 PrivateHeard = 0;
		State->OnHeroSheetsChanged.AddLambda([&PublicHeard] { ++PublicHeard; });
		Player->OnHeroPrivateChanged.AddLambda([&PrivateHeard] { ++PrivateHeard; });

		// Public part.
		TestTrue(TEXT("Slot 2 starts without hero"), State->GetHeroSheet(2) == FHOLHeroSheet());
		TestFalse(TEXT("Setting the same (None) sheet: unchanged"), State->SetHeroSheet(2, FHOLHeroSheet()));
		FHOLHeroSheet Alive;
		Alive.Level = 1;
		Alive.Health = 450;
		Alive.State = EHOLHeroState::Alive;
		TestTrue(TEXT("A different sheet: changed"), State->SetHeroSheet(2, Alive));
		TestTrue(TEXT("... stored"), State->GetHeroSheet(2) == Alive);
		TestTrue(TEXT("... the other slots untouched"), State->GetHeroSheet(1) == FHOLHeroSheet() && State->GetHeroSheet(3) == FHOLHeroSheet());
		TestFalse(TEXT("The same again: unchanged"), State->SetHeroSheet(2, Alive));
		Alive.Health = 447;
		TestTrue(TEXT("447 HP: changed"), State->SetHeroSheet(2, Alive));
		TestFalse(TEXT("Out of range: refused"), State->SetHeroSheet(FHOLMatchConfig::MaxSlots, Alive));
		TestEqual(TEXT("Heard once per change"), PublicHeard, 2);
		TestTrue(TEXT("HeroSheets replicated to all"), FindCondition(*State, TEXT("HeroSheets")) == COND_None);

		// Private part.
		TestFalse(TEXT("The same (default) private sheet: unchanged"), Player->SetHeroPrivate(FHOLHeroPrivateSheet()));
		FHOLHeroPrivateSheet Dead;
		Dead.XP = 7;
		Dead.DeathStep = 120;
		TestTrue(TEXT("A different private sheet: changed"), Player->SetHeroPrivate(Dead));
		TestTrue(TEXT("... stored"), Player->GetHeroPrivate() == Dead);
		TestFalse(TEXT("The same again: unchanged"), Player->SetHeroPrivate(Dead));
		TestEqual(TEXT("Heard once per change"), PrivateHeard, 1);
		TestTrue(TEXT("HeroPrivate replicated to its owner only (COND_OwnerOnly)"), FindCondition(*Player, TEXT("HeroPrivate")) == COND_OwnerOnly);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroSheetInCombatTest, "HeroesOfLiberty.Hero.SheetInCombat", UE::HeroesOfLiberty::Tests::HeroSheet::Flags)

// The whole chain on the server (J8a.3 loop 3): the real combat processor hits slot 0's hero in a test world, the step's
// end (OnStepEnded) makes UHOLHeroSubsystem write the sheets: the game state's HP is the unit view's at every step, the
// private part reaches slot 0's player state; killed, he is Dead with his death step, still after his body is destroyed.
// Slot 2's hero has no player state (an AI slot): his sheet is public all the same, his private part stays on the server.
bool FHOLHeroSheetInCombatTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroSheet;
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

	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);
	AHOLPlayerState* Player = World->SpawnActor<AHOLPlayerState>();
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>();
	if (TestNotNull(TEXT("game state"), GameState) && TestNotNull(TEXT("player state"), Player) && TestNotNull(TEXT("units"), Units)
		&& TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("entities"), EntitySubsystem) && TestNotNull(TEXT("heroes"), Heroes))
	{
		Player->SetMatchSlot(0, 0);
		if (!GameState->PlayerArray.Contains(Player))
		{
			GameState->AddPlayerState(Player);
		}
		TestTrue(TEXT("The hero subsystem follows the steps (not a client)"), Heroes->BindSteps());

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
		// As HeroesOfLiberty.Hero.ResistanceInCombat: born again until its identifier is a multiple of 4 (the test world's
		// step index stays 0 and a unit searches one step of 4 by its identifier).
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
		const auto GetHealth = [Units](const uint32 Id)
		{
			FHOLUnitView View;
			return Units->GetUnitView(Id, View) ? View.Health : -1;
		};
		const auto Sheet = [GameState](const int32 Slot) { return GameState->GetHeroSheet(Slot); };

		// Radii 72 and 60, reach 70: 150 cm apart, in reach of each other. Slot 2's hero 60 m away, out of the fight.
		const uint32 Hero = Place(FHOLStartRules::HeroTypeName, 0, FVector(0.0, 0.0, 0.0));
		const uint32 Foe = Place(FName(TEXT("ManAtArms")), 1, FVector(150.0, 0.0, 0.0));
		const uint32 AIHero = Place(FHOLStartRules::HeroTypeName, 2, FVector(6000.0, 0.0, 0.0));
		if (TestTrue(TEXT("two heroes and a Man-at-arms born"), Hero != 0 && Foe != 0 && AIHero != 0))
		{
			TestEqual(TEXT("FindHeroUnit(0): slot 0's hero"), Units->FindHeroUnit(0), Hero);
			TestEqual(TEXT("FindHeroUnit(2): slot 2's hero"), Units->FindHeroUnit(2), AIHero);
			TestEqual(TEXT("FindHeroUnit(1): none (a Man-at-arms is no hero)"), Units->FindHeroUnit(1), 0u);

			// While the Man-at-arms hits him (and falls to his hits): the sheet's HP is the view's at every step's end.
			int32 Mismatches = 0;
			int32 LowestSheet = MAX_int32;
			const int32 Steps = FMath::CeilToInt(4.f / StepSeconds);
			for (int32 Index = 0; Index < Steps; ++Index)
			{
				RunStep();
				const int32 ViewHealth = GetHealth(Hero);
				const FHOLHeroSheet Public = Sheet(0);
				LowestSheet = FMath::Min(LowestSheet, static_cast<int32>(Public.Health));
				if (static_cast<int32>(Public.Health) != ViewHealth || Public.State != EHOLHeroState::Alive || Public.Level != 1)
				{
					if (++Mismatches <= 3)
					{
						AddError(FString::Printf(TEXT("Step %d: sheet state %d level %d %d HP, view %d HP"), Index, static_cast<int32>(Public.State),
							Public.Level, Public.Health, ViewHealth));
					}
				}
			}
			TestEqual(TEXT("The sheet's HP is the view's at every step"), Mismatches, 0);
			TestTrue(FString::Printf(TEXT("... and he was hit (lowest sheet HP %d, below 450)"), LowestSheet), LowestSheet < 450 && LowestSheet > 0);
			TestEqual(TEXT("Slot 2's hero (AI, no player state): Alive in the game state"), static_cast<int32>(Sheet(2).State), static_cast<int32>(EHOLHeroState::Alive));
			TestEqual(TEXT("... full health"), static_cast<int32>(Sheet(2).Health), 450);
			TestEqual(TEXT("Slot 1 (no hero): None"), static_cast<int32>(Sheet(1).State), static_cast<int32>(EHOLHeroState::None));
			TestEqual(TEXT("Slot 0's private part in its player state: alive, no death step"), Player->GetHeroPrivate().DeathStep, -1);

			// XP (hol.Hero.SetXP): straight to the owner's player state; an AI slot's stays on the server.
			TestTrue(TEXT("SetXP(0, 7)"), Heroes->SetXP(0, 7));
			TestEqual(TEXT("... in slot 0's player state"), Player->GetHeroPrivate().XP, 7);
			TestTrue(TEXT("SetXP(2, 5): an AI slot"), Heroes->SetXP(2, 5));
			TestEqual(TEXT("... kept on the server"), Heroes->GetSheets(2).Private.XP, 5);
			TestEqual(TEXT("... slot 0's player state untouched"), Player->GetHeroPrivate().XP, 7);

			// Killed (hol.Hero.Kill): Dead at the step's end, his death step in his owner's private part.
			TestEqual(TEXT("KillUnitsOfOwner(0, Hero): one hero"), Units->KillUnitsOfOwner(0, FHOLStartRules::HeroTypeName), 1);
			RunStep();
			const FHOLCombatFragment* HeroCombat = EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Units->FindEntity(Hero));
			const int64 DeathStep = HeroCombat ? HeroCombat->DeathStep : -2;
			TestTrue(FString::Printf(TEXT("His body still there, a death step (%lld)"), DeathStep), HeroCombat && DeathStep >= 0);
			TestEqual(TEXT("Dead"), static_cast<int32>(Sheet(0).State), static_cast<int32>(EHOLHeroState::Dead));
			TestEqual(TEXT("... 0 HP"), static_cast<int32>(Sheet(0).Health), 0);
			TestEqual(TEXT("... his death step in slot 0's player state"), static_cast<int64>(Player->GetHeroPrivate().DeathStep), DeathStep);
			TestEqual(TEXT("... his XP kept"), Player->GetHeroPrivate().XP, 7);

			// His body destroyed (as the combat processor does DyingSteps later): still Dead, the same death step.
			const uint32 Gone[] = { Hero };
			Units->RequestDestroyUnits(Gone);
			Units->FlushDestructions();
			TestEqual(TEXT("FindHeroUnit(0) once his entity is destroyed: 0"), Units->FindHeroUnit(0), 0u);
			RunStep();
			TestEqual(TEXT("Body gone: still Dead"), static_cast<int32>(Sheet(0).State), static_cast<int32>(EHOLHeroState::Dead));
			TestEqual(TEXT("... 0 HP"), static_cast<int32>(Sheet(0).Health), 0);
			TestEqual(TEXT("... the same death step"), static_cast<int64>(Player->GetHeroPrivate().DeathStep), DeathStep);
			TestEqual(TEXT("Slot 2's hero still Alive"), static_cast<int32>(Sheet(2).State), static_cast<int32>(EHOLHeroState::Alive));
		}
	}

	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
