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

	/** The builders' grid of J6a: A house, Z lumber camp, E mining camp. */
	FHOLGridContext MakeBuilders()
	{
		FHOLGridContext Context;
		Context.bBuilders = true;
		Context.Buildable = { { HOLBuildingTypeNames::House, 0 }, { HOLBuildingTypeNames::LumberCamp, 1 }, { HOLBuildingTypeNames::MiningCamp, 2 } };
		return Context;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCommandGridCommandsTest, "HeroesOfLiberty.CommandGrid.Commands", UE::HeroesOfLiberty::Tests::CommandGrid::Flags)
bool FHOLCommandGridCommandsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::CommandGrid;

	// Builders selected: the three buildings directly on A, Z, E (D313); nothing elsewhere.
	FHOLGridContext Builders = MakeBuilders();
	const FHOLGridCommand A = HOLCommandGrid::GetCommand(Builders, 0);
	TestTrue(TEXT("A: build a house"), A.Command == EHOLGridCommand::Build && A.TypeName == HOLBuildingTypeNames::House);
	TestTrue(TEXT("Z: build a lumber camp"), HOLCommandGrid::GetCommand(Builders, 1).TypeName == HOLBuildingTypeNames::LumberCamp);
	TestTrue(TEXT("E: build a mining camp"), HOLCommandGrid::GetCommand(Builders, 2).TypeName == HOLBuildingTypeNames::MiningCamp);
	for (int32 Cell = 3; Cell < HOLCommandGrid::NumCells; ++Cell)
	{
		TestTrue(*FString::Printf(TEXT("cell %d empty for builders"), Cell), HOLCommandGrid::GetCommand(Builders, Cell).IsNone());
	}
	TestTrue(TEXT("out of the grid: nothing"), HOLCommandGrid::GetCommand(Builders, -1).IsNone() && HOLCommandGrid::GetCommand(Builders, 12).IsNone());

	// Units that do not build (no gatherer among them): nothing.
	FHOLGridContext Soldiers = MakeBuilders();
	Soldiers.bBuilders = false;
	TestTrue(TEXT("no builder: A empty"), HOLCommandGrid::GetCommand(Soldiers, 0).IsNone());

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
	for (const FName TypeName : { HOLBuildingTypeNames::MainCentre, HOLBuildingTypeNames::House, HOLBuildingTypeNames::LumberCamp, HOLBuildingTypeNames::MiningCamp })
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
	// The builders' grid of the table (D313): A house, Z lumber camp, E mining camp, R barracks (J7a.2: no military page,
	// the pages come later by age); the centre is nobody's to build.
	FHOLGridContext Context;
	Context.bBuilders = true;
	UHOLBuildingSettings::GetBuildableTypes(Context.Buildable);
	TestEqual(TEXT("four buildable types"), Context.Buildable.Num(), 4);
	TestTrue(TEXT("A: house"), HOLCommandGrid::GetCommand(Context, 0).TypeName == HOLBuildingTypeNames::House);
	TestTrue(TEXT("Z: lumber camp"), HOLCommandGrid::GetCommand(Context, 1).TypeName == HOLBuildingTypeNames::LumberCamp);
	TestTrue(TEXT("E: mining camp"), HOLCommandGrid::GetCommand(Context, 2).TypeName == HOLBuildingTypeNames::MiningCamp);
	TestTrue(TEXT("R: barracks"), HOLCommandGrid::GetCommand(Context, 3).TypeName == HOLBuildingTypeNames::Barracks);
	TestTrue(TEXT("R: a build command"), HOLCommandGrid::GetCommand(Context, 3).Command == EHOLGridCommand::Build);
	for (int32 Cell = 4; Cell < HOLCommandGrid::NumCells; ++Cell)
	{
		TestTrue(*FString::Printf(TEXT("cell %d: nothing"), Cell), HOLCommandGrid::GetCommand(Context, Cell).IsNone());
	}

	// The Barracks' grid (J7a.2): A Spearman, Z Man-at-arms once finished; a site produces nothing, Q cancels it.
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
	TestFalse(TEXT("the centre is not built"), Context.Buildable.ContainsByPredicate([](const TPair<FName, int32>& Type)
	{
		return Type.Key == HOLBuildingTypeNames::MainCentre;
	}));
	return true;
}

#endif
