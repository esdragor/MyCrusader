// The morale display rule (M6, MoraleInProgress.md D707): what a unit shows of its morale effects, pure, from a HOLMorale::FHOLMoraleState.
// STUB written with the red test (nothing is shown yet): the real rule comes in the next commit.
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

	FHOLMoraleBadge Badge(const HOLMorale::FHOLMoraleState& State, bool bPeasant)
	{
		FHOLMoraleBadge Badge;
		return Badge;
	}

	TArray<FHOLMoraleLine> Lines(const HOLMorale::FHOLMoraleState& State, bool bOwn, bool bPeasant)
	{
		TArray<FHOLMoraleLine> Result;
		return Result;
	}

	int ImmunitySeconds(const HOLMorale::FHOLMoraleState& State, bool bOwn, bool bPeasant)
	{
		return -1;
	}
}
