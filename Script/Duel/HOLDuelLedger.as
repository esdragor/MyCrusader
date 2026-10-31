// The match's book of duels (DuelInProgress.md D5), the script side of UHOLDuelSubsystem: the C++ part gives the step
// and the heroes' health and asks, once a combat step, which pairs of heroes may touch each other; the rules are
// HOLDuelRules.as (HOLDuel::BookCanHit, Tick). Server and solo only (the C++ base never calls it on a client).
class UHOLDuelLedger : UHOLDuelSubsystem
{
	FHOLDuelBook Book;
	FHOLDuelSettings Settings;
	bool bReady = false;

	// D6: each slot's kit state (posture, parry), reset when its duel begins.
	FHOLDuelKitSettings KitSettings;
	TArray<FHOLDuelKit> Kits;

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
	bool BeginDuel(int Challenger, int Target)
	{
		EnsureBook();
		if (Challenger == Target || HOLDuel::BookInDuel(Book, Challenger) || HOLDuel::BookInDuel(Book, Target))
			return false;
		Book.Duels.Add(HOLDuel::Respond(Settings, HOLDuel::Challenge(Settings, Challenger, Target), true));
		if (Challenger >= 0 && Challenger < Kits.Num())
			Kits[Challenger] = FHOLDuelKit();
		if (Target >= 0 && Target < Kits.Num())
			Kits[Target] = FHOLDuelKit();
		return true;
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
		for (int Index = 0; Index < Ended.Num(); Index++)
		{
			Log("Duel ended: slots " + Ended[Index].Duel.Challenger + " and " + Ended[Index].Duel.Target + ", end " + int(Ended[Index].Duel.End) + ", winner " + Ended[Index].Duel.Winner);
		}
		return Ended.Num();
	}
}
