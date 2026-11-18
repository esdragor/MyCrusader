// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLUnitTierSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// MIG-CPP-UnitTier (D836 point 3): the unit tier rule is the script's (Script/Units/HOLUnitTierRules.as, HOLUnitTier::Check), reached by the
// server through the bridge UHOLUnitTierSubsystem (Script/Units/HOLUnitTierBoardScript.as). The rule itself is tested in script
// (HOLUnitTierRules_Test.as); here the real chain C++ bridge -> script board, the answers being the production subsystem's refusal reasons.
// (In Simulation/ and not Tests/: the lot's files stay in the simulation folder; the module compiles it the same.)
namespace UE::HeroesOfLiberty::Tests::UnitTierBridge
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLUnitTierServerBoardTest, "HeroesOfLiberty.UnitTier.ServerBoard", UE::HeroesOfLiberty::Tests::UnitTierBridge::Flags)

bool FHOLUnitTierServerBoardTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLUnitTierSubsystem* Board = UHOLUnitTierSubsystem::Find(World);
	if (TestNotNull(TEXT("the script board of the unit tier"), Board))
	{
		constexpr int32 Available = 0, NotListed = 1, TierTooLow = 2, NoBuilding = 3, CannotPay = 4;
		TestEqual(TEXT("A Man-at-arms at tier 0 is refused for the tier (the production's refusal)"), Board->CheckUnitTierNow(TEXT("ManAtArms"), 0, true, true), TierTooLow);
		TestEqual(TEXT("A Man-at-arms at tier 1 comes out"), Board->CheckUnitTierNow(TEXT("ManAtArms"), 1, true, true), Available);
		TestEqual(TEXT("A Spearman at tier 0 comes out"), Board->CheckUnitTierNow(TEXT("Spearman"), 0, true, true), Available);
		TestEqual(TEXT("A Mangonel at tier 1 is refused for the tier"), Board->CheckUnitTierNow(TEXT("Mangonel"), 1, true, true), TierTooLow);
		TestEqual(TEXT("The Damned Warrior (Legions) at tier 0 is refused for the tier"), Board->CheckUnitTierNow(TEXT("DamnedWarrior"), 0, true, true), TierTooLow);
		TestEqual(TEXT("D836 point 6: the hero is of tier 0, as the column MinTier"), Board->CheckUnitTierNow(TEXT("Hero"), 0, true, true), Available);
		TestEqual(TEXT("A type outside the table is not listed"), Board->CheckUnitTierNow(TEXT("Dragon"), 3, true, true), NotListed);
		TestEqual(TEXT("The order of the reasons: no building"), Board->CheckUnitTierNow(TEXT("Archer"), 0, false, true), NoBuilding);
		TestEqual(TEXT("The order of the reasons: cannot pay"), Board->CheckUnitTierNow(TEXT("Archer"), 0, true, false), CannotPay);

		// The server's and the grid's call (the static one) is the board's answer, on every unit of the table and every tier. The expected answers are
		// LITERALS worked out by hand from the column MinTier (tier 0: Peasant, Spearman, Archer, Zombie, Hero, King; tier 1: ManAtArms, Crossbowman,
		// LightCavalry, Monk, Ram, DamnedWarrior; tier 2: Mangonel; tier 3: Arquebusier; Trebuchet and Cannon have no row yet: NotListed): 0 Available,
		// 1 NotListed, 2 TierTooLow; the building is finished and the stock pays, so no 3 or 4.
		struct FExpected
		{
			const TCHAR* TypeName;
			int32 AtTier[4];
		};
		const FExpected Expected[] = {
			{TEXT("Peasant"), {0, 0, 0, 0}}, {TEXT("Spearman"), {0, 0, 0, 0}}, {TEXT("Archer"), {0, 0, 0, 0}}, {TEXT("Zombie"), {0, 0, 0, 0}},
			{TEXT("Hero"), {0, 0, 0, 0}}, {TEXT("King"), {0, 0, 0, 0}},
			{TEXT("ManAtArms"), {2, 0, 0, 0}}, {TEXT("Crossbowman"), {2, 0, 0, 0}}, {TEXT("LightCavalry"), {2, 0, 0, 0}}, {TEXT("Monk"), {2, 0, 0, 0}},
			{TEXT("Ram"), {2, 0, 0, 0}}, {TEXT("DamnedWarrior"), {2, 0, 0, 0}},
			{TEXT("Mangonel"), {2, 2, 0, 0}}, {TEXT("Arquebusier"), {2, 2, 2, 0}},
			{TEXT("Trebuchet"), {1, 1, 1, 1}}, {TEXT("Cannon"), {1, 1, 1, 1}}};
		for (const FExpected& Row : Expected)
		{
			for (int32 Tier = 0; Tier <= 3; ++Tier)
			{
				TestEqual(*FString::Printf(TEXT("%s at tier %d: the bridge gives the hand-worked answer"), Row.TypeName, Tier),
					UHOLUnitTierSubsystem::CheckUnit(World, FName(Row.TypeName), Tier, true, true), Row.AtTier[Tier]);
			}
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
