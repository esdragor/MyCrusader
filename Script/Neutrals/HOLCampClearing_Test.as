// The clearing of a neutral camp alone (HALDIR-1120pe): no tree within the clearing radius of the post. The distances come from
// HOLCampClearing::RadiusCm() (no number retyped in the expectations).

const FVector ClearingPost = FVector(0, 0, 0);

// The tree the scenario neutrals_clairiere left in front of the camp is inside the radius: it must be on the list to clear.
void Test_ClearingTakesTheTreeInFrontOfTheCamp(FUnitTest& T)
{
	const float Radius = HOLCampClearing::RadiusCm();
	TArray<FVector> Trees;
	Trees.Add(FVector(Radius * 0.2f, Radius * 0.1f, 0));
	Trees.Add(FVector(Radius * 3.f, 0, 0));
	TArray<int> Taken = HOLCampClearing::TreesToClear(ClearingPost, Trees);
	T.AssertEquals(Taken.Num(), 1, "one of the two trees stands in the clearing");
	if (Taken.Num() == 1)
	{
		T.AssertEquals(Taken[0], 0, "the near tree is the one to clear");
	}
}

// The edge counts as inside, one centimetre beyond does not, and the height does not matter (a crown is high, the post is on the ground).
void Test_ClearingEdgeAndHeight(FUnitTest& T)
{
	const float Radius = HOLCampClearing::RadiusCm();
	T.AssertTrue(HOLCampClearing::IsInside(ClearingPost, FVector(Radius, 0, 0)), "on the edge: inside");
	T.AssertFalse(HOLCampClearing::IsInside(ClearingPost, FVector(Radius + 1.f, 0, 0)), "one centimetre beyond: outside");
	T.AssertTrue(HOLCampClearing::IsInside(ClearingPost, FVector(0, Radius, 650.f)), "a high point above the edge: height ignored, inside");
}

// Every tree of a grove ring inside the radius goes, none outside: the list is exactly the trees within the radius (counted apart).
void Test_ClearingTakesEveryTreeInsideAndNoneOutside(FUnitTest& T)
{
	const FVector Post = FVector(10000, -4000, 0);
	const float Radius = HOLCampClearing::RadiusCm();
	TArray<FVector> Trees;
	int Expected = 0;
	for (int Index = 0; Index < 12; Index++)
	{
		const float Distance = Radius * (0.25f + 0.2f * Index);
		Trees.Add(FVector(Post.X + Distance, Post.Y, 250));
		if (Distance <= Radius)
		{
			Expected++;
		}
	}
	TArray<int> Taken = HOLCampClearing::TreesToClear(Post, Trees);
	T.AssertEquals(Taken.Num(), Expected, "exactly the trees within the radius are taken");
	for (int Slot = 0; Slot < Taken.Num(); Slot++)
	{
		T.AssertTrue(HOLCampClearing::IsInside(Post, Trees[Taken[Slot]]), "every taken tree is inside the clearing");
	}
}
