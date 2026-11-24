// The match's book of duels (DuelInProgress.md D5), the script side of UHOLDuelSubsystem: the C++ part gives the step
// and the heroes' health and asks, once a combat step, which pairs of heroes may touch each other; the rules are
// HOLDuelRules.as (HOLDuel::BookCanHit, Tick). Server and solo only (the C++ base never calls it on a client).
// Where a target stood when it was challenged or answered (D866 Q2 = A: Hesitation is laid at the refusing hero's position). Slot -1 = none.
struct FHOLDuelRefuserPlace
{
	int Slot = -1;
	int XCm = 0;
	int YCm = 0;
}

namespace HOLDuelLedgerRules
{
	// Notes (or replaces) the place of a slot: one place per slot, the last note counts.
	void NotePlace(TArray<FHOLDuelRefuserPlace>& Places, int Slot, int XCm, int YCm)
	{
		if (Slot < 0)
			return;
		for (int Index = 0; Index < Places.Num(); Index++)
		{
			if (Places[Index].Slot == Slot)
			{
				Places[Index].XCm = XCm;
				Places[Index].YCm = YCm;
				return;
			}
		}
		FHOLDuelRefuserPlace Place;
		Place.Slot = Slot;
		Place.XCm = XCm;
		Place.YCm = YCm;
		Places.Add(Place);
	}

	// The morale orders of the duels that ended: Triumph and Demoralised at the arena's centre, a refusal's Hesitation at its refuser's
	// (the duel's target) noted position; a refuser never noted keeps a placeless order (the drain drops it).
	TArray<FHOLDuelMoraleOrder> OrdersOfEndedPlaced(const FHOLDuelSettings& S, const TArray<FHOLDuelEnded>& Ended, const TArray<FHOLDuelRefuserPlace>& Places)
	{
		TArray<FHOLDuelMoraleOrder> Result;
		for (int Index = 0; Index < Ended.Num(); Index++)
		{
			bool bFound = false;
			int XCm = 0;
			int YCm = 0;
			for (int P = 0; P < Places.Num(); P++)
			{
				if (Places[P].Slot == Ended[Index].Duel.Target)
				{
					bFound = true;
					XCm = Places[P].XCm;
					YCm = Places[P].YCm;
				}
			}
			if (bFound)
				Result.Append(HOLDuelRewards::OrdersAt(S, Ended[Index].Duel, XCm, YCm));
			else
				Result.Append(HOLDuelRewards::Orders(S, Ended[Index].Duel));
		}
		return Result;
	}
}

class UHOLDuelLedger : UHOLDuelSubsystem
{
	FHOLDuelBook Book;
	FHOLDuelSettings Settings;
	bool bReady = false;

	// D859 n. 3: the Honor of the duel, written on the gauge of the Dawn's slots alone (the faction of a slot: its text, the gauge test:
	// HOLFactionResource::HasResource; the rules and the numbers: HOLDuelRewards, HOLDawnValues).
	void ApplyHonorOrders(const TArray<FHOLDuelHonorOrder>& Orders)
	{
		HOLDuelRewards::ApplyHonor(Cast<AHOLGameState>(Gameplay::GetGameState()), Orders);
	}

	void SlotGauges(TArray<bool>& HasGauge, TArray<int>& Current)
	{
		AHOLGameState State = Cast<AHOLGameState>(Gameplay::GetGameState());
		for (int Slot = 0; Slot < Kits.Num(); Slot++)
		{
			HasGauge.Add(State != nullptr && HOLFactionResource::HasResource(EHOLFaction(State.GetSlotFaction(Slot))));
			Current.Add(State != nullptr ? State.GetFactionResource(Slot) : 0);
		}
	}

	// D6: each slot's kit state (posture, parry), reset when its duel begins.
	FHOLDuelKitSettings KitSettings;
	TArray<FHOLDuelKit> Kits;

	// D7: the morale orders of the duels that ended, kept for the bridge to the simulation's morale (MoraleInProgress.md M7, not
	// built yet: nothing applies them to the troops today).
	TArray<FHOLDuelMoraleOrder> PendingMorale;

	// Where each target stood when it was challenged and when it answered (D866 Q2 = A: Hesitation at the refusing hero's position).
	TArray<FHOLDuelRefuserPlace> RefuserPlaces;

	// S2-b: takes the orders the bridge lays now (the ones with a place, a slot and an effect) and empties PendingMorale. An order without
	// a place (a refusal whose hero's position was never noted) is dropped, not guessed.
	TArray<FHOLDuelMoraleOrder> TakePendingOrders()
	{
		TArray<FHOLDuelMoraleOrder> Laid;
		for (int Index = 0; Index < PendingMorale.Num(); Index++)
		{
			if (PendingMorale[Index].bHasCenter && PendingMorale[Index].Slot >= 0 && PendingMorale[Index].EffectId >= 0)
				Laid.Add(PendingMorale[Index]);
		}
		PendingMorale.Empty();
		return Laid;
	}

	// S2-b (PLAN-MORAL-1120 step 6): empties PendingMorale into the simulation's morale (one source: the C++ rule, through the bridge). An order
	// without a place (a refusal: the refusing hero's position is not held by the ledger yet) is dropped, not guessed.
	void ApplyMoraleOrders()
	{
		TArray<FHOLDuelMoraleOrder> Laid = TakePendingOrders();
		for (int Index = 0; Index < Laid.Num(); Index++)
		{
			HOLMorale::ApplyEffectAround(this, Laid[Index].EffectId, Laid[Index].CenterXCm, Laid[Index].CenterYCm, Laid[Index].Slot, Laid[Index].DurationMs, Laid[Index].RadiusCm);
			Log(HOLDuelRewards::DescribeMoraleOrder(Laid[Index]));
		}
	}

	void EnsureBook()
	{
		if (bReady)
			return;
		Settings = HOLDuel::Preset();
		KitSettings = HOLDuelKit::Preset();
		int NumSlots = Math::Max(HOLHero::GetNumSlots(), 8);
		Book = HOLDuel::NewBook(NumSlots);
		for (int Slot = 0; Slot < NumSlots; Slot++)
		{
			Kits.Add(FHOLDuelKit());
		}
		bReady = true;
	}

	// True when the slot is in a duel that is on (Active): the kit exists only then.
	bool InActiveDuel(int Slot)
	{
		for (int Index = 0; Index < Book.Duels.Num(); Index++)
		{
			if (Book.Duels[Index].Phase == EHOLDuelPhase::Active && HOLDuel::InDuel(Book.Duels[Index], Slot))
				return true;
		}
		return false;
	}

	UFUNCTION(BlueprintOverride)
	bool AnyDuelOn()
	{
		EnsureBook();
		for (int Index = 0; Index < Book.Duels.Num(); Index++)
		{
			if (Book.Duels[Index].Phase == EHOLDuelPhase::Active)
				return true;
		}
		return false;
	}

	UFUNCTION(BlueprintOverride)
	bool CanHit(int AttackerHeroSlot, int VictimHeroSlot)
	{
		EnsureBook();
		return HOLDuel::BookCanHit(Book, AttackerHeroSlot, VictimHeroSlot);
	}

	// A challenge accepted at once, the conditions of the challenge not checked (the development door; the command of D4
	// checks them through HOLDuel::Start). A hero already in a duel is refused.
	UFUNCTION(BlueprintOverride)
	bool BeginDuel(int Challenger, int Target, int AX, int AY, int BX, int BY)
	{
		EnsureBook();
		if (Challenger == Target || HOLDuel::BookInDuel(Book, Challenger) || HOLDuel::BookInDuel(Book, Target))
			return false;
		Book.Duels.Add(HOLDuel::Respond(Settings, HOLDuel::Challenge(Settings, Challenger, Target), true));
		HOLDuel::BookSetArena(Book, Challenger, Target, HOLDuel::MakeArena(Settings, AX, AY, BX, BY));
		if (Challenger >= 0 && Challenger < Kits.Num())
			Kits[Challenger] = FHOLDuelKit();
		if (Target >= 0 && Target < Kits.Num())
			Kits[Target] = FHOLDuelKit();
		return true;
	}

	// D4, the challenge command: "" and the challenge Pending, or the reason it cannot be made (HOLDuel::CanChallenge's).
	UFUNCTION(BlueprintOverride)
	FString StartChallenge(int Challenger, int Target, int Facts, int AX, int AY, int BX, int BY, const TArray<int>& ForumXY)
	{
		EnsureBook();
		FString Reason = HOLDuel::BookStartChallenge(Settings, Book, Challenger, Target, Facts, AX, AY, BX, BY, ForumXY);
		// B is the target's position (UHOLDuelSubsystem::MeasureFacts): noted for the Hesitation of a refusal by silence.
		if (Reason.IsEmpty())
			HOLDuelLedgerRules::NotePlace(RefuserPlaces, Target, BX, BY);
		return Reason;
	}

	// D4, the answer command: whether a challenge addressed to the slot was waiting. The conditions are read now (DU-Q2); an
	// acceptance that stands begins the duel with its circle where the heroes are, the kits fresh.
	UFUNCTION(BlueprintOverride)
	bool AnswerChallenge(int Slot, bool bAccept, int Facts, int AX, int AY, int BX, int BY, const TArray<int>& ForumXY)
	{
		EnsureBook();
		int Challenger = -1;
		for (int Index = 0; Index < Book.Duels.Num(); Index++)
		{
			if (Book.Duels[Index].Phase == EHOLDuelPhase::Pending && Book.Duels[Index].Target == Slot)
				Challenger = Book.Duels[Index].Challenger;
		}
		if (!HOLDuel::AnswerChecked(Settings, Book, Slot, bAccept, HOLDuel::ConditionsHold(Settings, Facts, AX, AY, BX, BY, ForumXY)))
			return false;
		// The answerer's position now (B): where its Hesitation is laid if the answer is a refusal.
		HOLDuelLedgerRules::NotePlace(RefuserPlaces, Slot, BX, BY);
		if (HOLDuel::BookInDuel(Book, Slot) && HOLDuel::BookAdversaryOf(Book, Slot) == Challenger)
		{
			HOLDuel::BookSetArena(Book, Challenger, Slot, HOLDuel::MakeArena(Settings, AX, AY, BX, BY));
			if (Challenger >= 0 && Challenger < Kits.Num())
				Kits[Challenger] = FHOLDuelKit();
			if (Slot >= 0 && Slot < Kits.Num())
				Kits[Slot] = FHOLDuelKit();
			// D859 n. 3: the acceptor of a duel that begins gains HonorDuelAccept (the Dawn's slots alone).
			TArray<bool> HasGauge;
			TArray<int> Current;
			SlotGauges(HasGauge, Current);
			TArray<FHOLDuelHonorOrder> Accepted;
			FHOLDuelHonorOrder Order = HOLDuelRewards::HonorOfAccept(Slot, HOLDuelRewards::SlotHas(HasGauge, Slot), HOLDuelRewards::SlotValue(Current, Slot));
			if (Order.Slot >= 0)
				Accepted.Add(Order);
			ApplyHonorOrders(Accepted);
		}
		return true;
	}

	// D4: the challenges waiting, as (challenger, target) pairs: the match reads the conditions again for each (DU-Q2).
	UFUNCTION(BlueprintOverride)
	TArray<int> PendingPairs()
	{
		EnsureBook();
		TArray<int> Pairs;
		for (int Index = 0; Index < Book.Duels.Num(); Index++)
		{
			if (Book.Duels[Index].Phase == EHOLDuelPhase::Pending)
			{
				Pairs.Add(Book.Duels[Index].Challenger);
				Pairs.Add(Book.Duels[Index].Target);
			}
		}
		return Pairs;
	}

	UFUNCTION(BlueprintOverride)
	bool RecheckChallenge(int Target, int Facts, int AX, int AY, int BX, int BY, const TArray<int>& ForumXY)
	{
		EnsureBook();
		return HOLDuel::BookRecheck(Settings, Book, Target, HOLDuel::ConditionsHold(Settings, Facts, AX, AY, BX, BY, ForumXY));
	}

	// D4: the slot's duel as its hero sheet shows it: code (0 none, 1 sent, 2 incoming, 3 on), the other hero, the time left,
	// the circle (centre X, Y, radius).
	UFUNCTION(BlueprintOverride)
	TArray<int> ViewOf(int Slot)
	{
		EnsureBook();
		FHOLDuelView View = HOLDuel::BookViewOf(Book, Slot);
		TArray<int> Out;
		Out.Add(View.Code);
		Out.Add(View.Opponent);
		Out.Add(View.LeftMs);
		Out.Add(View.CenterX);
		Out.Add(View.CenterY);
		Out.Add(View.RadiusCm);
		return Out;
	}

	UFUNCTION(BlueprintOverride)
	int Adversary(int Slot)
	{
		EnsureBook();
		return HOLDuel::BookAdversaryOf(Book, Slot);
	}

	UFUNCTION(BlueprintOverride)
	FVector ArenaOf(int Slot)
	{
		EnsureBook();
		FHOLDuelArena Arena;
		if (!HOLDuel::BookArenaOf(Book, Slot, Arena))
			return FVector(0, 0, -1);
		return FVector(Arena.CenterX, Arena.CenterY, Arena.RadiusCm);
	}

	UFUNCTION(BlueprintOverride)
	bool SetPosture(int Slot, int Posture)
	{
		EnsureBook();
		if (Slot < 0 || Slot >= Kits.Num() || Posture < 0 || Posture > int(EHOLDuelPosture::Defensive) || !InActiveDuel(Slot))
			return false;
		Kits[Slot] = HOLDuelKit::SetPosture(Kits[Slot], EHOLDuelPosture(Posture));
		return true;
	}

	UFUNCTION(BlueprintOverride)
	int DamagePercent(int Slot)
	{
		EnsureBook();
		if (Slot < 0 || Slot >= Kits.Num() || !InActiveDuel(Slot))
			return 100;
		return HOLDuelKit::DamagePercentOf(KitSettings, Kits[Slot].Posture);
	}

	UFUNCTION(BlueprintOverride)
	int AdvanceDuels(int DeltaMs, const TArray<int>& Health)
	{
		EnsureBook();
		for (int Slot = 0; Slot < Kits.Num(); Slot++)
		{
			Kits[Slot] = HOLDuelKit::Tick(Kits[Slot], DeltaMs);
		}
		TArray<FHOLDuelEnded> Ended = HOLDuel::Tick(Settings, Book, DeltaMs, Health);
		PendingMorale.Append(HOLDuelLedgerRules::OrdersOfEndedPlaced(Settings, Ended, RefuserPlaces));
		ApplyMoraleOrders();
		// D859 n. 3: the Honor of the duels that ended, on the gauges of the Dawn's slots alone.
		TArray<bool> HasGauge;
		TArray<int> Current;
		SlotGauges(HasGauge, Current);
		ApplyHonorOrders(HOLDuelRewards::HonorOfEnded(Settings, Ended, HasGauge, Current));
		// D859 n. 9: 150 XP to the winner of each duel that ended.
		HOLDuelRewards::ApplyXP(HOLDuelRewards::XPWinnersOfEnded(Settings, Ended));
		for (int Index = 0; Index < Ended.Num(); Index++)
		{
			Log("Duel ended: slots " + Ended[Index].Duel.Challenger + " and " + Ended[Index].Duel.Target + ", end " + int(Ended[Index].Duel.End) + ", winner " + Ended[Index].Duel.Winner);
		}
		return Ended.Num();
	}
}
