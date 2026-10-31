// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/DataTable.h"
#include "Match/HOLMarketTypes.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

// M2 (Market, GDD 5.5): the table Tools/Data/Market.csv, read through the same import as the editor's
// (UDataTable::CreateTableFromCSVString, what make_market.py does into /Game/Match/MarketRates). The expected numbers are the
// user's answer MK-Q1 = C (D582), typed here on purpose (the script test of HOLMarketRules.as holds the same figures: the
// two sources must agree).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMarketTableTest, "HeroesOfLiberty.Market.Table",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLMarketTableTest::RunTest(const FString& Parameters)
{
	const FString Path = FPaths::Combine(FPaths::ProjectDir(), TEXT("Tools"), TEXT("Data"), TEXT("Market.csv"));
	FString Text;
	if (!TestTrue(FString::Printf(TEXT("%s is there"), *Path), FFileHelper::LoadFileToString(Text, *Path)))
	{
		return false;
	}
	UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
	Table->RowStruct = FHOLMarketRow::StaticStruct();
	const TArray<FString> Problems = Table->CreateTableFromCSVString(Text);
	for (const FString& Problem : Problems)
	{
		AddError(FString::Printf(TEXT("Market.csv: %s"), *Problem));
	}
	if (!Problems.IsEmpty())
	{
		return false;
	}

	const FHOLMarketRow* Row = Table->FindRow<FHOLMarketRow>(TEXT("Standard"), TEXT("market rates"), false);
	if (!TestNotNull(TEXT("the Standard row is in the table"), Row))
	{
		return false;
	}

	// 1. The numbers of MK-Q1 = C (D582).
	TestEqual(TEXT("food base price"), Row->FoodPrice, 80);
	TestEqual(TEXT("wood base price"), Row->WoodPrice, 80);
	TestEqual(TEXT("stone base price"), Row->StonePrice, 120);
	TestEqual(TEXT("gold base price"), Row->GoldPrice, 100);
	TestEqual(TEXT("tax in percent"), Row->FeePercent, 30);
	TestEqual(TEXT("points a lot"), Row->Step, 3);
	TestEqual(TEXT("points back"), Row->RecoverPoints, 1);
	TestEqual(TEXT("seconds between two returns"), Row->RecoverEverySeconds, 10);
	TestEqual(TEXT("floor"), Row->MinPrice, 20);
	TestEqual(TEXT("cap"), Row->MaxPrice, 300);

	// 2. Coherence, whatever the numbers become: bounds in order, every base price inside them, a real tax and step.
	for (const TPair<FName, uint8*>& Pair : Table->GetRowMap())
	{
		const FHOLMarketRow& Each = *reinterpret_cast<const FHOLMarketRow*>(Pair.Value);
		const FString Name = Pair.Key.ToString();
		TestTrue(FString::Printf(TEXT("%s: floor below cap"), *Name), Each.MinPrice < Each.MaxPrice);
		const int32 Bases[] = { Each.FoodPrice, Each.WoodPrice, Each.StonePrice, Each.GoldPrice };
		for (const int32 Base : Bases)
		{
			TestTrue(FString::Printf(TEXT("%s: base price %d within the bounds"), *Name, Base), Base >= Each.MinPrice && Base <= Each.MaxPrice);
		}
		TestTrue(FString::Printf(TEXT("%s: tax between 0 and 99 percent"), *Name), Each.FeePercent >= 0 && Each.FeePercent < 100);
		TestTrue(FString::Printf(TEXT("%s: the price moves with a lot"), *Name), Each.Step > 0);
		TestTrue(FString::Printf(TEXT("%s: the price comes back"), *Name), Each.RecoverPoints > 0 && Each.RecoverEverySeconds > 0);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
