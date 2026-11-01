// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Simulation/HOLCampTable.h"

#if WITH_DEV_AUTOMATION_TESTS

// NQ3 = C (D613): Tools/Data/Camps.csv, the content of each neutral camp as a list of unit types. A type that Tools/Data/UnitTypes.csv
// does not know must fail the test (a camp could not be built from it). The checks run on the real files and, to prove the check
// itself, on a copy of the camps text with a made-up type.
namespace
{
	bool LoadData(FAutomationTestBase& Test, const TCHAR* File, FString& OutText)
	{
		const FString Path = FPaths::Combine(FPaths::ProjectDir(), TEXT("Tools"), TEXT("Data"), File);
		return Test.TestTrue(FString::Printf(TEXT("%s is there"), *Path), FFileHelper::LoadFileToString(OutText, *Path));
	}

	/** Parse problems and unknown types of a camps text against a unit types text, as readable lines. */
	TArray<FString> Check(const FString& CampsText, const FString& UnitTypesText, TArray<HOLCampTable::FCampDef>& OutCamps)
	{
		TArray<FString> Problems;
		HOLCampTable::Parse(CampsText, OutCamps, Problems);
		const TSet<FName> Known = HOLCampTable::ReadTypeNames(UnitTypesText);
		for (const HOLCampTable::FCampDef& Camp : OutCamps)
		{
			for (const FName Unknown : HOLCampTable::UnknownMembers(Camp, Known))
			{
				Problems.Add(FString::Printf(TEXT("camp %s: type %s is not in UnitTypes.csv"), *Camp.Name.ToString(), *Unknown.ToString()));
			}
		}
		return Problems;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCampTableCsvTest, "HeroesOfLiberty.Neutrals.CampTable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLCampTableCsvTest::RunTest(const FString& Parameters)
{
	FString CampsText;
	FString UnitsText;
	if (!LoadData(*this, TEXT("Camps.csv"), CampsText) || !LoadData(*this, TEXT("UnitTypes.csv"), UnitsText))
	{
		return false;
	}

	// 1. The real files agree: every member of every camp is a unit type.
	TArray<HOLCampTable::FCampDef> Camps;
	for (const FString& Problem : Check(CampsText, UnitsText, Camps))
	{
		AddError(FString::Printf(TEXT("Camps.csv: %s"), *Problem));
	}

	// 2. D613: the prototype's brigands are there, made of the four soldier types, none of them the Peasant, the King or the Hero.
	const HOLCampTable::FCampDef* Brigands = Camps.FindByPredicate([](const HOLCampTable::FCampDef& Camp) { return Camp.Name == TEXT("Brigands"); });
	if (TestNotNull(TEXT("the Brigands camp is in the table"), Brigands))
	{
		for (const TCHAR* Type : { TEXT("Spearman"), TEXT("ManAtArms"), TEXT("Archer"), TEXT("Crossbowman") })
		{
			TestTrue(FString::Printf(TEXT("the Brigands have a %s"), Type), Brigands->Members.Contains(FName(Type)));
		}
		for (const FName Member : Brigands->Members)
		{
			TestFalse(FString::Printf(TEXT("the Brigands do not field a %s"), *Member.ToString()),
				Member == TEXT("Peasant") || Member == TEXT("King") || Member == TEXT("Hero"));
		}
	}

	// 3. The check itself: the same table with a made-up type must be refused, naming the type; a type of the table must pass.
	TArray<HOLCampTable::FCampDef> Wrong;
	const FString WrongText = CampsText.TrimEnd() + TEXT("\nWrongCamp,Wrong camp,Spearman;Dragonling\n");
	const TArray<FString> WrongProblems = Check(WrongText, UnitsText, Wrong);
	TestTrue(TEXT("a camp with a type unknown to UnitTypes.csv is refused"),
		WrongProblems.ContainsByPredicate([](const FString& Problem) { return Problem.Contains(TEXT("Dragonling")); }));
	TestFalse(TEXT("the known type of that same camp is not blamed"),
		WrongProblems.ContainsByPredicate([](const FString& Problem) { return Problem.Contains(TEXT("type Spearman")); }));

	// 4. The shape: a missing members cell, a duplicate name and an empty member are problems, not silently dropped.
	TArray<HOLCampTable::FCampDef> Shaped;
	TArray<FString> ShapeProblems;
	HOLCampTable::Parse(TEXT("Name,DisplayName,Members\nA,Camp A\nB,Camp B,Spearman\nB,Camp B again,Archer\nC,Camp C,Spearman;;Archer\n"), Shaped, ShapeProblems);
	TestEqual(TEXT("one problem per bad line (missing cell, duplicate name, empty member)"), ShapeProblems.Num(), 3);
	TestEqual(TEXT("only the good first B is kept"), Shaped.Num(), 1);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
