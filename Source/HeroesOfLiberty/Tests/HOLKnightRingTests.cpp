// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "Player/HOLSelectionMarkers.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// The Virtuous Knight's ring on the ground (D864 + D865): the 8 m of its damage redirect, drawn to its owner alone. The radius is read from
// the unit type table (FHOLUnitTypeRow::DamageRedirectRadius), never a literal; the text is the line AuraRingsDescription carries.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLKnightRingTest, "HeroesOfLiberty.SelectionMarkers.KnightRing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLKnightRingTest::RunTest(const FString& Parameters)
{
	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	const FHOLUnitTypeRow* KnightRow = UnitTypes ? UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("VirtuousKnight"), TEXT("Knight ring test"), false) : nullptr;
	if (!TestNotNull(TEXT("VirtuousKnight in the unit type table"), KnightRow))
	{
		return false;
	}
	TestTrue(TEXT("the Knight's row redirects something"), KnightRow->DamageRedirectPercent > 0 && KnightRow->DamageRedirectRadius > 0.f);

	const int32 Slot = 0;
	FHOLUnitView Own;
	Own.UnitId = 1;
	Own.OwnerPlayer = Slot;
	Own.TypeName = TEXT("VirtuousKnight");
	Own.Location = FVector(1200.0, -3400.0, 150.0);
	FHOLUnitView Enemy = Own;
	Enemy.UnitId = 2;
	Enemy.OwnerPlayer = 1;
	Enemy.Location = FVector(5000.0, 0.0, 150.0);
	FHOLUnitView Plain = Own;
	Plain.UnitId = 3;
	Plain.TypeName = TEXT("Halberdier");
	Plain.Location = FVector(0.0, 0.0, 150.0);
	TArray<FHOLUnitView> Views;
	Views.Add(Plain);
	Views.Add(Enemy);
	Views.Add(Own);

	TArray<AHOLSelectionMarkers::FKnightRing> Rings;
	AHOLSelectionMarkers::CollectKnightRings(Views, Slot, UnitTypes, Rings);
	if (!TestEqual(TEXT("one ring: the player's own Knight (not the enemy's, not a Halberdier)"), Rings.Num(), 1))
	{
		return false;
	}
	TestEqual(TEXT("the ring's radius is the table's DamageRedirectRadius"), static_cast<float>(Rings[0].Radius), KnightRow->DamageRedirectRadius);
	TestEqual(TEXT("the ring is centred on the Knight"), Rings[0].Location, Own.Location);
	const FString Expected = FString::Printf(TEXT("knight (%.0f, %.0f) %.0f"), Own.Location.X, Own.Location.Y, KnightRow->DamageRedirectRadius);
	TestEqual(TEXT("AuraRingsDescription's part, from the table"), Rings[0].Description, Expected);
	TestTrue(TEXT("the part names the knight"), Rings[0].Description.StartsWith(TEXT("knight (")));

	Views.Reset();
	AHOLSelectionMarkers::CollectKnightRings(Views, Slot, UnitTypes, Rings);
	TestEqual(TEXT("no Knight on the map, no ring"), Rings.Num(), 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
