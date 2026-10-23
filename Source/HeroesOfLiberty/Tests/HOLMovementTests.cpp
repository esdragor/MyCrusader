// Copyright Epic Games, Inc. All Rights Reserved.

#include "Algo/Reverse.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Player/HOLSelectionMarkers.h"
#include "Player/HOLUnitPicking.h"
#include "Simulation/HOLGroupMoveRules.h"

#if WITH_DEV_AUTOMATION_TESTS

// J4 checks (D289): the rules of a group's move order (D286 clusters, D287 slots and their assignment).
namespace UE::HeroesOfLiberty::Tests::Movement
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** The Peasant's body (60 cm radius, D283) and its slot spacing. */
	constexpr double Spacing = 120. * HOLGroupMove::SlotSpacingInDiameters;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMovementClustersTest, "HeroesOfLiberty.Movement.Clusters", UE::HeroesOfLiberty::Tests::Movement::Flags)
bool FHOLMovementClustersTest::RunTest(const FString& Parameters)
{
	TArray<int32> Clusters;
	TestEqual(TEXT("no unit, no cluster"), HOLGroupMove::MakeClusters({}, 1500., Clusters), 0);

	// A chain 10 m apart (one cluster though its ends are 30 m apart), a lone unit far away, and a pair.
	const TArray<FVector2D> Positions = { { 0., 0. }, { 1000., 0. }, { 9000., 9000. }, { 2000., 0. }, { 3000., 0. },
		{ -8000., 0. }, { -8000., 500. } };
	TestEqual(TEXT("three clusters"), HOLGroupMove::MakeClusters(Positions, 1500., Clusters), 3);
	TestEqual(TEXT("a cluster per unit"), Clusters.Num(), Positions.Num());
	TestTrue(TEXT("the chain is one cluster, numbered 0 (its first unit comes first)"),
		Clusters[0] == 0 && Clusters[1] == 0 && Clusters[3] == 0 && Clusters[4] == 0);
	TestEqual(TEXT("the lone unit is cluster 1"), Clusters[2], 1);
	TestTrue(TEXT("the pair is cluster 2"), Clusters[5] == 2 && Clusters[6] == 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMovementSlotsTest, "HeroesOfLiberty.Movement.Slots", UE::HeroesOfLiberty::Tests::Movement::Flags)
bool FHOLMovementSlotsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Movement;
	TArray<FVector2D> Slots;
	HOLGroupMove::MakeSlots(1, Spacing, Slots);
	TestTrue(TEXT("a lone unit goes to the target itself"), Slots.Num() == 1 && Slots[0].IsNearlyZero());

	for (const int32 Count : { 7, 30, 175 })
	{
		HOLGroupMove::MakeSlots(Count, Spacing, Slots);
		TestEqual(FString::Printf(TEXT("%d slots"), Count), Slots.Num(), Count);
		TestTrue(TEXT("the first slot is the target"), Slots[0].IsNearlyZero());

		double SmallestGap = TNumericLimits<double>::Max();
		double Farthest = 0.;
		for (int32 First = 0; First < Slots.Num(); ++First)
		{
			Farthest = FMath::Max(Farthest, Slots[First].Size());
			for (int32 Second = First + 1; Second < Slots.Num(); ++Second)
			{
				SmallestGap = FMath::Min(SmallestGap, FVector2D::Distance(Slots[First], Slots[Second]));
			}
			if (First > 0)
			{
				TestTrue(TEXT("nearest first"), Slots[First].Size() >= Slots[First - 1].Size() - 1.);
			}
		}
		// No two slots closer than the spacing: no unit stands on another.
		TestTrue(FString::Printf(TEXT("%d slots: no overlap (smallest gap %.1f cm)"), Count, SmallestGap), SmallestGap >= Spacing - 0.01);
		// Tight: a honeycomb disc of Count cells has a radius of about Spacing * sqrt(Count * sqrt(3) / (2 pi)).
		const double DiscRadius = Spacing * FMath::Sqrt(Count * UE_SQRT_3 / (2. * UE_PI));
		TestTrue(FString::Printf(TEXT("%d slots: a tight disc (farthest %.0f cm, disc %.0f cm)"), Count, Farthest, DiscRadius),
			Farthest <= DiscRadius + Spacing);
	}

	HOLGroupMove::MakeSlots(7, Spacing, Slots);
	TArray<FVector2D> Again;
	HOLGroupMove::MakeSlots(7, Spacing, Again);
	TestTrue(TEXT("the same slots every time"), Slots == Again);

	// The march's frame: forward along +Y in the world, the right side is then -X.
	TestTrue(TEXT("forward offset turned along the march"), HOLGroupMove::ToWorld(FVector2D(100., 0.), FVector2D(0., 5.)).Equals(FVector2D(0., 100.), 0.01));
	TestTrue(TEXT("right offset turned to the march's right"), HOLGroupMove::ToWorld(FVector2D(0., 100.), FVector2D(0., 1.)).Equals(FVector2D(-100., 0.), 0.01));
	TestTrue(TEXT("no direction: the world's frame"), HOLGroupMove::ToWorld(FVector2D(10., 20.), FVector2D::ZeroVector).Equals(FVector2D(10., 20.), 0.01));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMovementAssignTest, "HeroesOfLiberty.Movement.Assign", UE::HeroesOfLiberty::Tests::Movement::Flags)
bool FHOLMovementAssignTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Movement;

	// A group already in the slots' shape, 50 m away, keeps it: each unit takes the slot of its own place.
	TArray<FVector2D> Offsets;
	HOLGroupMove::MakeSlots(12, Spacing, Offsets);
	const FVector2D Target(5000., 2000.);
	const FVector2D Forward(1., 0.);
	TArray<FVector2D> Slots;
	TArray<FVector2D> Units;
	for (const FVector2D& Offset : Offsets)
	{
		Slots.Add(Target + HOLGroupMove::ToWorld(Offset, Forward));
		Units.Add(HOLGroupMove::ToWorld(Offset, Forward));
	}
	// Units listed in another order than the slots.
	Algo::Reverse(Units);
	TArray<int32> SlotOfUnit;
	HOLGroupMove::AssignSlots(Units, Slots, SlotOfUnit);
	bool bKept = SlotOfUnit.Num() == Units.Num();
	for (int32 Unit = 0; bKept && Unit < Units.Num(); ++Unit)
	{
		bKept = SlotOfUnit[Unit] == Units.Num() - 1 - Unit;
	}
	TestTrue(TEXT("a group in the slots' shape keeps it"), bKept);

	// A line across the march, spread wider than the slots: the unit on the left arrives on the left, and every
	// slot is given once.
	TArray<FVector2D> Line;
	for (int32 Index = 0; Index < 7; ++Index)
	{
		Line.Emplace(0., -1500. + 500. * Index);	// marching along +X: the left is -Y
	}
	HOLGroupMove::MakeSlots(7, Spacing, Offsets);
	Slots.Reset();
	for (const FVector2D& Offset : Offsets)
	{
		Slots.Add(Target + HOLGroupMove::ToWorld(Offset, Forward));
	}
	HOLGroupMove::AssignSlots(Line, Slots, SlotOfUnit);
	TSet<int32> Given(SlotOfUnit);
	TestTrue(TEXT("every unit has its own slot"), Given.Num() == Line.Num() && !Given.Contains(INDEX_NONE));
	TestTrue(TEXT("leftmost unit arrives left of the rightmost"), Slots[SlotOfUnit[0]].Y < Slots[SlotOfUnit[6]].Y);
	TestTrue(TEXT("the middle unit takes the middle slot"), Slots[SlotOfUnit[3]].Equals(Target, 1.));

	// More slots than units: the extra ones stay free; fewer: the extra units get none.
	HOLGroupMove::AssignSlots(TArray<FVector2D>{ FVector2D(0., 0.) }, Slots, SlotOfUnit);
	TestTrue(TEXT("one unit, one slot"), SlotOfUnit.Num() == 1 && SlotOfUnit[0] != INDEX_NONE);
	HOLGroupMove::AssignSlots(Line, TArray<FVector2D>{ Target }, SlotOfUnit);
	TestEqual(TEXT("one slot for seven units: given once"), SlotOfUnit.FilterByPredicate([](int32 Slot) { return Slot != INDEX_NONE; }).Num(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMovementPickingTest, "HeroesOfLiberty.Movement.Picking", UE::HeroesOfLiberty::Tests::Movement::Flags)
bool FHOLMovementPickingTest::RunTest(const FString& Parameters)
{
	// Two units drawn 40 px tall, 10 px wide on each side; the second one in front of the first, overlapping it.
	auto Unit = [](double X, double FootY, double Depth)
	{
		FHOLPickCandidate Candidate;
		Candidate.Foot = FVector2D(X, FootY);
		Candidate.Head = FVector2D(X, FootY - 40.);
		Candidate.HalfWidth = 10.;
		Candidate.Depth = Depth;
		return Candidate;
	};
	const TArray<FHOLPickCandidate> Units = { Unit(100., 200., 3000.), Unit(108., 210., 2900.), Unit(400., 300., 2500.) };

	TestEqual(TEXT("a click on the lone unit's body"), HOLUnitPicking::PickAt(Units, FVector2D(402., 280.)), 2);
	TestEqual(TEXT("a click beside it: nothing"), HOLUnitPicking::PickAt(Units, FVector2D(430., 280.)), INDEX_NONE);
	TestEqual(TEXT("a click above its head: nothing"), HOLUnitPicking::PickAt(Units, FVector2D(400., 240.)), INDEX_NONE);
	TestEqual(TEXT("where two bodies overlap, the one in front"), HOLUnitPicking::PickAt(Units, FVector2D(104., 185.)), 1);
	TestEqual(TEXT("the one behind where only it is drawn"), HOLUnitPicking::PickAt(Units, FVector2D(95., 162.)), 0);

	// The target margin of a right click (J7c, retour d'essai 8, loop 3): near a body within the margin, the nearest one;
	// a click on a body always wins, even over one nearer the camera within the margin.
	using HOLUnitPicking::PickNear;
	bool bDirect = false;
	TestEqual(TEXT("body distance: 0 on it"), HOLUnitPicking::GetBodyDistance(Units[2], FVector2D(402., 280.)), 0.);
	TestEqual(TEXT("body distance: 20 px beside the middle, 10 px beside the body"), HOLUnitPicking::GetBodyDistance(Units[2], FVector2D(420., 280.)), 10.);
	TestEqual(TEXT("body distance: above the head, from the top of its margin"), HOLUnitPicking::GetBodyDistance(Units[2], FVector2D(400., 240.)), 15.);
	TestEqual(TEXT("margin: 10 px beside the lone unit, taken"), PickNear(Units, FVector2D(420., 280.), 14., bDirect), 2);
	TestFalse(TEXT("margin: not a direct hit"), bDirect);
	TestEqual(TEXT("margin: 20 px beside it, nothing"), PickNear(Units, FVector2D(430., 280.), 14., bDirect), INDEX_NONE);
	TestEqual(TEXT("margin: 15 px above the head, nothing at 14"), PickNear(Units, FVector2D(400., 240.), 14., bDirect), INDEX_NONE);
	TestEqual(TEXT("margin: 15 px above the head, taken at 16"), PickNear(Units, FVector2D(400., 240.), 16., bDirect), 2);
	TestEqual(TEXT("margin: on a body, the direct hit"), PickNear(Units, FVector2D(402., 280.), 14., bDirect), 2);
	TestTrue(TEXT("margin: on a body, direct"), bDirect);
	TestEqual(TEXT("margin: the direct hit behind wins over one in front 3 px away"), PickNear(Units, FVector2D(95., 162.), 14., bDirect), 0);
	TestTrue(TEXT("margin: the one behind, direct"), bDirect);
	TestEqual(TEXT("margin: between two, the nearest (8 px), not the one in front (16 px)"), PickNear(Units, FVector2D(82., 190.), 20., bDirect), 0);
	TestEqual(TEXT("margin: 0, on the body only"), PickNear(Units, FVector2D(420., 280.), 0., bDirect), INDEX_NONE);
	TestEqual(TEXT("margin scaled: as is at 1080 px"), HOLUnitPicking::GetScaledMargin(14., 1080.), 14.);
	TestEqual(TEXT("margin scaled: twice at 2160 px"), HOLUnitPicking::GetScaledMargin(14., 2160.), 28.);
	TestEqual(TEXT("margin scaled: as is when the height is unknown"), HOLUnitPicking::GetScaledMargin(14., 0.), 14.);

	TArray<int32> Boxed;
	HOLUnitPicking::PickInBox(Units, FVector2D(150., 150.), FVector2D(50., 250.), Boxed);
	TestTrue(TEXT("a box takes the two units whose middle is in it, corners in any order"), Boxed == TArray<int32>({ 0, 1 }));
	HOLUnitPicking::PickInBox(Units, FVector2D(380., 285.), FVector2D(420., 320.), Boxed);
	TestTrue(TEXT("a box over the feet only: the middle is out"), Boxed.Num() == 0);

	TestFalse(TEXT("a press released 5 px away is a click"), HOLUnitPicking::IsBox(FVector2D(10., 10.), FVector2D(13., 14.)));
	TestTrue(TEXT("released 7 px away, a box"), HOLUnitPicking::IsBox(FVector2D(10., 10.), FVector2D(17., 10.)));

	// The "." key (J5b): the idle workers in turn, back to the first after the last.
	const uint32 Idle[] = { 4, 9, 12 };
	TestEqual(TEXT("the first press: the first idle worker"), static_cast<int32>(HOLUnitPicking::NextInCycle(Idle, 0)), 4);
	TestEqual(TEXT("the next press: the next one"), static_cast<int32>(HOLUnitPicking::NextInCycle(Idle, 4)), 9);
	TestEqual(TEXT("after the last: the first again"), static_cast<int32>(HOLUnitPicking::NextInCycle(Idle, 12)), 4);
	TestEqual(TEXT("the last one got to work: the one after it"), static_cast<int32>(HOLUnitPicking::NextInCycle(Idle, 7)), 9);
	TestEqual(TEXT("no idle worker: none"), static_cast<int32>(HOLUnitPicking::NextInCycle(TConstArrayView<uint32>(), 4)), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMovementPickSameTypeTest, "HeroesOfLiberty.Movement.PickSameType", UE::HeroesOfLiberty::Tests::Movement::Flags)
bool FHOLMovementPickSameTypeTest::RunTest(const FString& Parameters)
{
	// A.1: a double click (or Ctrl + click) on one of the player's units takes all its own units of that type on the screen.
	using HOLUnitPicking::IsDoubleClick;
	TestTrue(TEXT("two clicks 0.3 s and 3 px apart: a double click"), IsDoubleClick(10., FVector2D(100., 100.), 10.3, FVector2D(102., 102.)));
	TestFalse(TEXT("0.6 s apart: two clicks"), IsDoubleClick(10., FVector2D(100., 100.), 10.6, FVector2D(100., 100.)));
	TestFalse(TEXT("20 px apart: two clicks"), IsDoubleClick(10., FVector2D(100., 100.), 10.2, FVector2D(120., 100.)));
	TestFalse(TEXT("no click before: a single click"), IsDoubleClick(-1., FVector2D(100., 100.), 0.1, FVector2D(100., 100.)));

	auto Unit = [](double X, double FootY, FName Type, bool bOwn)
	{
		FHOLTypedPickCandidate Candidate;
		Candidate.Pick.Foot = FVector2D(X, FootY);
		Candidate.Pick.Head = FVector2D(X, FootY - 40.);
		Candidate.Pick.HalfWidth = 10.;
		Candidate.TypeName = Type;
		Candidate.bOwn = bOwn;
		return Candidate;
	};
	const FName Archer(TEXT("Archer"));
	const FName ManAtArms(TEXT("ManAtArms"));
	const FVector2D Screen(1920., 1080.);
	// Own archers: two on the screen, one beyond its right side, one whose feet only are off its bottom; a man-at-arms of
	// the player's own; an enemy archer.
	const TArray<FHOLTypedPickCandidate> Units = { Unit(100., 200., Archer, true), Unit(500., 300., Archer, true),
		Unit(2100., 300., Archer, true), Unit(800., 1090., Archer, true), Unit(600., 300., ManAtArms, true), Unit(700., 300., Archer, false) };

	TArray<int32> Picked;
	HOLUnitPicking::PickSameTypeOnScreen(Units, 1, Screen, Picked);
	TestTrue(TEXT("own archer: the own archers on the screen, the clicked one first"), Picked == TArray<int32>({ 1, 0, 3 }));
	HOLUnitPicking::PickSameTypeOnScreen(Units, 4, Screen, Picked);
	TestTrue(TEXT("own man-at-arms: itself, the only one of its type"), Picked == TArray<int32>({ 4 }));
	HOLUnitPicking::PickSameTypeOnScreen(Units, 5, Screen, Picked);
	TestTrue(TEXT("an enemy archer: itself alone"), Picked == TArray<int32>({ 5 }));
	HOLUnitPicking::PickSameTypeOnScreen(Units, INDEX_NONE, Screen, Picked);
	TestTrue(TEXT("no unit clicked: none"), Picked.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMovementPickTopSideTest, "HeroesOfLiberty.Movement.PickTopSide", UE::HeroesOfLiberty::Tests::Movement::Flags)
bool FHOLMovementPickTopSideTest::RunTest(const FString& Parameters)
{
	// D.1: a box over several sides keeps the first one present, Own > Ally > Enemy > Neutral; an enemy group selected
	// shows its health bars, full or not.
	using HOLUnitPicking::KeepTopSide;
	using HOLUnitPicking::CombineBox;
	constexpr EHOLSelectionSide Own = EHOLSelectionSide::Own;
	constexpr EHOLSelectionSide Ally = EHOLSelectionSide::Ally;
	constexpr EHOLSelectionSide Enemy = EHOLSelectionSide::Enemy;
	constexpr EHOLSelectionSide Neutral = EHOLSelectionSide::Neutral;

	EHOLSelectionSide Top = Neutral;
	TestFalse(TEXT("no unit: no side"), HOLUnitPicking::GetTopSide({}, Top));
	TestTrue(TEXT("enemies and neutrals: the enemies' side"), HOLUnitPicking::GetTopSide({ Neutral, Enemy, Neutral }, Top) && Top == Enemy);

	TArray<int32> Kept;
	KeepTopSide({ Enemy, Own, Enemy, Own, Enemy, Enemy, Own, Enemy }, Kept);
	TestTrue(TEXT("3 own and 5 enemies: the 3 own"), Kept == TArray<int32>({ 1, 3, 6 }));
	KeepTopSide({ Enemy, Neutral, Enemy, Enemy, Neutral, Enemy, Enemy }, Kept);
	TestTrue(TEXT("5 enemies and 2 neutrals: the 5 enemies"), Kept == TArray<int32>({ 0, 2, 3, 5, 6 }));
	KeepTopSide({ Enemy, Ally, Neutral }, Kept);
	TestTrue(TEXT("an ally before an enemy"), Kept == TArray<int32>({ 1 }));
	KeepTopSide({ Neutral, Neutral }, Kept);
	TestTrue(TEXT("neutrals alone: kept"), Kept == TArray<int32>({ 0, 1 }));
	KeepTopSide({}, Kept);
	TestTrue(TEXT("no unit: none"), Kept.IsEmpty());

	TArray<uint32> Units;
	CombineBox({}, Own, { 10, 20, 30 }, { Enemy, Own, Enemy }, false, Units);
	TestTrue(TEXT("a box on own units and enemies: the own"), Units == TArray<uint32>({ 20 }));
	CombineBox({ 1, 2 }, Own, { 10, 30 }, { Enemy, Enemy }, false, Units);
	TestTrue(TEXT("a box on enemies, no Shift: the enemies replace the own"), Units == TArray<uint32>({ 10, 30 }));
	CombineBox({ 1, 2 }, Own, { 10, 30 }, { Enemy, Enemy }, true, Units);
	TestTrue(TEXT("Shift + box on enemies with own selected: the own kept"), Units == TArray<uint32>({ 1, 2 }));
	CombineBox({ 10 }, Enemy, { 1, 2 }, { Own, Own }, true, Units);
	TestTrue(TEXT("Shift + box on own units with an enemy selected: the own"), Units == TArray<uint32>({ 1, 2 }));
	CombineBox({ 10 }, Enemy, { 30, 40 }, { Enemy, Enemy }, true, Units);
	TestTrue(TEXT("Shift + box on enemies with an enemy selected: added"), Units == TArray<uint32>({ 10, 30, 40 }));
	CombineBox({ 1, 2 }, Own, { 3, 2 }, { Own, Own }, true, Units);
	TestTrue(TEXT("Shift + box on own units: added (as before D.1)"), Units == TArray<uint32>({ 1, 2, 3 }));
	CombineBox({ 1, 2, 3 }, Own, { 3, 2, 50 }, { Own, Own, Enemy }, true, Units);
	TestTrue(TEXT("Shift + box on own units all selected: removed (as before D.1)"), Units == TArray<uint32>({ 1 }));
	CombineBox({ 1, 2 }, Own, {}, {}, true, Units);
	TestTrue(TEXT("Shift + an empty box: the selection kept"), Units == TArray<uint32>({ 1, 2 }));
	CombineBox({ 1, 2 }, Own, {}, {}, false, Units);
	TestTrue(TEXT("an empty box, no Shift: nothing"), Units.IsEmpty());

	// D.2: Shift + click as a box on that unit alone; Shift + double click only adds.
	using HOLUnitPicking::CombineClick;
	CombineClick({ 1, 2 }, Own, 10, Enemy, false, Units);
	TestTrue(TEXT("a click on an enemy: itself alone"), Units == TArray<uint32>({ 10 }));
	CombineClick({ 1, 2 }, Own, 10, Enemy, true, Units);
	TestTrue(TEXT("Shift + click on an enemy with own selected: the own kept"), Units == TArray<uint32>({ 1, 2 }));
	CombineClick({ 10 }, Enemy, 30, Enemy, true, Units);
	TestTrue(TEXT("Shift + click on an enemy with an enemy selected: added"), Units == TArray<uint32>({ 10, 30 }));
	CombineClick({ 10, 30 }, Enemy, 30, Enemy, true, Units);
	TestTrue(TEXT("Shift + click on a selected enemy: removed"), Units == TArray<uint32>({ 10 }));
	CombineClick({ 10 }, Enemy, 1, Own, true, Units);
	TestTrue(TEXT("Shift + click on an own unit with an enemy selected: itself alone"), Units == TArray<uint32>({ 1 }));
	CombineClick({ 1 }, Own, 2, Own, true, Units);
	TestTrue(TEXT("Shift + click on an own unit with own selected: added (as before D.2)"), Units == TArray<uint32>({ 1, 2 }));
	CombineClick({}, Own, 10, Enemy, true, Units);
	TestTrue(TEXT("Shift + click on an enemy, nothing selected: itself"), Units == TArray<uint32>({ 10 }));
	CombineBox({ 10, 30 }, Enemy, { 30, 10 }, { Enemy, Enemy }, true, Units, /*bRemoveWhenAll*/ false);
	TestTrue(TEXT("Shift + double click on enemies all selected: kept, not removed"), Units == TArray<uint32>({ 10, 30 }));

	// D.2: a double click on an enemy takes its owner's units of its type on the screen.
	auto Unit = [](double X, FName Type, bool bOwn, int32 Owner)
	{
		FHOLTypedPickCandidate Candidate;
		Candidate.Pick.Foot = FVector2D(X, 300.);
		Candidate.Pick.Head = FVector2D(X, 260.);
		Candidate.Pick.HalfWidth = 10.;
		Candidate.TypeName = Type;
		Candidate.bOwn = bOwn;
		Candidate.OwnerPlayer = Owner;
		return Candidate;
	};
	const FName Archer(TEXT("Archer"));
	const FName Spearman(TEXT("Spearman"));
	// Slot 1's archers (two on the screen, one beyond its right side), slot 2's archer, slot 1's spearman, an own archer.
	const TArray<FHOLTypedPickCandidate> Typed = { Unit(100., Archer, false, 1), Unit(400., Archer, false, 1), Unit(2100., Archer, false, 1),
		Unit(500., Archer, false, 2), Unit(600., Spearman, false, 1), Unit(700., Archer, true, 0) };
	TArray<int32> Picked;
	HOLUnitPicking::PickSameTypeOnScreen(Typed, 1, FVector2D(1920., 1080.), Picked);
	TestTrue(TEXT("double click on slot 1's archer: slot 1's archers on the screen, the clicked one first"), Picked == TArray<int32>({ 1, 0 }));
	HOLUnitPicking::PickSameTypeOnScreen(Typed, 5, FVector2D(1920., 1080.), Picked);
	TestTrue(TEXT("double click on the own archer: itself, no enemy archer"), Picked == TArray<int32>({ 5 }));

	TestTrue(TEXT("a selected unit at full health (an enemy's too): its bar"), AHOLSelectionMarkers::ShowsHealthBar(true, true));
	TestTrue(TEXT("a selected wounded unit: its bar"), AHOLSelectionMarkers::ShowsHealthBar(true, false));
	TestTrue(TEXT("an unselected wounded unit: its bar"), AHOLSelectionMarkers::ShowsHealthBar(false, false));
	TestFalse(TEXT("an unselected unhurt unit: no bar"), AHOLSelectionMarkers::ShowsHealthBar(false, true));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMovementPickSideByTeamsTest, "HeroesOfLiberty.Movement.PickSideByTeams", UE::HeroesOfLiberty::Tests::Movement::Flags)
bool FHOLMovementPickSideByTeamsTest::RunTest(const FString& Parameters)
{
	// D.3: the sides come from the match's teams (a real configuration, through HOLVictoryRules::GetCamp), never given by
	// hand: me slot 0 (team 1), slot 1 (team 2) an enemy, slot 2 (team 1) an ally; then the same match without teams.
	auto MakeConfig = [](const int32 Team0, const int32 Team1, const int32 Team2)
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		Config.Slots.Reset();
		Config.Slots.AddDefaulted(FHOLMatchConfig::MaxSlots);
		Config.Slots[0].State = EHOLSlotState::Human;
		Config.Slots[1].State = EHOLSlotState::AI;
		Config.Slots[2].State = EHOLSlotState::AI;
		Config.Slots[0].Team = Team0;
		Config.Slots[1].Team = Team1;
		Config.Slots[2].Team = Team2;
		Config.Normalize();
		return Config;
	};
	const FHOLMatchConfig Teams = MakeConfig(1, 2, 1);
	const FHOLMatchConfig NoTeams = MakeConfig(0, 0, 0);
	constexpr int32 Me = 0;
	const FName ManAtArms(TEXT("ManAtArms"));
	const FName Spearman(TEXT("Spearman"));
	const FName Archer(TEXT("Archer"));
	// The units of the match: mine 1, 2 (Men-at-arms); slot 1's 10, 11 (Spearmen), 12 (Archer); slot 2's 20, 21
	// (Spearmen), 22 (Archer). Each one's slot, type and place on a 1920 x 1080 screen.
	struct FUnit { uint32 Id; int32 Slot; FName Type; double X; };
	const TArray<FUnit> Match = { { 1, 0, ManAtArms, 100. }, { 2, 0, ManAtArms, 150. }, { 10, 1, Spearman, 400. }, { 11, 1, Spearman, 450. },
		{ 12, 1, Archer, 500. }, { 20, 2, Spearman, 900. }, { 21, 2, Spearman, 950. }, { 22, 2, Archer, 1000. } };
	auto SlotOf = [&Match](const uint32 Id)
	{
		const FUnit* Unit = Match.FindByPredicate([Id](const FUnit& U) { return U.Id == Id; });
		return Unit ? Unit->Slot : INDEX_NONE;
	};
	auto SidesOf = [&SlotOf](const FHOLMatchConfig& Config, const TArray<uint32>& Ids)
	{
		TArray<EHOLSelectionSide> Sides;
		for (const uint32 Id : Ids)
		{
			Sides.Add(HOLUnitPicking::GetSide(Config, SlotOf(Id), Me));
		}
		return Sides;
	};
	// A box as the player controller makes it: the sides of the boxed units and of the selection, then CombineBox.
	auto Box = [&SidesOf](const FHOLMatchConfig& Config, const TArray<uint32>& Selected, const TArray<uint32>& Boxed, const bool bAdd)
	{
		EHOLSelectionSide SelectedSide = EHOLSelectionSide::Neutral;
		HOLUnitPicking::GetTopSide(SidesOf(Config, Selected), SelectedSide);
		TArray<uint32> Units;
		HOLUnitPicking::CombineBox(Selected, SelectedSide, Boxed, SidesOf(Config, Boxed), bAdd, Units);
		return Units;
	};
	auto Click = [&SidesOf](const FHOLMatchConfig& Config, const TArray<uint32>& Selected, const uint32 Clicked, const bool bAdd)
	{
		EHOLSelectionSide SelectedSide = EHOLSelectionSide::Neutral;
		HOLUnitPicking::GetTopSide(SidesOf(Config, Selected), SelectedSide);
		TArray<uint32> Units;
		HOLUnitPicking::CombineClick(Selected, SelectedSide, Clicked, SidesOf(Config, { Clicked })[0], bAdd, Units);
		return Units;
	};
	// A double click as SelectSameTypeAt: the same type of the same owner on the screen, then CombineBox without removal.
	auto DoubleClick = [&Match, &SidesOf](const FHOLMatchConfig& Config, const uint32 Clicked)
	{
		TArray<FHOLTypedPickCandidate> Typed;
		int32 ClickedIndex = INDEX_NONE;
		for (const FUnit& Unit : Match)
		{
			FHOLTypedPickCandidate& Candidate = Typed.AddDefaulted_GetRef();
			Candidate.Pick.Foot = FVector2D(Unit.X, 500.);
			Candidate.Pick.Head = FVector2D(Unit.X, 460.);
			Candidate.Pick.HalfWidth = 10.;
			Candidate.TypeName = Unit.Type;
			Candidate.bOwn = Unit.Slot == Me;
			Candidate.OwnerPlayer = Unit.Slot;
			ClickedIndex = Unit.Id == Clicked ? Typed.Num() - 1 : ClickedIndex;
		}
		TArray<int32> Indices;
		HOLUnitPicking::PickSameTypeOnScreen(Typed, ClickedIndex, FVector2D(1920., 1080.), Indices);
		TArray<uint32> Picked;
		for (const int32 Index : Indices)
		{
			Picked.Add(Match[Index].Id);
		}
		TArray<uint32> Units;
		HOLUnitPicking::CombineBox({}, EHOLSelectionSide::Neutral, Picked, SidesOf(Config, Picked), false, Units, /*bRemoveWhenAll*/ false);
		return Units;
	};

	TestTrue(TEXT("teams: my unit, own"), HOLUnitPicking::GetSide(Teams, 0, Me) == EHOLSelectionSide::Own);
	TestTrue(TEXT("teams: slot 1 (team 2), an enemy"), HOLUnitPicking::GetSide(Teams, 1, Me) == EHOLSelectionSide::Enemy);
	TestTrue(TEXT("teams: slot 2 (my team 1), an ally"), HOLUnitPicking::GetSide(Teams, 2, Me) == EHOLSelectionSide::Ally);
	TestTrue(TEXT("teams: from slot 2's eyes, slot 0 an ally"), HOLUnitPicking::GetSide(Teams, 0, 2) == EHOLSelectionSide::Ally);

	// The eight cases of D.3.
	TestTrue(TEXT("1. box on enemies alone: the enemies"), Box(Teams, {}, { 10, 11, 12 }, false) == TArray<uint32>({ 10, 11, 12 }));
	TestTrue(TEXT("1. their side: enemy"), SidesOf(Teams, { 10 })[0] == EHOLSelectionSide::Enemy);
	TestTrue(TEXT("1. selected at full health: their bars shown"), AHOLSelectionMarkers::ShowsHealthBar(true, true));
	TestTrue(TEXT("2. box on allies alone: the allies"), Box(Teams, {}, { 20, 21, 22 }, false) == TArray<uint32>({ 20, 21, 22 }));
	TestTrue(TEXT("2. their side: ally"), SidesOf(Teams, { 20 })[0] == EHOLSelectionSide::Ally);
	TestTrue(TEXT("3. box on mine and enemies: mine"), Box(Teams, {}, { 10, 1, 11, 2 }, false) == TArray<uint32>({ 1, 2 }));
	TestTrue(TEXT("4. box on mine and allies: mine"), Box(Teams, {}, { 20, 1, 21, 2 }, false) == TArray<uint32>({ 1, 2 }));
	TestTrue(TEXT("5. box on allies and enemies: the allies alone"), Box(Teams, {}, { 10, 20, 11, 21 }, false) == TArray<uint32>({ 20, 21 }));
	TestTrue(TEXT("6. allies selected, Shift + click on an enemy: the allies stay"), Click(Teams, { 20, 21 }, 10, true) == TArray<uint32>({ 20, 21 }));
	const TArray<uint32> AfterEnemy = Click(Teams, { 1, 2 }, 10, true);
	TestTrue(TEXT("7. mine selected, Shift + click on an enemy: mine stay"), AfterEnemy == TArray<uint32>({ 1, 2 }));
	TestTrue(TEXT("7. then Shift + click on an ally: mine stay"), Click(Teams, AfterEnemy, 20, true) == TArray<uint32>({ 1, 2 }));
	TestTrue(TEXT("8. double click on an ally Spearman: its owner's Spearmen on the screen"), DoubleClick(Teams, 21) == TArray<uint32>({ 21, 20 }));
	TestTrue(TEXT("8. double click on an enemy Spearman: its owner's Spearmen on the screen"), DoubleClick(Teams, 10) == TArray<uint32>({ 10, 11 }));

	// Without teams, everyone alone: every other slot an enemy, a box over slots 1 and 2 takes them all.
	TestTrue(TEXT("no teams: slot 2 an enemy"), HOLUnitPicking::GetSide(NoTeams, 2, Me) == EHOLSelectionSide::Enemy);
	TestTrue(TEXT("no teams: box on slots 1 and 2, all four (enemies all)"), Box(NoTeams, {}, { 10, 20, 11, 21 }, false) == TArray<uint32>({ 10, 20, 11, 21 }));
	TestTrue(TEXT("no teams: Shift + click on slot 1 with slot 2's selected: added (one side)"), Click(NoTeams, { 20, 21 }, 10, true) == TArray<uint32>({ 20, 21, 10 }));
	TestTrue(TEXT("no teams: box on mine and slot 2's: mine"), Box(NoTeams, {}, { 20, 1, 21, 2 }, false) == TArray<uint32>({ 1, 2 }));
	return true;
}

#endif
