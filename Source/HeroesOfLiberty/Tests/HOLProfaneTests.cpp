// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"
#include "UObject/Class.h"

#if WITH_DEV_AUTOMATION_TESTS

// L3 (LegionsInProgress.md, D393): the "Profane" armour category, on the real unit type table (/Game/Units/UnitTypes, imported from
// Tools/Data/UnitTypes.csv). The Legions' Zombie has it (and stays a Peasant for the economy); no unit of another faction has it, and
// neither has a Legions unit that is not undead (the hero, to come). The category is found by NAME in the enum so that the test builds
// before the value exists: it stands red until the value is added, and then reads the real rows. The Dawn's Blessing of Arms (T6) will
// be a bonus aimed at it: the effect test shows the existing bonus mechanism (FHOLAttackBonus, D345) hits a Zombie and not a Peasant.
namespace UE::HeroesOfLiberty::Tests::Profane
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** The enum value named Profane, or INDEX_NONE while the category does not exist. */
	int64 ProfaneValue()
	{
		const UEnum* Enum = StaticEnum<EHOLArmorCategory>();
		return Enum ? Enum->GetValueByNameString(TEXT("Profane")) : INDEX_NONE;
	}

	bool HasProfane(const FHOLUnitTypeRow& Row)
	{
		const int64 Value = ProfaneValue();
		for (const EHOLArmorCategory Category : Row.ArmorCategories)
		{
			if (Value != INDEX_NONE && static_cast<int64>(Category) == Value)
			{
				return true;
			}
		}
		return false;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLProfaneTableTest, "HeroesOfLiberty.Combat.Profane.Table", UE::HeroesOfLiberty::Tests::Profane::Flags)
bool FHOLProfaneTableTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Profane;
	TestTrue(TEXT("the armour category enum has a value named Profane (D393)"), ProfaneValue() != INDEX_NONE);

	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	if (!TestNotNull(TEXT("unit type table"), UnitTypes))
	{
		return false;
	}
	const FHOLUnitTypeRow* Zombie = UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("Zombie"), TEXT("Profane test"), false);
	if (!TestNotNull(TEXT("Zombie in the unit type table"), Zombie))
	{
		return false;
	}
	TestTrue(TEXT("the Zombie is Profane"), HasProfane(*Zombie));
	TestTrue(TEXT("... and still a Peasant (it builds and gathers, the economy reads that category)"), Zombie->ArmorCategories.Contains(EHOLArmorCategory::Peasant));

	// Nobody of the Dawn (or common to both) is Profane, and no hero: only Legions rows may be.
	UnitTypes->ForeachRow<FHOLUnitTypeRow>(TEXT("Profane test"), [this](const FName& Name, const FHOLUnitTypeRow& Row)
	{
		if (HasProfane(Row))
		{
			TestEqual(*FString::Printf(TEXT("%s: Profane only for a Legions type"), *Name.ToString()), Row.Faction, FName(TEXT("Legions")));
			TestFalse(*FString::Printf(TEXT("%s: a hero is never Profane (D393)"), *Name.ToString()), Row.ArmorCategories.Contains(EHOLArmorCategory::Hero));
		}
	});
	return true;
}

// The effect, through the real damage chain: a bonus aimed at Profane adds its damage against a Zombie and nothing against a Peasant.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLProfaneBonusTest, "HeroesOfLiberty.Combat.Profane.Bonus", UE::HeroesOfLiberty::Tests::Profane::Flags)
bool FHOLProfaneBonusTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Profane;
	const int64 Value = ProfaneValue();
	if (!TestTrue(TEXT("the Profane category exists"), Value != INDEX_NONE))
	{
		return false;
	}
	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	if (!TestNotNull(TEXT("unit type table"), UnitTypes))
	{
		return false;
	}
	const FHOLUnitTypeRow* SpearmanRow = UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("Spearman"), TEXT("Profane test"), false);
	const FHOLUnitTypeRow* ZombieRow = UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("Zombie"), TEXT("Profane test"), false);
	const FHOLUnitTypeRow* PeasantRow = UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("Peasant"), TEXT("Profane test"), false);
	if (!TestNotNull(TEXT("Spearman row"), SpearmanRow) || !TestNotNull(TEXT("Zombie row"), ZombieRow) || !TestNotNull(TEXT("Peasant row"), PeasantRow))
	{
		return false;
	}
	FHOLUnitTypeConstants Spearman = SpearmanRow->ToConstants(TEXT("Spearman"));
	const FHOLUnitTypeConstants Zombie = ZombieRow->ToConstants(TEXT("Zombie"));
	const FHOLUnitTypeConstants Peasant = PeasantRow->ToConstants(TEXT("Peasant"));

	TestTrue(TEXT("the Zombie's constants carry the Profane bit"), (Zombie.ArmorCategoryMask & (1 << static_cast<int32>(Value))) != 0);
	TestTrue(TEXT("the Peasant's constants do not"), (Peasant.ArmorCategoryMask & (1 << static_cast<int32>(Value))) == 0);

	const float ZombieBefore = HOLCombat::GetDamage(Spearman, Zombie);
	const float PeasantBefore = HOLCombat::GetDamage(Spearman, Peasant);
	FHOLAttackBonus Bonus;
	Bonus.Category = static_cast<EHOLArmorCategory>(Value);
	Bonus.Damage = 7;
	Spearman.Bonuses.Add(Bonus);
	TestEqual(TEXT("a bonus against Profane adds its damage on a Zombie"), HOLCombat::GetDamage(Spearman, Zombie), ZombieBefore + 7.f);
	TestEqual(TEXT("... and nothing on a Peasant"), HOLCombat::GetDamage(Spearman, Peasant), PeasantBefore);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
