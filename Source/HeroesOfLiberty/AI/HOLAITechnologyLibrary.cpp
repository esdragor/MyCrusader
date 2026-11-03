// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAITechnologyLibrary.h"

#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "HAL/IConsoleManager.h"
#include "Match/HOLPlayerState.h"
#include "Technologies/HOLTechnologyRules.h"
#include "Technologies/HOLTechnologyTypes.h"

namespace
{
	int32 AITechMinLevel = 1;
	FAutoConsoleVariableRef CVarAITechMinLevel(TEXT("hol.AI.TechMinLevel"), AITechMinLevel,
		TEXT("T7 (provisional): the lowest AI difficulty that researches technologies: 0 easy, 1 normal (default), 2 hard, 3 none"));
}

bool UHOLAITechnologyLibrary::ReadAITechnologyTable(TArray<FHOLAITechnology>& OutRows)
{
	OutRows.Reset();
	const UDataTable* Table = UHOLTechnologySettings::GetTable();
	if (!Table)
	{
		return false;
	}
	for (const FName Name : Table->GetRowNames())
	{
		const FHOLTechnologyRow* Row = Table->FindRow<FHOLTechnologyRow>(Name, TEXT("AI technology"), /*bWarnIfRowMissing*/ false);
		if (!Row)
		{
			continue;
		}
		FHOLAITechnology& Out = OutRows.AddDefaulted_GetRef();
		Out.Name = Name;
		Out.Building = Row->Building;
		Out.Category = static_cast<int32>(Row->Category);
		Out.MinTier = Row->MinTier;
		Out.MinHeroLevel = Row->MinHeroLevel;
		Out.Requires = Row->Requires;
		Out.CostFood = Row->Cost.Food;
		Out.CostWood = Row->Cost.Wood;
		Out.CostStone = Row->Cost.Stone;
		Out.CostGold = Row->Cost.Gold;
		Out.bImplemented = Row->Implemented;
	}
	return true;
}

void UHOLAITechnologyLibrary::ReadAITechnologiesDone(const UObject* WorldContextObject, const int32 PlayerIndex, TArray<FName>& OutDone)
{
	OutDone.Reset();
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	if (!GameState || PlayerIndex == INDEX_NONE)
	{
		return;
	}
	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		const AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState);
		if (HOLPlayerState && HOLPlayerState->GetPlayerIndex() == PlayerIndex)
		{
			OutDone = HOLPlayerState->GetTechnologies().Done;
			return;
		}
	}
}

int32 UHOLAITechnologyLibrary::GetAITechMinLevel()
{
	return AITechMinLevel;
}
