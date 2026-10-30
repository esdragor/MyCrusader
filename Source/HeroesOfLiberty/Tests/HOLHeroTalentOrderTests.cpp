// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLCommandSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLMatchConfig.h"
#include "MassEntitySubsystem.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLHeroXPRules.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// T3: the players' talent picks (GDD 9.6, D103-D105) through the order queue to the hero subsystem, and what the sheets carry
// (the picks to the owner, the seals and keys to all, D203). Levels come from the XP table (HOLHeroXP), the rule is the server's.
namespace UE::HeroesOfLiberty::Tests::HeroTalentOrders
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroTalentOrdersTest, "HeroesOfLiberty.HeroTalent.Orders", UE::HeroesOfLiberty::Tests::HeroTalentOrders::Flags)

// Slot 0's hero born at level 1 (no point). An order: refused, nothing taken. At level 3 (2 points): a family out of range
// refused; family 1 taken, then 1 again; a third refused (no point left). At the top level (5 points in the prototype):
// three more of family 1 give its seal (public bit 1), and after five its key talent; a sixth pick is the sixth point, which
// the prototype level does not give, so it is refused. A dropped order of no player's slot.
bool FHOLHeroTalentOrdersTest::RunTest(const FString& Parameters)
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
			const auto Picks = [&Sheets] { return Sheets.Private.TalentPicks.Num(); };
			const auto Order = [&](const int32 Family)
			{
				Commands->IssueHeroTalentOrder(0, Family);
				EndStep();
			};

			Order(1);
			TestEqual(TEXT("level 1: no point, refused"), Picks(), 0);

			Heroes->SetXP(0, HOLHeroXP::GetLevelXP(3));
			EndStep();
			TestEqual(TEXT("level 3 reached"), static_cast<int32>(Sheets.Public.Level), 3);
			Order(HOLHeroTalent::FamilyCount);
			Order(-1);
			TestEqual(TEXT("a family out of range: refused"), Picks(), 0);
			Order(1);
			TestEqual(TEXT("family 1 taken (row 2)"), Picks(), 1);
			Order(1);
			TestEqual(TEXT("family 1 again (row 3)"), Picks(), 2);
			Order(0);
			TestEqual(TEXT("2 points at level 3: a third refused"), Picks(), 2);
			TestTrue(TEXT("... the picks are the families in the order taken"), Sheets.Private.TalentPicks == TArray<uint8>({ 1, 1 }));
			TestEqual(TEXT("... no seal yet"), static_cast<int32>(Sheets.Public.TalentSeals), 0);

			Heroes->SetXP(0, HOLHeroXP::GetLevelXP(HOLHeroXP::MaxLevel));
			EndStep();
			Order(1);
			TestEqual(TEXT("three of family 1: its seal (bit 1) in the public sheet"), static_cast<int32>(Sheets.Public.TalentSeals), 1 << 1);
			TestEqual(TEXT("... no key yet"), static_cast<int32>(Sheets.Public.TalentKeys), 0);
			Order(1);
			Order(1);
			TestEqual(TEXT("five of family 1: its key talent too"), static_cast<int32>(Sheets.Public.TalentKeys), 1 << 1);
			TestEqual(TEXT("... five picks, the prototype's five points"), Picks(), 5);
			Order(2);
			TestEqual(TEXT("no sixth point at the prototype's top level: refused"), Picks(), 5);

			Commands->IssueHeroTalentOrder(INDEX_NONE, 1);
			Commands->IssueHeroTalentOrder(FHOLMatchConfig::MaxSlots, 1);
			TArray<FHOLHeroTalentOrder> Left;
			Commands->ConsumeHeroTalentOrders(Left);
			TestEqual(TEXT("the orders of no player's slot dropped"), Left.Num(), 0);
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
