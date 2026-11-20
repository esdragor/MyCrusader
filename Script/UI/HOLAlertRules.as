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

// D804 B (Q-alerts-minor = B): the two minor alerts that come from an event, "research done" and "building finished": a line in the thread,
// a short sound of their own, a ping on the minimap (not for the population full line), once per MinorRepeatSeconds PER KIND.
enum EHOLMinorAlertKind
{
	ResearchDone,
	BuildingDone
}

// What a minor alert does when it goes: the caller (the game screen) adds the line, plays the sound, pings the minimap.
struct FHOLMinorAlert
{
	bool bShow = false;
	// How long the line stays in the thread, seconds.
	float LineSeconds = 0.0;
	// The short sound's key, one per kind (the screen maps it to an asset; PlayAlertSound's ping stands in until D804 B's sounds exist).
	FName SoundKey = NAME_None;
	// A ping on the minimap at the place of the event.
	bool bPingMinimap = false;
}

namespace HOLAlertRules
{
	// D804 B: one line per kind every 30 s, however many researches or buildings end meanwhile.
	const float MinorRepeatSeconds = 30.0;
	// How long a minor line stays in the thread (the thread's own default, D446).
	const float MinorLineSeconds = 8.0;

	// What the alert of this kind does at NowSeconds, the last line of THIS kind having gone at LastShownSeconds (the caller keeps one time per
	// kind, -1000 before the first): shown from MinorRepeatSeconds after it, with the line's time, the kind's own sound, a minimap ping.
	FHOLMinorAlert MinorAlert(EHOLMinorAlertKind Kind, float NowSeconds, float LastShownSeconds)
	{
		FHOLMinorAlert Alert;
		if (NowSeconds - LastShownSeconds < MinorRepeatSeconds)
			return Alert;
		Alert.bShow = true;
		Alert.LineSeconds = MinorLineSeconds;
		Alert.SoundKey = Kind == EHOLMinorAlertKind::ResearchDone ? n"MinorResearchDone" : n"MinorBuildingDone";
		Alert.bPingMinimap = true;
		return Alert;
	}

	// The line's text for each kind (French source like the other thread lines; the translations go with Tools/Localization/fill_po.py).
	FText MinorAlertText(EHOLMinorAlertKind Kind)
	{
		if (Kind == EHOLMinorAlertKind::ResearchDone)
			return NSLOCTEXT("HOLGameScreen", "MinorResearchDone", "Recherche terminée");
		return NSLOCTEXT("HOLGameScreen", "MinorBuildingDone", "Bâtiment achevé");
	}
}

// The game screen's bookkeeping for the minor events (D804 B, the part in script): the counters read last (UHOLScriptScreen::GetMinorEventsNow gives
// cumulative ones) and when each kind's line last went (-1000 before the first).
struct FHOLMinorWatch
{
	bool bRead = false;
	int Buildings = 0;
	int Researches = 0;
	float LastBuildingShown = -1000.0;
	float LastResearchShown = -1000.0;
}

// What the screen does for one reading of the counters: one alert per kind (bShow false = nothing for that kind).
struct FHOLMinorStep
{
	FHOLMinorAlert Building;
	FHOLMinorAlert Research;
}

namespace HOLAlertRules
{
	// One reading of the cumulative counters at NowSeconds: the first reading only takes the baseline; a counter that went up since the last reading
	// is an event of that kind, which MinorAlert turns into line / sound / ping (so one line per kind per MinorRepeatSeconds, however many events).
	FHOLMinorStep MinorStep(FHOLMinorWatch& Watch, int Buildings, int Researches, float NowSeconds)
	{
		FHOLMinorStep Step;
		return Step;
	}
}