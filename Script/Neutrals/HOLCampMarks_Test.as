// N7: a neutral camp's dot on the minimap (HOLCampMarks.as; Retours\Q-neutrals-camp-indicator.md, Q2 B: red alive, grey cleared).

FHOLMinimapFrame CampMarksFrame()
{
	return HOLMinimap::MakeFrame(-40000.0, -40000.0, 40000.0, 40000.0, 200.0);
}

void Test_CampMarkAliveIsRed(FUnitTest& T)
{
	FHOLMinimapMark M = HOLCampMarks::Mark(CampMarksFrame(), FVector2D(0.0, 0.0), false);
	T.AssertTrue(M.Color.R > 0.6 && M.Color.G < 0.3 && M.Color.B < 0.3, f"a living camp is red (got {M.Color.R} {M.Color.G} {M.Color.B})");
}

void Test_CampMarkClearedIsGrey(FUnitTest& T)
{
	FHOLMinimapMark M = HOLCampMarks::Mark(CampMarksFrame(), FVector2D(0.0, 0.0), true);
	T.AssertTrue(M.Color.R < 0.8 && M.Color.R > 0.2, f"a cleared camp is mid grey (got R {M.Color.R})");
	T.AssertTrue(Math::Abs(M.Color.R - M.Color.G) < 0.05 && Math::Abs(M.Color.G - M.Color.B) < 0.05, "a cleared camp has no hue");
	FHOLMinimapMark Alive = HOLCampMarks::Mark(CampMarksFrame(), FVector2D(0.0, 0.0), false);
	T.AssertFalse(Alive.Color.G == M.Color.G && Alive.Color.R == M.Color.R, "alive and cleared differ");
}

void Test_CampMarkSitsOnThePost(FUnitTest& T)
{
	FHOLMinimapFrame F = CampMarksFrame();
	FVector2D Post(10000.0, 2000.0);
	FHOLMinimapMark M = HOLCampMarks::Mark(F, Post, false);
	FVector2D Expected = HOLMinimap::ToPicture(F, Post);
	T.AssertTrue(Math::Abs(M.Position.X - Expected.X) < 0.01 && Math::Abs(M.Position.Y - Expected.Y) < 0.01, f"the dot is at the post's picture point ({M.Position.X}, {M.Position.Y}) vs ({Expected.X}, {Expected.Y})");
	T.AssertTrue(M.Size > 6.0, f"a camp's dot is larger than a building's 6 px (got {M.Size})");
}
