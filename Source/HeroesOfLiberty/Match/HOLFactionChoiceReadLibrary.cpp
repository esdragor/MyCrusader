// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLFactionChoiceReadLibrary.h"

#include "Match/HOLFactionChoice.h"

bool UHOLFactionChoiceReadLibrary::IsChoosable(const EHOLFaction Faction, const bool bCheat)
{
	return HOLFactionChoice::IsChoosable(Faction, bCheat);
}

bool UHOLFactionChoiceReadLibrary::IsListed(const EHOLFaction Faction, const bool bCheat)
{
	return HOLFactionChoice::IsListed(Faction, bCheat);
}

TArray<EHOLFaction> UHOLFactionChoiceReadLibrary::GetListed(const bool bCheat)
{
	return HOLFactionChoice::GetListed(bCheat);
}

EHOLFaction UHOLFactionChoiceReadLibrary::Sanitize(const EHOLFaction Faction, const bool bCheat)
{
	return HOLFactionChoice::Sanitize(Faction, bCheat);
}

FString UHOLFactionChoiceReadLibrary::GetName(const EHOLFaction Faction)
{
	return HOLFactionChoice::GetName(Faction);
}

bool UHOLFactionChoiceReadLibrary::TryFindByName(const FString& Name, EHOLFaction& OutFaction)
{
	return HOLFactionChoice::TryFindByName(Name, OutFaction);
}

bool UHOLFactionChoiceReadLibrary::IsCheatOn()
{
	return HOLFactionChoice::IsCheatOn();
}

void UHOLFactionChoiceReadLibrary::SetCheatOn(const bool bOn)
{
	HOLFactionChoice::SetCheatOn(bOn);
}

double UHOLFactionChoiceReadLibrary::GetClickWindow()
{
	return HOLFactionChoice::FClickCounter::Window;
}

bool UHOLFactionChoiceReadLibrary::Click(FHOLClickCounter& Counter, const double Now)
{
	HOLFactionChoice::FClickCounter Rule;
	Rule.Count = Counter.Count;
	Rule.Last = Counter.Last;
	const bool bTriple = Rule.Click(Now);
	Counter.Count = Rule.Count;
	Counter.Last = Rule.Last;
	return bTriple;
}
