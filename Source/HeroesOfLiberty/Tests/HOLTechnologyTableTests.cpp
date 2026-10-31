// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypes.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Technologies/HOLTechnologyTypes.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

// T1 (Technologies, GDD 11.1): the table Tools/Data/Technologies.csv, read through the same import as the editor's
// (UDataTable::CreateTableFromCSVString, what make_technologies.py does into /Game/Technologies/Technologies). The expected
// content below is the GDD's grid 11.1 (names, tier, building when the GDD places it), not a copy of the CSV: the test fails
// when a technology of the grid is missing, wrongly tiered or put in a building the GDD names otherwise.
namespace UE::HeroesOfLiberty::Tests::TechnologyTable
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** One line of GDD 11.1: the row name, its tier, and its building when the GDD names it (empty: left to T-Q3). */
	struct FGridLine
	{
		const TCHAR* Name;
		int32 Tier;
		const TCHAR* Building;
	};

	constexpr FGridLine Grid[] = {
		// Forge: the four lines, I at tier 1 and II at tier 2.
		{ TEXT("MeleeAttackI"), 1, TEXT("Forge") }, { TEXT("MeleeAttackII"), 2, TEXT("Forge") },
		{ TEXT("RangedAttackI"), 1, TEXT("Forge") }, { TEXT("RangedAttackII"), 2, TEXT("Forge") },
		{ TEXT("InfantryArmorI"), 1, TEXT("Forge") }, { TEXT("InfantryArmorII"), 2, TEXT("Forge") },
		{ TEXT("CavalryArmorI"), 1, TEXT("Forge") }, { TEXT("CavalryArmorII"), 2, TEXT("Forge") },
		// Camps and farm (T-Q3 = A, D583): the Wheelbarrow is researched at the Farm, with the Plough, not at the main centre.
		{ TEXT("DoubleAxesI"), 1, TEXT("LumberCamp") }, { TEXT("DoubleAxesII"), 2, TEXT("LumberCamp") },
		{ TEXT("MinerPicksI"), 1, TEXT("MiningCamp") }, { TEXT("MinerPicksII"), 2, TEXT("MiningCamp") },
		{ TEXT("PloughI"), 1, TEXT("Farm") }, { TEXT("PloughII"), 2, TEXT("Farm") },
		{ TEXT("WheelbarrowI"), 1, TEXT("Farm") }, { TEXT("WheelbarrowII"), 2, TEXT("Farm") },
		// Main centre.
		{ TEXT("DefenceMilitia"), 1, TEXT("MainCentre") }, { TEXT("ImprovedAlarmBell"), 2, TEXT("MainCentre") },
		// Academy (tier 2 only).
		{ TEXT("Masonry"), 2, TEXT("Academy") }, { TEXT("Veterans"), 2, TEXT("Academy") }, { TEXT("Logistics"), 2, TEXT("Academy") },
		// Siege workshop.
		{ TEXT("ReinforcedRam"), 1, TEXT("SiegeWorkshop") }, { TEXT("HeavyMunitions"), 2, TEXT("SiegeWorkshop") },
		// The Dawn's own (D27, 11.1; T-Q3 = A, D583): Liturgy and Blessing of Arms at the Sanctuary, Tempered Halberds at the
		// Barracks, Vow of Poverty at the Chapel.
		{ TEXT("Liturgy"), 1, TEXT("Sanctuary") }, { TEXT("TemperedHalberds"), 1, TEXT("Barracks") },
		{ TEXT("BlessingOfArms"), 2, TEXT("Sanctuary") }, { TEXT("VowOfPoverty"), 2, TEXT("Chapel") },
	};

	/** The Dawn's own technologies of the grid (the others are common, D27). */
	constexpr const TCHAR* DawnOwn[] = { TEXT("Liturgy"), TEXT("TemperedHalberds"), TEXT("BlessingOfArms"), TEXT("VowOfPoverty") };

	/** Buildings of GDD 11.1 not yet in BuildingTypes.csv (their chantiers to come, T-Q4): a row may name them. */
	constexpr const TCHAR* PlannedBuildings[] = { TEXT("Forge"), TEXT("Academy"), TEXT("SiegeWorkshop"), TEXT("Sanctuary"), TEXT("Chapel") };

	/** The statistics the effects code (T5) will read; a new one is added here with its code. */
	constexpr const TCHAR* KnownStats[] = {
		TEXT("MeleeAttack"), TEXT("RangedAttack"), TEXT("InfantryArmor"), TEXT("CavalryArmor"),
		TEXT("GatherWood"), TEXT("GatherStone"), TEXT("GatherGold"), TEXT("GatherFood"), TEXT("WorkerSpeed"), TEXT("WorkerCarry"),
		TEXT("WorkerDefence"), TEXT("GarrisonSpeed"), TEXT("WallTowerHealth"), TEXT("CoreUnitHealth"), TEXT("ArmyMarchSpeed"),
		TEXT("RamHealth"), TEXT("MangonelDamage"), TEXT("HonourGain"), TEXT("HalberdierBonusVsHeavyInfantry"),
		TEXT("KnightAndHalberdierBonusVsProfane"), TEXT("MonkCostDiscount"),
	};

	template <typename T, int32 N>
	bool Contains(const T* const (&List)[N], const FString& Name)
	{
		for (const T* Item : List)
		{
			if (Name == Item)
			{
				return true;
			}
		}
		return false;
	}

	/** Imports a CSV of Tools/Data into a transient table of a row type, the way the editor script does; false (and the problems reported) if it fails. */
	bool Import(FAutomationTestBase& Test, const TCHAR* File, UScriptStruct* RowStruct, UDataTable*& OutTable)
	{
		const FString Path = FPaths::Combine(FPaths::ProjectDir(), TEXT("Tools"), TEXT("Data"), File);
		FString Text;
		if (!Test.TestTrue(FString::Printf(TEXT("%s is there (Tools/Data/%s)"), *Path, File), FFileHelper::LoadFileToString(Text, *Path)))
		{
			return false;
		}
		OutTable = NewObject<UDataTable>(GetTransientPackage());
		OutTable->RowStruct = RowStruct;
		const TArray<FString> Problems = OutTable->CreateTableFromCSVString(Text);
		for (const FString& Problem : Problems)
		{
			Test.AddError(FString::Printf(TEXT("%s: %s"), File, *Problem));
		}
		return Problems.IsEmpty();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLTechnologyTableTest, "HeroesOfLiberty.Technologies.Table", UE::HeroesOfLiberty::Tests::TechnologyTable::Flags)
bool FHOLTechnologyTableTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::TechnologyTable;

	UDataTable* Techs = nullptr;
	if (!Import(*this, TEXT("Technologies.csv"), FHOLTechnologyRow::StaticStruct(), Techs))
	{
		return false;
	}
	UDataTable* Buildings = nullptr;
	if (!Import(*this, TEXT("BuildingTypes.csv"), FHOLBuildingTypeRow::StaticStruct(), Buildings))
	{
		return false;
	}

	// 1. Every line of the grid 11.1 is there, at its tier and (when the GDD says it) in its building; nothing else.
	TestEqual(TEXT("as many rows as the grid 11.1 has lines"), Techs->GetRowNames().Num(), static_cast<int32>(UE_ARRAY_COUNT(Grid)));
	for (const FGridLine& Line : Grid)
	{
		const FHOLTechnologyRow* Row = Techs->FindRow<FHOLTechnologyRow>(Line.Name, TEXT("technology"), false);
		if (!TestNotNull(FString::Printf(TEXT("%s of the grid 11.1 is in the table"), Line.Name), Row))
		{
			continue;
		}
		TestEqual(FString::Printf(TEXT("%s: tier of the grid"), Line.Name), Row->MinTier, Line.Tier);
		if (*Line.Building)
		{
			TestEqual(FString::Printf(TEXT("%s: building of the grid"), Line.Name), Row->Building, FName(Line.Building));
		}
		// 2. The faction: the Dawn's own four, common for all the others (D27).
		TestEqual(FString::Printf(TEXT("%s: faction"), Line.Name), Row->Faction, Contains(DawnOwn, Line.Name) ? FName(TEXT("Dawn")) : FName(TEXT("Common")));
	}

	// 3. The three conditions of GDD 11 hold on every row, and the effect is readable by the code to come.
	for (const TPair<FName, uint8*>& Pair : Techs->GetRowMap())
	{
		const FString Name = Pair.Key.ToString();
		const FHOLTechnologyRow& Row = *reinterpret_cast<const FHOLTechnologyRow*>(Pair.Value);
		TestFalse(FString::Printf(TEXT("%s: has a name to show"), *Name), Row.DisplayName.IsEmpty());

		// Condition 1: the hero (a tier of the prototype, 1 or 2; a level only as the stricter rule).
		TestTrue(FString::Printf(TEXT("%s: tier 1 or 2 (the tier 3 of GDD 11.2 is not in the table, T-Q5)"), *Name), Row.MinTier >= 1 && Row.MinTier <= 2);
		TestEqual(FString::Printf(TEXT("%s: the tier alone decides (T-Q1 = A, D583): hero level 0"), *Name), Row.MinHeroLevel, 0);
		// Condition 2: a building that is in BuildingTypes.csv or planned.
		const bool bKnownBuilding = Buildings->FindRow<FHOLBuildingTypeRow>(Row.Building, TEXT("building"), false) != nullptr
			|| Contains(PlannedBuildings, Row.Building.ToString());
		TestTrue(FString::Printf(TEXT("%s: building %s is a type of BuildingTypes.csv or a planned one"), *Name, *Row.Building.ToString()), bKnownBuilding);
		// Condition 3: a cost, and a time.
		TestTrue(FString::Printf(TEXT("%s: costs something"), *Name), Row.Cost.Food + Row.Cost.Wood + Row.Cost.Stone + Row.Cost.Gold > 0);
		TestTrue(FString::Printf(TEXT("%s: costs no negative amount"), *Name), Row.Cost.Food >= 0 && Row.Cost.Wood >= 0 && Row.Cost.Stone >= 0 && Row.Cost.Gold >= 0);
		TestTrue(FString::Printf(TEXT("%s: takes time"), *Name), Row.ResearchTime > 0.f);

		// The effect: known statistics, a positive change.
		TestFalse(FString::Printf(TEXT("%s: changes a statistic"), *Name), Row.Stats.IsEmpty());
		for (const FName Stat : Row.Stats)
		{
			TestTrue(FString::Printf(TEXT("%s: statistic %s is one the effects code knows"), *Name, *Stat.ToString()), Contains(KnownStats, Stat.ToString()));
		}
		TestTrue(FString::Printf(TEXT("%s: changes by a positive percent"), *Name), Row.Percent > 0);

		// The chain: a level II asks for the level I of its line (same building and statistics, one tier below, a smaller effect).
		if (Name.EndsWith(TEXT("II")))
		{
			const FString First = Name.LeftChop(1);
			TestEqual(FString::Printf(TEXT("%s: asks for %s"), *Name, *First), Row.Requires, FName(*First));
		}
		if (!Row.Requires.IsNone())
		{
			const FHOLTechnologyRow* Before = Techs->FindRow<FHOLTechnologyRow>(Row.Requires, TEXT("required technology"), false);
			if (TestNotNull(FString::Printf(TEXT("%s: the technology it requires (%s) is in the table"), *Name, *Row.Requires.ToString()), Before))
			{
				TestTrue(FString::Printf(TEXT("%s: asks a lower tier than itself"), *Name), Before->MinTier < Row.MinTier);
				TestEqual(FString::Printf(TEXT("%s: researched in the same building as %s"), *Name, *Row.Requires.ToString()), Before->Building, Row.Building);
				TestTrue(FString::Printf(TEXT("%s: stronger than %s"), *Name, *Row.Requires.ToString()), Row.Percent > Before->Percent);
				TestTrue(FString::Printf(TEXT("%s: same statistics as %s"), *Name, *Row.Requires.ToString()), Row.Stats == Before->Stats);
			}
		}
	}
	return true;
}

#endif
