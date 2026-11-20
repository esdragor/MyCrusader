// FARAMIR-1120c: the faction power cast rule on its chain (HOLFactionPowerCast::Decide over HOLFactionResource, HOLDawnPowers,
// HOLGroundAbility). Every expected value is computed by those rules (the cost from the power book row, the gauge from the gauge
// rule, the cool-down end from CooldownEndMs), never copied. D838: a script test. RED on the first commit (Decide is a stub that
// always refuses "not Dawn"). The real placing (StartLightSeal) and the RPC need a world: not covered here.

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
	TArray<FHOLPowerRow> Rows = HOLDawnPowers::Rows();
	const int Cost = Rows[0].Cost;
	const int Cooldown = HOLFactionPowerCast::SealCooldownMs();
	FHOLFactionResource Gauge = FactionPowerCastTest::FullGauge();
	const int Before = Gauge.Value;
	const int Now = 1000;

	// First cast: in range (the point sits exactly at the range), ready, funded.
	const int Range = int(HOLFactionPowerCast::SealRangeCm());
	FHOLFactionPowerDecision First = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), 0, Gauge, 0, Now, 0, 0, Range, 0);
	T.AssertTrue(First.bAccepted, "a ready, funded, in-range cast of the Dawn is accepted");
	T.AssertEquals(HOLFactionPowerCast::RefusalNone(), First.Refusal, "an accepted cast has no refusal");
	T.AssertEquals(Before - Cost, First.NewGaugeValue, "the gauge pays the power book's cost");
	T.AssertEquals(HOLGroundAbility::CooldownEndMs(Now, Cooldown), First.NewCooldownEndMs, "the cool-down ends one Seal cool-down after the cast");

	// The same cast again, written back: refused while the cool-down runs, accepted the instant it ends (and pays again).
	FHOLFactionResource After = HOLFactionResource::Make(First.NewGaugeValue, 0, HOLDawnValues::HonorCap());
	FHOLFactionPowerDecision Again = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), 0, After, First.NewCooldownEndMs, First.NewCooldownEndMs - 1, 0, 0, 0, 0);
	T.AssertFalse(Again.bAccepted, "a second cast one millisecond before the cool-down ends is refused");
	T.AssertEquals(HOLFactionPowerCast::RefusalCooldown(), Again.Refusal, "the reason is the cool-down");
	T.AssertEquals(First.NewGaugeValue, Again.NewGaugeValue, "a refused cast leaves the gauge untouched");
	FHOLFactionPowerDecision Later = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), 0, After, First.NewCooldownEndMs, First.NewCooldownEndMs, 0, 0, 0, 0);
	T.AssertTrue(Later.bAccepted, "the cast is accepted again when the cool-down has ended");
	T.AssertEquals(First.NewGaugeValue - Cost, Later.NewGaugeValue, "and pays the cost a second time");
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
	T.AssertFalse(Broke.bAccepted, "an empty gauge refuses the cast");
	T.AssertEquals(HOLFactionPowerCast::RefusalGaugeShort(), Broke.Refusal, "the reason is the gauge");
	T.AssertEquals(0, Broke.NewGaugeValue, "an empty gauge stays empty");

	const int Cost = HOLDawnPowers::Rows()[0].Cost;
	FHOLFactionResource Short = HOLFactionResource::Make(Cost - 1, 0, HOLDawnValues::HonorCap());
	FHOLFactionPowerDecision OneShort = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), 0, Short, 0, 0, 0, 0, 0, 0);
	T.AssertEquals(HOLFactionPowerCast::RefusalGaugeShort(), OneShort.Refusal, "one Honor short of the cost is refused");

	FHOLFactionResource Exact = HOLFactionResource::Make(Cost, 0, HOLDawnValues::HonorCap());
	FHOLFactionPowerDecision Just = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), 0, Exact, 0, 0, 0, 0, 0, 0);
	T.AssertTrue(Just.bAccepted, "exactly the cost is enough");
	T.AssertEquals(0, Just.NewGaugeValue, "and empties the gauge");

	FHOLFactionPowerDecision Other = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn() + 1, 0, Full, 0, 0, 0, 0, 0, 0);
	T.AssertFalse(Other.bAccepted, "a slot that is not the Dawn is refused");
	T.AssertEquals(HOLFactionPowerCast::RefusalNotDawn(), Other.Refusal, "the reason is the faction");

	FHOLFactionPowerDecision NoPower = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), HOLDawnPowers::Rows().Num(), Full, 0, 0, 0, 0, 0, 0);
	T.AssertEquals(HOLFactionPowerCast::RefusalNoSuchPower(), NoPower.Refusal, "a power beyond the book is refused");
}
