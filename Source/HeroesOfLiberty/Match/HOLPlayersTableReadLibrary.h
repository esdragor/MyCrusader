// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Match/HOLForumRules.h"
#include "HOLPlayersTableReadLibrary.generated.h"

class AHOLGameState;

/**
 * One row of the players' table (FHOLPlayersTableRow, HOLForumRules::GetPlayersTable) as Angelscript reads it (HUD H-4, D839):
 * the texts as strings, the colour already converted as the C++ screen converted it, the relation as its number
 * (EHOLPlayerRelation: None 0, Self 1, Ally 2, Enemy 3, Neutral 4). A copy of the rule's output, no rule here.
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLPlayersTableRowRead
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Match")
	int32 Slot = -1;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Match")
	int32 ColorIndex = 0;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Match")
	FString Name;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Match")
	FString Lives;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Match")
	FString State;

	/** FLinearColor(HOLPlayerColors::GetColor(ColorIndex)): the same sRGB conversion as the C++ screen's. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Match")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Match")
	bool bGreyed = false;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Match")
	bool bStruck = false;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Match")
	bool bAway = false;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Match")
	bool bAIHeld = false;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Match")
	bool bNewGroup = false;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Match")
	int32 Relation = 0;

	/** The rule's report text ("slot 1 you colour 1 lives 1 ruin"): the table's change signature and hol.HUD.Report's. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Match")
	FString Shown;
};

/**
 * The players' table's rows read by Angelscript (namespace HOLPlayersTableRead, implicit world context). Wrappers only: the rows
 * stay HOLForumRules::GetPlayersTable's (one source); the script draws them and writes the journal.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLPlayersTableReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** FHOLMatchConfig::MaxSlots: the seats the table builds a row for at most. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static int32 GetMaxSlots();

	/**
	 * The table's rows as the C++ screen's RefreshPlayersTable built them (same forums, tiers, masks, Diplomacy book, launch
	 * configuration); LocalSlot is the shown player's slot (INDEX_NONE for a spectator). Empty without a game state.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Match", meta = (WorldContext = "WorldContextObject"))
	static void GetRows(const UObject* WorldContextObject, int32 LocalSlot, TArray<FHOLPlayersTableRowRead>& OutRows);

	/** C++ side (not script): the rows of GameState for the viewer LocalSlot; the C++ screen's Describe prints them. */
	static TArray<FHOLPlayersTableRow> BuildRows(const AHOLGameState* GameState, int32 LocalSlot);
};