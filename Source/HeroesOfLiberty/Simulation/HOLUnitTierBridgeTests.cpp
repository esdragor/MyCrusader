// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLUnitTierRules.h"
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

		// The server's call (the static one) is the board's answer, and the same as the C++ rule on every unit of the table.
		const TCHAR* Types[] = {TEXT("Peasant"), TEXT("Spearman"), TEXT("Archer"), TEXT("ManAtArms"), TEXT("Crossbowman"), TEXT("LightCavalry"), TEXT("Monk"), TEXT("Ram"),
			TEXT("Mangonel"), TEXT("Trebuchet"), TEXT("Cannon"), TEXT("Arquebusier"), TEXT("Zombie"), TEXT("DamnedWarrior"), TEXT("Hero"), TEXT("King")};
		for (const TCHAR* TypeName : Types)
		{
			for (int32 Tier = 0; Tier <= 3; ++Tier)
			{
				TestEqual(*FString::Printf(TEXT("%s at tier %d: the bridge gives the C++ rule's answer"), TypeName, Tier),
					UHOLUnitTierSubsystem::CheckUnit(World, FName(TypeName), Tier, true, true),
					static_cast<int32>(HOLUnitTier::Check(HOLUnitTier::GetMinTier(FName(TypeName)), Tier, true, true)));
			}
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
