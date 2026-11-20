// Who receives a morale effect (HOLMoraleRecipients.as, M4 / M7, D705 and D709). The effect is read on the real chain: the effect of the REAL table row
// (HOLMoraleTable::Effect) is posed by HOLMorale::Apply on the state of each recipient and read back by HOLMorale::Total; no number by hand.

FHOLMoraleUnit MoraleRecipientUnit(int Slot, bool bPeasant, bool bHostile, bool bNeutral, float XMeters)
{
	FHOLMoraleUnit Unit;
	Unit.Slot = Slot;
	Unit.bPeasant = bPeasant;
	Unit.bHostileToSource = bHostile;
	Unit.bNeutral = bNeutral;
	Unit.XCm = XMeters * 100.f;
	return Unit;
}

// The total of a lever on a unit after the effect is posed on it if it is in the recipients (a unit not in the list keeps an empty state).
int MoraleRecipientTotal(const TArray<int>& Recipients, int UnitIndex, int EffectId, int Lever)
{
	HOLMorale::FHOLMoraleState State;
	if (Recipients.Contains(UnitIndex))
	{
		HOLMorale::Apply(State, HOLMoraleTable::Effect(EffectId));
	}
	return HOLMorale::Total(State, Lever);
}

// A duel won: the winner's troops within the table's radius get Triumph, one 14 m away does not, a Peasant never, the loser's troops never.
void Test_MoraleRecipientsDuelWinnerWithinRadius(FUnitTest& T)
{
	TArray<FHOLMoraleUnit> Units;
	Units.Add(MoraleRecipientUnit(0, false, false, false, 10.f)); // 0 the winner's soldier, 10 m
	Units.Add(MoraleRecipientUnit(0, false, false, false, 14.f)); // 1 the winner's soldier, 14 m
	Units.Add(MoraleRecipientUnit(0, true, false, false, 5.f)); // 2 the winner's Peasant
	Units.Add(MoraleRecipientUnit(1, false, true, false, 10.f)); // 3 the loser's soldier
	const int RadiusCm = HOLMoraleTable::Row(HOLMoraleTable::Triumph).RadiusCm;
	const TArray<int> Recipients = HOLMoraleRecipients::AroundOrder(0, 0.f, 0.f, RadiusCm, Units);
	const int Near = MoraleRecipientTotal(Recipients, 0, HOLMoraleTable::Triumph, HOLMorale::LeverDealt);
	const int Far = MoraleRecipientTotal(Recipients, 1, HOLMoraleTable::Triumph, HOLMorale::LeverDealt);
	const int Peasant = MoraleRecipientTotal(Recipients, 2, HOLMoraleTable::Triumph, HOLMorale::LeverDealt);
	const int Loser = MoraleRecipientTotal(Recipients, 3, HOLMoraleTable::Triumph, HOLMorale::LeverDealt);
	T.AssertTrue(Near > 0, "the winner's soldier at 10 m deals more after Triumph");
	T.AssertEquals(0, Far, "the soldier at 14 m, outside the radius, deals as before");
	T.AssertEquals(0, Peasant, "a Peasant never receives");
	T.AssertEquals(0, Loser, "the loser's troops do not receive the winner's Triumph");
}

// A duel lost: Demoralised goes to the loser's camp (the order's slot), a malus on the dealt lever; the winner's troops at the same distance keep theirs.
void Test_MoraleRecipientsDemoralisedOnTheLosersCamp(FUnitTest& T)
{
	TArray<FHOLMoraleUnit> Units;
	Units.Add(MoraleRecipientUnit(1, false, false, false, 10.f)); // 0 the loser's soldier
	Units.Add(MoraleRecipientUnit(0, false, true, false, 10.f)); // 1 the winner's soldier
	const int RadiusCm = HOLMoraleTable::Row(HOLMoraleTable::Demoralised).RadiusCm;
	const TArray<int> Recipients = HOLMoraleRecipients::AroundOrder(1, 0.f, 0.f, RadiusCm, Units);
	const int Loser = MoraleRecipientTotal(Recipients, 0, HOLMoraleTable::Demoralised, HOLMorale::LeverDealt);
	const int Winner = MoraleRecipientTotal(Recipients, 1, HOLMoraleTable::Demoralised, HOLMorale::LeverDealt);
	T.AssertTrue(Loser < 0, "the loser's soldier deals less after Demoralised");
	T.AssertEquals(0, Winner, "the winner's soldier is not touched");
}

// An aura's bonus goes to the owner alone: not to the ally of the diplomacy, not to the adversary, not to a Peasant.
void Test_MoraleRecipientsAuraBonusOwnerOnly(FUnitTest& T)
{
	TArray<FHOLMoraleUnit> Units;
	Units.Add(MoraleRecipientUnit(0, false, false, false, 1.f)); // 0 the owner's soldier
	Units.Add(MoraleRecipientUnit(2, false, false, false, 1.f)); // 1 an ally's soldier
	Units.Add(MoraleRecipientUnit(1, false, true, false, 1.f)); // 2 an adversary
	Units.Add(MoraleRecipientUnit(0, true, false, false, 1.f)); // 3 the owner's Peasant
	TArray<int> Recipients;
	for (int Index = 0; Index < Units.Num(); Index++)
	{
		if (HOLMoraleRecipients::ReceivesFromSource(false, 0, Units[Index]))
		{
			Recipients.Add(Index);
		}
	}
	T.AssertTrue(MoraleRecipientTotal(Recipients, 0, HOLMoraleTable::DawnAura, HOLMorale::LeverTaken) > 0, "the owner's soldier is covered by the aura");
	T.AssertEquals(0, MoraleRecipientTotal(Recipients, 1, HOLMoraleTable::DawnAura, HOLMorale::LeverTaken), "an ally is not covered");
	T.AssertEquals(0, MoraleRecipientTotal(Recipients, 2, HOLMoraleTable::DawnAura, HOLMorale::LeverTaken), "an adversary is not covered");
	T.AssertEquals(0, MoraleRecipientTotal(Recipients, 3, HOLMoraleTable::DawnAura, HOLMorale::LeverTaken), "a Peasant is not covered (D705)");
}

// A malus from an aura or a zone touches every adversary and the camp's neutral, never the owner, never an ally, never a Peasant.
void Test_MoraleRecipientsMalusHitsAdversariesAndNeutral(FUnitTest& T)
{
	TArray<FHOLMoraleUnit> Units;
	Units.Add(MoraleRecipientUnit(1, false, true, false, 1.f)); // 0 an adversary
	Units.Add(MoraleRecipientUnit(-1, false, false, true, 1.f)); // 1 the camp's neutral
	Units.Add(MoraleRecipientUnit(0, false, false, false, 1.f)); // 2 the source's owner
	Units.Add(MoraleRecipientUnit(2, false, false, false, 1.f)); // 3 an ally
	Units.Add(MoraleRecipientUnit(1, true, true, false, 1.f)); // 4 an adversary's Peasant
	TArray<int> Recipients;
	for (int Index = 0; Index < Units.Num(); Index++)
	{
		if (HOLMoraleRecipients::ReceivesFromSource(true, 0, Units[Index]))
		{
			Recipients.Add(Index);
		}
	}
	T.AssertTrue(MoraleRecipientTotal(Recipients, 0, HOLMoraleTable::Demoralised, HOLMorale::LeverDealt) < 0, "an adversary receives the malus");
	T.AssertTrue(MoraleRecipientTotal(Recipients, 1, HOLMoraleTable::Demoralised, HOLMorale::LeverDealt) < 0, "the neutral receives it too (D401)");
	T.AssertEquals(0, MoraleRecipientTotal(Recipients, 2, HOLMoraleTable::Demoralised, HOLMorale::LeverDealt), "the owner does not");
	T.AssertEquals(0, MoraleRecipientTotal(Recipients, 3, HOLMoraleTable::Demoralised, HOLMorale::LeverDealt), "an ally does not");
	T.AssertEquals(0, MoraleRecipientTotal(Recipients, 4, HOLMoraleTable::Demoralised, HOLMorale::LeverDealt), "a Peasant never");
}
