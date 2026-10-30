// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLCommandSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLMatchConfig.h"
#include "MassEntitySubsystem.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLHeroAbilityRules.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLHeroXPRules.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// J8c.3: the players' ability orders (J8c-Q2, Q3) through the order queue to the hero subsystem, and what the sheets carry
// (J8-Q5: the cool-downs to the owner, the banner and the cries to all), numbers written from the decisions.
namespace UE::HeroesOfLiberty::Tests::HeroAbilityOrders
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroAbilityOrdersTest, "HeroesOfLiberty.HeroAbility.Orders", UE::HeroesOfLiberty::Tests::HeroAbilityOrders::Flags)

// Slot 0's hero born; the steps' ends broadcast by hand (no unit moves, no combat). A cry ordered: at the next step's end
// the cry is cast, his private sheet ready again 1 200 steps later (60 s at 20 Hz), his public sheet one cry more; ordered
// again: refused, nothing changes. A banner at level 1: refused. At level 4, 16 m away: planted, ready again 2 400 steps
// later, standing 900 steps (45 s), its place in the public sheet. An order of no player's slot: dropped.
bool FHOLHeroAbilityOrdersTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	if (TestTrue(TEXT("test world ready"), Units && Simulation && Heroes && Commands))
	{
		Heroes->BindSteps();
		int64 Step = 0;
		const auto EndStep = [Simulation, &Step]
		{
			++Step;
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		FHOLUnitSpawnRequest Request;
		Request.TypeName = FName(TEXT("Hero"));
		Request.Count = 1;
		Request.OwnerPlayer = 0;
		Request.Center = FVector::ZeroVector;
		Units->RequestUnits(Request);
		Units->ServeBirthRequests();
		EndStep();
		const FHOLHeroSheets& Sheets = Heroes->GetSheets(0);
		if (TestEqual(TEXT("slot 0's hero alive"), Sheets.Public.State, EHOLHeroState::Alive))
		{
			const int32 Now = static_cast<int32>(Simulation->GetStepIndex());
			const auto ReadyStep = [&Sheets](const EHOLHeroAbility Ability)
			{
				const int32 Index = static_cast<int32>(Ability);
				return Sheets.Private.CooldownEndSteps.IsValidIndex(Index) ? Sheets.Private.CooldownEndSteps[Index] : -1;
			};

			Commands->IssueHeroAbilityOrder(0, EHOLHeroAbility::CloseRanks, FVector::ZeroVector);
			EndStep();
			TestEqual(TEXT("the cry cast: one cry in his public sheet"), static_cast<int32>(Sheets.Public.Cries), 1);
			TestEqual(TEXT("... ready again 1 200 steps later (owner only)"), ReadyStep(EHOLHeroAbility::CloseRanks), Now + 1200);
			TestTrue(TEXT("... the cry pending for the combat step"), Heroes->ConsumeCloseRanks(0));
			Commands->IssueHeroAbilityOrder(0, EHOLHeroAbility::CloseRanks, FVector::ZeroVector);
			EndStep();
			TestEqual(TEXT("ordered again during its cool-down: still one cry"), static_cast<int32>(Sheets.Public.Cries), 1);
			TestFalse(TEXT("... none pending"), Heroes->ConsumeCloseRanks(0));

			Commands->IssueHeroAbilityOrder(0, EHOLHeroAbility::Banner, FVector(1600.0, 0.0, 0.0));
			EndStep();
			TestEqual(TEXT("no banner at level 1"), Sheets.Public.BannerEndStep, -1);
			Heroes->SetXP(0, HOLHeroXP::GetLevelXP(4));
			EndStep();
			Commands->IssueHeroAbilityOrder(0, EHOLHeroAbility::Banner, FVector(1600.0, 0.0, 0.0));
			EndStep();
			TestEqual(TEXT("at level 4, 16 m away: standing 900 steps"), Sheets.Public.BannerEndStep, Now + 900);
			TestEqual(TEXT("... its place in the public sheet"), Sheets.Public.BannerLocation, FIntPoint(1600, 0));
			TestEqual(TEXT("... ready again 2 400 steps later"), ReadyStep(EHOLHeroAbility::Banner), Now + 2400);
			TestEqual(TEXT("the aura has no cool-down"), ReadyStep(EHOLHeroAbility::Aura), 0);

			Commands->IssueHeroAbilityOrder(INDEX_NONE, EHOLHeroAbility::CloseRanks, FVector::ZeroVector);
			Commands->IssueHeroAbilityOrder(FHOLMatchConfig::MaxSlots, EHOLHeroAbility::CloseRanks, FVector::ZeroVector);
			TArray<FHOLHeroAbilityOrder> Left;
			Commands->ConsumeHeroAbilityOrders(Left);
			TestEqual(TEXT("the orders of no player's slot dropped"), Left.Num(), 0);
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
