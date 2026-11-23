// GLORFINDEL-1123a (S1-d): the Monk's Seal of Light cast rule on its chain (HOLMonkSeal::Decide over the HOLUnitAbilityTable LightSeal row and
// HOLGroundAbility). D862: range 8 m FROM THE MONK, cool-down 40 s, no Honor. D838: a script test. Expected values are computed by the table row
// and the ground rule, never typed from the rule under test. RED on the first commit (Decide is a stub that refuses everything).

void Test_MonkSealAcceptsInRangeFromTheMonk(FUnitTest& T)
{
	const int Dawn = int(EHOLFaction::DawnOrder);
	const int Now = 5000;
	const int Range = int(HOLMonkSeal::RangeCm());
	// The Monk stands far from the origin: the range is read from HIM, not from the hero banner / origin.
	const int MonkX = 30000;
	const int MonkY = -12000;
	FHOLMonkSealDecision AtEdge = HOLMonkSeal::Decide(Dawn, 0, Now, MonkX, MonkY, MonkX + Range, MonkY);
	T.AssertTrue(AtEdge.bAccepted, "a point exactly at the range edge from the Monk is accepted");
	T.AssertEquals(HOLMonkSeal::RefusalNone(), AtEdge.Refusal, "an accepted cast has no refusal");
	const int ExpectedEnd = Now + HOLUnitAbilityTable::Row(HOLUnitAbilityTable::LightSeal).CooldownMs;
	T.AssertEquals(ExpectedEnd, AtEdge.NewCooldownEndMs, "the cool-down ends one LightSeal row cool-down after the cast");
	// D862 pins on the row itself: 8 m, 40 s.
	T.AssertEquals(800, Range, "D862: the Seal reaches 8 m from the Monk");
	T.AssertEquals(40000, HOLMonkSeal::CooldownMs(), "D862: the Seal recharges in 40 s");
}

void Test_MonkSealRefusals(FUnitTest& T)
{
	const int Dawn = int(EHOLFaction::DawnOrder);
	const int Range = int(HOLMonkSeal::RangeCm());
	FHOLMonkSealDecision Far = HOLMonkSeal::Decide(Dawn, 0, 0, 100, 0, 100 + Range + 1, 0);
	T.AssertFalse(Far.bAccepted, "a point one centimetre past the range from the Monk is refused");
	T.AssertEquals(HOLMonkSeal::RefusalOutOfRange(), Far.Refusal, "the reason is the range");
	T.AssertEquals(0, Far.NewCooldownEndMs, "a refused cast leaves the cool-down as it was");

	// Not the Dawn: the Seal is the Dawn Monk's ability.
	FHOLMonkSealDecision Other = HOLMonkSeal::Decide(Dawn + 1, 0, 0, 0, 0, 0, 0);
	T.AssertFalse(Other.bAccepted, "another faction cannot cast the Seal");
	T.AssertEquals(HOLMonkSeal::RefusalNotDawn(), Other.Refusal, "the reason is the faction");

	// The cool-down: refused one millisecond before its end, accepted at its end (written back from a first cast).
	FHOLMonkSealDecision First = HOLMonkSeal::Decide(Dawn, 0, 1000, 0, 0, 0, 0);
	T.AssertTrue(First.bAccepted, "a ready Dawn Monk casts the Seal on his own spot");
	FHOLMonkSealDecision Again = HOLMonkSeal::Decide(Dawn, First.NewCooldownEndMs, First.NewCooldownEndMs - 1, 0, 0, 0, 0);
	T.AssertFalse(Again.bAccepted, "a second cast one millisecond before the cool-down ends is refused");
	T.AssertEquals(HOLMonkSeal::RefusalCooldown(), Again.Refusal, "the reason is the cool-down");
	FHOLMonkSealDecision Later = HOLMonkSeal::Decide(Dawn, First.NewCooldownEndMs, First.NewCooldownEndMs, 0, 0, 0, 0);
	T.AssertTrue(Later.bAccepted, "the cast is accepted again when the cool-down has ended");
}
