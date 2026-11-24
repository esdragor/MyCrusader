// The morale rewards of a duel's end, as a pure rule (DuelInProgress.md D7, GDD 10.5, D16, D35): from the final state of a
// duel, the list of morale effects to pose. Nothing is applied here: the match applies the orders to the troops (the bridge
// to the simulation's morale is MoraleInProgress.md M7). The XP of the duel (150, D859 n. 9) is XPWinnersOfEnded / ApplyXP below. Not here either: the
// faction's effect (Honour for the Dawn).

// One morale effect to pose: on the troops around the hero of the slot, for DurationMs, within RadiusCm.
struct FHOLDuelMoraleOrder
{
	int Slot = -1;
	int EffectId = -1;
	int DurationMs = 0;
	int RadiusCm = 0;
	// Where the effect is posed (D866 Q2 = A): the centre of the duel's arena for Triumph and Demoralised, the refusing hero's position for
	// Hesitation. bHasCenter is false when the rule was not given a place (a refusal without the hero's position).
	bool bHasCenter = false;
	int CenterXCm = 0;
	int CenterYCm = 0;
}

// One change of a slot's faction gauge (the Dawn's Honor): Delta is the REAL change (the cap and the floor already applied by
// HOLFactionResource::Gain / Lose), NewValue the gauge afterwards. Slot -1 = no order.
struct FHOLDuelHonorOrder
{
	int Slot = -1;
	int Delta = 0;
	int NewValue = 0;
}

namespace HOLDuelRewards
{
	// One journal line for a morale order a duel lays (F5, ARWEN-1124e): "Duel morale: <effect> slot S at (X, Y) radius R ms D".
	FString DescribeMoraleOrder(const FHOLDuelMoraleOrder& Order)
	{
		FString Name = "Effect " + Order.EffectId;
		if (Order.EffectId == HOLMoraleTable::Triumph)
			Name = "Triumph";
		else if (Order.EffectId == HOLMoraleTable::Demoralised)
			Name = "Demoralised";
		else if (Order.EffectId == HOLMoraleTable::Hesitation)
			Name = "Hesitation";
		return "Duel morale: " + Name + " slot " + Order.Slot + " at (" + Order.CenterXCm + ", " + Order.CenterYCm + ") radius " + Order.RadiusCm + " ms " + Order.DurationMs;
	}

	// The orders of the duel's end: Triumph for the winner and Demoralised for the loser; Hesitation on the one who refuses
	// (a duel refused or ignored, when the refusal has its cost) and on nobody else; nothing for a double knock-out, a time
	// up or a duel not over. The durations and the radius (12 m, D859 n. 8) are the morale table's.
	FHOLDuelMoraleOrder MakeOrder(int Slot, int EffectId)
	{
		FHOLDuelMoraleOrder Order;
		Order.Slot = Slot;
		Order.EffectId = EffectId;
		Order.DurationMs = HOLMoraleTable::Row(EffectId).DurationMs;
		Order.RadiusCm = HOLMoraleTable::Row(EffectId).RadiusCm;
		return Order;
	}

	FHOLDuelMoraleOrder PlacedAt(const FHOLDuelMoraleOrder& In, int XCm, int YCm)
	{
		// A parameter is not an l-value in script: work on a local copy.
		FHOLDuelMoraleOrder Order = In;
		Order.bHasCenter = true;
		Order.CenterXCm = XCm;
		Order.CenterYCm = YCm;
		return Order;
	}

	// The shared rule: Triumph and Demoralised at the arena's centre (when the duel has its circle), Hesitation at the refuser's
	// position when the caller has it (bHasRefuser).
	TArray<FHOLDuelMoraleOrder> PlacedOrders(const FHOLDuelSettings& S, const FHOLDuel& Duel, bool bHasRefuser, int RefuserXCm, int RefuserYCm)
	{
		TArray<FHOLDuelMoraleOrder> Result;
		// The duel rule says who wins, who loses and who pays for a refusal; nothing is read while the duel is not over.
		FHOLDuelEffects E = HOLDuel::Effects(S, Duel);
		if (E.TriumphSlot >= 0)
		{
			FHOLDuelMoraleOrder Order = MakeOrder(E.TriumphSlot, HOLMoraleTable::Triumph);
			if (Duel.bHasArena)
				Order = PlacedAt(Order, Duel.Arena.CenterX, Duel.Arena.CenterY);
			Result.Add(Order);
		}
		if (E.DemoralisedSlot >= 0)
		{
			FHOLDuelMoraleOrder Order = MakeOrder(E.DemoralisedSlot, HOLMoraleTable::Demoralised);
			if (Duel.bHasArena)
				Order = PlacedAt(Order, Duel.Arena.CenterX, Duel.Arena.CenterY);
			Result.Add(Order);
		}
		if (E.bRefusalCost && E.HesitationSlot >= 0)
		{
			FHOLDuelMoraleOrder Order = MakeOrder(E.HesitationSlot, HOLMoraleTable::Hesitation);
			if (bHasRefuser)
				Order = PlacedAt(Order, RefuserXCm, RefuserYCm);
			Result.Add(Order);
		}
		return Result;
	}

	TArray<FHOLDuelMoraleOrder> Orders(const FHOLDuelSettings& S, const FHOLDuel& Duel)
	{
		return PlacedOrders(S, Duel, false, 0, 0);
	}

	// The orders of the duel's end with their place (D866 Q2 = A): Triumph and Demoralised at the arena's centre, Hesitation at the refuser's position.
	TArray<FHOLDuelMoraleOrder> OrdersAt(const FHOLDuelSettings& S, const FHOLDuel& Duel, int RefuserXCm, int RefuserYCm)
	{
		return PlacedOrders(S, Duel, true, RefuserXCm, RefuserYCm);
	}

	// The Honor of the duel (D859 n. 3, pure): one gauge change, from the slot's gauge now, through the gauge chain (cap 100, floor 0).
	// bGain: Gain(Amount) else Lose(Amount). A slot without a gauge (not the Dawn, bHasGauge computed by the caller) gets no order.
	FHOLDuelHonorOrder HonorChange(int Slot, bool bHasGauge, int Current, bool bGain, int Amount)
	{
		FHOLDuelHonorOrder Order;
		if (Slot < 0 || !bHasGauge)
			return Order;
		FHOLFactionResource Gauge = HOLFactionResource::Make(Current, 0, HOLDawnValues::HonorCap());
		if (bGain)
			HOLFactionResource::Gain(Gauge, Amount);
		else
			HOLFactionResource::Lose(Gauge, Amount);
		Order.Slot = Slot;
		Order.Delta = Gauge.Value - Math::Clamp(Current, 0, HOLDawnValues::HonorCap());
		Order.NewValue = Gauge.Value;
		return Order;
	}

	bool SlotHas(const TArray<bool>& HasGauge, int Slot)
	{
		return Slot >= 0 && Slot < HasGauge.Num() && HasGauge[Slot];
	}

	int SlotValue(const TArray<int>& Values, int Slot)
	{
		return (Slot >= 0 && Slot < Values.Num()) ? Values[Slot] : 0;
	}

	// The acceptance: the one who accepts gains HonorDuelAccept.
	FHOLDuelHonorOrder HonorOfAccept(int Slot, bool bHasGauge, int Current)
	{
		return HonorChange(Slot, bHasGauge, Current, true, HOLDawnValues::HonorDuelAccept());
	}

	// The Honor of every duel that ended in one step, in the order of the list: the winner gains HonorDuelWin (bFactionEffect), the
	// refuser who pays loses HonorDuelRefuse (bRefusalCost). A running copy of the gauges carries the result from one order to the next.
	TArray<FHOLDuelHonorOrder> HonorOfEnded(const FHOLDuelSettings& S, const TArray<FHOLDuelEnded>& Ended, const TArray<bool>& HasGauge, const TArray<int>& Current)
	{
		TArray<FHOLDuelHonorOrder> Result;
		TArray<int> Running = Current;
		for (int Index = 0; Index < Ended.Num(); Index++)
		{
			FHOLDuelEffects E = HOLDuel::Effects(S, Ended[Index].Duel);
			FHOLDuelHonorOrder Order;
			if (E.bFactionEffect)
				Order = HonorChange(E.TriumphSlot, SlotHas(HasGauge, E.TriumphSlot), SlotValue(Running, E.TriumphSlot), true, HOLDawnValues::HonorDuelWin());
			else if (E.bRefusalCost)
				Order = HonorChange(E.HesitationSlot, SlotHas(HasGauge, E.HesitationSlot), SlotValue(Running, E.HesitationSlot), false, HOLDawnValues::HonorDuelRefuse());
			if (Order.Slot >= 0)
			{
				if (Order.Slot < Running.Num())
					Running[Order.Slot] = Order.NewValue;
				Result.Add(Order);
			}
		}
		return Result;
	}

	// Writes the orders to the gauges (authority). Returns how many were written.
	int ApplyHonor(AHOLGameState State, const TArray<FHOLDuelHonorOrder>& Orders)
	{
		if (State == nullptr)
			return 0;
		for (int Index = 0; Index < Orders.Num(); Index++)
		{
			State.SetFactionResource(Orders[Index].Slot, Orders[Index].NewValue);
		}
		return Orders.Num();
	}

	// The XP of the duels won (D859 n. 9): the winner's slot of every duel that ended in one step and whose effects carry the XP bonus,
	// in the order of the list. The 150 is C++ (HOLHeroXP::DuelXP, UHOLHeroSubsystem::AwardDuelXP).
	TArray<int> XPWinnersOfEnded(const FHOLDuelSettings& S, const TArray<FHOLDuelEnded>& Ended)
	{
		TArray<int> Result;
		for (int Index = 0; Index < Ended.Num(); Index++)
		{
			FHOLDuelEffects E = HOLDuel::Effects(S, Ended[Index].Duel);
			if (E.bXpBonus && E.TriumphSlot >= 0)
				Result.Add(E.TriumphSlot);
		}
		return Result;
	}

	// Gives each winner his duel XP (authority). Returns how many were given.
	int ApplyXP(const TArray<int>& Slots)
	{
		UHOLHeroSubsystem Heroes = UHOLHeroSubsystem::Get();
		if (Heroes == nullptr)
			return 0;
		for (int Index = 0; Index < Slots.Num(); Index++)
		{
			Heroes.AwardDuelXP(Slots[Index]);
		}
		return Slots.Num();
	}
	// The orders of every duel that ended in one step (what HOLDuel::Tick hands back), in the order of the list: the ledger keeps
	// them in PendingMorale until the bridge to the simulation's morale applies them (MoraleInProgress.md M7).
	TArray<FHOLDuelMoraleOrder> OrdersOfEnded(const FHOLDuelSettings& S, const TArray<FHOLDuelEnded>& Ended)
	{
		TArray<FHOLDuelMoraleOrder> Result;
		for (int Index = 0; Index < Ended.Num(); Index++)
		{
			Result.Append(Orders(S, Ended[Index].Duel));
		}
		return Result;
	}
}
