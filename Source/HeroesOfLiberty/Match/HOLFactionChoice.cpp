// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLFactionChoice.h"

namespace HOLFactionChoice
{
	namespace
	{
		/** The session's cheat: lives and dies with the process (a triple click on the title), never written anywhere. */
		bool bCheat = false;
	}

	bool IsListed(const EHOLFaction Faction, const bool bWithCheat)
	{
		return Faction != EHOLFaction::Templars || bWithCheat;
	}

	bool IsChoosable(const EHOLFaction Faction, const bool bWithCheat)
	{
		switch (Faction)
		{
		case EHOLFaction::DawnOrder:
		case EHOLFaction::BlackLegions:
			return true;
		case EHOLFaction::Templars:
			return bWithCheat;
		default:
			// Dragon, Shadow, Fire: listed, greyed (no content of their own yet).
			return false;
		}
	}

	TArray<EHOLFaction> GetListed(const bool bWithCheat)
	{
		TArray<EHOLFaction> Listed;
		for (int32 Value = 0; Value <= static_cast<int32>(EHOLFaction::Templars); ++Value)
		{
			const EHOLFaction Faction = static_cast<EHOLFaction>(Value);
			if (IsListed(Faction, bWithCheat))
			{
				Listed.Add(Faction);
			}
		}
		return Listed;
	}

	EHOLFaction Sanitize(const EHOLFaction Faction, const bool bWithCheat)
	{
		return Faction == EHOLFaction::Templars && !bWithCheat ? EHOLFaction::DawnOrder : Faction;
	}

	FString GetName(const EHOLFaction Faction)
	{
		return StaticEnum<EHOLFaction>()->GetNameStringByValue(static_cast<int64>(Faction));
	}

	bool TryFindByName(const FString& Name, EHOLFaction& OutFaction)
	{
		if (Name.IsEmpty())
		{
			return false;
		}
		const int64 Value = StaticEnum<EHOLFaction>()->GetValueByNameString(Name);
		if (Value == INDEX_NONE)
		{
			return false;
		}
		OutFaction = static_cast<EHOLFaction>(Value);
		return true;
	}

	bool IsCheatOn()
	{
		return bCheat;
	}

	void SetCheatOn(const bool bOn)
	{
		bCheat = bOn;
	}

	bool FClickCounter::Click(const double Now)
	{
		// A click too long after the last one starts a new series; the click that does is the first of it.
		Count = Count > 0 && Now - Last <= Window ? Count + 1 : 1;
		Last = Now;
		if (Count >= 3)
		{
			Count = 0;
			return true;
		}
		return false;
	}
}
