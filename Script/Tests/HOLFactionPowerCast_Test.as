// FARAMIR-1120c: the faction power cast rule on its chain (HOLFactionPowerCast::Decide over HOLFactionResource, HOLDawnPowers,
// HOLGroundAbility). Every expected value is computed by those rules (the cost from the power book row, the gauge from the gauge
// rule, the cool-down end from CooldownEndMs), never copied. D838: a script test. D862: the Seal costs NO Honor (a Monk ability, not a book power). RED on the first commit (Decide still spends the book row 0 cost). The real placing (StartLightSeal) and the RPC need a world: not covered here.

namespace FactionPowerCastTest
{
	FHOLFactionResource FullGauge()
	{
		FHOLFactionResource Gauge = HOLFactionResource::Make(0, 0, HOLDawnValues::HonorCap());
		HOLFactionResource::Gain(Gauge, HOLDawnValues::HonorCap());
		return Gauge;
	}

	int Dawn()
	{
		return int(EHOLFaction::DawnOrder);
	}
}

void Test_FactionPowerCastChain(FUnitTest& T)
{
	// S1-b: the cast is the book's Sacred Light (the Seal is the Monk's ability, cast through ServerCastUnitAbility, D862): row cost and row cool-down.
	const FHOLPowerRow Book = HOLDawnPowers::Rows()[0];
	const int Cooldown = Book.CooldownMs;
	FHOLFactionResource Gauge = FactionPowerCastTest::FullGauge();
	const int Before = Gauge.Value;
	FHOLFactionResource OneSpend = FactionPowerCastTest::FullGauge();
	HOLFactionResource::TrySpend(OneSpend, Book.Cost);
	const int Now = 1000;

	// First cast: in range (the point sits exactly at the range), ready, funded.
	const int Range = int(HOLFactionPowerCast::SealRangeCm());
	FHOLFactionPowerDecision First = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), 0, Gauge, 0, Now, 0, 0, Range, 0);
	T.AssertTrue(First.bAccepted, "a ready, funded, in-range cast of the Dawn is accepted");
	T.AssertEquals(HOLFactionPowerCast::RefusalNone(), First.Refusal, "an accepted cast has no refusal");
	T.AssertEquals(OneSpend.Value, First.NewGaugeValue, "S1-b: the cast pays the row's Honor cost");
	T.AssertTrue(First.NewGaugeValue < Before, "the gauge went down");
	T.AssertEquals(HOLGroundAbility::CooldownEndMs(Now, Cooldown), First.NewCooldownEndMs, "the cool-down ends one Seal cool-down after the cast");

	// The same cast again, written back: refused while the cool-down runs, accepted the instant it ends (and pays again).
	FHOLFactionResource After = HOLFactionResource::Make(First.NewGaugeValue, 0, HOLDawnValues::HonorCap());
	FHOLFactionPowerDecision Again = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), 0, After, First.NewCooldownEndMs, First.NewCooldownEndMs - 1, 0, 0, 0, 0);
	T.AssertFalse(Again.bAccepted, "a second cast one millisecond before the cool-down ends is refused");
	T.AssertEquals(HOLFactionPowerCast::RefusalCooldown(), Again.Refusal, "the reason is the cool-down");
	T.AssertEquals(First.NewGaugeValue, Again.NewGaugeValue, "a refused cast leaves the gauge untouched");
	FHOLFactionPowerDecision Later = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), 0, After, First.NewCooldownEndMs, First.NewCooldownEndMs, 0, 0, 0, 0);
	T.AssertTrue(Later.bAccepted, "the cast is accepted again when the cool-down has ended");
	HOLFactionResource::TrySpend(OneSpend, Book.Cost);
	T.AssertEquals(OneSpend.Value, Later.NewGaugeValue, "and pays the Honor cost again the second time");
}

void Test_FactionPowerCastRefusals(FUnitTest& T)
{
	FHOLFactionResource Full = FactionPowerCastTest::FullGauge();
	const int Range = int(HOLFactionPowerCast::SealRangeCm());

	FHOLFactionPowerDecision Far = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), 0, Full, 0, 0, 100, 0, 100 + Range + 1, 0);
	T.AssertFalse(Far.bAccepted, "a point one centimetre beyond the range is refused");
	T.AssertEquals(HOLFactionPowerCast::RefusalOutOfRange(), Far.Refusal, "the reason is the range");
	T.AssertEquals(Full.Value, Far.NewGaugeValue, "an out-of-range cast leaves the gauge untouched");

	FHOLFactionResource Empty = HOLFactionResource::Make(0, 0, HOLDawnValues::HonorCap());
	FHOLFactionPowerDecision Broke = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), 0, Empty, 0, 0, 0, 0, 0, 0);
	T.AssertFalse(Broke.bAccepted, "S1-b: an empty gauge refuses the Sacred Light (the row has an Honor cost)");
	T.AssertEquals(HOLFactionPowerCast::RefusalGaugeShort(), Broke.Refusal, "the reason is the gauge, checked last");
	T.AssertEquals(Empty.Value, Broke.NewGaugeValue, "an empty gauge stays as it was");

	const int Cost = HOLDawnPowers::Rows()[0].Cost;
	FHOLFactionResource Part = HOLFactionResource::Make(FactionPowerCastTest::FullGauge().Value / 2, 0, HOLDawnValues::HonorCap());
	FHOLFactionResource PartAfter = HOLFactionResource::Make(Part.Value, 0, HOLDawnValues::HonorCap());
	HOLFactionResource::TrySpend(PartAfter, Cost);
	FHOLFactionPowerDecision Half = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), 0, Part, 0, 0, 0, 0, 0, 0);
	T.AssertTrue(Half.bAccepted, "a half-filled gauge pays the cost and casts");
	T.AssertEquals(PartAfter.Value, Half.NewGaugeValue, "and loses exactly the row's Honor cost");

	FHOLFactionResource JustShort = HOLFactionResource::Make(Cost - 1, 0, HOLDawnValues::HonorCap());
	FHOLFactionPowerDecision Short = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), 0, JustShort, 0, 0, 0, 0, 0, 0);
	T.AssertEquals(HOLFactionPowerCast::RefusalGaugeShort(), Short.Refusal, "one Honor short of the cost is refused");

	FHOLFactionPowerDecision Other = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn() + 1, 0, Full, 0, 0, 0, 0, 0, 0);
	T.AssertFalse(Other.bAccepted, "a slot that is not the Dawn is refused");
	T.AssertEquals(HOLFactionPowerCast::RefusalNotDawn(), Other.Refusal, "the reason is the faction");

	FHOLFactionPowerDecision NoPower = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), HOLDawnPowers::Rows().Num(), Full, 0, 0, 0, 0, 0, 0);
	T.AssertEquals(HOLFactionPowerCast::RefusalNoSuchPower(), NoPower.Refusal, "a power beyond the book is refused");
}


// D862: the Seal is the Monk ability of the unit ability table (row LightSeal): its range and its cool-down are that row's, and the cast leaves the Honor gauge alone.
void Test_FactionPowerCastIsTheMonkAbilityRow(FUnitTest& T)
{
	HOLUnitAbilityTable::FHOLUnitAbilityRow Row = HOLUnitAbilityTable::Row(HOLUnitAbilityTable::LightSeal);
	FHOLFactionResource Full = FactionPowerCastTest::FullGauge();
	const int Now = 5000;
	FHOLFactionPowerDecision AtRange = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), 0, Full, 0, Now, 0, 0, int(Row.RangeCm), 0);
	T.AssertTrue(AtRange.bAccepted, "a point at the Monk row's range is accepted");
	const FHOLPowerRow Book = HOLDawnPowers::Rows()[0];
	FHOLFactionResource Spent = FactionPowerCastTest::FullGauge();
	HOLFactionResource::TrySpend(Spent, Book.Cost);
	T.AssertEquals(Spent.Value, AtRange.NewGaugeValue, "S1-b: the Sacred Light pays the book row's Honor");
	T.AssertEquals(HOLGroundAbility::CooldownEndMs(Now, Book.CooldownMs), AtRange.NewCooldownEndMs, "the cool-down is the book row's (the Monk row's belongs to ServerCastUnitAbility)");
	FHOLFactionPowerDecision PastRange = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), 0, Full, 0, Now, 0, 0, int(Row.RangeCm) + 1, 0);
	T.AssertEquals(HOLFactionPowerCast::RefusalOutOfRange(), PastRange.Refusal, "one centimetre past the Monk row's range is refused");
}

// S1-b (GLORFINDEL-1123c): the faction power is the book's Sacred Light line, NOT the Seal: it pays the row's Honor (25 of a full gauge), writes the ROW's
// cool-down (20 s, not the Seal's 40 s), and a gauge short of the cost refuses it without writing anything. Every value is computed by the rule chain.
void Test_CastPaysHonorAndHealsNotSeal(FUnitTest& T)
{
	TArray<FHOLPowerRow> Rows = HOLDawnPowers::Rows();
	T.AssertTrue(Rows.Num() > 0, "the Dawn book has the Sacred Light row");
	const FHOLPowerRow Row = Rows[0];
	FHOLFactionResource Full = FactionPowerCastTest::FullGauge();
	const int Now = 1000;
	FHOLFactionPowerDecision Paid = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), 0, Full, 0, Now, 0, 0, 0, 0);
	T.AssertTrue(Paid.bAccepted, "a funded, ready, in-range cast of the Sacred Light is accepted");
	FHOLFactionResource Expected = FactionPowerCastTest::FullGauge();
	const bool bSpent = HOLFactionResource::TrySpend(Expected, Row.Cost);
	T.AssertTrue(bSpent, "the full gauge can pay the row's cost");
	T.AssertEquals(Expected.Value, Paid.NewGaugeValue, "the cast pays the row's Honor cost (gauge after TrySpend)");
	T.AssertTrue(Paid.NewGaugeValue < Full.Value, "the Honor gauge went down");
	T.AssertEquals(HOLGroundAbility::CooldownEndMs(Now, Row.CooldownMs), Paid.NewCooldownEndMs, "the cool-down written is the book row's, not the Seal's");
	T.AssertTrue(Paid.NewCooldownEndMs != HOLGroundAbility::CooldownEndMs(Now, HOLFactionPowerCast::SealCooldownMs()), "it is not the Monk Seal's cool-down");

	FHOLFactionResource Empty = HOLFactionResource::Make(0, 0, HOLDawnValues::HonorCap());
	FHOLFactionPowerDecision Broke = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), 0, Empty, 777, Now, 0, 0, 0, 0);
	T.AssertFalse(Broke.bAccepted, "a gauge short of the cost refuses the Sacred Light");
	T.AssertEquals(HOLFactionPowerCast::RefusalGaugeShort(), Broke.Refusal, "the reason is the gauge");
	T.AssertEquals(Empty.Value, Broke.NewGaugeValue, "a refused cast leaves the gauge untouched");
	T.AssertEquals(777, Broke.NewCooldownEndMs, "a refused cast leaves the cool-down untouched");
}