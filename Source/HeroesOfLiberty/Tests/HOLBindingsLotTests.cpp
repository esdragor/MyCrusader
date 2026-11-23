// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIGarrisonOrders.h"
#include "AI/HOLAISummary.h"
#include "Misc/AutomationTest.h"
#include "Player/HOLCommandGrid.h"
#include "Player/HOLCommandGridReadLibrary.h"
#include "Simulation/HOLUnitScriptLibrary.h"
#include "Simulation/HOLUnitView.h"

#if WITH_DEV_AUTOMATION_TESTS

// ELROND-1123e: the grouped C++ bindings (D839). One test per binding that reads a value: the AI summary's garrison kind (S4-b), the grid's LightSeal
// (the two enums keep the same order), the unit owner and type of a UnitId unknown to the machine, the AI garrison order refused without a world.
namespace UE::HeroesOfLiberty::Tests::BindingsLot
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FHOLUnitView MakeUnit(const uint32 Id, const FName Type, const bool bSiege, const bool bRanged, const bool bHero)
	{
		FHOLUnitView Unit;
		Unit.UnitId = Id;
		Unit.OwnerPlayer = 0;
		Unit.TypeName = Type;
		Unit.bSiege = bSiege;
		Unit.bRanged = bRanged;
		Unit.bHero = bHero;
		Unit.Activity = EHOLUnitActivity::Idle;
		return Unit;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBindingsAIGarrisonKindTest, "HeroesOfLiberty.AI.Summary.GarrisonKind", UE::HeroesOfLiberty::Tests::BindingsLot::Flags)
bool FHOLBindingsAIGarrisonKindTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::BindingsLot;
	const TArray<FHOLUnitView> Units = {
		MakeUnit(1, FName(TEXT("Spearman")), false, false, false), MakeUnit(2, FName(TEXT("Archer")), false, true, false) };
	const FHOLAISummary Slot0 = HOLAISummary::Make(0, Units, {}, {}, {});
	TMap<int32, int32> KindById;
	for (const FHOLAIUnit& Unit : Slot0.Army)
	{
		KindById.Add(Unit.Id, Unit.GarrisonKind);
	}
	// HOLGarrison::EKind: 0 melee, 1 ranged. A unit left out of the Army reads as INDEX_NONE and fails.
	TestEqual(TEXT("Spearman: melee infantry"), KindById.FindRef(1), 0);
	TestEqual(TEXT("Archer: ranged infantry"), KindById.FindRef(2), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBindingsLightSealTest, "HeroesOfLiberty.Player.CommandGrid.LightSealEnum", UE::HeroesOfLiberty::Tests::BindingsLot::Flags)
bool FHOLBindingsLightSealTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("LightSeal has the same number in the rule's enum and the script's UENUM"), static_cast<uint8>(EHOLGridCommandKind::LightSeal), static_cast<uint8>(EHOLGridCommand::LightSeal));
	TestEqual(TEXT("LightSeal follows UngarrisonAll"), static_cast<int32>(EHOLGridCommand::LightSeal), static_cast<int32>(EHOLGridCommand::UngarrisonAll) + 1);
	FHOLGridContext Context;
	TestFalse(TEXT("bMonkUnit false by default"), Context.bMonkUnit);
	FHOLGridContextView View;
	TestFalse(TEXT("the view's bMonkUnit false by default"), View.bMonkUnit);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBindingsUnitOwnerTypeTest, "HeroesOfLiberty.Simulation.UnitScriptLibrary.OwnerType", UE::HeroesOfLiberty::Tests::BindingsLot::Flags)
bool FHOLBindingsUnitOwnerTypeTest::RunTest(const FString& Parameters)
{
	int32 Owner = 5;
	TestFalse(TEXT("no world: the owner is not found"), UHOLUnitScriptLibrary::GetUnitOwner(nullptr, 7, Owner));
	TestEqual(TEXT("no world: the owner slot is INDEX_NONE"), Owner, static_cast<int32>(INDEX_NONE));
	TestEqual(TEXT("no world: the type is none"), UHOLUnitScriptLibrary::GetUnitType(nullptr, 7), FName());
	TestFalse(TEXT("no world: the AI garrison order is refused"), UHOLAIGarrisonOrderLibrary::IssueAIGarrisonOrder(nullptr, 0, TArray<int32>{ 7 }, nullptr));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
