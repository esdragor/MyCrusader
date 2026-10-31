// The fairness of a camp layout between the players (NeutralsInProgress.md N1b, GDD 8.4 and D32: "placed symmetrically"). Pure functions
// of the start positions and the camps' posts, whatever the source of the layout (map recipe, data file or hand-placed actors: V4-Q1 of
// the volcano is open for camps too). For each player the distances to every camp are sorted; a fair layout gives every player the same
// list (the same camps at the same distances, whatever their direction): by rotation for 2 to 8 players placed on a circle, or by mirror
// for 2. The gap is the largest difference found, in cm: 0 for a symmetric layout, and the number to bound in a map test.

namespace HOLCampLayout
{
	float FlatDistance(FVector A, FVector B)
	{
		FVector Line = A - B;
		Line.Z = 0;
		return Line.Size();
	}

	// The distances from a start to every post, ascending.
	TArray<float> SortedDistances(FVector Start, const TArray<FVector>& Posts)
	{
		TArray<float> Distances;
		for (int Index = 0; Index < Posts.Num(); Index++)
		{
			const float Distance = FlatDistance(Start, Posts[Index]);
			int Slot = 0;
			while (Slot < Distances.Num() && Distances[Slot] <= Distance)
			{
				Slot++;
			}
			Distances.Insert(Distance, Slot);
		}
		return Distances;
	}

	// The largest difference, in cm, between two players' sorted distance lists, over every start compared to the first. 0 with fewer than
	// two starts or no post: nothing to compare.
	float EquityGap(const TArray<FVector>& Starts, const TArray<FVector>& Posts)
	{
		float Gap = 0.f;
		if (Starts.Num() < 2 || Posts.Num() == 0)
		{
			return Gap;
		}
		const TArray<float> First = SortedDistances(Starts[0], Posts);
		for (int Index = 1; Index < Starts.Num(); Index++)
		{
			const TArray<float> Other = SortedDistances(Starts[Index], Posts);
			for (int Rank = 0; Rank < First.Num(); Rank++)
			{
				Gap = Math::Max(Gap, Math::Abs(Other[Rank] - First[Rank]));
			}
		}
		return Gap;
	}
}
