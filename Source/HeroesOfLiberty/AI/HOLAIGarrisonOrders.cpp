// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIGarrisonOrders.h"

#include "Buildings/HOLBuilding.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLMatchConfig.h"

bool UHOLAIGarrisonOrderLibrary::IssueAIGarrisonOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (World && World->GetNetMode() == NM_Client)
	{
		UE_LOG(LogHOLMatch, Warning, TEXT("AI garrison order refused on a client: the AI runs on the server"));
		return false;
	}
	UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	if (!Commands || !Building || PlayerIndex < 0 || PlayerIndex >= FHOLMatchConfig::MaxSlots || UnitIds.IsEmpty())
	{
		return false;
	}
	TArray<uint32> Ids;
	Ids.Reserve(UnitIds.Num());
	for (const int32 Id : UnitIds)
	{
		if (Id > 0)
		{
			Ids.Add(static_cast<uint32>(Id));
		}
	}
	if (Ids.IsEmpty())
	{
		return false;
	}
	UE_LOG(LogHOLMatch, Display, TEXT("Garrison order: slot %d, %d unit(s) queued (AI)"), PlayerIndex, Ids.Num());
	Commands->IssueGarrisonOrder(PlayerIndex, Ids, Building);
	return true;
}
