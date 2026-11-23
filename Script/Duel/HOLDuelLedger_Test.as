// The ledger's morale drain (S2-b, GALADRIEL-1123pc): what ApplyMoraleOrders hands to the bridge (HOLMorale::ApplyEffectAround) is
// TakePendingOrders; the call itself reaches the C++ queue (covered by HeroesOfLiberty.Morale.ApplyEffectAroundLaysDuelOrders).

int LedgerFindOrder(const TArray<FHOLDuelMoraleOrder>& Orders, int EffectId, int Slot)
{
	for (int Index = 0; Index < Orders.Num(); Index++)
	{
		if (Orders[Index].EffectId == EffectId && Orders[Index].Slot == Slot)
			return Index;
	}
	return -1;
}

// A duel won at the arena's centre C (slot 1 wins, slot 0 falls), played by the real rule: after the drain PendingMorale is empty and
// the orders laid are exactly Triumph for the winner's camp and Demoralised for the loser's camp, at C. An order without a place and an
// empty order are not laid.
void Test_LedgerDrainLaysTriumphAndDemoralisedAtTheCentre(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	FHOLDuel D = HOLDuel::Respond(S, HOLDuel::Challenge(S, 0, 1), true);
	D.bHasArena = true;
	D.Arena = HOLDuel::MakeArena(S, 1000, 2000, 1400, 2400);
	D = HOLDuel::Advance(S, D, 3000, 0, 40);
	FHOLDuelEnded Done;
	Done.Duel = D;
	Done.Effects = HOLDuel::Effects(S, D);
	TArray<FHOLDuelEnded> Ended;
	Ended.Add(Done);
	UHOLDuelLedger Ledger = Cast<UHOLDuelLedger>(NewObject(GetTransientPackage(), UHOLDuelLedger::StaticClass()));
	Ledger.PendingMorale.Append(HOLDuelRewards::OrdersOfEnded(S, Ended));
	Ledger.PendingMorale.Add(FHOLDuelMoraleOrder());
	FHOLDuelMoraleOrder NoPlace = HOLDuelRewards::MakeOrder(2, HOLMoraleTable::Hesitation);
	Ledger.PendingMorale.Add(NoPlace);
	TArray<FHOLDuelMoraleOrder> Laid = Ledger.TakePendingOrders();
	int PendingLeft = Ledger.PendingMorale.Num();
	T.AssertEquals(0, PendingLeft, "the drain empties PendingMorale");
	T.AssertEquals(2, Laid.Num(), "exactly two orders laid: the empty one and the one without a place are dropped");
	int Win = LedgerFindOrder(Laid, HOLMoraleTable::Triumph, 1);
	int Lose = LedgerFindOrder(Laid, HOLMoraleTable::Demoralised, 0);
	T.AssertTrue(Win >= 0, "Triumph for the winner's slot");
	T.AssertTrue(Lose >= 0, "Demoralised for the loser's slot");
	if (Win >= 0 && Lose >= 0)
	{
		T.AssertTrue(Laid[Win].bHasCenter, "Triumph has its place");
		T.AssertEquals(D.Arena.CenterX, Laid[Win].CenterXCm, "Triumph at the centre, x");
		T.AssertEquals(D.Arena.CenterY, Laid[Win].CenterYCm, "Triumph at the centre, y");
		T.AssertEquals(D.Arena.CenterX, Laid[Lose].CenterXCm, "Demoralised at the centre, x");
		T.AssertEquals(D.Arena.CenterY, Laid[Lose].CenterYCm, "Demoralised at the centre, y");
		T.AssertEquals(HOLMoraleTable::Row(HOLMoraleTable::Triumph).RadiusCm, Laid[Win].RadiusCm, "the table's radius");
	}
	TArray<FHOLDuelMoraleOrder> Again = Ledger.TakePendingOrders();
	T.AssertEquals(0, Again.Num(), "a second drain lays nothing: no order twice");
}