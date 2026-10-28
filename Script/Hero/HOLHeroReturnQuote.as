// The hero's return quoted (J8a.5 loop 1b): what the hero's block of the HUD shows during the death, on every machine. For
// an order sent at step Step, taken by the queue at the next step (TakeStep = Step + 1, J8a.4 loop 4), so priced as
// UHOLHeroReturnRule.Buy will price it: HOLHeroReturn::PriceAt(Step + 1, ...). Never a guessed price: a client knows its
// own death step and gold only (-1 for the others), and no server step before its reception clock's first sample.
namespace HOLHeroReturnQuote
{
	// The quote from what is known (pure, no world): Step the order's step (-1 unknown), State and Level the public sheet's,
	// DeathStep the private one's (-1 unknown), Gold the slot's (-1 unknown or no stock), bHasPlace a forum to come back to
	// (HOLHeroReturn::ReturnPlace), bDefeated the slot out of the match.
	FHOLHeroReturnQuote Compute(int Step, EHOLHeroState State, int DeathStep, int Level, int Gold, bool bHasPlace, bool bDefeated)
	{
		FHOLHeroReturnQuote Quote;
		Quote.Gold = Gold;
		Quote.bHasPlace = bHasPlace;
		if (State == EHOLHeroState::Alive)
		{
			Quote.State = EHOLHeroReturnQuoteState::Alive;
			return Quote;
		}
		// Dead and its death step and the server step known, or nothing said (as Buy refuses a hero not dead).
		if ((State != EHOLHeroState::Dead && State != EHOLHeroState::ReadyToReturn) || DeathStep < 0 || Step < 0)
			return Quote;
		Quote.TakeStep = Step + 1;
		Quote.StepsToReady = Math::Max(0, HOLHeroReturn::ReadyStep(DeathStep, Level) - Quote.TakeStep);
		Quote.SecondsToReady = Math::IntegerDivisionTrunc(Quote.StepsToReady + HOLHeroReturn::StepsPerSecond - 1, HOLHeroReturn::StepsPerSecond);
		if (!HOLHeroReturn::CanBuyReturn(Quote.TakeStep, DeathStep, Level))
		{
			Quote.State = EHOLHeroReturnQuoteState::Waiting;
			return Quote;
		}
		Quote.State = EHOLHeroReturnQuoteState::Ready;
		Quote.Price = HOLHeroReturn::PriceAt(Quote.TakeStep, DeathStep, Level);
		Quote.bCanBuy = !bDefeated && bHasPlace && Gold >= 0 && Gold >= Quote.Price;
		return Quote;
	}

	// The slot's return place (HOLHeroReturn::ReturnPlace on its forums): PlaceMainForum, PlaceNone or an other forum's index.
	int PlaceOf(int Slot)
	{
		return HOLHeroReturn::ReturnPlace(HOLHero::IsMainForumStanding(Slot), HOLHero::GetMainForumPosition(Slot),
			HOLHero::GetOtherForumPositions(Slot));
	}

	// The slot's quote for an order sent at step Step, from the bridges (the true sheets on the server, the copies on a client).
	FHOLHeroReturnQuote Read(int Slot, int Step)
	{
		return Compute(Step, HOLHero::GetHeroState(Slot), HOLHero::GetHeroDeathStep(Slot), HOLHero::GetHeroLevel(Slot),
			HOLHero::GetSlotGold(Slot), PlaceOf(Slot) != HOLHeroReturn::PlaceNone, HOLHero::IsSlotDefeated(Slot));
	}

	// The slot's quote for an order sent now (HOLHero::GetServerStep: the reception clock's estimate on a client).
	FHOLHeroReturnQuote ReadNow(int Slot)
	{
		return Read(Slot, HOLHero::GetServerStep());
	}
}
