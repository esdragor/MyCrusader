// The camp layout's fairness alone (NeutralsInProgress.md N1b): starts on a circle, camps placed by rotation (fair) or not (unfair).

const float LayoutRadius = 20000.f;

// Points at the given radius, one per index, turned by Offset degrees: Count of them regularly around the origin.
TArray<FVector> LayoutRing(int Count, float Radius, float OffsetDegrees)
{
	TArray<FVector> Points;
	for (int Index = 0; Index < Count; Index++)
	{
		const float Angle = Math::DegreesToRadians(OffsetDegrees + 360.f * float(Index) / float(Count));
		Points.Add(FVector(Math::Cos(Angle) * Radius, Math::Sin(Angle) * Radius, 0));
	}
	return Points;
}

// Camps on two rings, one turned halfway between the starts, one aligned with them: the same for every player.
TArray<FVector> LayoutCamps(int Players)
{
	TArray<FVector> Posts = LayoutRing(Players, LayoutRadius * 0.5f, 0.f);
	TArray<FVector> Between = LayoutRing(Players, LayoutRadius * 0.8f, 180.f / float(Players));
	for (int Index = 0; Index < Between.Num(); Index++)
	{
		Posts.Add(Between[Index]);
	}
	return Posts;
}

void Test_CampLayoutSortedDistances(FUnitTest& T)
{
	TArray<FVector> Posts;
	Posts.Add(FVector(3000, 0, 500));
	Posts.Add(FVector(0, 1000, 0));
	Posts.Add(FVector(-2000, 0, 0));
	const TArray<float> Distances = HOLCampLayout::SortedDistances(FVector(0, 0, 0), Posts);
	T.AssertEquals(3, Distances.Num(), "one distance per post");
	T.AssertEquals(1000.f, Distances[0], "the nearest first");
	T.AssertEquals(2000.f, Distances[1], "then the next");
	T.AssertEquals(3000.f, Distances[2], "the height does not count");
}

// For 2, 4, 6 and 8 players: a layout by rotation is fair (gap 0, to the rounding of the angles).
void Test_CampLayoutRotationIsFair(FUnitTest& T)
{
	for (int Players = 2; Players <= 8; Players += 2)
	{
		const float Gap = HOLCampLayout::EquityGap(LayoutRing(Players, LayoutRadius, 0.f), LayoutCamps(Players));
		T.AssertTrue(Gap < 1.f, f"{Players} players: the rotated layout is fair (gap {Gap} cm)");
	}
}

// A camp moved by hand breaks the fairness, and the gap says by how much (the displaced camp is 1000 cm nearer to a start).
void Test_CampLayoutDisplacedCampIsUnfair(FUnitTest& T)
{
	TArray<FVector> Starts = LayoutRing(4, LayoutRadius, 0.f);
	TArray<FVector> Posts = LayoutCamps(4);
	const float Fair = HOLCampLayout::EquityGap(Starts, Posts);
	T.AssertTrue(Fair < 1.f, "fair to begin with");
	Posts[0] = Posts[0] + (Starts[0] - Posts[0]) / (Starts[0] - Posts[0]).Size() * 1000.f;
	const float Gap = HOLCampLayout::EquityGap(Starts, Posts);
	T.AssertTrue(Gap > 500.f, f"a camp moved 1000 cm toward the first start: unfair (gap {Gap} cm)");
	T.AssertTrue(Gap < 1001.f, f"and the gap is no more than the move (gap {Gap} cm)");
}

// A mirror layout is fair for two players; a camp added on one side only is not.
void Test_CampLayoutMirrorForTwo(FUnitTest& T)
{
	TArray<FVector> Starts;
	Starts.Add(FVector(-LayoutRadius, 0, 0));
	Starts.Add(FVector(LayoutRadius, 0, 0));
	TArray<FVector> Posts;
	Posts.Add(FVector(-5000, 7000, 0));
	Posts.Add(FVector(5000, 7000, 0));
	Posts.Add(FVector(-5000, -7000, 0));
	Posts.Add(FVector(5000, -7000, 0));
	T.AssertTrue(HOLCampLayout::EquityGap(Starts, Posts) < 1.f, "a mirror layout is fair");
	Posts.Add(FVector(-12000, 0, 0));
	T.AssertTrue(HOLCampLayout::EquityGap(Starts, Posts) > 1000.f, "one extra camp on one side only: unfair");
}

// Nothing to compare: a single start or no camp gives no gap.
void Test_CampLayoutNothingToCompare(FUnitTest& T)
{
	TArray<FVector> One;
	One.Add(FVector(0, 0, 0));
	T.AssertEquals(0.f, HOLCampLayout::EquityGap(One, LayoutCamps(4)), "one start: nothing to compare");
	T.AssertEquals(0.f, HOLCampLayout::EquityGap(LayoutRing(4, LayoutRadius, 0.f), TArray<FVector>()), "no camp: nothing to compare");
}
