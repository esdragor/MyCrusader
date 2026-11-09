// The morale display rule (M6, MoraleInProgress.md D707), pure, from a HOLMorale::FHOLMoraleState; tested in HOLMoraleBlock_Test.as.
// What the player reads: beside the health bar a green pastille (a bonus), a red one (a malus), a gold one (the immunity); in the selection block
// the list of the effects, bonus first then malus (each group in the order the effects were posed), with the time left on OUR units only (an
// aura has no time: it lasts while the unit is in range). A visible enemy unit shows its effects without a time. A Peasant shows nothing, ever
// (D705: morale never reaches the Peasants). The names come from the effects table (HOLMoraleTable::Name(Id)); the C++ drawing and the script
// block read this rule. Assumptions to confirm at the display step: the order inside a group is the posing order; the immunity time is shown
// on our units like an effect time (the dossier only says "pastille doree").
namespace HOLMoraleBlock
{
	// The pastilles beside the health bar: a green one for a bonus, a red one for a malus, a gold one for the immunity.
	struct FHOLMoraleBadge
	{
		bool bBonus = false;
		bool bMalus = false;
		bool bImmune = false;
	}

	// One line of the selection block: the effect (a row of the table), its sign, the seconds left (-1 = none shown: an aura, or not our unit).
	struct FHOLMoraleLine
	{
		int Id = -1;
		bool bMalus = false;
		bool bAura = false;
		int SecondsLeft = -1;
	}

	// Milliseconds to the second still started (a started second counts).
	int SecondsOf(int Ms)
	{
		return Ms <= 0 ? 0 : (Ms + 999) / 1000;
	}

	FHOLMoraleBadge Badge(const HOLMorale::FHOLMoraleState& State, bool bPeasant)
	{
		FHOLMoraleBadge Badge;
		if (bPeasant)
			return Badge;
		for (int Index = 0; Index < State.Effects.Num(); Index++)
		{
			if (HOLMorale::IsMalus(State.Effects[Index]))
				Badge.bMalus = true;
			else
				Badge.bBonus = true;
		}
		Badge.bImmune = State.ImmunityMs > 0;
		return Badge;
	}

	TArray<FHOLMoraleLine> Lines(const HOLMorale::FHOLMoraleState& State, bool bOwn, bool bPeasant)
	{
		TArray<FHOLMoraleLine> Result;
		if (bPeasant)
			return Result;
		// Two passes keep the posing order inside each group: the bonus first, then the malus.
		for (int Pass = 0; Pass < 2; Pass++)
		{
			const bool bWantMalus = Pass == 1;
			for (int Index = 0; Index < State.Effects.Num(); Index++)
			{
				const HOLMorale::FHOLMoraleEffect Effect = State.Effects[Index];
				if (HOLMorale::IsMalus(Effect) != bWantMalus)
					continue;
				FHOLMoraleLine Line;
				Line.Id = Effect.Id;
				Line.bMalus = bWantMalus;
				Line.bAura = Effect.bAura;
				Line.SecondsLeft = (bOwn && !Effect.bAura) ? SecondsOf(Effect.DurationMs) : -1;
				Result.Add(Line);
			}
		}
		return Result;
	}

	// The seconds of immunity left, on our own unit only; -1 when there is none to show.
	int ImmunitySeconds(const HOLMorale::FHOLMoraleState& State, bool bOwn, bool bPeasant)
	{
		if (bPeasant || !bOwn || State.ImmunityMs <= 0)
			return -1;
		return SecondsOf(State.ImmunityMs);
	}
}
