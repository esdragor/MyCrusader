// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLSelectionChipsReadLibrary.generated.h"

class APlayerController;

/**
 * HUD H-9, lot C (the selection block's chips, Script\UI\HOLSelectionChips.as; HOLSelectionChipsRead:: in script): what the controller's selection says
 * of its chips, already localized, and the chip's click. Same module as HOLGameNames (the names are read there); it sees the controller and its subgroups
 * (HeroesOfLiberty, a dependency of this module). Ported from UHOLGameScreen::RefreshUnitSelection (chips), RefreshBuildingGroup, RefreshChips and
 * HandleChipClicked (HOLGameScreen.cpp), which the switch removes. The rule of the chips stays in the controller (Tab's subgroups): one source.
 * The LOCTEXTs keep the namespace "HOLGameScreen" (Tools\Localization\fill_po.py reads HOLGameScreen,SelectionChip).
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLSelectionChipsReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * The chips of the selection of OwningPlayer's controller, in the order Tab goes through: several units (the player's: the controller's unit subgroups, "Tous (8)" first;
	 * another player's: "Tous" if several types, then one per type, soldiers first), or a group of buildings ("Tous (7)", "Maison (3)", a main centre alone).
	 * OutLabels empty: no chips. OutActive: the lit chip's index, -1 (INDEX_NONE) for none (another player's units, no chips).
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|SelectionChips")
	static void GetChipLabels(const APlayerController* OwningPlayer, TArray<FText>& OutLabels, int32& OutActive);

	/**
	 * The selection as a group of buildings (the controller's building group, no unit and no source selected). False and the outs empty otherwise.
	 * OutReport: the report string "%d buildings (own): X xN Y xM, subgroup S" (ShownSelection, word for word); OutOwner: "Vous"; OutColour: the first building's colour;
	 * OutActivityShown: ", sites N" or empty; OutActivityText: "N en construction".
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|SelectionChips")
	static bool GetBuildingGroupView(const APlayerController* OwningPlayer, FString& OutReport, FText& OutOwner, FLinearColor& OutColour, FString& OutActivityShown, FText& OutActivityText);

	/** A click on a chip: as many Tab as it takes from the active subgroup to the chip's; the journal line "HUD: chip %d clicked" (LogHOLUI, Display). False for no controller, no active subgroup or a chip out of range. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|SelectionChips")
	static bool ClickChip(APlayerController* OwningPlayer, int32 Chip);
};
