// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIRageOrders.h"

#include "Commands/HOLCommandSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLMatchConfig.h"

bool UHOLAIRageOrderLibrary::IssueAIRageOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const TArray<int32>& UnitIds)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (World && World->GetNetMode() == NM_Client)
	{
		UE_LOG(LogHOLMatch, Warning, TEXT("AI rage order refused on a client: the AI runs on the server"));
		return false;
	}
	UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	if (!Commands || PlayerIndex < 0 || PlayerIndex >= FHOLMatchConfig::MaxSlots || UnitIds.IsEmpty())
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
	UE_LOG(LogHOLMatch, Display, TEXT("Rage order: slot %d, %d Damned Warrior(s) queued (AI)"), PlayerIndex, Ids.Num());
	Commands->IssueRageOrder(PlayerIndex, Ids);
	return true;
}
