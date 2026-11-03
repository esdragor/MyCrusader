// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLDiplomacyReadLibrary.generated.h"

/**
 * What the Diplomacy panel (D7 of DiplomacyInProgress.md, GDD D387) reads of the game, for Angelscript (HOLDiplomacyRead:: in script): the
 * stances of the book as this machine has them (the replicated state of AHOLGameState), who is in the match, their names, factions, colours and
 * lives as the players' table shows them, the local player's Market and the tribute's tax. Read only: the orders go through AHOLPlayerController
 * (RequestStance, RequestShareVision, RequestPanelTribute).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLDiplomacyReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** The match is in the Diplomacy mode (the stances can be set); false in the standard modes (locked: the teams). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static bool IsDiplomacyMode(const UObject* WorldContextObject);

	/** Slot holds a player (human or AI) of this match. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static bool IsSlotInMatch(const UObject* WorldContextObject, int32 Slot);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static bool IsSlotAI(const UObject* WorldContextObject, int32 Slot);

	/** The stance From has set toward To (0 Enemy, 1 Neutral, 2 Ally; HOLDiplomacy::StanceOf). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static int32 GetStance(const UObject* WorldContextObject, int32 From, int32 To);

	/** The relation of a pair (the most hostile of the two stances, D375). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static int32 GetRelation(const UObject* WorldContextObject, int32 A, int32 B);

	/** From lets To see what it sees (D371). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static bool GetVision(const UObject* WorldContextObject, int32 From, int32 To);

	/** The slot's colour in the palette (HOLPlayerColors), grey outside the match. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static FLinearColor GetSlotColor(const UObject* WorldContextObject, int32 Slot);

	/** The slot's name as the players' table shows it from Viewer's seat: « Vous » on its own row, « Joueur N » otherwise (a named human: its name). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static FString GetSlotNameText(const UObject* WorldContextObject, int32 Viewer, int32 Slot);

	/** The slot's lives as the players' table shows them (« 2 vies », « 1 vie »; empty once defeated, « Vaincu » is the state). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static FString GetSlotLivesText(const UObject* WorldContextObject, int32 Viewer, int32 Slot);

	/** The slot's state as the players' table shows it (« Ruine », « Vaincu »...), empty standing. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static FString GetSlotStateText(const UObject* WorldContextObject, int32 Viewer, int32 Slot);

	/** The slot's faction name in the current language (the new game screen's own table, § 13). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static FString GetSlotFactionText(const UObject* WorldContextObject, int32 Slot);

	/** The slot owns a finished Market (D372: the tribute needs one). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static bool HasFinishedMarket(const UObject* WorldContextObject, int32 Slot);

	/** The tribute's tax in percent (HOLTribute::DefaultTaxPercent, D372). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static int32 GetTributeTaxPercent();

	/** The panel's presentation (hol.UI.DiplomacyLayout 0|1|2): 0 three buttons, 1 cycle on the colour chip, 2 drop-down menu. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static int32 GetPanelLayout();
};
