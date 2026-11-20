// The clearing of a neutral camp (HALDIR-1120pe, scenario neutrals_clairiere): nothing grows within the clearing radius of a camp's post.
// Pure rule, tested alone in script (HOLCampClearing_Test.as). The scenario's tree that stayed in front of the camp was a WOOD SOURCE
// (the game's own trees, AHOLSourceField), which hol.Decor.ClearNear leaves alone by design (HOLDecorClear.cpp, the `continue` on AHOLSourceField in CountNear and in ClearNear);
// this rule says which trees must go, whoever draws them. Nothing here touches the world.

namespace HOLCampClearing
{
	// The clearing around the post, in cm (25 m: the scenario neutrals_clairiere asks for 25 m of bare ground). Provisional.
	float RadiusCm()
	{
		return 2500.f;
	}

	// Is the point (ground distance, height ignored) inside the clearing of the post? The edge counts as inside.
	bool IsInside(FVector Post, FVector Point)
	{
		const float Radius = RadiusCm();
		const float DX = Point.X - Post.X;
		const float DY = Point.Y - Post.Y;
		return DX * DX + DY * DY <= Radius * Radius;
	}

	// The indices of the trees (their ground positions) that stand in the clearing of the post, in the list's order.
	TArray<int> TreesToClear(FVector Post, const TArray<FVector>& Trees)
	{
		TArray<int> Taken;
		for (int Index = 0; Index < Trees.Num(); Index++)
		{
			if (IsInside(Post, Trees[Index]))
			{
				Taken.Add(Index);
			}
		}
		return Taken;
	}
}
