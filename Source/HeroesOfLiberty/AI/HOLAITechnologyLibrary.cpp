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
	// D648: every level researches, the delay (game seconds before its first research) is what differs: Hard first, Normal later, Easy much later.
	// The values are the user's (Q-ia-tech-retards = A, D657): Hard 0, Normal 6 min, Easy 14 min.
	float AITechDelayEasy = 840.f;
	FAutoConsoleVariableRef CVarAITechDelayEasy(TEXT("hol.AI.TechDelayEasy"), AITechDelayEasy,
		TEXT("D648: game seconds before an Easy AI researches its first technology (default 840)"));
	float AITechDelayNormal = 360.f;
	FAutoConsoleVariableRef CVarAITechDelayNormal(TEXT("hol.AI.TechDelayNormal"), AITechDelayNormal,
		TEXT("D648: game seconds before a Normal AI researches its first technology (default 360)"));
	float AITechDelayHard = 0.f;
	FAutoConsoleVariableRef CVarAITechDelayHard(TEXT("hol.AI.TechDelayHard"), AITechDelayHard,
		TEXT("D648: game seconds before a Hard AI researches its first technology (default 0: the 20 Peasants of its economy are the only wait)"));
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

float UHOLAITechnologyLibrary::GetAITechDelay(const int32 Level)
{
	return Level <= 0 ? AITechDelayEasy : (Level == 1 ? AITechDelayNormal : AITechDelayHard);
}
