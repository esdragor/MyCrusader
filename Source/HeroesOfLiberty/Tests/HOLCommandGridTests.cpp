// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypes.h"
#include "Misc/AutomationTest.h"
#include "Player/HOLCommandGrid.h"

#if WITH_DEV_AUTOMATION_TESTS

// J6a.5 checks (D313): what the command grid does for a selection, what a cell shows, what a press tells the player, the
// F keys (D30, the user's trial of J6a.5): a category's buildings selected together, then the camera from one to the next;
// a group's subgroups (Tab), their merged cells and the even spread of a production (the user's 2nd trial of J6a.5).
namespace UE::HeroesOfLiberty::Tests::CommandGrid
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FName Peasant(TEXT("Peasant"));

	/** The builders' grid: in the panel of age I, its economic row, A house, Z lumber camp, E mining camp (the user's 3rd trial of G.1). */
	FHOLGridContext MakeBuilders()
	{
		FHOLGridContext Context;
		Context.bBuilders = true;
		Context.Buildable = { { HOLBuildingTypeNames::House, 0, 0 }, { HOLBuildingTypeNames::LumberCamp, 1, 0 }, { HOLBuildingTypeNames::MiningCamp, 2, 0 } };
		return Context;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCommandGridCommandsTest, "HeroesOfLiberty.CommandGrid.Commands", UE::HeroesOfLiberty::Tests::CommandGrid::Flags)
bool FHOLCommandGridCommandsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::CommandGrid;

	// Builders selected (the user's trials of G.1): the age panels, panel I A Z E the three buildings (D313); nothing
	// elsewhere, nothing in the grid on the right.
	FHOLGridContext Builders = MakeBuilders();
	const FHOLGridCommand A = HOLCommandGrid::GetAgeBuilding(Builders, 0, 0);
	TestTrue(TEXT("I A: build a house"), A.Command == EHOLGridCommand::Build && A.TypeName == HOLBuildingTypeNames::House && A.Age == 0 && !A.bGreyed);
	TestTrue(TEXT("I Z: build a lumber camp"), HOLCommandGrid::GetAgeBuilding(Builders, 0, 1).TypeName == HOLBuildingTypeNames::LumberCamp);
	TestTrue(TEXT("I E: build a mining camp"), HOLCommandGrid::GetAgeBuilding(Builders, 0, 2).TypeName == HOLBuildingTypeNames::MiningCamp);
	for (int32 Cell = 0; Cell < HOLCommandGrid::NumCells; ++Cell)
	{
		TestTrue(*FString::Printf(TEXT("cell %d of the grid empty for builders"), Cell), HOLCommandGrid::GetCommand(Builders, Cell).IsNone());
		TestTrue(*FString::Printf(TEXT("cell %d of panel I: a building on A Z E only"), Cell), HOLCommandGrid::GetAgeBuilding(Builders, 0, Cell).IsNone() == (Cell > 2));
	}
	TestTrue(TEXT("out of the panels: nothing"), HOLCommandGrid::GetAgeBuilding(Builders, -1, 0).IsNone() && HOLCommandGrid::GetAgeBuilding(Builders, 4, 0).IsNone()
		&& HOLCommandGrid::GetAgeBuilding(Builders, 0, 12).IsNone());

	// Units that do not build (no gatherer among them): nothing.
	FHOLGridContext Soldiers = MakeBuilders();
	Soldiers.bBuilders = false;
	TestTrue(TEXT("no builder: A empty"), HOLCommandGrid::GetCommand(Soldiers, 0).IsNone() && HOLCommandGrid::GetAgeBuilding(Soldiers, 0, 0).IsNone());

	// The player's centre selected: A produces the Peasant (the same key, another meaning: D281).
	FHOLGridContext Centre;
	Centre.bOwnBuilding = true;
	Centre.bBuildingFinished = true;
	Centre.Produces = { Peasant };
	const FHOLGridCommand Produce = HOLCommandGrid::GetCommand(Centre, 0);
	TestTrue(TEXT("centre: A produces a Peasant"), Produce.Command == EHOLGridCommand::Produce && Produce.TypeName == Peasant);
	TestTrue(TEXT("centre: Z empty"), HOLCommandGrid::GetCommand(Centre, 1).IsNone());
	TestTrue(TEXT("centre: no cancel cell"), HOLCommandGrid::GetCommand(Centre, HOLCommandGrid::CancelCell).IsNone());

	// One of the player's sites selected: only « Annuler », on V; it produces nothing.
	FHOLGridContext Site = Centre;
	Site.bBuildingFinished = false;
	TestTrue(TEXT("site: A produces nothing"), HOLCommandGrid::GetCommand(Site, 0).IsNone());
	TestTrue(TEXT("site: V cancels it"), HOLCommandGrid::GetCommand(Site, HOLCommandGrid::CancelCell).Command == EHOLGridCommand::CancelConstruction);
	TestEqual(TEXT("the cancel cell is V, bottom right"), HOLCommandGrid::CancelCell, 11);

	// What a cell shows (the user's trial of J6a.5): its icon, its name while there is none; never its cost (the tooltip's).
	const FHOLGridCellFace NoIcon = HOLCommandGrid::GetCellFace(false);
	TestTrue(TEXT("no icon yet: the name, no cost"), NoIcon.bName && !NoIcon.bIcon && !NoIcon.bCost);
	const FHOLGridCellFace Icon = HOLCommandGrid::GetCellFace(true);
	TestTrue(TEXT("an icon: the icon alone, no cost"), Icon.bIcon && !Icon.bName && !Icon.bCost);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCommandGridFeedbackTest, "HeroesOfLiberty.CommandGrid.Feedback", UE::HeroesOfLiberty::Tests::CommandGrid::Flags)
bool FHOLCommandGridFeedbackTest::RunTest(const FString& Parameters)
{
	// The first resource missing, in the order of the top bar.
	EHOLResource Missing = EHOLResource::Gold;
	TestFalse(TEXT("covered: nothing missing"), HOLCommandGrid::FindMissingResource(FHOLResources(0, 50, 0, 0), FHOLResources(0, 50, 0, 0), Missing));
	TestTrue(TEXT("49 wood for 50: wood missing"), HOLCommandGrid::FindMissingResource(FHOLResources(500, 49, 0, 0), FHOLResources(0, 50, 0, 0), Missing)
		&& Missing == EHOLResource::Wood);
	TestTrue(TEXT("food and gold missing: food first"), HOLCommandGrid::FindMissingResource(FHOLResources(0, 0, 0, 0), FHOLResources(10, 0, 0, 10), Missing)
		&& Missing == EHOLResource::Food);
	TestTrue(TEXT("stone missing"), HOLCommandGrid::FindMissingResource(FHOLResources(9, 9, 0, 9), FHOLResources(0, 0, 1, 0), Missing)
		&& Missing == EHOLResource::Stone);

	// A building: its cost alone (« Pas assez de bois », D313); the command is not sent.
	const FHOLCommandFeedback NoWood = HOLCommandGrid::CheckBuild(FHOLResources(200, 0, 200, 200), FHOLResources(0, 50, 0, 0));
	TestTrue(TEXT("house without wood: not enough wood"), NoWood.Kind == EHOLCommandFeedback::NotEnoughResources && NoWood.Resource == EHOLResource::Wood);
	TestTrue(TEXT("not enough: blocks"), HOLCommandGrid::Blocks(NoWood));
	TestTrue(TEXT("house with 50 wood: goes through"), HOLCommandGrid::CheckBuild(FHOLResources(0, 50, 0, 0), FHOLResources(0, 50, 0, 0)).IsNone());
	TestEqual(TEXT("logged"), HOLCommandGrid::Describe(NoWood), FString(TEXT("not enough Wood")));

	// A unit: queue full first, then the cost, then the population (queued, waits: told, not blocked).
	const FHOLResources Peasant(50, 0, 0, 0);
	TestTrue(TEXT("room, resources: goes through"), HOLCommandGrid::CheckProduction(FHOLResources(50, 0, 0, 0), Peasant, 0, 5, 3, 10, 1).IsNone());
	const FHOLCommandFeedback Full = HOLCommandGrid::CheckProduction(FHOLResources(0, 0, 0, 0), Peasant, 5, 5, 10, 10, 1);
	TestTrue(TEXT("queue full before the cost"), Full.Kind == EHOLCommandFeedback::QueueFull && HOLCommandGrid::Blocks(Full));
	const FHOLCommandFeedback NoFood = HOLCommandGrid::CheckProduction(FHOLResources(49, 0, 0, 0), Peasant, 0, 5, 10, 10, 1);
	TestTrue(TEXT("the cost before the population"), NoFood.Kind == EHOLCommandFeedback::NotEnoughResources && NoFood.Resource == EHOLResource::Food);
	const FHOLCommandFeedback Population = HOLCommandGrid::CheckProduction(FHOLResources(50, 0, 0, 0), Peasant, 0, 5, 10, 10, 1);
	TestTrue(TEXT("10 / 10: population full"), Population.Kind == EHOLCommandFeedback::PopulationFull);
	TestFalse(TEXT("population full: queued all the same (it waits)"), HOLCommandGrid::Blocks(Population));
	TestTrue(TEXT("9 / 10: room for one"), HOLCommandGrid::CheckProduction(FHOLResources(50, 0, 0, 0), Peasant, 0, 5, 9, 10, 1).IsNone());
	TestTrue(TEXT("9 / 10: no room for two places"), HOLCommandGrid::IsPopulationBlocked(9, 10, 2));
	TestFalse(TEXT("9 / 10: room for one place"), HOLCommandGrid::IsPopulationBlocked(9, 10, 1));

	// A red ghost: blocks.
	FHOLCommandFeedback Ghost;
	Ghost.Kind = EHOLCommandFeedback::PlacementRefused;
	Ghost.Refusal = EHOLPlacementRefusal::Building;
	TestTrue(TEXT("red ghost: blocks"), HOLCommandGrid::Blocks(Ghost));
	TestEqual(TEXT("red ghost logged"), HOLCommandGrid::Describe(Ghost), FString(TEXT("placement refused (on a building)")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCommandGridCycleTest, "HeroesOfLiberty.CommandGrid.Cycle", UE::HeroesOfLiberty::Tests::CommandGrid::Flags)
bool FHOLCommandGridCycleTest::RunTest(const FString& Parameters)
{
	// An F key pressed again (D30): the centre first, then the houses, the lumber camps, the mining camps; within a type, by key.
	const TArray<FHOLCycleBuilding> Buildings = {
		{ /*Rank*/ 1, /*Key*/ 40 },  // a house
		{ 0, 90 },                   // the centre, made before by the map but its key higher
		{ 2, 10 },                   // a lumber camp
		{ 1, 20 },                   // another house
	};
	TestEqual(TEXT("first press: the centre"), HOLCommandGrid::NextInCycle(Buildings, 0), 1);
	TestEqual(TEXT("then the house of the lowest key"), HOLCommandGrid::NextInCycle(Buildings, 90), 3);
	TestEqual(TEXT("then the other house"), HOLCommandGrid::NextInCycle(Buildings, 20), 0);
	TestEqual(TEXT("then the lumber camp"), HOLCommandGrid::NextInCycle(Buildings, 40), 2);
	TestEqual(TEXT("after the last one: the centre again"), HOLCommandGrid::NextInCycle(Buildings, 10), 1);
	TestEqual(TEXT("a key gone (building cancelled): the first one"), HOLCommandGrid::NextInCycle(Buildings, 55), 1);
	TestEqual(TEXT("no building: none"), HOLCommandGrid::NextInCycle(TArray<FHOLCycleBuilding>(), 0), INDEX_NONE);
	TestEqual(TEXT("one building: always it"), HOLCommandGrid::NextInCycle({ { 0, 7 } }, 7), 0);

	// The table's types: the centre first, then the builders' grid, A, Z, E.
	const FHOLBuildingTypeRow* MainCentre = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::MainCentre);
	const FHOLBuildingTypeRow* House = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::House);
	const FHOLBuildingTypeRow* MiningCamp = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::MiningCamp);
	if (TestNotNull(TEXT("centre"), MainCentre) && TestNotNull(TEXT("house"), House) && TestNotNull(TEXT("mining camp"), MiningCamp))
	{
		TestTrue(TEXT("ranks: centre < house < mining camp"), HOLCommandGrid::GetCycleRank(*MainCentre) < HOLCommandGrid::GetCycleRank(*House)
			&& HOLCommandGrid::GetCycleRank(*House) < HOLCommandGrid::GetCycleRank(*MiningCamp));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCommandGridCategoriesTest, "HeroesOfLiberty.CommandGrid.Categories", UE::HeroesOfLiberty::Tests::CommandGrid::Flags)
bool FHOLCommandGridCategoriesTest::RunTest(const FString& Parameters)
{
	// The table's types are all economic (§ 17, the column Category): F2 takes them, F1 and F3 none.
	for (const FName TypeName : { HOLBuildingTypeNames::MainCentre, HOLBuildingTypeNames::House, HOLBuildingTypeNames::LumberCamp, HOLBuildingTypeNames::MiningCamp, HOLBuildingTypeNames::Mill })
	{
		const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(TypeName);
		if (TestNotNull(*TypeName.ToString(), Type))
		{
			TestTrue(*FString::Printf(TEXT("%s: economic"), *TypeName.ToString()), Type->Category == EHOLBuildingCategory::Economic);
			TestTrue(*FString::Printf(TEXT("%s: F2 takes it"), *TypeName.ToString()), HOLCommandGrid::IsSelectedByCategory(*Type, EHOLBuildingCategory::Economic));
			TestFalse(*FString::Printf(TEXT("%s: F1 does not"), *TypeName.ToString()), HOLCommandGrid::IsSelectedByCategory(*Type, EHOLBuildingCategory::Military));
			TestFalse(*FString::Printf(TEXT("%s: F3 does not"), *TypeName.ToString()), HOLCommandGrid::IsSelectedByCategory(*Type, EHOLBuildingCategory::Technological));
		}
	}

	// Whatever its state, a ghost (a foundation not started, its owner's alone) included (the user's 2nd trial of J6a.5):
	// the category alone decides.
	FHOLBuildingTypeRow Barracks;
	Barracks.Category = EHOLBuildingCategory::Military;
	TestTrue(TEXT("a military building, site or ghost: F1 takes it"), HOLCommandGrid::IsSelectedByCategory(Barracks, EHOLBuildingCategory::Military));
	TestFalse(TEXT("a military building: F2 leaves it"), HOLCommandGrid::IsSelectedByCategory(Barracks, EHOLBuildingCategory::Economic));

	// A press again: the same key, and the selection still what it selected (in any order).
	const TArray<uint32> Selected = { 7, 3, 9 };
	TestTrue(TEXT("same key, same buildings: the next one"), HOLCommandGrid::IsRepeatPress({ 3, 9, 7 }, Selected, true));
	TestFalse(TEXT("another key: select again"), HOLCommandGrid::IsRepeatPress({ 3, 9, 7 }, Selected, false));
	TestFalse(TEXT("one building left the selection (a click): select again"), HOLCommandGrid::IsRepeatPress({ 3 }, Selected, true));
	TestFalse(TEXT("another selection of as many: select again"), HOLCommandGrid::IsRepeatPress({ 3, 9, 8 }, Selected, true));
	TestFalse(TEXT("nothing selected: select"), HOLCommandGrid::IsRepeatPress({}, {}, true));
	TestTrue(TEXT("one building alone (the centre at the start): the camera on it"), HOLCommandGrid::IsRepeatPress({ 4 }, { 4 }, true));

	// No building of the category: the short message.
	FHOLCommandFeedback None;
	None.Kind = EHOLCommandFeedback::NoBuilding;
	None.Category = EHOLBuildingCategory::Military;
	TestEqual(TEXT("no military building logged"), HOLCommandGrid::Describe(None), FString(TEXT("no Military building")));
	TestFalse(TEXT("told, nothing to block"), HOLCommandGrid::Blocks(None));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCommandGridGroupTest, "HeroesOfLiberty.CommandGrid.Group", UE::HeroesOfLiberty::Tests::CommandGrid::Flags)
bool FHOLCommandGridGroupTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::CommandGrid;

	// One of several shown together: a finished centre produces the Peasant on A; a site nothing, not even « Annuler ».
	FHOLGridContext Centres;
	Centres.bOwnBuilding = true;
	Centres.bBuildingGroup = true;
	Centres.bBuildingFinished = true;
	Centres.Produces = { Peasant };
	TestTrue(TEXT("a centre among others: A produces a Peasant"), HOLCommandGrid::GetCommand(Centres, 0).Command == EHOLGridCommand::Produce);
	FHOLGridContext WithSite = Centres;
	WithSite.bBuildingFinished = false;
	WithSite.Produces.Reset();
	for (int32 Cell = 0; Cell < HOLCommandGrid::NumCells; ++Cell)
	{
		TestTrue(*FString::Printf(TEXT("a site among others: cell %d empty"), Cell), HOLCommandGrid::GetCommand(WithSite, Cell).IsNone());
	}

	// A subgroup of one site alone (Tab): read as that site selected, its « Annuler » on V.
	FHOLGridContext OneSite = WithSite;
	OneSite.bBuildingGroup = false;
	TestTrue(TEXT("a subgroup of one site: V cancels it"), HOLCommandGrid::GetCommand(OneSite, HOLCommandGrid::CancelCell).Command == EHOLGridCommand::CancelConstruction);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCommandGridSubgroupsTest, "HeroesOfLiberty.CommandGrid.Subgroups", UE::HeroesOfLiberty::Tests::CommandGrid::Flags)
bool FHOLCommandGridSubgroupsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::CommandGrid;

	// The user's 2nd trial of J6a.5: « Tous », then one subgroup per type in the group's order, each main centre alone.
	const FName House = HOLBuildingTypeNames::House;
	const FName Camp = HOLBuildingTypeNames::LumberCamp;
	const FName Centre = HOLBuildingTypeNames::MainCentre;
	TestTrue(TEXT("the main centre is alone in a group"), HOLCommandGrid::IsAloneInGroup(Centre) && !HOLCommandGrid::IsAloneInGroup(House));
	const TArray<FHOLGroupMember> Members = {
		{ Centre, true },  // 0
		{ House, false },  // 1
		{ Camp, false },   // 2
		{ House, false },  // 3
		{ Centre, true },  // 4: a second forum, distinct
		{ House, false },  // 5
	};
	const TArray<FHOLSubgroup> Subgroups = HOLCommandGrid::SplitSubgroups(Members);
	if (TestEqual(TEXT("All, centre, houses, camp, other centre"), Subgroups.Num(), 5))
	{
		TestTrue(TEXT("first: All, every building"), Subgroups[0].bAll && Subgroups[0].Members == TArray<int32>({ 0, 1, 2, 3, 4, 5 }));
		TestTrue(TEXT("then the first centre alone"), !Subgroups[1].bAll && Subgroups[1].TypeName == Centre && Subgroups[1].Members == TArray<int32>({ 0 }));
		TestTrue(TEXT("then the houses, all three"), Subgroups[2].TypeName == House && Subgroups[2].Members == TArray<int32>({ 1, 3, 5 }));
		TestTrue(TEXT("then the camp"), Subgroups[3].TypeName == Camp && Subgroups[3].Members == TArray<int32>({ 2 }));
		TestTrue(TEXT("the second centre, a subgroup of its own"), Subgroups[4].TypeName == Centre && Subgroups[4].Members == TArray<int32>({ 4 }));
	}
	const TArray<FHOLSubgroup> Houses = HOLCommandGrid::SplitSubgroups({ { House, false }, { House, false } });
	TestTrue(TEXT("houses alone: All, then the houses"), Houses.Num() == 2 && Houses[0].bAll && Houses[1].Members == TArray<int32>({ 0, 1 }));
	TestTrue(TEXT("no group: no subgroup"), HOLCommandGrid::SplitSubgroups({}).IsEmpty());

	// « Tous »: a key two buildings use goes to the main centre, then to the group's order (a mill's technology on A, the
	// forum's Peasant on A: A = Peasant).
	const FName Technology(TEXT("Technology"));
	const FName Militia(TEXT("Militia"));
	const TArray<FHOLGroupMember> Mixed = { { TEXT("Mill"), false }, { Centre, true }, { TEXT("Barracks"), false } };
	TestTrue(TEXT("priority: the centre, then the group's order"), HOLCommandGrid::GetCommandPriority(Mixed) == TArray<int32>({ 1, 0, 2 }));
	const auto Cells = [](const TArray<TPair<int32, FName>>& Filled)
	{
		TArray<FHOLGridCommand> Result;
		Result.SetNum(HOLCommandGrid::NumCells);
		for (const TPair<int32, FName>& Cell : Filled)
		{
			Result[Cell.Key].Command = EHOLGridCommand::Produce;
			Result[Cell.Key].TypeName = Cell.Value;
		}
		return Result;
	};
	const TArray<TArray<FHOLGridCommand>> PerBuilding = {
		Cells({ { 0, Technology }, { 1, Technology } }),  // the mill: A and Z
		Cells({ { 0, Peasant } }),                         // the forum: A
		Cells({ { 0, Militia }, { 2, Militia } }),         // the barracks: A and E
	};
	const TArray<FHOLGridCommand> Merged = HOLCommandGrid::MergeCommands(PerBuilding, HOLCommandGrid::GetCommandPriority(Mixed));
	if (TestEqual(TEXT("twelve cells"), Merged.Num(), HOLCommandGrid::NumCells))
	{
		TestTrue(TEXT("A: the forum's Peasant"), Merged[0].TypeName == Peasant);
		TestTrue(TEXT("Z: the mill's (no other on Z)"), Merged[1].TypeName == Technology);
		TestTrue(TEXT("E: the barracks' (no other on E)"), Merged[2].TypeName == Militia);
		TestTrue(TEXT("R: empty"), Merged[3].IsNone());
	}
	const TArray<FHOLGroupMember> NoCentre = { { TEXT("Mill"), false }, { TEXT("Forge"), false }, { TEXT("Barracks"), false } };
	TestTrue(TEXT("no centre: A to the first in the group's order"),
		HOLCommandGrid::MergeCommands(PerBuilding, HOLCommandGrid::GetCommandPriority(NoCentre))[0].TypeName == Technology);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCommandGridDistributionTest, "HeroesOfLiberty.CommandGrid.Distribution", UE::HeroesOfLiberty::Tests::CommandGrid::Flags)
bool FHOLCommandGridDistributionTest::RunTest(const FString& Parameters)
{
	// The user's 2nd trial of J6a.5: a unit asked of several buildings is spread evenly: the fewest queued, between equals
	// the next one after the last picked, in the group's order.
	TestEqual(TEXT("the shortest queue"), HOLCommandGrid::PickEvenly({ { 3, 5, 1 }, { 1, 5, 2 }, { 2, 5, 3 } }, 0), 1);
	TestEqual(TEXT("equal queues, none picked yet: the first in the group's order"), HOLCommandGrid::PickEvenly({ { 2, 5, 9 }, { 2, 5, 4 } }, 0), 0);
	TestEqual(TEXT("a full queue is passed"), HOLCommandGrid::PickEvenly({ { 1, 1, 1 }, { 4, 5, 2 } }, 0), 1);
	TestEqual(TEXT("every queue full: none"), HOLCommandGrid::PickEvenly({ { 5, 5, 1 }, { 5, 5, 2 } }, 0), INDEX_NONE);
	TestEqual(TEXT("one building: it, again and again"), HOLCommandGrid::PickEvenly({ { 0, 5, 7 } }, 7), 0);
	TestEqual(TEXT("the last one picked gone: the first of the fewest"), HOLCommandGrid::PickEvenly({ { 1, 5, 3 }, { 1, 5, 4 } }, 99), 0);

	// 4 presses on 2 centres: 2 + 2, whether their queues reach this machine between the presses or not.
	for (const bool bQueuesSeen : { false, true })
	{
		TArray<FHOLQueueChoice> Centres = { { 0, 5, 10 }, { 0, 5, 20 } };
		uint32 Last = 0;
		int32 Given[2] = { 0, 0 };
		for (int32 Press = 0; Press < 4; ++Press)
		{
			const int32 Index = HOLCommandGrid::PickEvenly(Centres, Last);
			if (!TestTrue(TEXT("a centre takes it"), Index != INDEX_NONE))
			{
				break;
			}
			++Given[Index];
			Last = Centres[Index].Key;
			if (bQueuesSeen)
			{
				++Centres[Index].QueueLength;
			}
		}
		TestTrue(*FString::Printf(TEXT("2 + 2 (queues seen: %d; %d + %d)"), bQueuesSeen ? 1 : 0, Given[0], Given[1]), Given[0] == 2 && Given[1] == 2);
	}

	// Three barracks, one already busy: the two others first, in turn, then all three in turn.
	TArray<FHOLQueueChoice> Barracks = { { 2, 5, 1 }, { 0, 5, 2 }, { 0, 5, 3 } };
	uint32 Last = 0;
	TArray<int32> Order;
	for (int32 Press = 0; Press < 7; ++Press)
	{
		const int32 Index = HOLCommandGrid::PickEvenly(Barracks, Last);
		Order.Add(Index);
		Last = Barracks[Index].Key;
		++Barracks[Index].QueueLength;
	}
	TestTrue(*FString::Printf(TEXT("1 2 1 2, then 0 1 2 (%s)"), *FString::JoinBy(Order, TEXT(" "), [](const int32 Index) { return FString::FromInt(Index); })),
		Order == TArray<int32>({ 1, 2, 1, 2, 0, 1, 2 }));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCommandGridTablesTest, "HeroesOfLiberty.CommandGrid.Tables", UE::HeroesOfLiberty::Tests::CommandGrid::Flags)
bool FHOLCommandGridTablesTest::RunTest(const FString& Parameters)
{
	// The builders' grid of the table (D313, the user's 3rd trial of G.1): the panel of age I, one row per category, A house,
	// Z lumber camp, E mining camp, R mill (economic), Q barracks, S archery range (military, AR-Q2a), W farm (economic,
	// IA1.2-Q1: the economic row is full); every type of age 0 for now; the centre is nobody's to build.
	FHOLGridContext Context;
	Context.bBuilders = true;
	UHOLBuildingSettings::GetBuildableTypes(Context.Buildable);
	TestEqual(TEXT("seven buildable types"), Context.Buildable.Num(), 7);
	TestTrue(TEXT("A: house"), HOLCommandGrid::GetAgeBuilding(Context, 0, 0).TypeName == HOLBuildingTypeNames::House);
	TestTrue(TEXT("Z: lumber camp"), HOLCommandGrid::GetAgeBuilding(Context, 0, 1).TypeName == HOLBuildingTypeNames::LumberCamp);
	TestTrue(TEXT("E: mining camp"), HOLCommandGrid::GetAgeBuilding(Context, 0, 2).TypeName == HOLBuildingTypeNames::MiningCamp);
	TestTrue(TEXT("R: mill (D346)"), HOLCommandGrid::GetAgeBuilding(Context, 0, 3).TypeName == HOLBuildingTypeNames::Mill);
	TestTrue(TEXT("Q: barracks"), HOLCommandGrid::GetAgeBuilding(Context, 0, 4).TypeName == HOLBuildingTypeNames::Barracks);
	TestTrue(TEXT("Q: a build command"), HOLCommandGrid::GetAgeBuilding(Context, 0, 4).Command == EHOLGridCommand::Build);
	TestTrue(TEXT("S: archery range (AR-Q2a)"), HOLCommandGrid::GetAgeBuilding(Context, 0, 5).TypeName == HOLBuildingTypeNames::ArcheryRange);
	TestTrue(TEXT("S: a build command"), HOLCommandGrid::GetAgeBuilding(Context, 0, 5).Command == EHOLGridCommand::Build);
	// IA1.2 (F4): the farm on W, cell 8 (A Z E R / Q S D F / W X C V, make_game_input.py's GRID_KEYS): row 3, column 1.
	constexpr int32 FarmCell = 8;
	TestEqual(TEXT("W: row 3"), FarmCell / HOLCommandGrid::GridColumns, 2);
	TestEqual(TEXT("W: column 1"), FarmCell % HOLCommandGrid::GridColumns, 0);
	TestTrue(TEXT("W: farm (IA1.2-Q1)"), HOLCommandGrid::GetAgeBuilding(Context, 0, FarmCell).TypeName == HOLBuildingTypeNames::Farm);
	TestTrue(TEXT("W: a build command, age I reached"), HOLCommandGrid::GetAgeBuilding(Context, 0, FarmCell).Command == EHOLGridCommand::Build
		&& !HOLCommandGrid::GetAgeBuilding(Context, 0, FarmCell).bGreyed);
	TestTrue(TEXT("W with age I chosen: builds the farm"), HOLCommandGrid::PressKey(Context, 0, FarmCell).Target == EHOLGridKeyTarget::Build
		&& HOLCommandGrid::PressKey(Context, 0, FarmCell).TypeName == HOLBuildingTypeNames::Farm);
	// The other cells of panel I unchanged: D F and X C V empty.
	for (const int32 Cell : { 6, 7, 9, 10, 11 })
	{
		TestTrue(*FString::Printf(TEXT("panel I, cell %d: no building"), Cell), HOLCommandGrid::GetAgeBuilding(Context, 0, Cell).IsNone());
	}
	for (const FHOLBuildable& Buildable : Context.Buildable)
	{
		TestEqual(*FString::Printf(TEXT("%s: age I"), *Buildable.TypeName.ToString()), Buildable.Age, 0);
		// Its row is its category's (the user's 3rd trial of G.1): A Z E R economic, Q S D F military, W X C V technology;
		// the farm alone stands on the third row (IA1.2-Q1: the economic row of age I is full).
		const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(Buildable.TypeName);
		if (TestNotNull(*FString::Printf(TEXT("%s in the table"), *Buildable.TypeName.ToString()), Type))
		{
			TestTrue(*FString::Printf(TEXT("%s: a cell of the grid"), *Buildable.TypeName.ToString()), Buildable.Cell >= 0 && Buildable.Cell < HOLCommandGrid::NumCells);
			const bool bFarm = Buildable.TypeName == HOLBuildingTypeNames::Farm;
			TestEqual(*FString::Printf(TEXT("%s: the row of its category%s"), *Buildable.TypeName.ToString(), bFarm ? TEXT(" (but the farm: row 3)") : TEXT("")),
				Buildable.Cell / HOLCommandGrid::GridColumns, bFarm ? FarmCell / HOLCommandGrid::GridColumns : HOLCommandGrid::GetCategoryRow(Type->Category));
		}
	}
	TestEqual(TEXT("economic: row 1"), HOLCommandGrid::GetCategoryRow(EHOLBuildingCategory::Economic), 0);
	TestEqual(TEXT("military: row 2"), HOLCommandGrid::GetCategoryRow(EHOLBuildingCategory::Military), 1);
	TestEqual(TEXT("technology: row 3"), HOLCommandGrid::GetCategoryRow(EHOLBuildingCategory::Technological), 2);
	for (int32 Cell = 8; Cell < HOLCommandGrid::NumCells; ++Cell)
	{
		TestTrue(*FString::Printf(TEXT("cell %d: nothing"), Cell), HOLCommandGrid::GetCommand(Context, Cell).IsNone());
	}

	// The Barracks' grid (J7a.2): A Spearman, Z Man-at-arms once finished (the Archer of J7c.1 went to the Archery range, AR-Q2);
	// a site produces nothing, Q cancels it.
	const FHOLBuildingTypeRow* BarracksType = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Barracks);
	if (TestNotNull(TEXT("Barracks in the building type table"), BarracksType))
	{
		FHOLGridContext Barracks;
		Barracks.bOwnBuilding = true;
		Barracks.bBuildingFinished = true;
		Barracks.Produces = BarracksType->Produces;
		const FHOLGridCommand A = HOLCommandGrid::GetCommand(Barracks, 0);
		const FHOLGridCommand Z = HOLCommandGrid::GetCommand(Barracks, 1);
		TestTrue(TEXT("Barracks, A: produce a Spearman"), A.Command == EHOLGridCommand::Produce && A.TypeName == FName(TEXT("Spearman")));
		TestTrue(TEXT("Barracks, Z: produce a Man-at-arms"), Z.Command == EHOLGridCommand::Produce && Z.TypeName == FName(TEXT("ManAtArms")));
		for (int32 Cell = 2; Cell < HOLCommandGrid::NumCells; ++Cell)
		{
			TestTrue(*FString::Printf(TEXT("Barracks, cell %d: nothing"), Cell), HOLCommandGrid::GetCommand(Barracks, Cell).IsNone());
		}
		Barracks.bBuildingFinished = false;
		for (int32 Cell = 0; Cell < HOLCommandGrid::NumCells; ++Cell)
		{
			const FHOLGridCommand Command = HOLCommandGrid::GetCommand(Barracks, Cell);
			TestTrue(*FString::Printf(TEXT("Barracks site, cell %d: no production"), Cell), Command.Command != EHOLGridCommand::Produce);
		}
		TestTrue(TEXT("Barracks site: the cancel cell cancels it"),
			HOLCommandGrid::GetCommand(Barracks, HOLCommandGrid::CancelCell).Command == EHOLGridCommand::CancelConstruction);
	}
	// The Archery range's grid (AR-Q2a): A Archer, Z Crossbowman once finished; nothing else, and no Archer left at the Barracks.
	const FHOLBuildingTypeRow* RangeType = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::ArcheryRange);
	if (TestNotNull(TEXT("Archery range in the building type table"), RangeType))
	{
		FHOLGridContext Range;
		Range.bOwnBuilding = true;
		Range.bBuildingFinished = true;
		Range.Produces = RangeType->Produces;
		const FHOLGridCommand A = HOLCommandGrid::GetCommand(Range, 0);
		const FHOLGridCommand Z = HOLCommandGrid::GetCommand(Range, 1);
		TestTrue(TEXT("Archery range, A: produce an Archer"), A.Command == EHOLGridCommand::Produce && A.TypeName == FName(TEXT("Archer")));
		TestTrue(TEXT("Archery range, Z: produce a Crossbowman"), Z.Command == EHOLGridCommand::Produce && Z.TypeName == FName(TEXT("Crossbowman")));
		for (int32 Cell = 2; Cell < HOLCommandGrid::NumCells; ++Cell)
		{
			TestTrue(*FString::Printf(TEXT("Archery range, cell %d: nothing"), Cell), HOLCommandGrid::GetCommand(Range, Cell).IsNone());
		}
	}
	if (BarracksType)
	{
		TestFalse(TEXT("no Archer at the Barracks any more (AR-Q2)"), BarracksType->Produces.Contains(FName(TEXT("Archer"))));
	}
	TestFalse(TEXT("the centre is not built"), Context.Buildable.ContainsByPredicate([](const FHOLBuildable& Type)
	{
		return Type.TypeName == HOLBuildingTypeNames::MainCentre;
	}));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCommandGridUnitOrdersTest, "HeroesOfLiberty.CommandGrid.UnitOrders", UE::HeroesOfLiberty::Tests::CommandGrid::Flags)
bool FHOLCommandGridUnitOrdersTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::CommandGrid;

	// The orders' panel left of the grid (the user's choice of 2026-10-21, AoE4's places): Q attack-move (soldiers among the
	// selection), S stop, V hold (any own units).
	TestEqual(TEXT("Q is cell 4"), HOLCommandGrid::AttackMoveCell, 4);
	TestEqual(TEXT("S is cell 5"), HOLCommandGrid::StopCell, 5);
	TestEqual(TEXT("V is cell 11"), HOLCommandGrid::HoldCell, 11);

	// Soldiers alone: Q S V on the left, nothing else; nothing on the right (they do not build).
	FHOLGridContext Soldiers;
	Soldiers.bOwnUnits = true;
	Soldiers.bSoldiers = true;
	TestTrue(TEXT("soldiers: Q attack-move"), HOLCommandGrid::GetOrder(Soldiers, HOLCommandGrid::AttackMoveCell).Command == EHOLGridCommand::AttackMove);
	TestTrue(TEXT("soldiers: S stop"), HOLCommandGrid::GetOrder(Soldiers, HOLCommandGrid::StopCell).Command == EHOLGridCommand::Stop);
	TestTrue(TEXT("soldiers: V hold"), HOLCommandGrid::GetOrder(Soldiers, HOLCommandGrid::HoldCell).Command == EHOLGridCommand::Hold);
	for (int32 Cell = 0; Cell < HOLCommandGrid::NumCells; ++Cell)
	{
		if (Cell != HOLCommandGrid::AttackMoveCell && Cell != HOLCommandGrid::StopCell && Cell != HOLCommandGrid::HoldCell)
		{
			TestTrue(*FString::Printf(TEXT("soldiers: order %d empty"), Cell), HOLCommandGrid::GetOrder(Soldiers, Cell).IsNone());
		}
		TestTrue(*FString::Printf(TEXT("soldiers: grid cell %d empty"), Cell), HOLCommandGrid::GetCommand(Soldiers, Cell).IsNone());
	}
	TestTrue(TEXT("D is no order any more"), HOLCommandGrid::GetOrder(Soldiers, 6).IsNone());

	// Peasants alone (builders, no soldier): no attack-move, S and V still there; the builders' grid on the right.
	FHOLGridContext Peasants;
	Peasants.bBuilders = true;
	Peasants.bOwnUnits = true;
	UHOLBuildingSettings::GetBuildableTypes(Peasants.Buildable);
	TestTrue(TEXT("Peasants: the builders' grid"), HOLCommandGrid::IsBuildGrid(Peasants));
	for (int32 Cell = 0; Cell < HOLCommandGrid::NumCells; ++Cell)
	{
		TestTrue(*FString::Printf(TEXT("Peasants alone: no attack-move on order %d"), Cell),
			HOLCommandGrid::GetOrder(Peasants, Cell).Command != EHOLGridCommand::AttackMove);
	}
	TestTrue(TEXT("Peasants: S stop"), HOLCommandGrid::GetOrder(Peasants, HOLCommandGrid::StopCell).Command == EHOLGridCommand::Stop);
	TestTrue(TEXT("Peasants: V hold"), HOLCommandGrid::GetOrder(Peasants, HOLCommandGrid::HoldCell).Command == EHOLGridCommand::Hold);
	TestTrue(TEXT("Peasants: A of panel I the house, on the right"), HOLCommandGrid::GetAgeBuilding(Peasants, 0, 0).TypeName == HOLBuildingTypeNames::House);

	// Peasants and soldiers together (« Tous » of a mixed selection): the orders, Q included; the builders' grid hidden
	// (the Peasants' subgroup, Tab, has it).
	FHOLGridContext Mixed = Peasants;
	Mixed.bSoldiers = true;
	TestFalse(TEXT("mixed: no builders' grid"), HOLCommandGrid::IsBuildGrid(Mixed));
	TestTrue(TEXT("mixed: Q attack-move"), HOLCommandGrid::GetOrder(Mixed, HOLCommandGrid::AttackMoveCell).Command == EHOLGridCommand::AttackMove);
	for (int32 Cell = 0; Cell < HOLCommandGrid::NumCells; ++Cell)
	{
		TestTrue(*FString::Printf(TEXT("mixed: cell %d empty on the right"), Cell), HOLCommandGrid::GetCommand(Mixed, Cell).IsNone());
	}

	// An enemy selection (no own unit): no order.
	FHOLGridContext Enemy;
	for (int32 Cell = 0; Cell < HOLCommandGrid::NumCells; ++Cell)
	{
		TestTrue(*FString::Printf(TEXT("enemy selection: order %d empty"), Cell), HOLCommandGrid::GetOrder(Enemy, Cell).IsNone());
	}

	// A building of the player's (the Barracks, finished or a site): its grid only, no orders.
	FHOLGridContext Barracks;
	Barracks.bOwnBuilding = true;
	Barracks.Produces = { FName(TEXT("Spearman")), FName(TEXT("ManAtArms")) };
	Barracks.bOwnUnits = true;  // never set with a building; the building wins all the same
	Barracks.bSoldiers = true;
	for (const bool bFinished : { true, false })
	{
		Barracks.bBuildingFinished = bFinished;
		for (int32 Cell = 0; Cell < HOLCommandGrid::NumCells; ++Cell)
		{
			TestTrue(*FString::Printf(TEXT("Barracks (finished %d): no order %d"), bFinished ? 1 : 0, Cell), HOLCommandGrid::GetOrder(Barracks, Cell).IsNone());
		}
		TestTrue(*FString::Printf(TEXT("Barracks (finished %d): A its first unit or nothing"), bFinished ? 1 : 0),
			HOLCommandGrid::GetCommand(Barracks, 0).Command == (bFinished ? EHOLGridCommand::Produce : EHOLGridCommand::None));
		TestTrue(*FString::Printf(TEXT("Barracks (finished %d): keys to the grid"), bFinished ? 1 : 0),
			HOLCommandGrid::PressKey(Barracks, INDEX_NONE, 4).Target == EHOLGridKeyTarget::Grid);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCommandGridHeroAbilitiesTest, "HeroesOfLiberty.CommandGrid.HeroAbilities", UE::HeroesOfLiberty::Tests::CommandGrid::Flags)
bool FHOLCommandGridHeroAbilitiesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::CommandGrid;

	// J8c.4 (J8c-Q4): the hero among the units commanded, his abilities on the grid on the right, A the cry, Z the banner;
	// the passive aura has no cell.
	TestEqual(TEXT("A: Serrez les rangs"), HOLCommandGrid::GetHeroAbilityCell(EHOLHeroAbility::CloseRanks), 0);
	TestEqual(TEXT("Z: Bannière"), HOLCommandGrid::GetHeroAbilityCell(EHOLHeroAbility::Banner), 1);
	TestEqual(TEXT("the aura: no cell"), HOLCommandGrid::GetHeroAbilityCell(EHOLHeroAbility::Aura), static_cast<int32>(INDEX_NONE));

	// The hero alone at level 1: A the cry, ready; Z the banner, greyed (level 4); nothing else; the orders as soldiers'.
	FHOLGridContext Hero;
	Hero.bOwnUnits = true;
	Hero.bSoldiers = true;
	Hero.bHero = true;
	Hero.HeroLevel = 1;
	const FHOLGridCommand Cry = HOLCommandGrid::GetCommand(Hero, 0);
	TestTrue(TEXT("level 1: A the cry, ready"), Cry.Command == EHOLGridCommand::HeroAbility && Cry.Ability == EHOLHeroAbility::CloseRanks && !Cry.bGreyed
		&& Cry.CooldownSeconds == 0);
	const FHOLGridCommand Banner = HOLCommandGrid::GetCommand(Hero, 1);
	TestTrue(TEXT("level 1: Z the banner, greyed"), Banner.Command == EHOLGridCommand::HeroAbility && Banner.Ability == EHOLHeroAbility::Banner && Banner.bGreyed);
	for (int32 Cell = 2; Cell < HOLCommandGrid::NumCells; ++Cell)
	{
		TestTrue(*FString::Printf(TEXT("hero: grid cell %d empty"), Cell), HOLCommandGrid::GetCommand(Hero, Cell).IsNone());
	}
	TestTrue(TEXT("hero: Q attack-move"), HOLCommandGrid::GetOrder(Hero, HOLCommandGrid::AttackMoveCell).Command == EHOLGridCommand::AttackMove);
	TestTrue(TEXT("hero: A no order"), HOLCommandGrid::GetOrder(Hero, 0).IsNone());

	// The keys: A and Z to the grid, the others to the orders as before.
	TestTrue(TEXT("hero: A to the grid"), HOLCommandGrid::PressKey(Hero, INDEX_NONE, 0).Target == EHOLGridKeyTarget::Grid);
	TestTrue(TEXT("hero: Z to the grid"), HOLCommandGrid::PressKey(Hero, INDEX_NONE, 1).Target == EHOLGridKeyTarget::Grid);
	TestTrue(TEXT("hero: E to the orders"), HOLCommandGrid::PressKey(Hero, INDEX_NONE, 2).Target == EHOLGridKeyTarget::Orders);
	TestTrue(TEXT("hero: Q to the orders"), HOLCommandGrid::PressKey(Hero, INDEX_NONE, HOLCommandGrid::AttackMoveCell).Target == EHOLGridKeyTarget::Orders);

	// Level 4: the banner open. Cooling down: greyed, the seconds left told.
	Hero.HeroLevel = 4;
	TestFalse(TEXT("level 4: Z ready"), HOLCommandGrid::GetCommand(Hero, 1).bGreyed);
	Hero.HeroCooldownSeconds[static_cast<int32>(EHOLHeroAbility::CloseRanks)] = 42;
	const FHOLGridCommand Cooling = HOLCommandGrid::GetCommand(Hero, 0);
	TestTrue(TEXT("cry cooling down: greyed, 42 s"), Cooling.bGreyed && Cooling.CooldownSeconds == 42);
	TestFalse(TEXT("the banner's cool-down is its own"), HOLCommandGrid::GetCommand(Hero, 1).bGreyed);

	// No hero among the units commanded (soldiers alone, another subgroup): nothing on the right, A and Z to the orders.
	FHOLGridContext Soldiers = Hero;
	Soldiers.bHero = false;
	TestTrue(TEXT("no hero: A empty"), HOLCommandGrid::GetCommand(Soldiers, 0).IsNone() && HOLCommandGrid::GetCommand(Soldiers, 1).IsNone());
	TestTrue(TEXT("no hero: A to the orders"), HOLCommandGrid::PressKey(Soldiers, INDEX_NONE, 0).Target == EHOLGridKeyTarget::Orders);

	// A building selected wins (never set together; the building's grid all the same).
	FHOLGridContext Building = Hero;
	Building.bOwnBuilding = true;
	Building.bBuildingFinished = true;
	TestTrue(TEXT("a building: no hero cell"), HOLCommandGrid::GetCommand(Building, 0).IsNone());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCommandGridAgesTest, "HeroesOfLiberty.CommandGrid.Ages", UE::HeroesOfLiberty::Tests::CommandGrid::Flags)
bool FHOLCommandGridAgesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::CommandGrid;
	using Key = FHOLGridKeyResult;

	// The 4 age panels (the user's 1st trial of G.1): only age I reached for now; a building of an age not reached greyed.
	FHOLGridContext Builders = MakeBuilders();
	Builders.bOwnUnits = true;
	Builders.Buildable.Add({ HOLBuildingTypeNames::Barracks, 4, 1 });
	TestEqual(TEXT("the age reached: I for now"), HOLCommandGrid::GetReachedAge(), 0);
	for (int32 Age = 0; Age < HOLCommandGrid::NumAges; ++Age)
	{
		TestEqual(*FString::Printf(TEXT("age %d reached: I only"), Age), HOLCommandGrid::IsAgeReached(Builders, Age), Age == 0);
	}
	const FHOLGridCommand Locked = HOLCommandGrid::GetAgeBuilding(Builders, 1, 4);
	TestTrue(TEXT("II Q: the barracks, greyed"), Locked.TypeName == HOLBuildingTypeNames::Barracks && Locked.Age == 1 && Locked.bGreyed);
	TestTrue(TEXT("III: empty"), HOLCommandGrid::GetAgeBuilding(Builders, 2, 4).IsNone());

	// No age chosen: A chooses I; Z (not reached) nothing; Q S V the orders (a Peasant has no attack-move: GetOrder).
	TestTrue(TEXT("A: age I chosen"), HOLCommandGrid::PressKey(Builders, INDEX_NONE, 0) == Key{ EHOLGridKeyTarget::Age, 0 });
	TestTrue(TEXT("Z, not reached: nothing"), HOLCommandGrid::PressKey(Builders, INDEX_NONE, 1) == Key{ EHOLGridKeyTarget::None, INDEX_NONE });
	TestTrue(TEXT("Q, no age: the orders"), HOLCommandGrid::PressKey(Builders, INDEX_NONE, 4) == Key{ EHOLGridKeyTarget::Orders, INDEX_NONE });
	TestTrue(TEXT("S, no age: the orders (stop)"), HOLCommandGrid::PressKey(Builders, INDEX_NONE, 5).Target == EHOLGridKeyTarget::Orders
		&& HOLCommandGrid::GetOrder(Builders, 5).Command == EHOLGridCommand::Stop);
	TestTrue(TEXT("V, no age: the orders"), HOLCommandGrid::PressKey(Builders, INDEX_NONE, 11).Target == EHOLGridKeyTarget::Orders);

	// Age I chosen (the user's 3rd trial of G.1): every key its building's, A then A the house's ghost, the age kept; then
	// E the mining camp's instead (the ghost changed), Z the lumber camp's (never another age), still I.
	TestTrue(TEXT("A then A: the house, I kept"), HOLCommandGrid::PressKey(Builders, 0, 0) == Key{ EHOLGridKeyTarget::Build, 0, HOLBuildingTypeNames::House });
	TestTrue(TEXT("then E: the mining camp, I kept"), HOLCommandGrid::PressKey(Builders, 0, 2) == Key{ EHOLGridKeyTarget::Build, 0, HOLBuildingTypeNames::MiningCamp });
	TestTrue(TEXT("then Z: the lumber camp, I kept"), HOLCommandGrid::PressKey(Builders, 0, 1) == Key{ EHOLGridKeyTarget::Build, 0, HOLBuildingTypeNames::LumberCamp });
	// An empty cell of the chosen age: nothing, never an order nor an age, the age kept.
	TestTrue(TEXT("I, V (empty): nothing, I kept"), HOLCommandGrid::PressKey(Builders, 0, 11) == Key{ EHOLGridKeyTarget::None, 0 });
	TestTrue(TEXT("I, R (empty): nothing, I kept"), HOLCommandGrid::PressKey(Builders, 0, 3) == Key{ EHOLGridKeyTarget::None, 0 });
	TestTrue(TEXT("I, Q (empty): nothing, I kept"), HOLCommandGrid::PressKey(Builders, 0, 4) == Key{ EHOLGridKeyTarget::None, 0 });
	TestTrue(TEXT("out of the grid: nothing, I kept"), HOLCommandGrid::PressKey(Builders, 0, 12) == Key{ EHOLGridKeyTarget::None, 0 });

	// Age II reached (later): Z chooses it with no age chosen, its Q the barracks; an age chosen, Z is its building's.
	FHOLGridContext Later = Builders;
	Later.ReachedAge = 1;
	TestTrue(TEXT("II reached, no age, Z: II chosen"), HOLCommandGrid::PressKey(Later, INDEX_NONE, 1) == Key{ EHOLGridKeyTarget::Age, 1 });
	TestTrue(TEXT("II reached, I chosen, Z: the lumber camp, I kept"), HOLCommandGrid::PressKey(Later, 0, 1) == Key{ EHOLGridKeyTarget::Build, 0, HOLBuildingTypeNames::LumberCamp });
	TestTrue(TEXT("II, A (empty): nothing, II kept"), HOLCommandGrid::PressKey(Later, 1, 0) == Key{ EHOLGridKeyTarget::None, 1 });
	TestTrue(TEXT("II, Q: the barracks"), HOLCommandGrid::PressKey(Later, 1, 4) == Key{ EHOLGridKeyTarget::Build, 1, HOLBuildingTypeNames::Barracks });
	TestTrue(TEXT("II, S (empty): nothing"), HOLCommandGrid::PressKey(Later, 1, 5) == Key{ EHOLGridKeyTarget::None, 1 });

	// The mouse: a header chooses its age, a building its ghost (its age chosen); an age not reached nothing, the chosen kept.
	TestTrue(TEXT("click header I"), HOLCommandGrid::ClickAge(Builders, INDEX_NONE, 0, INDEX_NONE) == Key{ EHOLGridKeyTarget::Age, 0 });
	TestTrue(TEXT("click header II (not reached): nothing"), HOLCommandGrid::ClickAge(Builders, 0, 1, INDEX_NONE) == Key{ EHOLGridKeyTarget::None, 0 });
	TestTrue(TEXT("click header I, I chosen: I"), HOLCommandGrid::ClickAge(Builders, 0, 0, INDEX_NONE) == Key{ EHOLGridKeyTarget::Age, 0 });
	TestTrue(TEXT("click I Z: the lumber camp, I chosen"), HOLCommandGrid::ClickAge(Builders, INDEX_NONE, 0, 1) == Key{ EHOLGridKeyTarget::Build, 0, HOLBuildingTypeNames::LumberCamp });
	TestTrue(TEXT("click II Q (not reached): nothing"), HOLCommandGrid::ClickAge(Builders, INDEX_NONE, 1, 4) == Key{ EHOLGridKeyTarget::None, INDEX_NONE });

	// The chosen age closed by a placement, a cancel, Escape, another selection; kept by a placement with Shift.
	TestEqual(TEXT("placed: closed"), HOLCommandGrid::GetChosenAgeAfter(0, EHOLAgeEvent::Placed), int32(INDEX_NONE));
	TestEqual(TEXT("placed with Shift: kept"), HOLCommandGrid::GetChosenAgeAfter(0, EHOLAgeEvent::PlacedKept), 0);
	TestEqual(TEXT("cancelled: closed"), HOLCommandGrid::GetChosenAgeAfter(0, EHOLAgeEvent::Cancelled), int32(INDEX_NONE));
	TestEqual(TEXT("Escape: closed"), HOLCommandGrid::GetChosenAgeAfter(0, EHOLAgeEvent::Escape), int32(INDEX_NONE));
	TestEqual(TEXT("another selection: closed"), HOLCommandGrid::GetChosenAgeAfter(0, EHOLAgeEvent::SelectionChanged), int32(INDEX_NONE));

	// Soldiers among the selection: no panels, A Z E R go to the orders (nothing there), Q to the attack-move.
	FHOLGridContext Mixed = Builders;
	Mixed.bSoldiers = true;
	TestTrue(TEXT("mixed, A: the orders"), HOLCommandGrid::PressKey(Mixed, INDEX_NONE, 0) == Key{ EHOLGridKeyTarget::Orders, INDEX_NONE });
	TestTrue(TEXT("mixed, Q: the orders"), HOLCommandGrid::PressKey(Mixed, INDEX_NONE, 4).Target == EHOLGridKeyTarget::Orders);
	TestTrue(TEXT("mixed, Q: attack-move"), HOLCommandGrid::GetOrder(Mixed, 4).Command == EHOLGridCommand::AttackMove);
	TestTrue(TEXT("mixed: no panel"), HOLCommandGrid::GetAgeBuilding(Mixed, 0, 0).IsNone());
	TestTrue(TEXT("mixed: a click on a panel does nothing"), HOLCommandGrid::ClickAge(Mixed, INDEX_NONE, 0, 0) == Key{ EHOLGridKeyTarget::None, INDEX_NONE });
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCommandGridMouseCursorTest, "HeroesOfLiberty.CommandGrid.MouseCursor", UE::HeroesOfLiberty::Tests::CommandGrid::Flags)

bool FHOLCommandGridMouseCursorTest::RunTest(const FString& Parameters)
{
	// The user's trial of A.2: the red attack cursor while Q waits for its point, the arrow again once chosen or cancelled.
	TestEqual(TEXT("Q pressed, the point not chosen: the attack cursor"), int32(HOLCommandGrid::GetMouseCursor(/*bAttackMoveArmed*/ true, /*bPlacing*/ false)), int32(EMouseCursor::Crosshairs));
	TestEqual(TEXT("the point chosen or the mode cancelled: the arrow"), int32(HOLCommandGrid::GetMouseCursor(false, false)), int32(EMouseCursor::Default));
	TestEqual(TEXT("placement mode: the arrow (the ghost shows it)"), int32(HOLCommandGrid::GetMouseCursor(false, true)), int32(EMouseCursor::Default));
	TestEqual(TEXT("placement wins over a mode left armed"), int32(HOLCommandGrid::GetMouseCursor(true, true)), int32(EMouseCursor::Default));
	return true;
}

#endif
