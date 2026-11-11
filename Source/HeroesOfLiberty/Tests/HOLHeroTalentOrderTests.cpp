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


// T6: the table of talents having an effect (HOLHeroTalent::HasEffect, drawn on by the AI) stays in step with the two effects coded: for
// every row and family, the table says yes exactly where the picks of a hero who took that talent make GrantsAuraArmor / GrantsDawnStep true.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroTalentEffectTableTest, "HeroesOfLiberty.HeroTalent.EffectTable", UE::HeroesOfLiberty::Tests::HeroTalentOrders::Flags)
bool FHOLHeroTalentEffectTableTest::RunTest(const FString& Parameters)
{
	int32 Effects = 0;
	for (int32 Row = HOLHeroTalent::FirstRow; Row <= HOLHeroTalent::LastRow; ++Row)
	{
		for (int32 Family = 0; Family < HOLHeroTalent::FamilyCount; ++Family)
		{
			// A hero who took only this family up to this row: its row's talent is the last pick.
			TArray<uint8> Picks;
			Picks.Init(static_cast<uint8>(Family), Row - HOLHeroTalent::FirstRow + 1);
			const bool bCoded = (Row == HOLHeroTalent::FirstRow && HOLHeroTalent::GrantsAuraArmor(Picks)) || (Row == HOLHeroTalent::FirstRow + 1 && HOLHeroTalent::GrantsHealBonus(Picks)) || (Row == HOLHeroTalent::FirstRow + 2 && HOLHeroTalent::GrantsDawnStep(Picks));
			TestEqual(*FString::Printf(TEXT("row %d family %d: the table says what the code does"), Row, Family), HOLHeroTalent::HasEffect(Row, Family), bCoded);
			Effects += HOLHeroTalent::HasEffect(Row, Family) ? 1 : 0;
		}
	}
	TestEqual(TEXT("three talents have an effect: Boucliers levés, Mains du guérisseur and Pas de l'Aube"), Effects, 3);
	TestFalse(TEXT("no row 1"), HOLHeroTalent::HasEffect(1, HOLHeroTalent::FamilyCaptain));
	TestFalse(TEXT("no family 3"), HOLHeroTalent::HasEffect(2, 3));
	return true;
}

#endif
