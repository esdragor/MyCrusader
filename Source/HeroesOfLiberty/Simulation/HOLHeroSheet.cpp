// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLHeroSheet.h"

#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLHeroXPRules.h"

FHOLHeroSheets HOLHeroSheet::Make(const FHOLHeroFacts& Facts, const FHOLHeroSheets& Previous)
{
	FHOLHeroSheets Sheets;
	Sheets.Private.XP = Previous.Private.XP;
	Sheets.Private.CooldownEndSteps = Previous.Private.CooldownEndSteps;
	if (!Facts.bPresent)
	{
		// His body destroyed after the fall: still dead, his death step kept. Never born: no hero.
		const bool bWasDead = Previous.Public.State == EHOLHeroState::Dead || Previous.Public.State == EHOLHeroState::ReadyToReturn;
		if (!bWasDead)
		{
			return FHOLHeroSheets();
		}
		// J8b.2: the XP of development still raises him while dead (D06).
		Sheets = Previous;
		Sheets.Public.Level = static_cast<uint8>(HOLHeroXP::GetLevel(Previous.Private.XP));
		return Sheets;
	}
	// J8b.2: his level from his XP (J8b-Q2).
	Sheets.Public.Level = static_cast<uint8>(HOLHeroXP::GetLevel(Previous.Private.XP));
	if (Facts.DeathStep >= 0 || Facts.Health <= 0.f)
	{
		// J8a.4: ready to return stays so while his body still lies.
		Sheets.Public.State = Previous.Public.State == EHOLHeroState::ReadyToReturn ? EHOLHeroState::ReadyToReturn : EHOLHeroState::Dead;
		Sheets.Public.Health = 0;
		// A fall seen before its step is known keeps the one already noted.
		Sheets.Private.DeathStep = Facts.DeathStep >= 0 ? static_cast<int32>(Facts.DeathStep) : Previous.Private.DeathStep;
		return Sheets;
	}
	Sheets.Public.State = EHOLHeroState::Alive;
	Sheets.Public.Health = static_cast<uint16>(FMath::Min(HOLCombat::GetShownHealth(Facts.Health), static_cast<int32>(MAX_uint16)));
	Sheets.Private.DeathStep = -1;
	return Sheets;
}
