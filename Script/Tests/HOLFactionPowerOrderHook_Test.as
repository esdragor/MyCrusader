// GLORFINDEL-1124se (D909, rest of 1124sd): ToggleDeploy and DropOff are hero-capable player orders too: an order that names the slot's hero cancels his pending cast, an order of
// other units does not. Real chain: the rules HOLPlayerOrders::*For issue their command on a detached queue, then the pending list of the slot is read back through PendingStepOf
// (COMPUTED by the rule). Garrison cannot run detached (TakesGarrisonOrder reads a real building): its hook is the same one-line NoteHeroOrder as Move / Attack / Stop.

FHOLPendingCast OHPending(int Slot, const FVector& Point)
{
	FHOLPendingCast Order;
	Order.Slot = Slot;
	Order.PowerIndex = 0;
	Order.Point = Point;
	return Order;
}

TArray<int> OHUnits(int HeroId, bool bWithHero)
{
	TArray<int> Ids;
	Ids.Add(3);
	Ids.Add(4);
	if (bWithHero)
	{
		Ids.Add(HeroId);
	}
	return Ids;
}

void Test_FactionHeroToggleDeployCancelsThePendingCast(FUnitTest& T)
{
	const int HeroId = 7;
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	TArray<FHOLPendingCast> Pending;
	HOLFactionPowerServer::SetPending(Pending, OHPending(0, FVector(2000.0, 0.0, 0.0)));
	const bool bOthers = HOLPlayerOrders::ToggleDeployFor(Commands, 0, OHUnits(HeroId, false), Pending, HeroId);
	T.AssertTrue(bOthers, "the deploy order of other units is accepted");
	const int Kept = HOLFactionPowerServer::PendingStepOf(Pending, 0, FVector(1000.0, 0.0, 0.0), 1200.0);
	T.AssertEquals(2, Kept, "an order that does not name the hero leaves the pending cast (in range from 1000 cm)");
	const bool bHero = HOLPlayerOrders::ToggleDeployFor(Commands, 0, OHUnits(HeroId, true), Pending, HeroId);
	T.AssertTrue(bHero, "the deploy order naming the hero is accepted");
	const int Gone = HOLFactionPowerServer::PendingStepOf(Pending, 0, FVector(1000.0, 0.0, 0.0), 1200.0);
	T.AssertEquals(0, Gone, "a deploy order naming the hero cancels his pending cast");
}

void Test_FactionHeroDropOffCancelsThePendingCast(FUnitTest& T)
{
	const int HeroId = 7;
	UHOLCommandSubsystem Commands = HOLCommandScript::MakeDetachedForTest();
	AHOLBuilding NoBuilding = nullptr;
	const FVector Spot = FVector(500.0, 0.0, 0.0);
	TArray<FHOLPendingCast> Pending;
	HOLFactionPowerServer::SetPending(Pending, OHPending(0, FVector(2000.0, 0.0, 0.0)));
	const bool bOthers = HOLPlayerOrders::DropOffFor(Commands, 0, OHUnits(HeroId, false), NoBuilding, Spot, false, false, Pending, HeroId);
	T.AssertTrue(bOthers, "the drop-off order of other units is accepted");
	const int Kept = HOLFactionPowerServer::PendingStepOf(Pending, 0, FVector(1000.0, 0.0, 0.0), 1200.0);
	T.AssertEquals(2, Kept, "an order that does not name the hero leaves the pending cast");
	const bool bHero = HOLPlayerOrders::DropOffFor(Commands, 0, OHUnits(HeroId, true), NoBuilding, Spot, false, false, Pending, HeroId);
	T.AssertTrue(bHero, "the drop-off order naming the hero is accepted (not a deposit: all walk to the point)");
	const int Gone = HOLFactionPowerServer::PendingStepOf(Pending, 0, FVector(1000.0, 0.0, 0.0), 1200.0);
	T.AssertEquals(0, Gone, "a drop-off order naming the hero (the !bDeposit walk too) cancels his pending cast");
}
