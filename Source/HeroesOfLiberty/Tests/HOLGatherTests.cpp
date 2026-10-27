// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Commands/HOLPlayerOrders.h"
#include "Commands/HOLUnitOrderProcessor.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceLayout.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLGatherProcessor.h"
#include "Simulation/HOLGatherRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// J5 checks (D299): the gathering rules (D296) and the provisional values of the tables (D296, D297).
namespace UE::HeroesOfLiberty::Tests::Gather
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** The fixed step of D217 (20 Hz). */
	constexpr float StepSeconds = 0.05f;

	/** Gathers Resource Steps steps in a row; returns the units taken (the load lost added to OutLost, D306). */
	int32 GatherSteps(FHOLGatherLoad& Load, const int32 Steps, const float Rate, const int32 Capacity, int32& SourceRemaining,
		const EHOLResource Resource = EHOLResource::Wood, int32* OutLost = nullptr)
	{
		int32 Taken = 0;
		for (int32 Step = 0; Step < Steps; ++Step)
		{
			Taken += HOLGather::Gather(Load, Resource, Rate, StepSeconds, Capacity, SourceRemaining, OutLost);
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

	/** "1 3 0": the sources a search tried, in order. */
	FString JoinIndices(const TArray<int32>& Indices)
	{
		return FString::JoinBy(Indices, TEXT(" "), [](const int32 Index) { return FString::FromInt(Index); });
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
	HOLGather::StartGathering(Load, EHOLResource::Wood);
	TestEqual(TEXT("same resource: load kept"), Load.Amount, 7);

	// Another resource (D306): the load is kept at the order and while no whole unit of it is gathered, lost at the first.
	HOLGather::StartGathering(Load, EHOLResource::Food);
	TestEqual(TEXT("order on berries: the 7 wood kept"), Load.Amount, 7);
	TestEqual(TEXT("still carries wood"), int32(Load.Resource), int32(EHOLResource::Wood));
	int32 Bush = 200;
	int32 Lost = 0;
	TestEqual(TEXT("nothing whole after 1 s of berries"), GatherSteps(Load, 20, 0.7f, 10, Bush, EHOLResource::Food, &Lost), 0);
	TestEqual(TEXT("the 7 wood still carried"), Load.Amount, 7);
	TestEqual(TEXT("nothing lost yet"), Lost, 0);
	TestEqual(TEXT("first food after 2 s"), GatherSteps(Load, 20, 0.7f, 10, Bush, EHOLResource::Food, &Lost), 1);
	TestEqual(TEXT("the 7 wood lost then"), Lost, 7);
	TestEqual(TEXT("now carries 1 food"), Load.Amount, 1);
	TestEqual(TEXT("now carries food"), int32(Load.Resource), int32(EHOLResource::Food));
	GatherSteps(Load, 60, 0.7f, 10, Bush, EHOLResource::Food);
	HOLGather::Deposit(Load, Stock);
	TestTrue(TEXT("food deposited, the lost wood never"), Stock == FHOLResources(203, 210, 200, 200));

	// A full load of another resource is not full of this one: it gathers on, the progress toward wood not counted.
	FHOLGatherLoad Full;
	Full.Resource = EHOLResource::Wood;
	Full.Amount = 10;
	Full.Progress = 0.5f;
	Full.ProgressResource = EHOLResource::Wood;
	TestTrue(TEXT("full of wood"), HOLGather::IsFullOf(Full, EHOLResource::Wood, 10));
	TestFalse(TEXT("not full of food"), HOLGather::IsFullOf(Full, EHOLResource::Food, 10));
	int32 FullLost = 0;
	TestEqual(TEXT("full of wood: nothing whole after 1 s of berries"), GatherSteps(Full, 20, 0.7f, 10, Bush, EHOLResource::Food, &FullLost), 0);
	TestEqual(TEXT("then its first food"), GatherSteps(Full, 20, 0.7f, 10, Bush, EHOLResource::Food, &FullLost), 1);
	TestEqual(TEXT("the 10 wood lost"), FullLost, 10);

	// An empty load takes the ordered resource at once (the HUD shows it on the way).
	FHOLGatherLoad Empty;
	HOLGather::StartGathering(Empty, EHOLResource::Gold);
	TestEqual(TEXT("empty: gold at the order"), int32(Empty.Resource), int32(EHOLResource::Gold));
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

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGatherMassifTest, "HeroesOfLiberty.Gather.Massif", UE::HeroesOfLiberty::Tests::Gather::Flags)
bool FHOLGatherMassifTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Gather;
	// A massif (D309, the user's rule at the 3rd trial of J5b.2): the sources of one resource linked step by step, 8 m
	// apart at most; a gatherer with no room looks through it from its source outward.
	TArray<FHOLGatherSource> Sources;
	for (int32 Tree = 0; Tree < 6; ++Tree)
	{
		// A chain of trees 6 m apart: 30 m from end to end, every one reached step by step.
		Sources.Add(MakeSource(600. * Tree, 0., EHOLResource::Wood, Tree == 3 ? 0 : 100)); // tree 3 used up
	}
	Sources.Add(MakeSource(5000., 0., EHOLResource::Wood, 100));     // 6: 20 m beyond the chain's end
	Sources.Add(MakeSource(1200., 300., EHOLResource::Food, 200));   // 7: berries beside tree 2
	Sources.Add(MakeSource(0., 10000., EHOLResource::Stone, 1000));  // 8: a vein
	Sources.Add(MakeSource(6000., 10000., EHOLResource::Stone, 1000)); // 9: another vein, 60 m away
	FHOLSourceMassifs Massifs;
	Massifs.Build(Sources, HOLGather::NoRoomSourceDistance);
	TestEqual(TEXT("built for every source"), Massifs.Num(), Sources.Num());
	TestEqual(TEXT("the chain's end: one link"), Massifs.GetLinks(0).Num(), 1);
	TestEqual(TEXT("berries beside a tree: no link to it"), Massifs.GetLinks(7).Num(), 0);
	TestEqual(TEXT("a vein alone"), Massifs.GetLinks(8).Num(), 0);

	// Every source refused: the order they were tried in, from tree 2 outward.
	TArray<int32> Tried;
	const auto Refuse = [&Tried](const int32 Index)
	{
		Tried.Add(Index);
		return false;
	};
	TestEqual(TEXT("the whole chain full: none"), Massifs.Search(2, Refuse), int32(INDEX_NONE));
	TestEqual(TEXT("nearest first, a tie to the first index, past the used-up tree 3, tree 2 itself not tried"), JoinIndices(Tried), FString(TEXT("1 3 0 4 5")));
	TestFalse(TEXT("the tree 20 m away never reached"), Tried.Contains(6));
	TestFalse(TEXT("nor the berries beside it"), Tried.Contains(7));

	// The first one with room, nearest first; the used-up tree refused leads on to the chain beyond it.
	Tried.Reset();
	const auto RoomAt5 = [&Tried, &Sources](const int32 Index)
	{
		Tried.Add(Index);
		return Sources[Index].Remaining > 0 && Index == 5;
	};
	TestEqual(TEXT("tree 5, at the far end"), Massifs.Search(0, RoomAt5), 5);
	TestEqual(TEXT("each tree tried once, in order"), JoinIndices(Tried), FString(TEXT("1 2 3 4 5")));
	Tried.Reset();
	TestEqual(TEXT("the first one with room stops the search"), Massifs.Search(0, [&Tried](const int32 Index) { Tried.Add(Index); return Index != 3; }), 1);
	TestEqual(TEXT("nothing tried beyond it"), Tried.Num(), 1);
	TestEqual(TEXT("from the tree 20 m away: nothing"), Massifs.Search(6, [](int32) { return true; }), int32(INDEX_NONE));
	TestEqual(TEXT("a vein never leads to another"), Massifs.Search(8, [](int32) { return true; }), int32(INDEX_NONE));

	// A wood (7 x 7 trees 3 m apart) clicked at a corner tree: it fills from there outward, as far as its farthest tree.
	TArray<FHOLGatherSource> Wood;
	for (int32 Y = 0; Y < 7; ++Y)
	{
		for (int32 X = 0; X < 7; ++X)
		{
			Wood.Add(MakeSource(300. * X, 300. * Y, EHOLResource::Wood, 100));
		}
	}
	FHOLSourceMassifs WoodMassifs;
	WoodMassifs.Build(Wood, HOLGather::NoRoomSourceDistance);
	TArray<int32> Filled;
	WoodMassifs.Search(0, [&Filled](const int32 Index) { Filled.Add(Index); return false; });
	TestEqual(TEXT("every other tree of the wood tried"), Filled.Num(), Wood.Num() - 1);
	bool bOutward = true;
	for (int32 Index = 1; Index < Filled.Num(); ++Index)
	{
		bOutward &= FVector2D::Distance(Wood[Filled[Index]].Site.Position, Wood[0].Site.Position)
			>= FVector2D::Distance(Wood[Filled[Index - 1]].Site.Position, Wood[0].Site.Position) - 0.01;
	}
	TestTrue(TEXT("from the tree clicked outward"), bOutward);
	TestEqual(TEXT("the opposite corner last"), Filled.Last(), Wood.Num() - 1);

	// Toward a deposit (the user's trial of D309, after AoE IV): the edge facing it first. The wood of Plain: 8 trees
	// along X by 5 rows 2.2 m apart, its near row at Y = 0; a main centre (15 m square) whose side is 10.5 m in front.
	TArray<FHOLGatherSource> Plain;
	for (int32 Row = 0; Row < 5; ++Row)
	{
		for (int32 Column = 0; Column < 8; ++Column)
		{
			Plain.Add(MakeSource((Column - 3.5) * 220., -220. * Row, EHOLResource::Wood, 100));
		}
	}
	const FHOLGatherDeposit Centre{ FVector2D(0., 1800.), FVector2D(750., 750.), 0.f };
	FHOLSourceMassifs PlainMassifs;
	PlainMassifs.Build(Plain, HOLGather::NoRoomSourceDistance);
	for (const int32 Clicked : { 3, 0 })
	{
		// Clicked in the middle of the near row, then at its end: every other tree refused, in the order tried.
		TArray<int32> PlainTried;
		PlainMassifs.Search(Clicked, [&PlainTried](const int32 Index) { PlainTried.Add(Index); return false; }, &Centre);
		TestEqual(*FString::Printf(TEXT("tree %d clicked: every other tree tried"), Clicked), PlainTried.Num(), Plain.Num() - 1);
		int32 LastNear = INDEX_NONE;
		int32 FirstBack = MAX_int32;
		bool bAlongFromClicked = true;
		double LastAlong = 0.;
		for (int32 Order = 0; Order < PlainTried.Num(); ++Order)
		{
			const int32 Row = PlainTried[Order] / 8;
			if (Row == 0)
			{
				LastNear = Order;
				const double Along = FMath::Abs(Plain[PlainTried[Order]].Site.Position.X - Plain[Clicked].Site.Position.X);
				bAlongFromClicked &= Along >= LastAlong - 0.01;
				LastAlong = Along;
			}
			else if (Row >= 3)
			{
				FirstBack = FMath::Min(FirstBack, Order);
			}
		}
		TestTrue(*FString::Printf(TEXT("tree %d clicked: the near edge along from it (%s)"), Clicked, *JoinIndices(PlainTried)), bAlongFromClicked);
		TestTrue(*FString::Printf(TEXT("tree %d clicked: the whole near edge before the back of the wood (%s)"), Clicked, *JoinIndices(PlainTried)),
			LastNear < FirstBack);
	}
	// With no deposit, the nearest the tree clicked first, as before: the tree behind it before the end of its row.
	TArray<int32> Plainly;
	PlainMassifs.Search(0, [&Plainly](const int32 Index) { Plainly.Add(Index); return false; });
	TestTrue(TEXT("no deposit: the tree 2.2 m behind before the tree 6.6 m along"), Plainly.IndexOfByKey(8) < Plainly.IndexOfByKey(3));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGatherNearbyMassifTest, "HeroesOfLiberty.Gather.NearbyMassif", UE::HeroesOfLiberty::Tests::Gather::Flags)
bool FHOLGatherNearbyMassifTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Gather;
	// A whole massif full (D309 point 6, the user's choice): a nearby grove of the same resource, ~20 m at most from one
	// of its sources, in the same order as the massif; none, the idle ones wait in front on rows without limit.
	TArray<FHOLGatherSource> Sources;
	Sources.Add(MakeSource(0., 0., EHOLResource::Wood, 100));        // 0: wood A, clicked
	Sources.Add(MakeSource(500., 0., EHOLResource::Wood, 100));      // 1: wood A
	Sources.Add(MakeSource(1000., 0., EHOLResource::Wood, 0));       // 2: wood A, used up
	Sources.Add(MakeSource(2500., 0., EHOLResource::Wood, 100));     // 3: wood B, 15 m beyond the stump 2
	Sources.Add(MakeSource(3000., 0., EHOLResource::Wood, 100));     // 4: wood B
	Sources.Add(MakeSource(-2600., 0., EHOLResource::Wood, 100));    // 5: wood C, 26 m from tree 0: out of range
	Sources.Add(MakeSource(500., 1200., EHOLResource::Food, 200));   // 6: berries 12 m from tree 1: another resource
	Sources.Add(MakeSource(500., -1900., EHOLResource::Wood, 100));  // 7: wood D, 19 m from tree 1, nearer the deposit
	Sources.Add(MakeSource(0., 10000., EHOLResource::Stone, 1000));  // 8: a vein
	Sources.Add(MakeSource(6000., 10000., EHOLResource::Stone, 1000)); // 9: another vein, 60 m away
	FHOLSourceMassifs Massifs;
	Massifs.Build(Sources, HOLGather::NoRoomSourceDistance, HOLGather::NearbyMassifDistance);
	TestEqual(TEXT("one massif for wood A"), Massifs.GetMassif(2), Massifs.GetMassif(0));
	TestNotEqual(TEXT("wood B a massif of its own"), Massifs.GetMassif(3), Massifs.GetMassif(0));
	TestEqual(TEXT("wood A: two nearby groves (B, D)"), Massifs.GetNearbyMassifs(Massifs.GetMassif(0)).Num(), 2);
	TestEqual(TEXT("a vein: no nearby grove"), Massifs.GetNearbyMassifs(Massifs.GetMassif(8)).Num(), 0);

	// Wood A full: its own trees first, then those of B and D, never C (out of range) nor the berries (another resource).
	TArray<int32> Tried;
	TestEqual(TEXT("everything full: none"), Massifs.Search(0, [&Tried](const int32 Index) { Tried.Add(Index); return false; }), int32(INDEX_NONE));
	TestEqual(TEXT("its massif, then the groves nearest the tree clicked"), JoinIndices(Tried), FString(TEXT("1 2 7 3 4")));
	TestFalse(TEXT("wood C, 26 m away: never"), Tried.Contains(5));
	TestFalse(TEXT("berries 12 m away: never"), Tried.Contains(6));
	const auto NotA = [&Massifs](const int32 Index) { return Massifs.GetMassif(Index) != Massifs.GetMassif(0); };
	TestEqual(TEXT("wood A full: the first tree of a nearby grove"), Massifs.Search(0, NotA), 7);

	// Toward a deposit south of wood A (the same order as in the massif): D, on the deposit's side, before B.
	const FHOLGatherDeposit South{ FVector2D(500., -4000.), FVector2D(750., 750.), 0.f };
	Tried.Reset();
	Massifs.Search(0, [&Tried](const int32 Index) { Tried.Add(Index); return false; }, &South);
	TestTrue(*FString::Printf(TEXT("toward the deposit: grove D first (%s)"), *JoinIndices(Tried)), Tried.IndexOfByKey(7) < Tried.IndexOfByKey(3));
	// A deposit east of wood B: B first.
	const FHOLGatherDeposit East{ FVector2D(5000., 0.), FVector2D(750., 750.), 0.f };
	TestEqual(TEXT("toward a deposit beyond grove B: B first"), Massifs.Search(0, NotA, &East), 4);

	// Within the range, not beyond; a vein never leads to another (60 m), nor without nearby groves (range 0).
	TestEqual(TEXT("a vein never leads to another"), Massifs.Search(8, [](int32) { return true; }), int32(INDEX_NONE));
	FHOLSourceMassifs Short;
	Short.Build(Sources, HOLGather::NoRoomSourceDistance, 1400.);
	TestEqual(TEXT("14 m: grove B (15 m) out of range"), Short.Search(0, NotA), int32(INDEX_NONE));
	FHOLSourceMassifs None;
	None.Build(Sources, HOLGather::NoRoomSourceDistance);
	TestEqual(TEXT("no range: no nearby grove"), None.Search(0, NotA), int32(INDEX_NONE));
	FHOLSourceMassifs Long;
	Long.Build(Sources, HOLGather::NoRoomSourceDistance, 2700.);
	Tried.Reset();
	Long.Search(0, [&Tried](const int32 Index) { Tried.Add(Index); return false; });
	TestTrue(TEXT("27 m: wood C in range"), Tried.Contains(5));
	TestFalse(TEXT("27 m: still no berries"), Tried.Contains(6));
	// From grove B: wood A nearby, but not D (40 m from B), nor through A.
	Tried.Reset();
	Massifs.Search(3, [&Tried](const int32 Index) { Tried.Add(Index); return false; });
	TestEqual(TEXT("from grove B: its tree, then wood A only"), JoinIndices(Tried), FString(TEXT("4 2 1 0")));

	// No grove with room: rows without limit in front of the tree (300 idle gatherers, far more than 6 rows hold), each on
	// a point of its own, two bodies and a gap apart, clear of the spots of the tree and of the one beside it.
	const FHOLGatherSite Tree{ FVector2D(5000., 5000.), 60.f };
	constexpr double SpotOffset = 75.; // body radius 60 + gap 15
	constexpr double StandSpacing = 140.;
	const double SpotDistance = Tree.Radius + SpotOffset;
	const FHOLGatherSite Sites[] = { Tree, FHOLGatherSite{ Tree.Position + FVector2D(220., 0.), 60.f } };
	FHOLSpotClaims Claims;
	const auto NeverStale = [](const FHOLSpotClaims::FClaim&) { return false; };
	TArray<FVector2D> Stood;
	TArray<FVector2D> Points;
	constexpr int32 RowsAtATime = 6;
	int32 FarthestRow = 0;
	for (uint64 Idle = 1; Idle <= 300; ++Idle)
	{
		bool bFound = false;
		for (int32 FirstRow = 0; FirstRow < 64 && !bFound; FirstRow += RowsAtATime)
		{
			HOLGather::GetStandPoints(Tree.Position, SpotDistance, StandSpacing, FVector2D(0., -1.), RowsAtATime, Points, FirstRow);
			for (const FVector2D& Point : Points)
			{
				if (HOLGather::IsClearOfSpots(Point, Sites, SpotOffset, StandSpacing) && !Claims.IsTaken(Point, StandSpacing, Idle, NeverStale))
				{
					Claims.Add(Idle, INDEX_NONE, Point);
					Stood.Add(Point);
					FarthestRow = FMath::Max(FarthestRow, FirstRow);
					bFound = true;
					break;
				}
			}
		}
	}
	TestEqual(TEXT("300 idle gatherers, 300 points"), Stood.Num(), 300);
	TestTrue(*FString::Printf(TEXT("beyond the first 6 rows (rows from %d)"), FarthestRow), FarthestRow >= RowsAtATime);
	double Closest = TNumericLimits<double>::Max();
	for (int32 A = 0; A < Stood.Num(); ++A)
	{
		for (int32 B = A + 1; B < Stood.Num(); ++B)
		{
			Closest = FMath::Min(Closest, FVector2D::Distance(Stood[A], Stood[B]));
		}
	}
	TestTrue(*FString::Printf(TEXT("no two idle gatherers closer than 140 cm (%.0f cm)"), Closest), Closest >= StandSpacing - 0.01);
	// The rows go on as asked: row 10 lies 11 spacings beyond the spots.
	HOLGather::GetStandPoints(Tree.Position, SpotDistance, StandSpacing, FVector2D(0., -1.), 1, Points, 10);
	if (TestTrue(TEXT("row 10"), Points.Num() > 0))
	{
		TestEqual(TEXT("row 10, 11 spacings out"), FVector2D::Distance(Points[0], Tree.Position), SpotDistance + 11. * (StandSpacing + 1.), 0.01);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGatherSpotsTest, "HeroesOfLiberty.Gather.Spots", UE::HeroesOfLiberty::Tests::Gather::Flags)
bool FHOLGatherSpotsTest::RunTest(const FString& Parameters)
{
	// Two Peasants (60 cm bodies) at their spots stand the spacing apart at least, at one source or two side by side (D309).
	// Checked with 70 cm, an earlier value passed explicitly: the checks hold whatever hol.Gather.SpotSpacing is.
	constexpr double Spacing = 70.;
	TSet<uint64> Stale;
	int32 StaleAsked = 0;
	const auto IsStale = [&Stale, &StaleAsked](const FHOLSpotClaims::FClaim& Claim)
	{
		++StaleAsked;
		return Stale.Contains(Claim.Owner);
	};
	FHOLSpotClaims Claims;
	TestFalse(TEXT("nothing claimed"), Claims.IsTaken(FVector2D(135., 0.), Spacing, 1, IsStale));

	// Tree 0 at the origin: gatherer 1 on its spot facing +X.
	Claims.Add(1, 0, FVector2D(135., 0.));
	TestTrue(TEXT("same source, 50 cm away: taken"), Claims.IsTaken(FVector2D(135., 50.), Spacing, 2, IsStale));
	TestTrue(TEXT("on the same point: taken"), Claims.IsTaken(FVector2D(135., 0.), Spacing, 2, IsStale));
	TestFalse(TEXT("same source, 75 cm away: free"), Claims.IsTaken(FVector2D(135., 75.), Spacing, 2, IsStale));
	// Tree 1, 2.2 m along X: its spot at 165 degrees is 57 cm from that gatherer; another source, taken all the same.
	const FVector2D NeighbourSpot = FVector2D(220., 0.) + 135. * FVector2D(FMath::Cos(FMath::DegreesToRadians(165.)), FMath::Sin(FMath::DegreesToRadians(165.)));
	TestTrue(TEXT("neighbour source: taken"), Claims.IsTaken(NeighbourSpot, Spacing, 2, IsStale));

	// Across the grid's cell borders, negative coordinates too.
	Claims.Add(3, 2, FVector2D(199., 900.));
	TestTrue(TEXT("across a cell border"), Claims.IsTaken(FVector2D(201., 900.), Spacing, 2, IsStale));
	Claims.Add(4, 2, FVector2D(-1., -501.));
	TestTrue(TEXT("across the axes"), Claims.IsTaken(FVector2D(1., -499.), Spacing, 2, IsStale));
	TestTrue(TEXT("a spacing wider than a cell"), Claims.IsTaken(FVector2D(-420., -501.), 450., 2, IsStale));

	// Its own old spot is no obstacle: left as it looks for a new one.
	TestEqual(TEXT("three claims"), Claims.Num(), 3);
	TestFalse(TEXT("its own spot: free"), Claims.IsTaken(FVector2D(135., 0.), Spacing, 1, IsStale));
	TestEqual(TEXT("its old claim dropped"), Claims.Num(), 2);

	// A stale claim (its gatherer went elsewhere) is dropped once met; a far one is not even looked at.
	Claims.Add(5, 3, FVector2D(2000., 0.));
	Stale.Add(5);
	StaleAsked = 0;
	TestFalse(TEXT("150 cm from a stale claim: free"), Claims.IsTaken(FVector2D(2150., 0.), Spacing, 2, IsStale));
	TestEqual(TEXT("far claims: staleness not asked"), StaleAsked, 0);
	TestFalse(TEXT("on a stale claim: free"), Claims.IsTaken(FVector2D(2000., 10.), Spacing, 2, IsStale));
	TestEqual(TEXT("the stale claim dropped"), Claims.Num(), 2);

	// The spots tried around a source: as many as its circle holds at the spacing, the side facing the deposit first.
	const FHOLGatherSite Tree{ FVector2D(5000., 5000.), 60.f };
	constexpr double SpotOffset = 75.; // body radius 60 + gap 15
	const double SpotDistance = Tree.Radius + SpotOffset;
	const FVector2D Deposit = Tree.Position + FVector2D(0., 2000.);
	TArray<FVector2D> Ring;
	HOLGather::GetSourceSpots(Tree.Position, SpotDistance, Spacing, Deposit - Tree.Position, Ring);
	TestEqual(TEXT("11 spots around a tree at 70 cm"), Ring.Num(), 11);
	if (Ring.Num() == 11)
	{
		TestTrue(TEXT("the first one faces the deposit"), Ring[0].Equals(Tree.Position + FVector2D(0., SpotDistance), 0.01));
		double ClosestSpots = TNumericLimits<double>::Max();
		bool bNearestDepositFirst = true;
		for (int32 A = 0; A < Ring.Num(); ++A)
		{
			bNearestDepositFirst &= A == 0 || FVector2D::Distance(Ring[A], Deposit) >= FVector2D::Distance(Ring[A - 1], Deposit) - 0.01;
			for (int32 B = A + 1; B < Ring.Num(); ++B)
			{
				ClosestSpots = FMath::Min(ClosestSpots, FVector2D::Distance(Ring[A], Ring[B]));
			}
		}
		TestTrue(*FString::Printf(TEXT("no two spots closer than the spacing (%.1f cm)"), ClosestSpots), ClosestSpots >= Spacing);
		TestTrue(TEXT("nearest the deposit first"), bNearestDepositFirst);
	}
	HOLGather::GetSourceSpots(Tree.Position, SpotDistance, 140., FVector2D(1., 0.), Ring);
	TestEqual(TEXT("5 around a tree at 140 cm (before)"), Ring.Num(), 5);
	HOLGather::GetSourceSpots(Tree.Position, 110. + SpotOffset, Spacing, FVector2D(1., 0.), Ring);
	TestEqual(TEXT("16 around a bush"), Ring.Num(), 16);
	HOLGather::GetSourceSpots(Tree.Position, 300. + SpotOffset, Spacing, FVector2D(1., 0.), Ring);
	TestEqual(TEXT("33 around a vein"), Ring.Num(), 33);

	// Idle gatherers in front of a full tree (D309, the user's 3rd trial of J5b.2): rows beyond its spots, facing them,
	// at their own spacing (two bodies and a gap: they never overlap, nor stand on a spot, however close the spots).
	constexpr double StandSpacing = 140.;
	TArray<FVector2D> Points;
	HOLGather::GetStandPoints(Tree.Position, SpotDistance, StandSpacing, FVector2D(-1., 0.), 3, Points);
	if (TestTrue(TEXT("stand points"), Points.Num() > 6))
	{
		TestTrue(TEXT("the first one faces the gatherer, one spacing beyond the spots"),
			Points[0].Equals(Tree.Position + FVector2D(-(SpotDistance + StandSpacing + 1.), 0.), 0.01));
		bool bFacing = true;
		bool bOffSpots = true;
		for (const FVector2D& Point : Points)
		{
			bFacing &= Point.X <= Tree.Position.X + 0.01;
			bOffSpots &= FVector2D::Distance(Point, Tree.Position) >= SpotDistance + StandSpacing - 0.01;
		}
		TestTrue(TEXT("on the half facing it"), bFacing);
		TestTrue(TEXT("a spacing beyond the spots"), bOffSpots);
		TestTrue(TEXT("two points of a row a spacing apart"), FVector2D::Distance(Points[0], Points[1]) >= StandSpacing - 0.01);
	}
	// A tree beside it: its spots are not stood on either.
	const FHOLGatherSite Sites[] = { Tree, FHOLGatherSite{ Tree.Position + FVector2D(-300., 0.), 60.f } };
	TestTrue(TEXT("its own spots only: clear"), HOLGather::IsClearOfSpots(Points[0], MakeArrayView(Sites, 1), SpotOffset, StandSpacing));
	TestFalse(TEXT("on the spots of the tree beside it"), HOLGather::IsClearOfSpots(Points[0], Sites, SpotOffset, StandSpacing));
	TestFalse(TEXT("on its own spots"), HOLGather::IsClearOfSpots(Tree.Position + FVector2D(0., SpotDistance), Sites, SpotOffset, StandSpacing));

	// Ten idle gatherers, three gatherers at their spots: each takes the first point free, none closer than the spacing.
	FHOLSpotClaims Front;
	const auto NeverStale = [](const FHOLSpotClaims::FClaim&) { return false; };
	for (uint64 Gatherer = 1; Gatherer <= 3; ++Gatherer)
	{
		const double Angle = UE_DOUBLE_PI + (static_cast<double>(Gatherer) - 2.) * 0.8;
		Front.Add(Gatherer, 7, Tree.Position + SpotDistance * FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)));
	}
	TArray<FVector2D> Stood;
	for (uint64 Idle = 10; Idle < 20; ++Idle)
	{
		for (const FVector2D& Point : Points)
		{
			if (HOLGather::IsClearOfSpots(Point, MakeArrayView(Sites, 1), SpotOffset, StandSpacing) && !Front.IsTaken(Point, StandSpacing, Idle, NeverStale))
			{
				Front.Add(Idle, INDEX_NONE, Point);
				Stood.Add(Point);
				break;
			}
		}
	}
	TestEqual(TEXT("every idle gatherer has a point"), Stood.Num(), 10);
	double Closest = TNumericLimits<double>::Max();
	for (int32 A = 0; A < Stood.Num(); ++A)
	{
		for (int32 B = A + 1; B < Stood.Num(); ++B)
		{
			Closest = FMath::Min(Closest, FVector2D::Distance(Stood[A], Stood[B]));
		}
	}
	TestTrue(*FString::Printf(TEXT("idle gatherers a spacing apart (%.0f cm)"), Closest), Closest >= StandSpacing - 0.01);
	TestFalse(TEXT("a gatherer's spot behind the tree is still free"), Front.IsTaken(Tree.Position + FVector2D(SpotDistance, 0.), Spacing, 30, NeverStale));
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
			// The map's sources keep their behaviour (IA1.2): places all around, own rate, used up, placed by the map.
			TestEqual(*FString::Printf(TEXT("%s: places all around"), *Name), Row->Places, 0);
			TestEqual(*FString::Printf(TEXT("%s: rate scale 1"), *Name), Row->RateScale, 1.f);
			TestFalse(*FString::Printf(TEXT("%s: used up"), *Name), Row->Endless);
			TestFalse(*FString::Printf(TEXT("%s: not linked"), *Name), Row->Linked);
		}
	}

	// The farm's field (IA1.2): food, one place, endless, linked to its building, 0.55/s for the Peasant (his 0.7/s
	// through the row's GatherRate, the one rate function of the gathering, F3).
	const FHOLSourceTypeRow* Field = UHOLSourceSettings::FindType(HOLSourceTypeNames::FarmField);
	if (TestNotNull(TEXT("FarmField in the source type table"), Field))
	{
		TestEqual(TEXT("FarmField: resource"), int32(Field->Resource), int32(EHOLResource::Food));
		TestTrue(TEXT("FarmField: a body"), Field->Radius > 0.f);
		TestEqual(TEXT("FarmField: one place"), Field->Places, 1);
		TestTrue(TEXT("FarmField: endless"), Field->Endless);
		TestTrue(TEXT("FarmField: linked"), Field->Linked);
		const UDataTable* Units = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
		const FHOLUnitTypeRow* Farmer = Units ? Units->FindRow<FHOLUnitTypeRow>(TEXT("Peasant"), TEXT("Gather test")) : nullptr;
		if (TestNotNull(TEXT("Peasant for the farm rate"), Farmer))
		{
			const FHOLUnitTypeConstants Constants = Farmer->ToConstants(TEXT("Peasant"));
			TestEqual(TEXT("FarmField: the Peasant gathers 0.55/s, exactly"),
				Field->GatherRate(Constants.GatherRates[EHOLResource::Food]), 0.55f, 0.f);
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


// IA1.2 F2: a farm field on the real source registry, in a test world. Added by code with its owner, it has no shape and
// no hole in the navmesh (its farm stands for it), the map never places one, gathering never counts it down, and
// RemoveSource takes it off the map (its index kept, skipped like a used-up source). The map's tree is untouched.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGatherLinkedSourceTest, "HeroesOfLiberty.Gather.LinkedSource", UE::HeroesOfLiberty::Tests::Gather::Flags)
bool FHOLGatherLinkedSourceTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLSourceSubsystem* Registry = World->GetSubsystem<UHOLSourceSubsystem>();
	AHOLSourceLayout* Layout = World->SpawnActor<AHOLSourceLayout>();
	const FArrayProperty* LayoutSources = FindFProperty<FArrayProperty>(AHOLSourceLayout::StaticClass(), TEXT("Sources"));
	if (TestNotNull(TEXT("source registry"), Registry) && TestNotNull(TEXT("map layout"), Layout)
		&& TestNotNull(TEXT("the layout's source list"), LayoutSources))
	{
		// The map asks for a tree and a farm field: only the tree is placed.
		TArray<FHOLPlacedSource>& Placed = *LayoutSources->ContainerPtrToValuePtr<TArray<FHOLPlacedSource>>(Layout);
		Placed.Add(FHOLPlacedSource{ HOLSourceTypeNames::Tree, FVector(0.0, 0.0, 0.0) });
		Placed.Add(FHOLPlacedSource{ HOLSourceTypeNames::FarmField, FVector(600.0, 0.0, 0.0) });
		Registry->OnWorldBeginPlay(*World);
		TestEqual(TEXT("the map placed its tree, not its farm field"), Registry->GetSources().Num(), 1);
		const int32 Tree = 0;
		const int32 TreeAmount = Registry->GetSources().IsValidIndex(Tree) ? Registry->GetSources()[Tree].Remaining : 0;
		TestTrue(TEXT("the map's tree belongs to no one"), Registry->GetSources().IsValidIndex(Tree)
			&& Registry->GetSources()[Tree].OwnerSlot == FHOLSource::NoOwner);

		const int32 Farm = Registry->AddSource(HOLSourceTypeNames::FarmField, FVector(2000.0, 0.0, 0.0), INDEX_NONE, 2);
		if (TestTrue(TEXT("a farm field added by code"), Registry->GetSources().IsValidIndex(Farm)))
		{
			const FHOLSource& Field = Registry->GetSources()[Farm];
			TestEqual(TEXT("the farm field's owner"), static_cast<int32>(Field.OwnerSlot), 2);
			TestTrue(TEXT("the farm field stands"), Field.IsStanding());
			TestEqual(TEXT("two standing"), Registry->GetNumStanding(), 2);
			const FString Report = Registry->MakeReport();
			AddInfo(Report);
			TestTrue(TEXT("the farm field has no shape and no hole: shown 1, navmesh 1 (the tree alone)"),
				Report.EndsWith(TEXT("; shown 1, navmesh 1")));
			TestTrue(TEXT("the report says it is endless"), Report.Contains(TEXT("FarmField 1: endless")));
			TestEqual(TEXT("the food nearest the field: the field"), Registry->FindNearestStanding(FVector(2000.0, 0.0, 0.0), EHOLResource::Food), Farm);

			// Gathering takes from it many times: it gives all that is asked and never goes down.
			const int32 Before = Field.Remaining;
			int32 Taken = 0;
			for (int32 Load = 0; Load < 100; ++Load)
			{
				Taken += Registry->Take(Farm, 10);
			}
			TestEqual(TEXT("100 loads of 10 taken from the endless field"), Taken, 1000);
			TestEqual(TEXT("its remaining unchanged"), Registry->GetSources()[Farm].Remaining, Before);
			TestTrue(TEXT("still standing"), Registry->GetSources()[Farm].IsStanding());
			TestEqual(TEXT("a huge take given whole, not counted down"), Registry->Take(Farm, MAX_int32), MAX_int32);
			TestTrue(TEXT("still standing after it"), Registry->GetSources()[Farm].IsStanding());

			// The farm dies: its field leaves the map.
			TestTrue(TEXT("the field removed"), Registry->RemoveSource(Farm));
			TestFalse(TEXT("no longer standing"), Registry->GetSources()[Farm].IsStanding());
			TestEqual(TEXT("its index kept"), Registry->GetSources().Num(), 2);
			TestEqual(TEXT("one standing: the tree"), Registry->GetNumStanding(), 1);
			TestEqual(TEXT("no food left near it"), Registry->FindNearestStanding(FVector(2000.0, 0.0, 0.0), EHOLResource::Food), INDEX_NONE);
			TestEqual(TEXT("the nearest source: the tree"), Registry->FindNearestStanding(FVector(2000.0, 0.0, 0.0)), Tree);
			TestEqual(TEXT("nothing taken from a removed field"), Registry->Take(Farm, 10), 0);
			TestFalse(TEXT("removed twice: no"), Registry->RemoveSource(Farm));
			const FString After = Registry->MakeReport();
			AddInfo(After);
			TestTrue(TEXT("the report: one standing of two, the tree still shown"), After.Contains(TEXT(": 1 standing of 2 ("))
				&& After.EndsWith(TEXT("; shown 1, navmesh 1")) && !After.Contains(TEXT("FarmField")));
			TestTrue(TEXT("the map's tree untouched"), Registry->GetSources()[Tree].IsStanding()
				&& Registry->GetSources()[Tree].Remaining == TreeAmount && TreeAmount > 0);
		}
		TestFalse(TEXT("nothing removed at a bad index"), Registry->RemoveSource(99));
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// IA1.2 F3: farm fields on the real chain, as a match's server: a test world, the order queue (HOLPlayerOrders, what the
// controller's RPCs and the AI call), the order processor then the gathering at the fixed step (no movement runs: the
// Peasants are set down touching their sources).
namespace UE::HeroesOfLiberty::Tests::GatherFarm
{
	const FName Peasant(TEXT("Peasant"));

	/** Steps the gatherers work (10 s at 20 Hz): a Peasant's load of 10 is not full yet, at a field nor at the berries. */
	constexpr int32 Steps = 200;

	/** The test world: a game state (slot 0 human, slot 1 AI), the subsystems, slot 0's mill and the step's processors. */
	struct FFarmWorld
	{
		UWorld* World = nullptr;
		UHOLUnitSubsystem* Units = nullptr;
		UHOLSourceSubsystem* Sources = nullptr;
		TSharedPtr<FMassEntityManager> EntityManager;
		UHOLGatherProcessor* Gather = nullptr;
		UHOLUnitOrderProcessor* Orders = nullptr;
		AHOLBuilding* Mill = nullptr;

		FFarmWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			AHOLGameState* State = World->SpawnActor<AHOLGameState>();
			Units = World->GetSubsystem<UHOLUnitSubsystem>();
			Sources = World->GetSubsystem<UHOLSourceSubsystem>();
			UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
			UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
			if (!State || !Units || !Sources || !Simulation || !EntitySubsystem)
			{
				return;
			}
			FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
			for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
			{
				Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : (Index == 1 ? EHOLSlotState::AI : EHOLSlotState::Closed);
			}
			State->InitVictory(Config);
			Sources->OnWorldBeginPlay(*World);
			// Slot 0's deposit of food, finished: its farms' spots face it.
			Mill = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
			if (Mill)
			{
				Mill->Setup(HOLBuildingTypeNames::Mill, 0, 0);
			}
			EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
			Gather = NewObject<UHOLGatherProcessor>(Simulation);
			Gather->CallInitialize(Simulation, EntityManager.ToSharedRef());
			Orders = NewObject<UHOLUnitOrderProcessor>(Simulation);
			Orders->CallInitialize(Simulation, EntityManager.ToSharedRef());
		}

		~FFarmWorld()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		bool IsReady() const { return Gather && Orders && Mill; }

		/** One fixed step: the order processor, then the gathering (the pipeline's order). */
		void RunStep()
		{
			const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
			{
				FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Orders, ProcessingContext);
			}
			{
				FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Gather, ProcessingContext);
			}
		}

		/** A Peasant born for Owner (far from every source); its identifier, 0 if none was born. */
		uint32 PlacePeasant(const int32 Owner)
		{
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = Peasant;
			Request.Count = 1;
			Request.OwnerPlayer = static_cast<uint8>(Owner);
			Request.Center = FVector(-20000.0, -20000.0, 0.0);
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			TArray<uint32> After;
			Units->GetAllUnitIds(After);
			After.RemoveAll([&Before](const uint32 Id) { return Before.Contains(Id); });
			return After.Num() == 1 ? After[0] : 0;
		}

		/** Sets the unit down at Location (its height kept). */
		void SetDown(const uint32 UnitId, const FVector2D& Location)
		{
			const FMassEntityHandle Entity = Units->FindEntity(UnitId);
			if (FTransformFragment* Transform = EntityManager->IsEntityValid(Entity) ? EntityManager->GetFragmentDataPtr<FTransformFragment>(Entity) : nullptr)
			{
				const double Z = Transform->GetTransform().GetLocation().Z;
				Transform->GetMutableTransform().SetLocation(FVector(Location, Z));
			}
		}

		const FHOLGatherFragment* GetGather(const uint32 UnitId) const
		{
			const FMassEntityHandle Entity = Units->FindEntity(UnitId);
			return EntityManager->IsEntityValid(Entity) ? EntityManager->GetFragmentDataPtr<FHOLGatherFragment>(Entity) : nullptr;
		}

		const FHOLUnitTypeConstants* GetConstants(const uint32 UnitId) const
		{
			const FMassEntityHandle Entity = Units->FindEntity(UnitId);
			return EntityManager->IsEntityValid(Entity) ? &EntityManager->GetConstSharedFragmentDataChecked<FHOLUnitTypeConstants>(Entity) : nullptr;
		}

		/** The unit gathers at SourceIndex now (touching it, taking from it). */
		bool IsGatheringAt(const uint32 UnitId, const int32 SourceIndex) const
		{
			const FHOLGatherFragment* Fragment = GetGather(UnitId);
			return Fragment && Fragment->Phase == EHOLGatherPhase::Gathering && Fragment->SourceIndex == SourceIndex;
		}

		/** Food the unit gathered: whole units carried and the share of the next one. */
		double GetFoodGathered(const uint32 UnitId) const
		{
			const FHOLGatherFragment* Fragment = GetGather(UnitId);
			if (!Fragment)
			{
				return 0.0;
			}
			const double Whole = Fragment->Load.Resource == EHOLResource::Food ? Fragment->Load.Amount : 0.0;
			const double Share = Fragment->Load.ProgressResource == EHOLResource::Food ? Fragment->Load.Progress : 0.0;
			return Whole + Share;
		}
	};
}

// (a) A Peasant of slot 0 sent to its own farm field, by its mill, gathers 0.55 food a second: the field's RateScale times
// its own rate, both read from the real tables; the endless field never sends it away. (b) A second Peasant sent with it
// never works the field at the same time (one place): it goes to the free farm 6 m away (its massif); at a farm alone, it
// stands idle. (c) A Peasant of slot 1 never gathers slot 0's field: its order to it is refused, and the berries of the
// map it used up do not send it to slot 0's farm beside them. (d) The map's berries keep the Peasant's own rate (0.7).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGatherFarmFieldTest, "HeroesOfLiberty.Gather.FarmField", UE::HeroesOfLiberty::Tests::Gather::Flags)
bool FHOLGatherFarmFieldTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::GatherFarm;
	FFarmWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	UWorld& World = *Test.World;
	UHOLSourceSubsystem& Registry = *Test.Sources;
	// Every group 70 m or more from the others (beyond a nearby grove's 20 m, D309 point 6).
	const int32 Farm = Registry.AddSource(HOLSourceTypeNames::FarmField, FVector(1000.0, 0.0, 0.0), INDEX_NONE, 0);
	const int32 NextFarm = Registry.AddSource(HOLSourceTypeNames::FarmField, FVector(1000.0, 600.0, 0.0), INDEX_NONE, 0);
	const int32 LoneFarm = Registry.AddSource(HOLSourceTypeNames::FarmField, FVector(1000.0, 8000.0, 0.0), INDEX_NONE, 0);
	const int32 GuardedFarm = Registry.AddSource(HOLSourceTypeNames::FarmField, FVector(1000.0, -8000.0, 0.0), INDEX_NONE, 0);
	const int32 LastBerries = Registry.AddSource(HOLSourceTypeNames::BerryBush, FVector(-8000.0, 0.0, 0.0), 1);
	const int32 FarmByBerries = Registry.AddSource(HOLSourceTypeNames::FarmField, FVector(-8000.0, 600.0, 0.0), INDEX_NONE, 0);
	const int32 Berries = Registry.AddSource(HOLSourceTypeNames::BerryBush, FVector(8000.0, 0.0, 0.0));
	const TArray<FHOLSource>& Sources = Registry.GetSources();
	if (!TestTrue(TEXT("five farm fields of slot 0 and two bushes of the map"), Sources.Num() == 7 && Farm != INDEX_NONE && NextFarm != INDEX_NONE
		&& LoneFarm != INDEX_NONE && GuardedFarm != INDEX_NONE && LastBerries != INDEX_NONE && FarmByBerries != INDEX_NONE && Berries != INDEX_NONE))
	{
		return false;
	}
	const FHOLSourceTypeRow* FieldRow = Registry.GetType(Sources[Farm].TypeIndex);
	const FHOLSourceTypeRow* BushRow = Registry.GetType(Sources[Berries].TypeIndex);
	if (!TestNotNull(TEXT("the farm field's row"), FieldRow) || !TestNotNull(TEXT("the berry bush's row"), BushRow))
	{
		return false;
	}

	// Slot 0: two Peasants for the farm by the mill, two for the lone farm, one for the berries; slot 1: one sent to slot
	// 0's farm, one to the last berries.
	const uint32 First = Test.PlacePeasant(0);
	const uint32 Second = Test.PlacePeasant(0);
	const uint32 LoneFirst = Test.PlacePeasant(0);
	const uint32 LoneSecond = Test.PlacePeasant(0);
	const uint32 Picker = Test.PlacePeasant(0);
	const uint32 Intruder = Test.PlacePeasant(1);
	const uint32 Passer = Test.PlacePeasant(1);
	const FHOLUnitTypeConstants* Constants = Test.GetConstants(First);
	if (!TestTrue(TEXT("seven Peasants born"), First && Second && LoneFirst && LoneSecond && Picker && Intruder && Passer)
		|| !TestNotNull(TEXT("the Peasant's constants"), Constants))
	{
		return false;
	}
	const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
	const float UnitRate = Constants->GatherRates[EHOLResource::Food];
	const float FieldRate = FieldRow->GatherRate(UnitRate);
	const float BushRate = BushRow->GatherRate(UnitRate);
	TestTrue(TEXT("the field gives less than the Peasant's own rate (RateScale of the table)"), UnitRate > 0.f && FieldRate < UnitRate);
	TestEqual(TEXT("the berries give the Peasant's own rate"), BushRate, UnitRate);
	TestEqual(TEXT("one place at a field (the table)"), FieldRow->Places, 1);

	// Each one touching its source (within the contact gap, D296): no walk needed, none is run.
	const double BodyRadius = Constants->Radius;
	const auto Touching = [&Sources, BodyRadius](const int32 SourceIndex, const FVector2D& Direction)
	{
		return FVector2D(Sources[SourceIndex].Location) + Direction * (Sources[SourceIndex].Radius + BodyRadius + 10.0);
	};
	Test.SetDown(First, Touching(Farm, FVector2D(-1.0, 0.0)));         // between the field and the mill
	Test.SetDown(Second, Touching(Farm, FVector2D(1.0, 0.0)));         // the other side, 3.4 m from the first
	Test.SetDown(LoneFirst, Touching(LoneFarm, FVector2D(-1.0, 0.0)));
	Test.SetDown(LoneSecond, Touching(LoneFarm, FVector2D(1.0, 0.0)));
	Test.SetDown(Picker, Touching(Berries, FVector2D(-1.0, 0.0)));
	Test.SetDown(Intruder, Touching(GuardedFarm, FVector2D(-1.0, 0.0)));
	Test.SetDown(Passer, Touching(LastBerries, FVector2D(0.0, -1.0))); // the side away from slot 0's farm

	TestTrue(TEXT("slot 0's order to its farm"), HOLPlayerOrders::Gather(World, 0, TArray<uint32>({ First, Second }), Farm));
	TestTrue(TEXT("slot 0's order to its lone farm"), HOLPlayerOrders::Gather(World, 0, TArray<uint32>({ LoneFirst, LoneSecond }), LoneFarm));
	TestTrue(TEXT("slot 0's order to the berries"), HOLPlayerOrders::Gather(World, 0, TArray<uint32>({ Picker }), Berries));
	TestTrue(TEXT("slot 1's order to slot 0's farm enters the queue (the gathering decides)"),
		HOLPlayerOrders::Gather(World, 1, TArray<uint32>({ Intruder }), GuardedFarm));
	TestTrue(TEXT("slot 1's order to the last berries"), HOLPlayerOrders::Gather(World, 1, TArray<uint32>({ Passer }), LastBerries));

	// The first step: the orders applied, each one touching its source starts at once, or looks elsewhere.
	Test.RunStep();
	const bool bFirstHolds = Test.IsGatheringAt(First, Farm);
	const uint32 Holder = bFirstHolds ? First : Second;
	const uint32 Other = bFirstHolds ? Second : First;
	TestTrue(TEXT("(a) one Peasant gathers the farm by the mill"), Test.IsGatheringAt(Holder, Farm));
	TestFalse(TEXT("(b) the other does not gather it with him"), Test.IsGatheringAt(Other, Farm));
	const FHOLGatherFragment* OtherGather = Test.GetGather(Other);
	TestTrue(TEXT("(b) the other goes to the free farm of its massif, 6 m away"), OtherGather && OtherGather->IsGathering()
		&& OtherGather->SourceIndex == NextFarm);
	const bool bLoneFirstHolds = Test.IsGatheringAt(LoneFirst, LoneFarm);
	const uint32 LoneOther = bLoneFirstHolds ? LoneSecond : LoneFirst;
	TestTrue(TEXT("(b) one Peasant gathers the lone farm"), Test.IsGatheringAt(bLoneFirstHolds ? LoneFirst : LoneSecond, LoneFarm));
	const FHOLGatherFragment* LoneOtherGather = Test.GetGather(LoneOther);
	TestTrue(TEXT("(b) the other one, no free farm near, stands idle"), LoneOtherGather && !LoneOtherGather->IsGathering());
	const FHOLGatherFragment* IntruderGather = Test.GetGather(Intruder);
	TestTrue(TEXT("(c) slot 1's Peasant refused at slot 0's farm: not gathering"), IntruderGather && !IntruderGather->IsGathering());

	// The steps after: at most one at a time on each field; slot 1 never at slot 0's fields.
	int32 MostOnFarm = 0;
	int32 MostOnLoneFarm = 0;
	bool bIntruderGathered = false;
	bool bPasserToFarm = false;
	for (int32 Step = 1; Step < Steps; ++Step)
	{
		Test.RunStep();
		MostOnFarm = FMath::Max(MostOnFarm, (Test.IsGatheringAt(First, Farm) ? 1 : 0) + (Test.IsGatheringAt(Second, Farm) ? 1 : 0));
		MostOnLoneFarm = FMath::Max(MostOnLoneFarm, (Test.IsGatheringAt(LoneFirst, LoneFarm) ? 1 : 0) + (Test.IsGatheringAt(LoneSecond, LoneFarm) ? 1 : 0));
		const FHOLGatherFragment* Intruding = Test.GetGather(Intruder);
		bIntruderGathered |= !Intruding || Intruding->IsGathering() || Test.GetFoodGathered(Intruder) > 0.0;
		const FHOLGatherFragment* Passing = Test.GetGather(Passer);
		bPasserToFarm |= !Passing || Passing->SourceIndex == FarmByBerries;
	}

	// (a) 0.55 a second at the field, give or take the first step (spent starting).
	const double Seconds = Steps * StepSeconds;
	TestTrue(TEXT("(a) the holder still gathers its field (endless: never used up)"), Test.IsGatheringAt(Holder, Farm));
	TestEqual(TEXT("(a) food gathered at the field: the field's rate"), Test.GetFoodGathered(Holder), FieldRate * Seconds, FieldRate * StepSeconds + 1e-3);
	TestTrue(TEXT("(a) the field still stands"), Sources[Farm].IsStanding());
	// (b)
	TestEqual(TEXT("(b) never two at once at the farm by the mill"), MostOnFarm, 1);
	TestEqual(TEXT("(b) never two at once at the lone farm"), MostOnLoneFarm, 1);
	// (c)
	TestFalse(TEXT("(c) slot 1's Peasant never gathered slot 0's farm"), bIntruderGathered);
	TestFalse(TEXT("(c) the last berries used up"), Sources[LastBerries].IsStanding());
	TestEqual(TEXT("(c) slot 1's Peasant took the last berry"), Test.GetFoodGathered(Passer), 1.0, 1e-3);
	TestFalse(TEXT("(c) ... and was never sent to slot 0's farm beside them"), bPasserToFarm);
	// (d)
	TestTrue(TEXT("(d) the berries' Peasant still gathers"), Test.IsGatheringAt(Picker, Berries));
	TestEqual(TEXT("(d) food gathered at the berries: the Peasant's own rate"), Test.GetFoodGathered(Picker), BushRate * Seconds, BushRate * StepSeconds + 1e-3);
	return true;
}

#endif
