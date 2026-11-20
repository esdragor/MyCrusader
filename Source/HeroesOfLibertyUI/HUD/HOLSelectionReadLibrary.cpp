// Copyright Epic Games, Inc. All Rights Reserved.

#include "HUD/HOLSelectionReadLibrary.h"

#include "Buildings/HOLBuilding.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HUD/HOLGameNames.h"
#include "Kismet/GameplayStatics.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerColors.h"
#include "Player/HOLPlayerController.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"

#define LOCTEXT_NAMESPACE "HOLGameScreen"

// Unique namespace: unity builds merge the .cpp files.
namespace HOLSelectionReadPrivate
{
	const UWorld* WorldOf(const UObject* WorldContextObject)
	{
		return GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	}

	/** The selection is the local controller's (the one the screen holds, UHOLGameScreen::Controller). */
	const AHOLPlayerController* LocalController(const UObject* WorldContextObject)
	{
		const UWorld* World = WorldOf(WorldContextObject);
		return World ? Cast<AHOLPlayerController>(UGameplayStatics::GetPlayerController(World, 0)) : nullptr;
	}

	/** Same words as HOLGameScreen.cpp's GetOwnerNameIn (IA0.1-Q1). */
	FText OwnerNameIn(const UWorld* World, const int32 Owner, const bool bOwn)
	{
		const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
		if (bOwn || !GameState)
		{
			return bOwn ? LOCTEXT("You", "Vous") : FText::Format(LOCTEXT("PlayerN", "Joueur {0}"), Owner + 1);
		}
		return HOLForumMessages::GetPlayerName(GameState->GetLaunchConfig(), Owner, INDEX_NONE);
	}
}

int32 UHOLSelectionReadLibrary::GetSelectionKind(const UObject* WorldContextObject)
{
	const AHOLPlayerController* Controller = HOLSelectionReadPrivate::LocalController(WorldContextObject);
	if (!Controller)
	{
		return 0;
	}
	// The order of UHOLGameScreen::Refresh: units, then a source, then a group of buildings, then the building alone.
	if (!Controller->GetSelectedUnits().IsEmpty())
	{
		return 2;
	}
	if (Controller->GetSelectedSource() != INDEX_NONE)
	{
		return 4;
	}
	if (Controller->HasBuildingGroup())
	{
		return 3;
	}
	return Controller->GetSelection() ? 1 : 0;
}

bool UHOLSelectionReadLibrary::GetBuildingSelectionView(const UObject* WorldContextObject, FHOLBuildingSelView& Out)
{
	Out = FHOLBuildingSelView();
	const AHOLPlayerController* Controller = HOLSelectionReadPrivate::LocalController(WorldContextObject);
	const AHOLBuilding* Building = Controller ? Controller->GetSelection() : nullptr;
	if (!Building)
	{
		return false;
	}
	Out.bValid = true;
	Out.bOwn = Building == Controller->GetOwnSelection();
	Out.TypeKey = Building->GetTypeName().ToString();
	Out.Name = HOLGameNames::GetBuildingName(Building->GetTypeName(), Building->GetType());
	Out.bFinished = Building->IsFinished();
	Out.bFoundation = Building->IsFoundation();
	Out.Progress = Building->GetProgress();
	Out.Health = Building->GetHealth();
	Out.MaxHealth = Building->GetMaxHealth();
	Out.OwnerSlot = Building->GetOwnerPlayer();
	Out.Owner = HOLSelectionReadPrivate::OwnerNameIn(HOLSelectionReadPrivate::WorldOf(WorldContextObject), Out.OwnerSlot, Out.bOwn);
	Out.Colour = FLinearColor(HOLPlayerColors::GetColor(Building->GetColorIndex()));
	Out.Builders = Building->GetBuildersAtWork();
	const FHOLForumBlock Forum = Building->GetForumBlock(Out.bOwn);
	Out.bForumShown = Forum.bShown;
	Out.bForumHideHealth = Forum.bHideHealth;
	Out.ForumShown = Forum.Shown;
	Out.ForumText = Forum.Text;
	Out.ForumLives = Forum.Lives;
	return true;
}

bool UHOLSelectionReadLibrary::GetSourceSelectionView(const UObject* WorldContextObject, FHOLSourceSelView& Out)
{
	Out = FHOLSourceSelView();
	const AHOLPlayerController* Controller = HOLSelectionReadPrivate::LocalController(WorldContextObject);
	const UWorld* World = HOLSelectionReadPrivate::WorldOf(WorldContextObject);
	const int32 SourceIndex = Controller ? Controller->GetSelectedSource() : INDEX_NONE;
	const UHOLSourceSubsystem* Sources = World ? World->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
	const FHOLSource* Source = Sources && Sources->GetSources().IsValidIndex(SourceIndex) ? &Sources->GetSources()[SourceIndex] : nullptr;
	const FName TypeName = Source && Sources->GetTypeNames().IsValidIndex(Source->TypeIndex) ? Sources->GetTypeNames()[Source->TypeIndex] : NAME_None;
	if (TypeName.IsNone())
	{
		return false;
	}
	Out.bValid = true;
	Out.Index = SourceIndex;
	Out.TypeKey = TypeName.ToString();
	Out.Name = HOLGameNames::GetSourceName(TypeName, Sources->GetType(Source->TypeIndex));
	Out.Remaining = Source->Remaining;
	return true;
}

FString UHOLSelectionReadLibrary::GetUnitTypeKey(const UObject* WorldContextObject, const int32 UnitId)
{
	return FString(); // stub (red)
}

void UHOLSelectionReadLibrary::ReadGarrisonOccupants(const UObject* WorldContextObject, const AHOLBuilding* Building, FHOLBuildingSelView& Out)
{
	// stub (red): leaves the two lists empty
}

FText UHOLSelectionReadLibrary::GetOwnerName(const UObject* WorldContextObject, const int32 OwnerSlot, const bool bOwn)
{
	return HOLSelectionReadPrivate::OwnerNameIn(HOLSelectionReadPrivate::WorldOf(WorldContextObject), OwnerSlot, bOwn);
}

#undef LOCTEXT_NAMESPACE
