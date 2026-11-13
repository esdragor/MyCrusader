// N7 (NeutralsInProgress.md, Retours\Q-neutrals-camp-indicator.md answered 2026-11-11: Q2 B, Q1 C): a neutral camp's dot on the minimap,
// pure rule. Red while the camp lives, grey once it is cleared, a square a little larger than a building's (6 px) so it reads at a
// glance. Only the camps the player's camp has seen reach the script (the server writes them to his state, UHOLCampSubsystem), so a
// camp never seen has no dot: the fog does not leak (D381 to D383).
namespace HOLCampMarks
{
	// Side of a camp's square, px (a building's is 6, a unit's 3). 2026-11-13: 7 px hid under the 3 px dots of the camp's own
	// guards on the 240 px minimap and its red did not read; 12 px (twice a building's) leaves a red rim around them.
	const float Size = 12.0;

	FLinearColor AliveColor()
	{
		return FLinearColor(0.85, 0.12, 0.10, 1.0);
	}

	FLinearColor ClearedColor()
	{
		return FLinearColor(0.45, 0.45, 0.45, 1.0);
	}

	// The dot of a camp whose post is at World (X, Y cm), alive or cleared.
	FHOLMinimapMark Mark(FHOLMinimapFrame Frame, FVector2D World, bool bCleared)
	{
		FHOLMinimapMark Out;
		Out.Position = HOLMinimap::ToPicture(Frame, World);
		Out.Color = bCleared ? ClearedColor() : AliveColor();
		Out.Size = Size;
		return Out;
	}
}
