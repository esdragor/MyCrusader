// The Dawn's Sacred Light from a full Honor bar (D859, step 1b; HOLDawnPowers.as): the number of casts is CALCULATED by the real
// chain (HOLFactionResource::Make / TrySpend, HOLPowerBook::StateOf, HOLGroundAbility::CooldownEndMs), never copied.

// Full bar (the cap of the source): each cast spends the row's cost while the book says Ready, a fifth cast is refused.
void Test_SacredLightFromFullBar(FUnitTest& T)
{
	TArray<FHOLPowerRow> Rows = HOLDawnPowers::Rows();
	T.AssertEquals(1, Rows.Num(), "one power in the Dawn's book");
	FHOLPowerRow Row = Rows[0];
	T.AssertEquals("SacredLight", Row.Key, "the power is the Sacred Light");
	// The book shows it greyed while its effect is not coded; the cost rule is tested as if it were.
	Row.bEffectCoded = true;

	FHOLFactionResource Honor = HOLFactionResource::Make(HOLDawnValues::HonorCap(), 0, HOLDawnValues::HonorCap());
	int Casts = 0;
	// Bounded loop: a power that costs nothing must not hang the test.
	for (int Try = 0; Try < 10; Try++)
	{
		if (HOLPowerBook::StateOf(Row, Honor.Value, 0, 0) != EHOLPowerState::Ready)
		{
			break;
		}
		if (!HOLFactionResource::TrySpend(Honor, Row.Cost))
		{
			break;
		}
		Casts++;
	}
	T.AssertEquals(4, Casts, "a full bar of 100 pays four Sacred Lights of 25");
	T.AssertFalse(HOLFactionResource::TrySpend(Honor, Row.Cost), "the fifth cast is refused");
	T.AssertEquals(0, Honor.Value, "the bar is empty, the refused cast left it untouched");
	T.AssertTrue(HOLPowerBook::StateOf(Row, Honor.Value, 0, 0) == EHOLPowerState::ShortOfResource, "the book says short of Honor");
}

// Right after a cast the book says Cooling for the row's cool-down (20 s): usable again at cast + 20 s.
void Test_SacredLightCooldown(FUnitTest& T)
{
	FHOLPowerRow Row = HOLDawnPowers::Rows()[0];
	Row.bEffectCoded = true;
	const int CooldownEnd = HOLGroundAbility::CooldownEndMs(5000, Row.CooldownMs);
	T.AssertEquals(25000, CooldownEnd, "cast at 5 s: usable again at 25 s");
	T.AssertTrue(HOLPowerBook::StateOf(Row, 100, CooldownEnd - 6000, 0) == EHOLPowerState::Cooling, "at 6 s the book says Cooling");
	T.AssertTrue(HOLPowerBook::StateOf(Row, 100, 0, 0) == EHOLPowerState::Ready, "at the end of the cool-down it is Ready");
}

// D890: the Sacred Light has its OWN range of 1200 cm (12 m) in its row of the Dawn's table; the Seal keeps the 800 cm of the LightSeal
// row of the unit ability table. The cast rule (HOLFactionPowerCast::Decide) reads the row: 1100 and 1200 cm accepted, 1300 refused.
void Test_SacredLightRangeIs1200(FUnitTest& T)
{
	T.AssertEquals(1200, HOLDawnPowers::SacredLightRangeCm(), "the Sacred Light has its own range of 12 m");
	FHOLFactionResource Full = HOLFactionResource::Make(HOLDawnValues::HonorCap(), 0, HOLDawnValues::HonorCap());
	FHOLFactionPowerDecision Near = HOLFactionPowerCast::Decide(int(EHOLFaction::DawnOrder), 0, Full, 0, 0, 0, 0, 1100, 0);
	T.AssertTrue(Near.bAccepted, "a cast 11 m from the origin is accepted");
	FHOLFactionPowerDecision Edge = HOLFactionPowerCast::Decide(int(EHOLFaction::DawnOrder), 0, Full, 0, 0, 0, 0, 1200, 0);
	T.AssertTrue(Edge.bAccepted, "a cast exactly 12 m from the origin is accepted");
	FHOLFactionPowerDecision Far = HOLFactionPowerCast::Decide(int(EHOLFaction::DawnOrder), 0, Full, 0, 0, 0, 0, 1300, 0);
	T.AssertFalse(Far.bAccepted, "a cast 13 m from the origin is refused");
	T.AssertEquals(HOLFactionPowerCast::RefusalOutOfRange(), Far.Refusal, "the reason is the range");
	T.AssertEquals(800, int(HOLFactionPowerCast::SealRangeCm()), "the Seal keeps its own 800 cm");
}
