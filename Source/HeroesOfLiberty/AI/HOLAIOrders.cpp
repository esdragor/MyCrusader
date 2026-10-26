// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIOrders.h"

#include "Commands/HOLPlayerOrders.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HeroesOfLibertyLog.h"

namespace
{
	/** The world of WorldContextObject when it holds the authority (server, solo); null on a client. */
	UWorld* GetAuthorityWorld(const UObject* WorldContextObject)
	{
		UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
		if (World && World->GetNetMode() == NM_Client)
		{
			UE_LOG(LogHOLMatch, Warning, TEXT("AI order refused on a client: the AI runs on the server"));
			return nullptr;
		}
		return World;
	}

	/** Script's int32 identifiers back to the simulation's uint32 (a negative one names no unit: 0). */
	TArray<uint32> ToUnitIds(const TArray<int32>& UnitIds)
	{
		TArray<uint32> Ids;
		Ids.Reserve(UnitIds.Num());
		for (const int32 Id : UnitIds)
		{
			Ids.Add(Id > 0 ? static_cast<uint32>(Id) : 0u);
		}
		return Ids;
	}
}

bool UHOLAIOrderLibrary::IssueAIGatherOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const TArray<int32>& UnitIds, const int32 SourceIndex,
	const bool bQueue)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	return World && HOLPlayerOrders::Gather(*World, PlayerIndex, ToUnitIds(UnitIds), SourceIndex, bQueue);
}

bool UHOLAIOrderLibrary::IssueAIMoveOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const TArray<int32>& UnitIds, const FVector Target,
	const bool bQueue)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	return World && HOLPlayerOrders::Move(*World, PlayerIndex, ToUnitIds(UnitIds), Target, bQueue);
}
