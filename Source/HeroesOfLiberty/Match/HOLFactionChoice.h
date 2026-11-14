// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Match/HOLMatchConfig.h"

/**
 * FactionChoiceInProgress.md: what the faction list of "Nouvelle partie" offers. Pure rules (the tests feed them the flag).
 *  - the Dawn and the Black Legions are chosen;
 *  - the Dragon, the Shadow and the Fire are listed but greyed (no content of their own yet);
 *  - the Templars are not listed at all, unless the session's cheat is on (a triple click on the title of the title screen),
 *    so the cheat stays secret. They play as the Dawn until they get rules of their own (HOLFactionRules::ResolveKey).
 */
namespace HOLFactionChoice
{
	/** The list offers the faction, and the player can take it. */
	HEROESOFLIBERTY_API bool IsChoosable(EHOLFaction Faction, bool bCheat);

	/** The list shows the faction (greyed or not): the first five always, the Templars with the cheat only. */
	HEROESOFLIBERTY_API bool IsListed(EHOLFaction Faction, bool bCheat);

	/** The listed factions, in the order of the enum (the drop-down's index is the enum's value). */
	HEROESOFLIBERTY_API TArray<EHOLFaction> GetListed(bool bCheat);

	/** What a launch plays: the Templars without the cheat fall back to the Dawn (a config from a stale screen, a lobby of another client). */
	HEROESOFLIBERTY_API EHOLFaction Sanitize(EHOLFaction Faction, bool bCheat);

	/** The enum's name as the manager writes it ("DawnOrder", "BlackLegions"... "Templars"), and back (false: unknown name). */
	HEROESOFLIBERTY_API FString GetName(EHOLFaction Faction);
	HEROESOFLIBERTY_API bool TryFindByName(const FString& Name, EHOLFaction& OutFaction);

	/** The session's cheat flag: process lifetime only, never saved. */
	HEROESOFLIBERTY_API bool IsCheatOn();
	HEROESOFLIBERTY_API void SetCheatOn(bool bOn);

	/**
	 * Counts the clicks of the title: three, each within Window seconds of the one before, are a triple click (the count then
	 * starts again, so six quick clicks are two triples). Slate gives no triple click of its own.
	 */
	struct HEROESOFLIBERTY_API FClickCounter
	{
		static constexpr double Window = 2.0;

		/** A click at Now (seconds); true on the third of a series. */
		bool Click(double Now);

		int32 Count = 0;
		double Last = 0.0;
	};
}
