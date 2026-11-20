// D860 H2 (minimap): the square a received signal draws on the minimap. Pure rule: the same pulse as the ground's mark
// (HOLSignalInputRead::PulseScale, one rule in C++), read on the game's clock, the one HOLSignalInputRead::Expire reads.
// The minimap view centres every square on its Position (HOLMinimapView.cpp: At = Position - Size / 2), so a size that varies keeps its centre.
namespace HOLSignalMarks
{
	// Side of the square at scale 1, px (what the fixed square was).
	const float BaseSize = 10.0;

	// The mark of a signal at world Location (cm), received at ReceivedAt, seen at Now (both seconds of the game's clock).
	FHOLMinimapMark Mark(FHOLMinimapFrame Frame, FVector Location, FLinearColor Color, double ReceivedAt, double Now)
	{
		FHOLMinimapMark Out;
		Out.Position = HOLMinimap::ToPicture(Frame, FVector2D(Location.X, Location.Y));
		Out.Color = Color;
		Out.Size = BaseSize * HOLSignalInputRead::PulseScale(Now - ReceivedAt);
		return Out;
	}
}
