// Copyright Epic Games, Inc. All Rights Reserved.

#include "Algo/Reverse.h"
#include "Misc/AutomationTest.h"
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

#endif
