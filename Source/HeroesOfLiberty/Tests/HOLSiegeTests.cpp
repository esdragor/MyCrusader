// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Siege/HOLSiegeRules.h"
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
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
