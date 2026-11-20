// Who receives a morale effect (MoraleInProgress.md M4 / M7, D705 and D709): a pure rule, no unit, no world. The caller (the duel's bridge, later the
// effects processor) hands the units as plain records; the rule answers which of them are touched.

// One unit as the rule sees it. Slot: its owner; bHostileToSource: the caller's answer from the diplomacy (not an ally of the source, not the source);
// bNeutral: a camp's neutral (D401); X / Y in cm.
struct FHOLMoraleUnit
{
	int Slot = -1;
	bool bPeasant = false;
	bool bHostileToSource = false;
	bool bNeutral = false;
	float XCm = 0.f;
	float YCm = 0.f;
}

namespace HOLMoraleRecipients
{
	// An effect of an aura or a zone of the slot SourceSlot (D705): a bonus goes to the owner alone (not to the allies of the diplomacy); a malus goes to
	// every adversary, the camp's neutral included; a Peasant never.
	bool ReceivesFromSource(bool bMalus, int SourceSlot, const FHOLMoraleUnit& Unit)
	{
		if (Unit.bPeasant)
		{
			return false;
		}
		if (bMalus)
		{
			return Unit.bNeutral || Unit.bHostileToSource;
		}
		return Unit.Slot == SourceSlot && !Unit.bNeutral;
	}

	// An effect of a duel's end (D709): the troops of the camp the order names, a bonus or a malus alike (Triumph to the winner, Demoralised to the loser,
	// Hesitation to the refuser's camp); a Peasant never.
	bool ReceivesFromOrder(int OrderSlot, const FHOLMoraleUnit& Unit)
	{
		return !Unit.bPeasant && !Unit.bNeutral && Unit.Slot == OrderSlot;
	}

	// The indices of the units of the order's camp within RadiusCm of (CenterXCm, CenterYCm), in the list's order: a snapshot taken at the duel's end (a
	// unit that arrives afterwards is not in the list the caller passes, so it receives nothing).
	TArray<int> AroundOrder(int OrderSlot, float CenterXCm, float CenterYCm, int RadiusCm, const TArray<FHOLMoraleUnit>& Units)
	{
		TArray<int> Result;
		const double RadiusSquared = double(RadiusCm) * double(RadiusCm);
		for (int Index = 0; Index < Units.Num(); Index++)
		{
			if (!ReceivesFromOrder(OrderSlot, Units[Index]))
			{
				continue;
			}
			const double DX = double(Units[Index].XCm) - double(CenterXCm);
			const double DY = double(Units[Index].YCm) - double(CenterYCm);
			if (DX * DX + DY * DY <= RadiusSquared)
			{
				Result.Add(Index);
			}
		}
		return Result;
	}
}
