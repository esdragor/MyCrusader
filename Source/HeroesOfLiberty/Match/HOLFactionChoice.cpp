// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLFactionChoice.h"

// STUB (red test first): no behaviour yet.
namespace HOLFactionChoice
{
	bool IsChoosable(const EHOLFaction, const bool) { return true; }
	bool IsListed(const EHOLFaction, const bool) { return true; }
	TArray<EHOLFaction> GetListed(const bool) { return {}; }
	EHOLFaction Sanitize(const EHOLFaction Faction, const bool) { return Faction; }
	FString GetName(const EHOLFaction) { return FString(); }
	bool TryFindByName(const FString&, EHOLFaction&) { return false; }
	bool IsCheatOn() { return false; }
	void SetCheatOn(const bool) {}
	bool FClickCounter::Click(const double) { return false; }
}
