// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLSelectionReadLibrary.generated.h"

/**
 * HUD H-9, lot A (the selection block, Script\UI\HOLSelection.as; HOLSelectionRead:: in script): what the local player's controller
 * has selected, in simple types, for the script panel. In the UI module (not Match) because the names come from HOLGameNames (HUD/).
 * The localized texts (building and source names, owner, forum line) are produced here and come back formatted: the script only
 * decides what is shown. The viewer is the local player controller (the selection is the controller's, a spectator's included).
 */
USTRUCT(BlueprintType)
struct FHOLBuildingSelView
{
	GENERATED_BODY()

	/** False: no building selected (or none seen); every other field is then empty. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	bool bValid = false;

	/** The type key, as ShownSelection writes it ("MainCentre"; Building->GetTypeName()). */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	FString TypeKey;

	/** HOLGameNames::GetBuildingName, localized. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	FText Name;

	/** The selection is the local player's own (GetSelection() == GetOwnSelection()). */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	bool bOwn = false;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	bool bFinished = false;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	bool bFoundation = false;

	/** The site's progress, 0 to 1 (Building->GetProgress()). */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	float Progress = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	int32 Health = 0;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	int32 MaxHealth = 0;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	int32 OwnerSlot = INDEX_NONE;

	/** The owner as the block writes it: Vous, or the slot's name as the players' table shows it. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	FText Owner;

	/** The owner's colour (HOLPlayerColors::GetColor(ColorIndex)). */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	FLinearColor Colour = FLinearColor::White;

	/** The builders at work on a site (only its owner receives it, D313). */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	int32 Builders = 0;

	/** A main forum (FHOLForumBlock = Building->GetForumBlock(bOwn)): the block is shown. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	bool bForumShown = false;

	/** Forum: in ruin or rebuilding, the health row hidden. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	bool bForumHideHealth = false;

	/** Forum: FHOLForumBlock::Shown (", lives 2", ", in ruin 25 s, lives 1"...), the report's text. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	FString ForumShown;

	/** Forum: FHOLForumBlock::Text, localized; empty when it stands. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	FText ForumText;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	int32 ForumLives = 0;
};

USTRUCT(BlueprintType)
struct FHOLSourceSelView
{
	GENERATED_BODY()

	/** False: no source selected, or its type unknown. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	bool bValid = false;

	/** The source's index (ShownSelection writes "<type> source <index>"). */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	int32 Index = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	FString TypeKey;

	/** HOLGameNames::GetSourceName, localized. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	FText Name;

	/** What it still holds. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Selection")
	int32 Remaining = 0;
};

UCLASS()
class HEROESOFLIBERTYUI_API UHOLSelectionReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Which path the block takes (UHOLGameScreen::Refresh's order): 0 nothing, 1 a building (alone), 2 units, 3 a group of buildings, 4 a source. 0 without a local controller. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Selection", meta = (WorldContext = "WorldContextObject"))
	static int32 GetSelectionKind(const UObject* WorldContextObject);

	/** The building selected alone (GetSelection()). False and Out empty when none. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Selection", meta = (WorldContext = "WorldContextObject"))
	static bool GetBuildingSelectionView(const UObject* WorldContextObject, FHOLBuildingSelView& Out);

	/** The source selected (tree, vein). False and Out empty when none. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Selection", meta = (WorldContext = "WorldContextObject"))
	static bool GetSourceSelectionView(const UObject* WorldContextObject, FHOLSourceSelView& Out);

	/** Vous when bOwn, else the slot's name as the players' table shows it (an AI's name), Joueur N without a game state. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Selection", meta = (WorldContext = "WorldContextObject"))
	static FText GetOwnerName(const UObject* WorldContextObject, int32 OwnerSlot, bool bOwn);
};
