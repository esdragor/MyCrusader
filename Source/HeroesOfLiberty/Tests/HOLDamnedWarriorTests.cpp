// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"
#include "UObject/Class.h"

#if WITH_DEV_AUTOMATION_TESTS

// L4 (LegionsInProgress.md, plan step 2): the Damned Warrior (profile A, Q-legions-l4 Q1 = A) on the REAL unit type table
// (/Game/Units/UnitTypes, imported from Tools/Data/UnitTypes.csv). Relations only, as in Tools/Tests/legions_warrior_test.py: the
// Legions' emblematic Barracks unit is a fragile berserker, so it is Profane and heavy, has less health than the Man-at-arms and hits
// harder per second. It stays RED until the DamnedWarrior row exists (plan step 3). The necrotic rage (plan steps 4 and 5) is tested
// on the real chain once its order and its state exist: Combat.DamnedWarrior.Rage is written with that code (its API is not designed yet).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDamnedWarriorTableTest, "HeroesOfLiberty.Combat.DamnedWarrior.Table", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLDamnedWarriorTableTest::RunTest(const FString& Parameters)
{
	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	if (!TestNotNull(TEXT("unit type table"), UnitTypes))
	{
		return false;
	}
	const FHOLUnitTypeRow* Warrior = UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("DamnedWarrior"), TEXT("Damned Warrior test"), false);
	const FHOLUnitTypeRow* ManAtArms = UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("ManAtArms"), TEXT("Damned Warrior test"), false);
	if (!TestNotNull(TEXT("DamnedWarrior in the unit type table"), Warrior) || !TestNotNull(TEXT("ManAtArms in the unit type table"), ManAtArms))
	{
		return false;
	}
	TestEqual(TEXT("the Damned Warrior is the Legions' (D395)"), Warrior->Faction, FName(TEXT("Legions")));
	TestEqual(TEXT("tier 1 (D395)"), Warrior->MinTier, 1);
	TestTrue(TEXT("Heavy infantry"), Warrior->ArmorCategories.Contains(EHOLArmorCategory::HeavyInfantry));
	TestTrue(TEXT("Heavy (D428, unit by unit)"), Warrior->ArmorCategories.Contains(EHOLArmorCategory::Heavy));
	const UEnum* Enum = StaticEnum<EHOLArmorCategory>();
	const int64 Profane = Enum ? Enum->GetValueByNameString(TEXT("Profane")) : INDEX_NONE;
	bool bProfane = false;
	for (const EHOLArmorCategory Category : Warrior->ArmorCategories)
	{
		bProfane |= Profane != INDEX_NONE && static_cast<int64>(Category) == Profane;
	}
	TestTrue(TEXT("Profane (D393)"), bProfane);
	TestFalse(TEXT("not a Peasant (it does not build or gather)"), Warrior->ArmorCategories.Contains(EHOLArmorCategory::Peasant));
	TestTrue(TEXT("profile A: fragile, less health than the Man-at-arms"), Warrior->MaxHealth < ManAtArms->MaxHealth);
	TestTrue(TEXT("... and more damage per second than the Man-at-arms"), Warrior->Attack / Warrior->AttackInterval > ManAtArms->Attack / ManAtArms->AttackInterval);
	TestTrue(TEXT("it has a cost"), Warrior->Cost.Food + Warrior->Cost.Wood + Warrior->Cost.Stone + Warrior->Cost.Gold > 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
