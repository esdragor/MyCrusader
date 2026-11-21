// FARAMIR-1121se: the faction slot (gauge, bottom left) sits over the hero's block, which grows upward with the talent icons row and the
// "Talents taken (N)" line; the slot's rise was a fixed 150 px, so past that height the hero's block covered the gauge (preflight talents_heal,
// "HUD overlap HeroPanel x FactionPanel"). The rule HOLFactionSlotRule::BottomPadding is pure (boxes in, padding out) and read by
// HOLGameScreen::FactionSlotPlace with the hero panel's real box.

// The slot's rectangle (16 to 260 wide, 70 high, bottom edge at Room - padding) and the hero block's rectangle (height Height over the
// bottom left place, Base from the bottom): they never overlap, whatever the hero block's height (0 to 4 talents taken).
void Test_FactionSlotClearsHeroBlock(FUnitTest& T)
{
	const float Room = 1080.0;
	const float Base = 40.0;
	const float Rise = 150.0;
	const float SlotHeight = 70.0;
	TArray<float> Heights;
	Heights.Add(120.0); // no talent taken
	Heights.Add(200.0); // one icons row
	Heights.Add(260.0); // two talents + list line
	Heights.Add(320.0);
	Heights.Add(420.0);
	for (int Index = 0; Index < Heights.Num(); ++Index)
	{
		const float HeroTop = Room - Base - Heights[Index];
		const float Padding = HOLFactionSlotRule::BottomPadding(Base, Rise, Room, HeroTop, true);
		const float SlotBottom = Room - Padding;
		const bool bOverlap = HOLHUDLayoutRead::Overlaps(FVector2D(16.0, SlotBottom - SlotHeight), FVector2D(260.0, SlotBottom),
			FVector2D(16.0, HeroTop), FVector2D(300.0, Room - Base), 1.0f);
		T.AssertFalse(bOverlap, f"hero block {Heights[Index]} px high: the slot does not overlap it");
	}
}

// The hero's block not there (hidden, or no geometry yet): the old place, the bottom left place plus the rise.
void Test_FactionSlotWithoutHeroBlock(FUnitTest& T)
{
	const float Padding = HOLFactionSlotRule::BottomPadding(40.0, 150.0, 1080.0, 0.0, false);
	T.AssertEquals(Padding, 190.0f, "no hero box: the bottom left place plus the rise");
	const float Low = HOLFactionSlotRule::BottomPadding(40.0, 150.0, 1080.0, 1000.0, true);
	T.AssertEquals(Low, 190.0f, "a low hero block (80 px) keeps the rise");
}
