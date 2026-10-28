// The hero's return decision (J8a.4 loop 3, J8a4-Q6 = A): at the end of each simulation step, where the simulation runs
// (solo, server; the C++ base never calls it on a client), each slot still in the match whose hero is dead:
// 1. ready once the delay is over (Dead -> ReadyToReturn, the copies written at once);
// 2. back by itself once the price reaches 0 (J8a4-Q1, Q4, Q5: players, AI and absent players alike), by its main
//    forum if it stands, else by its other forum nearest to it, else not yet: asked again at each next step (a forum
//    rebuilt brings him back).
// The paid return (the player's order, loop 4) goes through the same rules (HOLHeroReturn::CanBuyReturn, PriceAt).
class UHOLHeroReturnRule : UHOLHeroReturnSubsystem
{
	UFUNCTION(BlueprintOverride)
	bool Decide(int Step)
	{
		for (int Slot = 0; Slot < HOLHero::GetNumSlots(); Slot++)
		{
			DecideSlot(Slot, Step);
		}
		return true;
	}

	// The player's paid return (J8a.4 loop 4): its order taken from the queue at the end of step Step (the same step as
	// Decide's, before it), priced at that step. Refused, with nothing taken: a defeated slot, a hero not dead, before the
	// ready step, no place to come back to; then HOLHero::BuyHeroReturn takes the gold and asks the birth in one block
	// (not enough gold, no stock, a return already asked this step: refused there, nothing taken).
	UFUNCTION(BlueprintOverride)
	bool Buy(int Slot, int Step)
	{
		if (HOLHero::IsSlotDefeated(Slot))
			return Refuse(Slot, Step, "slot defeated");
		const EHOLHeroState State = HOLHero::GetHeroState(Slot);
		const int DeathStep = HOLHero::GetHeroDeathStep(Slot);
		if ((State != EHOLHeroState::Dead && State != EHOLHeroState::ReadyToReturn) || DeathStep < 0)
			return Refuse(Slot, Step, "hero not dead");
		const int Level = HOLHero::GetHeroLevel(Slot);
		if (!HOLHeroReturn::CanBuyReturn(Step, DeathStep, Level))
			return Refuse(Slot, Step, "not ready (ready at step " + HOLHeroReturn::ReadyStep(DeathStep, Level) + ")");
		const int Place = HOLHeroReturn::ReturnPlace(HOLHero::IsMainForumStanding(Slot), HOLHero::GetMainForumPosition(Slot),
			HOLHero::GetOtherForumPositions(Slot));
		if (Place == HOLHeroReturn::PlaceNone)
			return Refuse(Slot, Step, "no forum to come back to");
		const int Price = HOLHeroReturn::PriceAt(Step, DeathStep, Level);
		if (!HOLHero::BuyHeroReturn(Slot, Place, Price))
			return Refuse(Slot, Step, "not paid or not born (price " + Price + ")");
		return true;
	}

	bool Refuse(int Slot, int Step, FString Reason)
	{
		Log("Hero return refused: slot " + Slot + " at step " + Step + ": " + Reason);
		return false;
	}

	void DecideSlot(int Slot, int Step)
	{
		if (HOLHero::IsSlotDefeated(Slot))
			return;
		const EHOLHeroState State = HOLHero::GetHeroState(Slot);
		if (State != EHOLHeroState::Dead && State != EHOLHeroState::ReadyToReturn)
			return;
		const int DeathStep = HOLHero::GetHeroDeathStep(Slot);
		if (DeathStep < 0)
			return;
		const int Level = HOLHero::GetHeroLevel(Slot);

		if (State == EHOLHeroState::Dead && HOLHeroReturn::IsReady(Step, DeathStep, Level))
			HOLHero::SetHeroReturnReady(Slot);

		if (!HOLHeroReturn::ShouldAutoReturn(Step, DeathStep, Level))
			return;
		const int Place = HOLHeroReturn::ReturnPlace(HOLHero::IsMainForumStanding(Slot), HOLHero::GetMainForumPosition(Slot),
			HOLHero::GetOtherForumPositions(Slot));
		if (Place == HOLHeroReturn::PlaceNone)
			return;
		HOLHero::RequestHeroReturn(Slot, Place);
	}
}
