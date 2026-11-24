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
// D866 Q2 = A, GDD 10.1 (D16): a refusal lays Hesitation at the refusing hero's position (the target's, noted when it was challenged and
// again when it answered: the last one counts), a won duel keeps the arena's centre; a refuser whose position was never noted has no place.
void Test_LedgerPlacesHesitationAtTheRefusersPosition(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	FHOLDuel Won = HOLDuel::Respond(S, HOLDuel::Challenge(S, 0, 1), true);
	Won.bHasArena = true;
	Won.Arena = HOLDuel::MakeArena(S, 1000, 2000, 1400, 2400);
	Won = HOLDuel::Advance(S, Won, 3000, 0, 40);
	FHOLDuel Refused = HOLDuel::Respond(S, HOLDuel::Challenge(S, 2, 3), false);
	FHOLDuel RefusedUnknown = HOLDuel::Respond(S, HOLDuel::Challenge(S, 4, 5), false);
	TArray<FHOLDuelEnded> Ended;
	FHOLDuelEnded First;
	First.Duel = Won;
	First.Effects = HOLDuel::Effects(S, Won);
	Ended.Add(First);
	FHOLDuelEnded Second;
	Second.Duel = Refused;
	Second.Effects = HOLDuel::Effects(S, Refused);
	Ended.Add(Second);
	FHOLDuelEnded Third;
	Third.Duel = RefusedUnknown;
	Third.Effects = HOLDuel::Effects(S, RefusedUnknown);
	Ended.Add(Third);
	TArray<FHOLDuelRefuserPlace> Places;
	HOLDuelLedgerRules::NotePlace(Places, 3, 777, -555);
	HOLDuelLedgerRules::NotePlace(Places, 3, 800, -600);
	T.AssertEquals(1, Places.Num(), "a slot has one place: the last note replaces the first");
	TArray<FHOLDuelMoraleOrder> Orders = HOLDuelLedgerRules::OrdersOfEndedPlaced(S, Ended, Places);
	T.AssertEquals(4, Orders.Num(), "Triumph, Demoralised and two Hesitations");
	int Hes = LedgerFindOrder(Orders, HOLMoraleTable::Hesitation, 3);
	int Unknown = LedgerFindOrder(Orders, HOLMoraleTable::Hesitation, 5);
	int Win = LedgerFindOrder(Orders, HOLMoraleTable::Triumph, 1);
	T.AssertTrue(Hes >= 0, "Hesitation on the refuser");
	T.AssertTrue(Unknown >= 0, "Hesitation on the second refuser");
	T.AssertTrue(Win >= 0, "Triumph kept");
	if (Hes >= 0)
	{
		T.AssertTrue(Orders[Hes].bHasCenter, "Hesitation has a place");
		T.AssertEquals(800, Orders[Hes].CenterXCm, "x = the refuser's last noted position");
		T.AssertEquals(-600, Orders[Hes].CenterYCm, "y = the refuser's last noted position");
	}
	if (Unknown >= 0)
		T.AssertFalse(Orders[Unknown].bHasCenter, "a refuser never noted has no place (the drain drops it)");
	if (Win >= 0)
		T.AssertEquals(Won.Arena.CenterX, Orders[Win].CenterXCm, "a won duel keeps the arena's centre");
}

// F5 (ARWEN-1124e): the morale a duel lays is readable in the journal, one line per order, from the real orders of the real rule: a won duel
// (Triumph for the winner's camp, Demoralised for the loser's, at the arena's centre) and a refusal (Hesitation at the refuser's place).
void Test_LedgerDescribesEachMoraleOrder(FUnitTest& T)
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
	TArray<FHOLDuelMoraleOrder> Won = HOLDuelRewards::OrdersOfEnded(S, Ended);
	FHOLDuel Refused = HOLDuel::Respond(S, HOLDuel::Challenge(S, 2, 3), false);
	TArray<FHOLDuelMoraleOrder> Refusal = HOLDuelRewards::OrdersAt(S, Refused, 800, -600);
	int Win = LedgerFindOrder(Won, HOLMoraleTable::Triumph, 1);
	int Lose = LedgerFindOrder(Won, HOLMoraleTable::Demoralised, 0);
	int Hes = LedgerFindOrder(Refusal, HOLMoraleTable::Hesitation, 3);
	T.AssertTrue(Win >= 0 && Lose >= 0 && Hes >= 0, "the real rule gives Triumph, Demoralised and Hesitation orders");
	if (Win < 0 || Lose < 0 || Hes < 0)
		return;
	FString TriumphLine = HOLDuelRewards::DescribeMoraleOrder(Won[Win]);
	FString DemoralisedLine = HOLDuelRewards::DescribeMoraleOrder(Won[Lose]);
	FString HesitationLine = HOLDuelRewards::DescribeMoraleOrder(Refusal[Hes]);
	FString Centre = "at (" + D.Arena.CenterX + ", " + D.Arena.CenterY + ")";
	T.AssertTrue(TriumphLine.StartsWith("Duel morale: Triumph slot 1 "), "Triumph line names the effect and the winner's slot: " + TriumphLine);
	T.AssertTrue(TriumphLine.Contains(Centre), "Triumph line gives the arena's centre: " + TriumphLine);
	T.AssertTrue(TriumphLine.Contains("radius " + Won[Win].RadiusCm + " ms " + Won[Win].DurationMs), "Triumph line gives the radius and the duration: " + TriumphLine);
	T.AssertTrue(DemoralisedLine.StartsWith("Duel morale: Demoralised slot 0 "), "Demoralised line names the effect and the loser's slot: " + DemoralisedLine);
	T.AssertTrue(DemoralisedLine.Contains(Centre), "Demoralised line gives the arena's centre: " + DemoralisedLine);
	T.AssertTrue(HesitationLine.StartsWith("Duel morale: Hesitation slot 3 "), "Hesitation line names the effect and the refuser's slot: " + HesitationLine);
	T.AssertTrue(HesitationLine.Contains("at (800, -600)"), "Hesitation line gives the refuser's place: " + HesitationLine);
}