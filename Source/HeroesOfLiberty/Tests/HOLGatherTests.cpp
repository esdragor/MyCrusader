// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypes.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLGatherRules.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// J5 checks (D299): the gathering rules (D296) and the provisional values of the tables (D296, D297).
namespace UE::HeroesOfLiberty::Tests::Gather
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** The fixed step of D217 (20 Hz). */
	constexpr float StepSeconds = 0.05f;

	/** Gathers Steps steps in a row; returns the units taken. */
	int32 GatherSteps(FHOLGatherLoad& Load, const int32 Steps, const float Rate, const int32 Capacity, int32& SourceRemaining)
	{
		int32 Taken = 0;
		for (int32 Step = 0; Step < Steps; ++Step)
		{
			Taken += HOLGather::Gather(Load, Rate, StepSeconds, Capacity, SourceRemaining);
		}
		return Taken;
	}

	FHOLGatherSource MakeSource(const double X, const double Y, const EHOLResource Resource, const int32 Remaining)
	{
		FHOLGatherSource Source;
		Source.Site.Position = FVector2D(X, Y);
		Source.Site.Radius = 60.f;
		Source.Resource = Resource;
		Source.Remaining = Remaining;
		return Source;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGatherLoadTest, "HeroesOfLiberty.Gather.Load", UE::HeroesOfLiberty::Tests::Gather::Flags)
bool FHOLGatherLoadTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Gather;
	FHOLGatherLoad Load;
	HOLGather::StartGathering(Load, EHOLResource::Wood);
	int32 Tree = 100;

	// 0.7 a second: nothing whole after 1 s, one unit after 2 s.
	TestEqual(TEXT("nothing whole after 1 s"), GatherSteps(Load, 20, 0.7f, 10, Tree), 0);
	TestEqual(TEXT("progress kept between steps"), Load.Progress, 0.7f, 1e-4f);
	TestEqual(TEXT("one unit after 2 s"), GatherSteps(Load, 20, 0.7f, 10, Tree), 1);
	TestEqual(TEXT("taken from the source"), Tree, 99);

	// A load of 10 takes 10 / 0.7 = 14.3 s: 9 after 285 steps, 10 after 286.
	GatherSteps(Load, 245, 0.7f, 10, Tree);
	TestEqual(TEXT("9 after 285 steps"), Load.Amount, 9);
	TestFalse(TEXT("not full at 9"), HOLGather::IsFull(Load, 10));
	GatherSteps(Load, 1, 0.7f, 10, Tree);
	TestEqual(TEXT("10 after 286 steps"), Load.Amount, 10);
	TestTrue(TEXT("full at 10"), HOLGather::IsFull(Load, 10));
	TestEqual(TEXT("10 taken from the tree"), Tree, 90);
	TestEqual(TEXT("no progress kept once full"), Load.Progress, 0.f);

	// Full: nothing more is taken.
	TestEqual(TEXT("a full load takes nothing"), GatherSteps(Load, 100, 0.7f, 10, Tree), 0);
	TestEqual(TEXT("the tree keeps the rest"), Tree, 90);

	// A rate of 0 (a soldier, not a gatherer) takes nothing.
	FHOLGatherLoad Soldier;
	TestEqual(TEXT("rate 0 takes nothing"), GatherSteps(Soldier, 400, 0.f, 10, Tree), 0);

	// A used-up source: the gatherer keeps what it took, short of a full load.
	FHOLGatherLoad Last;
	HOLGather::StartGathering(Last, EHOLResource::Wood);
	int32 Stump = 3;
	TestEqual(TEXT("the last 3 units"), GatherSteps(Last, 400, 0.7f, 10, Stump), 3);
	TestEqual(TEXT("source used up"), Stump, 0);
	TestEqual(TEXT("load of 3"), Last.Amount, 3);
	TestFalse(TEXT("not full"), HOLGather::IsFull(Last, 10));
	TestEqual(TEXT("no progress kept on an empty source"), Last.Progress, 0.f);
	TestEqual(TEXT("an empty source gives nothing"), GatherSteps(Last, 100, 0.7f, 10, Stump), 0);
	TestEqual(TEXT("never below 0"), Stump, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGatherDepositTest, "HeroesOfLiberty.Gather.Deposit", UE::HeroesOfLiberty::Tests::Gather::Flags)
bool FHOLGatherDepositTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Gather;
	FHOLResources Stock(200, 200, 200, 200);
	FHOLGatherLoad Load;
	HOLGather::StartGathering(Load, EHOLResource::Wood);
	int32 Tree = 100;
	GatherSteps(Load, 300, 0.7f, 10, Tree);
	TestTrue(TEXT("the stock does not rise while gathering"), Stock == FHOLResources(200, 200, 200, 200));

	TestEqual(TEXT("10 deposited"), HOLGather::Deposit(Load, Stock), 10);
	TestTrue(TEXT("the wood rises at the deposit, alone"), Stock == FHOLResources(200, 210, 200, 200));
	TestEqual(TEXT("load emptied"), Load.Amount, 0);
	TestEqual(TEXT("an empty load deposits nothing"), HOLGather::Deposit(Load, Stock), 0);
	TestEqual(TEXT("wood unchanged"), Stock.Wood, 210);

	// Back to the same resource: the load is kept (a move order also keeps it, D296).
	GatherSteps(Load, 210, 0.7f, 10, Tree);
	TestEqual(TEXT("7 wood carried"), Load.Amount, 7);
	TestEqual(TEXT("same resource: nothing lost"), HOLGather::StartGathering(Load, EHOLResource::Wood), 0);
	TestEqual(TEXT("load kept"), Load.Amount, 7);

	// Another resource: the load is lost, as in AoE.
	TestEqual(TEXT("berries: the 7 wood are lost"), HOLGather::StartGathering(Load, EHOLResource::Food), 7);
	TestEqual(TEXT("load emptied"), Load.Amount, 0);
	TestEqual(TEXT("now carries food"), int32(Load.Resource), int32(EHOLResource::Food));
	int32 Bush = 200;
	GatherSteps(Load, 100, 0.7f, 10, Bush);
	HOLGather::Deposit(Load, Stock);
	TestTrue(TEXT("food deposited, the lost wood never"), Stock == FHOLResources(203, 210, 200, 200));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGatherChoiceTest, "HeroesOfLiberty.Gather.Choice", UE::HeroesOfLiberty::Tests::Gather::Flags)
bool FHOLGatherChoiceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Gather;

	// Nearest deposit, measured to its footprint: a large centre farther away may be the nearer one.
	const TArray<FHOLGatherDeposit> Deposits = {
		{ FVector2D(2000., 0.), FVector2D(100., 100.), 0.f },
		{ FVector2D(-2400., 0.), FVector2D(750., 750.), 0.f },
	};
	TestEqual(TEXT("the nearer footprint wins"), HOLGather::FindNearestDeposit(FVector2D::ZeroVector, Deposits), 1);
	TestEqual(TEXT("near the small one"), HOLGather::FindNearestDeposit(FVector2D(1500., 0.), Deposits), 0);
	TestEqual(TEXT("no deposit"), HOLGather::FindNearestDeposit(FVector2D::ZeroVector, TConstArrayView<FHOLGatherDeposit>()), int32(INDEX_NONE));
	TestEqual(TEXT("distance to a footprint"), HOLGather::GetDepositDistance(FVector2D::ZeroVector, Deposits[1]), 1650., 0.01);
	TestEqual(TEXT("distance to a corner"), HOLGather::GetDepositDistance(FVector2D(-1350., 1150.), Deposits[1]), 500., 0.01);
	TestEqual(TEXT("0 on the footprint"), HOLGather::GetDepositDistance(FVector2D(-2400., 100.), Deposits[1]), 0., 0.01);

	// A turned footprint (45 degrees): its corner points along X, 141 cm from its centre.
	const FHOLGatherDeposit Turned = { FVector2D::ZeroVector, FVector2D(100., 100.), 45.f };
	TestEqual(TEXT("turned: 100 cm past the corner"), HOLGather::GetDepositDistance(FVector2D(241.42, 0.), Turned), 100., 0.5);

	// Unloading: against the side it faces, Gap cm out, slid along it for a second try; close enough within 150 cm.
	const FHOLGatherDeposit& Centre = Deposits[1];
	const FVector2D Spot = HOLGather::GetDepositSpot(FVector2D(-2400., 3000.), Centre, 70.);
	TestTrue(TEXT("spot against the facing side"), Spot.Equals(FVector2D(-2400., 820.), 0.01));
	TestTrue(TEXT("spot slid along the side"), HOLGather::GetDepositSpot(FVector2D(-2400., 3000.), Centre, 70., 150.).Equals(FVector2D(-2250., 820.), 0.01));
	TestTrue(TEXT("spot stays on the side"), HOLGather::GetDepositSpot(FVector2D(-1000., 3000.), Centre, 70., 150.).Equals(FVector2D(-1650., 820.), 0.01));
	TestTrue(TEXT("unloads at its spot"), HOLGather::IsAtDeposit(Spot, 60.f, Centre));
	TestFalse(TEXT("too far to unload"), HOLGather::IsAtDeposit(FVector2D(-2400., 1000.), 60.f, Centre));
	TestEqual(TEXT("edge distance of a source"), HOLGather::GetEdgeDistance(FVector2D::ZeroVector, FHOLGatherSite{ FVector2D(-2400., 0.), 750.f }), 1650.);

	// Contact: body against body, give or take 30 cm.
	const FHOLGatherSite Tree = { FVector2D::ZeroVector, 60.f };
	TestTrue(TEXT("in contact at the edge"), HOLGather::IsInContact(FVector2D(120., 0.), 60.f, Tree));
	TestTrue(TEXT("in contact 30 cm off"), HOLGather::IsInContact(FVector2D(150., 0.), 60.f, Tree));
	TestFalse(TEXT("not in contact 50 cm off"), HOLGather::IsInContact(FVector2D(170., 0.), 60.f, Tree));

	// A used-up tree: the nearest tree within 15 m that still holds wood; berries, empty trees and far trees are not.
	const TArray<FHOLGatherSource> Sources = {
		MakeSource(0., 0., EHOLResource::Wood, 0),         // the used-up tree
		MakeSource(300., 0., EHOLResource::Food, 200),     // berries, nearer
		MakeSource(400., 0., EHOLResource::Wood, 0),       // empty tree, nearer
		MakeSource(900., 0., EHOLResource::Wood, 100),
		MakeSource(0., -700., EHOLResource::Wood, 40),     // the nearest tree that holds wood
		MakeSource(1600., 0., EHOLResource::Wood, 100),    // 16 m away
	};
	TestEqual(TEXT("nearest tree with wood"), HOLGather::FindNeighbourSource(FVector2D::ZeroVector, EHOLResource::Wood, Sources), 4);
	TestEqual(TEXT("berries for food"), HOLGather::FindNeighbourSource(FVector2D::ZeroVector, EHOLResource::Food, Sources), 1);
	TestEqual(TEXT("no gold: idle"), HOLGather::FindNeighbourSource(FVector2D::ZeroVector, EHOLResource::Gold, Sources), int32(INDEX_NONE));
	TestEqual(TEXT("nothing within 15 m: idle"),
		HOLGather::FindNeighbourSource(FVector2D(3200., 0.), EHOLResource::Wood, Sources), int32(INDEX_NONE));
	TestEqual(TEXT("exactly 15 m is within reach"),
		HOLGather::FindNeighbourSource(FVector2D(3100., 0.), EHOLResource::Wood, Sources), 5);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGatherTablesTest, "HeroesOfLiberty.Gather.Tables", UE::HeroesOfLiberty::Tests::Gather::Flags)
bool FHOLGatherTablesTest::RunTest(const FString& Parameters)
{
	struct FExpectedSource
	{
		FName Name;
		EHOLResource Resource;
		int32 Amount;
	};
	const FExpectedSource Expected[] = {
		{ HOLSourceTypeNames::Tree, EHOLResource::Wood, 100 },
		{ HOLSourceTypeNames::BerryBush, EHOLResource::Food, 200 },
		{ HOLSourceTypeNames::StoneVein, EHOLResource::Stone, 2500 },
		{ HOLSourceTypeNames::GoldVein, EHOLResource::Gold, 2500 },
	};
	for (const FExpectedSource& Source : Expected)
	{
		const FString Name = Source.Name.ToString();
		const FHOLSourceTypeRow* Row = UHOLSourceSettings::FindType(Source.Name);
		if (TestNotNull(*FString::Printf(TEXT("%s in the source type table"), *Name), Row))
		{
			TestEqual(*FString::Printf(TEXT("%s: resource"), *Name), int32(Row->Resource), int32(Source.Resource));
			TestEqual(*FString::Printf(TEXT("%s: amount (D297)"), *Name), Row->Amount, Source.Amount);
			TestTrue(*FString::Printf(TEXT("%s: a body"), *Name), Row->Radius > 0.f);
		}
	}

	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	if (TestNotNull(TEXT("unit type table"), UnitTypes))
	{
		const FHOLUnitTypeRow* Peasant = UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("Peasant"), TEXT("Gather test"));
		if (TestNotNull(TEXT("Peasant in the unit type table"), Peasant))
		{
			TestEqual(TEXT("Peasant carries 10 (D296)"), Peasant->CarryCapacity, 10);
			TestTrue(TEXT("Peasant gathers the 4 resources at 0.7/s (D296)"), Peasant->GatherRates == FHOLResourceRates(0.7f, 0.7f, 0.7f, 0.7f));
			const FHOLUnitTypeConstants Constants = Peasant->ToConstants(TEXT("Peasant"));
			TestEqual(TEXT("carry reaches the shared fragment"), Constants.CarryCapacity, 10);
			TestEqual(TEXT("wood rate reaches the shared fragment"), Constants.GatherRates[EHOLResource::Wood], 0.7f);
		}
		const FHOLUnitTypeRow* Infantry = UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("LightInfantry"), TEXT("Gather test"));
		if (TestNotNull(TEXT("LightInfantry in the unit type table"), Infantry))
		{
			TestEqual(TEXT("a soldier carries nothing"), Infantry->CarryCapacity, 0);
			TestTrue(TEXT("a soldier gathers nothing"), Infantry->GatherRates == FHOLResourceRates());
		}
	}

	const FHOLBuildingTypeRow* MainCentre = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::MainCentre);
	if (TestNotNull(TEXT("main centre in the building type table"), MainCentre))
	{
		TestTrue(TEXT("the main centre takes the four resources (D295)"), MainCentre->AcceptsResource(EHOLResource::Food)
			&& MainCentre->AcceptsResource(EHOLResource::Wood) && MainCentre->AcceptsResource(EHOLResource::Stone)
			&& MainCentre->AcceptsResource(EHOLResource::Gold));
	}
	return true;
}

#endif
