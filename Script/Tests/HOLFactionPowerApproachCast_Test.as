// GLORFINDEL-1124e (D900, F3): the pending cast of an approached Sacred Light. The hero walked to the stop point (HOLFactionPowerServer::ApproachPointOf), the cast waits;
// every step PendingStepOf says: nothing pending (0), still walking (1), arrived within the range of the aimed point (2: cast now). Values are COMPUTED by the rule.

FHOLPendingCast MakePending(int Slot, const FVector& Point)
{
	FHOLPendingCast Order;
	Order.Slot = Slot;
	Order.PowerIndex = 0;
	Order.Point = Point;
	return Order;
}

// The hero is still 2000 cm from the point, the range is 1200 cm: the cast waits (step 1), then at the stop point of ApproachPointOf (1150 cm from the point) it is due (step 2).
void Test_FactionApproachPendingWaitsThenCastsOnArrival(FUnitTest& T)
{
	TArray<FHOLPendingCast> Pending;
	const FVector Hero = FVector(0.0, 0.0, 0.0);
	const FVector Point = FVector(2000.0, 0.0, 0.0);
	HOLFactionPowerServer::SetPending(Pending, MakePending(0, Point));
	const int Walking = HOLFactionPowerServer::PendingStepOf(Pending, 0, Hero, 1200.0);
	T.AssertEquals(Walking, 1, "2000 cm from the point with a range of 1200 cm: the hero is still walking");
	const FHOLFactionApproach Walk = HOLFactionPowerServer::ApproachPointOf(Hero, Point, 1200.0);
	const int Arrived = HOLFactionPowerServer::PendingStepOf(Pending, 0, Walk.StopPoint, 1200.0);
	T.AssertEquals(Arrived, 2, "at the stop point the aimed point is in range: the cast is due");
}

// Another slot has nothing pending; a new order of the hero cancels the pending cast (default: any new order cancels).
void Test_FactionApproachPendingIsPerSlotAndCancelledByAnOrder(FUnitTest& T)
{
	TArray<FHOLPendingCast> Pending;
	HOLFactionPowerServer::SetPending(Pending, MakePending(0, FVector(2000.0, 0.0, 0.0)));
	const int Other = HOLFactionPowerServer::PendingStepOf(Pending, 1, FVector(0.0, 0.0, 0.0), 1200.0);
	T.AssertEquals(Other, 0, "slot 1 has no pending cast");
	HOLFactionPowerServer::CancelPending(Pending, 0);
	const int After = HOLFactionPowerServer::PendingStepOf(Pending, 0, FVector(1000.0, 0.0, 0.0), 1200.0);
	T.AssertEquals(After, 0, "a cancelled cast never fires, even once the hero is in range");
}

// A second aimed cast replaces the first (one pending cast per slot): the new point decides the arrival.
void Test_FactionApproachSecondCastReplacesTheFirst(FUnitTest& T)
{
	TArray<FHOLPendingCast> Pending;
	HOLFactionPowerServer::SetPending(Pending, MakePending(0, FVector(2000.0, 0.0, 0.0)));
	HOLFactionPowerServer::SetPending(Pending, MakePending(0, FVector(-5000.0, 0.0, 0.0)));
	T.AssertEquals(Pending.Num(), 1, "one pending cast per slot");
	const int Step = HOLFactionPowerServer::PendingStepOf(Pending, 0, FVector(1000.0, 0.0, 0.0), 1200.0);
	T.AssertEquals(Step, 1, "the first point (2000) would be in range from 1000, the replacing one (-5000) is not");
}

// GLORFINDEL-1124sa (D908): the Honor is controlled at the ORDER. The decision is the rule's (HOLFactionPowerCast::Decide measured from the point itself), never copied.
// GLORFINDEL-1124sb (D911): a refused power says WHY to the player. The notice key comes from the refusal the rule COMPUTED (Decide), never from a number typed here.
void Test_FactionRefusalNoticeNamesTheShortHonor(FUnitTest& T)
{
	const FVector Point = FVector(2000.0, 0.0, 0.0);
	const FHOLFactionPowerDecision Short = HOLFactionPowerServer::ApproachDecisionOf(int(EHOLFaction::DawnOrder), 0, 0, 0, 1000, Point);
	const FString ShortKey = HOLFactionPowerServer::RefusalNoticeKeyOf(Short.Refusal);
	T.AssertEquals("PowerRefusedHonor", ShortKey, "a power refused for a short Honor gauge has the Honor notice");
	const FString ShortText = HOLFactionPowerServer::RefusalNoticeTextOf(Short.Refusal).ToString();
	T.AssertTrue(ShortText.Len() > 0, "the Honor notice has a text in the current language");
	const FHOLFactionPowerDecision Enough = HOLFactionPowerServer::ApproachDecisionOf(int(EHOLFaction::DawnOrder), 0, HOLDawnValues::HonorCap(), 0, 1000, Point);
	const FString EnoughKey = HOLFactionPowerServer::RefusalNoticeKeyOf(Enough.Refusal);
	T.AssertEquals("", EnoughKey, "an accepted power raises no notice");
}

// Honor 0: the order is refused for the gauge reason and the gauge it carries is unchanged (nothing paid).
void Test_FactionApproachOrderRefusedWhenHonorIsShort(FUnitTest& T)
{
	const FHOLPowerRow Book = HOLDawnPowers::Rows()[0];
	const int Now = 1000;
	const FVector Point = FVector(2000.0, 0.0, 0.0);
	const FHOLFactionPowerDecision Order = HOLFactionPowerServer::ApproachDecisionOf(int(EHOLFaction::DawnOrder), 0, 0, 0, Now, Point);
	T.AssertTrue(Book.Cost > 0, "the Sacred Light row costs Honor (else the test proves nothing)");
	T.AssertFalse(Order.bAccepted, "with no Honor the order of an approached cast is refused at once");
	T.AssertEquals(HOLFactionPowerCast::RefusalGaugeShort(), Order.Refusal, "the reason is the short Honor gauge");
	T.AssertEquals(0, Order.NewGaugeValue, "a refused order leaves the gauge untouched");
}

// Honor enough: the order is accepted, the decision carries the price the ARRIVAL will write (the rule's: the row cost), and the same decision is what Decide says.
void Test_FactionApproachOrderAcceptedWhenHonorIsEnough(FUnitTest& T)
{
	const FHOLPowerRow Book = HOLDawnPowers::Rows()[0];
	const int Now = 1000;
	const FVector Point = FVector(2000.0, 0.0, 0.0);
	FHOLFactionResource Gauge = HOLFactionResource::Make(0, 0, HOLDawnValues::HonorCap());
	HOLFactionResource::Gain(Gauge, HOLDawnValues::HonorCap());
	const int Before = Gauge.Value;
	FHOLFactionResource OneSpend = HOLFactionResource::Make(Before, 0, HOLDawnValues::HonorCap());
	HOLFactionResource::TrySpend(OneSpend, Book.Cost);
	const FHOLFactionPowerDecision Order = HOLFactionPowerServer::ApproachDecisionOf(int(EHOLFaction::DawnOrder), 0, Before, 0, Now, Point);
	T.AssertTrue(Order.bAccepted, "with enough Honor the order is accepted");
	T.AssertEquals(OneSpend.Value, Order.NewGaugeValue, "the decision carries the row's price, written only on arrival (D900)");
	T.AssertTrue(Order.NewGaugeValue < Before, "the price is below the full gauge");
}

// GLORFINDEL-1124sc (D909): a new order that names the slot's hero cancels its pending cast; an order of other units does not. Step values are COMPUTED by PendingStepOf.
void Test_FactionHeroOrderCancelsThePendingCast(FUnitTest& T)
{
	const int HeroId = 7;
	TArray<FHOLPendingCast> Pending;
	HOLFactionPowerServer::SetPending(Pending, MakePending(0, FVector(2000.0, 0.0, 0.0)));
	TArray<int> Others;
	Others.Add(3);
	Others.Add(4);
	const bool bOthersCancel = HOLFactionPowerServer::CancelPendingOnOrder(Pending, 0, Others, HeroId);
	T.AssertFalse(bOthersCancel, "an order that does not name the hero cancels nothing");
	const int StillThere = HOLFactionPowerServer::PendingStepOf(Pending, 0, FVector(1000.0, 0.0, 0.0), 1200.0);
	T.AssertEquals(2, StillThere, "the cast still waits (in range from 1000 cm) after an order of other units");
	TArray<int> WithHero = Others;
	WithHero.Add(HeroId);
	const bool bHeroCancels = HOLFactionPowerServer::CancelPendingOnOrder(Pending, 0, WithHero, HeroId);
	T.AssertTrue(bHeroCancels, "an order naming the hero cancels the pending cast");
	const int Gone = HOLFactionPowerServer::PendingStepOf(Pending, 0, FVector(1000.0, 0.0, 0.0), 1200.0);
	T.AssertEquals(0, Gone, "the cancelled cast never fires, even in range");
	const bool bNoHero = HOLFactionPowerServer::CancelPendingOnOrder(Pending, 0, WithHero, -1);
	T.AssertFalse(bNoHero, "no hero (id -1): nothing to cancel");
}

// GLORFINDEL-1124sc (D911): a refusal at the ORDER (short Honor) is raised as a notice for its slot, and read back by TakeNoticeFrom; the key is the rule's.
void Test_FactionRefusalAtTheOrderRaisesANotice(FUnitTest& T)
{
	const FVector Point = FVector(2000.0, 0.0, 0.0);
	const FHOLFactionPowerDecision Short = HOLFactionPowerServer::ApproachDecisionOf(int(EHOLFaction::DawnOrder), 0, 0, 0, 1000, Point);
	TArray<FHOLPendingNotice> Notices;
	const bool bRaised = HOLFactionPowerServer::RaiseRefusalNotice(Notices, 2, Short.Refusal);
	T.AssertTrue(bRaised, "a short Honor at the order raises a notice");
	const int OtherSlot = HOLFactionPowerServer::TakeNoticeFrom(Notices, 1);
	T.AssertEquals(0, OtherSlot, "another slot sees nothing");
	const int Taken = HOLFactionPowerServer::TakeNoticeFrom(Notices, 2);
	T.AssertEquals(Short.Refusal, Taken, "the owner receives the refusal the rule computed");
	const FHOLFactionPowerDecision Enough = HOLFactionPowerServer::ApproachDecisionOf(int(EHOLFaction::DawnOrder), 0, HOLDawnValues::HonorCap(), 0, 1000, Point);
	const bool bAccepted = HOLFactionPowerServer::RaiseRefusalNotice(Notices, 2, Enough.Refusal);
	T.AssertFalse(bAccepted, "an accepted order raises no notice");
	T.AssertEquals(0, Notices.Num(), "nothing queued");
}
