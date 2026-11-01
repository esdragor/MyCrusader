// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypes.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Siege/HOLSiegeRules.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"
#include "Technologies/HOLTechnologyTypes.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

// S1b (SiegeInProgress.md, GDD 6.1, D23, D77, D180): the C++ copy of the siege engines' rules (the simulation reads C++, D483; the
// script's Script/Siege/HOLSiegeRules.as is tested with the same cases) and its link with the REAL technology table
// (Tools/Data/Technologies.csv): the technologies of the siege workshop must open at the tier of the engine they improve. The
// expectations are the GDD's (Ram 1, Mangonel 2, Trebuchet and Cannon 3; the prototype's two engines), not copies of the rules.
namespace UE::HeroesOfLiberty::Tests::Siege
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSiegeTiersTest, "HeroesOfLiberty.Siege.Tiers", UE::HeroesOfLiberty::Tests::Siege::Flags)
bool FHOLSiegeTiersTest::RunTest(const FString& Parameters)
{
	using namespace HOLSiege;

	// GDD 6.1: the tier of each engine; D77: the prototype's two.
	TestEqual(TEXT("Ram opens at tier 1"), GetTier(EEngine::Ram), 1);
	TestEqual(TEXT("Mangonel opens at tier 2"), GetTier(EEngine::Mangonel), 2);
	TestEqual(TEXT("Trebuchet opens at tier 3"), GetTier(EEngine::Trebuchet), 3);
	TestEqual(TEXT("Cannon opens at tier 3 (D180)"), GetTier(EEngine::Cannon), 3);
	TestTrue(TEXT("Ram in the prototype"), InPrototype(EEngine::Ram));
	TestTrue(TEXT("Mangonel in the prototype"), InPrototype(EEngine::Mangonel));
	TestFalse(TEXT("Trebuchet not in the prototype"), InPrototype(EEngine::Trebuchet));
	TestFalse(TEXT("Cannon not in the prototype"), InPrototype(EEngine::Cannon));

	// The reasons, one at a time and in order (the same cases as Script/Siege/HOLSiegeRules_Test.as).
	TestTrue(TEXT("Ram, tier 1, workshop, paid: available"), Check(EEngine::Ram, 1, true, true) == EBlock::None);
	TestTrue(TEXT("Ram, tier 0: tier too low"), Check(EEngine::Ram, 0, true, true) == EBlock::TierTooLow);
	TestTrue(TEXT("Mangonel, tier 1: tier too low"), Check(EEngine::Mangonel, 1, true, true) == EBlock::TierTooLow);
	TestTrue(TEXT("Mangonel, tier 2: available"), Check(EEngine::Mangonel, 2, true, true) == EBlock::None);
	TestTrue(TEXT("no workshop"), Check(EEngine::Ram, 1, false, true) == EBlock::NoWorkshop);
	TestTrue(TEXT("cannot pay"), Check(EEngine::Ram, 1, true, false) == EBlock::CannotPay);
	TestTrue(TEXT("Trebuchet refused even at tier 3"), Check(EEngine::Trebuchet, 3, true, true) == EBlock::NotInPrototype);
	TestTrue(TEXT("outside the prototype comes first"), Check(EEngine::Cannon, 0, false, false) == EBlock::NotInPrototype);
	TestTrue(TEXT("then the tier"), Check(EEngine::Mangonel, 0, false, false) == EBlock::TierTooLow);
	TestTrue(TEXT("then the workshop"), Check(EEngine::Mangonel, 2, false, false) == EBlock::NoWorkshop);
	TestTrue(TEXT("then the cost"), Check(EEngine::Mangonel, 2, true, false) == EBlock::CannotPay);
	return true;
}

// The real table: the technologies researched at the siege workshop open at the tier of the engine they improve (GDD 11.1).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSiegeTechnologiesTest, "HeroesOfLiberty.Siege.Technologies", UE::HeroesOfLiberty::Tests::Siege::Flags)
bool FHOLSiegeTechnologiesTest::RunTest(const FString& Parameters)
{
	const FString Path = FPaths::Combine(FPaths::ProjectDir(), TEXT("Tools"), TEXT("Data"), TEXT("Technologies.csv"));
	FString Text;
	if (!TestTrue(TEXT("Tools/Data/Technologies.csv is there"), FFileHelper::LoadFileToString(Text, *Path)))
	{
		return false;
	}
	UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
	Table->RowStruct = FHOLTechnologyRow::StaticStruct();
	const TArray<FString> Problems = Table->CreateTableFromCSVString(Text);
	for (const FString& Problem : Problems)
	{
		AddError(FString::Printf(TEXT("Technologies.csv: %s"), *Problem));
	}
	if (!Problems.IsEmpty())
	{
		return false;
	}

	// Which engine each siege technology of GDD 11.1 improves (Bélier renforcé -> Ram, Munitions lourdes -> Mangonel).
	struct FLine
	{
		const TCHAR* Technology;
		HOLSiege::EEngine Engine;
	};
	const FLine Lines[] = { { TEXT("ReinforcedRam"), HOLSiege::EEngine::Ram }, { TEXT("HeavyMunitions"), HOLSiege::EEngine::Mangonel } };

	int32 AtTheWorkshop = 0;
	for (const FName RowName : Table->GetRowNames())
	{
		const FHOLTechnologyRow* Row = Table->FindRow<FHOLTechnologyRow>(RowName, TEXT("technology"), false);
		if (Row != nullptr && Row->Building == FName(TEXT("SiegeWorkshop")))
		{
			AtTheWorkshop++;
		}
	}
	TestEqual(TEXT("two technologies at the siege workshop (GDD 11.1)"), AtTheWorkshop, static_cast<int32>(UE_ARRAY_COUNT(Lines)));

	for (const FLine& Line : Lines)
	{
		const FHOLTechnologyRow* Row = Table->FindRow<FHOLTechnologyRow>(Line.Technology, TEXT("technology"), false);
		if (!TestNotNull(FString::Printf(TEXT("%s is in the table"), Line.Technology), Row))
		{
			continue;
		}
		TestEqual(FString::Printf(TEXT("%s opens at the tier of the engine it improves"), Line.Technology), Row->MinTier, HOLSiege::GetTier(Line.Engine));
		TestTrue(FString::Printf(TEXT("%s improves an engine of the prototype"), Line.Technology), HOLSiege::InPrototype(Line.Engine));
		// S2: the building the technology names is a real type now, no longer a planned one, and the technology is not older than it.
		const FHOLBuildingTypeRow* Workshop = UHOLBuildingSettings::FindType(Row->Building);
		if (TestNotNull(FString::Printf(TEXT("%s: its building %s is in the building type table"), Line.Technology, *Row->Building.ToString()), Workshop))
		{
			TestTrue(FString::Printf(TEXT("%s: not researched before its building exists (tier %d, building age %d)"), Line.Technology, Row->MinTier, Workshop->Age),
				Row->MinTier >= Workshop->Age);
			TestTrue(FString::Printf(TEXT("%s: the workshop produces the engine it improves"), Line.Technology),
				Workshop->Produces.Contains(Line.Engine == HOLSiege::EEngine::Ram ? FName(TEXT("Ram")) : FName(TEXT("Mangonel"))));
		}
	}
	return true;
}

// S2 (SiegeInProgress.md, SQ1 = B / D614, SQ2 = A / D615): the tables of the two engines and of the siege workshop, read from the REAL
// assets (/Game/Units/UnitTypes, /Game/Buildings/BuildingTypes, imported from the CSVs). The numbers are the user's starting values
// (provisional, balanced at the trial); the production times and the armours are not in D615 (SQ3 in Retours): only checked to be
// positive / not negative.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSiegeTablesTest, "HeroesOfLiberty.Siege.Table", UE::HeroesOfLiberty::Tests::Siege::Flags)
bool FHOLSiegeTablesTest::RunTest(const FString& Parameters)
{
	const double Grid = UHOLBuildingSettings::GetTileSize();
	const FName WorkshopName(TEXT("SiegeWorkshop"));
	const FName RamName(TEXT("Ram"));
	const FName MangonelName(TEXT("Mangonel"));

	const FHOLBuildingTypeRow* Workshop = UHOLBuildingSettings::FindType(WorkshopName);
	if (TestNotNull(TEXT("SiegeWorkshop in the building type table"), Workshop))
	{
		TestTrue(TEXT("Workshop: built by Peasants"), Workshop->IsBuildable());
		TestEqual(TEXT("Workshop: military"), int32(Workshop->Category), int32(EHOLBuildingCategory::Military));
		TestEqual(TEXT("Workshop: age II, the tier of the Ram (GDD 6.1, 11.1)"), Workshop->Age, HOLSiege::GetTier(HOLSiege::EEngine::Ram));
		TestEqual(TEXT("Workshop: Q of page II (slot 4, military row)"), Workshop->BuildGridSlot, 4);
		TestEqual(TEXT("Workshop: 3 x 3 tiles"), Workshop->Footprint, FVector2D(3.0 * Grid, 3.0 * Grid));
		TestEqual(TEXT("Workshop: 1 500 health"), Workshop->MaxHealth, 1500);
		TestTrue(TEXT("Workshop: 175 wood, nothing else"), Workshop->Cost.Wood == 175 && Workshop->Cost.Food == 0 && Workshop->Cost.Stone == 0 && Workshop->Cost.Gold == 0);
		TestEqual(TEXT("Workshop: 25 s to build"), Workshop->BuildTime, 25.f);
		TestTrue(TEXT("Workshop: produces the Ram then the Mangonel"), Workshop->Produces == TArray<FName>({ RamName, MangonelName }));
		TestEqual(TEXT("Workshop: no population"), Workshop->PopulationProvided, 0);
		TestTrue(TEXT("Workshop: not a deposit"), Workshop->Accepts.IsEmpty());
		TestFalse(TEXT("Workshop: not walkable"), Workshop->Walkable);
		TestTrue(TEXT("Workshop: sees at least as far as a house"), Workshop->SightRadius >= 2000.f);
	}

	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	if (!TestNotNull(TEXT("unit type table"), UnitTypes))
	{
		return false;
	}

	const FHOLUnitTypeRow* Ram = UnitTypes->FindRow<FHOLUnitTypeRow>(RamName, TEXT("Siege test"), false);
	if (TestNotNull(TEXT("Ram in the unit type table"), Ram))
	{
		TestEqual(TEXT("Ram: 400 health"), Ram->MaxHealth, 400);
		TestEqual(TEXT("Ram: 220 cm/s"), Ram->Speed, 220.f);
		TestEqual(TEXT("Ram: 3 places"), Ram->Population, 3);
		TestTrue(TEXT("Ram: 150 wood + 75 gold"), Ram->Cost.Wood == 150 && Ram->Cost.Gold == 75 && Ram->Cost.Food == 0 && Ram->Cost.Stone == 0);
		TestTrue(TEXT("Ram: armour category Siege alone (D345, the Light cavalry's bonus aims at it)"), Ram->ArmorCategories == TArray<EHOLArmorCategory>({ EHOLArmorCategory::Siege }));
		// SQ1 = B: the Ram hits buildings only, 60 siege damage every 2.5 s; against a unit its attack is zero.
		TestEqual(TEXT("Ram: siege attack on buildings"), int32(Ram->BuildingAttackType), int32(EHOLAttackType::Siege));
		TestEqual(TEXT("Ram: 60 damage on a building"), Ram->BuildingAttack, 60);
		TestEqual(TEXT("Ram: every 2.5 s"), Ram->BuildingAttackInterval, 2.5f);
		TestEqual(TEXT("Ram: no attack on units (SQ1 = B)"), Ram->Attack, 0);
		TestEqual(TEXT("Ram: no area"), Ram->AreaRadius, 0.f);
		TestTrue(TEXT("Ram: a production time"), Ram->BuildTime > 0.f);
		TestTrue(TEXT("Ram: armours not negative"), Ram->MeleeArmor >= 0 && Ram->RangedArmor >= 0 && Ram->SiegeArmor >= 0 && Ram->HeroArmor >= 0);
		TestTrue(TEXT("Ram: sees at least a tile beyond its reach"), Ram->SightRadius >= Ram->BuildingAttackRange + 400.f);
	}

	const FHOLUnitTypeRow* Mangonel = UnitTypes->FindRow<FHOLUnitTypeRow>(MangonelName, TEXT("Siege test"), false);
	if (TestNotNull(TEXT("Mangonel in the unit type table"), Mangonel))
	{
		TestEqual(TEXT("Mangonel: 200 health"), Mangonel->MaxHealth, 200);
		TestEqual(TEXT("Mangonel: 180 cm/s"), Mangonel->Speed, 180.f);
		TestEqual(TEXT("Mangonel: 4 places"), Mangonel->Population, 4);
		TestTrue(TEXT("Mangonel: 200 wood + 150 gold"), Mangonel->Cost.Wood == 200 && Mangonel->Cost.Gold == 150 && Mangonel->Cost.Food == 0 && Mangonel->Cost.Stone == 0);
		TestTrue(TEXT("Mangonel: armour category Siege alone"), Mangonel->ArmorCategories == TArray<EHOLArmorCategory>({ EHOLArmorCategory::Siege }));
		// SQ1 = B: 45 siege damage every 5 s at 3 000 cm, in an area of 250 cm on units, one blow on a building.
		TestEqual(TEXT("Mangonel: siege attack"), int32(Mangonel->AttackType), int32(EHOLAttackType::Siege));
		TestEqual(TEXT("Mangonel: 45 damage"), Mangonel->Attack, 45);
		TestEqual(TEXT("Mangonel: every 5 s"), Mangonel->AttackInterval, 5.f);
		TestEqual(TEXT("Mangonel: 3 000 cm reach"), Mangonel->AttackRange, 3000.f);
		TestEqual(TEXT("Mangonel: area of 250 cm on units"), Mangonel->AreaRadius, 250.f);
		TestEqual(TEXT("Mangonel: siege attack on buildings"), int32(Mangonel->BuildingAttackType), int32(EHOLAttackType::Siege));
		TestEqual(TEXT("Mangonel: 45 damage on a building, one blow"), Mangonel->BuildingAttack, 45);
		TestEqual(TEXT("Mangonel: a building every 5 s"), Mangonel->BuildingAttackInterval, 5.f);
		TestEqual(TEXT("Mangonel: 3 000 cm reach on a building"), Mangonel->BuildingAttackRange, 3000.f);
		TestTrue(TEXT("Mangonel: a production time"), Mangonel->BuildTime > 0.f);
		TestTrue(TEXT("Mangonel: armours not negative"), Mangonel->MeleeArmor >= 0 && Mangonel->RangedArmor >= 0 && Mangonel->SiegeArmor >= 0 && Mangonel->HeroArmor >= 0);
		TestTrue(TEXT("Mangonel: sees at least a tile beyond its reach"), Mangonel->SightRadius >= Mangonel->AttackRange + 400.f);
	}

	// Only the Mangonel has an area; no other unit type does (the column is new).
	for (const FName Name : UnitTypes->GetRowNames())
	{
		const FHOLUnitTypeRow* Row = UnitTypes->FindRow<FHOLUnitTypeRow>(Name, TEXT("Siege test"), false);
		if (Row != nullptr && Name != MangonelName)
		{
			TestEqual(*FString::Printf(TEXT("%s: no area damage"), *Name.ToString()), Row->AreaRadius, 0.f);
		}
	}

	// The workshop's engines are all in the unit table, each with the Siege armour category.
	if (Workshop != nullptr)
	{
		for (const FName Produced : Workshop->Produces)
		{
			const FHOLUnitTypeRow* Row = UnitTypes->FindRow<FHOLUnitTypeRow>(Produced, TEXT("Siege test"), false);
			if (TestNotNull(*FString::Printf(TEXT("Workshop: %s in the unit type table"), *Produced.ToString()), Row))
			{
				TestTrue(*FString::Printf(TEXT("Workshop: %s is a siege unit"), *Produced.ToString()), Row->HasArmorCategory(EHOLArmorCategory::Siege));
			}
		}
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
