// The minor alerts' pure rules (D202, GDD 15: « fil discret sur le côté »; AlertsMinorInProgress.md): when a minor alert's line goes to the thread
// of messages (D446) and how it is kept from repeating. No world here: the game screen reads the game, calls these, and adds the line.
// Defaults pending Retours/Q-alerts-minor.md (answer A meanwhile: a silent line, no ping, D449).

// What the screen last saw of its player's population, for the population full alert.
struct FHOLPopulationAlertWatch
{
	// The last reading was full (Used >= Cap).
	bool bWasFull = false;
	// When the last line went (seconds, the HUD's real time).
	float LastAlertSeconds = -1000.0;
}

namespace HOLAlertRules
{
	// Q-alerts-minor (default A): no new population full line sooner than this after the last one.
	const float PopulationRepeatSeconds = 30.0;

	// Used counts the units in production too (D282); Cap the places the buildings give (175 at most, D03, D83).
	bool IsPopulationFull(int Used, int Cap)
	{
		// No place at all (no finished building yet) is not « full »: nothing to build a house for is said.
		return Cap > 0 && Used >= Cap;
	}

	// True when the population full line goes now: the population has just become full and the last line is RepeatSeconds old or more.
	// Full again sooner (a unit died and another was queued) stays quiet until the population has room again and fills once more later.
	bool ShouldAlertPopulationFull(FHOLPopulationAlertWatch& Watch, int Used, int Cap, float NowSeconds, float RepeatSeconds)
	{
		const bool bFull = IsPopulationFull(Used, Cap);
		const bool bGoes = bFull && !Watch.bWasFull && NowSeconds - Watch.LastAlertSeconds >= RepeatSeconds;
		Watch.bWasFull = bFull;
		if (bGoes)
			Watch.LastAlertSeconds = NowSeconds;
		return bGoes;
	}

	// D833: how long the population full line stays in the thread, seconds (the thread's default is shorter; this one asks to be read).
	const float PopulationFullLineSeconds = 15.0;

	// The line's text: the production queue's note, the same key (« Population pleine », translated in Tools/Localization/fill_po.py).
	FText PopulationFullText()
	{
		return NSLOCTEXT("HOLGameScreen", "PopulationFull", "Population pleine");
	}
}

// D804 B (Q-alerts-minor): a unit or a building of the player attacked where the camera does not look: a line (and a short sound),
// at most once per AttackRepeatSeconds, however many hits land meanwhile.
struct FHOLAttackAlertWatch
{
	// When the last line went (seconds, the HUD's real time).
	float LastAlertSeconds = -1000.0;
}

namespace HOLAlertRules
{
	// D804 B: "attaque ailleurs" once per 30 s.
	const float AttackRepeatSeconds = 30.0;

	// True when the attacked elsewhere line goes now: a hit on something of the player out of the camera's view, the last line
	// RepeatSeconds old or more. A hit in view says nothing (the player sees it) and does not delay the next line.
	bool ShouldAlertAttackedElsewhere(FHOLAttackAlertWatch& Watch, bool bHitInView, float NowSeconds, float RepeatSeconds)
	{
		if (bHitInView || NowSeconds - Watch.LastAlertSeconds < RepeatSeconds)
			return false;
		Watch.LastAlertSeconds = NowSeconds;
		return true;
	}
}